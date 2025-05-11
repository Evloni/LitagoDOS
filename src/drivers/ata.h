#ifndef ATA_H
#define ATA_H

#include <stdint.h>

// ATA I/O ports
#define ATA_DATA        0x1F0
#define ATA_FEATURES    0x1F1
#define ATA_SECTOR_CNT  0x1F2
#define ATA_LBA_LOW     0x1F3
#define ATA_LBA_MID     0x1F4
#define ATA_LBA_HIGH    0x1F5
#define ATA_DRIVE_HEAD  0x1F6
#define ATA_STATUS      0x1F7
#define ATA_COMMAND     0x1F7

// ATA commands
#define ATA_CMD_READ_SECTORS  0x20
#define ATA_CMD_WRITE_SECTORS 0x30

// Status register bits
#define ATA_SR_ERR  0x01
#define ATA_SR_DRQ  0x08
#define ATA_SR_BSY  0x80

// Drive/Head register bits
#define ATA_DRIVE_LBA 0xE0
#define ATA_DRIVE_MASTER 0xA0
#define ATA_DRIVE_SLAVE  0xB0

// Function declarations
void ata_init(void);
int ata_read_sectors(uint32_t lba, uint8_t sectors, void* buffer);
int ata_write_sectors(uint32_t lba, uint8_t sectors, const void* buffer);

#endif 