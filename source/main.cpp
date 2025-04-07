#include <grrlib.h>
#include <wiiuse/wpad.h>
#include <stdlib.h>
#include <stdbool.h>

#include <asndlib.h>
#include "oggplayer.h"

#include "debug.hpp"
#include "themesgallery.h"
#include "ui_button.h"

// Gráficos
#include "cursor_png.h"
#include "themebutton_png.h"
#include "downloadsbutton_png.h"
#include "settingsbutton_png.h"
#include "bg_png.h"

// Áudio OGG
#include "hover_ogg.h"
#include "click_ogg.h"

GRRLIB_texImg *cursor;
GRRLIB_texImg *btn_theme;
GRRLIB_texImg *btn_settings;
GRRLIB_texImg *btn_downloads;
GRRLIB_texImg *background;

int main() {
    GRRLIB_Init();
    WPAD_Init();
    WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
    ASND_Init();
    InitDebug();

    cursor = GRRLIB_LoadTexturePNG(cursor_png);
    btn_theme = GRRLIB_LoadTexturePNG(themebutton_png);
    btn_downloads = GRRLIB_LoadTexturePNG(downloadsbutton_png);
    btn_settings = GRRLIB_LoadTexturePNG(settingsbutton_png);
    background = GRRLIB_LoadTexturePNG(bg_png);

    GRRLIB_SetMidHandle(cursor, true);

    Button buttons[3] = {
        {50, 130, 1.0f, 0xFFFFFFFF, btn_theme, false, false, ShowThemesGallery},
        {50, 190, 1.0f, 0xFFFFFFFF, btn_downloads, false, false, NULL},
        {50, 310, 1.0f, 0xFFFFFFFF, btn_settings, false, false, NULL}
    };

    bool was_hovering[3] = {false, false, false};
    bool was_pressed[3] = {false, false, false};

    while (1) {
        WPAD_ScanPads();
        ir_t ir;
        WPAD_IR(WPAD_CHAN_0, &ir);

        if (WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_HOME) break;

        GRRLIB_DrawImg(0, 0, background, 0, 1, 1, 0xFFFFFFFF);

        int pointer_x = ir.x;
        int pointer_y = ir.y;

        if (ir.valid) {
            pointer_x = (int)(ir.x * 1.1f) + 10;
            pointer_y = (int)(ir.y * 1.1f) + 10;

            if (pointer_x > 640) pointer_x = 640;
            if (pointer_y > 480) pointer_y = 480;
        }

        bool a_held = WPAD_ButtonsHeld(WPAD_CHAN_0) & WPAD_BUTTON_A;

        for (int i = 0; i < 3; i++) {
            update_button(&buttons[i], pointer_x, pointer_y, a_held);
            draw_button(&buttons[i]);

            bool is_hovering = (pointer_x >= buttons[i].x &&
                                pointer_x <= buttons[i].x + buttons[i].image->w * buttons[i].scale &&
                                pointer_y >= buttons[i].y &&
                                pointer_y <= buttons[i].y + buttons[i].image->h * buttons[i].scale);

            if (ir.valid && is_hovering && !was_hovering[i]) {
                StopOgg();
                PlayOgg((u8*)hover_ogg, hover_ogg_size, 0, OGG_ONE_TIME);
            }

            if (buttons[i].is_pressed && !was_pressed[i]) {
                StopOgg();
                PlayOgg((u8*)click_ogg, click_ogg_size, 0, OGG_ONE_TIME);
                if (buttons[i].callback != NULL) {
                    buttons[i].callback();
                }
            }

            was_hovering[i] = is_hovering;
            was_pressed[i] = buttons[i].is_pressed;
        }

        if (ir.valid) {
            GRRLIB_DrawImg(pointer_x, pointer_y, cursor, 0, 1, 1, 0xFFFFFFFF);
        }

        DrawDebugInfo();
        GRRLIB_Render();
    }

    GRRLIB_FreeTexture(cursor);
    GRRLIB_FreeTexture(btn_theme);
    GRRLIB_FreeTexture(btn_settings);
    GRRLIB_FreeTexture(btn_downloads);
    GRRLIB_FreeTexture(background);

    StopOgg();
    ASND_End();
    GRRLIB_Exit();
    return 0;
}
