#ifndef FONT_8X16_H
#define FONT_8X16_H

#include <stdint.h>

// Font structure definition
struct font {
    int width;
    int height;
    int first_char;
    int last_char;
    const uint8_t* data;
};

// Declare the font data as external
extern const uint8_t font_8x16_data[96 * 16];

// Declare the font structure as external
extern const struct font font_8x16;

#endif // FONT_8X16_H
