#include "../include/editor.h"
#include "../include/keyboardDriver.h"
#include "../include/string.h"
#include "../include/memory/heap.h"
#include "../include/fs/fat16.h"
#include "../include/drivers/vbe.h"
#include "../include/shell.h"  // This contains the current_cluster declaration
#include <string.h>
#include <stdlib.h>

// Replace the current external declaration with:
extern struct modifier_state modifier_state;

#define EDITOR_MAX_LINES 1000
#define EDITOR_MAX_LINE_LENGTH 256
#define EDITOR_EDITABLE_HEIGHT (VBE_HEIGHT / 16 - 1)  // Leave one line for status bar
#define LINE_NUMBER_GUTTER_WIDTH 4  // Increased from 2 to 4 to handle larger numbers
#define LINE_NUMBER_COLOR 0xFF555555  // Color for line numbers

// Add this helper function at the top of the file, after the includes
static void format_line_number(char* buffer, int width, int number) {
    // Convert number to string
    int i = width - 1;
    buffer[i] = ' ';  // Space separator
    i--;
    
    // Handle zero case
    if (number == 0) {
        while (i >= 0) {
            buffer[i] = ' ';
            i--;
        }
        buffer[width - 2] = '0';
        return;
    }
    
    // Convert number to string, right-aligned
    while (number > 0 && i >= 0) {
        buffer[i] = '0' + (number % 10);
        number /= 10;
        i--;
    }
    
    // Fill remaining space with spaces
    while (i >= 0) {
        buffer[i] = ' ';
        i--;
    }
}

// Add this helper function to handle scrolling
static void editor_adjust_scroll(Editor* editor) {
    // If cursor is above the visible area, scroll up
    if (editor->cursor_y < editor->scroll_offset) {
        editor->scroll_offset = editor->cursor_y;
    }
    // If cursor is below the visible area, scroll down
    else if (editor->cursor_y >= editor->scroll_offset + EDITOR_EDITABLE_HEIGHT) {
        editor->scroll_offset = editor->cursor_y - EDITOR_EDITABLE_HEIGHT + 1;
    }
}

// Initialize a new editor instance
void editor_init(Editor* editor) {
    // Allocate memory for lines
    editor->lines = (char**)kmalloc(EDITOR_MAX_LINES * sizeof(char*));
    
    // Initialize each line
    for (int i = 0; i < EDITOR_MAX_LINES; i++) {
        editor->lines[i] = (char*)kmalloc(EDITOR_MAX_LINE_LENGTH);
        editor->lines[i][0] = '\0';  // Empty string
    }
    
    // Initialize other fields
    editor->num_lines = 1;           // Start with one empty line
    editor->cursor_x = 0;
    editor->cursor_y = 0;
    editor->scroll_offset = 0;
    editor->modified = false;
    editor->filename = NULL;
    // Set Soft Theme colors
    editor->text_color = 0xFFFFFFFF;
    editor->bg_color = 0x000000;
}

// Free editor resources
void editor_free(Editor* editor) {
    if (editor->lines) {
        for (int i = 0; i < EDITOR_MAX_LINES; i++) {
            if (editor->lines[i]) {
                kfree(editor->lines[i]);
            }
        }
        kfree(editor->lines);
    }
    if (editor->filename) {
        kfree(editor->filename);
    }
}

