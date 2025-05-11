#include "ata.h"
#include "../include/io.h"
#include "../include/vga.h"

// Helper function to wait for the drive to be ready
static int ata_wait_ready(void) {
    uint8_t status;
    do {
        status = inb(ATA_STATUS);
    } while (status & ATA_SR_BSY);
    return (status & ATA_SR_ERR) ? -1 : 0;
}

// Initialize ATA controller
void ata_init(void) {
    // Reset the controller
    outb(ATA_DRIVE_HEAD, ATA_DRIVE_MASTER | ATA_DRIVE_LBA);
    outb(ATA_FEATURES, 0);
    outb(ATA_SECTOR_CNT, 0);
    outb(ATA_LBA_LOW, 0);
    outb(ATA_LBA_MID, 0);
    outb(ATA_LBA_HIGH, 0);
    outb(ATA_COMMAND, 0xEC); // IDENTIFY command

    // Wait for the drive to be ready
    if (ata_wait_ready() != 0) {
        terminal_writestring("ATA initialization failed\n");
        return;
    }

    terminal_writestring("ATA controller initialized\n");
}

// Read sectors from disk
int ata_read_sectors(uint32_t lba, uint8_t sectors, void* buffer) {
    uint16_t* buf = (uint16_t*)buffer;
    uint8_t i;

    // Wait for the drive to be ready
    if (ata_wait_ready() != 0) {
        return -1;
    }

    // Set up the command
    outb(ATA_DRIVE_HEAD, ATA_DRIVE_MASTER | ATA_DRIVE_LBA | ((lba >> 24) & 0x0F));
    outb(ATA_SECTOR_CNT, sectors);
    outb(ATA_LBA_LOW, lba & 0xFF);
    outb(ATA_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_LBA_HIGH, (lba >> 16) & 0xFF);
    outb(ATA_COMMAND, ATA_CMD_READ_SECTORS);

    // Read the data
    for (i = 0; i < sectors; i++) {
        // Wait for data to be ready
        if (ata_wait_ready() != 0) {
            return -1;
        }

        // Read 256 words (512 bytes) per sector
        for (int j = 0; j < 256; j++) {
            *buf++ = inw(ATA_DATA);
        }
    }

    return 0;
}

// Write sectors to disk
int ata_write_sectors(uint32_t lba, uint8_t sectors, const void* buffer) {
    const uint16_t* buf = (const uint16_t*)buffer;
    uint8_t i;

    // Wait for the drive to be ready
    if (ata_wait_ready() != 0) {
        return -1;
    }

    // Set up the command
    outb(ATA_DRIVE_HEAD, ATA_DRIVE_MASTER | ATA_DRIVE_LBA | ((lba >> 24) & 0x0F));
    outb(ATA_SECTOR_CNT, sectors);
    outb(ATA_LBA_LOW, lba & 0xFF);
    outb(ATA_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_LBA_HIGH, (lba >> 16) & 0xFF);
    outb(ATA_COMMAND, ATA_CMD_WRITE_SECTORS);

    // Write the data
    for (i = 0; i < sectors; i++) {
        // Wait for the drive to be ready
        if (ata_wait_ready() != 0) {
            return -1;
        }

        // Write 256 words (512 bytes) per sector
        for (int j = 0; j < 256; j++) {
            outw(ATA_DATA, *buf++);
        }
    }

    return 0;
} 