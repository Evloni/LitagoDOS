#include "../../include/tests/memtest.h"
#include "../../include/memory/pmm.h"
#include "../../include/vga.h"
#include <stdint.h>

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
    terminal_setcolor(VGA_COLOR_YELLOW);
    terminal_writestring("Starting memory test...\n");
    terminal_setcolor(VGA_COLOR_WHITE);

    // Print memory status before allocation
    terminal_writestring("Memory status before test:\n");
    terminal_writestring("Total pages: ");
    char num_str[20];
    int i = 0;
    size_t total = pmm_get_total_pages();
    do {
        num_str[i++] = '0' + (total % 10);
        total /= 10;
    } while (total > 0);
    while (--i >= 0) {
        terminal_putchar(num_str[i]);
    }
    terminal_writestring("\n");

    terminal_writestring("Free pages: ");
    i = 0;
    size_t free = pmm_get_free_pages();
    do {
        num_str[i++] = '0' + (free % 10);
        free /= 10;
    } while (free > 0);
    while (--i >= 0) {
        terminal_putchar(num_str[i]);
    }
    terminal_writestring("\n\n");

    // Get a block of memory to test
    void* test_block = pmm_alloc_page();  // Allocate a single page
    if (!test_block) {
        terminal_setcolor(VGA_COLOR_RED);
        terminal_writestring("Failed to allocate memory for testing\n");
        terminal_writestring("No free pages available\n");
        terminal_setcolor(VGA_COLOR_WHITE);
        return;
    }

    terminal_writestring("Successfully allocated page at address: 0x");
    uint32_t addr = (uint32_t)test_block;
    for (int j = 7; j >= 0; j--) {
        char c = "0123456789ABCDEF"[(addr >> (j * 4)) & 0xF];
        terminal_putchar(c);
    }
    terminal_writestring("\n\n");

    uint32_t* test_addr = (uint32_t*)test_block;
    bool all_passed = true;

    // Test each pattern
    for (size_t i = 0; i < NUM_PATTERNS; i++) {
        terminal_writestring("Testing pattern 0x");
        // Print pattern in hex
        uint32_t pattern = test_patterns[i];
        for (int j = 7; j >= 0; j--) {
            char c = "0123456789ABCDEF"[(pattern >> (j * 4)) & 0xF];
            terminal_putchar(c);
        }
        terminal_writestring("... ");

        if (test_memory_block(test_addr, 4096, pattern)) {  // Test one page (4KB)
            terminal_setcolor(VGA_COLOR_GREEN);
            terminal_writestring("PASSED\n");
        } else {
            terminal_setcolor(VGA_COLOR_RED);
            terminal_writestring("FAILED\n");
            all_passed = false;
        }
        terminal_setcolor(VGA_COLOR_WHITE);
    }

    // Free the test block
    pmm_free_page(test_block);

    // Print final result
    terminal_writestring("\nMemory test ");
    if (all_passed) {
        terminal_setcolor(VGA_COLOR_GREEN);
        terminal_writestring("COMPLETED SUCCESSFULLY\n");
    } else {
        terminal_setcolor(VGA_COLOR_RED);
        terminal_writestring("FAILED\n");
    }
    terminal_setcolor(VGA_COLOR_WHITE);
} 