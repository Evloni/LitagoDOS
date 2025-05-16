#include "../../include/fs/fat16.h"
#include "../../include/drivers/ata.h"
#include "../../include/vga.h"
#include <string.h>

// Global variables
static fat16_boot_sector_t boot_sector;
static uint16_t* fat_table = NULL;
static uint32_t fat_start_sector;
static uint32_t root_dir_start_sector;
static uint32_t data_start_sector;
static uint32_t sectors_per_fat;
static uint32_t root_dir_sectors;

// Initialize FAT16 filesystem
bool fat16_init(void) {
    // Read boot sector
    if (!ata_read_sectors(0, 1, &boot_sector)) {
        terminal_writestring("Failed to read boot sector\n");
        return false;
    }

    // Verify FAT16 signature
    if (boot_sector.fs_type[0] != 'F' || 
        boot_sector.fs_type[1] != 'A' || 
        boot_sector.fs_type[2] != 'T' || 
        boot_sector.fs_type[3] != '1' || 
        boot_sector.fs_type[4] != '6') {
        terminal_writestring("Not a FAT16 filesystem\n");
        return false;
    }

    // Calculate important sector locations
    fat_start_sector = boot_sector.reserved_sectors;
    sectors_per_fat = boot_sector.fat_size_16;
    root_dir_sectors = ((boot_sector.root_entries * 32) + (boot_sector.bytes_per_sector - 1)) / boot_sector.bytes_per_sector;
    root_dir_start_sector = fat_start_sector + (sectors_per_fat * boot_sector.num_fats);
    data_start_sector = root_dir_start_sector + root_dir_sectors;

    // Allocate memory for FAT table
    fat_table = (uint16_t*)malloc(sectors_per_fat * boot_sector.bytes_per_sector);
    if (!fat_table) {
        terminal_writestring("Failed to allocate memory for FAT table\n");
        return false;
    }

    // Read FAT table
    if (!ata_read_sectors(fat_start_sector, sectors_per_fat, fat_table)) {
        terminal_writestring("Failed to read FAT table\n");
        free(fat_table);
        fat_table = NULL;
        return false;
    }

    return true;
}

// Convert cluster number to LBA
uint32_t fat16_cluster_to_lba(uint16_t cluster) {
    return data_start_sector + ((cluster - 2) * boot_sector.sectors_per_cluster);
}

// Get next cluster in chain
uint16_t fat16_get_next_cluster(uint16_t cluster) {
    if (cluster < 2 || cluster >= 0xFFF8) {
        return 0xFFFF;
    }
    return fat_table[cluster];
}

// Check if cluster is end of chain
bool fat16_is_end_of_chain(uint16_t cluster) {
    return (cluster >= 0xFFF8);
}

