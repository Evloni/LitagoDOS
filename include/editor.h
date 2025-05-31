#ifndef EDITOR_H
#define EDITOR_H

#include <stdbool.h>
#include <stdint.h>
#include "../include/drivers/vbe.h"

// Editor configuration
#define EDITOR_MAX_LINES 1000
#define EDITOR_MAX_LINE_LENGTH 256
#define EDITOR_STATUS_BAR_HEIGHT 1
#define EDITOR_EDITABLE_HEIGHT (VBE_HEIGHT / 16 - EDITOR_STATUS_BAR_HEIGHT)  // Assuming 8x16 font

// Editor state structure
typedef struct {
    char** lines;                    // Array of text lines
    int num_lines;                   // Number of lines
    int cursor_x;                    // Cursor X position
    int cursor_y;                    // Cursor Y position
    int scroll_offset;               // Number of lines scrolled up
    bool modified;                   // Whether the file has been modified
    char* filename;                  // Current file name
    uint32_t text_color;             // Text color (32-bit ARGB)
    uint32_t bg_color;               // Background color (32-bit ARGB)
} Editor;

// Function declarations
void editor_init(Editor* editor);
void editor_free(Editor* editor);
void editor_draw(Editor* editor);
bool editor_handle_input(Editor* editor);
void editor_main_loop(Editor* editor);

// Line operations
void editor_insert_char(Editor* editor, char c);
void editor_delete_char(Editor* editor);
void editor_new_line(Editor* editor);
void editor_delete_line(Editor* editor);

// Cursor operations
void editor_move_cursor(Editor* editor, int dx, int dy);
void editor_scroll(Editor* editor, int lines);

// File operations
bool editor_load_file(Editor* editor, const char* filename);
bool editor_save_file(Editor* editor, const char* filename);

#endif // EDITOR_H