#include "../include/drivers/font_loader.h"
#include "../include/PSF1_parser/psf1_parser.h"
#include "../include/font_8x16.h"
#include "../include/fs/fat16.h"
#include "../include/memory/heap.h"
#include <stdbool.h>
#include <stdint.h>

// Global font state
static PSF1Font* main_font = NULL;
static bool main_font_loaded = false;

bool font_loader_init(const char* font_file) {
    // Clean up any existing font
    font_loader_cleanup();
    
    // If no font file specified, use embedded font
    if (!font_file) {
        main_font_loaded = false;
        return true;
    }
    
    // Try to load the main font
    main_font = load_psf1(font_file);
    if (main_font) {
        main_font_loaded = true;
        return true;
    }
    
    // If loading failed, fall back to embedded font
    main_font_loaded = false;
    return true;
}

void font_loader_cleanup(void) {
    if (main_font) {
        free_psf1(main_font);
        main_font = NULL;
        main_font_loaded = false;
    }
}

const uint8_t* font_get_char_bitmap(char c) {
    // Convert to unsigned to properly handle extended ASCII
    unsigned char uc = (unsigned char)c;
    
    // Try main font first
    if (main_font_loaded && main_font && uc < main_font->glyph_count) {
        return &main_font->glyphs[uc * main_font->header.char_height];
    }
    
    // Fall back to embedded font
    if (uc >= font_8x16.first_char && uc <= font_8x16.last_char) {
        return &font_8x16.data[(uc - font_8x16.first_char) * font_8x16.height];
    }
    
    // Return space character if not found
    return &font_8x16.data[0];
}

int font_get_char_width(char c) {
    // Convert to unsigned to properly handle extended ASCII
    unsigned char uc = (unsigned char)c;
    
    // PSF1 fonts are always 8 pixels wide
    if (main_font_loaded && main_font && uc < main_font->glyph_count) {
        return 8;
    }
    
    // Fall back to embedded font
    return font_8x16.width;
}

int font_get_char_height(char c) {
    // Convert to unsigned to properly handle extended ASCII
    unsigned char uc = (unsigned char)c;
    
    // Try main font first
    if (main_font_loaded && main_font && uc < main_font->glyph_count) {
        return main_font->header.char_height;
    }
    
    // Fall back to embedded font
    return font_8x16.height;
}

bool font_is_char_available(char c) {
    // Convert to unsigned to properly handle extended ASCII
    unsigned char uc = (unsigned char)c;
    
    // Check main font first
    if (main_font_loaded && main_font && uc < main_font->glyph_count) {
        return true;
    }
    
    // Check embedded font
    return (uc >= font_8x16.first_char && uc <= font_8x16.last_char);
}

const PSF1Font* get_current_psf1_font(void) {
    return main_font_loaded ? main_font : NULL;
} 