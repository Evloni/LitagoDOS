#ifndef SHELL_H
#define SHELL_H

#include <stddef.h>  // For size_t definition

// Prompt position variables
extern size_t prompt_x;
extern size_t prompt_y;

void shell_init();
void shell_run();

#endif // SHELL_H
