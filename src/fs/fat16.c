#include "fat16.h"
#include "../include/io.h"
#include "../include/vga.h"
#include "../drivers/ata.h"
#include <stddef.h>

// Global variables
static fat16_boot_sector_t boot_sector;
static uint16_t* fat_table;
static uint32_t root_dir_sectors;
static uint32_t first_data_sector;
static uint32_t first_fat_sector;

// Helper function to read sectors from disk
static int read_sectors(uint32_t lba, uint8_t sectors, void* buffer) {
    return ata_read_sectors(lba, sectors, buffer);
}

// Helper function to write sectors to disk
static int write_sectors(uint32_t lba, uint8_t sectors, const void* buffer) {
    return ata_write_sectors(lba, sectors, buffer);
}

int fat16_init(void) {
    // Initialize ATA controller
    ata_init();

    // Read boot sector
    if (read_sectors(0, 1, &boot_sector) != 0) {
        terminal_writestring("Failed to read boot sector\n");
        return -1;
    }

    // Calculate important filesystem parameters
    root_dir_sectors = ((boot_sector.root_entries * 32) + 
                       (boot_sector.bytes_per_sector - 1)) / 
                       boot_sector.bytes_per_sector;
    
    first_fat_sector = boot_sector.reserved_sectors;
    first_data_sector = first_fat_sector + 
                       (boot_sector.number_of_fats * boot_sector.fat_size_16) + 
                       root_dir_sectors;

    // Allocate and read FAT table
    // TODO: Replace with proper memory allocation
    fat_table = (uint16_t*)0x100000; // Temporary fixed address
    if (read_sectors(first_fat_sector, boot_sector.fat_size_16, fat_table) != 0) {
        terminal_writestring("Failed to read FAT table\n");
        return -1;
    }

    terminal_writestring("FAT16 filesystem initialized successfully\n");
    return 0;
}

// Helper function to find a file in the root directory
static fat16_dir_entry_t* find_file(const char* filename) {
    fat16_dir_entry_t* dir_entry = (fat16_dir_entry_t*)0x200000; // Temporary fixed address
    uint32_t root_dir_sector = first_fat_sector + 
                              (boot_sector.number_of_fats * boot_sector.fat_size_16);

    // Search through root directory entries
    for (uint16_t i = 0; i < boot_sector.root_entries; i++) {
        if (read_sectors(root_dir_sector + (i / 16), 1, dir_entry) != 0) {
            return NULL;
        }

        dir_entry += (i % 16);
        
        // Check if this is the file we're looking for
        if (dir_entry->filename[0] != 0x00 && dir_entry->filename[0] != 0xE5) {
            char name[13];
            // TODO: Implement proper string comparison
            // For now, just return the first valid entry
            return dir_entry;
        }
    }

    return NULL;
}

int fat16_read_file(const char* filename, void* buffer, uint32_t size) {
    fat16_dir_entry_t* file_entry = find_file(filename);
    if (!file_entry) {
        terminal_writestring("File not found: ");
        terminal_writestring(filename);
        terminal_writestring("\n");
        return -1;
    }

    uint16_t cluster = file_entry->first_cluster_low;
    uint32_t bytes_read = 0;
    uint32_t bytes_per_cluster = boot_sector.sectors_per_cluster * 
                                boot_sector.bytes_per_sector;

    while (cluster != 0xFFFF && bytes_read < size) {
        uint32_t sector = first_data_sector + 
                         ((cluster - 2) * boot_sector.sectors_per_cluster);
        
        if (read_sectors(sector, boot_sector.sectors_per_cluster, 
                       (char*)buffer + bytes_read) != 0) {
            return -1;
        }

        bytes_read += bytes_per_cluster;
        cluster = fat_table[cluster];
    }

    return bytes_read;
}

int fat16_write_file(const char* filename, const void* buffer, uint32_t size) {
    // TODO: Implement file writing
    terminal_writestring("File writing not implemented yet\n");
    return -1;
}

int fat16_create_file(const char* filename) {
    // TODO: Implement file creation
    terminal_writestring("File creation not implemented yet\n");
    return -1;
}

int fat16_delete_file(const char* filename) {
    // TODO: Implement file deletion
    terminal_writestring("File deletion not implemented yet\n");
    return -1;
}

int fat16_list_directory(const char* path) {
    // TODO: Implement directory listing
    terminal_writestring("Directory listing not implemented yet\n");
    return -1;
} 