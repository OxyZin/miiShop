#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

// Função para esperar um certo número de quadros (frames)
void wait_frames(int frames) {
    for (int i = 0; i < frames; i++) {
        VIDEO_WaitVSync();
    }
}

// Função para fazer o Wiimote vibrar fraco
void vibrate_weak() {
    WPAD_Rumble(0, 1); // Ativa a vibração no Wiimote (1 = ativado)
    wait_frames(10); // Espera por 10 quadros
    WPAD_Rumble(0, 0); // Desativa a vibração
}

// Função para exibir o menu principal
void show_main_menu(int opcao) {
    // Limpa a tela
    printf("\x1b[2J");

    // Exibe o menu
    printf("\nWBFS Utility:\n");
    printf("Press HOME to return to loader\n");
    
    // Variáveis para armazenar as opções
    const char *official_games = "Official Games";
    const char *patched_hacks = "Patched Hacks";
    const char *riivolution_files = "Riivolution Files";
    
    const char *nomes_opcoes[] = {official_games, patched_hacks, riivolution_files};
    
    for (int i = 0; i < 3; i++) {
        if (i + 1 == opcao) {
            printf("> %d - %s\n", i + 1, nomes_opcoes[i]); // Destaca a opção atual com '>'
        } else {
            printf("  %d - %s\n", i + 1, nomes_opcoes[i]);
        }
    }
}

// Função para exibir um submenu baseado na opção escolhida
void show_submenu(int opcao, int opcao_submenu) {
    printf("\x1b[2J");
    
    // Submenus para cada opção
    const char *official_games_submenu[] = {
        "Official Game 1",
        "Official Game 2",
        "Official Game 3"
    };
    
    const char *patched_hacks_submenu[] = {
        "Patched Hack 1",
        "Patched Hack 2",
        "Patched Hack 3"
    };
    
    const char *riivolution_files_submenu[] = {
        "Riivolution File 1",
        "Riivolution File 2",
        "Riivolution File 3"
    };
    
    switch (opcao) {
        case 1: 
            printf("Official Games Submenu:\n");
            for (int i = 0; i < 3; i++) {
                if (i + 1 == opcao_submenu) {
                    printf("> %d - %s\n", i + 1, official_games_submenu[i]); // Destaca a opção atual com '>'
                } else {
                    printf("  %d - %s\n", i + 1, official_games_submenu[i]);
                }
            }
            break;
        case 2:
            printf("Patched Hacks Submenu:\n");
            for (int i = 0; i < 3; i++) {
                if (i + 1 == opcao_submenu) {
                    printf("> %d - %s\n", i + 1, patched_hacks_submenu[i]);
                } else {
                    printf("  %d - %s\n", i + 1, patched_hacks_submenu[i]);
                }
            }
            break;
        case 3:
            printf("Riivolution Files Submenu:\n");
            for (int i = 0; i < 3; i++) {
                if (i + 1 == opcao_submenu) {
                    printf("> %d - %s\n", i + 1, riivolution_files_submenu[i]);
                } else {
                    printf("  %d - %s\n", i + 1, riivolution_files_submenu[i]);
                }
            }
            break;
    }
}

int main(int argc, char **argv) {
    // Inicializa o sistema de vídeo
    VIDEO_Init();

    // Inicializa os controles (Wiimote)
    WPAD_Init();

    // Obtém o modo de vídeo preferido do sistema
    rmode = VIDEO_GetPreferredMode(NULL);

    // Aloca memória para o framebuffer na região não cacheada
    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

    // Inicializa o console para usar printf
    console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);

    // Configura o modo de vídeo
    VIDEO_Configure(rmode);

    // Define o framebuffer a ser usado
    VIDEO_SetNextFramebuffer(xfb);

    // Torna a tela visível
    VIDEO_SetBlack(false);

    // Aplica as configurações de vídeo
    VIDEO_Flush();

    // Espera pela sincronização vertical
    VIDEO_WaitVSync();
    if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();

    // Variáveis para armazenar as opções escolhidas
    int opcao = 1; // Começa no menu principal, primeira opção
    int opcao_selecionada = 0;
    int opcao_submenu = 1; // Começa no primeiro item do submenu
    int em_submenu = 0;

    // Loop principal do menu
    while (1) {
        if (em_submenu) {
            show_submenu(opcao, opcao_submenu); // Exibe o submenu
        } else {
            show_main_menu(opcao); // Exibe o menu principal
        }

        // Exibe a opção selecionada (se houver)
        if (opcao_selecionada > 0) {
            printf("\nVoce selecionou: %s\n", opcao == 1 ? "Official Games" : opcao == 2 ? "Patched Hacks" : "Riivolution Files");
            wait_frames(60); // Espera 1 segundo (aproximadamente 60 frames)
            opcao_selecionada = 0; // Reseta para permitir nova seleção
        }

        // Espera o usuário interagir com o Wiimote
        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(0); // Botões pressionados no Wiimote

        // Navegação com o d-pad no menu principal
        if (!em_submenu) {
            if (pressed & WPAD_BUTTON_UP) {
                if (opcao > 1) opcao--; // Move para cima
                vibrate_weak(); // Vibra fraco quando troca de opção
            } else if (pressed & WPAD_BUTTON_DOWN) {
                if (opcao < 3) opcao++; // Move para baixo
                vibrate_weak(); // Vibra fraco quando troca de opção
            }

            // Seleção com o botão A
            if (pressed & WPAD_BUTTON_A) {
                if (em_submenu) {
                    // Permite selecionar uma opção no submenu
                    opcao_selecionada = opcao_submenu;
                } else {
                    // Entra no submenu
                    opcao_selecionada = 0; // Reset
                    em_submenu = 1;
                }
            }
        } else {
            // Navegação com o d-pad no submenu
            if (pressed & WPAD_BUTTON_UP) {
                if (opcao_submenu > 1) opcao_submenu--; // Move para cima
                vibrate_weak(); // Vibra fraco quando troca de opção
            } else if (pressed & WPAD_BUTTON_DOWN) {
                if (opcao_submenu < 3) opcao_submenu++; // Move para baixo
                vibrate_weak(); // Vibra fraco quando troca de opção
            }

            // Seleção com o botão A (no submenu)
            if (pressed & WPAD_BUTTON_A) {
                printf("Você selecionou: %s\n", opcao_submenu == 1 ? "Games 1" : opcao_submenu == 2 ? "Games 2" : "Games 3");
                wait_frames(60); // Espera 1 segundo
            }

            // Voltar ao menu principal com o botão B
            if (pressed & WPAD_BUTTON_B) {
                em_submenu = 0; // Volta ao menu principal
                opcao_submenu = 1; // Reseta para o primeiro item do submenu
            }
        }

        // Verifica se o botão HOME foi pressionado para sair
        if (pressed & WPAD_BUTTON_HOME) {
            exit(0);
        }

        // Espera pela sincronização vertical para evitar uso excessivo da CPU
        VIDEO_WaitVSync();
    }

    return 0;
}
