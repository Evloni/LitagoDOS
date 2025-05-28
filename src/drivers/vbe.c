#include "../../include/vbe.h"
#include "../../include/io.h"
#include "../../include/multiboot.h"
#include "../../include/font_8x16.h"

#include <stdint.h>
#include <stddef.h>

// Global VBE variables
uint32_t* framebuffer = NULL;
int screen_width = 0;
int screen_height = 0;
int screen_pitch = 0;

// Initialize VBE mode
void vbe_init(uint32_t multiboot_magic, void* multiboot_info_ptr) {
    if (multiboot_magic != 0x2BADB002) {
        return; // Not booted by a multiboot-compliant bootloader
    }

    struct multiboot_header* mb_info = (struct multiboot_header*)multiboot_info_ptr;
    
    // Check if VBE info is available
    if (!(mb_info->flags & (1 << 11))) {
        return; // VBE info not available
    }

    // Get VBE mode info
    struct vbe_mode_info* mode_info = (struct vbe_mode_info*)(mb_info->vbe_mode_info);
    
    // Initialize global variables
    framebuffer = (uint32_t*)mode_info->framebuffer;
    screen_width = mode_info->width;
    screen_height = mode_info->height;
    screen_pitch = mode_info->pitch;
}

// Get screen width
uint16_t vbe_get_width(void) {
    return screen_width;
}

// Get screen height
uint16_t vbe_get_height(void) {
    return screen_height;
}

// Clear the screen with a color
void vbe_clear_screen(uint32_t color) {
    if (!framebuffer) return;
    
    for (int y = 0; y < screen_height; y++) {
        uint32_t* fb = (uint32_t*)((uint8_t*)framebuffer + y * screen_pitch);
        for (int x = 0; x < screen_width; x++) {
            fb[x] = color;
        }
    }
}

// Plots a pixel (x, y) with color (32-bit)
void vbe_plot_pixel(int x, int y, uint32_t color) {
    if (x < 0 || y < 0 || x >= screen_width || y >= screen_height) return;
    uint32_t* fb = (uint32_t*)((uint8_t*)framebuffer + y * screen_pitch);
    fb[x] = color;
}

// Fill rectangle with color (used for background text)
void vbe_fill_rect(int x, int y, int w, int h, uint32_t color) {
    for (int dy = 0; dy < h; dy++) {
        for (int dx = 0; dx < w; dx++) {
            vbe_plot_pixel(x + dx, y + dy, color);
        }
    }
}

// Draw single character
void vbe_draw_char(int x, int y, char c, uint32_t color, const struct font* font) {
    if (!framebuffer || !font || c < font->first_char || c > font->last_char) return;

    int char_index = c - font->first_char;
    const uint8_t* char_data = &font->data[char_index * font->height];

    for (int row = 0; row < font->height; row++) {
        uint8_t bits = char_data[row];
        for (int col = 0; col < font->width; col++) {
            if (bits & (1 << (7 - col))) {
                vbe_plot_pixel(x + col, y + row, color);
            }
        }
    }
}

// Draw string
void vbe_draw_string(int x, int y, const char* str, uint32_t color, const struct font* font) {
    while (*str) {
        vbe_draw_char(x, y, *str, color, font);
        x += font->width;
        str++;
    }
}

// Draw string with background color
void vbe_draw_string_bg(int x, int y, const char* str, uint32_t fg, uint32_t bg, const struct font* font) {
    while (*str) {
        vbe_fill_rect(x, y, font->width, font->height, bg);
        vbe_draw_char(x, y, *str, fg, font);
        x += font->width;
        str++;
    }
}