// Read root directory
bool fat16_read_root_dir(void) {
    fat16_dir_entry_t* root_dir = (fat16_dir_entry_t*)malloc(root_dir_sectors * boot_sector.bytes_per_sector);
    if (!root_dir) {
        terminal_writestring("Failed to allocate memory for root directory\n");
        return false;
    }

    if (!ata_read_sectors(root_dir_start_sector, root_dir_sectors, root_dir)) {
        terminal_writestring("Failed to read root directory\n");
        free(root_dir);
        return false;
    }

    // Print header
    terminal_writestring("NAME         SIZE      TYPE    \n");
    terminal_writestring("-------------------------------\n");

    for (int i = 0; i < boot_sector.root_entries; i++) {
        // End of directory
        if (root_dir[i].filename[0] == 0x00) break;
        // Deleted entry
        if (root_dir[i].filename[0] == 0xE5) continue;
        // Skip long filename entries
        if ((root_dir[i].attributes & FAT16_ATTR_LONG_NAME) == FAT16_ATTR_LONG_NAME) continue;
        // Skip volume labels
        if (root_dir[i].attributes & FAT16_ATTR_VOLUME_ID) continue;

        // Format filename
        char name[13] = {0};
        int name_len = 8, ext_len = 3;
        while (name_len > 0 && root_dir[i].filename[name_len - 1] == ' ') name_len--;
        int idx = 0;
        for (int j = 0; j < name_len; j++) name[idx++] = root_dir[i].filename[j];
        // Extension
        while (ext_len > 0 && root_dir[i].extension[ext_len - 1] == ' ') ext_len--;
        if (ext_len > 0) {
            name[idx++] = '.';
            for (int j = 0; j < ext_len; j++) name[idx++] = root_dir[i].extension[j];
        }
        name[idx] = '\0';

        // Print name, padded to 12 chars
        int actual_name_len = strlen(name);
        terminal_writestring(name);
        for (int s = actual_name_len; s < 12; s++) terminal_putchar(' ');

        // Print extension, padded to 5 chars
        int actual_ext_len = strlen(name + actual_name_len + 1);
        terminal_writestring(name + actual_name_len + 1);
        for (int s = actual_ext_len; s < 5; s++) terminal_putchar(' ');

        // Print size or blank for directories, padded to 9 chars
        if (root_dir[i].attributes & FAT16_ATTR_DIRECTORY) {
            for (int s = 0; s < 9; s++) terminal_putchar(' ');
        } else {
            char size_str[16];
            itoa(root_dir[i].file_size, size_str, 10);
            terminal_writestring(size_str);
            terminal_writestring(" bytes");
            int len = strlen(size_str) + 6;
            for (int s = len; s < 9; s++) terminal_putchar(' ');
        }

        // Print type, centered in 8 chars
        const char* type_str = (root_dir[i].attributes & FAT16_ATTR_DIRECTORY) ? "DIR" : "FILE";
        int type_len = strlen(type_str);
        int total_width = 8;
        int left_pad = (total_width - type_len) / 2;
        int right_pad = total_width - type_len - left_pad;
        for (int s = 0; s < left_pad; s++) terminal_putchar(' ');
        terminal_writestring(type_str);
        for (int s = 0; s < right_pad; s++) terminal_putchar(' ');
        terminal_putchar('\n');
    }

    free(root_dir);
    return true;
}

// Read file contents
int fat16_read_file(const char* filename, void* buffer, uint32_t max_size) {
    fat16_dir_entry_t* root_dir = (fat16_dir_entry_t*)malloc(root_dir_sectors * boot_sector.bytes_per_sector);
    if (!root_dir) {
        return 0;
    }

    if (!ata_read_sectors(root_dir_start_sector, root_dir_sectors, root_dir)) {
        free(root_dir);
        return 0;
    }

    // Find file in root directory
    fat16_dir_entry_t* file_entry = NULL;
    for (int i = 0; i < boot_sector.root_entries; i++) {
        if (root_dir[i].filename[0] == 0x00) break;
        if (root_dir[i].filename[0] == 0xE5) continue;

        char entry_name[13];
        int name_idx = 0;
        // Copy filename
        for (int j = 0; j < 8; j++) {
            if (root_dir[i].filename[j] != ' ') {
                entry_name[name_idx++] = root_dir[i].filename[j];
            }
        }
        // Add extension if it exists
        if (root_dir[i].extension[0] != ' ') {
            entry_name[name_idx++] = '.';
            for (int j = 0; j < 3; j++) {
                if (root_dir[i].extension[j] != ' ') {
                    entry_name[name_idx++] = root_dir[i].extension[j];
                }
            }
        }
        entry_name[name_idx] = '\0';

        if (strcmp(entry_name, filename) == 0) {
            file_entry = &root_dir[i];
            break;
        }
    }

    if (!file_entry) {
        free(root_dir);
        return 0;
    }

    // Check for empty file
    if (file_entry->file_size == 0) {
        free(root_dir);
        return -1; // Special value for empty file
    }

    // Read file data
    uint16_t cluster = file_entry->starting_cluster;
    uint32_t bytes_read = 0;
    uint8_t* data_buffer = (uint8_t*)buffer;

    while (cluster != 0xFFFF && !fat16_is_end_of_chain(cluster)) {
        uint32_t lba = fat16_cluster_to_lba(cluster);
        if (!ata_read_sectors(lba, boot_sector.sectors_per_cluster, data_buffer + bytes_read)) {
            free(root_dir);
            return 0;
        }
        bytes_read += boot_sector.sectors_per_cluster * boot_sector.bytes_per_sector;
        if (bytes_read >= max_size) break;
        cluster = fat16_get_next_cluster(cluster);
    }

    free(root_dir);
    return 1;
}

