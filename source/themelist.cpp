#include <grrlib.h>
#include <wiiuse/wpad.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ogc/system.h>
#include <ogc/lwp_watchdog.h>
#include <fat.h>
#include <string.h>
#include <stdio.h>
#include "debug.hpp"

#include "themelist.hpp"
#include "cursor_png.h"
#include "bg_png.h"
#include "theme_frame_png.h"
#include "option_png.h"
#include "arial_ttf.h" // fonte TTF convertida com bin2o

// Theme names storage (dinâmico agora)
static char** theme_names = NULL;
static int theme_count = 0;
static int theme_capacity = 0;

void show_theme_list_common() {
    GRRLIB_texImg *cursor = GRRLIB_LoadTexturePNG(cursor_png);
    GRRLIB_texImg *background = GRRLIB_LoadTexturePNG(bg_png);
    GRRLIB_texImg *theme_frame = GRRLIB_LoadTexturePNG(theme_frame_png);
    GRRLIB_texImg *option = GRRLIB_LoadTexturePNG(option_png);

    GRRLIB_SetMidHandle(cursor, true);
    GRRLIB_SetMidHandle(option, true);

    GRRLIB_ttfFont *fonte = GRRLIB_LoadTTF(Arial_ttf, Arial_ttf_size);

    float scroll_y = 0;
    float target_scroll_y = 0;
    const float SCROLL_SPEED = 0.1f;
    const int OPTION_SPACING = 70;
    const float OPTION_SCALE = 0.6f;

    InitDebug();
    while (1) {
        WPAD_ScanPads();
        ir_t ir;
        WPAD_IR(WPAD_CHAN_0, &ir);

        if (WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_B) break;

        u32 btnsHeld = WPAD_ButtonsHeld(WPAD_CHAN_0);
        if (btnsHeld & WPAD_BUTTON_DOWN) {
            target_scroll_y -= 5;
        } else if (btnsHeld & WPAD_BUTTON_UP) {
            target_scroll_y += 5;
        }

        scroll_y += (target_scroll_y - scroll_y) * SCROLL_SPEED;

        GRRLIB_DrawImg(0, 0, background, 0, 1, 1, 0xFFFFFFFF);

        int theme_frame_x = (640 - theme_frame->w) / 2;
        int theme_frame_y = (480 - theme_frame->h) / 2;

        GRRLIB_DrawImg(theme_frame_x, theme_frame_y, theme_frame, 0, 1, 1, 0xFFFFFFFF);

        GRRLIB_ClipDrawing(theme_frame_x + 5, theme_frame_y + 5, theme_frame->w - 10, theme_frame->h - 10);

        // Load and parse wii.json uma vez só
        if (theme_count == 0) {
            FILE* file = fopen("sd:/apps/miiShop/database/wii.json", "r");
            if (!file) {
                theme_capacity = 1;
                theme_names = (char**)malloc(sizeof(char*) * theme_capacity);
                theme_names[0] = strdup("Default Theme");
                theme_count = 1;
            } else {
                fseek(file, 0, SEEK_END);
                long fsize = ftell(file);
                fseek(file, 0, SEEK_SET);
                char* json = (char*)malloc(fsize + 1);
                fread(json, 1, fsize, file);
                fclose(file);
                json[fsize] = 0;

                char* pos = json;
                while ((pos = strstr(pos, "\"Name\": \""))) {
                    pos += 9;
                    char* end = strchr(pos, '\"');
                    if (end) {
                        if (theme_count >= theme_capacity) {
                            theme_capacity += 10;
                            theme_names = (char**)realloc(theme_names, sizeof(char*) * theme_capacity);
                        }
                        theme_names[theme_count] = (char*)malloc(end - pos + 1);
                        strncpy(theme_names[theme_count], pos, end - pos);
                        theme_names[theme_count][end - pos] = 0;
                        theme_count++;
                        pos = end + 1;
                    }
                }
                free(json);
            }
        }

        // Scroll dinâmico com base na altura real da moldura
        float scroll_area_height = theme_frame->h - 10; // mesma usada no ClipDrawing
        float total_list_height = theme_count * OPTION_SPACING;
        float min_scroll = scroll_area_height - total_list_height;
        if (min_scroll > 0) min_scroll = 0;

        if (scroll_y > 0) scroll_y = 0;
        if (scroll_y < min_scroll) scroll_y = min_scroll;

        for (int i = 0; i < theme_count; i++) {
            float option_y = theme_frame_y + 50 + (i * OPTION_SPACING) + scroll_y;

            if (option_y < theme_frame_y || option_y > theme_frame_y + scroll_area_height) continue;

            float option_x = 640 / 2;
            u32 color = 0xFFFFFFFF;

            if (ir.valid) {
                int pointer_x = (int)(ir.x * 1.1f) + 10;
                int pointer_y = (int)(ir.y * 1.1f) + 10;

                float half_width = (option->w * OPTION_SCALE) / 2;
                float half_height = (option->h * OPTION_SCALE) / 2;

                if (pointer_x >= option_x - half_width && pointer_x <= option_x + half_width &&
                    pointer_y >= option_y - half_height && pointer_y <= option_y + half_height) {
                    color = 0x00A8FF80;
                }
            }

            GRRLIB_DrawImg(option_x, option_y, option, 0, OPTION_SCALE, OPTION_SCALE, color);

            if (i < theme_count && theme_names[i]) {
                int text_width = GRRLIB_WidthTTF(fonte, theme_names[i], 14);
                float text_x = option_x - (text_width / 2);
                float text_y = option_y - 10;
                GRRLIB_PrintfTTF(text_x, text_y, fonte, theme_names[i], 14, 0xFFFFFFFF);
            }
        }

        GRRLIB_ClipDrawing(0, 0, 640, 480);

        if (ir.valid) {
            int pointer_x = (int)(ir.x * 1.1f) + 10;
            int pointer_y = (int)(ir.y * 1.1f) + 10;
            if (pointer_x > 640) pointer_x = 640;
            if (pointer_y > 480) pointer_y = 480;

            GRRLIB_DrawImg(pointer_x, pointer_y, cursor, 0, 1, 1, 0xFFFFFFFF);
        }

        // Mostra o número de temas carregados no canto inferior esquerdo
        char theme_count_text[64];
        sprintf(theme_count_text, "Total themes: %d", theme_count);
        GRRLIB_PrintfTTF(10, 450, fonte, theme_count_text, 14, 0x000000FF);

        DrawDebugInfo();
        GRRLIB_Render();
    }

    GRRLIB_FreeTexture(cursor);
    GRRLIB_FreeTexture(background);
    GRRLIB_FreeTexture(theme_frame);
    GRRLIB_FreeTexture(option);
    GRRLIB_FreeTTF(fonte);

    for (int i = 0; i < theme_count; i++) {
        free(theme_names[i]);
    }
    free(theme_names);
    theme_names = NULL;
    theme_count = 0;
    theme_capacity = 0;
}

void show_theme_list_WII() {
    show_theme_list_common();
}

void show_theme_list_USB() {
    show_theme_list_common();
}

void show_theme_list_HBC() {
    show_theme_list_common();
}
