#include "../../include/utils/progress.h"
#include "../../include/drivers/vbe.h"
#include "../../include/font_8x16.h"
#include "../../include/string.h"
#include "../../include/PSF1_parser/psf1_parser.h"
#include <stddef.h>


void show_progress_bar(int width, int steps) {
    
    // Get screen dimensions
    int screen_width = vbe_get_width();
    int screen_height = vbe_get_height();
    
    // Calculate progress bar dimensions
    int bar_width = width;  // Width in characters
    int bar_height = 3;     // Height in characters
    int bar_x = (screen_width / 8 - bar_width - 6) / 2;  // Center horizontally, accounting for percentage text
    int bar_y = (screen_height / 16 - bar_height) / 2;  // Center vertically
    
    // Draw progress bar
    for (int i = 0; i < steps; i++) {
        // Calculate progress
        int progress = (i + 1) * bar_width / steps;
        
        // Draw the progress bar frame
        for (int y = 0; y < bar_height; y++) {
            for (int x = 0; x < bar_width; x++) {
                char c;
                if (y == 0 || y == bar_height - 1) {
                    // Top and bottom borders
                    c = '─';
                } else if (x == 0) {
                    // Left border
                    c = '│';
                } else if (x == bar_width - 1) {
                    // Right border
                    c = '│';
                } else if (x < progress) {
                    // Progress fill
                    c = '█';
                } else {
                    // Empty space
                    c = '░';
                }
            }
        }
  
        
        // Use kernel's delay animation
    }
}

