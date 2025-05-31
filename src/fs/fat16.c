#include "../../include/fs/fat16.h"
#include "../../include/drivers/iso_fs.h"
#include "../../include/drivers/vbe.h"
#include <string.h>

// Simple toupper implementation
static char toupper(char c) {
    if (c >= 'a' && c <= 'z') {
        return c - ('a' - 'A');
    }
    return c;
}

// Global variables
fat16_boot_sector_t boot_sector;
uint16_t* fat_table = NULL;
uint32_t fat_start_sector;
uint32_t root_dir_start_sector;
uint32_t data_start_sector;
uint32_t sectors_per_fat;
uint32_t root_dir_sectors;

// Initialize FAT16 filesystem
bool fat16_init(void) {
    // Initialize ISO filesystem first
    if (!iso_fs_init()) {
        return false;
    }

    // Read boot sector
    uint8_t sector_buffer[512];
    if (!iso_fs_read_sectors(0, 1, sector_buffer)) {
        return false;
    }
    memcpy(&boot_sector, sector_buffer, sizeof(fat16_boot_sector_t));

    // Verify FAT16 signature
    if (boot_sector.fs_type[0] != 'F' || 
        boot_sector.fs_type[1] != 'A' || 
        boot_sector.fs_type[2] != 'T' || 
        boot_sector.fs_type[3] != '1' || 
        boot_sector.fs_type[4] != '6') {
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
        return false;
    }

    // Read FAT table
    if (!iso_fs_read_sectors(fat_start_sector, sectors_per_fat, fat_table)) {
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

// Helper function to get file type string
static const char* get_file_type(const fat16_dir_entry_t* entry) {
    if (entry->attributes & FAT16_ATTR_DIRECTORY) {
        return "DIR";
    }
    
    // Check file extension
    char ext[4] = {0};
    int ext_idx = 0;
    for (int i = 0; i < 3; i++) {
        if (entry->extension[i] != ' ') {
            ext[ext_idx++] = toupper(entry->extension[i]);  // Convert to uppercase while copying
        }
    }
    ext[ext_idx] = '\0';
    
    // Common file types (all comparisons are now case-insensitive)
    if (strcmp(ext, "TXT") == 0) return "TXT";
    if (strcmp(ext, "BAT") == 0) return "BAT";
    if (strcmp(ext, "COM") == 0) return "COM";
    if (strcmp(ext, "EXE") == 0) return "EXE";
    if (strcmp(ext, "SYS") == 0) return "SYS";
    if (strcmp(ext, "BIN") == 0) return "BIN";
    if (strcmp(ext, "IMG") == 0) return "IMG";
    if (strcmp(ext, "DAT") == 0) return "DAT";
    if (strcmp(ext, "CFG") == 0) return "CFG";
    if (strcmp(ext, "INI") == 0) return "INI";
    
    // If no specific type is found, return "FILE"
    return "FILE";
}

// Read root directory
bool fat16_read_root_dir(void) {
    fat16_dir_entry_t* root_dir = (fat16_dir_entry_t*)malloc(root_dir_sectors * boot_sector.bytes_per_sector);
    if (!root_dir) {
        return false;
    }

    if (!iso_fs_read_sectors(root_dir_start_sector, root_dir_sectors, root_dir)) {
        free(root_dir);
        return false;
    }

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
        for (int s = actual_name_len; s < 12; s++) name[idx++] = ' ';
        name[idx] = '\0';

        // Print size or blank for directories, padded to 9 chars
        if (root_dir[i].attributes & FAT16_ATTR_DIRECTORY) {
            for (int s = 0; s < 9; s++) name[idx++] = ' ';
        } else {
            char size_str[16];
            itoa(root_dir[i].file_size, size_str, 10);
            for (int s = strlen(size_str) + 6; s < 9; s++) name[idx++] = ' ';
        }

        // Print type, centered in 8 chars
        const char* type_str = get_file_type(&root_dir[i]);
        int type_len = strlen(type_str);
        int total_width = 8;
        int left_pad = (total_width - type_len) / 2;
        int right_pad = total_width - type_len - left_pad;
        for (int s = 0; s < left_pad; s++) name[idx++] = ' ';
        for (int s = 0; s < type_len; s++) name[idx++] = type_str[s];
        for (int s = 0; s < right_pad; s++) name[idx++] = ' ';
        name[idx] = '\0';
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

    if (!iso_fs_read_sectors(root_dir_start_sector, root_dir_sectors, root_dir)) {
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
        if (!iso_fs_read_sectors(lba, boot_sector.sectors_per_cluster, data_buffer + bytes_read)) {
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
        return false;
    }

    return fat16_read_root_dir();
}

// Helper function to parse filename and extension
static void parse_filename(const char* filename, char* name, char* ext) {
    // Initialize with spaces
    memset(name, ' ', 8);
    memset(ext, ' ', 3);
    
    const char* dot = strchr(filename, '.');
    int name_len = dot ? (dot - filename) : strlen(filename);
    
    // Handle name (up to 8 chars)
    if (name_len > 8) name_len = 8;
    for (int i = 0; i < name_len; i++) {
        // Preserve original case
        name[i] = filename[i];
    }
    
    // Handle extension (up to 3 chars)
    if (dot) {
        int ext_len = strlen(dot + 1);
        if (ext_len > 3) ext_len = 3;
        for (int i = 0; i < ext_len; i++) {
            // Preserve original case
            ext[i] = dot[1 + i];
        }
    }
}

// Helper function to compare filenames case-insensitively
static bool compare_filenames(const char* name1, const char* name2) {
    char upper1[13] = {0};
    char upper2[13] = {0};
    int idx1 = 0, idx2 = 0;
    
    // Convert first name to uppercase
    for (int i = 0; name1[i]; i++) {
        upper1[idx1++] = toupper(name1[i]);
    }
    
    // Convert second name to uppercase
    for (int i = 0; name2[i]; i++) {
        upper2[idx2++] = toupper(name2[i]);
    }
    
    return strcmp(upper1, upper2) == 0;
}

// Helper function to find first free cluster
static uint16_t find_free_cluster(void) {
    // Start from cluster 2 (first data cluster)
    for (uint16_t cluster = 2; cluster < 0xFFF8; cluster++) {
        if (fat_table[cluster] == 0x0000) {
            return cluster;
        }
    }
    return 0xFFFF;  // No free clusters found
}

bool fat16_remove_file(const char* filename) {
    // Only support root directory
    fat16_dir_entry_t* root_dir = (fat16_dir_entry_t*)malloc(root_dir_sectors * boot_sector.bytes_per_sector);
    if (!root_dir) return false;
    
    // Read root directory
    if (!iso_fs_read_sectors(root_dir_start_sector, root_dir_sectors, root_dir)) {
        free(root_dir);
        return false;
    }

    // Find file entry
    int file_index = -1;
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
        
        if (compare_filenames(entry_name, filename)) {
            file_index = i;
            break;
        }
    }

    if (file_index == -1) {
        free(root_dir);
        return false; // File not found
    }

    // Free all clusters used by the file
    uint16_t cluster = root_dir[file_index].starting_cluster;
    if (cluster != 0) {  // Only try to free clusters if the file has any
        while (cluster != 0xFFFF && !fat16_is_end_of_chain(cluster)) {
            uint16_t next_cluster = fat_table[cluster];
            fat_table[cluster] = 0x0000;  // Mark as free
            cluster = next_cluster;
        }

        // Write back FAT table first
        if (!iso_fs_write_sectors(fat_start_sector, sectors_per_fat, fat_table)) {
            free(root_dir);
            return false;
        }
    }

    // Now mark directory entry as deleted
    root_dir[file_index].filename[0] = 0xE5;
    root_dir[file_index].starting_cluster = 0;  // Clear starting cluster
    root_dir[file_index].file_size = 0;        // Clear file size

    // Write back FAT table
    if (!iso_fs_write_sectors(fat_start_sector, sectors_per_fat, fat_table)) {
        free(root_dir);
        return false;
    }

    // Write back root directory
    if (!iso_fs_write_sectors(root_dir_start_sector, root_dir_sectors, root_dir)) {
        free(root_dir);
        return false;
    }

    free(root_dir);
    return true;
}

bool fat16_write_file(const char* filename, const void* buffer, uint32_t size) {
    // Only support root directory
    fat16_dir_entry_t* root_dir = (fat16_dir_entry_t*)malloc(root_dir_sectors * boot_sector.bytes_per_sector);
    if (!root_dir) return false;
    if (!iso_fs_read_sectors(root_dir_start_sector, root_dir_sectors, root_dir)) {
        free(root_dir);
        return false;
    }

    // Find or create file entry
    fat16_dir_entry_t* file_entry = NULL;
    int file_index = -1;
    for (int i = 0; i < boot_sector.root_entries; i++) {
        if (root_dir[i].filename[0] == 0x00) break;
        if (root_dir[i].filename[0] == 0xE5) continue;
        
        char entry_name[13];
        int name_idx = 0;
        for (int j = 0; j < 8; j++) if (root_dir[i].filename[j] != ' ') entry_name[name_idx++] = root_dir[i].filename[j];
        if (root_dir[i].extension[0] != ' ') {
            entry_name[name_idx++] = '.';
            for (int j = 0; j < 3; j++) if (root_dir[i].extension[j] != ' ') entry_name[name_idx++] = root_dir[i].extension[j];
        }
        entry_name[name_idx] = '\0';
        
        if (compare_filenames(entry_name, filename)) {
            file_entry = &root_dir[i];
            file_index = i;
            break;
        }
    }

    // If file exists, free its clusters
    if (file_entry && file_entry->starting_cluster != 0) {
        uint16_t cluster = file_entry->starting_cluster;
        while (cluster != 0xFFFF && !fat16_is_end_of_chain(cluster)) {
            uint16_t next_cluster = fat_table[cluster];
            fat_table[cluster] = 0x0000;  // Mark as free
            cluster = next_cluster;
        }
    }

    if (!file_entry) {
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
        // Parse filename and extension
        char name[8], ext[3];
        parse_filename(filename, name, ext);
        memcpy(root_dir[free_idx].filename, name, 8);
        memcpy(root_dir[free_idx].extension, ext, 3);
        root_dir[free_idx].attributes = 0;
        memset(root_dir[free_idx].reserved, 0, 10);
        root_dir[free_idx].time = 0;
        root_dir[free_idx].date = 0;
        root_dir[free_idx].starting_cluster = 0;
        root_dir[free_idx].file_size = 0;
        file_entry = &root_dir[free_idx];
        file_index = free_idx;
    }

    // Calculate how many clusters are needed
    uint32_t bytes_per_cluster = boot_sector.sectors_per_cluster * boot_sector.bytes_per_sector;
    uint32_t clusters_needed = (size + bytes_per_cluster - 1) / bytes_per_cluster;
    if (clusters_needed == 0) clusters_needed = 1;

    // Allocate clusters
    uint16_t first_cluster = 0;
    uint16_t prev_cluster = 0;
    uint32_t bytes_written = 0;
    for (uint32_t c = 0; c < clusters_needed; c++) {
        // Find a free cluster
        uint16_t free_cluster = find_free_cluster();
        if (free_cluster == 0xFFFF) {
            // If we failed to allocate all clusters, free the ones we did allocate
            if (first_cluster != 0) {
                uint16_t cluster = first_cluster;
                while (cluster != 0xFFFF && !fat16_is_end_of_chain(cluster)) {
                    uint16_t next_cluster = fat_table[cluster];
                    fat_table[cluster] = 0x0000;  // Mark as free
                    cluster = next_cluster;
                }
            }
            free(root_dir);
            return false; // No free cluster
        }
        // Mark as end of chain for now
        fat_table[free_cluster] = 0xFFF8;
        if (prev_cluster != 0) fat_table[prev_cluster] = free_cluster;
        if (first_cluster == 0) first_cluster = free_cluster;
        prev_cluster = free_cluster;

        // Write data to cluster
        uint32_t lba = fat16_cluster_to_lba(free_cluster);
        uint32_t to_write = (size - bytes_written > bytes_per_cluster) ? bytes_per_cluster : (size - bytes_written);
        if (!iso_fs_write_sectors(lba, boot_sector.sectors_per_cluster, (uint8_t*)buffer + bytes_written)) {
            // If write failed, free all allocated clusters
            if (first_cluster != 0) {
                uint16_t cluster = first_cluster;
                while (cluster != 0xFFFF && !fat16_is_end_of_chain(cluster)) {
                    uint16_t next_cluster = fat_table[cluster];
                    fat_table[cluster] = 0x0000;  // Mark as free
                    cluster = next_cluster;
                }
            }
            free(root_dir);
            return false;
        }
        bytes_written += to_write;
    }

    // Update directory entry
    file_entry->starting_cluster = first_cluster;
    file_entry->file_size = size;

    // Write back FAT table
    if (!iso_fs_write_sectors(fat_start_sector, sectors_per_fat, fat_table)) {
        free(root_dir);
        return false;
    }
    // Write back root directory
    if (!iso_fs_write_sectors(root_dir_start_sector, root_dir_sectors, root_dir)) {
        free(root_dir);
        return false;
    }
    free(root_dir);
    return true;
}

bool fat16_create_file(const char* filename) {
    // Only support root directory
    fat16_dir_entry_t* root_dir = (fat16_dir_entry_t*)malloc(root_dir_sectors * boot_sector.bytes_per_sector);
    if (!root_dir) return false;
    if (!iso_fs_read_sectors(root_dir_start_sector, root_dir_sectors, root_dir)) {
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
        
        if (compare_filenames(entry_name, filename)) {
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

    // Find a free cluster
    uint16_t free_cluster = find_free_cluster();
    if (free_cluster == 0xFFFF) {
        free(root_dir);
        return false; // No free clusters
    }

    // Parse filename and extension
    char name[8], ext[3];
    parse_filename(filename, name, ext);

    // Write entry
    memcpy(root_dir[free_idx].filename, name, 8);
    memcpy(root_dir[free_idx].extension, ext, 3);
    root_dir[free_idx].attributes = 0;
    memset(root_dir[free_idx].reserved, 0, 10);
    root_dir[free_idx].time = 0;
    root_dir[free_idx].date = 0;
    root_dir[free_idx].starting_cluster = free_cluster;
    root_dir[free_idx].file_size = 0;

    // Mark the cluster as end of chain
    fat_table[free_cluster] = 0xFFF8;

    // Write back FAT table
    if (!iso_fs_write_sectors(fat_start_sector, sectors_per_fat, fat_table)) {
        free(root_dir);
        return false;
    }

    // Write back root directory
    bool ok = iso_fs_write_sectors(root_dir_start_sector, root_dir_sectors, root_dir);
    free(root_dir);
    return ok;
} 