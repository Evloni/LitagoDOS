#include "../include/syscall/syscall.h"
#include "../include/drivers/vbe.h"
#include "../include/string.h"
#include <stddef.h>
#include "../include/idt.h"
#include "../include/memory/pmm.h"
#include "../include/io.h"
// Add after the includes
extern void syscall_entry(void);
// Syscall handler function
void syscall_handler(struct regs *r) {
    switch (r->eax) {
        case SYSCALL_WRITE:
            // Write to screen
            terminal_writestring((char*)r->ebx);
            r->eax = 0; // Return success
            break;
            
        case SYSCALL_READ:
            // Read from keyboard (implement this based on your keyboard driver)
            // For now, just return 0
            r->eax = 0;
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
