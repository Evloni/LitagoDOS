#ifndef PSF1_PARSER_H
#define PSF1_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04
#define VBE_WHITE 0xFFFFFFFF
#define VBE_BLACK 0x00000000


typedef struct {
    uint8_t magic[2];
    uint8_t mode;
    uint8_t char_height;
} PSF1Header;

typedef struct {
    PSF1Header header;
    uint8_t* glyphs;
    size_t glyph_count;
} PSF1Font;

PSF1Font* load_psf1(const char* path);
void render_glyph_vbe(const PSF1Font* font, int index, uint32_t* framebuffer, int fb_width, int fb_pitch, int x, int y);
void free_psf1(PSF1Font* font);

#endif // PSF1_PARSER_H