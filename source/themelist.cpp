#include <grrlib.h>
#include <wiiuse/wpad.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ogc/system.h>
#include <ogc/lwp_watchdog.h>
#include "debug.hpp"

#include "themelist.hpp"
#include "cursor_png.h"
#include "bg_png.h"
#include "theme_frame_png.h"
#include "option_png.h"
#include "arial_ttf.h" // fonte TTF convertida com bin2o

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
    const int VISIBLE_AREA = 400;
    const float OPTION_SCALE = 0.6f;

    const int NUM_OPTIONS = 20; // número de opções (ajuste conforme seu caso)
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

        float max_scroll = 0;
        float min_scroll = -(NUM_OPTIONS * OPTION_SPACING - VISIBLE_AREA);
        if (scroll_y > max_scroll) scroll_y = max_scroll;
        if (scroll_y < min_scroll) scroll_y = min_scroll;

        GRRLIB_DrawImg(0, 0, background, 0, 1, 1, 0xFFFFFFFF);

        int theme_frame_x = (640 - theme_frame->w) / 2;
        int theme_frame_y = (480 - theme_frame->h) / 2;

        GRRLIB_DrawImg(theme_frame_x, theme_frame_y, theme_frame, 0, 1, 1, 0xFFFFFFFF);

        GRRLIB_ClipDrawing(theme_frame_x + 5, theme_frame_y + 5, theme_frame->w - 10, theme_frame->h - 10);

        for (int i = 0; i < NUM_OPTIONS; i++) {
            float option_y = theme_frame_y + 50 + (i * OPTION_SPACING) + scroll_y;
        
            // Otimização: desenhar só se a opção estiver dentro da área visível
            if (option_y < theme_frame_y || option_y > theme_frame_y + VISIBLE_AREA) continue;
        
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
        
            // Texto "Hello World!" centralizado sobre a opção
            float text_x = option_x - 50;
            float text_y = option_y - 10;
            GRRLIB_PrintfTTF(text_x, text_y, fonte, "Hello World!", 14, 0xFFFFFFFF);
        }
        

        GRRLIB_ClipDrawing(0, 0, 640, 480);

        if (ir.valid) {
            int pointer_x = (int)(ir.x * 1.1f) + 10;
            int pointer_y = (int)(ir.y * 1.1f) + 10;
            if (pointer_x > 640) pointer_x = 640;
            if (pointer_y > 480) pointer_y = 480;

            GRRLIB_DrawImg(pointer_x, pointer_y, cursor, 0, 1, 1, 0xFFFFFFFF);
        }
        DrawDebugInfo();
        GRRLIB_Render();
    }

    GRRLIB_FreeTexture(cursor);
    GRRLIB_FreeTexture(background);
    GRRLIB_FreeTexture(theme_frame);
    GRRLIB_FreeTexture(option);
    GRRLIB_FreeTTF(fonte);
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