// List directory contents
bool fat16_list_directory(const char* path) {
    // For now, we only support root directory
    if (path[0] != '\0' && strcmp(path, "/") != 0) {
        terminal_writestring("Only root directory is supported\n");
        return false;
    }

    return fat16_read_root_dir();
}

bool fat16_create_file(const char* filename) {
    // Only support root directory
    fat16_dir_entry_t* root_dir = (fat16_dir_entry_t*)malloc(root_dir_sectors * boot_sector.bytes_per_sector);
    if (!root_dir) return false;
    if (!ata_read_sectors(root_dir_start_sector, root_dir_sectors, root_dir)) {
        free(root_dir);
        return false;
    }

    // Check if file already exists
    for (int i = 0; i < boot_sector.root_entries; i++) {
        if (root_dir[i].filename[0] == 0x00) break;
        if (root_dir[i].filename[0] == 0xE5) continue;
        if ((root_dir[i].attributes & FAT16_ATTR_LONG_NAME) == FAT16_ATTR_LONG_NAME) continue;
        if (root_dir[i].attributes & FAT16_ATTR_VOLUME_ID) continue;
        // Compare name
        char entry_name[13];
        int name_idx = 0;
        for (int j = 0; j < 8; j++) if (root_dir[i].filename[j] != ' ') entry_name[name_idx++] = root_dir[i].filename[j];
        if (root_dir[i].extension[0] != ' ') {
            entry_name[name_idx++] = '.';
            for (int j = 0; j < 3; j++) if (root_dir[i].extension[j] != ' ') entry_name[name_idx++] = root_dir[i].extension[j];
        }
        entry_name[name_idx] = '\0';
        if (strcmp(entry_name, filename) == 0) {
            free(root_dir);
            return false; // File already exists
        }
    }

    // Find a free entry
    int free_idx = -1;
    for (int i = 0; i < boot_sector.root_entries; i++) {
        if (root_dir[i].filename[0] == 0x00 || root_dir[i].filename[0] == 0xE5) {
            free_idx = i;
            break;
        }
    }
    if (free_idx == -1) {
        free(root_dir);
        return false; // No free entry
    }

    // Parse filename and extension (8.3 format)
    char name[8] = {' '}, ext[3] = {' '};
    const char* dot = strchr(filename, '.');
    int name_len = dot ? (dot - filename) : strlen(filename);
    if (name_len > 8) name_len = 8;
    for (int i = 0; i < name_len; i++) name[i] = filename[i];
    if (dot) {
        int ext_len = strlen(dot + 1);
        if (ext_len > 3) ext_len = 3;
        for (int i = 0; i < ext_len; i++) ext[i] = dot[1 + i];
    }

    // Write entry
    memcpy(root_dir[free_idx].filename, name, 8);
    memcpy(root_dir[free_idx].extension, ext, 3);
    root_dir[free_idx].attributes = 0;
    memset(root_dir[free_idx].reserved, 0, 10);
    root_dir[free_idx].time = 0;
    root_dir[free_idx].date = 0;
    root_dir[free_idx].starting_cluster = 0;
    root_dir[free_idx].file_size = 0;

    // Write back root directory
    bool ok = ata_write_sectors(root_dir_start_sector, root_dir_sectors, root_dir);
    free(root_dir);
    return ok;
} 