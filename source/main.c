#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
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

    printf(" Choose an option:\n\n");

    printf(" %s [1] Wii Menu Themes\n", (opcao == 0) ? ">>" : "   ");
    printf(" %s [2] USB Loader GX Themes\n", (opcao == 1) ? ">>" : "   ");
    printf(" %s [3] Homebrew Channel Themes\n", (opcao == 2) ? ">>" : "   ");
    
    printf("\n------------------------------------\n\n");
    printf(" %s [4] About\n", (opcao == 3) ? ">>" : "   ");
    printf(" %s [5] Exit\n", (opcao == 4) ? ">>" : "   ");
    printf("\n------------------------------------\n");
    printf(" UP / DOWN for navigation  |  A to select\n");
    printf(" HOME to exit\n");
    printf("------------------------------------\n");
    printf(" Made by @oxyzin\n");
}

void mostrarSobre() {
    limparTela();
    printf("====================================\n");
    printf("            About miiShop         \n");
    printf("====================================\n\n");

    printf("miiShop is a project developed for the Wii console, \n");
    printf("allowing you to download themes directly to the console. \n");
    printf("This project is open source, developed by @oxyzin.\n");
    printf("\nPress B to return to the main menu...\n");

    while (1) {
        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(0);

        if (pressed & WPAD_BUTTON_B) {
            return; // Voltar ao menu principal
        }
        
        VIDEO_WaitVSync();
    }
}

int main() {
    iniciarVideo();
    WPAD_Init();

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
            if (opcao == 0) abrirWIIsubmenu();
            else if (opcao == 1) abrirUSBsubmenu();
            else if (opcao == 2) abrirHBCsubmenu();
            else if (opcao == 3) mostrarSobre();
            else if (opcao == 4) {
                limparTela();
                printf("Exiting miiShop...\n");
                exit(0);
            }
            mostrarMenu(opcao); // Volta ao menu principal
        }
        if (pressed & WPAD_BUTTON_HOME) {
            limparTela();
            printf("Exiting miiShop...\n");
            exit(0);
        }
        VIDEO_WaitVSync();
    }
    return 0;
}
