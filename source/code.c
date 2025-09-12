#include <fat.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <malloc.h>
#include <gccore.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <unistd.h>
#include <strings.h>
#include <gctypes.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <curl/curl.h>
#include <wiisocket.h>
#include <wiiuse/wpad.h>

#include "cJSON.h"

u32 __stack_size = 64 * 1024; 

#define APIURL "http://backend.psogc.tk/api"
#define THEMEURL APIURL "/database?platform=%s&page=%d"
#define THEME_DOWN_PATH "sd:/themes/"
#define THEME_DOWN_URL APIURL "/themes/%s/download"
#define TMP "sd:/temp/"
#define MAXWII 4

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

struct FileStruct {
    char filename[256];
    FILE *stream;
    const char *friendly_name;
};

typedef struct {
    char id[16];
    char name[128];
    char description[256];
} Theme;

typedef enum {
    VIEW_PLAT_SEL,
    VIEW_GAL,
    VIEW_THEME_INF,
    VIEW_DOWN,
    VIEW_ERR
} ViewState;

struct {
    Theme *themes;
    int theme_count;
    int curr_page;
    int tot_page;
    int sel_ind;
    int runin;
    ViewState view_state;
    const char *platfor[3];
    int sel_plat_ind;
    char status_mesg[256];
    bool do_print;
    const char *more_info;
} state;

void pr_view();
int del_dir_recur(const char *path);


void cls_con(void) {
    printf("\x1b[2J");
    fflush(stdout);
}

int do_dir_exist(const char *path) {
    mkdir(path, 0700);
    return 1;
}

int del_dir_recur(const char *path) {
   DIR *d = opendir(path);
   size_t path_len = strlen(path);
   int r = -1;

   if (d) {
      struct dirent *p;

      r = 0;
      while (!r && (p=readdir(d))) {
          int r2 = -1;
          char *buf;
          size_t len;

          //* Skip the names "." and ".." as we don't want to recurse on them or else...
          if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
             continue;

          len = path_len + strlen(p->d_name) + 2; 
          buf = malloc(len);

          if (buf) {
             struct stat statbuf;

             snprintf(buf, len, "%s/%s", path, p->d_name);
             if (!stat(buf, &statbuf)) {
                if (S_ISDIR(statbuf.st_mode))
                   r2 = del_dir_recur(buf);
                else
                   r2 = unlink(buf);
             }
             free(buf);
          }
          r = r2;
      }
      closedir(d);
   }

   if (!r)
      r = rmdir(path);

   return r;
}

static size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userp) {
    size_t written = fwrite(ptr, size, nmemb, (FILE *)userp);
    return written * size;
}

static size_t theme_write_callback(void *ptr, size_t size, size_t nmemb, void *userp) {
    struct FileStruct *out = (struct FileStruct *)userp;
    
    // If the stream isn't open, open it now.
    if (!out->stream) {
        out->stream = fopen(out->filename, "wb");
        if (!out->stream) {
            perror("fopen error in theme_write_callback");
            return 0; // Signal an error to curl
        }
    }

    size_t written_items = fwrite(ptr, size, nmemb, out->stream);
    return written_items * size;
}

#define ALPHABET_SIZE 256

static void precomp_shifts(const char *needle, size_t needle_len, int shifts[ALPHABET_SIZE]) {
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        shifts[i] = needle_len;
    }
    for (size_t i = 0; i < needle_len - 1; i++) {
        shifts[(unsigned char)tolower(needle[i])] = needle_len - 1 - i;
    }
}

char *strcasestr(const char *haystack, const char *needle) {
    size_t haystack_len = strlen(haystack);
    size_t needle_len = strlen(needle);

    if (needle_len == 0) return (char *)haystack;
    if (haystack_len < needle_len) return NULL;

    int shifts[ALPHABET_SIZE];
    precomp_shifts(needle, needle_len, shifts);

    size_t i = 0;
    while (i <= haystack_len - needle_len) {
        size_t j = needle_len - 1;
        while (j < needle_len && tolower((unsigned char)haystack[i + j]) == tolower((unsigned char)needle[j])) {
            if (j == 0) return (char *)(haystack + i);
            j--;
        }
        i += shifts[(unsigned char)tolower(haystack[i + needle_len - 1])];
    }
    return NULL;
}


