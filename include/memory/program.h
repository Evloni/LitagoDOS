#ifndef _PROGRAM_H
#define _PROGRAM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Program entry point type
typedef void (*program_entry_t)(void);

// Program structure
struct program {
    void* code_segment;      // Code segment address
    void* data_segment;      // Data segment address
    size_t code_size;        // Size of code segment
    size_t data_size;        // Size of data segment
    program_entry_t entry;   // Entry point
};

// Load a program from memory
bool program_load(void* code_addr, size_t code_size, void* data_addr, size_t data_size, program_entry_t entry, struct program* prog);

// Unload a program
void program_unload(struct program* prog);

// Execute a program
bool program_execute(struct program* prog);

#endif // _PROGRAM_H 