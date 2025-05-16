#include "../../include/drivers/ata.h"
#include "../../include/io.h"
#include "../../include/vga.h"
#include <stdbool.h>
#include <stdint.h>

// Wait for the drive to be ready
static bool ata_wait_ready(void) {
    uint8_t status;
    int timeout = 10000;  // Timeout after 10000 attempts

    while (timeout--) {
        status = inb(ATA_STATUS);
        if (!(status & ATA_SR_BSY)) {
            return true;
        }
    }
    return false;
}

// Wait for the drive to request data
static bool ata_wait_data(void) {
    uint8_t status;
    int timeout = 10000;  // Timeout after 10000 attempts

    while (timeout--) {
        status = inb(ATA_STATUS);
        if (status & ATA_SR_DRQ) {
            return true;
        }
        if (status & ATA_SR_ERR) {
            return false;
        }
    }
    return false;
}

// Initialize the ATA driver
bool ata_init(void) {
    // Reset the drive
    outb(ATA_DRIVE_HEAD, ATA_DRIVE_MASTER | ATA_DRIVE_LBA);
    outb(ATA_FEATURES, 0);
    outb(ATA_COMMAND, 0xEC);  // IDENTIFY command

    // Wait for the drive to be ready
    if (!ata_wait_ready()) {
        terminal_writestring("ATA drive not responding (timeout waiting for ready)\n");
        return false;
    }

    // Check if the drive is present
    uint8_t status = inb(ATA_STATUS);
    if (status == 0) {
        terminal_writestring("No ATA drive found (status register is 0)\n");
        return false;
    }

    // Print status register for debugging
    terminal_writestring("ATA Status Register: 0x");
    uint8_t status_hex = status;
    for (int i = 0; i < 2; i++) {
        uint8_t nibble = (status_hex >> (4 * (1 - i))) & 0x0F;
        terminal_putchar(nibble < 10 ? '0' + nibble : 'A' + (nibble - 10));
    }
    terminal_putchar('\n');

    // Wait for data
    if (!ata_wait_data()) {
        terminal_writestring("ATA drive error (timeout waiting for data)\n");
        return false;
    }

    // Read identify data (we don't need it, but we need to read it)
    for (int i = 0; i < 256; i++) {
        inw(ATA_DATA);
    }

    terminal_writestring("ATA drive initialized successfully\n");
    return true;
}

// Read sectors from the drive
bool ata_read_sectors(uint32_t lba, uint8_t sectors, void* buffer) {
    if (!ata_wait_ready()) {
        return false;
    }

    // Set up the command
    outb(ATA_DRIVE_HEAD, ATA_DRIVE_MASTER | ATA_DRIVE_LBA | ((lba >> 24) & 0x0F));
    outb(ATA_SECTOR_CNT, sectors);
    outb(ATA_LBA_LOW, lba & 0xFF);
    outb(ATA_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_LBA_HIGH, (lba >> 16) & 0xFF);
    outb(ATA_COMMAND, ATA_CMD_READ_SECTORS);

    // Read the data
    uint16_t* data = (uint16_t*)buffer;
    for (int i = 0; i < sectors; i++) {
        if (!ata_wait_data()) {
            return false;
        }

        // Read 256 words (512 bytes) per sector
        for (int j = 0; j < 256; j++) {
            data[j] = inw(ATA_DATA);
        }
        data += 256;
    }

    return true;
}

// Write sectors to the drive
bool ata_write_sectors(uint32_t lba, uint8_t sectors, const void* buffer) {
    if (!ata_wait_ready()) {
        return false;
    }

    // Set up the command
    outb(ATA_DRIVE_HEAD, ATA_DRIVE_MASTER | ATA_DRIVE_LBA | ((lba >> 24) & 0x0F));
    outb(ATA_SECTOR_CNT, sectors);
    outb(ATA_LBA_LOW, lba & 0xFF);
    outb(ATA_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_LBA_HIGH, (lba >> 16) & 0xFF);
    outb(ATA_COMMAND, ATA_CMD_WRITE_SECTORS);

    // Write the data
    const uint16_t* data = (const uint16_t*)buffer;
    for (int i = 0; i < sectors; i++) {
        if (!ata_wait_data()) {
            return false;
        }

        // Write 256 words (512 bytes) per sector
        for (int j = 0; j < 256; j++) {
            outw(ATA_DATA, data[j]);
        }
        data += 256;
    }

    return true;
} 