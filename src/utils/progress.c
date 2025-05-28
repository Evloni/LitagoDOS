#include "../../include/utils/progress.h"
#include "../../include/vbe.h"
#include "../../include/font_8x16.h"
#include "../../include/string.h"
#include <stddef.h>

// External delay function from kernel
extern void delay_animation(int dots);

void show_progress_bar(int width, int steps) {
    // Get screen dimensions
    int screen_width = vbe_get_width();
    int screen_height = vbe_get_height();
    
    // Calculate progress bar dimensions
    int bar_width = width * 8;  // Each character is 8 pixels wide
    int bar_height = 20;        // Height of the progress bar
    int bar_x = (screen_width - bar_width) / 2;  // Center horizontally
    int bar_y = (screen_height - bar_height) / 2;  // Center vertically
    
    // Draw background
    vbe_fill_rect(bar_x, bar_y, bar_width, bar_height, 0x333333);  // Dark gray background
    
    // Draw progress bar
    for (int i = 0; i < steps; i++) {
        int progress = (i + 1) * bar_width / steps;
        vbe_fill_rect(bar_x, bar_y, progress, bar_height, 0x00FF00);  // Green progress
        
        // Draw percentage text
        char percent_str[8];
        int percent = (i + 1) * 100 / steps;
        itoa(percent, percent_str, 10);
        strcat(percent_str, "%");
        
        // Calculate text position to center it
        int text_x = bar_x + (bar_width - strlen(percent_str) * 8) / 2;
        int text_y = bar_y + (bar_height - 16) / 2;  // Center vertically
        
        // Draw percentage text
        vbe_draw_string(text_x, text_y, percent_str, 0xFFFFFF, &font_8x16);
        
        // Use kernel's delay animation
        delay_animation(1);  // Add a small delay between updates
    }
} 