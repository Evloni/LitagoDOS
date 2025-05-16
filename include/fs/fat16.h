#ifndef FAT16_H
#define FAT16_H

#include <stdint.h>
#include <stdbool.h>

// FAT16 Boot Sector structure
typedef struct {
    uint8_t jump[3];
    uint8_t oem[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t num_fats;
    uint16_t root_entries;
    uint16_t total_sectors_16;
    uint8_t media_type;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    uint8_t drive_number;
    uint8_t reserved;
    uint8_t boot_signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t fs_type[8];
} __attribute__((packed)) fat16_boot_sector_t;

// FAT16 Directory Entry structure
typedef struct {
    uint8_t filename[8];
    uint8_t extension[3];
    uint8_t attributes;
    uint8_t reserved[10];
    uint16_t time;
    uint16_t date;
    uint16_t starting_cluster;
    uint32_t file_size;
} __attribute__((packed)) fat16_dir_entry_t;

// File attributes
#define FAT16_ATTR_READ_ONLY  0x01
#define FAT16_ATTR_HIDDEN     0x02
#define FAT16_ATTR_SYSTEM     0x04
#define FAT16_ATTR_VOLUME_ID  0x08
#define FAT16_ATTR_DIRECTORY  0x10
#define FAT16_ATTR_ARCHIVE    0x20
#define FAT16_ATTR_LONG_NAME  0x0F

// Function prototypes
bool fat16_init(void);
bool fat16_read_root_dir(void);
bool fat16_read_file(const char* filename, void* buffer, uint32_t max_size);
bool fat16_write_file(const char* filename, const void* buffer, uint32_t size);
bool fat16_create_file(const char* filename);
bool fat16_delete_file(const char* filename);
bool fat16_list_directory(const char* path);

// Helper functions
uint16_t fat16_get_next_cluster(uint16_t cluster);
bool fat16_is_end_of_chain(uint16_t cluster);
uint32_t fat16_cluster_to_lba(uint16_t cluster);

#endif // FAT16_H 