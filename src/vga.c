#include "../include/vga.h"
#include "../include/io.h"
#include "../include/keyboardDriver.h"
#include "../include/string.h"
#include <stddef.h>
#include <stdint.h>

// VGA text mode constants
static uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;

// Current cursor position
static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

// Input buffer for string input
#define INPUT_BUFFER_SIZE 256
static char input_buffer[INPUT_BUFFER_SIZE];
static size_t input_buffer_pos = 0;

// Initialize terminal interface
void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = VGA_MEMORY;
    
    // Set cursor size (smaller cursor)
    outb(0x3D4, 0x0A);  // Cursor start register
    outb(0x3D5, 0x0F);  // Start scan line 15
    outb(0x3D4, 0x0B);  // Cursor end register
    outb(0x3D5, 0x0F);  // End scan line 15 (same as start = single line cursor)
    
    // Move cursor to start position
    terminal_set_cursor(0, 0);
}

// Set cursor position
void terminal_set_cursor(size_t x, size_t y) {
    uint16_t pos = y * VGA_WIDTH + x;
    
    // Update cursor position
    outb(0x3D4, 0x0F);  // Low byte
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);  // High byte
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
    
    terminal_row = y;
    terminal_column = x;
}

// Get cursor position
void terminal_get_cursor(size_t* x, size_t* y) {
    *x = terminal_column;
    *y = terminal_row;
}

// Set the terminal color
void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

// Put a character at a specific position
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

// Scroll the terminal up by one line
static void terminal_scroll(void) {
    // Move all lines up by one
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            const size_t next_index = (y + 1) * VGA_WIDTH + x;
            terminal_buffer[index] = terminal_buffer[next_index];
        }
    }
    
    // Clear the last line
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        const size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
}

// Put a character at the current position
void terminal_putchar(char c) {
    if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
            terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
        }
        terminal_update_cursor();
        return;
    } else if (c == '\n') {
        // Handle newline
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
            terminal_row = VGA_HEIGHT - 1;
        }
        terminal_update_cursor();
    } else {
        terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
        if (++terminal_column == VGA_WIDTH) {
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT) {
                terminal_scroll();
                terminal_row = VGA_HEIGHT - 1;
            }
        }
        terminal_update_cursor();
    }
}

// Write a string of a specific size
void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

// Write a null-terminated string
void terminal_writestring(const char* data) {
    for (size_t i = 0; data[i] != '\0'; i++)
        terminal_putchar(data[i]);
}

// Update the cursor position
void terminal_update_cursor(void) {
    uint16_t pos = terminal_row * VGA_WIDTH + terminal_column;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

// Clear the terminal screen
void terminal_clear(void) {
    // Fill the screen with spaces
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
    
    // Reset cursor position
    terminal_row = 0;
    terminal_column = 0;
    terminal_update_cursor();
}

// Get a single character from keyboard input
char terminal_getchar(void) {
    char c;
    while (1) {
        // Wait for a key press
        while (!(inb(0x64) & 1));
        
        // Read the scancode
        uint8_t scancode = inb(0x60);
        
        // Check if it's a key press (scancode < 0x80)
        if (scancode < 0x80) {
            // Convert scancode to ASCII (simplified version)
            switch (scancode) {
                case 0x1C: return '\n';  // Enter
                case 0x0E: return '\b';  // Backspace
                default:
                    if (scancode >= 0x02 && scancode <= 0x0D) {
                        return "1234567890-="[scancode - 0x02];
                    } else if (scancode >= 0x10 && scancode <= 0x1B) {
                        return shift_pressed ? "QWERTYUIOP[]"[scancode - 0x10] : "qwertyuiop[]"[scancode - 0x10];
                    } else if (scancode >= 0x1E && scancode <= 0x28) {
                        return shift_pressed ? "ASDFGHJKL;'"[scancode - 0x1E] : "asdfghjkl;'"[scancode - 0x1E];
                    } else if (scancode >= 0x2C && scancode <= 0x35) {
                        return shift_pressed ? "ZXCVBNM,./"[scancode - 0x2C] : "zxcvbnm,./"[scancode - 0x2C];
                    } else if (scancode == 0x39) {
                        return ' ';  // Space
                    }
            }
        }
    }
}

// Get a string from keyboard input
void terminal_getstring(char* buffer, size_t max_length) {
    size_t pos = 0;
    
    while (pos < max_length - 1) {
        char c = terminal_getchar();
        
        if (c == '\n') {
            // End of input
            buffer[pos] = '\0';
            terminal_putchar('\n');
            return;
        } else if (c == '\b') {
            // Handle backspace
            if (pos > 0) {
                pos--;
                terminal_putchar('\b');
                terminal_putchar(' ');
                terminal_putchar('\b');
            }
        } else if (c >= 32 && c <= 126) {
            // Printable character
            buffer[pos++] = c;
            terminal_putchar(c);
        }
    }
    
    // Ensure string is null-terminated
    buffer[pos] = '\0';
}

// Helper function to write an integer to the terminal
void terminal_writeint(int num) {
    char buf[32];
    itoa(num, buf, 10);
    terminal_writestring(buf);
}

// Helper function to write a hex number to the terminal
void terminal_writehex(uint32_t num) {
    char buf[32];
    char hex_chars[] = "0123456789ABCDEF";
    int i = 0;
    
    // Convert to hex string
    do {
        buf[i++] = hex_chars[num & 0xF];
        num >>= 4;
    } while (num > 0);
    
    // Add "0x" prefix
    buf[i++] = 'x';
    buf[i++] = '0';
    
    // Reverse the string
    for (int j = 0; j < i/2; j++) {
        char temp = buf[j];
        buf[j] = buf[i-j-1];
        buf[i-j-1] = temp;
    }
    
    buf[i] = '\0';
    terminal_writestring(buf);
}

// Initialize VGA
void vga_init(void) {
    terminal_initialize();
} 