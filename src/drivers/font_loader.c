#include "../include/drivers/font_loader.h"
#include "../include/drivers/bdf_font.h"
#include "../include/font_8x16.h"
#include "../include/fs/fat16.h"
#include "../include/memory/heap.h"
#include <stdbool.h>
#include <stdint.h>

// Global font state
static struct bdf_font main_font;
static bool main_font_loaded = false;

bool font_loader_init(const char* font_file) {
    // Try to load the main font
    if (bdf_load_font(font_file, &main_font)) {
        main_font_loaded = true;
        return true;
    }
    return false;
}

void font_loader_cleanup(void) {
    if (main_font_loaded) {
        bdf_free_font(&main_font);
        main_font_loaded = false;
    }
}

const uint8_t* font_get_char_bitmap(char c) {
    // Try main font first
    if (main_font_loaded && bdf_char_exists(&main_font, c)) {
        return bdf_get_char_bitmap(&main_font, c);
    }
    
    // Fall back to embedded font
    if (c >= font_8x16.first_char && c <= font_8x16.last_char) {
        return &font_8x16.data[(c - font_8x16.first_char) * font_8x16.height];
    }
    
    // Return space character if not found
    return &font_8x16.data[0];
}

int font_get_char_width(char c) {
    // Try main font first
    if (main_font_loaded && bdf_char_exists(&main_font, c)) {
        return bdf_get_char_width(&main_font, c);
    }
    
    // Fall back to embedded font
    return font_8x16.width;
}

int font_get_char_height(char c) {
    // Try main font first
    if (main_font_loaded && bdf_char_exists(&main_font, c)) {
        return bdf_get_char_height(&main_font, c);
    }
    
    // Fall back to embedded font
    return font_8x16.height;
}

bool font_is_char_available(char c) {
    // Check main font first
    if (main_font_loaded && bdf_char_exists(&main_font, c)) {
        return true;
    }
    
    // Check embedded font
    return (c >= font_8x16.first_char && c <= font_8x16.last_char);
} 