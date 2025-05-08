#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

#include <stdint.h>
#include <stddef.h>

// Multiboot memory map entry types
#define MULTIBOOT_MEMORY_AVAILABLE 1
#define MULTIBOOT_MEMORY_RESERVED 2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MEMORY_NVS 4
#define MULTIBOOT_MEMORY_BADRAM 5

// Multiboot memory map entry structure
struct multiboot_mmap_entry {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} __attribute__((packed));

// Memory map structure
struct memory_map {
    struct multiboot_mmap_entry* entries;
    size_t count;
    size_t capacity;
};

// Initialize memory map from Multiboot info
void memory_map_init(uint32_t multiboot_magic, void* multiboot_info);

// Get total usable memory size
uint64_t memory_map_get_total_memory(void);

// Get memory map
const struct memory_map* memory_map_get(void);

// Print memory map information
void memory_map_print(void);

#endif // MEMORY_MAP_H 