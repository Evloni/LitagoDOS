#ifndef VGA_H
#define VGA_H

#include <stddef.h>
#include <stdint.h>

// VGA text mode constants
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

// External terminal position variables
extern size_t terminal_row;
extern size_t terminal_column;

// VGA graphics mode constants
#define VGA_GRAPHICS_WIDTH 320
#define VGA_GRAPHICS_HEIGHT 200
#define VGA_GRAPHICS_MEMORY 0xA0000

// VGA colors
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_YELLOW = 14,
    VGA_COLOR_WHITE = 15,
};

// VGA mode types
enum vga_mode {
    VGA_MODE_TEXT = 0,
    VGA_MODE_GRAPHICS = 1
};

// Function declarations
void terminal_initialize(void);
void terminal_setcolor(uint8_t color);
uint8_t terminal_getcolor(void);
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void terminal_clear(void);
void terminal_set_cursor(size_t x, size_t y);
void terminal_get_cursor(size_t* x, size_t* y);
void terminal_save_cursor(void);
void terminal_restore_cursor(void);
void terminal_writeint(int num);
void terminal_writehex(uint32_t num);
void terminal_scroll(void);
void terminal_update_cursor(void);

// Function declarations for graphics mode
void vga_set_mode(enum vga_mode mode);
void vga_plot_pixel(int x, int y, uint8_t color);
void vga_clear_screen(uint8_t color);
void vga_draw_line(int x0, int y0, int x1, int y1, uint8_t color);
void vga_draw_rect(int x, int y, int width, int height, uint8_t color);
void vga_fill_rect(int x, int y, int width, int height, uint8_t color);

// Helper functions
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)uc | (uint16_t)color << 8;
}

#endif // VGA_H 