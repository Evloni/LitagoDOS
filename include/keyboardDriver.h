#ifndef KEYBOARD_DRIVER_H
#define KEYBOARD_DRIVER_H

#include "system.h"
#include <stdbool.h>
#include <stdint.h>

// Keyboard ports
#define KEYBOARD_DATA_PORT    0x60
#define KEYBOARD_STATUS_PORT  0x64
#define KEYBOARD_COMMAND_PORT 0x64

// Function declarations
bool keyboard_init(void);
bool keyboard_shutdown(void);
void keyboard_handler(struct regs *r);
void init_keyboard(void);
char get_scancode(void);
char keyboard_getchar(void);
int keyboard_getkey(void);

// Special key code definitions
#define KEY_ESCAPE       0x1B
#define KEY_BACKSPACE    0x08
#define KEY_TAB         0x09
#define KEY_ENTER       0x0A
#define KEY_CTRL_L      0x1B
#define KEY_CTRL_R      0x1C
#define KEY_SHIFT_L     0x1D
#define KEY_SHIFT_R     0x1E
#define KEY_ALT_L       0x1F
#define KEY_ALT_R       0x20
#define KEY_CAPS_LOCK   0x21
#define KEY_F1          0x22
#define KEY_F2          0x23
#define KEY_F3          0x24
#define KEY_F4          0x25
#define KEY_F5          0x26
#define KEY_F6          0x27
#define KEY_F7          0x28
#define KEY_F8          0x29
#define KEY_F9          0x2A
#define KEY_F10         0x2B
#define KEY_F11         0x2C
#define KEY_F12         0x2D
#define KEY_NUM_LOCK    0x2E
#define KEY_SCROLL_LOCK 0x2F
#define KEY_HOME        0x30
#define KEY_UP          0x31
#define KEY_PAGE_UP     0x32
#define KEY_LEFT        0x33
#define KEY_RIGHT       0x34
#define KEY_END         0x35
#define KEY_DOWN        0x36
#define KEY_PAGE_DOWN   0x37
#define KEY_INSERT      0x38
#define KEY_DELETE      0x39

#endif // KEYBOARD_DRIVER_H 