#include "../include/vga.h"
#include "../include/io.h"
#include "../include/keyboardDriver.h"
#include "../include/string.h"
#include "../include/ansi.h"
#include <stddef.h>
#include <stdint.h>

// Helper function for absolute value
static int abs(int x) {
    return (x < 0) ? -x : x;
}

// BIOS interrupt wrapper
static void bios_int(int interrupt, int ax, int bx, int cx, int dx) {
    __asm__ __volatile__(
        "int $0x10"
        : : "a"(ax), "b"(bx), "c"(cx), "d"(dx)
    );
}

// VGA text mode constants
static uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;

// VGA graphics mode memory
#define VGA_GRAPHICS_MEMORY ((uint8_t*)0xA0000)

// Current cursor position
size_t terminal_row;
size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

// Saved cursor position for ANSI escape sequences
static size_t saved_row;
static size_t saved_column;

// Input buffer for string input
#define INPUT_BUFFER_SIZE 256
static char input_buffer[INPUT_BUFFER_SIZE];
static size_t input_buffer_pos = 0;

// Current VGA mode
static enum vga_mode current_mode = VGA_MODE_TEXT;

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
    
    // Initialize ANSI support
    ansi_init();
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

// Save cursor position
void terminal_save_cursor(void) {
    saved_row = terminal_row;
    saved_column = terminal_column;
}

// Restore cursor position
void terminal_restore_cursor(void) {
    terminal_set_cursor(saved_column, saved_row);
}

// Set the terminal color
void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

// Get the current terminal color
uint8_t terminal_getcolor(void) {
    return terminal_color;
}

// Put a character at a specific position
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

// Scroll the terminal up by one line
void terminal_scroll(void) {
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
    // Process ANSI escape sequences
    if (ansi_is_enabled()) {
        ansi_process_char(c);
        return;
    }

    // Handle special characters
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
        return;
    } else if (c == '\r') {
        // Handle carriage return
        terminal_column = 0;
        terminal_update_cursor();
        return;
    }

    // Handle normal characters
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

// Write a string of a specific size
void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        terminal_putchar(data[i]);
    }
}

