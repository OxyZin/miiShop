#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include <grrlib.h>
#include <stdbool.h>

typedef struct {
    int x, y;
    float scale;
    u32 color;
    GRRLIB_texImg *image;
    bool is_hovering;
    bool is_pressed;
} Button;

#ifdef __cplusplus
extern "C" {
#endif

void update_button(Button *btn, int pointer_x, int pointer_y, bool a_held);
void draw_button(Button *btn);

#ifdef __cplusplus
}
#endif

#endif
