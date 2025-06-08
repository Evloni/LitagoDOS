#include "../../../include/drivers/vbe.h"
#include "../../../include/font_8x16.h"
#include "../../../include/drivers/font_loader.h"

// Box drawing characters
#define BOX_TOP_LEFT     0xC2
#define BOX_TOP_RIGHT    0xC3
#define BOX_BOTTOM_LEFT  0xC4
#define BOX_BOTTOM_RIGHT 0xC5
#define BOX_HORIZONTAL   0xC0
#define BOX_VERTICAL     0xC1

// Draw a box at the specified position with the given dimensions
void draw_box(int x, int y, int width, int height, uint32_t color) {
    // Draw top border
    vbe_draw_char_font_loader(x * 8, y * 16, BOX_TOP_LEFT, color);
    for (int i = 1; i < width - 1; i++) {
        vbe_draw_char_font_loader((x + i) * 8, y * 16, BOX_HORIZONTAL, color);
    }
    vbe_draw_char_font_loader((x + width - 1) * 8, y * 16, BOX_TOP_RIGHT, color);

    // Draw sides
    for (int i = 1; i < height - 1; i++) {
        // Left side
        vbe_draw_char_font_loader(x * 8, (y + i) * 16, BOX_VERTICAL, color);
        
        // Right side
        vbe_draw_char_font_loader((x + width - 1) * 8, (y + i) * 16, BOX_VERTICAL, color);
    }

    // Draw bottom border
    vbe_draw_char_font_loader(x * 8, (y + height - 1) * 16, BOX_BOTTOM_LEFT, color);
    for (int i = 1; i < width - 1; i++) {
        vbe_draw_char_font_loader((x + i) * 8, (y + height - 1) * 16, BOX_HORIZONTAL, color);
    }
    vbe_draw_char_font_loader((x + width - 1) * 8, (y + height - 1) * 16, BOX_BOTTOM_RIGHT, color);
}

// Draw a box with text centered inside it
void draw_box_with_text(int x, int y, int width, int height, const char* text, uint32_t box_color, uint32_t text_color) {
    // Draw the box
    draw_box(x, y, width, height, box_color);

    // Calculate text position for centering
    int text_x = x + (width - strlen(text)) / 2;
    int text_y = y + (height - 1) / 2;

    // Draw the text
    vbe_draw_string_font_loader(text_x * 8, text_y * 16, text, text_color);
} 