// Write a null-terminated string
void terminal_writestring(const char* data) {
    for (size_t i = 0; data[i] != '\0'; i++) {
        terminal_putchar(data[i]);
    }
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
    return keyboard_getchar();
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

// Set VGA mode
void vga_set_mode(enum vga_mode mode) {
    if (mode == current_mode) {
        return;  // Already in the requested mode
    }

    if (mode == VGA_MODE_TEXT) {
        // Switch to text mode (mode 3)
        outb(0x3D4, 0x00);  // Horizontal total
        outb(0x3D5, 0x5F);
        outb(0x3D4, 0x01);  // Horizontal display end
        outb(0x3D5, 0x4F);
        outb(0x3D4, 0x02);  // Start horizontal blanking
        outb(0x3D5, 0x50);
        outb(0x3D4, 0x03);  // End horizontal blanking
        outb(0x3D5, 0x82);
        outb(0x3D4, 0x04);  // Start horizontal retrace
        outb(0x3D5, 0x54);
        outb(0x3D4, 0x05);  // End horizontal retrace
        outb(0x3D5, 0x80);
        outb(0x3D4, 0x06);  // Vertical total
        outb(0x3D5, 0x0D);
        outb(0x3D4, 0x07);  // Overflow
        outb(0x3D5, 0x3E);
        outb(0x3D4, 0x08);  // Preset row scan
        outb(0x3D5, 0x00);
        outb(0x3D4, 0x09);  // Maximum scan line
        outb(0x3D5, 0x01);
        outb(0x3D4, 0x0A);  // Cursor start
        outb(0x3D5, 0x0F);
        outb(0x3D4, 0x0B);  // Cursor end
        outb(0x3D5, 0x0F);
        outb(0x3D4, 0x0C);  // Start address high
        outb(0x3D5, 0x00);
        outb(0x3D4, 0x0D);  // Start address low
        outb(0x3D5, 0x00);
        outb(0x3D4, 0x0E);  // Cursor location high
        outb(0x3D5, 0x00);
        outb(0x3D4, 0x0F);  // Cursor location low
        outb(0x3D5, 0x00);

        terminal_clear();
    } else if (mode == VGA_MODE_GRAPHICS) {
        // Switch to graphics mode (mode 13h)
        outb(0x3C2, 0x63);  // Set misc output register
        
        // Sequencer registers
        outb(0x3C4, 0x00);  // Reset
        outb(0x3C5, 0x03);
        outb(0x3C4, 0x01);  // Clocking mode
        outb(0x3C5, 0x01);
        outb(0x3C4, 0x04);  // Memory mode
        outb(0x3C5, 0x0E);
        
        // Graphics Controller registers
        outb(0x3CE, 0x05);  // Mode
        outb(0x3CF, 0x00);
        outb(0x3CE, 0x06);  // Miscellaneous
        outb(0x3CF, 0x05);
        
        // CRT Controller registers
        outb(0x3D4, 0x00);  // Horizontal total
        outb(0x3D5, 0x5F);
        outb(0x3D4, 0x01);  // Horizontal display end
        outb(0x3D5, 0x4F);
        outb(0x3D4, 0x02);  // Start horizontal blanking
        outb(0x3D5, 0x50);
        outb(0x3D4, 0x03);  // End horizontal blanking
        outb(0x3D5, 0x82);
        outb(0x3D4, 0x04);  // Start horizontal retrace
        outb(0x3D5, 0x54);
        outb(0x3D4, 0x05);  // End horizontal retrace
        outb(0x3D5, 0x80);
        outb(0x3D4, 0x06);  // Vertical total
        outb(0x3D5, 0x0D);
        outb(0x3D4, 0x07);  // Overflow
        outb(0x3D5, 0x3E);
        outb(0x3D4, 0x08);  // Preset row scan
        outb(0x3D5, 0x00);
        outb(0x3D4, 0x09);  // Maximum scan line
        outb(0x3D5, 0x41);
        
        // Disable cursor
        outb(0x3D4, 0x0A);
        outb(0x3D5, 0x20);
        
        // Clear graphics screen
        vga_clear_screen(0);
    }

    current_mode = mode;
}

// Plot a pixel in graphics mode
void vga_plot_pixel(int x, int y, uint8_t color) {
    if (current_mode != VGA_MODE_GRAPHICS) {
        return;
    }
    if (x < 0 || x >= VGA_GRAPHICS_WIDTH || y < 0 || y >= VGA_GRAPHICS_HEIGHT) {
        return;
    }
    VGA_GRAPHICS_MEMORY[y * VGA_GRAPHICS_WIDTH + x] = color;
}

// Clear the graphics screen
void vga_clear_screen(uint8_t color) {
    if (current_mode != VGA_MODE_GRAPHICS) {
        return;
    }
    for (int i = 0; i < VGA_GRAPHICS_WIDTH * VGA_GRAPHICS_HEIGHT; i++) {
        VGA_GRAPHICS_MEMORY[i] = color;
    }
}

// Draw a line in graphics mode using Bresenham's algorithm
void vga_draw_line(int x0, int y0, int x1, int y1, uint8_t color) {
    if (current_mode != VGA_MODE_GRAPHICS) {
        return;
    }

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        vga_plot_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

// Draw a rectangle outline
void vga_draw_rect(int x, int y, int width, int height, uint8_t color) {
    if (current_mode != VGA_MODE_GRAPHICS) {
        return;
    }
    vga_draw_line(x, y, x + width - 1, y, color);
    vga_draw_line(x, y + height - 1, x + width - 1, y + height - 1, color);
    vga_draw_line(x, y, x, y + height - 1, color);
    vga_draw_line(x + width - 1, y, x + width - 1, y + height - 1, color);
}

// Fill a rectangle
void vga_fill_rect(int x, int y, int width, int height, uint8_t color) {
    if (current_mode != VGA_MODE_GRAPHICS) {
        return;
    }
    for (int i = y; i < y + height; i++) {
        for (int j = x; j < x + width; j++) {
            vga_plot_pixel(j, i, color);
        }
    }
} 