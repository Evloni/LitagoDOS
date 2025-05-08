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

// External variable for shift state
extern int shift_pressed;

#endif // KEYBOARD_DRIVER_H 