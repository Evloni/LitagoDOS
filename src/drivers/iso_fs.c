#include "../../include/drivers/iso_fs.h"
#include "../../include/vga.h"
#include <stdbool.h>
#include <stdint.h>

// The filesystem image will be loaded at this address
static uint32_t fs_base = ISO_FS_BASE;

// Set the base address for the filesystem
void iso_fs_set_base(uint32_t base) {
    fs_base = base;
}

// Read sectors from the ISO filesystem
bool iso_fs_read_sectors(uint32_t lba, uint8_t sectors, void* buffer) {
    // Calculate the address in memory
    uint32_t addr = fs_base + (lba * 512);
    
    // Copy the data
    uint8_t* src = (uint8_t*)addr;
    uint8_t* dst = (uint8_t*)buffer;
    
    for (int i = 0; i < sectors * 512; i++) {
        dst[i] = src[i];
    }
    
    return true;
}

// Write sectors to the ISO filesystem
bool iso_fs_write_sectors(uint32_t lba, uint8_t sectors, const void* buffer) {
    // Calculate the address in memory
    uint32_t addr = fs_base + (lba * 512);
    
    // Copy the data
    uint8_t* dst = (uint8_t*)addr;
    const uint8_t* src = (const uint8_t*)buffer;
    
    for (int i = 0; i < sectors * 512; i++) {
        dst[i] = src[i];
    }
    
    return true;
}

// Initialize the ISO filesystem
bool iso_fs_init(void) {
    // The filesystem is already loaded by GRUB
    return true;
}
