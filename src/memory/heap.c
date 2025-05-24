#include "../../include/memory/heap.h"
#include "../../include/memory/pmm.h"
#include <stddef.h>
#include <stdint.h>

// Simple heap implementation using physical memory pages
#define HEAP_START 0x2000000  // Start at 32MB
#define HEAP_SIZE 0x1000000   // 16MB heap size

static uint32_t heap_ptr = HEAP_START;

void heap_init(void) {
    heap_ptr = HEAP_START;
}

void* kmalloc(size_t size) {
    // Round up to nearest page size
    size_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    
    // Allocate pages
    void* ptr = heap_ptr;
    heap_ptr += pages * PAGE_SIZE;
    
    // Check if we've exceeded heap size
    if (heap_ptr > HEAP_START + HEAP_SIZE) {
        return NULL;  // Out of memory
    }
    
    return ptr;
}

void kfree(void* ptr) {
    // In this simple implementation, we don't actually free memory
    // The heap just grows and never shrinks
    (void)ptr;  // Silence unused parameter warning
} 