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

void iniciarVideo() {
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

void limparTela() {
    printf("\x1b[2J");
}

void mostrarMenu(int opcao) {
    limparTela();
    
    printf("====================================\n");
    printf("              miiShop              \n");
    printf("====================================\n\n");

    printf(" Escolha uma opção:\n\n");

    printf(" %s [1] Temas do Wii Menu\n", (opcao == 0) ? ">>" : "   ");
    printf(" %s [2] Temas do USB Loader GX\n", (opcao == 1) ? ">>" : "   ");
    printf(" %s [3] Temas do Homebrew Channel\n", (opcao == 2) ? ">>" : "   ");
    
    printf("\n------------------------------------\n\n");
    printf(" %s [4] Sobre\n", (opcao == 3) ? ">>" : "   ");
    printf(" %s [5] Sair\n", (opcao == 4) ? ">>" : "   ");
    printf("\n------------------------------------\n");
    printf(" CIMA / BAIXO para navegar  |  A para selecionar\n");
    printf(" HOME para sair\n");
    printf("------------------------------------\n");
    printf(" Feito por @oxyzin\n");
}

void mostrarSobre() {
    limparTela();
    printf("====================================\n");
    printf("            Sobre o miiShop         \n");
    printf("====================================\n\n");

    printf("miiShop é um projeto desenvolvido para o Wii,\n");
    printf("permitindo baixar temas diretamente no console.\n");
    printf("Este projeto é open source, criado por @oxyzin.\n");
    printf("\nPressione B para voltar ao menu principal...\n");

    while (1) {
        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(0);

        if (pressed & WPAD_BUTTON_B) {
            return;
        }
        
        VIDEO_WaitVSync();
    }
}

void verificarConfiguracoes() {
    FILE* fp = fopen("sd:/apps/miiShop/settings.txt", "r");
    if (fp) {
        printf("Arquivo de configurações encontrado: sd:/apps/miiShop/settings.txt\n");
        fclose(fp);
        // Aqui você pode adicionar leitura do conteúdo quando estruturar o arquivo
    } else {
        printf("Arquivo de configurações não encontrado.\n");
    }
}

void listarTemas(const char* pasta, const char* arquivoJson) {
    limparTela();
    printf("====================================\n");
    printf("         Temas Disponíveis         \n");
    printf("====================================\n\n");

    DIR* dir = opendir(pasta);
    if (!dir) {
        printf("Erro: Não foi possível abrir a pasta %s!\n", pasta);
        printf("Verifique se o cartão SD está inserido.\n");
        printf("\nPressione B para voltar...");
        while (1) {
            WPAD_ScanPads();
            if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B) return;
            VIDEO_WaitVSync();
        }
    }

    char caminhoJson[256];
    snprintf(caminhoJson, sizeof(caminhoJson), "%s/%s", pasta, arquivoJson);
    FILE* fp = fopen(caminhoJson, "r");
    if (fp) {
        printf("Arquivo de temas encontrado: %s\n", caminhoJson);
        fclose(fp);
        // Aqui você pode adicionar leitura do JSON quando estruturar o arquivo
    } else {
        printf("Arquivo de temas não encontrado: %s\n", caminhoJson);
    }

    struct dirent* entrada;
    while ((entrada = readdir(dir)) != NULL) {
        if (entrada->d_type == DT_REG) {
            char caminhoCompleto[256];
            snprintf(caminhoCompleto, sizeof(caminhoCompleto), "%s/%s", pasta, entrada->d_name);
            int atributos = FAT_getAttr(caminhoCompleto);
            printf("- %s %s\n", entrada->d_name, (atributos & ATTR_DIRECTORY) ? "[DIR]" : "");
        }
    }
    closedir(dir);

    printf("\nPressione B para voltar...");
    while (1) {
        WPAD_ScanPads();
        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B) return;
        VIDEO_WaitVSync();
    }
}

int main() {
    iniciarVideo();
    WPAD_Init();

    if (!fatInitDefault()) {
        limparTela();
        printf("Erro: Não foi possível inicializar o sistema de arquivos!\n");
        printf("Insira um cartão SD e reinicie o programa.\n");
        while (1) VIDEO_WaitVSync();
    }

    // Verifica o arquivo de configurações ao iniciar
    limparTela();
    verificarConfiguracoes();
    printf("Pressione qualquer botão para continuar...\n");
    while (1) {
        WPAD_ScanPads();
        if (WPAD_ButtonsDown(0)) break;
        VIDEO_WaitVSync();
    }

    int opcao = 0;
    mostrarMenu(opcao);

    while (1) {
        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(0);

        if (pressed & WPAD_BUTTON_DOWN) {
            opcao = (opcao + 1) % 5;
            mostrarMenu(opcao);
        }
        if (pressed & WPAD_BUTTON_UP) {
            opcao = (opcao - 1 + 5) % 5;
            mostrarMenu(opcao);
        }
        if (pressed & WPAD_BUTTON_A) {
            if (opcao == 0) {
                abrirWIIsubmenu(); // Chama a função do WIIsubmenu.c
            }
            else if (opcao == 1) {
                abrirUSBsubmenu(); // Chama a função do USBsubmenu.c
            }
            else if (opcao == 2) {
                abrirHBCsubmenu(); // Chama a função do HBCsubmenu.c
            }
            else if (opcao == 3) {
                mostrarSobre();
            }
            else if (opcao == 4) {
                limparTela();
                printf("Saindo do miiShop...\n");
                fatUnmount("sd:");
                exit(0);
            }
            mostrarMenu(opcao); // Volta ao menu principal após sair do submenu
        }
        if (pressed & WPAD_BUTTON_HOME) {
            limparTela();
            printf("Saindo do miiShop...\n");
            fatUnmount("sd:");
            exit(0);
        }
        VIDEO_WaitVSync();
    }
    return 0;
}