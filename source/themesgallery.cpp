#include <grrlib.h>
#include <wiiuse/wpad.h>
#include <stdbool.h>
#include <stdlib.h>

#include "themesgallery.h"
#include "ui_button.h"

#include "cursor_png.h"
#include "bg_png.h"
#include "USB_png.h"
#include "WII_png.h"
#include "HBC_png.h"

void ShowThemesGallery() {
    GRRLIB_texImg *cursor = GRRLIB_LoadTexturePNG(cursor_png);
    GRRLIB_texImg *background = GRRLIB_LoadTexturePNG(bg_png);
    GRRLIB_texImg *btn_usb = GRRLIB_LoadTexturePNG(USB_png);
    GRRLIB_texImg *btn_wii = GRRLIB_LoadTexturePNG(WII_png);
    GRRLIB_texImg *btn_hbc = GRRLIB_LoadTexturePNG(HBC_png);

    GRRLIB_SetMidHandle(cursor, true);

    float scale = 1.0f;
    int button_width = 124 * scale;
    int spacing = 40;
    int total_width = (button_width * 3) + (spacing * 2);
    int startX = (640 - total_width) / 2;
    int y = (480 - (148 * scale)) / 2;

    Button buttons[3] = {
        {startX + 0 * (button_width + spacing), y, scale, 0xFFFFFFFF, btn_wii},
        {startX + 1 * (button_width + spacing), y, scale, 0xFFFFFFFF, btn_usb},
        {startX + 2 * (button_width + spacing), y, scale, 0xFFFFFFFF, btn_hbc}
    };

    while (1) {
        WPAD_ScanPads();
        ir_t ir;
        WPAD_IR(WPAD_CHAN_0, &ir);

        if (WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_B) break;

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

        GRRLIB_Render();
    }

    GRRLIB_FreeTexture(cursor);
    GRRLIB_FreeTexture(background);
    GRRLIB_FreeTexture(btn_usb);
    GRRLIB_FreeTexture(btn_wii);
    GRRLIB_FreeTexture(btn_hbc);
}