static size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata) {
    struct FileStruct *file_data = (struct FileStruct *)userdata;
    const char *disposition_prefix = "Content-Disposition:";
    const char *filename_prefix = "filename=";
    size_t realsize = size * nitems;

    if (strncasecmp(buffer, disposition_prefix, strlen(disposition_prefix)) == 0) {
        char *filename_start = strcasestr(buffer, filename_prefix);
        if (filename_start) {
            filename_start += strlen(filename_prefix);
            char server_filename[256] = {0};
            
            if (*filename_start == '"') {
                filename_start++;
                char *filename_end = strchr(filename_start, '"');
                if (filename_end) {
                    size_t len = filename_end - filename_start;
                    if (len < sizeof(server_filename) -1) {
                        strncpy(server_filename, filename_start, len);
                        server_filename[len] = '\0';
                    }
                }
            } else {
                char *filename_end = strpbrk(filename_start, " \r\n;");
                if (filename_end) {
                    size_t len = filename_end - filename_start;
                    if (len < sizeof(server_filename) - 1) {
                       strncpy(server_filename, filename_start, len);
                       server_filename[len] = '\0';
                    }
                } else {
                    strncpy(server_filename, filename_start, sizeof(server_filename) -1);
                }
            }

            if (server_filename[0] != '\0') {
                const char *extension = strrchr(server_filename, '.');
                if (extension) {
                    snprintf(file_data->filename, sizeof(file_data->filename), "%s%s%s", THEME_DOWN_PATH, file_data->friendly_name, extension);
                } else {
                    snprintf(file_data->filename, sizeof(file_data->filename), "%s%s", THEME_DOWN_PATH, file_data->friendly_name);
                }
            }
        }
    }
    return realsize;
}

void set_ter_raw() {
    VIDEO_Init();
    WPAD_Init();
    rmode = VIDEO_GetPreferredMode(NULL);
    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);
    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (rmode->viTVMode & VI_NON_INTERLACE) {VIDEO_WaitVSync(); printf("\x1b[2;0H");}
    wiisocket_init();
    if (wiisocket_get_status() == 0 || wiisocket_get_status() != 1) {
        printf("Error Init Network, Exiting...");
        sleep(5);
        exit(-1);
    }
    if (!fatInitDefault()) {
        printf("Error Init SD Card, Check your SD card.\n");
        printf("Is it inserted? Is it FAT32? Is it locked?\n");
        sleep(10);
        exit(-1); // Stop the program
    }
}

void reset_term_mode() {
    //printf("Cleaning up temporary files...\n");
    const char *tmpfile_path = "sd:/temp/tmp_list.json";
    if (remove(tmpfile_path) == 0){
        //printf("we ok");
    } else {
        //printf("WE FUCKED UP BAD TIME ITS GONNA CRASHHHHHHHH")
    }
    //if (del_dir_recur(TMP) == 0) {
    if (rmdir(TMP) == 0) {
        //printf("Temp folder deleted successfully.\n");
    } else {
        //fprintf(stderr, "Warning: Failed to delete temp folder: %s\n", TMP);
    }
    sleep(1);
    cls_con();
}


u32 Wpad_GetButtons(void) {
    u32 buttons = 0;
    WPAD_ScanPads();
    for (int i = 0; i < MAXWII; i++) {
        buttons |= WPAD_ButtonsDown(i);
    }
    return buttons;
}

u32 Wpad_WaitButtons(void) {
    u32 buttons = 0;
    while (!buttons) {
        buttons = Wpad_GetButtons();
        VIDEO_WaitVSync();
    }
    return buttons;
}

u32 get_key_press() {
    return Wpad_WaitButtons();
}

