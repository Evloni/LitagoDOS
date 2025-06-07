#ifndef FONT_LOADER_H
#define FONT_LOADER_H

#include <stdbool.h>
#include <stdint.h>
#include "../PSF1_parser/psf1_parser.h"

// Initialize the font loader with a font file
bool font_loader_init(const char* font_file);

// Clean up font resources
void font_loader_cleanup(void);

// Get the bitmap data for a character
const uint8_t* font_get_char_bitmap(char c);

// Get the width of a character
int font_get_char_width(char c);

// Get the height of a character
int font_get_char_height(char c);

// Check if a character is available in either font
bool font_is_char_available(char c);

// Get the currently loaded PSF1 font
const PSF1Font* get_current_psf1_font(void);

#endif // FONT_LOADER_H 