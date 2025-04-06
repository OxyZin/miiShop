#include "ui_button.h"

#define LERP(a, b, t) ((a) + ((b) - (a)) * (t))

void update_button(Button *btn, int pointer_x, int pointer_y, bool a_held) {
    float width = btn->image->w * btn->scale;
    float height = btn->image->h * btn->scale;

    btn->is_hovering = false;
    btn->is_pressed = false;

    if (pointer_x >= btn->x && pointer_x <= btn->x + width &&
        pointer_y >= btn->y && pointer_y <= btn->y + height) {
        btn->is_hovering = true;
        if (a_held) {
            btn->is_pressed = true;
        }
    }

    float target_scale;
    u32 target_color;

    if (btn->is_pressed) {
        target_scale = 0.9f;
        target_color = 0xAAAAAAFF;
    } else if (btn->is_hovering) {
        target_scale = 1.1f;
        target_color = 0xFFFFFFFF;
    } else {
        target_scale = 1.0f;
        target_color = 0xFFFFFFFF;
    }

    btn->scale = LERP(btn->scale, target_scale, 0.2f);

    u8 r = (btn->color >> 24) & 0xFF;
    u8 g = (btn->color >> 16) & 0xFF;
    u8 b = (btn->color >> 8) & 0xFF;
    u8 a = btn->color & 0xFF;

    u8 tr = (target_color >> 24) & 0xFF;
    u8 tg = (target_color >> 16) & 0xFF;
    u8 tb = (target_color >> 8) & 0xFF;

    r = LERP(r, tr, 0.2f);
    g = LERP(g, tg, 0.2f);
    b = LERP(b, tb, 0.2f);
    btn->color = (r << 24) | (g << 16) | (b << 8) | a;
}

void draw_button(Button *btn) {
    float draw_x = btn->x - (btn->image->w * btn->scale - btn->image->w) / 2;
    float draw_y = btn->y - (btn->image->h * btn->scale - btn->image->h) / 2;

    GRRLIB_DrawImg(draw_x, draw_y, btn->image, 0, btn->scale, btn->scale, btn->color);
}
