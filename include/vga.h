#ifndef VGA_H
#define VGA_H

#include <stddef.h>
#include <stdint.h>

// VGA text mode color constants
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
    VGA_COLOR_YELLOW = 14,  // Same as LIGHT_BROWN
    VGA_COLOR_WHITE = 15,
};

// Function declarations
void vga_init(void);
void terminal_initialize(void);
void terminal_setcolor(uint8_t color);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void terminal_set_cursor(size_t x, size_t y);
void terminal_get_cursor(size_t* x, size_t* y);
void terminal_update_cursor(void);
void terminal_enable_cursor(void);
void terminal_clear(void);  // Clear the terminal screen
char terminal_getchar(void);  // Get a single character
void terminal_getstring(char* buffer, size_t max_length);  // Get a string

// Helper functions
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

#endif // VGA_H 