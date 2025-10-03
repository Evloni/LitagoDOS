#ifndef SHELL_H
#define SHELL_H

#include <stddef.h>  // For size_t definition
#include <stdbool.h>
#include <stdint.h>

// Prompt position variables
extern size_t prompt_x;
extern size_t prompt_y;

// Current directory cluster
extern uint16_t current_cluster;

// Initialize the shell
void shell_init(void);

// Start the shell (initializes and runs the main loop)
void shell_start(void);

// Run the shell command processing loop
void shell_run(void);

// Command handlers
bool cmd_help(int argc, char* argv[]);
bool cmd_clear(int argc, char* argv[]);
bool cmd_echo(int argc, char* argv[]);
bool cmd_memtest(int argc, char* argv[]);
bool cmd_syscall_test(int argc, char* argv[]);

#endif // SHELL_H