int get_themes(const char* platform, int page) {
    CURL *curl_handle;
    CURLcode res;
    char url[256];
    const char *temp_json_path = TMP "tmp_list.json";
    char *json_buffer = NULL;
    cJSON *root = NULL;

    if (!do_dir_exist(TMP)) {
        snprintf(state.status_mesg, sizeof(state.status_mesg), "Error: Could not create temp directory.");
        state.view_state = VIEW_ERR;
        state.do_print = true;
        return 0;
    }
    FILE *temp_file = fopen(temp_json_path, "wb");
    if (!temp_file) {
        // The file wasn't created, which means the download failed.
        snprintf(state.status_mesg, sizeof(state.status_mesg), "Error: Download failed.");
        state.more_info = "Temporary file was not created.";
        state.view_state = VIEW_ERR;
        state.do_print = true;
        return 0;
    }
    fseek(temp_file, 0, SEEK_END);
    long file_size = ftell(temp_file);

    snprintf(url, sizeof(url), THEMEURL, platform, page);
    curl_handle = curl_easy_init();
    if (!curl_handle) {
        fclose(temp_file); // Always close the file on error!
        snprintf(state.status_mesg, sizeof(state.status_mesg), "Error: Failed to initialize curl.");
        state.view_state = VIEW_ERR;
        state.do_print = true;
        return 0;
    }

    // Use our new, simpler callback and pass the file handle directly
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)temp_file);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl");
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 20L);
    curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1L);

    cls_con();
    printf("Downloading theme list...\n");
    res = curl_easy_perform(curl_handle);

    // 3. IMPORTANT: Close the file handle as soon as the download is done.
    fclose(temp_file);
    //fatsync("sd:");
    curl_easy_cleanup(curl_handle);


    if (res != CURLE_OK) {
        snprintf(state.status_mesg, sizeof(state.status_mesg), "Network Error: %s", curl_easy_strerror(res));
        state.view_state = VIEW_ERR;
        state.do_print = true;
        remove(temp_json_path); 
        return 0;
    }

    FILE *json_file = fopen(temp_json_path, "rb");
    if (!json_file) {
        snprintf(state.status_mesg, sizeof(state.status_mesg), "Error: Could not re-open temp file for reading.");
        state.view_state = VIEW_ERR;
        state.do_print = true;
        remove(temp_json_path);
        return 0;
    }

    fseek(json_file, 0, SEEK_END);
    file_size = ftell(json_file);
    fseek(json_file, 0, SEEK_SET);

    if (file_size <= 0) {
        fclose(json_file);
        remove(temp_json_path);
        return 0;
    }

    json_buffer = malloc(file_size + 1);
    if (!json_buffer) {
        fclose(json_file);
        remove(temp_json_path);
        return 0;
    }

    if(fread(json_buffer, 1, file_size, json_file) != (size_t)file_size) {
        free(json_buffer);
        fclose(json_file);
        remove(temp_json_path);
        return 0;
    }
    fclose(json_file);
    remove(temp_json_path);
    json_buffer[file_size] = '\0';

    root = cJSON_Parse(json_buffer);
    free(json_buffer);

    if (root == NULL) {
        cJSON_Delete(root);
        return 0;
    }

    cJSON *items_array = cJSON_GetObjectItemCaseSensitive(root, "items");
    cJSON *total_pages_item = cJSON_GetObjectItemCaseSensitive(root, "total_pages");

    if (cJSON_IsArray(items_array) && cJSON_IsNumber(total_pages_item)) {
        state.theme_count = cJSON_GetArraySize(items_array);
        state.tot_page = total_pages_item->valueint;

        free(state.themes);
        state.themes = calloc(state.theme_count, sizeof(Theme));
        if (!state.themes) {
            cJSON_Delete(root);
            return 0;
        }

        int i = 0;
        cJSON *theme_item;
        cJSON_ArrayForEach(theme_item, items_array) {
            cJSON* id_json = cJSON_GetObjectItemCaseSensitive(theme_item, "id");
            cJSON* name_json = cJSON_GetObjectItemCaseSensitive(theme_item, "name");
            cJSON* desc_json = cJSON_GetObjectItemCaseSensitive(theme_item, "description");

            if (cJSON_IsString(id_json) && cJSON_IsString(name_json) && cJSON_IsString(desc_json)) {
                strncpy(state.themes[i].id, id_json->valuestring, sizeof(state.themes[i].id) - 1);
                strncpy(state.themes[i].name, name_json->valuestring, sizeof(state.themes[i].name) - 1);
                strncpy(state.themes[i].description, desc_json->valuestring, sizeof(state.themes[i].description) - 1);
                i++;
            }
        }
    } else {
        cJSON_Delete(root);
        return 0;
    }

    cJSON_Delete(root);
    state.do_print = true;
    return 1;
}

