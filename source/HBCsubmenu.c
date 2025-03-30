#include <stdio.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <fat.h>
#include <sdcard/wiisd_io.h>
#include <string.h>
#include <stdlib.h>
#include <network.h>

#define limparTela() printf("\x1b[2J")
#define MAX_TEMAS 20
#define VISIBLE_ITEMS 10

typedef struct {
    char *nome;
    char *descricao;
    char *url;
} Tema;

static char* lerArquivo(const char* caminho) {
    FILE *file = fopen(caminho, "r");
    if (!file) return NULL;
    
    fseek(file, 0, SEEK_END);
    long tamanho = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *conteudo = (char*)malloc(tamanho + 1);
    fread(conteudo, 1, tamanho, file);
    conteudo[tamanho] = '\0';
    
    fclose(file);
    return conteudo;
}

static s32 downloadFile(const char *url, const char *destino) {
    s32 sockfd;
    struct sockaddr_in server;
    char request[1024];
    char response[1024];
    u32 ipaddr;
    char *host, *path;
    char host_buf[256], path_buf[256];
    FILE *file;
    s32 ret;

    ret = net_init();
    if (ret < 0) {
        printf("Error initializing network: %d\n", ret);
        return -1;
    }

    if (strncmp(url, "http://", 7) != 0) {
        printf("Invalid URL: %s\n", url);
        return -1;
    }
    strcpy(host_buf, url + 7);
    host = host_buf;
    path = strchr(host_buf, '/');
    if (path) {
        *path = '\0';
        path++;
        strcpy(path_buf, path);
    } else {
        strcpy(path_buf, "");
    }

    struct hostent *he = net_gethostbyname(host);
    if (!he) {
        printf("Error resolving host: %s\n", host);
        return -1;
    }
    ipaddr = *(u32*)he->h_addr_list[0];

    sockfd = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sockfd < 0) {
        printf("Error creating socket: %d\n", sockfd);
        return -1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(80);
    server.sin_addr.s_addr = ipaddr;

    ret = net_connect(sockfd, (struct sockaddr*)&server, sizeof(server));
    if (ret < 0) {
        printf("Error connecting: %d\n", ret);
        net_close(sockfd);
        return -1;
    }

    snprintf(request, sizeof(request),
             "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
             path_buf, host);

    ret = net_send(sockfd, request, strlen(request), 0);
    if (ret < 0) {
        printf("Error sending request: %d\n", ret);
        net_close(sockfd);
        return -1;
    }

    file = fopen(destino, "wb");
    if (!file) {
        printf("Error opening destination file: %s\n", destino);
        net_close(sockfd);
        return -1;
    }

    char *content_start = NULL;
    s32 total_bytes = 0;
    while ((ret = net_recv(sockfd, response, sizeof(response) - 1, 0)) > 0) {
        response[ret] = '\0';
        if (!content_start) {
            content_start = strstr(response, "\r\n\r\n");
            if (content_start) {
                content_start += 4;
                fwrite(content_start, 1, ret - (content_start - response), file);
                total_bytes += ret - (content_start - response);
            }
        } else {
            fwrite(response, 1, ret, file);
            total_bytes += ret;
        }
    }

    fclose(file);
    net_close(sockfd);

    if (ret < 0) {
        printf("Error receiving data: %d\n", ret);
        return -1;
    }

    printf("Download completed: %d bytes saved to %s\n", total_bytes, destino);
    return 0;
}

