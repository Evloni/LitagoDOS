#ifndef ANSI_H
#define ANSI_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "vga.h"

// ANSI escape sequence states
typedef enum {
    ANSI_STATE_NORMAL,
    ANSI_STATE_ESCAPE,
    ANSI_STATE_BRACKET,
    ANSI_STATE_PARAMETER
} ansi_state_t;

// ANSI text attributes
#define ANSI_ATTR_RESET       0
#define ANSI_ATTR_BOLD        1
#define ANSI_ATTR_DIM         2
#define ANSI_ATTR_UNDERLINE   4
#define ANSI_ATTR_BLINK       5
#define ANSI_ATTR_REVERSE     7
#define ANSI_ATTR_HIDDEN      8

// ANSI foreground colors
#define ANSI_FG_BLACK        30
#define ANSI_FG_RED          31
#define ANSI_FG_GREEN        32
#define ANSI_FG_YELLOW       33
#define ANSI_FG_BLUE         34
#define ANSI_FG_MAGENTA      35
#define ANSI_FG_CYAN         36
#define ANSI_FG_WHITE        37
#define ANSI_FG_DEFAULT      39

// ANSI background colors
#define ANSI_BG_BLACK        40
#define ANSI_BG_RED          41
#define ANSI_BG_GREEN        42
#define ANSI_BG_YELLOW       43
#define ANSI_BG_BLUE         44
#define ANSI_BG_MAGENTA      45
#define ANSI_BG_CYAN         46
#define ANSI_BG_WHITE        47
#define ANSI_BG_DEFAULT      49

// ANSI cursor movement
#define ANSI_CURSOR_UP        'A'
#define ANSI_CURSOR_DOWN      'B'
#define ANSI_CURSOR_FORWARD   'C'
#define ANSI_CURSOR_BACKWARD  'D'
#define ANSI_CURSOR_NEXT_LINE 'E'
#define ANSI_CURSOR_PREV_LINE 'F'
#define ANSI_CURSOR_HORIZONTAL 'G'
#define ANSI_CURSOR_POSITION  'H'
#define ANSI_CURSOR_SAVE      's'
#define ANSI_CURSOR_RESTORE   'u'

// ANSI screen control
#define ANSI_CLEAR_SCREEN     'J'
#define ANSI_CLEAR_LINE       'K'
#define ANSI_SCROLL_UP        'S'
#define ANSI_SCROLL_DOWN      'T'

// ANSI parser context
typedef struct {
    ansi_state_t state;
    char params[16];  // Buffer for parameters
    size_t param_count;
    size_t param_pos;
    bool enabled;     // Add enabled flag
    uint8_t attributes; // Current text attributes
    uint8_t fg_color;  // Current foreground color
    uint8_t bg_color;  // Current background color
} ansi_context_t;

// Function declarations
void ansi_init(void);
void ansi_process_char(char c);
void ansi_process_string(const char* str);
void ansi_reset_state(void);
void ansi_set_enabled(bool enabled);
bool ansi_is_enabled(void);

// Internal functions
void ansi_handle_escape_sequence(const char* params, size_t count, char cmd);
void ansi_set_foreground_color(uint8_t color);
void ansi_set_background_color(uint8_t color);
void ansi_set_attribute(uint8_t attribute);
void ansi_clear_attribute(uint8_t attribute);
void ansi_move_cursor(int x, int y);
void ansi_clear_screen(void);
void ansi_clear_line(void);
void ansi_save_cursor(void);
void ansi_restore_cursor(void);

#endif // ANSI_H 