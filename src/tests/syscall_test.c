#include "../../include/syscall/syscall.h"
#include "../../include/drivers/vbe.h"
#include "../../include/string.h"
#include <stddef.h>

// Helper function to make syscalls
static inline int syscall(int num, int arg1, int arg2, int arg3) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a" (ret)
        : "a" (num), "b" (arg1), "c" (arg2), "d" (arg3)
        : "memory"
    );
    return ret;
}

void test_syscalls(void) {
    terminal_writestring("Testing system calls...\n");
    
    // Test syscall 0 (print string)
    const char* test_str = "Hello from syscall!\n";
    asm volatile("int $0x80" : : "a"(0), "b"(test_str));
    
    // Test syscall 1 (get key)
    terminal_writestring("Press any key to continue...\n");
    int key = 0;
    asm volatile("int $0x80" : "=a"(key) : "a"(1));
    
    // Test syscall 2 (clear screen)
    asm volatile("int $0x80" : : "a"(2));
    
    terminal_writestring("System call test complete!\n");
} 