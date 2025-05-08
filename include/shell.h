#ifndef SHELL_H
#define SHELL_H

#include <stddef.h>  // For size_t definition
#include <stdbool.h>

// Prompt position variables
extern size_t prompt_x;
extern size_t prompt_y;

// Initialize the shell
void shell_init(void);

// Run the shell command processing loop
void shell_run(void);

// Command handlers
bool cmd_help(int argc, char* argv[]);
bool cmd_clear(int argc, char* argv[]);
bool cmd_echo(int argc, char* argv[]);
bool cmd_memtest(int argc, char* argv[]);

#endif // SHELL_H
