#include <stdio.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <fat.h>
#include <sdcard/wiisd_io.h>
#include <string.h>
#include <stdlib.h>

#define limparTela() printf("\x1b[2J")
#define MAX_TEMAS 10

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

void abrirUSBsubmenu() {
    limparTela();
    if (!fatInitDefault()) {
        printf("ERRO: fatInitDefault() falhou\n");
        goto esperar;
    }
    
    const char *caminho = "sd:/apps/miiShop/themes/usb.json";
    char *json_str = lerArquivo(caminho);
    if (!json_str) {
        printf("ERRO: Não foi possível ler usb.json\n");
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
        printf("ERRO: Nenhum tema encontrado no JSON\n");
        free(json_str);
        goto esperar;
    }
    
    int opcao = 0;
    while (1) {
        limparTela();
        printf("=== USB Loader GX Themes ===\n\n");
        
        for (int i = 0; i < num_temas; i++) {
            printf("%s %s\n", (opcao == i) ? ">>" : "  ", temas[i].nome);
        }
        
        printf("\nPressione A para selecionar, B para voltar...\n");
        
        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(0);
        
        if (pressed & WPAD_BUTTON_A) {
            limparTela();
            printf("Tema Selecionado:\n\n");
            printf("Nome: %s\n", temas[opcao].nome);
            printf("Descrição: %s\n", temas[opcao].descricao);
            printf("Download: %s\n", temas[opcao].url);
            printf("\nPressione B para voltar...\n");
            
            while (1) {
                WPAD_ScanPads();
                if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B) break;
                VIDEO_WaitVSync();
            }
        }
        
        if (pressed & WPAD_BUTTON_B) break;
        if (pressed & WPAD_BUTTON_DOWN) opcao = (opcao + 1) % num_temas;
        if (pressed & WPAD_BUTTON_UP) opcao = (opcao - 1 + num_temas) % num_temas;
        
        VIDEO_WaitVSync();
    }
    
    for (int i = 0; i < num_temas; i++) {
        free(temas[i].nome);
        free(temas[i].descricao);
        free(temas[i].url);
    }
    free(json_str);

esperar:
    printf("\nPressione B para sair...\n");
    while (1) {
        WPAD_ScanPads();
        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B) break;
        VIDEO_WaitVSync();
    }
}