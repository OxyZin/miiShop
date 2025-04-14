#include <grrlib.h>
#include <wiiuse/wpad.h>
#include <stdbool.h>
#include <stdlib.h>
#include "debug.hpp"

#include "themesgallery.h"
#include "ui_button.h"
#include "themelist.hpp"

#include "cursor_png.h"
#include "bg_png.h"
#include "USB_png.h"
#include "WII_png.h"
#include "HBC_png.h"

#include <asndlib.h>
#include "oggplayer.h"
#include "hover_ogg.h"
#include "click_ogg.h"

void on_wii_button_clicked() {
    show_theme_list_WII();
}

void on_usb_button_clicked() {
    show_theme_list_USB();
}

void on_hbc_button_clicked() {
    show_theme_list_HBC();
}

void ShowThemesGallery() {
    GRRLIB_texImg *cursor = GRRLIB_LoadTexturePNG(cursor_png);
    GRRLIB_texImg *background = GRRLIB_LoadTexturePNG(bg_png);
    GRRLIB_texImg *btn_usb = GRRLIB_LoadTexturePNG(USB_png);
    GRRLIB_texImg *btn_wii = GRRLIB_LoadTexturePNG(WII_png);
    GRRLIB_texImg *btn_hbc = GRRLIB_LoadTexturePNG(HBC_png);
    InitDebug();

    GRRLIB_SetMidHandle(cursor, true);
    if (WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_2) {
        ToggleDebugVisibility();
    }
    

    float scale = 1.0f;
    int button_width = 120 * scale;
    int spacing = 40;
    int total_width = (button_width * 3) + (spacing * 2);
    int startX = (640 - total_width) / 2;
    int y = (480 - (148 * scale)) / 2;

    Button buttons[3] = {
        {startX + 0 * (button_width + spacing), y, scale, 0xFFFFFFFF, btn_wii, false, false, on_wii_button_clicked},
        {startX + 1 * (button_width + spacing), y, scale, 0xFFFFFFFF, btn_usb, false, false, on_usb_button_clicked},
        {startX + 2 * (button_width + spacing), y, scale, 0xFFFFFFFF, btn_hbc, false, false, on_hbc_button_clicked}
    };

    bool was_hovering[3] = {false, false, false};
    bool was_pressed[3] = {false, false, false};

    while (1) {
        WPAD_ScanPads();
        ir_t ir;
        WPAD_IR(WPAD_CHAN_0, &ir);

        if (WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_B) break;

        GRRLIB_DrawImg(0, 0, background, 0, 1, 1, 0xFFFFFFFF);

        int pointer_x = (int)(ir.x * 1.1f) + 10;
        int pointer_y = (int)(ir.y * 1.1f) + 10;

        if (pointer_x > 640) pointer_x = 640;
        if (pointer_y > 480) pointer_y = 480;

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
            GRRLIB_DrawImg(pointer_x, pointer_y, cursor, ir.angle, 1, 1, 0xFFFFFFFF);
        }

        DrawDebugInfo();
        GRRLIB_Render();
    }

    GRRLIB_FreeTexture(cursor);
    GRRLIB_FreeTexture(background);
    GRRLIB_FreeTexture(btn_usb);
    GRRLIB_FreeTexture(btn_wii);
    GRRLIB_FreeTexture(btn_hbc);
}
