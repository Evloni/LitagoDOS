#ifndef BOX_DRAWING_H
#define BOX_DRAWING_H

#include <stdint.h>

// Draw a box at the specified position with the given dimensions
void draw_box(int x, int y, int width, int height, uint32_t color);

// Draw a box with text centered inside it
void draw_box_with_text(int x, int y, int width, int height, const char* text, uint32_t box_color, uint32_t text_color);

#endif // BOX_DRAWING_H 