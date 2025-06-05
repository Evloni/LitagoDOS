#include "../../include/drivers/iso_fs.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// The filesystem image will be loaded at this address
static uint32_t fs_base = ISO_FS_BASE;
static uint32_t fs_size = 0;  // Size of the filesystem in bytes

// Set the base address for the filesystem
void iso_fs_set_base(uint32_t base) {
    // Ensure the base address is 512-byte aligned
    if (base & 0x1FF) {
        return;  // Invalid alignment
    }
    fs_base = base;
}

// Set the size of the filesystem
void iso_fs_set_size(uint32_t size) {
    // Ensure size is a multiple of 512 bytes
    if (size & 0x1FF) {
        return;  // Invalid size
    }
    fs_size = size;
}

// Read sectors from the ISO filesystem
bool iso_fs_read_sectors(uint32_t lba, uint8_t sectors, void* buffer) {
    // Validate parameters
    if (!buffer || sectors == 0) {
        terminal_writestring("ISO: Invalid parameters for read\n");
        return false;
    }

    // Calculate the address in memory
    uint32_t addr = fs_base + (lba * 512);
    uint32_t size = sectors * 512;

    // Check if the read would exceed the filesystem size
    if (fs_size > 0 && (addr + size > fs_base + fs_size)) {
        terminal_writestring("ISO: Read would exceed filesystem size\n");
        return false;
    }

    // Copy the data
    uint8_t* src = (uint8_t*)addr;
    uint8_t* dst = (uint8_t*)buffer;
    for (int i = 0; i < size; i++) {
        dst[i] = src[i];
    }
    return true;
}

// Write sectors to the ISO filesystem
bool iso_fs_write_sectors(uint32_t lba, uint8_t sectors, const void* buffer) {
    // Validate parameters
    if (!buffer || sectors == 0) {
        return false;
    }

    // Calculate the address in memory
    uint32_t addr = fs_base + (lba * 512);
    uint32_t size = sectors * 512;

    // Check if the write would exceed the filesystem size
    if (fs_size > 0 && (addr + size > fs_base + fs_size)) {
        return false;
    }

    // Copy the data
    uint8_t* dst = (uint8_t*)addr;
    const uint8_t* src = (const uint8_t*)buffer;
    for (int i = 0; i < size; i++) {
        dst[i] = src[i];
    }
    return true;
}

// Initialize the ISO filesystem
bool iso_fs_init(void) {
    terminal_writestring("ISO: Initializing filesystem...\n");
    terminal_writestring("ISO: Base address: ");
    char addr_str[32];
    sprintf(addr_str, "0x%x\n", fs_base);
    terminal_writestring(addr_str);
    terminal_writestring("ISO: Size: ");
    sprintf(addr_str, "%d bytes\n", fs_size);
    terminal_writestring(addr_str);
    
    // The filesystem is already loaded by GRUB
    if (fs_size == 0) {
        terminal_writestring("ISO: Warning - Filesystem size is 0\n");
    }
    
    terminal_writestring("ISO: Filesystem initialized\n");
    return true;
}
