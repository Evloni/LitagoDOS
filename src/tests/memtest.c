#include "../../include/memory/pmm.h"
#include "../../include/drivers/vbe.h"
#include "../../include/string.h"
#include <stddef.h>
#include <stdbool.h>

// Memory test patterns
static const uint32_t test_patterns[] = {
    0x00000000,  // All zeros
    0xFFFFFFFF,  // All ones
    0x55555555,  // Alternating 0/1
    0xAAAAAAAA,  // Alternating 1/0
    0x12345678,  // Random pattern
    0x87654321,  // Random pattern
    0x0000FFFF,  // 16-bit patterns
    0xFFFF0000,
    0x00FF00FF,
    0xFF00FF00
};

#define NUM_PATTERNS (sizeof(test_patterns) / sizeof(test_patterns[0]))

static bool test_memory_block(uint32_t* start, size_t size, uint32_t pattern) {
    // Write pattern
    for (size_t i = 0; i < size / sizeof(uint32_t); i++) {
        start[i] = pattern;
    }

    // Verify pattern
    for (size_t i = 0; i < size / sizeof(uint32_t); i++) {
        if (start[i] != pattern) {
            return false;
        }
    }

    return true;
}

void memtest_run(void) {
    terminal_writestring("Starting memory test...\n");
    
    // Test physical memory allocation
    void* page1 = pmm_alloc_page();
    void* page2 = pmm_alloc_page();
    void* page3 = pmm_alloc_page();
    
    if (page1 && page2 && page3) {
        terminal_writestring("Physical memory allocation successful\n");
        
        // Test memory writing
        uint32_t* ptr1 = (uint32_t*)page1;
        uint32_t* ptr2 = (uint32_t*)page2;
        uint32_t* ptr3 = (uint32_t*)page3;
        
        // Write test patterns
        for (int i = 0; i < 1024; i++) {
            ptr1[i] = 0xAAAAAAAA;
            ptr2[i] = 0x55555555;
            ptr3[i] = 0x12345678;
        }
        
        // Verify patterns
        bool success = true;
        for (int i = 0; i < 1024; i++) {
            if (ptr1[i] != 0xAAAAAAAA || ptr2[i] != 0x55555555 || ptr3[i] != 0x12345678) {
                success = false;
                break;
            }
        }
        
        if (success) {
            terminal_writestring("Memory write/read test successful\n");
        } else {
            terminal_writestring("Memory write/read test failed\n");
        }
        
        // Free memory
        pmm_free_page(page1);
        pmm_free_page(page2);
        pmm_free_page(page3);
        terminal_writestring("Memory freed successfully\n");
    } else {
        terminal_writestring("Physical memory allocation failed\n");
    }
    
    // Test heap memory
    void* heap1 = malloc(1024);
    void* heap2 = malloc(2048);
    void* heap3 = malloc(4096);
    
    if (heap1 && heap2 && heap3) {
        terminal_writestring("Heap memory allocation successful\n");
        
        // Free heap memory
        free(heap1);
        free(heap2);
        free(heap3);
        terminal_writestring("Heap memory freed successfully\n");
    } else {
        terminal_writestring("Heap memory allocation failed\n");
    }
    
    terminal_writestring("Memory test complete\n");
} 