// Draw the editor interface
void editor_draw(Editor* editor) {
    // Clear the screen
    vbe_clear_screen(editor->bg_color);
    
    // Draw visible lines
    for (int i = 0; i < EDITOR_EDITABLE_HEIGHT; i++) {
        int line_num = i + editor->scroll_offset;
        if (line_num < editor->num_lines) {
            // Draw line number
            char line_num_str[LINE_NUMBER_GUTTER_WIDTH + 1];
            format_line_number(line_num_str, LINE_NUMBER_GUTTER_WIDTH, line_num + 1);
            line_num_str[LINE_NUMBER_GUTTER_WIDTH] = '\0';
            
            // Draw line number in dark grey
            vbe_draw_string(0, i * 16, line_num_str, LINE_NUMBER_COLOR, &font_8x16);
            
            // Draw line content
            char* line = editor->lines[line_num];
            vbe_draw_string(LINE_NUMBER_GUTTER_WIDTH * 8, i * 16, line, editor->text_color, &font_8x16);
        }
    }
    
    // Draw status bar with white text on black background
    vbe_draw_rect(0, (VBE_HEIGHT / 16 - 1) * 16, VBE_WIDTH, 16, 0x000000);
    
    // Show filename and modified status on the left
    char status[256] = "-- ";
    if (editor->filename) {
        strcat(status, editor->filename);
    }
    if (editor->modified) {
        strcat(status, " (modified)");
    }
    strcat(status, " -- ");
    vbe_draw_string(0, (VBE_HEIGHT / 16 - 1) * 16, status, 0xFFFFFFFF, &font_8x16);

    // Add line and column information on the right
    char position_str[20];
    int pos = 0;
    
    // Format "Ln X, Col Y"
    position_str[pos++] = 'L';
    position_str[pos++] = 'n';
    position_str[pos++] = ' ';
    // Convert line number to string
    int line_num = editor->cursor_y + 1;
    char line_str[10];
    int line_len = 0;
    while (line_num > 0) {
        line_str[line_len++] = '0' + (line_num % 10);
        line_num /= 10;
    }
    while (--line_len >= 0) {
        position_str[pos++] = line_str[line_len];
    }
    
    position_str[pos++] = ',';
    position_str[pos++] = ' ';
    position_str[pos++] = 'C';
    position_str[pos++] = 'o';
    position_str[pos++] = 'l';
    position_str[pos++] = ' ';
    // Convert column number to string
    int col_num = editor->cursor_x + 1;
    char col_str[10];
    int col_len = 0;
    while (col_num > 0) {
        col_str[col_len++] = '0' + (col_num % 10);
        col_num /= 10;
    }
    while (--col_len >= 0) {
        position_str[pos++] = col_str[col_len];
    }
    position_str[pos] = '\0';
    
    // Draw position string right-aligned
    int pos_x = VBE_WIDTH - (strlen(position_str) * 8);
    vbe_draw_string(pos_x, (VBE_HEIGHT / 16 - 1) * 16, position_str, 0xFFFFFFFF, &font_8x16);
}

// Handle a single character input
bool editor_handle_input(Editor* editor) {
    char c = keyboard_getchar();
    
    // Check if this is a Ctrl+S combination
    if (c == 's' && modifier_state.ctrl) {  // Use the global modifier_state
        if (editor->filename) {
            if (editor_save_file(editor, editor->filename)) {
                // Show save success message
                terminal_writestring("File saved successfully\n");
            } else {
                // Show save error message
                terminal_writestring("Error saving file\n");
            }
        } else {
            // Show no filename message
            terminal_writestring("No filename specified\n");
        }
        return true;  // Continue editor loop
    }
    
    // Check if this is a Ctrl+Q combination to exit
    if (c == 'q' && modifier_state.ctrl) {
        // If file is modified, show warning
        if (editor->modified) {
            terminal_writestring("Warning: File has unsaved changes. Press Ctrl+Q again to exit anyway.\n");
            return true;  // Continue editor loop
        }
        return false;  // Exit editor loop
    }
    
    // Handle arrow keys and other navigation
    if (c == '\033') {  // Escape sequence
        char next = keyboard_getchar();
        if (next == '[') {  // Control sequence introducer
            char code = keyboard_getchar();
            switch (code) {
                case 'A':  // Up arrow
                    if (editor->cursor_y > 0) {
                        editor->cursor_y--;
                        // Adjust cursor_x to not exceed line length
                        int line_len = strlen(editor->lines[editor->cursor_y]);
                        if (editor->cursor_x > line_len) {
                            editor->cursor_x = line_len;
                        }
                        editor_adjust_scroll(editor);
                    }
                    break;
                case 'B':  // Down arrow
                    if (editor->cursor_y < editor->num_lines - 1) {
                        editor->cursor_y++;
                        // Adjust cursor_x to not exceed line length
                        int line_len = strlen(editor->lines[editor->cursor_y]);
                        if (editor->cursor_x > line_len) {
                            editor->cursor_x = line_len;
                        }
                        editor_adjust_scroll(editor);
                    }
                    break;
                case 'C':  // Right arrow
                    if (editor->cursor_x < strlen(editor->lines[editor->cursor_y])) {
                        editor->cursor_x++;
                    }
                    break;
                case 'D':  // Left arrow
                    if (editor->cursor_x > 0) {
                        editor->cursor_x--;
                    }
                    break;
            }
        }
        return true;
    }
    
    switch (c) {
        case '\b':  // Backspace
            editor_delete_char(editor);
            break;
        case '\n':  // Enter
            editor_new_line(editor);
            break;
        case '\t':  // Tab
            // Insert 4 spaces
            for (int i = 0; i < 4; i++) {
                editor_insert_char(editor, ' ');
            }
            break;
        default:
            if (c >= 32 && c <= 126) {  // Printable ASCII
                editor_insert_char(editor, c);
            }
            break;
    }
    return true;  // Continue editor loop
}

