#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

// Structure to hold register values during interrupt
struct regs {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} __attribute__((packed));

/* System-level operations */
static inline void cli(void) {
    asm volatile("cli");
}

static inline void sti(void) {
    asm volatile("sti");
}

#endif /* SYSTEM_H */ 