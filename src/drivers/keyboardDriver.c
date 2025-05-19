#include "../../include/keyboardDriver.h"
#include "../../include/io.h"
#include "../../include/vga.h"
#include "../../include/idt.h"
#include "../../include/string.h"
#include "../../include/system.h"
#include <stddef.h>

// External prompt position variables
extern size_t prompt_x;
extern size_t prompt_y;

// Basic key codes
#define KEY_ESCAPE    0x1B
#define KEY_BACKSPACE 0x08
#define KEY_TAB       0x09
#define KEY_ENTER     0x0A

// Keyboard buffer
#define KEYBOARD_BUFFER_SIZE 256
static char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static size_t keyboard_buffer_head = 0;
static size_t keyboard_buffer_tail = 0;

// Simple modifier state
struct modifier_state modifier_state = {0};

// Standard PC keyboard scancode set 1 mapping (unshifted)
static const char scancode_to_ascii[] = {
    0,  0,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',  // 0x00-0x0E
    '\t','q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',      // 0x0F-0x1C
    0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',            // 0x1D-0x29
    0,  '\\','z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*',          // 0x2A-0x37
    0,  ' ', 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,             // 0x38-0x45
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,             // 0x46-0x53
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0              // 0x54-0x61
};

// Standard PC keyboard scancode set 1 mapping (shifted)
static const char scancode_to_ascii_shift[] = {
    0,  0,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',  // 0x00-0x0E
    '\t','Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',      // 0x0F-0x1C
    0,  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',            // 0x1D-0x29
    0,  '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*',          // 0x2A-0x37
    0,  ' ', 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,             // 0x38-0x45
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,             // 0x46-0x53
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0              // 0x54-0x61
};

// Add a character to the keyboard buffer
static void keyboard_buffer_add(char c) {
    size_t next_head = (keyboard_buffer_head + 1) % KEYBOARD_BUFFER_SIZE;
    if (next_head != keyboard_buffer_tail) {
        keyboard_buffer[keyboard_buffer_head] = c;
        keyboard_buffer_head = next_head;
    }
}

// Get a character from the keyboard buffer
static char keyboard_buffer_get(void) {
    if (keyboard_buffer_head == keyboard_buffer_tail) {
        return 0;  // Buffer is empty
    }
    char c = keyboard_buffer[keyboard_buffer_tail];
    keyboard_buffer_tail = (keyboard_buffer_tail + 1) % KEYBOARD_BUFFER_SIZE;
    return c;
}

// Clear the keyboard buffer
void keyboard_clear_buffer(void) {
    keyboard_buffer_head = keyboard_buffer_tail;
}

// Get a character from keyboard input
char keyboard_getchar(void) {
    while (1) {
        char c = keyboard_buffer_get();
        if (c != 0) {
            return c;
        }
        __asm__("hlt");
    }
}

bool keyboard_init(void) {
    // Wait for keyboard controller to be ready
    while (inb(KEYBOARD_STATUS_PORT) & 0x02);
    
    // Enable keyboard
    outb(KEYBOARD_COMMAND_PORT, 0xAE);
    
    // Enable keyboard interrupt (IRQ1)
    uint8_t pic_mask = inb(0x21);
    pic_mask &= ~(1 << 1);  // Clear bit 1 to enable keyboard interrupt
    outb(0x21, pic_mask);
    
    return true;
}

void keyboard_handler(struct regs *r) {
    uint8_t scancode = inb(0x60);
    
    // Only process key presses (scancode < 0x80)
    if (scancode < 0x80) {
        // Handle modifier keys
        switch (scancode) {
            case 0x2A: modifier_state.shift = true; break;  // Left shift
            case 0x36: modifier_state.shift = true; break;  // Right shift
            case 0x1D: modifier_state.ctrl = true; break;   // Left control
            case 0x38: modifier_state.alt = true; break;    // Left alt
            case 0x48: keyboard_buffer_add('\033'); keyboard_buffer_add('['); keyboard_buffer_add('A'); break;  // Up arrow
            case 0x50: keyboard_buffer_add('\033'); keyboard_buffer_add('['); keyboard_buffer_add('B'); break;  // Down arrow
            case 0x4D: keyboard_buffer_add('\033'); keyboard_buffer_add('['); keyboard_buffer_add('C'); break;  // Right arrow
            case 0x4B: keyboard_buffer_add('\033'); keyboard_buffer_add('['); keyboard_buffer_add('D'); break;  // Left arrow
            default: {
                // Only process scancodes within our mapping range
                if (scancode < sizeof(scancode_to_ascii)) {
                    char c = modifier_state.shift ? 
                        scancode_to_ascii_shift[scancode] : 
                        scancode_to_ascii[scancode];
                    
                    // Only add valid ASCII characters to the buffer
                    if (c != 0 && ((c >= 32 && c <= 126) || c == '\b' || c == '\t' || c == '\n')) {
                        keyboard_buffer_add(c);
                    }
                }
                break;
            }
        }
    } else {
        // Handle key release
        scancode &= 0x7F;
        switch (scancode) {
            case 0x2A: modifier_state.shift = false; break;  // Left shift
            case 0x36: modifier_state.shift = false; break;  // Right shift
            case 0x1D: modifier_state.ctrl = false; break;   // Left control
            case 0x38: modifier_state.alt = false; break;    // Left alt
        }
    }
    
    // Send EOI to PIC
    outb(0x20, 0x20);
}

char get_scancode(void) {
    return inb(0x60);
}

int keyboard_getkey(void) {
    while (1) {
        uint8_t scancode = inb(0x60);
        if (scancode < 0x80) {  // Key press
            return scancode;
        }
        __asm__("hlt");
    }
}