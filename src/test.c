#include "../include/vga.h"
#include "../include/memory/pmm.h"
#include "../include/memory/heap.h"
#include "../include/memory/program.h"
#include <stddef.h>
#include <stdbool.h>

void test_memory_management() {
    terminal_setcolor(VGA_COLOR_MAGENTA);
    terminal_writestring("\n!!! THIS IS MEMTEST2: Advanced Memory Management Test !!!\n");
    terminal_setcolor(VGA_COLOR_WHITE);

    // Test PMM
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
        terminal_setcolor(VGA_COLOR_RED);
        terminal_writestring("✗ Failed to allocate physical pages\n");
        terminal_setcolor(VGA_COLOR_WHITE);
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
        terminal_setcolor(VGA_COLOR_RED);
        terminal_writestring("✗ Failed to allocate heap memory\n");
        terminal_setcolor(VGA_COLOR_WHITE);
    }

    // Test memory writing
    terminal_writestring("\nTesting Memory Writing:\n");
    
    // Allocate a page and write to it
    void* test_page = pmm_alloc_page();
    if (test_page) {
        // Write some test data
        uint32_t* data = (uint32_t*)test_page;
        for (int i = 0; i < 1024; i++) {
            data[i] = i;
        }
        
        // Verify the data
        bool success = true;
        for (int i = 0; i < 1024; i++) {
            if (data[i] != i) {
                success = false;
                break;
            }
        }
        
        if (success) {
            terminal_writestring("✓ Successfully wrote and verified memory\n");
        } else {
            terminal_setcolor(VGA_COLOR_RED);
            terminal_writestring("✗ Memory verification failed\n");
            terminal_setcolor(VGA_COLOR_WHITE);
        }
        
        pmm_free_page(test_page);
    }

    terminal_setcolor(VGA_COLOR_CYAN);
    terminal_writestring("\n=== Memory Management Test Complete ===\n\n");
    terminal_setcolor(VGA_COLOR_WHITE);
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