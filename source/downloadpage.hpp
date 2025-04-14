#ifndef DOWNLOADPAGE_H
#define DOWNLOADPAGE_H

#include <grrlib.h>

GRRLIB_texImg* create_text_texture(GRRLIB_ttfFont* font, const char* text, int font_size, u32 color);
void ShowDownloadPage(const char* themeName, const char* themeDesc);

#endif