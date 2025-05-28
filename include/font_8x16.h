#ifndef FONT_8X16_H
#define FONT_8X16_H

#include <stdint.h>

// Font structure for 8x16 bitmap fonts
struct font {
    uint8_t width;           // Width of each character in pixels
    uint8_t height;          // Height of each character in pixels
    uint8_t first_char;      // First character in the font
    uint8_t last_char;       // Last character in the font
    const uint8_t* data;     // Pointer to the font bitmap data
};

// Basic 8x16 font data (ASCII characters 32-127)
extern const uint8_t font_8x16_data[];

// Default 8x16 font instance
extern const struct font font_8x16;

#endif // FONT_8X16_H 