#ifndef VBE_H
#define VBE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../multiboot.h"
#include "font_8x16.h"
#include "bdf_font.h"

// VBE default dimensions
#define VBE_WIDTH 1024
#define VBE_HEIGHT 768

// Global cursor position
extern int vbe_cursor_x;
extern int vbe_cursor_y;

// VBE Information Block
struct vbe_info_block {
    char signature[4];         // "VESA"
    uint16_t version;          // VBE version
    uint32_t oem_string_ptr;   // Pointer to OEM string
    uint32_t capabilities;     // Capabilities
    uint32_t video_mode_ptr;   // Pointer to video mode list
    uint16_t total_memory;     // Total memory in 64KB blocks
    uint16_t oem_software_rev; // OEM software revision
    uint32_t oem_vendor_name_ptr;
    uint32_t oem_product_name_ptr;
    uint32_t oem_product_rev_ptr;
    char reserved[222];
    char oem_data[256];
} __attribute__((packed));

// VBE Mode Information Block
struct vbe_mode_info {
    uint16_t attributes;
    uint8_t window_a;
    uint8_t window_b;
    uint16_t granularity;
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr;
    uint16_t pitch;
    uint16_t width;
    uint16_t height;
    uint8_t w_char;
    uint8_t y_char;
    uint8_t planes;
    uint8_t bpp;
    uint8_t banks;
    uint8_t memory_model;
    uint8_t bank_size;
    uint8_t image_pages;
    uint8_t reserved0;
    uint8_t red_mask;
    uint8_t red_position;
    uint8_t green_mask;
    uint8_t green_position;
    uint8_t blue_mask;
    uint8_t blue_position;
    uint8_t reserved_mask;
    uint8_t reserved_position;
    uint8_t direct_color_attributes;
    uint32_t framebuffer;
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size;
    uint8_t reserved1[206];
} __attribute__((packed));

struct vbe_text_context {
    int cursor_x;
    int cursor_y;
    uint32_t fg_color;
    uint32_t bg_color;
    const struct font* font;
    int tab_size;
    int line_height;
    int char_spacing;
};

// Function declarations
void vbe_init(uint32_t multiboot_magic, void* multiboot_info);
void vbe_set_mode(uint16_t mode);
void vbe_put_pixel(int x, int y, uint32_t color);
void vbe_clear_screen(uint32_t color);
void vbe_draw_rect(int x, int y, int width, int height, uint32_t color);
void vbe_draw_char(int x, int y, char c, uint32_t color, const struct font* font);
void vbe_draw_string(int x, int y, const char* str, uint32_t color, const struct font* font);
void vbe_draw_string_centered(int y, const char* str, uint32_t color, const struct font* font);
uint16_t vbe_get_width(void);
uint16_t vbe_get_height(void);
void vbe_init_text_context(struct vbe_text_context* ctx, int start_x, int start_y, 
                          uint32_t fg_color, uint32_t bg_color, const struct font* font);
void vbe_draw_string_wrapped(struct vbe_text_context* ctx, const char* str, int max_width);
void vbe_draw_string_scroll(struct vbe_text_context* ctx, const char* str, int max_width, int max_height);
uint32_t vbe_get_pixel(int x, int y);

// Terminal color functions
void vbe_setcolor(uint32_t color);
uint32_t vbe_getcolor(void);

// Cursor functions
void vbe_set_cursor(int x, int y);

// Terminal-compatible wrapper functions
void terminal_initialize(void);
void terminal_clear(void);
void terminal_putchar(char c);
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
void terminal_writestring(const char* data);
void terminal_writehex(uint32_t n);
void terminal_get_cursor(size_t* x, size_t* y);
void terminal_update_cursor(void);

// ANSI support
void ansi_set_enabled(bool enabled);

// BDF font drawing functions
void vbe_draw_char_bdf(int x, int y, char c, uint32_t color, const struct bdf_font* font);
void vbe_draw_string_bdf(int x, int y, const char* str, uint32_t color, const struct bdf_font* font);
void vbe_draw_string_centered_bdf(int y, const char* str, uint32_t color, const struct bdf_font* font);

// Font loader drawing functions
void vbe_draw_char_font_loader(int x, int y, char c, uint32_t color);
void vbe_draw_string_font_loader(int x, int y, const char* str, uint32_t color);
void vbe_draw_string_centered_font_loader(int y, const char* str, uint32_t color);

#endif // VBE_H 