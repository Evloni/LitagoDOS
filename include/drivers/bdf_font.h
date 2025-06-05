#ifndef BDF_FONT_H
#define BDF_FONT_H

#include <stdint.h>
#include <stdbool.h>

// BDF font structure
struct bdf_font {
    int width;           // Font width
    int height;          // Font height
    int first_char;      // First character code
    int last_char;       // Last character code
    int default_char;    // Default character code
    int x_offset;        // X offset for all characters
    int y_offset;        // Y offset for all characters
    uint8_t* data;       // Font bitmap data
    bool* char_exists;   // Array indicating which characters exist
    int data_size;       // Size of the font data
};

// Function declarations
bool bdf_load_font(const char* filename, struct bdf_font* font);
void bdf_free_font(struct bdf_font* font);
bool bdf_char_exists(const struct bdf_font* font, int char_code);
const uint8_t* bdf_get_char_bitmap(const struct bdf_font* font, int char_code);
int bdf_get_char_width(const struct bdf_font* font, int char_code);
int bdf_get_char_height(const struct bdf_font* font, int char_code);

#endif // BDF_FONT_H 