void down_theme(const char* theme_id, const char* theme_name) {
    CURL *curl_handle;
    CURLcode res;
    char url[256];

    if (!do_dir_exist(THEME_DOWN_PATH)) {
        snprintf(state.status_mesg, sizeof(state.status_mesg), "Download failed: Could not create directory.");
        state.view_state = VIEW_ERR;
        state.do_print = true;
        return;
    }
    
    snprintf(url, sizeof(url), THEME_DOWN_URL, theme_id);

    struct FileStruct file_data = { .stream = NULL, .friendly_name = theme_name };
    snprintf(file_data.filename, sizeof(file_data.filename), "%s%s.tmp", THEME_DOWN_PATH, theme_name);

    curl_handle = curl_easy_init();
    if (curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, url);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl");
        curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, header_callback);
        curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, &file_data);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, theme_write_callback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &file_data);
        curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1L);
        curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 60L);

        res = curl_easy_perform(curl_handle);

        if (res != CURLE_OK) {
            snprintf(state.status_mesg, sizeof(state.status_mesg), "Download failed: %s", curl_easy_strerror(res));
            state.view_state = VIEW_ERR;
            if (file_data.stream) {
                 fclose(file_data.stream);
                 file_data.stream = NULL;
                 remove(file_data.filename);
            }
        } else {
            snprintf(state.status_mesg, sizeof(state.status_mesg), "Success! Saved as %s", file_data.filename);
        }

        curl_easy_cleanup(curl_handle);
        if (file_data.stream) fclose(file_data.stream);
    }
    state.do_print = true;
}


void pr_sel_theme_inf() {
    cls_con();
    printf("--- Theme Details ---\n\n");
    Theme selected = state.themes[state.sel_ind];
    printf("  Name: %s\n", selected.name);
    printf("  ID:   %s\n", selected.id);
    printf("  Desc: %s\n\n", selected.description);
    printf("------------------------------------------\n");
    printf("  [A] Download | [B] Back\n");
    printf("------------------------------------------\n");
}

void pr_gal_view() {
    cls_con();
    printf("--- Theme Gallery (%s) ---\n\n", state.platfor[state.sel_plat_ind]);
    if (state.theme_count == 0) {
        printf("No themes found or failed to load.\n\n");
    } else {
        for (int i = 0; i < state.theme_count; i++) {
            printf("%s %s\n", (i == state.sel_ind) ? ">>" : "  ", state.themes[i].name);
        }
    }
    printf("\n-------------------------------------------------------------------------\n");
    printf("  Page %d of %d\n", state.curr_page + 1, state.tot_page);
    printf("  [Up/Down] Navigate | [Left/Right] Page | [A] Select | [B] Back\n");
    printf("-------------------------------------------------------------------------\n");
    state.more_info = "";
}

void pr_plat_men() {
    cls_con();
    printf("--- MiiShop ---\n\n");
    printf("Select a platform:\n\n");
    for (int i = 0; i < 3; i++) {
        printf("%s %s\n", (i == state.sel_plat_ind) ? ">>" : "  ", state.platfor[i]);
    }
    printf("\n-------------------------------------------------\n");
    printf("  [Up/Down] Navigate | [A] Select | [Home] Exit\n");
    printf("-------------------------------------------------\n");
    state.more_info = "";
}

void pr_stat_view(const char* message) {
    cls_con();
    printf("--- Status ---\n\n");
    if (strcmp(state.more_info, "") != 1){
        printf("  %s\n", message);
        printf("  %s\n\n", state.more_info);
    } else {
        printf("  %s\n\n", message);
    }
    printf("------------------------------------------\n");
    if (state.view_state == VIEW_ERR) {
        printf("  Press [B] to return to the main menu.\n");
    } else {
        printf("  Please wait...\n");
    }
    printf("------------------------------------------\n");
    state.more_info = "";
}