void abrirHBCsubmenu() {
    limparTela();
    if (!fatInitDefault()) {
        printf("ERROR: fatInitDefault() failed\n");
        goto esperar;
    }
    
    const char *caminho = "sd:/apps/miiShop/themes/hbc.json";
    char *json_str = lerArquivo(caminho);
    if (!json_str) {
        printf("ERROR: Could not read hbc.json\n");
        goto esperar;
    }
    
    Tema temas[MAX_TEMAS];
    int num_temas = 0;
    
    char *ptr = json_str;
    while (num_temas < MAX_TEMAS && (ptr = strstr(ptr, "\"nome\"")) != NULL) {
        ptr = strchr(ptr, ':') + 1;
        while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n') ptr++;
        ptr++;
        char *nome_end = strchr(ptr, '"');
        if (!nome_end) break;
        int nome_len = nome_end - ptr;
        temas[num_temas].nome = (char*)malloc(nome_len + 1);
        strncpy(temas[num_temas].nome, ptr, nome_len);
        temas[num_temas].nome[nome_len] = '\0';
        
        ptr = strstr(ptr, "\"descricao\"");
        if (!ptr) break;
        ptr = strchr(ptr, ':') + 1;
        while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n') ptr++;
        ptr++;
        char *desc_end = strchr(ptr, '"');
        if (!desc_end) break;
        int desc_len = desc_end - ptr;
        temas[num_temas].descricao = (char*)malloc(desc_len + 1);
        strncpy(temas[num_temas].descricao, ptr, desc_len);
        temas[num_temas].descricao[desc_len] = '\0';
        
        ptr = strstr(ptr, "\"url_download\"");
        if (!ptr) break;
        ptr = strchr(ptr, ':') + 1;
        while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n') ptr++;
        ptr++;
        char *url_end = strchr(ptr, '"');
        if (!url_end) break;
        int url_len = url_end - ptr;
        temas[num_temas].url = (char*)malloc(url_len + 1);
        strncpy(temas[num_temas].url, ptr, url_len);
        temas[num_temas].url[url_len] = '\0';
        
        num_temas++;
        ptr = url_end;
    }
    
    if (num_temas == 0) {
        printf("ERROR: No theme found in JSON\n");
        free(json_str);
        goto esperar;
    }
    
    int opcao = 0;
    int scroll_offset = 0;
    
    while (1) {
        limparTela();
        printf("=== Homebrew Channel Themes ===\n\n");
        
        int start = scroll_offset;
        int end = start + VISIBLE_ITEMS;
        if (end > num_temas) end = num_temas;
        
        if (scroll_offset > 0) printf("  More items above\n");
        
        for (int i = start; i < end; i++) {
            printf("%s %s\n", (opcao == i) ? ">>" : "  ", temas[i].nome);
        }
        
        if (end < num_temas) printf("  More items below\n");
        
        printf("\nPress A for details, + to download, B to go back...\n");
        printf("Use UP/DOWN to scroll and select\n");
        
        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(0);
        
        if (pressed & WPAD_BUTTON_A) {
            limparTela();
            printf("Selected Theme:\n\n");
            printf("Name: %s\n", temas[opcao].nome);
            printf("Description: %s\n", temas[opcao].descricao);
            printf("Download: %s\n", temas[opcao].url);
            printf("\nPress B to go back...\n");
            
            while (1) {
                WPAD_ScanPads();
                if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B) break;
                VIDEO_WaitVSync();
            }
        } else if (pressed & WPAD_BUTTON_PLUS) {
            limparTela();
            printf("Starting download of: %s\n", temas[opcao].nome);
            
            char destino[256];
            snprintf(destino, sizeof(destino), "sd:/apps/miiShop/downloads/%s.zip", temas[opcao].nome);
            
            if (downloadFile(temas[opcao].url, destino) == 0) {
                printf("Download completed successfully!\n");
            } else {
                printf("Download failed.\n");
            }
            printf("Press B to go back...\n");
            
            while (1) {
                WPAD_ScanPads();
                if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B) break;
                VIDEO_WaitVSync();
            }
        }
        
        if (pressed & WPAD_BUTTON_B) break;
        
        if (pressed & WPAD_BUTTON_DOWN) {
            if (opcao < num_temas - 1) {
                opcao++;
                if (opcao >= scroll_offset + VISIBLE_ITEMS) {
                    scroll_offset++;
                }
            }
        }
        
        if (pressed & WPAD_BUTTON_UP) {
            if (opcao > 0) {
                opcao--;
                if (opcao < scroll_offset) {
                    scroll_offset--;
                }
            }
        }
        
        VIDEO_WaitVSync();
    }
    
    for (int i = 0; i < num_temas; i++) {
        free(temas[i].nome);
        free(temas[i].descricao);
        free(temas[i].url);
    }
    free(json_str);

esperar:
    printf("\nPress B to exit...\n");
    while (1) {
        WPAD_ScanPads();
        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B) break;
        VIDEO_WaitVSync();
    }
}