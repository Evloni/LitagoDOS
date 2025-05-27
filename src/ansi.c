#include "../include/ansi.h"
#include "../include/vga.h"
#include "../include/string.h"
#include <stddef.h>
#include <stdint.h>

// Global ANSI context
static ansi_context_t ansi_ctx = {
    .state = ANSI_STATE_NORMAL,
    .param_count = 0,
    .param_pos = 0,
    .enabled = true,
    .attributes = 0,
    .fg_color = VGA_COLOR_LIGHT_GREY,
    .bg_color = VGA_COLOR_BLACK
};

// ANSI to VGA color mapping table
static const uint8_t ansi_to_vga[8] = {
    VGA_COLOR_BLACK,    // 30
    VGA_COLOR_RED,      // 31
    VGA_COLOR_GREEN,    // 32
    VGA_COLOR_BROWN,    // 33 (yellow)
    VGA_COLOR_BLUE,     // 34
    VGA_COLOR_MAGENTA,  // 35
    VGA_COLOR_CYAN,     // 36
    VGA_COLOR_LIGHT_GREY// 37 (white)
};

// Initialize ANSI support
void ansi_init(void) {
    ansi_reset_state();
    ansi_ctx.enabled = true;
    ansi_ctx.attributes = 0;
    ansi_ctx.fg_color = VGA_COLOR_LIGHT_GREY;
    ansi_ctx.bg_color = VGA_COLOR_BLACK;
    terminal_setcolor(vga_entry_color(ansi_ctx.fg_color, ansi_ctx.bg_color));
}

// Process a single character
void ansi_process_char(char c) {
    if (!ansi_ctx.enabled) {
        terminal_putchar(c);
        return;
    }

    switch (ansi_ctx.state) {
        case ANSI_STATE_NORMAL:
            if (c == '\x1B') {  // ESC
                ansi_ctx.state = ANSI_STATE_ESCAPE;
            } else if (c == '\n') {
                terminal_column = 0;
                if (++terminal_row == VGA_HEIGHT) {
                    terminal_scroll();
                    terminal_row = VGA_HEIGHT - 1;
                }
                terminal_update_cursor();
            } else if (c == '\r') {
                terminal_column = 0;
                terminal_update_cursor();
            } else if ((unsigned char)c < 0x20) {
                // Ignore other control characters
            } else {
                terminal_putentryat(c, vga_entry_color(ansi_ctx.fg_color, ansi_ctx.bg_color), terminal_column, terminal_row);
                if (++terminal_column == VGA_WIDTH) {
                    terminal_column = 0;
                    if (++terminal_row == VGA_HEIGHT) {
                        terminal_scroll();
                        terminal_row = VGA_HEIGHT - 1;
                    }
                }
                terminal_update_cursor();
            }
            break;

        case ANSI_STATE_ESCAPE:
            if (c == '[') {
                ansi_ctx.state = ANSI_STATE_BRACKET;
                ansi_ctx.param_count = 0;
                ansi_ctx.param_pos = 0;
                memset(ansi_ctx.params, 0, sizeof(ansi_ctx.params)); // Clear params buffer
            } else {
                // Invalid sequence, print ESC and this character
                terminal_putchar('\x1B');
                terminal_putchar(c);
                ansi_ctx.state = ANSI_STATE_NORMAL;
            }
            break;

        case ANSI_STATE_BRACKET:
            if (c >= '0' && c <= '9') {
                ansi_ctx.state = ANSI_STATE_PARAMETER;
                ansi_ctx.params[ansi_ctx.param_pos++] = c;
            } else if (c == ';') {
                ansi_ctx.params[ansi_ctx.param_pos++] = c;
            } else {
                // Store the command character separately
                char cmd = c;
                ansi_ctx.params[ansi_ctx.param_pos] = '\0';
                ansi_handle_escape_sequence(ansi_ctx.params, ansi_ctx.param_pos, cmd);
                ansi_ctx.state = ANSI_STATE_NORMAL;
                ansi_ctx.param_pos = 0; // Reset param_pos after handling
                memset(ansi_ctx.params, 0, sizeof(ansi_ctx.params)); // Clear params buffer
            }
            break;

        case ANSI_STATE_PARAMETER:
            if (c >= '0' && c <= '9') {
                ansi_ctx.params[ansi_ctx.param_pos++] = c;
            } else if (c == ';') {
                ansi_ctx.params[ansi_ctx.param_pos++] = c;
            } else {
                // Store the command character separately
                char cmd = c;
                ansi_ctx.params[ansi_ctx.param_pos] = '\0';
                ansi_handle_escape_sequence(ansi_ctx.params, ansi_ctx.param_pos, cmd);
                ansi_ctx.state = ANSI_STATE_NORMAL;
                ansi_ctx.param_pos = 0; // Reset param_pos after handling
                memset(ansi_ctx.params, 0, sizeof(ansi_ctx.params)); // Clear params buffer
            }
            break;
    }
}

