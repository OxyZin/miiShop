#include <stdio.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <fat.h>
#include <sdcard/wiisd_io.h>
#include <string.h>
#include <stdlib.h>

#define limparTela() printf("\x1b[2J")
#define MAX_TEMAS 20
#define VISIBLE_ITEMS 10  // Number of items visible at once

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

void abrirWIIsubmenu() {
    limparTela();
    if (!fatInitDefault()) {
        printf("ERROR: fatInitDefault() failed\n");
        goto esperar;
    }
    
    const char *caminho = "sd:/apps/miiShop/themes/wii.json";
    char *json_str = lerArquivo(caminho);
    if (!json_str) {
        printf("ERROR: Could not read wii.json\n");
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
    int scroll_offset = 0;  // Starting position of visible items
    
    while (1) {
        limparTela();
        printf("=== Wii Menu Themes ===\n\n");
        
        // Calculate the visible range
        int start = scroll_offset;
        int end = start + VISIBLE_ITEMS;
        if (end > num_temas) end = num_temas;
        
        // Show scroll indicators
        if (scroll_offset > 0) printf("  More items above\n");
        
        // Display visible items
        for (int i = start; i < end; i++) {
            printf("%s %s\n", (opcao == i) ? ">>" : "  ", temas[i].nome);
        }
        
        if (end < num_temas) printf("  More items below\n");
        
        printf("\nPress A for details, B to go back...\n");
        printf("Use UP/DOWN to scroll and select\n");
        
        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(0);
        
        if (pressed & WPAD_BUTTON_A) {
            limparTela();
            printf("Theme Details:\n\n");
            printf("Name: %s\n", temas[opcao].nome);
            printf("Description: %s\n", temas[opcao].descricao);
            printf("Download: %s\n", temas[opcao].url);
            printf("\nPress B to go back...\n");
            
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