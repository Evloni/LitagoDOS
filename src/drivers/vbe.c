#include "../../include/drivers/vbe.h"
#include "../../include/io.h"
#include "../../include/string.h"
#include "../../include/multiboot.h"
#include "../../include/PSF1_parser/psf1_parser.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

// VBE display dimensions
#define VBE_WIDTH 1024
#define VBE_HEIGHT 768
#define VBE_BPP 32     // Bits per pixel
#define VBE_PITCH (VBE_WIDTH * (VBE_BPP / 8))

// VBE control block
static struct {
    uint32_t* framebuffer;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
    bool initialized;
} vbe_state = {
    .framebuffer = NULL,
    .width = VBE_WIDTH,
    .height = VBE_HEIGHT,
    .pitch = VBE_PITCH,
    .bpp = VBE_BPP,
    .initialized = false
};

// Current cursor position
int vbe_cursor_x = 0;
int vbe_cursor_y = 0;

// Current text color
static uint32_t current_color = 0xFFFFFFFF;  // Default to white

// Initialize VBE
void vbe_init(uint32_t multiboot_magic, void* multiboot_info) {
    if (multiboot_magic != 0x2BADB002) {
        return;
    }

    struct multiboot_header* mb_info = (struct multiboot_header*)multiboot_info;
    
    // Check if VBE info is available
    if (!(mb_info->flags & (1 << 11))) {
        return;
    }

    // Get VBE mode info
    struct vbe_mode_info* mode_info = (struct vbe_mode_info*)mb_info->vbe_mode_info;
    
    // Store framebuffer address
    vbe_state.framebuffer = (uint32_t*)mode_info->framebuffer;
    vbe_state.width = mode_info->width;
    vbe_state.height = mode_info->height;
    vbe_state.pitch = mode_info->pitch;
    vbe_state.bpp = mode_info->bpp;
    vbe_state.initialized = true;
}

// Draw a single character
void vbe_draw_char(int x, int y, char c, uint32_t color, const struct font* font) {
    if (!vbe_state.initialized || !font) return;
    
    // Convert char to unsigned to properly handle extended ASCII
    unsigned char uc = (unsigned char)c;
    
    // Calculate the correct index in the font data
    // For standard ASCII (32-127), subtract 0x20
    // For extended ASCII (128-255), use the character code directly
    size_t index;
    if (uc < 128) {
        index = (uc - 0x20) * font->height;
    } else {
        index = uc * font->height;
    }
    
    // Get character bitmap
    const uint8_t* bitmap = &font->data[index];
    
    // Draw each pixel of the character
    for (int py = 0; py < font->height; py++) {
        for (int px = 0; px < font->width; px++) {
            if (bitmap[py] & (1 << (7 - px))) {
                int screen_x = x + px;
                int screen_y = y + py;
                
                if (screen_x >= 0 && screen_x < vbe_state.width &&
                    screen_y >= 0 && screen_y < vbe_state.height) {
                    uint32_t* pixel = vbe_state.framebuffer + screen_y * (vbe_state.pitch / 4) + screen_x;
                    *pixel = color;
                }
            }
        }
    }
}

// Draw a string
void vbe_draw_string(int x, int y, const char* str, uint32_t color, const struct font* font) {
    if (!vbe_state.initialized || !str || !font) return;
    
    int current_x = x;
    int current_y = y;
    
    while (*str) {
        if (*str == '\n') {
            current_x = x;
            current_y += font->height;
        } else {
            vbe_draw_char(current_x, current_y, *str, color, font);
            current_x += font->width;
            
            // Check for line wrapping
            if (current_x + font->width > vbe_state.width) {
                current_x = x;
                current_y += font->height;
            }
        }
        str++;
    }
}

// Draw a string centered horizontally
void vbe_draw_string_centered(int y, const char* str, uint32_t color, const struct font* font) {
    if (!vbe_state.initialized || !str || !font) return;
    
    int width = strlen(str) * font->width;
    int x = (vbe_state.width - width) / 2;
    vbe_draw_string(x, y, str, color, font);
}

// Draw a rectangle
void vbe_draw_rect(int x, int y, int width, int height, uint32_t color) {
    if (!vbe_state.initialized) return;
    
    for (int py = y; py < y + height; py++) {
        for (int px = x; px < x + width; px++) {
            if (px >= 0 && px < vbe_state.width &&
                py >= 0 && py < vbe_state.height) {
                uint32_t* pixel = vbe_state.framebuffer + py * (vbe_state.pitch / 4) + px;
                *pixel = color;
            }
        }
    }
}