// Process a string of characters
void ansi_process_string(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        ansi_process_char(str[i]);
    }
}

// Reset ANSI state
void ansi_reset_state(void) {
    ansi_ctx.state = ANSI_STATE_NORMAL;
    ansi_ctx.param_count = 0;
    ansi_ctx.param_pos = 0;
}

// Set ANSI support enabled/disabled
void ansi_set_enabled(bool enabled) {
    ansi_ctx.enabled = enabled;
}

// Check if ANSI support is enabled
bool ansi_is_enabled(void) {
    return ansi_ctx.enabled;
}

// Handle escape sequence
void ansi_handle_escape_sequence(const char* params, size_t count, char cmd) {
    if (count == 0) return;

    // Parse parameters
    int values[16] = {0};
    int value_count = 0;
    int current_value = 0;

    // Parse the parameters
    for (size_t i = 0; i < count; i++) {
        if (params[i] >= '0' && params[i] <= '9') {
            current_value = current_value * 10 + (params[i] - '0');
        } else if (params[i] == ';') {
            values[value_count++] = current_value;
            current_value = 0;
        }
    }
    values[value_count++] = current_value;

    // Handle the command
    switch (cmd) {
        case 'A':  // Move cursor up
            if (value_count > 0 && values[0] > 0) {
                int steps = values[0];
                if (terminal_row >= steps) {
                    terminal_row -= steps;
                } else {
                    terminal_row = 0;
                }
                terminal_update_cursor();
            }
            break;

        case 'B':  // Move cursor down
            if (value_count > 0 && values[0] > 0) {
                int steps = values[0];
                if (terminal_row + steps < VGA_HEIGHT) {
                    terminal_row += steps;
                } else {
                    terminal_row = VGA_HEIGHT - 1;
                }
                terminal_update_cursor();
            }
            break;

        case 'C':  // Move cursor right
            if (value_count > 0 && values[0] > 0) {
                int steps = values[0];
                if (terminal_column + steps < VGA_WIDTH) {
                    terminal_column += steps;
                } else {
                    terminal_column = VGA_WIDTH - 1;
                }
                terminal_update_cursor();
            }
            break;

        case 'D':  // Move cursor left
            if (value_count > 0 && values[0] > 0) {
                int steps = values[0];
                if (terminal_column >= steps) {
                    terminal_column -= steps;
                } else {
                    terminal_column = 0;
                }
                terminal_update_cursor();
            }
            break;

        case 'm':  // Set attributes
            for (int i = 0; i < value_count; i++) {
                int value = values[i];
                if (value == 0) {
                    // Reset all attributes
                    ansi_ctx.attributes = 0;
                    ansi_ctx.fg_color = VGA_COLOR_LIGHT_GREY;
                    ansi_ctx.bg_color = VGA_COLOR_BLACK;
                } else if (value >= 30 && value <= 37) {
                    // Set foreground color using lookup table
                    ansi_set_foreground_color(ansi_to_vga[value - 30]);
                } else if (value >= 40 && value <= 47) {
                    // Set background color using lookup table
                    ansi_set_background_color(ansi_to_vga[value - 40]);
                } else if (value == 1) {
                    // Bold
                    ansi_set_attribute(ANSI_ATTR_BOLD);
                } else if (value == 2) {
                    // Dim (use a darker color if possible)
                    ansi_set_attribute(ANSI_ATTR_DIM);
                } else if (value == 4) {
                    // Underline (simulate by using cyan if not already cyan)
                    ansi_set_attribute(ANSI_ATTR_UNDERLINE);
                } else if (value == 5) {
                    // Blink (not implemented, see comment)
                    ansi_set_attribute(ANSI_ATTR_BLINK);
                } else if (value == 7) {
                    // Reverse
                    ansi_set_attribute(ANSI_ATTR_REVERSE);
                } else if (value == 8) {
                    // Hidden
                    ansi_set_attribute(ANSI_ATTR_HIDDEN);
                }
            }
            // Apply attributes to color
            uint8_t fg = ansi_ctx.fg_color;
            uint8_t bg = ansi_ctx.bg_color;
            if (ansi_ctx.attributes & ANSI_ATTR_BOLD) {
                if (fg < 8) fg += 8; // Use bright color for bold
            }
            if (ansi_ctx.attributes & ANSI_ATTR_DIM) {
                if (fg >= 8 && fg <= 15) fg -= 8; // Use normal color for dim
            }
            if (ansi_ctx.attributes & ANSI_ATTR_UNDERLINE) {
                // Simulate underline by using cyan if not already cyan
                if (fg != VGA_COLOR_CYAN && fg != VGA_COLOR_LIGHT_CYAN) fg = VGA_COLOR_CYAN;
            }
            if (ansi_ctx.attributes & ANSI_ATTR_HIDDEN) {
                fg = bg; // Foreground = background
            }
            // (Blink is not implemented)
            terminal_setcolor(vga_entry_color(fg, bg));
            break;

        case 'H':  // Set cursor position
            if (value_count >= 2) {
                ansi_move_cursor(values[1] - 1, values[0] - 1);
            }
            break;

        case 'J':  // Clear screen
            ansi_clear_screen();
            break;

        case 'K':  // Clear line
            ansi_clear_line();
            break;

        case 's':  // Save cursor position
            ansi_save_cursor();
            break;

        case 'u':  // Restore cursor position
            ansi_restore_cursor();
            break;

        case 'E':  // Next line
            if (value_count > 0 && values[0] > 0) {
                int steps = values[0];
                if (terminal_row + steps < VGA_HEIGHT) {
                    terminal_row += steps;
                } else {
                    terminal_row = VGA_HEIGHT - 1;
                }
                terminal_column = 0;
                terminal_update_cursor();
            }
            break;

        case 'F':  // Previous line
            if (value_count > 0 && values[0] > 0) {
                int steps = values[0];
                if (terminal_row >= steps) {
                    terminal_row -= steps;
                } else {
                    terminal_row = 0;
                }
                terminal_column = 0;
                terminal_update_cursor();
            }
            break;

        case 'G':  // Horizontal position (column)
            if (value_count > 0 && values[0] > 0) {
                int col = values[0] - 1; // 1-based to 0-based
                if (col < VGA_WIDTH) {
                    terminal_column = col;
                } else {
                    terminal_column = VGA_WIDTH - 1;
                }
                terminal_update_cursor();
            }
            break;
    }
}

// Set foreground color
void ansi_set_foreground_color(uint8_t color) {
    ansi_ctx.fg_color = color;
}

// Set background color
void ansi_set_background_color(uint8_t color) {
    ansi_ctx.bg_color = color;
}

// Set text attribute
void ansi_set_attribute(uint8_t attribute) {
    ansi_ctx.attributes |= attribute;
}

// Clear text attribute
void ansi_clear_attribute(uint8_t attribute) {
    ansi_ctx.attributes &= ~attribute;
}

// Move cursor
void ansi_move_cursor(int x, int y) {
    terminal_set_cursor(x, y);
}

// Clear screen
void ansi_clear_screen(void) {
    terminal_clear();
}

// Clear line
void ansi_clear_line(void) {
    size_t x, y;
    terminal_get_cursor(&x, &y);
    for (size_t i = x; i < VGA_WIDTH; i++) {
        terminal_putentryat(' ', terminal_getcolor(), i, y);
    }
    terminal_set_cursor(x, y);
}

// Save cursor position
void ansi_save_cursor(void) {
    terminal_save_cursor();
}

// Restore cursor position
void ansi_restore_cursor(void) {
    terminal_restore_cursor();
} 