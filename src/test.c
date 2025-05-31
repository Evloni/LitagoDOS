#include "../include/memory/pmm.h"
#include "../include/memory/heap.h"
#include "../include/memory/program.h"
#include "../include/drivers/vbe.h"
#include <stddef.h>
#include <stdbool.h>

void test_memory_management() {
    terminal_writestring("!!! THIS IS MEMTEST2: Advanced Memory Management Test !!!\n");
    terminal_writestring("\nTesting Physical Memory Manager (PMM):\n");
    
    // Allocate some physical pages
    void* page1 = pmm_alloc_page();
    void* page2 = pmm_alloc_page();
    void* page3 = pmm_alloc_page();

    if (page1 && page2 && page3) {
        terminal_writestring("✓ Successfully allocated 3 physical pages\n");
        
        // Free the pages
        pmm_free_page(page1);
        pmm_free_page(page2);
        pmm_free_page(page3);
        terminal_writestring("✓ Successfully freed 3 physical pages\n");
    } else {
        terminal_writestring("✗ Failed to allocate physical pages\n");
    }

    // Test Heap
    terminal_writestring("\nTesting Heap Memory Management:\n");
    
    // Allocate some heap memory
    void* heap1 = malloc(1024);  // 1KB
    void* heap2 = malloc(2048);  // 2KB
    void* heap3 = malloc(4096);  // 4KB

    if (heap1 && heap2 && heap3) {
        terminal_writestring("✓ Successfully allocated heap memory\n");
        
        // Free the heap memory
        free(heap1);
        free(heap2);
        free(heap3);
        terminal_writestring("✓ Successfully freed heap memory\n");
    } else {
        terminal_writestring("✗ Failed to allocate heap memory\n");
    }

    // Test memory writing
    terminal_writestring("\nTesting Memory Writing:\n");
    
    // Allocate a page and write to it
    void* test_page = pmm_alloc_page();
    if (test_page) {
        // Write a pattern to the page
        uint32_t* ptr = (uint32_t*)test_page;
        for (int i = 0; i < 1024; i++) {
            ptr[i] = 0xDEADBEEF;
        }
        
        // Verify the pattern
        bool success = true;
        for (int i = 0; i < 1024; i++) {
            if (ptr[i] != 0xDEADBEEF) {
                success = false;
                break;
            }
        }
        
        if (success) {
            terminal_writestring("✓ Successfully wrote and verified memory pattern\n");
        } else {
            terminal_writestring("✗ Memory verification failed\n");
        }
        
        pmm_free_page(test_page);
    } else {
        terminal_writestring("✗ Failed to allocate test page\n");
    }
}

void my_test_program(void) {
    terminal_writestring("Hello from loaded program!\n");
}

void test_program_loading(void) {
    struct program prog;
    // For this test, we don't need a data segment
    // We'll use a fixed code size (e.g., 128 bytes)
    if (program_load((void*)my_test_program, 128, NULL, 0, my_test_program, &prog)) {
        terminal_writestring("Program loaded successfully.\n");
        program_execute(&prog);
        program_unload(&prog);
        terminal_writestring("Program unloaded.\n");
    } else {
        terminal_writestring("Program loading failed.\n");
    }
}

void run_tests() {
    // ... existing code ...
    test_memory_management();
    test_program_loading();
    // ... existing code ...
} 