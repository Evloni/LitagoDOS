// Block device interface
#ifndef BLOCK_DEVICE_H
#define BLOCK_DEVICE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    // Read sectors from the device
    bool (*read_sectors)(uint32_t start_sector, uint32_t count, void* buffer);
    
    // Write sectors to the device
    bool (*write_sectors)(uint32_t start_sector, uint32_t count, const void* buffer);
    
    // Get total number of sectors
    uint32_t (*get_total_sectors)(void);
    
    // Get sector size in bytes
    uint16_t (*get_sector_size)(void);
} block_device_t;

// Global block device interface
extern block_device_t* current_block_device;

// Initialize block device interface with IDE driver
bool block_device_init(void);

#endif // BLOCK_DEVICE_H 