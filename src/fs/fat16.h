#ifndef FAT16_H
#define FAT16_H

#include <stdint.h>

// FAT16 Boot Sector structure
typedef struct {
    uint8_t jump[3];
    uint8_t oem[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t number_of_fats;
    uint16_t root_entries;
    uint16_t total_sectors_16;
    uint8_t media_type;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    uint8_t drive_number;
    uint8_t reserved;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t fs_type[8];
} __attribute__((packed)) fat16_boot_sector_t;

// FAT16 Directory Entry structure
typedef struct {
    uint8_t filename[8];
    uint8_t extension[3];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creation_time_tenths;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t first_cluster_high;
    uint16_t last_write_time;
    uint16_t last_write_date;
    uint16_t first_cluster_low;
    uint32_t file_size;
} __attribute__((packed)) fat16_dir_entry_t;

// Filesystem functions
int fat16_init(void);
int fat16_read_file(const char* filename, void* buffer, uint32_t size);
int fat16_write_file(const char* filename, const void* buffer, uint32_t size);
int fat16_create_file(const char* filename);
int fat16_delete_file(const char* filename);
int fat16_list_directory(const char* path);

#endif 