#ifndef ISO_FS_H
#define ISO_FS_H

#include <stdbool.h>
#include <stdint.h>

// The filesystem image will be loaded at this address
#define ISO_FS_BASE 0x1000000  // 16MB mark

// Initialize the ISO filesystem
bool iso_fs_init(void);

// Set the base address for the filesystem
void iso_fs_set_base(uint32_t base);

// Read sectors from the ISO filesystem
bool iso_fs_read_sectors(uint32_t lba, uint8_t sectors, void* buffer);

// Write sectors to the ISO filesystem
bool iso_fs_write_sectors(uint32_t lba, uint8_t sectors, const void* buffer);

#endif
