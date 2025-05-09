#ifndef _DISK_H
#define _DISK_H

#include <stdint.h>

// Initialize the disk subsystem
// Returns 0 on success, negative value on error
int disk_init(void);

// Read sectors from disk
// disk_id: 0 for master, 1 for slave
// lba: Logical Block Address (sector number)
// count: Number of sectors to read
// buffer: Buffer to store the read data (must be large enough for count * 512 bytes)
// Returns 0 on success, negative value on error
int disk_read_sectors(int disk_id, uint32_t lba, uint16_t count, void* buffer);

// Write sectors to disk
// disk_id: 0 for master, 1 for slave
// lba: Logical Block Address (sector number)
// count: Number of sectors to write
// buffer: Buffer containing the data to write (must be count * 512 bytes)
// Returns 0 on success, negative value on error
int disk_write_sectors(int disk_id, uint32_t lba, uint16_t count, const void* buffer);

#endif // _DISK_H 