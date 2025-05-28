#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

// Memory page size (4KB)
#define PAGE_SIZE 4096

// Initialize the physical memory manager
void pmm_init(void);

// Allocate a single physical page
void* pmm_alloc_page(void);

// Free a single physical page
void pmm_free_page(void* page);

// Get the total number of available pages
size_t pmm_get_total_pages(void);

// Get the number of free pages
size_t pmm_get_free_pages(void);

// Map physical address to virtual address
void* pmm_map_physical_to_virtual(uint32_t physical_addr);

#endif // PMM_H 