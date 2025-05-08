#include "../../include/keyboardDriver.h"
#include "../../include/io.h"
#include "../../include/driver.h"
#include "../../include/vga.h"
#include "../../include/idt.h"
#include "../../include/string.h"
#include "../../include/system.h"
#include <stddef.h>

// External prompt position variables
extern size_t prompt_x;
extern size_t prompt_y;

// Helper to print a byte as two hex digits
static void print_hex(uint8_t value) {
    const char *hex = "0123456789ABCDEF";
    terminal_putchar(hex[(value >> 4) & 0x0F]);
    terminal_putchar(hex[value & 0x0F]);
}

// Scancode to ASCII mapping table (US layout)
static const char scancode_to_ascii[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0
};

// Scancode to ASCII mapping table for shift key (US layout)
static const char scancode_to_ascii_shift[] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0
};

// Shift key state
int shift_pressed = 0;

bool keyboard_init(void) {
    // Wait for keyboard controller to be ready
    while (inb(KEYBOARD_STATUS_PORT) & 0x02);
    
    // Enable keyboard
    outb(KEYBOARD_COMMAND_PORT, 0xAE);
    
    // Wait for keyboard controller to be ready
    while (inb(KEYBOARD_STATUS_PORT) & 0x02);
    
    // Read command byte
    outb(KEYBOARD_COMMAND_PORT, 0x20);
    while (inb(KEYBOARD_STATUS_PORT) & 0x01 == 0);
    uint8_t command_byte = inb(KEYBOARD_DATA_PORT);
    
    // Enable keyboard interrupt
    command_byte |= 0x01;
    
    // Write command byte back
    outb(KEYBOARD_COMMAND_PORT, 0x60);
    while (inb(KEYBOARD_STATUS_PORT) & 0x02);
    outb(KEYBOARD_DATA_PORT, command_byte);
    
    // Enable keyboard
    outb(KEYBOARD_COMMAND_PORT, 0xAE);
    
    return true;
}

bool keyboard_shutdown(void) {
    // Disable keyboard interrupt
    outb(0x21, inb(0x21) | 0x02);
    return true;
}

void init_keyboard(void) {
    // Register the keyboard driver
    if (!register_driver("keyboard", keyboard_init, keyboard_shutdown, NULL)) {
        // If we can't register the driver, we can't continue
        while(1) { __asm__("hlt"); }
    }

    // Enable keyboard interrupt (IRQ1)
    uint8_t pic_mask = inb(0x21);
    pic_mask &= ~(1 << 1);  // Clear bit 1 to enable keyboard interrupt
    outb(0x21, pic_mask);
}

void keyboard_handler(struct regs *r) {
    uint8_t scancode = inb(0x60);
    
    // Check if it's a key press (scancode < 0x80) or release (scancode >= 0x80)
    if (scancode < 0x80) {
        // Handle special keys
        if (scancode == 0x2A) {  // Left shift
            shift_pressed = 1;
            return;
        }
    } else {
        // Handle key release
        if (scancode == 0xAA) {  // Left shift release
            shift_pressed = 0;
        }
    }
    
    // Send EOI to PIC
    outb(0x20, 0x20);
}

char get_scancode(void) {
    return inb(0x60);
}