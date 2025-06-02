#include "../../include/drivers/vbe.h"
#include "../../include/font_8x16.h"
#include "../../include/version.h"
#include "../../include/shell.h"
#include <stdint.h>

// Animation colors
#define LOGO_COLOR 0xFF00FFFF  // Cyan
#define COW_COLOR 0xFFFFFFFF   // White
#define TEXT_COLOR 0xFFFFFFFF  // White
#define BAR_COLOR 0xFF00FF00   // Green
#define BAR_BG_COLOR 0xFF333333 // Dark gray

// ASCII art logo
static const char* logo[] = {
    " _     _ _                    ",
    "| |   (_) |_ __ _  __ _  ___  ",
    "| |   | | __/ _` |/ _` |/ _ \\ ",
    "| |___| | || (_| | (_| | (_) |",
    "|_____|_|\\__\\__,_|\\__, |\\___/ ",
    "                  |___/       ",
    "                             ",
    "     Operating System        "
};

// ASCII art cow frames
static const char* cow_frames[] = {
    // Frame 1 - Normal
    "    ^__^         ",
    "    (oo)\\_______ ",
    "    (__)\\       )\\/\\",
    "        ||----w |",
    "        ||     ||",
    "                ",
    "                ",
    "                ",
    
    // Frame 2 - Starting to moo
    "    ^__^         ",
    "    (oo)\\_______ ",
    "    (__)\\       )\\/\\",
    "        ||----w |",
    "        ||     ||",
    "        M...    ",
    "                ",
    "                ",
    
    // Frame 3 - Mooing
    "    ^__^         ",
    "    (oo)\\_______ ",
    "    (__)\\       )\\/\\",
    "        ||----w |",
    "        ||     ||",
    "        Moo!    ",
    "                ",
    "                ",
    
    // Frame 4 - Full moo
    "    ^__^         ",
    "    (oo)\\_______ ",
    "    (__)\\       )\\/\\",
    "        ||----w |",
    "        ||     ||",
    "        MOO!    ",
    "                ",
    "                ",
    
    // Frame 5 - Mooing
    "    ^__^         ",
    "    (oo)\\_______ ",
    "    (__)\\       )\\/\\",
    "        ||----w |",
    "        ||     ||",
    "        Moo!    ",
    "                ",
    "                ",
    
    // Frame 6 - Ending moo
    "    ^__^         ",
    "    (oo)\\_______ ",
    "    (__)\\       )\\/\\",
    "        ||----w |",
    "        ||     ||",
    "        M...    ",
    "                ",
    "                ",
    
    // Frame 7 - Blinking
    "    ^__^         ",
    "    (oo)\\_______ ",
    "    (__)\\       )\\/\\",
    "        ||----w |",
    "        ||     ||",
    "                ",
    "                ",
    "                ",
    
    // Frame 8 - Back to normal
    "    ^__^         ",
    "    (oo)\\_______ ",
    "    (__)\\       )\\/\\",
    "        ||----w |",
    "        ||     ||",
    "                ",
    "                ",
    "                "
};

// Draw the logo with a fade-in effect
static void draw_logo(int y_offset) {
    for (int i = 0; i < 8; i++) {
        vbe_draw_string(32, y_offset + (i * 16), logo[i], LOGO_COLOR, &font_8x16);
    }
}

// Draw the animated cow
static void draw_cow(int x, int y, int frame) {
    for (int i = 0; i < 8; i++) {
        vbe_draw_string(x, y + (i * 16), cow_frames[frame * 8 + i], COW_COLOR, &font_8x16);
    }
}

// Draw a loading bar with animation
static void draw_loading_bar(int x, int y, int width, int height, int progress) {
    // Draw background
    vbe_draw_rect(x, y, width, height, BAR_BG_COLOR);
    
    // Draw progress
    int progress_width = (width * progress) / 100;
    vbe_draw_rect(x, y, progress_width, height, BAR_COLOR);
}

// Draw version information
static void draw_version_info(int y_offset) {
    char version_str[64];
    snprintf(version_str, sizeof(version_str), "Version %s", VERSION_STRING);
    vbe_draw_string(32, y_offset, version_str, TEXT_COLOR, &font_8x16);
    
    char build_str[64];
    snprintf(build_str, sizeof(build_str), "Build: %s %s", BUILD_DATE, BUILD_TIME);
    vbe_draw_string(32, y_offset + 16, build_str, TEXT_COLOR, &font_8x16);
}

// Main boot animation function
void show_boot_animation(void) {
    // Clear screen with black background
    vbe_clear_screen(0x00000000);
    
    // Calculate center position
    int center_y = (VBE_HEIGHT - (8 * 16)) / 2;  // 8 lines of logo * 16px height
    
    // Draw logo
    draw_logo(center_y - 64);
    
    // Animate the cow
    int cow_x = 400;  // Position the cow to the right of the logo
    int current_frame = 0;
    
    // Draw version info below logo
    draw_version_info(center_y + 64);
    
    // Draw loading bar
    int bar_width = 400;
    int bar_height = 20;
    int bar_x = (VBE_WIDTH - bar_width) / 2;
    int bar_y = center_y + 96;
    
    // Animate loading bar and cow
    for (int i = 0; i <= 100; i += 2) {
        // Clear previous cow frame
        vbe_draw_rect(cow_x, center_y - 64, 200, 128, 0x00000000);
        
        // Draw new cow frame
        draw_cow(cow_x, center_y - 64, current_frame);
        current_frame = (current_frame + 1) % 8;  // Cycle through 8 frames instead of 4
        
        // Draw loading bar
        draw_loading_bar(bar_x, bar_y, bar_width, bar_height, i);
        
        // Add a small delay
        for (volatile int j = 0; j < 10000000; j++);
    }
    
    // Add a final delay before continuing
    for (volatile int j = 0; j < 500000000; j++);
    shell_start();
} 