#include "../../include/utils/progress.h"
#include "../../include/vga.h"
#include "../../include/string.h"
#include <stddef.h>

void show_progress_bar(int width, int steps) {
    terminal_writestring("\x1B[?25l");  // Hide cursor
    
    terminal_writestring("\x1B[37m");  // white color and start bracket
    
    for (int i = 0; i < width; i++) {
        char block[2] = {219, '\0'}; // Full block
        terminal_writestring(block);
        // Simulate some work
        for (volatile int j = 0; j < 10000000; j++);
    }
    
    terminal_writestring("\x1B[0m\n");  // End bracket and reset color
    terminal_writestring("\x1B[?25h");  // Show cursor
} 