// Clear the screen
void vbe_clear_screen(uint32_t color) {
    if (!vbe_state.initialized) return;
    
    for (int y = 0; y < vbe_state.height; y++) {
        for (int x = 0; x < vbe_state.width; x++) {
            uint32_t* pixel = vbe_state.framebuffer + y * (vbe_state.pitch / 4) + x;
            *pixel = color;
        }
    }
}

// Get screen width
uint16_t vbe_get_width(void) {
    return (uint16_t)vbe_state.width;
}

// Get screen height
uint16_t vbe_get_height(void) {
    return (uint16_t)vbe_state.height;
}

// Set cursor position
void vbe_set_cursor(int x, int y) {
    vbe_cursor_x = x;
    vbe_cursor_y = y;
}

// Get cursor position
void vbe_get_cursor(int* x, int* y) {
    if (x) *x = vbe_cursor_x;
    if (y) *y = vbe_cursor_y;
}

// Terminal-compatible wrapper functions
void terminal_initialize(void) {
    // Already initialized by vbe_init
    vbe_cursor_x = 0;
    vbe_cursor_y = 0;
}

void terminal_clear(void) {
    vbe_clear_screen(0x00000000); // Black background
    vbe_cursor_x = 0;
    vbe_cursor_y = 0;
}

void terminal_putchar(char c) {
    if (c == '\n') {
        vbe_cursor_x = 0;
        vbe_cursor_y += font_get_char_height(c);
    } else {
        vbe_draw_char_font_loader(vbe_cursor_x, vbe_cursor_y, c, current_color);
        vbe_cursor_x += font_get_char_width(c);
        
        // Check for line wrapping
        if (vbe_cursor_x + font_get_char_width(c) > vbe_state.width) {
            vbe_cursor_x = 0;
            vbe_cursor_y += font_get_char_height(c);
        }
    }
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    // Convert from character coordinates to pixel coordinates
    int pixel_x = x * font_get_char_width(c);
    int pixel_y = y * font_get_char_height(c);
    
    // Draw the character
    vbe_draw_char_font_loader(pixel_x, pixel_y, c, color);
    
    // Update cursor position
    vbe_cursor_x = pixel_x + font_get_char_width(c);
    vbe_cursor_y = pixel_y;
}

void terminal_writestring(const char* data) {
    if (!data) return;
    
    while (*data) {
        terminal_putchar(*data++);
    }
}

void terminal_writehex(uint32_t n) {
    char hex[9];
    hex[8] = '\0';
    for (int i = 7; i >= 0; i--) {
        uint8_t digit = n & 0xF;
        hex[i] = digit < 10 ? '0' + digit : 'A' + (digit - 10);
        n >>= 4;
    }
    terminal_writestring(hex);
}

void terminal_get_cursor(size_t* x, size_t* y) {
    if (x) *x = vbe_cursor_x / font_get_char_width(' ');
    if (y) *y = vbe_cursor_y / font_get_char_height(' ');
}

void terminal_update_cursor(void) {
    // Ensure cursor stays within screen bounds
    if (vbe_cursor_x >= vbe_state.width) {
        vbe_cursor_x = 0;
        vbe_cursor_y += font_get_char_height(' ');
    }
    if (vbe_cursor_y >= vbe_state.height) {
        // Scroll the screen up by one line
        vbe_cursor_y -= font_get_char_height(' ');
        // TODO: Implement screen scrolling
    }
}

// Set the current text color
void vbe_setcolor(uint32_t color) {
    current_color = color;
}

void terminal_putchar_color(char c, uint32_t color) {
    if (c == '\n') {
        vbe_cursor_x = 0;
        vbe_cursor_y += font_8x16.height;
    } else {
        vbe_draw_char(vbe_cursor_x, vbe_cursor_y, c, color, &font_8x16);
        vbe_cursor_x += font_8x16.width;
        if (vbe_cursor_x + font_8x16.width > vbe_state.width) {
            vbe_cursor_x = 0;
            vbe_cursor_y += font_8x16.height;
        }
    }
}

void terminal_writestring_color(const char* data, uint32_t color) {
    if (!data) return;
    while (*data) {
        terminal_putchar_color(*data++, color);
    }
}

