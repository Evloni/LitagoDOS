#include "../../include/memory/memory_map.h"
#include "../../include/drivers/vbe.h"
#include "../../include/io.h"
#include <stddef.h>

// Maximum number of memory map entries
#define MAX_MEMORY_MAP_ENTRIES 32

// Static memory map
static struct memory_map memory_map = {
    .entries = NULL,
    .count = 0,
    .capacity = MAX_MEMORY_MAP_ENTRIES
};

// Static array to store memory map entries
static struct multiboot_mmap_entry entries[MAX_MEMORY_MAP_ENTRIES];

// Multiboot header structure
struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint32_t vbe_mode;
    uint32_t vbe_interface_seg;
    uint32_t vbe_interface_off;
    uint32_t vbe_interface_len;
} __attribute__((packed));

void memory_map_init(uint32_t multiboot_magic, void* multiboot_info_ptr) {
    if (multiboot_magic != 0x2BADB002) {
        terminal_writestring("Invalid Multiboot magic number\n");
        return;
    }

    struct multiboot_info* mb_info = (struct multiboot_info*)multiboot_info_ptr;
    
    // Debug output
    terminal_writestring("Multiboot flags: 0x");
    char hex[9];
    for (int i = 7; i >= 0; i--) {
        hex[7-i] = "0123456789ABCDEF"[(mb_info->flags >> (i * 4)) & 0xF];
    }
    hex[8] = '\0';
    terminal_writestring(hex);
    terminal_writestring("\n");
    
    if (!(mb_info->flags & 0x40)) {  // Check if memory map is available
        terminal_writestring("No memory map available in Multiboot info\n");
        return;
    }

    // Debug output for memory map info
    terminal_writestring("Memory map address: 0x");
    for (int i = 7; i >= 0; i--) {
        hex[7-i] = "0123456789ABCDEF"[(mb_info->mmap_addr >> (i * 4)) & 0xF];
    }
    terminal_writestring(hex);
    terminal_writestring("\n");
    
    terminal_writestring("Memory map length: 0x");
    for (int i = 7; i >= 0; i--) {
        hex[7-i] = "0123456789ABCDEF"[(mb_info->mmap_length >> (i * 4)) & 0xF];
    }
    terminal_writestring(hex);
    terminal_writestring("\n");

    // Copy memory map entries
    struct multiboot_mmap_entry* mmap = (struct multiboot_mmap_entry*)mb_info->mmap_addr;
    size_t count = 0;

    while ((uint32_t)mmap < mb_info->mmap_addr + mb_info->mmap_length && count < MAX_MEMORY_MAP_ENTRIES) {
        entries[count] = *mmap;
        count++;
        mmap = (struct multiboot_mmap_entry*)((uint32_t)mmap + mmap->size + sizeof(uint32_t));
    }

    memory_map.entries = entries;
    memory_map.count = count;

    terminal_writestring("Memory map initialized successfully with ");
    char count_str[10];
    int i = 0;
    size_t temp_count = count;
    do {
        count_str[i++] = '0' + (temp_count % 10);
        temp_count /= 10;
    } while (temp_count > 0);
    while (--i >= 0) {
        terminal_putchar(count_str[i]);
    }
    terminal_writestring(" entries\n");
}

uint64_t memory_map_get_total_memory(void) {
    uint64_t total = 0;
    
    for (size_t i = 0; i < memory_map.count; i++) {
        if (memory_map.entries[i].type == MULTIBOOT_MEMORY_AVAILABLE) {
            total += memory_map.entries[i].len;
        }
    }
    
    return total;
}

const struct memory_map* memory_map_get(void) {
    return &memory_map;
}

void memory_map_print(void) {
    terminal_writestring("\nMemory Map:\n");
    terminal_writestring("Base Address        Length              Type\n");
    terminal_writestring("------------------------------------------------\n");

    for (size_t i = 0; i < memory_map.count; i++) {
        struct multiboot_mmap_entry* entry = &memory_map.entries[i];
        
        // Convert numbers to hex strings
        uint64_t base = entry->addr;
        uint64_t length = entry->len;
        
        // Format the output
        terminal_writestring("0x");
        for (int j = 15; j >= 0; j--) {
            char c = "0123456789ABCDEF"[(base >> (j * 4)) & 0xF];
            terminal_putchar(c);
        }
        
        terminal_writestring("  0x");
        for (int j = 15; j >= 0; j--) {
            char c = "0123456789ABCDEF"[(length >> (j * 4)) & 0xF];
            terminal_putchar(c);
        }
        
        // Print type
        terminal_writestring("  ");
        switch (entry->type) {
            case MULTIBOOT_MEMORY_AVAILABLE:
                terminal_writestring("Available");
                break;
            case MULTIBOOT_MEMORY_RESERVED:
                terminal_writestring("Reserved");
                break;
            case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
                terminal_writestring("ACPI Reclaimable");
                break;
            case MULTIBOOT_MEMORY_NVS:
                terminal_writestring("ACPI NVS");
                break;
            case MULTIBOOT_MEMORY_BADRAM:
                terminal_writestring("Bad Memory");
                break;
            default:
                terminal_writestring("Unknown");
                break;
        }
        terminal_writestring("\n");
    }
    
    // Print total usable memory
    uint64_t total = memory_map_get_total_memory();
    terminal_writestring("\nTotal Usable Memory: ");
    
    // Convert to MB for display
    uint64_t total_mb = total / (1024 * 1024);
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
    terminal_writestring(" MB\n\n");
} 