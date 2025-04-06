#include <grrlib.h>
#include <wiiuse/wpad.h>
#include <stdlib.h>
#include <stdbool.h>

#include "themesgallery.h"
#include "ui_button.h"

// Gráficos
#include "cursor_png.h"
#include "themebutton_png.h"
#include "downloadsbutton_png.h"
#include "settingsbutton_png.h"
#include "bg_png.h"

GRRLIB_texImg *cursor;
GRRLIB_texImg *btn_theme;
GRRLIB_texImg *btn_settings;
GRRLIB_texImg *btn_downloads;
GRRLIB_texImg *background;

int main() {
    GRRLIB_Init();
    WPAD_Init();
    WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);

    cursor = GRRLIB_LoadTexturePNG(cursor_png);
    btn_theme = GRRLIB_LoadTexturePNG(themebutton_png);
    btn_downloads = GRRLIB_LoadTexturePNG(downloadsbutton_png);
    btn_settings = GRRLIB_LoadTexturePNG(settingsbutton_png);
    background = GRRLIB_LoadTexturePNG(bg_png);

    GRRLIB_SetMidHandle(cursor, true);

    Button buttons[3] = {
        {50, 130, 1.0f, 0xFFFFFFFF, btn_theme},
        {50, 190, 1.0f, 0xFFFFFFFF, btn_downloads},
        {50, 310, 1.0f, 0xFFFFFFFF, btn_settings}
    };

    while (1) {
        WPAD_ScanPads();
        ir_t ir;
        WPAD_IR(WPAD_CHAN_0, &ir);

        if (WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_HOME) break;

        GRRLIB_DrawImg(0, 0, background, 0, 1, 1, 0xFFFFFFFF);

        int pointer_x = ir.x;
        int pointer_y = ir.y;
        bool a_held = WPAD_ButtonsHeld(WPAD_CHAN_0) & WPAD_BUTTON_A;

        for (int i = 0; i < 3; i++) {
            update_button(&buttons[i], pointer_x, pointer_y, a_held);
            draw_button(&buttons[i]);
        }

        if (ir.valid) {
            GRRLIB_DrawImg(ir.x, ir.y, cursor, 0, 1, 1, 0xFFFFFFFF);
        }

        if (buttons[0].is_pressed) {
            ShowThemesGallery();
        }

        GRRLIB_Render();
    }

    GRRLIB_FreeTexture(cursor);
    GRRLIB_FreeTexture(btn_theme);
    GRRLIB_FreeTexture(btn_settings);
    GRRLIB_FreeTexture(btn_downloads);
    GRRLIB_FreeTexture(background);
    GRRLIB_Exit();
    return 0;
}
