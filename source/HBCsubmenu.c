#include <stdio.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include "dados.h"

extern void limparTela();

void abrirHBCsubmenu() { 
    int opcao = 0;

    while (1) {
        limparTela();
        printf("=== Homebrew Channel Themes ===\n\n");

        for (int i = 0; i < NUM_HBC; i++) {
            printf("%s %s\n", (opcao == i) ? ">>" : "  ", HBC[i].nome);
        }

        printf("\nPress A to select, B to return....\n");

        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(0);

        if (pressed & WPAD_BUTTON_A) {
            limparTela();
            printf("Selected Theme:\n");
            printf("Name: %s\n", HBC[opcao].nome);
            printf("Description: %s\n", HBC[opcao].descricao);
            printf("Download: %s\n", HBC[opcao].url_download);
            printf("\nPress B to return...\n");

            while (1) {
                WPAD_ScanPads();
                if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B) break;
                VIDEO_WaitVSync();
            }
        }

        if (pressed & WPAD_BUTTON_B) break;
        if (pressed & WPAD_BUTTON_DOWN) opcao = (opcao + 1) % NUM_HBC;
        if (pressed & WPAD_BUTTON_UP) opcao = (opcao - 1 + NUM_HBC) % NUM_HBC;

        VIDEO_WaitVSync();
    }
}
