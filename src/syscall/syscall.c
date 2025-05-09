#include "../include/syscall/syscall.h"
#include "../include/vga.h"
#include "../include/memory/pmm.h"
#include "../include/io.h"
#include <stddef.h>
#include "../include/idt.h"
#include "../include/keyboardDriver.h"
// Add after the includes
extern void syscall_entry(void);

// Helper to print a byte as two hex digits (for kernel diagnostics)
static void kernel_print_byte_hex(uint8_t value) {
    const char *hex_chars = "0123456789ABCDEF";
    // Assuming terminal_putchar is available and works at this stage
    terminal_putchar(hex_chars[(value >> 4) & 0x0F]);
    terminal_putchar(hex_chars[value & 0x0F]);
}

// Syscall handler function
void syscall_handler(struct regs *r) {
    switch (r->eax) {
        case SYSCALL_WRITE:
            // Write to screen
            terminal_writestring((char*)r->ebx);
            r->eax = 0; // Return success
            break;
            
        case SYSCALL_READ:
            // Read from keyboard 
            {
                char c_kernel = 0;
                while ((c_kernel = keyboard_getchar()) == 0) {
                    asm volatile ("sti"); // Enable interrupts
                    asm volatile ("hlt"); // Wait for an interrupt (e.g., keyboard)
                    asm volatile ("cli"); // Disable interrupts again before checking buffer
                }
                r->eax = c_kernel; // Return the character
            }
            break;
            
        case SYSCALL_OPEN:
            // Open a file (implement when you have file system)
            r->eax = -1; // Return error for now
            break;
            
        case SYSCALL_CLOSE:
            // Close a file (implement when you have file system)
            r->eax = -1; // Return error for now
            break;
            
        case SYSCALL_EXIT:
            // Exit program (implement when you have process management)
            r->eax = 0;
            break;
            
        case SYSCALL_MALLOC:
            // Allocate memory
            r->eax = (uint32_t)pmm_alloc_page();
            break;
            
        case SYSCALL_FREE:
            // Free memory
            pmm_free_page((void*)r->ebx);
            r->eax = 0;
            break;
            
        default:
            r->eax = -1; // Invalid syscall
            break;
    }
}

// Initialize syscall system
void syscall_init(void) {
    // Set up syscall entry point in IDT
    idt_set_gate(0x80, (uint32_t)syscall_entry, 0x08, 0xEE); // Present, Ring 3, 32-bit Interrupt Gate
}
