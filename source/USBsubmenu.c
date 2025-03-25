#include <stdio.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include "dados.h"

extern void limparTela();

void abrirUSBsubmenu() { 
    int opcao = 0;

    while (1) {
        limparTela();
        printf("=== USB Loader GX Themes ===\n\n");

        for (int i = 0; i < NUM_USB_LOADER; i++) {
            printf("%s %s\n", (opcao == i) ? ">>" : "  ", USBLoaderGX[i].nome);
        }

        printf("\nPress A to select, B to return...\n");

        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(0);

        if (pressed & WPAD_BUTTON_A) {
            limparTela();
            printf("Selected Theme:\n");
            printf("Name: %s\n", USBLoaderGX[opcao].nome);
            printf("Description: %s\n", USBLoaderGX[opcao].descricao);
            printf("Download: %s\n", USBLoaderGX[opcao].url_download);
            printf("\nPress B to return...\n");

            while (1) {
                WPAD_ScanPads();
                if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B) break;
                VIDEO_WaitVSync();
            }
        }

        if (pressed & WPAD_BUTTON_B) break;
        if (pressed & WPAD_BUTTON_DOWN) opcao = (opcao + 1) % NUM_USB_LOADER;
        if (pressed & WPAD_BUTTON_UP) opcao = (opcao - 1 + NUM_USB_LOADER) % NUM_USB_LOADER;

        VIDEO_WaitVSync();
    }
}
