#include "../include/shell.h"
#include "../include/vga.h"
#include "../include/string.h"
#include <stdint.h>

// Shell visual elements
#define PROMPT_COLOR VGA_COLOR_LIGHT_GREEN
#define TEXT_COLOR VGA_COLOR_WHITE
#define HEADER_COLOR VGA_COLOR_LIGHT_CYAN
#define BORDER_COLOR VGA_COLOR_LIGHT_BLUE

// Track the prompt position
size_t prompt_x = 0;
size_t prompt_y = 0;

void draw_header() {
    terminal_setcolor(HEADER_COLOR);
    terminal_set_cursor(2, 1);
    terminal_writestring("+------------------------------------------------------------------+");
    terminal_set_cursor(2, 2);
    terminal_writestring("|                                                                  |");
    terminal_set_cursor(2, 3);
    terminal_writestring("|                    Litago Operating System                       |");
    terminal_set_cursor(2, 4);
    terminal_writestring("|                                                                  |");
    terminal_set_cursor(2, 5);
    terminal_writestring("+------------------------------------------------------------------+");
    terminal_writestring("\n");
}

void draw_prompt() {
    terminal_setcolor(PROMPT_COLOR);
    terminal_writestring("\n[litago] ");
    terminal_setcolor(TEXT_COLOR);
    
    // Store the current cursor position after drawing the prompt
    terminal_get_cursor(&prompt_x, &prompt_y);
}

// Function to handle shell commands
static void handle_command(const char* command) {
    if (strcmp(command, "exit") == 0) {
        terminal_writestring("Exiting shell...\n");
    } else if (strcmp(command, "print") == 0) {
        terminal_writestring("Hello from LitagoDOS!\n");
    } else {
        terminal_writestring("Unknown command: ");
        terminal_writestring(command);
        terminal_writestring("\n");
    }
}

void shell_init() {
    terminal_clear();
    draw_header();
    draw_prompt();
}

void shell_run() {
    char command[256];
    int command_index = 0;

    while (1) {
        char c = terminal_getchar();
        
        if (c == '\n') {
            command[command_index] = '\0';
            terminal_putchar('\n');
            
            if (command_index > 0) {
                handle_command(command);
            }
            
            draw_prompt();
            command_index = 0;
        } else if (c == '\b') {
            if (command_index > 0) {
                command_index--;
                terminal_putchar('\b');
                terminal_putchar(' ');
                terminal_putchar('\b');
            }
        } else if (c != 0 && command_index < 255) {
            command[command_index++] = c;
            terminal_putchar(c);
        }
    }
}
