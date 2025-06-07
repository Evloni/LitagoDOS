#ifndef PROGRESS_H
#define PROGRESS_H

#include <stdint.h>

// Function to show a progress bar
void show_progress_bar(int width, int steps);

// Font management functions
void init_progress_font(void);
void cleanup_progress_font(void);

#endif // PROGRESS_H 