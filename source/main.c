#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <fat.h>
#include <dirent.h>
#include "WIIsubmenu.h"
#include "USBsubmenu.h"
#include "HBCsubmenu.h"

static void *framebuffer;
static GXRModeObj *rmode;

void initializeVideo() {
    VIDEO_Init();
    rmode = VIDEO_GetPreferredMode(NULL);
    framebuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    console_init(framebuffer, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);
    
    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(framebuffer);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    VIDEO_WaitVSync();
}

void clearScreen() {
    printf("\x1b[2J");
}

void showMenu(int option) {
    clearScreen();
    
    printf("====================================\n");
    printf("              miiShop              \n");
    printf("====================================\n\n");

    printf(" Choose an option:\n\n");

    printf(" %s [1] Wii Menu Themes\n", (option == 0) ? ">>" : "   ");
    printf(" %s [2] USB Loader GX Themes\n", (option == 1) ? ">>" : "   ");
    printf(" %s [3] Homebrew Channel Themes\n", (option == 2) ? ">>" : "   ");
    
    printf("\n------------------------------------\n\n");
    printf(" %s [4] About\n", (option == 3) ? ">>" : "   ");
    printf(" %s [5] Exit\n", (option == 4) ? ">>" : "   ");
    printf("\n------------------------------------\n");
    printf(" UP / DOWN to navigate  |  A to select\n");
    printf(" HOME to exit\n");
    printf("------------------------------------\n");
    printf(" Made by @oxyzin\n");
}

void showAbout() {
    clearScreen();
    printf("====================================\n");
    printf("            About miiShop          \n");
    printf("====================================\n\n");

    printf("miiShop is a project developed for the Wii,\n");
    printf("allowing you to download themes directly to the console.\n");
    printf("This project is open source, created by @oxyzin.\n");
    printf("\nPress B to return to the main menu...\n");

    while (1) {
        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(0);

        if (pressed & WPAD_BUTTON_B) {
            return;
        }
        
        VIDEO_WaitVSync();
    }
}

void checkSettings() {
    FILE* fp = fopen("sd:/apps/miiShop/settings.json", "r");
    if (fp) {
        printf("Settings file found: sd:/apps/miiShop/settings.json\n");
        fclose(fp);
    } else {
        printf("Settings file not found.\n");
    }
}

void listThemes(const char* folder, const char* jsonFile) {
    clearScreen();
    printf("====================================\n");
    printf("         Available Themes          \n");
    printf("====================================\n\n");

    DIR* dir = opendir(folder);
    if (!dir) {
        printf("Error: Could not open folder %s!\n", folder);
        printf("Please check if the SD card is inserted.\n");
        printf("\nPress B to go back...");
        while (1) {
            WPAD_ScanPads();
            if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B) return;
            VIDEO_WaitVSync();
        }
    }

    char jsonPath[256];
    snprintf(jsonPath, sizeof(jsonPath), "%s/%s", folder, jsonFile);
    FILE* fp = fopen(jsonPath, "r");
    if (fp) {
        printf("Themes file found: %s\n", jsonPath);
        fclose(fp);
    } else {
        printf("Themes file not found: %s\n", jsonPath);
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char fullPath[256];
            snprintf(fullPath, sizeof(fullPath), "%s/%s", folder, entry->d_name);
            int attributes = FAT_getAttr(fullPath);
            printf("- %s %s\n", entry->d_name, (attributes & ATTR_DIRECTORY) ? "[DIR]" : "");
        }
    }
    closedir(dir);

    printf("\nPress B to go back...");
    while (1) {
        WPAD_ScanPads();
        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B) return;
        VIDEO_WaitVSync();
    }
}

int main() {
    initializeVideo();
    WPAD_Init();

    if (!fatInitDefault()) {
        clearScreen();
        printf("Error: Could not initialize file system!\n");
        printf("Insert an SD card and restart the program.\n");
        while (1) VIDEO_WaitVSync();
    }

    clearScreen();
    checkSettings();
    printf("Press any button to continue...\n");
    while (1) {
        WPAD_ScanPads();
        if (WPAD_ButtonsDown(0)) break;
        VIDEO_WaitVSync();
    }

    int option = 0;
    showMenu(option);

    while (1) {
        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(0);

        if (pressed & WPAD_BUTTON_DOWN) {
            option = (option + 1) % 5;
            showMenu(option);
        }
        if (pressed & WPAD_BUTTON_UP) {
            option = (option - 1 + 5) % 5;
            showMenu(option);
        }
        if (pressed & WPAD_BUTTON_A) {
            if (option == 0) {
                abrirWIIsubmenu();
            }
            else if (option == 1) {
                abrirUSBsubmenu();
            }
            else if (option == 2) {
                abrirHBCsubmenu();
            }
            else if (option == 3) {
                showAbout();
            }
            else if (option == 4) {
                clearScreen();
                printf("Exiting miiShop...\n");
                fatUnmount("sd:");
                exit(0);
            }
            showMenu(option);
        }
        if (pressed & WPAD_BUTTON_HOME) {
            clearScreen();
            printf("Exiting miiShop...\n");
            fatUnmount("sd:");
            exit(0);
        }
        VIDEO_WaitVSync();
    }
    return 0;
}