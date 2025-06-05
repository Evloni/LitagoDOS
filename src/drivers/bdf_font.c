#include "../../include/drivers/bdf_font.h"
#include "../../include/fs/fat16.h"
#include "../../include/string.h"
#include "../../include/memory/heap.h"
#include <stdbool.h>
#include <stdint.h>

// Helper function to parse a hex string
static uint8_t parse_hex_byte(const char* hex) {
    uint8_t value = 0;
    for (int i = 0; i < 2; i++) {
        char c = hex[i];
        value <<= 4;
        if (c >= '0' && c <= '9') {
            value |= c - '0';
        } else if (c >= 'A' && c <= 'F') {
            value |= c - 'A' + 10;
        } else if (c >= 'a' && c <= 'f') {
            value |= c - 'a' + 10;
        }
    }
    return value;
}

// Helper function to parse a decimal string
static int parse_decimal(const char* str) {
    int value = 0;
    while (*str >= '0' && *str <= '9') {
        value = value * 10 + (*str - '0');
        str++;
    }
    return value;
}

bool bdf_load_font(const char* filename, struct bdf_font* font) {
    // Initialize font structure
    memset(font, 0, sizeof(struct bdf_font));
    
    // First, get the file size
    terminal_writestring("Getting font file size...\n");
    uint32_t file_size = fat16_get_file_size(filename);
    if (file_size == 0) {
        terminal_writestring("Error: Font file not found or empty\n");
        return false;
    }
    
    terminal_writestring("Font file size: ");
    char size_str[32];
    sprintf(size_str, "%d bytes\n", file_size);
    terminal_writestring(size_str);
    
    // Allocate buffer for the file
    uint8_t* buffer = (uint8_t*)kmalloc(file_size);
    if (!buffer) {
        terminal_writestring("Error: Failed to allocate memory for font file\n");
        return false;
    }
    
    // Read the file
    terminal_writestring("Reading font file...\n");
    if (!fat16_read_file(filename, buffer, file_size)) {
        terminal_writestring("Error: Failed to read font file\n");
        kfree(buffer);
        return false;
    }
    
    terminal_writestring("Parsing BDF file...\n");
    
    // Parse the BDF file
    char* line = (char*)buffer;
    char* end = (char*)(buffer + file_size);
    bool in_char = false;
    int current_char = 0;
    int bitmap_row = 0;
    int max_width = 0;
    int max_height = 0;
    
    // First pass: determine font properties and count characters
    while (line < end) {
        char* next_line = strchr(line, '\n');
        if (!next_line) break;
        *next_line = '\0';
        
        if (strncmp(line, "FONTBOUNDINGBOX", 15) == 0) {
            char* width_str = line + 16;
            char* height_str = strchr(width_str, ' ');
            if (height_str) {
                *height_str = '\0';
                height_str++;
                max_width = parse_decimal(width_str);
                max_height = parse_decimal(height_str);
            }
        }
        else if (strncmp(line, "STARTCHAR", 9) == 0) {
            in_char = true;
            bitmap_row = 0;
        }
        else if (strncmp(line, "ENDCHAR", 7) == 0) {
            in_char = false;
            current_char++;
        }
        
        line = next_line + 1;
    }
    
    // Allocate font data
    font->width = max_width;
    font->height = max_height;
    font->first_char = 32;  // Start with space
    font->last_char = 127;  // End with DEL
    font->default_char = 32;  // Space as default
    font->x_offset = 0;
    font->y_offset = 0;
    
    int char_count = font->last_char - font->first_char + 1;
    font->char_exists = (bool*)kmalloc(char_count * sizeof(bool));
    memset(font->char_exists, 0, char_count * sizeof(bool));
    
    // Calculate data size needed
    int bytes_per_char = (max_width + 7) / 8 * max_height;
    font->data_size = char_count * bytes_per_char;
    font->data = (uint8_t*)kmalloc(font->data_size);
    memset(font->data, 0, font->data_size);
    
    // Second pass: parse character data
    line = (char*)buffer;
    in_char = false;
    current_char = 0;
    bitmap_row = 0;
    int current_char_code = 0;
    
    terminal_writestring("Parsing character data...\n");
    terminal_writestring("Font dimensions: ");
    char dims[32];
    sprintf(dims, "%dx%d\n", max_width, max_height);
    terminal_writestring(dims);
    
    while (line < end) {
        char* next_line = strchr(line, '\n');
        if (!next_line) break;
        *next_line = '\0';
        
        if (strncmp(line, "STARTCHAR", 9) == 0) {
            in_char = true;
            bitmap_row = 0;
        }
        else if (strncmp(line, "ENCODING", 8) == 0) {
            current_char_code = parse_decimal(line + 9);
            if (current_char_code == 65) { // Character 'A'
                terminal_writestring("Found character 'A'\n");
            }
        }
        else if (strncmp(line, "BITMAP", 6) == 0) {
            // Start reading bitmap data
            bitmap_row = 0;
            if (current_char_code == 65) {
                terminal_writestring("Reading bitmap for 'A'\n");
            }
        }
        else if (in_char && bitmap_row < max_height) {
            // Parse bitmap row
            int char_index = current_char_code - font->first_char;
            if (char_index >= 0 && char_index < char_count) {
                font->char_exists[char_index] = true;
                uint8_t* char_data = font->data + char_index * bytes_per_char;
                int bytes_per_row = (max_width + 7) / 8;
                
                // Parse the hex string into bytes
                for (int i = 0; i < bytes_per_row; i++) {
                    // Each byte in the BDF file is represented by 2 hex characters
                    char hex_byte[3] = {line[i * 2], line[i * 2 + 1], '\0'};
                    uint8_t value = parse_hex_byte(hex_byte);
                    char_data[bitmap_row * bytes_per_row + i] = value;
                    
                    // Debug output for character 'A' (ASCII 65)
                    if (current_char_code == 65 && bitmap_row < 16) {
                        terminal_writestring("Row ");
                        char row_str[4];
                        sprintf(row_str, "%d: ", bitmap_row);
                        terminal_writestring(row_str);
                        terminal_writehex(value);
                        terminal_writestring("\n");
                    }
                }
            }
            bitmap_row++;
        }
        else if (strncmp(line, "ENDCHAR", 7) == 0) {
            in_char = false;
            if (current_char_code == 65) {
                terminal_writestring("Finished reading 'A'\n");
            }
        }
        
        line = next_line + 1;
    }
    
    terminal_writestring("Font parsing complete\n");
    terminal_writestring("First character: ");
    sprintf(dims, "%d\n", font->first_char);
    terminal_writestring(dims);
    terminal_writestring("Last character: ");
    sprintf(dims, "%d\n", font->last_char);
    terminal_writestring(dims);
    
    kfree(buffer);
    return true;
}

void bdf_free_font(struct bdf_font* font) {
    if (font) {
        if (font->data) {
            kfree(font->data);
        }
        if (font->char_exists) {
            kfree(font->char_exists);
        }
        memset(font, 0, sizeof(struct bdf_font));
    }
}

bool bdf_char_exists(const struct bdf_font* font, int char_code) {
    if (!font || !font->char_exists) return false;
    int index = char_code - font->first_char;
    return (index >= 0 && index <= font->last_char - font->first_char) && font->char_exists[index];
}

const uint8_t* bdf_get_char_bitmap(const struct bdf_font* font, int char_code) {
    if (!bdf_char_exists(font, char_code)) {
        char_code = font->default_char;
    }
    int index = char_code - font->first_char;
    int bytes_per_char = (font->width + 7) / 8 * font->height;
    return font->data + index * bytes_per_char;
}

int bdf_get_char_width(const struct bdf_font* font, int char_code) {
    return font->width;
}

int bdf_get_char_height(const struct bdf_font* font, int char_code) {
    return font->height;
} 