// Main editor loop
void editor_main_loop(Editor* editor) {
    // Draw initial state
    editor_draw(editor);
    
    // Main loop
    while (1) {
        if (!editor_handle_input(editor)) {
            break;
        }
        editor_draw(editor);
    }
}

bool editor_load_file(Editor* editor, const char* filename) {
    // First clear existing content
    for (int i = 0; i < EDITOR_MAX_LINES; i++) {
        editor->lines[i][0] = '\0';
    }
    editor->num_lines = 1;
    editor->cursor_x = 0;
    editor->cursor_y = 0;
    editor->scroll_offset = 0;

    // Allocate buffer for file content
    char* buffer = (char*)kmalloc(4096);
    if (!buffer) return false;

    // Read file content
    int result = fat16_read_file(filename, buffer, 4096);
    if (result == 0) {  // File not found
        kfree(buffer);
        return false;
    }

    // If file is empty (result == -1), just return true with empty editor
    if (result == -1) {
        kfree(buffer);
        editor->filename = strdup(filename);
        editor->modified = false;
        return true;
    }

    // Parse file content into lines
    char* line_start = buffer;
    char* current = buffer;
    int line_count = 0;

    while (*current && line_count < EDITOR_MAX_LINES) {
        if (*current == '\n' || *current == '\0') {
            // Calculate line length
            int line_length = current - line_start;
            if (line_length >= EDITOR_MAX_LINE_LENGTH) {
                line_length = EDITOR_MAX_LINE_LENGTH - 1;
            }

            // Copy line content
            memcpy(editor->lines[line_count], line_start, line_length);
            editor->lines[line_count][line_length] = '\0';
            line_count++;

            // Move to next line
            line_start = current + 1;
        }
        current++;
    }

    // Set editor state
    editor->num_lines = line_count > 0 ? line_count : 1;
    editor->filename = strdup(filename);
    editor->modified = false;

    kfree(buffer);
    return true;
}

bool editor_save_file(Editor* editor, const char* filename) {
    if (!filename) {
        filename = editor->filename;
    }
    
    if (!filename) {
        return false;
    }

    // Calculate total size needed
    size_t total_size = 0;
    for (int i = 0; i < editor->num_lines; i++) {
        total_size += strlen(editor->lines[i]) + 1;  // +1 for newline
    }

    // Allocate buffer for file contents
    char* buffer = (char*)kmalloc(total_size + 1);
    if (!buffer) {
        return false;
    }

    // Combine all lines into buffer
    char* ptr = buffer;
    for (int i = 0; i < editor->num_lines; i++) {
        size_t line_len = strlen(editor->lines[i]);
        memcpy(ptr, editor->lines[i], line_len);
        ptr += line_len;
        *ptr++ = '\n';
    }
    *ptr = '\0';

    // Write to file
    bool success = fat16_write_file(filename, buffer, ptr - buffer);
    
    if (success) {
        editor->modified = false;
        if (!editor->filename) {
            editor->filename = strdup(filename);
        }
    }

    kfree(buffer);
    return success;
}

void editor_move_cursor(Editor* editor, int dx, int dy) {
    editor->cursor_x += dx;
    editor->cursor_y += dy;
    
    // Keep cursor within bounds
    if (editor->cursor_x < 0) editor->cursor_x = 0;
    if (editor->cursor_y < 0) editor->cursor_y = 0;
    if (editor->cursor_x >= VBE_WIDTH) editor->cursor_x = VBE_WIDTH - 1;
    if (editor->cursor_y >= VBE_HEIGHT - 1) editor->cursor_y = VBE_HEIGHT - 2;
}

