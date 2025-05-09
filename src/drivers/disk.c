#include "../include/disk.h"
#include "../include/io.h"
#include "../include/string.h"
#include "../include/vga.h"

// IDE/ATA I/O ports
#define IDE_DATA_PORT        0x1F0
#define IDE_FEATURES_PORT    0x1F1
#define IDE_SECTOR_COUNT     0x1F2
#define IDE_LBA_LOW         0x1F3
#define IDE_LBA_MID         0x1F4
#define IDE_LBA_HIGH        0x1F5
#define IDE_DRIVE_HEAD      0x1F6
#define IDE_COMMAND_PORT    0x1F7
#define IDE_STATUS_PORT     0x1F7

// IDE/ATA Commands
#define IDE_CMD_READ_SECTORS    0x20
#define IDE_CMD_WRITE_SECTORS   0x30
#define IDE_CMD_IDENTIFY       0xEC

// IDE/ATA Status Register Bits
#define IDE_STATUS_ERR     0x01
#define IDE_STATUS_DRQ     0x08
#define IDE_STATUS_BSY     0x80

// IDE/ATA Drive Selection
#define IDE_DRIVE_MASTER    0xA0
#define IDE_DRIVE_SLAVE     0xB0

// Maximum number of retries for disk operations
#define MAX_RETRIES 3

// Maximum timeout for disk operations (in milliseconds)
#define DISK_TIMEOUT 5000

// Internal function to wait for disk to be ready
static int disk_wait_ready(int timeout_ms) {
    uint8_t status;
    int timeout = timeout_ms * 1000; // Convert to microseconds

    while (timeout--) {
        status = inb(IDE_STATUS_PORT);
        if (!(status & IDE_STATUS_BSY)) {
            return 0; // Disk is ready
        }
        // Simple delay
        for (volatile int i = 0; i < 1000; i++);
    }
    return -1; // Timeout
}

// Internal function to select drive
static void disk_select_drive(int disk_id) {
    uint8_t drive = (disk_id == 0) ? IDE_DRIVE_MASTER : IDE_DRIVE_SLAVE;
    outb(IDE_DRIVE_HEAD, drive);
    // Wait a bit for the drive to respond
    for (volatile int i = 0; i < 1000; i++);
}

// Initialize the disk subsystem
int disk_init(void) {
    terminal_writestring("Initializing disk subsystem...\n");
    
    // Select primary master drive
    terminal_writestring("Selecting primary master drive...\n");
    disk_select_drive(0);
    
    // Wait for drive to be ready
    terminal_writestring("Waiting for drive to be ready...\n");
    if (disk_wait_ready(DISK_TIMEOUT) != 0) {
        terminal_setcolor(VGA_COLOR_RED);
        terminal_writestring("Error: Drive not ready after timeout\n");
        terminal_setcolor(VGA_COLOR_WHITE);
        return -1;
    }

    // Send IDENTIFY command
    terminal_writestring("Sending IDENTIFY command...\n");
    outb(IDE_COMMAND_PORT, IDE_CMD_IDENTIFY);
    
    // Wait for drive to be ready
    terminal_writestring("Waiting for IDENTIFY response...\n");
    if (disk_wait_ready(DISK_TIMEOUT) != 0) {
        terminal_setcolor(VGA_COLOR_RED);
        terminal_writestring("Error: No response to IDENTIFY command\n");
        terminal_setcolor(VGA_COLOR_WHITE);
        return -2;
    }

    // Check if drive exists
    uint8_t status = inb(IDE_STATUS_PORT);
    if (status == 0) {
        terminal_setcolor(VGA_COLOR_RED);
        terminal_writestring("Error: No drive detected (status = 0)\n");
        terminal_setcolor(VGA_COLOR_WHITE);
        return -3; // No drive
    }

    // Read identify data (we don't need to store it for now)
    terminal_writestring("Reading drive identification data...\n");
    for (int i = 0; i < 256; i++) {
        inw(IDE_DATA_PORT);
    }

    terminal_setcolor(VGA_COLOR_GREEN);
    terminal_writestring("Disk subsystem initialized successfully\n");
    terminal_setcolor(VGA_COLOR_WHITE);
    return 0;
}

// Read sectors from disk
int disk_read_sectors(int disk_id, uint32_t lba, uint16_t count, void* buffer) {
    if (!buffer || count == 0) {
        terminal_writestring("disk_read_sectors: invalid buffer or count\n");
        return -1;
    }

    // Select the appropriate drive
    disk_select_drive(disk_id);

    // Wait for drive to be ready
    if (disk_wait_ready(DISK_TIMEOUT) != 0) {
        terminal_writestring("disk_read_sectors: drive not ready\n");
        return -2;
    }

    // Set up the command
    outb(IDE_SECTOR_COUNT, count);
    outb(IDE_LBA_LOW, lba & 0xFF);
    outb(IDE_LBA_MID, (lba >> 8) & 0xFF);
    outb(IDE_LBA_HIGH, (lba >> 16) & 0xFF);
    outb(IDE_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));

    // Send read command
    outb(IDE_COMMAND_PORT, IDE_CMD_READ_SECTORS);

    // Read the data
    uint16_t* data = (uint16_t*)buffer;
    for (int sector = 0; sector < count; sector++) {
        // Wait for data to be ready
        if (disk_wait_ready(DISK_TIMEOUT) != 0) {
            terminal_writestring("disk_read_sectors: data not ready\n");
            return -3;
        }

        // Check for errors
        uint8_t status = inb(IDE_STATUS_PORT);
        if (status & IDE_STATUS_ERR) {
            terminal_writestring("disk_read_sectors: IDE error, status=");
            char status_buf[8];
            itoa(status, status_buf, 16);
            terminal_writestring(status_buf);
            terminal_writestring("\n");
            return -4;
        }

        // Read 256 words (512 bytes) per sector
        for (int i = 0; i < 256; i++) {
            data[i] = inw(IDE_DATA_PORT);
        }
        data += 256;
    }

    return 0;
}

// Write sectors to disk
int disk_write_sectors(int disk_id, uint32_t lba, uint16_t count, const void* buffer) {
    if (!buffer || count == 0) {
        return -1;
    }

    // Select the appropriate drive
    disk_select_drive(disk_id);

    // Wait for drive to be ready
    if (disk_wait_ready(DISK_TIMEOUT) != 0) {
        return -2;
    }

    // Set up the command
    outb(IDE_SECTOR_COUNT, count);
    outb(IDE_LBA_LOW, lba & 0xFF);
    outb(IDE_LBA_MID, (lba >> 8) & 0xFF);
    outb(IDE_LBA_HIGH, (lba >> 16) & 0xFF);
    outb(IDE_DRIVE_HEAD, ((lba >> 24) & 0x0F) | IDE_DRIVE_MASTER);

    // Send write command
    outb(IDE_COMMAND_PORT, IDE_CMD_WRITE_SECTORS);

    // Write the data
    const uint16_t* data = (const uint16_t*)buffer;
    for (int sector = 0; sector < count; sector++) {
        // Wait for drive to be ready
        if (disk_wait_ready(DISK_TIMEOUT) != 0) {
            return -3;
        }

        // Check for errors
        uint8_t status = inb(IDE_STATUS_PORT);
        if (status & IDE_STATUS_ERR) {
            return -4;
        }

        // Write 256 words (512 bytes) per sector
        for (int i = 0; i < 256; i++) {
            outw(IDE_DATA_PORT, data[i]);
        }
        data += 256;
    }

    return 0;
} 