#include "../include/vga.h"
#include "../include/io.h"
#include <stddef.h>

// VGA text mode constants
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;

// Current cursor position
static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

// Initialize terminal interface
void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = VGA_MEMORY;
    
    // Clear the screen
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
    
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
static void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

// Put a character at the current position
void terminal_putchar(char c) {
    if (c == '\n') {
        // Handle newline
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_row = 0;
        }
    } else {
        terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
        if (++terminal_column == VGA_WIDTH) {
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT) {
                terminal_row = 0;
            }
        }
    }
    terminal_update_cursor();
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

// Initialize VGA
void vga_init(void) {
    terminal_initialize();
} 