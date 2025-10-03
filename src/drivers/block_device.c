#include "../../include/drivers/block_device.h"
#include "../../include/drivers/ide.h"
#include <stddef.h>

// Global block device interface
block_device_t* current_block_device = NULL;

// IDE block device implementation
static bool block_device_ide_read_sectors(uint32_t start_sector, uint32_t count, void* buffer) {
    return ide_read_sectors(0, 0, start_sector, count, buffer);  // Using primary master
}

static bool block_device_ide_write_sectors(uint32_t start_sector, uint32_t count, const void* buffer) {
    return ide_write_sectors(0, 0, start_sector, count, buffer);  // Using primary master
}

static uint32_t block_device_ide_get_total_sectors(void) {
    // For now, return a reasonable default size (1GB = 2,097,152 sectors)
    // In a real implementation, you'd get this from the drive's identify data
    return 2097152;  // 1GB in sectors
}

static uint16_t block_device_ide_get_sector_size(void) {
    return 512;  // Standard sector size for IDE
}

// IDE block device structure
static block_device_t ide_device = {
    .read_sectors = block_device_ide_read_sectors,
    .write_sectors = block_device_ide_write_sectors,
    .get_total_sectors = block_device_ide_get_total_sectors,
    .get_sector_size = block_device_ide_get_sector_size
};

// Initialize block device interface with IDE driver
bool block_device_init(void) {
    // Initialize IDE driver
    if (!ide_init()) {
        return false;
    }
    
    // Set current block device to IDE
    current_block_device = &ide_device;
    return true;
} 