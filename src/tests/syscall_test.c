#include "../../include/syscall/syscall.h"
#include "../../include/tests/syscall_test.h"
#include "../../include/system.h"
#include "../../include/vga.h"

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
    // Test SYSCALL_WRITE
    terminal_writestring("Starting syscall tests...\n");
    
    const char* message = "Testing SYSCALL_WRITE...\n";
    int write_result = syscall(SYSCALL_WRITE, (int)message, 0, 0);
    terminal_writestring("SYSCALL_WRITE result: ");
    terminal_writeint(write_result);
    terminal_writestring("\n");

    // Test SYSCALL_MALLOC
    terminal_writestring("Testing SYSCALL_MALLOC...\n");
    void* ptr = (void*)syscall(SYSCALL_MALLOC, 0, 0, 0);
    if (ptr) {
        terminal_writestring("Memory allocated at: ");
        terminal_writehex((uint32_t)ptr);
        terminal_writestring("\n");
        
        // Test SYSCALL_FREE
        terminal_writestring("Testing SYSCALL_FREE...\n");
        int free_result = syscall(SYSCALL_FREE, (int)ptr, 0, 0);
        terminal_writestring("SYSCALL_FREE result: ");
        terminal_writeint(free_result);
        terminal_writestring("\n");
    } else {
        terminal_writestring("Memory allocation failed!\n");
    }
    terminal_setcolor(VGA_COLOR_LIGHT_GREEN);
    terminal_writestring("All syscall tests completed.\n");
    terminal_setcolor(VGA_COLOR_WHITE);
} 