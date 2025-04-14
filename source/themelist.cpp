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
#include "downloadpage.hpp" // Inclui a declaração de ShowDownloadPage
#include "themelist.hpp"
#include "cursor_png.h"
#include "bg_png.h"
#include "theme_frame_png.h"
#include "option_png.h"
#include "arial_ttf.h"
#include <asndlib.h>
#include "oggplayer.h"
#include "hover_ogg.h"
#include "click_ogg.h"

typedef struct {
    char* name;
    char* desc;
    GRRLIB_texImg* name_tex;
    GRRLIB_texImg* desc_tex;
} ThemeEntry;

static ThemeEntry* themes = NULL;
static int theme_count = 0;
static int theme_capacity = 0;

bool debug_visible = true;

void show_theme_list_common(const char* json_path) {
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

    // Carregamento dos temas
    if (theme_count == 0) {
        FILE* file = fopen(json_path, "r");
        if (!file) {
            theme_capacity = 1;
            themes = (ThemeEntry*)malloc(sizeof(ThemeEntry) * theme_capacity);
            themes[0].name = strdup("Default Theme");
            themes[0].desc = strdup("System default");
            themes[0].name_tex = create_text_texture(fonte, themes[0].name, 14, 0x000000FF);
            themes[0].desc_tex = create_text_texture(fonte, themes[0].desc, 12, 0x444444FF);
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
                char* end_name = strchr(pos, '\"');
                if (!end_name) break;

                int name_len = end_name - pos;
                char* name = (char*)malloc(name_len + 1);
                strncpy(name, pos, name_len);
                name[name_len] = '\0';

                char* desc_pos = strstr(end_name, "\"Desc\": \"");
                if (!desc_pos) break;
                desc_pos += 9;
                char* end_desc = strchr(desc_pos, '\"');
                if (!end_desc) break;

                int desc_len = end_desc - desc_pos;
                char* desc = (char*)malloc(desc_len + 1);
                strncpy(desc, desc_pos, desc_len);
                desc[desc_len] = '\0';

                if (theme_count >= theme_capacity) {
                    theme_capacity += 10;
                    themes = (ThemeEntry*)realloc(themes, sizeof(ThemeEntry) * theme_capacity);
                }

                themes[theme_count].name = name;
                themes[theme_count].desc = desc;
                themes[theme_count].name_tex = create_text_texture(fonte, name, 14, 0x000000FF);
                themes[theme_count].desc_tex = create_text_texture(fonte, desc, 12, 0x444444FF);
                theme_count++;
                pos = end_desc + 1;
            }
            free(json);
        }
    }

    int frame_x = (640 - 512) / 2;
    int frame_y = (480 - 256) / 2;
    int visible_area = 256 - 3;

    // Arrays para rastrear estado de hover e clique
    bool* was_hovering = (bool*)calloc(theme_count, sizeof(bool));
    bool* was_pressed = (bool*)calloc(theme_count, sizeof(bool));

    while (1) {
        WPAD_ScanPads();
        ir_t ir;
        WPAD_IR(WPAD_CHAN_0, &ir);

        if (WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_B) break;
        if (WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_2) debug_visible = !debug_visible;

        u32 btnsHeld = WPAD_ButtonsHeld(WPAD_CHAN_0);
        if (btnsHeld & WPAD_BUTTON_DOWN) target_scroll_y -= 5;
        else if (btnsHeld & WPAD_BUTTON_UP) target_scroll_y += 5;

        scroll_y += (target_scroll_y - scroll_y) * SCROLL_SPEED;

        float total_list_height = theme_count * OPTION_SPACING;
        float min_scroll = visible_area - total_list_height;
        if (min_scroll > 0) min_scroll = 0;
        if (scroll_y > 0) scroll_y = 0;
        if (scroll_y < min_scroll) scroll_y = min_scroll;

        GRRLIB_DrawImg(0, 0, background, 0, 1, 1, 0xFFFFFFFF);
        GRRLIB_DrawImg(frame_x, frame_y, theme_frame, 0, 1, 1, 0xFFFFFFFF);

        GRRLIB_ClipDrawing(frame_x, frame_y, 512, visible_area);

        int first_visible = (-scroll_y) / OPTION_SPACING;
        int last_visible = ((-scroll_y + visible_area) / OPTION_SPACING) + 1;
        if (first_visible < 0) first_visible = 0;
        if (last_visible >= theme_count) last_visible = theme_count - 1;

        bool a_held = WPAD_ButtonsHeld(WPAD_CHAN_0) & WPAD_BUTTON_A;

        for (int i = first_visible; i <= last_visible; i++) {
            float option_y = frame_y + 35 + (i * OPTION_SPACING) + scroll_y;
            ThemeEntry* theme = &themes[i];
            float option_x = 640 / 2;
            u32 color = 0xFFFFFFFF;

            int pointer_x = (int)(ir.x * 1.1f) + 10;
            int pointer_y = (int)(ir.y * 1.1f) + 10;
            float half_width = (option->w * OPTION_SCALE) / 2;
            float half_height = (option->h * OPTION_SCALE) / 2;

            bool is_hovering = (ir.valid &&
                               pointer_x >= option_x - half_width && pointer_x <= option_x + half_width &&
                               pointer_y >= option_y - half_height && pointer_y <= option_y + half_height);

            bool is_pressed = is_hovering && a_held;

            if (is_hovering && !was_hovering[i]) {
                StopOgg();
                PlayOgg((u8*)hover_ogg, hover_ogg_size, 0, OGG_ONE_TIME);
                color = 0xFEFEFEFF;
            }

            if (is_pressed && !was_pressed[i]) {
                StopOgg();
                PlayOgg((u8*)click_ogg, click_ogg_size, 0, OGG_ONE_TIME);
                // Chamar ShowDownloadPage com o tema selecionado
                ShowDownloadPage(theme->name, theme->desc);
            }

            GRRLIB_DrawImg(option_x, option_y, option, 0, OPTION_SCALE, OPTION_SCALE, color);

            if (theme->name_tex) {
                float name_x = option_x - (theme->name_tex->w / 2);
                float name_y = option_y - 20;
                GRRLIB_DrawImg(name_x, name_y, theme->name_tex, 0, 1, 1, 0xFFFFFFFF);
            }

            if (theme->desc_tex) {
                float desc_x = option_x - (theme->desc_tex->w / 2);
                float desc_y = option_y + 5;
                GRRLIB_DrawImg(desc_x, desc_y, theme->desc_tex, 0, 1, 1, 0xFFFFFFFF);
            }

            was_hovering[i] = is_hovering;
            was_pressed[i] = is_pressed;
        }

        GRRLIB_ClipDrawing(0, 0, 640, 480);

        if (ir.valid) {
            int pointer_x = (int)(ir.x * 1.1f) + 10;
            int pointer_y = (int)(ir.y * 1.1f) + 10;
            if (pointer_x > 640) pointer_x = 640;
            if (pointer_y > 480) pointer_y = 480;
            GRRLIB_DrawImg(pointer_x, pointer_y, cursor, ir.angle, 1, 1, 0xFFFFFFFF);
        }

        if (debug_visible) DrawDebugInfo();

        GRRLIB_Render();
    }

    // Liberação de recursos
    GRRLIB_FreeTexture(cursor);
    GRRLIB_FreeTexture(background);
    GRRLIB_FreeTexture(theme_frame);
    GRRLIB_FreeTexture(option);
    GRRLIB_FreeTTF(fonte);

    for (int i = 0; i < theme_count; i++) {
        free(themes[i].name);
        free(themes[i].desc);
        GRRLIB_FreeTexture(themes[i].name_tex);
        GRRLIB_FreeTexture(themes[i].desc_tex);
    }
    free(themes);
    free(was_hovering);
    free(was_pressed);
    themes = NULL;
    theme_count = 0;
    theme_capacity = 0;
}

void show_theme_list_WII() {
    show_theme_list_common("sd:/apps/miiShop/database/wii.json");
}

void show_theme_list_USB() {
    show_theme_list_common("sd:/apps/miiShop/database/usb.json");
}

void show_theme_list_HBC() {
    show_theme_list_common("sd:/apps/miiShop/database/hbc.json");
}