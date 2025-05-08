#ifndef SYSCALL_H
#define SYSCALL_H

#include "../system.h"
#include <stdint.h>

// Syscall numbers
#define SYSCALL_WRITE    0
#define SYSCALL_READ     1
#define SYSCALL_OPEN     2
#define SYSCALL_CLOSE    3
#define SYSCALL_EXIT     4
#define SYSCALL_MALLOC   5
#define SYSCALL_FREE     6

// Syscall structure to hold parameters
struct syscall_params {
    uint32_t eax;    // Syscall number
    uint32_t ebx;    // Parameter 1
    uint32_t ecx;    // Parameter 2
    uint32_t edx;    // Parameter 3
    uint32_t esi;    // Parameter 4
    uint32_t edi;    // Parameter 5
};

// Function declarations
void syscall_init(void);
void syscall_handler(struct regs *r);

#endif // SYSCALL_H
