#include "../../include/memory/pmm.h"
#include "../../include/memory/memory_map.h"
#include "../../include/vga.h"
#include <stddef.h>
#include <stdint.h>

// Bitmap for tracking physical memory pages
static uint32_t* bitmap = NULL;
static size_t bitmap_size = 0;
static size_t total_pages = 0;
static size_t free_pages = 0;

// Set a bit in the bitmap
static void bitmap_set(size_t bit) {
    bitmap[bit / 32] |= (1 << (bit % 32));
}

// Clear a bit in the bitmap
static void bitmap_clear(size_t bit) {
    bitmap[bit / 32] &= ~(1 << (bit % 32));
}

// Test if a bit is set
static int bitmap_test(size_t bit) {
    return bitmap[bit / 32] & (1 << (bit % 32));
}

// Find the first free bit in the bitmap
static size_t bitmap_first_free(void) {
    for (size_t i = 0; i < bitmap_size; i++) {
        if (bitmap[i] != 0xFFFFFFFF) {
            for (size_t j = 0; j < 32; j++) {
                if (!(bitmap[i] & (1 << j))) {
                    return i * 32 + j;
                }
            }
        }
    }
    return (size_t)-1; // No free pages
}

void pmm_init(void) {
    // Get total memory from memory map
    uint64_t total_memory = memory_map_get_total_memory();
    
    // Calculate total number of pages (4KB each)
    total_pages = total_memory / 4096;
    free_pages = total_pages;
    
    // Calculate bitmap size (1 bit per page, 32 bits per uint32_t)
    bitmap_size = (total_pages + 31) / 32;
    
    // Allocate bitmap at the end of the first 1MB
    bitmap = (uint32_t*)0x100000;  // 1MB mark
    
    // Initialize bitmap to all zeros (all pages free)
    for (size_t i = 0; i < bitmap_size; i++) {
        bitmap[i] = 0;
    }
    
    // Mark the first 1MB as used (where kernel and bitmap reside)
    for (size_t i = 0; i < 256; i++) {  // 1MB / 4KB = 256 pages
        bitmap[i / 32] |= (1 << (i % 32));
        free_pages--;
    }
    
    // Print memory information
    terminal_writestring("\nMemory Information:\n");
    terminal_writestring("Total Memory: ");
    
    // Convert to MB for display
    uint64_t total_mb = total_memory / (1024 * 1024);
    char mb_str[20];
    int i = 0;
    
    // Convert to string
    do {
        mb_str[i++] = '0' + (total_mb % 10);
        total_mb /= 10;
    } while (total_mb > 0);
    
    // Print in reverse
    while (--i >= 0) {
        terminal_putchar(mb_str[i]);
    }
    terminal_writestring(" MB\n");
    
    terminal_writestring("Total Pages: ");
    // Convert total_pages to string
    i = 0;
    size_t pages = total_pages;
    do {
        mb_str[i++] = '0' + (pages % 10);
        pages /= 10;
    } while (pages > 0);
    
    // Print in reverse
    while (--i >= 0) {
        terminal_putchar(mb_str[i]);
    }
    terminal_writestring("\n");
    
    terminal_writestring("Free Pages: ");
    // Convert free_pages to string
    i = 0;
    pages = free_pages;
    do {
        mb_str[i++] = '0' + (pages % 10);
        pages /= 10;
    } while (pages > 0);
    
    // Print in reverse
    while (--i >= 0) {
        terminal_putchar(mb_str[i]);
    }
    terminal_writestring("\n\n");
}

void* pmm_alloc_page(void) {
    if (free_pages == 0) {
        return NULL; // No free pages
    }
    
    size_t page = bitmap_first_free();
    if (page == (size_t)-1) {
        return NULL;
    }
    
    bitmap_set(page);
    free_pages--;
    
    return (void*)(page * PAGE_SIZE);
}

void pmm_free_page(void* page) {
    if (page == NULL) {
        return;
    }
    
    size_t page_num = (size_t)page / PAGE_SIZE;
    if (page_num >= total_pages) {
        return; // Invalid page
    }
    
    bitmap_clear(page_num);
    free_pages++;
}

size_t pmm_get_total_pages(void) {
    return total_pages;
}

size_t pmm_get_free_pages(void) {
    return free_pages;
} 