// Draw a character using a PSF1 font
void vbe_draw_char_psf1(int x, int y, char c, uint32_t color, const PSF1Font* font) {
    if (!font || x < 0 || y < 0 || x >= vbe_state.width || y >= vbe_state.height) return;
    
    // Get the glyph for this character
    uint8_t* glyph = &font->glyphs[c * font->header.char_height];
    
    // Draw each pixel of the glyph
    for (int row = 0; row < font->header.char_height; row++) {
        uint8_t bits = glyph[row];
        for (int col = 0; col < 8; col++) {
            if (bits & (0x80 >> col)) {
                int pixel_x = x + col;
                int pixel_y = y + row;
                if (pixel_x < vbe_state.width && pixel_y < vbe_state.height) {
                    uint32_t* pixel = vbe_state.framebuffer + pixel_y * (vbe_state.pitch / 4) + pixel_x;
                    *pixel = color;
                }
            }
        }
    }
}

// Draw a string using a PSF1 font
void vbe_draw_string_psf1(int x, int y, const char* str, uint32_t color, const PSF1Font* font) {
    if (!str || !font) return;
    
    int current_x = x;
    while (*str) {
        vbe_draw_char_psf1(current_x, y, *str, color, font);
        current_x += 8;  // PSF1 fonts are always 8 pixels wide
        str++;
    }
}

// Draw a string centered horizontally using a PSF1 font
void vbe_draw_string_centered_psf1(int y, const char* str, uint32_t color, const PSF1Font* font) {
    if (!str || !font) return;
    
    int width = strlen(str) * 8;  // PSF1 fonts are always 8 pixels wide
    int x = (vbe_state.width - width) / 2;
    vbe_draw_string_psf1(x, y, str, color, font);
}

// Draw a character using the font loader
void vbe_draw_char_font_loader(int x, int y, char c, uint32_t color) {
    if (!vbe_state.initialized) return;
    
    // Get character bitmap and dimensions
    const uint8_t* bitmap = font_get_char_bitmap(c);
    int width = font_get_char_width(c);
    int height = font_get_char_height(c);
    
    // Draw each pixel of the character
    for (int py = 0; py < height; py++) {
        for (int px = 0; px < width; px++) {
            int byte_index = py * ((width + 7) / 8);
            int bit_index = 7 - (px % 8);  // Most significant bit first
            
            if (bitmap[byte_index] & (1 << bit_index)) {
                int screen_x = x + px;
                int screen_y = y + py;
                
                if (screen_x >= 0 && screen_x < vbe_state.width &&
                    screen_y >= 0 && screen_y < vbe_state.height) {
                    uint32_t* pixel = vbe_state.framebuffer + screen_y * (vbe_state.pitch / 4) + screen_x;
                    *pixel = color;
                }
            }
        }
    }
}

// Draw a string using the font loader
void vbe_draw_string_font_loader(int x, int y, const char* str, uint32_t color) {
    if (!vbe_state.initialized || !str) return;
    
    int current_x = x;
    int current_y = y;
    
    while (*str) {
        if (*str == '\n') {
            current_x = x;
            current_y += font_get_char_height(*str);
        } else {
            vbe_draw_char_font_loader(current_x, current_y, *str, color);
            current_x += font_get_char_width(*str);
            
            // Check for line wrapping
            if (current_x + font_get_char_width(*str) > vbe_state.width) {
                current_x = x;
                current_y += font_get_char_height(*str);
            }
        }
        str++;
    }
}

// Draw a string centered horizontally using the font loader
void vbe_draw_string_centered_font_loader(int y, const char* str, uint32_t color) {
    if (!vbe_state.initialized || !str) return;
    
    // Calculate total width of the string
    int total_width = 0;
    const char* s = str;
    while (*s) {
        total_width += font_get_char_width(*s);
        s++;
    }
    
    int x = (vbe_state.width - total_width) / 2;
    vbe_draw_string_font_loader(x, y, str, color);
}

// Get framebuffer address
void* vbe_get_framebuffer(void) {
    return vbe_state.framebuffer;
}

// Get framebuffer pitch
uint32_t vbe_get_pitch(void) {
    return vbe_state.pitch;
}

// Draw a single pixel
void vbe_put_pixel(int x, int y, uint32_t color) {
    if (!vbe_state.initialized || 
        x < 0 || x >= vbe_state.width || 
        y < 0 || y >= vbe_state.height) {
        return;
    }

    uint32_t* pixel = vbe_state.framebuffer + y * (vbe_state.pitch / 4) + x;
    *pixel = color;
}

