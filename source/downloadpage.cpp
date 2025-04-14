#include <grrlib.h>
#include <wiiuse/wpad.h>
#include <ogc/system.h>
#include <stdlib.h>
#include <string.h>
#include "downloadpage.hpp"
#include "debug.hpp"
#include "downloadbutton_png.h"
#include "bg_png.h"
#include "theme_frame_png.h"
#include "cursor_png.h"
#include "arial_ttf.h"
#include "ui_button.h"

GRRLIB_texImg* create_text_texture(GRRLIB_ttfFont* font, const char* text, int font_size, u32 color) {
    int width = GRRLIB_WidthTTF(font, text, font_size);
    if (width <= 0) width = 1;

    GRRLIB_texImg* tex = GRRLIB_CreateEmptyTexture(width, font_size + 4);
    GRRLIB_texImg* screen_buffer = GRRLIB_CreateEmptyTexture(640, 480);
    GRRLIB_Screen2Texture(0, 0, screen_buffer, false);

    GRRLIB_FillScreen(0x00000000);
    GRRLIB_PrintfTTF(0, 0, font, text, font_size, color);
    GRRLIB_Screen2Texture(0, 0, tex, true);

    GRRLIB_FillScreen(0x00000000);
    GRRLIB_DrawImg(0, 0, screen_buffer, 0, 1, 1, 0xFFFFFFFF);
    GRRLIB_FreeTexture(screen_buffer);

    return tex;
}

void ShowDownloadPage(const char* themeName, const char* themeDesc) {
    GRRLIB_texImg* bg = GRRLIB_LoadTexturePNG(bg_png);
    if (!bg) {
        SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
        return;
    }

    GRRLIB_texImg* button_image = GRRLIB_LoadTexturePNG(downloadbutton_png);
    GRRLIB_texImg* theme_frame = GRRLIB_LoadTexturePNG(theme_frame_png);
    GRRLIB_texImg* cursor = GRRLIB_LoadTexturePNG(cursor_png);
    GRRLIB_SetMidHandle(cursor, true);

    GRRLIB_ttfFont* font = GRRLIB_LoadTTF(Arial_ttf, Arial_ttf_size);

    const char* themePhrase = "Enhance your Wii Menu!";
    const char* buttonText = "2.3MB";

    int frame_x = (640 - 512) / 2;
    int frame_y = (480 - 256) / 2;
    int center_x = 640 / 2;

    int button_x = center_x - (button_image->w / 2);
    int button_y = 320 - (button_image->h / 2);
    Button download_button = {
        .x = button_x,
        .y = button_y,
        .scale = 1.0f,
        .color = 0xFFFFFFFF,
        .image = button_image,
        .is_hovering = false,
        .is_pressed = false,
        .callback = NULL
    };

    // Limpa a tela antes de entrar no loop
    GRRLIB_FillScreen(0x00000000);
    GRRLIB_Render();
    InitDebug();

    while (1) {
        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(WPAD_CHAN_0);
        if (pressed & WPAD_BUTTON_B) break;

        ir_t ir;
        WPAD_IR(WPAD_CHAN_0, &ir);

        // Reinicia o clipping para que toda a tela seja desenhada
        GRRLIB_ClipReset();

        // Limpa a tela e desenha o BG escalado para 640x480
        GRRLIB_FillScreen(0x00000000);
        float bgScaleX = 640.0f / bg->w;
        float bgScaleY = 480.0f / bg->h;
        GRRLIB_DrawImg(0, 0, bg, 0, bgScaleX, bgScaleY, 0xFFFFFFFF);
        GRRLIB_DrawImg(frame_x, frame_y, theme_frame, 0, 1, 1, 0xFFFFFFFF);

        int name_width = GRRLIB_WidthTTF(font, themeName, 22);
        int desc_width = GRRLIB_WidthTTF(font, themeDesc, 16);
        int phrase_width = GRRLIB_WidthTTF(font, themePhrase, 18);
        int button_text_width = GRRLIB_WidthTTF(font, buttonText, 18);

        GRRLIB_PrintfTTF(center_x - name_width / 2, frame_y + 20, font, themeName, 22, 0x000000FF);
        GRRLIB_PrintfTTF(center_x - desc_width / 2, frame_y + 52, font, themeDesc, 16, 0x444444FF);
        GRRLIB_PrintfTTF(center_x - phrase_width / 2, frame_y + 100, font, themePhrase, 18, 0x0066CCFF);

        int pointer_x = 320;
        int pointer_y = 240;
        if (ir.valid) {
            pointer_x = (int)(ir.x * 1.1f) + 10;
            pointer_y = (int)(ir.y * 1.1f) + 10;
            if (pointer_x > 640) pointer_x = 640;
            if (pointer_y > 480) pointer_y = 480;
        }

        update_button(&download_button, pointer_x, pointer_y, WPAD_ButtonsHeld(WPAD_CHAN_0) & WPAD_BUTTON_A);
        draw_button(&download_button);

        GRRLIB_PrintfTTF(center_x - button_text_width / 2,
                         download_button.y + (download_button.image->h * download_button.scale) / 2 + 6,
                         font, buttonText, 18, 0xFFFFFFFF);

                         GRRLIB_DrawImg(pointer_x, pointer_y, cursor, ir.angle, 1, 1, 0xFFFFFFFF);
        DrawDebugInfo();
        GRRLIB_Render();
    }

    GRRLIB_FreeTexture(bg);
    GRRLIB_FreeTexture(button_image);
    GRRLIB_FreeTexture(theme_frame);
    GRRLIB_FreeTexture(cursor);
    GRRLIB_FreeTTF(font);
}