void pr_view() {
    switch(state.view_state) {
        case VIEW_PLAT_SEL: pr_plat_men(); break;
        case VIEW_GAL: pr_gal_view(); break;
        case VIEW_THEME_INF: pr_sel_theme_inf(); break;
        case VIEW_DOWN: pr_stat_view("Downloading theme..."); break;
        case VIEW_ERR: pr_stat_view(state.status_mesg); break;
    }
    state.do_print = false;
}

void thy_input() {
    u32 key = get_key_press();
    if (key == 0) return;

    if (key & WPAD_BUTTON_HOME) {
        state.runin = 0;
        return;
    }

    switch (state.view_state) {
        case VIEW_PLAT_SEL:
            if (key & WPAD_BUTTON_UP && state.sel_plat_ind > 0) state.sel_plat_ind--;
            if (key & WPAD_BUTTON_DOWN && state.sel_plat_ind < 2) state.sel_plat_ind++;
            if (key & WPAD_BUTTON_A) {
                state.view_state = VIEW_GAL;
                state.curr_page = 0;
                state.sel_ind = 0;
                get_themes(state.platfor[state.sel_plat_ind], 0);
            }
            state.do_print = true;
            break;

        case VIEW_GAL:
            if (key & WPAD_BUTTON_UP && state.sel_ind > 0) state.sel_ind--;
            if (key & WPAD_BUTTON_DOWN && state.sel_ind < state.theme_count - 1) state.sel_ind++;
            if (key & WPAD_BUTTON_LEFT && state.curr_page > 0) {
                state.curr_page--;
                state.sel_ind = 0;
                get_themes(state.platfor[state.sel_plat_ind], state.curr_page);
            }
            if (key & WPAD_BUTTON_RIGHT && state.curr_page < state.tot_page - 1) {
                state.curr_page++;
                state.sel_ind = 0;
                get_themes(state.platfor[state.sel_plat_ind], state.curr_page);
            }
            if (key & WPAD_BUTTON_A && state.theme_count > 0) state.view_state = VIEW_THEME_INF;
            if (key & WPAD_BUTTON_B) state.view_state = VIEW_PLAT_SEL;
            state.do_print = true;
            break;

        case VIEW_THEME_INF:
            if (key & WPAD_BUTTON_A) {
                state.view_state = VIEW_DOWN;
                pr_view();
                Theme selected = state.themes[state.sel_ind];
                down_theme(selected.id, selected.name);
                if (state.view_state != VIEW_ERR) {
                   state.view_state = VIEW_GAL;
                }
            }
            if (key & WPAD_BUTTON_B) state.view_state = VIEW_GAL;
            state.do_print = true;
            break;

        case VIEW_ERR:
             if (key & WPAD_BUTTON_B) {
                 state.view_state = VIEW_PLAT_SEL;
                 state.do_print = true;
             }
             break;

        default:
             break;
    }
}


void ini_state() {
    state.themes = NULL;
    state.theme_count = 0;
    state.curr_page = 0;
    state.tot_page = 1;
    state.sel_ind = 0;
    state.sel_plat_ind = 0;
    state.runin = 1;
    state.view_state = VIEW_PLAT_SEL;
    state.platfor[0] = "wii";
    state.platfor[1] = "usbloader";
    state.platfor[2] = "hbc";
    state.status_mesg[0] = '\0';
    state.do_print = true;
    state.more_info = "";
}

int main() {
    set_ter_raw();
    ini_state();
    curl_global_init(CURL_GLOBAL_ALL);
    atexit(reset_term_mode);

    while (state.runin) {
        if (state.do_print) {
            pr_view();
        }
        thy_input();
        VIDEO_WaitVSync();
    }

    free(state.themes);
    state.themes = NULL;
    cls_con();
    printf("\nExiting program...\n");
    sleep(1);
    exit(0);
    return 0;
}