void editor_insert_char(Editor* editor, char c) {
    // Ensure current line exists
    if (editor->cursor_y >= editor->num_lines) {
        editor->num_lines = editor->cursor_y + 1;
    }

    char* line = editor->lines[editor->cursor_y];
    int len = strlen(line);

    if (len < EDITOR_MAX_LINE_LENGTH - 1) {
        // Shift characters to the right to make space for the new char
        memmove(&line[editor->cursor_x + 1], &line[editor->cursor_x], len - editor->cursor_x + 1);

        // Insert the character
        line[editor->cursor_x] = c;
        editor->modified = true;

        // Move cursor forward
        editor->cursor_x++;

        // Compute visible screen row
        int display_y = editor->cursor_y - editor->scroll_offset;

        // Draw only if the line is on screen
        if (display_y >= 0 && display_y < EDITOR_EDITABLE_HEIGHT) {
            // Redraw the entire line from the modified position
            vbe_draw_string(LINE_NUMBER_GUTTER_WIDTH * 8, display_y * 16, &line[editor->cursor_x - 1], editor->text_color, &font_8x16);

            // Clear the rest of the line visually
            for (int i = strlen(line); i < VBE_WIDTH / 8 - LINE_NUMBER_GUTTER_WIDTH; i++) {
                vbe_draw_char((i + LINE_NUMBER_GUTTER_WIDTH) * 8, display_y * 16, ' ', editor->bg_color, &font_8x16);
            }
        }
    }
}

void editor_delete_char(Editor* editor) {
    if (editor->cursor_x > 0) {
        char* line = editor->lines[editor->cursor_y];
        int len = strlen(line);
        
        // Shift characters to the left
        memmove(&line[editor->cursor_x - 1], &line[editor->cursor_x], len - editor->cursor_x + 1);
        
        // Move cursor back
        editor->cursor_x--;
        editor->modified = true;
        
        // Compute visible screen row
        int display_y = editor->cursor_y - editor->scroll_offset;
        
        // Draw only if the line is on screen
        if (display_y >= 0 && display_y < EDITOR_EDITABLE_HEIGHT) {
            // Redraw the entire line from the modified position
            vbe_draw_string(LINE_NUMBER_GUTTER_WIDTH * 8, display_y * 16, &line[editor->cursor_x], editor->text_color, &font_8x16);
            
            // Clear the rest of the line visually
            for (int i = strlen(line); i < VBE_WIDTH / 8 - LINE_NUMBER_GUTTER_WIDTH; i++) {
                vbe_draw_char((i + LINE_NUMBER_GUTTER_WIDTH) * 8, display_y * 16, ' ', editor->bg_color, &font_8x16);
            }
        }
    }
}

void editor_new_line(Editor* editor) {
    if (editor->num_lines < EDITOR_MAX_LINES) {
        // Move all lines down
        for (int i = EDITOR_MAX_LINES - 1; i > editor->cursor_y; i--) {
            strcpy(editor->lines[i], editor->lines[i-1]);
        }
        
        // Clear the new line
        editor->lines[editor->cursor_y + 1][0] = '\0';
        
        // Move cursor to start of new line
        editor->cursor_x = 0;
        editor->cursor_y++;
        editor->num_lines++;
        editor->modified = true;
        
        // Add this line to handle scrolling after new line
        editor_adjust_scroll(editor);
    }
}

bool editor_create_file(const char* filename) {
    // Create and initialize editor
    Editor editor;
    editor_init(&editor);
    
    // Try to create the file
    if (fat16_create_file(filename, current_cluster)) {
        editor.filename = strdup(filename);
        editor.modified = true;
        return true;
    } else {
        vbe_draw_string(0, (VBE_HEIGHT / 16 - 1) * 16, "Failed to create file", 0xFFFFFFFF, &font_8x16);
        editor_free(&editor);
        return false;
    }
}

void editor_edit_command(const char* filename) {
    // Create and initialize editor
    Editor editor;
    editor_init(&editor);
    
    // Try to load the file if it exists
    if (!editor_load_file(&editor, filename)) {
        // If file doesn't exist, create a new one
        if (!fat16_create_file(filename, current_cluster)) {
            vbe_draw_string(0, (VBE_HEIGHT / 16 - 1) * 16, "Failed to create file", 0xFFFFFFFF, &font_8x16);
            editor_free(&editor);
            return;
        }
        editor.filename = strdup(filename);
        editor.modified = true;
    }
    
    // Start the editor
    editor_main_loop(&editor);
    
    // Clean up when editor exits
    editor_free(&editor);
    
    // Clear the screen and restore shell state
    vbe_clear_screen(0x000000);
    draw_header();
    
    // Reset cursor position to where the shell expects it
    vbe_cursor_x = 0;
    vbe_cursor_y = 125;  // Same as in shell_init
    draw_prompt();
}