#include "../../include/memory/pmm.h"
#include "../../include/memory/memory_map.h"
#include "../../include/vga.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Bitmap for tracking physical memory pages
static uint32_t* bitmap = NULL;
static size_t bitmap_size = 0;
static size_t total_pages = 0;
static size_t free_pages = 0;
static uint32_t last_allocated_page = 0;

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
    // Start from the last allocated page to improve performance
    size_t start_word = last_allocated_page / 32;
    size_t start_bit = last_allocated_page % 32;
    
    // Search from last allocated page to end
    for (size_t i = start_word; i < bitmap_size; i++) {
        uint32_t word = bitmap[i];
        if (word != 0xFFFFFFFF) {
            for (size_t j = (i == start_word ? start_bit : 0); j < 32; j++) {
                if (!(word & (1 << j))) {
                    return i * 32 + j;
                }
            }
        }
    }
    
    // If not found, search from beginning to last allocated page
    for (size_t i = 0; i < start_word; i++) {
        uint32_t word = bitmap[i];
        if (word != 0xFFFFFFFF) {
            for (size_t j = 0; j < 32; j++) {
                if (!(word & (1 << j))) {
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
    total_pages = total_memory / PAGE_SIZE;
    free_pages = total_pages;
    
    // Calculate bitmap size (1 bit per page, 32 bits per uint32_t)
    bitmap_size = (total_pages + 31) / 32;
    
    // Find a suitable location for the bitmap in available memory
    const struct memory_map* map = memory_map_get();
    bool bitmap_allocated = false;
    
    // Look for a suitable memory region for the bitmap
    for (size_t i = 0; i < map->count; i++) {
        if (map->entries[i].type == MULTIBOOT_MEMORY_AVAILABLE) {
            uint64_t start = map->entries[i].addr;
            uint64_t length = map->entries[i].len;
            
            // Check if this region is large enough for the bitmap
            if (length >= bitmap_size * sizeof(uint32_t)) {
                // Place bitmap at the start of this region
                bitmap = (uint32_t*)start;
                bitmap_allocated = true;
                
                // Mark this region as used in the bitmap
                size_t start_page = start / PAGE_SIZE;
                size_t bitmap_pages = (bitmap_size * sizeof(uint32_t) + PAGE_SIZE - 1) / PAGE_SIZE;
                
                // Clear the bitmap first
                memset(bitmap, 0, bitmap_size * sizeof(uint32_t));
                
                // Mark bitmap pages as used
                for (size_t j = 0; j < bitmap_pages; j++) {
                    bitmap_set(start_page + j);
                    free_pages--;
                }
                
                terminal_writestring("PMM bitmap address: 0x");
                terminal_writehex((uint32_t)bitmap);
                terminal_writestring("\n");
                break;
            }
        }
    }
    
    if (!bitmap_allocated) {
        terminal_writestring("Failed to allocate bitmap for PMM\n");
        return;
    }
    
    // Mark the first 1MB as used (where kernel and bitmap reside)
    for (size_t i = 0; i < 256; i++) {  // 1MB / 4KB = 256 pages
        if (!bitmap_test(i)) {  // Only mark if not already marked
            bitmap_set(i);
            free_pages--;
        }
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
    
    // Mark page as used
    bitmap_set(page);
    free_pages--;
    
    // Update last allocated page
    last_allocated_page = page;
    
    // Convert page number to physical address
    return (void*)(page * PAGE_SIZE);
}

void pmm_free_page(void* page) {
    if (!page) return;
    
    // Convert physical address to page number
    size_t page_num = (size_t)page / PAGE_SIZE;
    
    // Check if page is valid
    if (page_num >= total_pages) {
        return;
    }
    
    // Check if page is actually allocated
    if (!bitmap_test(page_num)) {
        return;
    }
    
    // Mark page as free
    bitmap_clear(page_num);
    free_pages++;
}

size_t pmm_get_total_pages(void) {
    return total_pages;
}

size_t pmm_get_free_pages(void) {
    return free_pages;
}

// Map physical address to virtual address
void* pmm_map_physical_to_virtual(uint32_t physical_addr) {
    // For now, we'll use a simple 1:1 mapping
    // In a real system, this would use page tables
    return (void*)physical_addr;
} 