#include "../include/PSF1_parser/psf1_parser.h"
#include "../include/fs/fat16.h"
#include "../include/memory/heap.h"
#include <stdbool.h>
#include <stdint.h>

PSF1Font* load_psf1(const char* path) {
    // Read the file into a buffer
    uint8_t* buffer = malloc(4096);  // 4KB buffer should be enough for most PSF1 fonts
    if (!buffer) {
        terminal_writestring("Failed to allocate memory for font file\n");
        return NULL;
    }

    int result = fat16_read_file(path, buffer, 4096);
    if (result <= 0) {
        terminal_writestring("Failed to read font file\n");
        free(buffer);
        return NULL;
    }

    // Parse the header
    PSF1Header* header = (PSF1Header*)buffer;
    if (header->magic[0] != PSF1_MAGIC0 || header->magic[1] != PSF1_MAGIC1) {
        terminal_writestring("Not a valid PSF1 font\n");
        free(buffer);
        return NULL;
    }

    size_t glyph_count = (header->mode & 0x01) ? 512 : 256;
    size_t glyph_size = header->char_height;
    size_t total_size = glyph_count * glyph_size;
    
    // Allocate memory for the font structure
    PSF1Font* font = malloc(sizeof(PSF1Font));
    if (!font) {
        terminal_writestring("Failed to allocate memory for font structure\n");
        free(buffer);
        return NULL;
    }
    
    // Allocate memory for glyphs
    font->glyphs = malloc(total_size);
    if (!font->glyphs) {
        terminal_writestring("Failed to allocate memory for glyphs\n");
        free(font);
        free(buffer);
        return NULL;
    }
    
    // Copy the header and glyph data
    font->header = *header;
    font->glyph_count = glyph_count;
    memcpy(font->glyphs, buffer + sizeof(PSF1Header), total_size);
    
    free(buffer);
    return font;
}

void render_glyph_vbe(const PSF1Font* font, int index, uint32_t* framebuffer, int fb_width, int fb_pitch, int x, int y) {
    if (index >= font->glyph_count) return;

    uint8_t* glyph = &font->glyphs[index * font->header.char_height];
    for (int row = 0; row < font->header.char_height; row++) {
        uint8_t bits = glyph[row];
        for (int col = 0; col < 8; col++) {
            int pixel_on = bits & (0x80 >> col);
            int fb_index = (y + row) * (fb_pitch / 4) + (x + col);
            framebuffer[fb_index] = pixel_on ? VBE_WHITE : VBE_BLACK;
        }
    }
}

void free_psf1(PSF1Font* font) {
    if (font) {
        free(font->glyphs);
        free(font);
    }
}
