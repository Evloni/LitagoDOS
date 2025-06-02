#include "../../include/fs/fat16.h"
#include "../../include/drivers/iso_fs.h"
#include "../../include/drivers/vbe.h"
#include <string.h>

// Function declarations
static fat16_dir_entry_t* find_directory_entry(fat16_dir_entry_t* dir, int num_entries, const char* name);

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
uint16_t current_cluster = 0;  // Current directory cluster (0 for root)

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
const char* get_file_type(const fat16_dir_entry_t* entry) {
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

    // Print header
    terminal_writestring("Name           Size    Type\n");
    terminal_writestring("----------------------------------------\n");

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
        int name_idx = 0;
        for (int j = 0; j < 8; j++) {
            if (root_dir[i].filename[j] != ' ') {
                name[name_idx++] = root_dir[i].filename[j];
            }
        }
        if (root_dir[i].extension[0] != ' ') {
            name[name_idx++] = '.';
            for (int j = 0; j < 3; j++) {
                if (root_dir[i].extension[j] != ' ') {
                    name[name_idx++] = root_dir[i].extension[j];
                }
            }
        }
        name[name_idx] = '\0';

        // Print name, padded to 16 chars
        terminal_writestring(name);
        int name_len = strlen(name);
        for (int s = name_len; s < 16; s++) {
            terminal_putchar(' ');
        }

        // Print size or blank for directories
        if (root_dir[i].attributes & FAT16_ATTR_DIRECTORY) {
            terminal_writestring("        ");
        } else {
            char size_str[16];
            itoa(root_dir[i].file_size, size_str, 10);
            terminal_writestring(size_str);
            int size_len = strlen(size_str);
            for (int s = size_len; s < 8; s++) {
                terminal_putchar(' ');
            }
        }

        // Print type
        const char* type_str = get_file_type(&root_dir[i]);
        terminal_writestring(type_str);
        terminal_putchar('\n');
    }

    free(root_dir);
    return true;
}

// Read file contents
int fat16_read_file(const char* filename, void* buffer, uint32_t max_size) {
    fat16_dir_entry_t* dir_entries = (fat16_dir_entry_t*)malloc(root_dir_sectors * boot_sector.bytes_per_sector);
    if (!dir_entries) {
        return 0;
    }

    // Read current directory
    if (!fat16_read_directory(current_cluster, dir_entries, boot_sector.root_entries)) {
        free(dir_entries);
        return 0;
    }

    // Find file in directory
    fat16_dir_entry_t* file_entry = find_directory_entry(dir_entries, boot_sector.root_entries, filename);
    if (!file_entry) {
        free(dir_entries);
        return 0;
    }

    // Check for empty file
    if (file_entry->file_size == 0) {
        free(dir_entries);
        return -1; // Special value for empty file
    }

    // Read file data
    uint16_t cluster = file_entry->starting_cluster;
    uint32_t bytes_read = 0;
    uint8_t* data_buffer = (uint8_t*)buffer;

    while (cluster != 0xFFFF && !fat16_is_end_of_chain(cluster)) {
        uint32_t lba = fat16_cluster_to_lba(cluster);
        if (!iso_fs_read_sectors(lba, boot_sector.sectors_per_cluster, data_buffer + bytes_read)) {
            free(dir_entries);
            return 0;
        }
        bytes_read += boot_sector.sectors_per_cluster * boot_sector.bytes_per_sector;
        if (bytes_read >= max_size) break;
        cluster = fat16_get_next_cluster(cluster);
    }

    free(dir_entries);
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
    fat16_dir_entry_t* dir_entries = (fat16_dir_entry_t*)malloc(root_dir_sectors * boot_sector.bytes_per_sector);
    if (!dir_entries) return false;

    // Read current directory
    if (!fat16_read_directory(current_cluster, dir_entries, boot_sector.root_entries)) {
        free(dir_entries);
        return false;
    }

    // Find or create file entry
    fat16_dir_entry_t* file_entry = find_directory_entry(dir_entries, boot_sector.root_entries, filename);
    int file_index = -1;

    // If file exists, free its clusters
    if (file_entry) {
        uint16_t cluster = file_entry->starting_cluster;
        while (cluster != 0xFFFF && !fat16_is_end_of_chain(cluster)) {
            uint16_t next_cluster = fat_table[cluster];
            fat_table[cluster] = 0x0000;  // Mark as free
            cluster = next_cluster;
        }
    } else {
        // Find a free entry
        for (int i = 0; i < boot_sector.root_entries; i++) {
            if (dir_entries[i].filename[0] == 0x00 || dir_entries[i].filename[0] == 0xE5) {
                file_index = i;
                file_entry = &dir_entries[i];
                break;
            }
        }
        if (file_index == -1) {
            free(dir_entries);
            return false; // No free entry
        }
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
            free(dir_entries);
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
            free(dir_entries);
            return false;
        }
        bytes_written += to_write;
    }

    // Update directory entry
    if (file_index != -1) {
        // Parse filename and extension
        char name[8], ext[3];
        parse_filename(filename, name, ext);
        memcpy(file_entry->filename, name, 8);
        memcpy(file_entry->extension, ext, 3);
        file_entry->attributes = 0;
        memset(file_entry->reserved, 0, 10);
        file_entry->time = 0;
        file_entry->date = 0;
    }
    file_entry->starting_cluster = first_cluster;
    file_entry->file_size = size;

    // Write back FAT table
    if (!iso_fs_write_sectors(fat_start_sector, sectors_per_fat, fat_table)) {
        free(dir_entries);
        return false;
    }

    // Write back directory
    if (current_cluster == 0) {
        // Root directory
        if (!iso_fs_write_sectors(root_dir_start_sector, root_dir_sectors, dir_entries)) {
            free(dir_entries);
            return false;
        }
    } else {
        // Subdirectory - write the entire cluster chain
        uint16_t cluster = current_cluster;
        uint32_t bytes_written = 0;
        uint32_t bytes_per_cluster = boot_sector.sectors_per_cluster * boot_sector.bytes_per_sector;
        
        while (cluster != 0xFFFF && !fat16_is_end_of_chain(cluster)) {
            uint32_t lba = fat16_cluster_to_lba(cluster);
            if (!iso_fs_write_sectors(lba, boot_sector.sectors_per_cluster, 
                                   (uint8_t*)dir_entries + bytes_written)) {
                free(dir_entries);
                return false;
            }
            bytes_written += bytes_per_cluster;
            cluster = fat16_get_next_cluster(cluster);
        }
        
        // If we need more clusters for the directory
        if (bytes_written < root_dir_sectors * boot_sector.bytes_per_sector) {
            uint16_t new_cluster = find_free_cluster();
            if (new_cluster == 0xFFFF) {
                free(dir_entries);
                return false;
            }
            
            // Link the new cluster
            fat_table[cluster] = new_cluster;
            fat_table[new_cluster] = 0xFFF8;
            
            // Write the remaining data
            uint32_t lba = fat16_cluster_to_lba(new_cluster);
            if (!iso_fs_write_sectors(lba, boot_sector.sectors_per_cluster, 
                                   (uint8_t*)dir_entries + bytes_written)) {
                free(dir_entries);
                return false;
            }
            
            // Write back FAT table
            if (!iso_fs_write_sectors(fat_start_sector, sectors_per_fat, fat_table)) {
                free(dir_entries);
                return false;
            }
        }
    }

    free(dir_entries);
    return true;
}

bool fat16_create_file(const char* filename, uint16_t current_cluster) {
    fat16_dir_entry_t* dir_entries = (fat16_dir_entry_t*)malloc(root_dir_sectors * boot_sector.bytes_per_sector);
    if (!dir_entries) return false;

    // Read current directory
    if (!fat16_read_directory(current_cluster, dir_entries, boot_sector.root_entries)) {
        free(dir_entries);
        return false;
    }

    // Check if file already exists
    for (int i = 0; i < boot_sector.root_entries; i++) {
        if (dir_entries[i].filename[0] == 0x00) break;
        if (dir_entries[i].filename[0] == 0xE5) continue;
        if ((dir_entries[i].attributes & FAT16_ATTR_LONG_NAME) == FAT16_ATTR_LONG_NAME) continue;
        if (dir_entries[i].attributes & FAT16_ATTR_VOLUME_ID) continue;
        
        // Compare name
        char entry_name[13];
        int name_idx = 0;
        for (int j = 0; j < 8; j++) if (dir_entries[i].filename[j] != ' ') entry_name[name_idx++] = dir_entries[i].filename[j];
        if (dir_entries[i].extension[0] != ' ') {
            entry_name[name_idx++] = '.';
            for (int j = 0; j < 3; j++) if (dir_entries[i].extension[j] != ' ') entry_name[name_idx++] = dir_entries[i].extension[j];
        }
        entry_name[name_idx] = '\0';
        
        if (compare_filenames(entry_name, filename)) {
            free(dir_entries);
            return false; // File already exists
        }
    }

    // Find a free entry
    int free_idx = -1;
    for (int i = 0; i < boot_sector.root_entries; i++) {
        if (dir_entries[i].filename[0] == 0x00 || dir_entries[i].filename[0] == 0xE5) {
            free_idx = i;
            break;
        }
    }
    if (free_idx == -1) {
        free(dir_entries);
        return false; // No free entry
    }

    // Find a free cluster for the new file
    uint16_t free_cluster = find_free_cluster();
    if (free_cluster == 0xFFFF) {
        free(dir_entries);
        return false; // No free clusters
    }

    // Parse filename and extension
    char name[8], ext[3];
    parse_filename(filename, name, ext);

    // Write entry
    memcpy(dir_entries[free_idx].filename, name, 8);
    memcpy(dir_entries[free_idx].extension, ext, 3);
    dir_entries[free_idx].attributes = 0;
    memset(dir_entries[free_idx].reserved, 0, 10);
    dir_entries[free_idx].time = 0;
    dir_entries[free_idx].date = 0;
    dir_entries[free_idx].starting_cluster = free_cluster;
    dir_entries[free_idx].file_size = 0;

    // Mark the cluster as end of chain
    fat_table[free_cluster] = 0xFFF8;

    // Write back FAT table
    if (!iso_fs_write_sectors(fat_start_sector, sectors_per_fat, fat_table)) {
        free(dir_entries);
        return false;
    }

    // Write back directory
    if (current_cluster == 0) {
        // Root directory
        if (!iso_fs_write_sectors(root_dir_start_sector, root_dir_sectors, dir_entries)) {
            free(dir_entries);
            return false;
        }
    } else {
        // Subdirectory handling
        uint32_t entries_per_cluster = (boot_sector.sectors_per_cluster * boot_sector.bytes_per_sector) / sizeof(fat16_dir_entry_t);
        uint32_t target_cluster = current_cluster;
        uint32_t remaining_entries = free_idx;
        
        // Find the cluster containing our entry
        while (remaining_entries >= entries_per_cluster) {
            target_cluster = fat16_get_next_cluster(target_cluster);
            if (target_cluster == 0xFFFF || fat16_is_end_of_chain(target_cluster)) {
                // We need to extend the directory cluster chain
                uint16_t new_cluster = find_free_cluster();
                if (new_cluster == 0xFFFF) {
                    free(dir_entries);
                    return false;
                }
                
                // Link the new cluster
                fat_table[target_cluster] = new_cluster;
                fat_table[new_cluster] = 0xFFF8;
                
                // Update target cluster
                target_cluster = new_cluster;
                
                // Write back FAT table
                if (!iso_fs_write_sectors(fat_start_sector, sectors_per_fat, fat_table)) {
                    free(dir_entries);
                    return false;
                }
                
                break;
            }
            remaining_entries -= entries_per_cluster;
        }
        
        // Calculate offset within the target cluster
        uint32_t offset = remaining_entries * sizeof(fat16_dir_entry_t);
        uint32_t lba = fat16_cluster_to_lba(target_cluster);
        
        // Read the target cluster
        uint8_t* cluster_buffer = (uint8_t*)malloc(boot_sector.sectors_per_cluster * boot_sector.bytes_per_sector);
        if (!cluster_buffer) {
            free(dir_entries);
            return false;
        }
        
        if (!iso_fs_read_sectors(lba, boot_sector.sectors_per_cluster, cluster_buffer)) {
            free(cluster_buffer);
            free(dir_entries);
            return false;
        }
        
        // Copy our new entry to the correct position
        memcpy(cluster_buffer + offset, &dir_entries[free_idx], sizeof(fat16_dir_entry_t));
        
        // Write back the modified cluster
        if (!iso_fs_write_sectors(lba, boot_sector.sectors_per_cluster, cluster_buffer)) {
            free(cluster_buffer);
            free(dir_entries);
            return false;
        }
        
        free(cluster_buffer);
    }

    free(dir_entries);
    return true;
}

// Function to find a directory entry by name
static fat16_dir_entry_t* find_directory_entry(fat16_dir_entry_t* dir, int num_entries, const char* name) {
    for (int i = 0; i < num_entries; i++) {
        if (dir[i].filename[0] == 0x00) break;
        if (dir[i].filename[0] == 0xE5) continue;
        if ((dir[i].attributes & FAT16_ATTR_LONG_NAME) == FAT16_ATTR_LONG_NAME) continue;
        if (dir[i].attributes & FAT16_ATTR_VOLUME_ID) continue;

        // Format entry name
        char entry_name[13] = {0};
        int name_idx = 0;
        for (int j = 0; j < 8; j++) {
            if (dir[i].filename[j] != ' ') {
                entry_name[name_idx++] = dir[i].filename[j];
            }
        }
        if (dir[i].extension[0] != ' ') {
            entry_name[name_idx++] = '.';
            for (int j = 0; j < 3; j++) {
                if (dir[i].extension[j] != ' ') {
                    entry_name[name_idx++] = dir[i].extension[j];
                }
            }
        }
        entry_name[name_idx] = '\0';

        if (compare_filenames(entry_name, name)) {
            return &dir[i];
        }
    }
    return NULL;
}

// Function to read a directory's contents
bool fat16_read_directory(uint16_t cluster, fat16_dir_entry_t* entries, int max_entries) {
    if (cluster == 0) {
        // Root directory
        return iso_fs_read_sectors(root_dir_start_sector, root_dir_sectors, entries);
    }

    // Read directory clusters
    int entry_count = 0;
    uint32_t bytes_read = 0;
    uint32_t bytes_per_cluster = boot_sector.sectors_per_cluster * boot_sector.bytes_per_sector;
    
    while (cluster != 0xFFFF && !fat16_is_end_of_chain(cluster)) {
        uint32_t lba = fat16_cluster_to_lba(cluster);
        
        // Read the entire cluster
        if (!iso_fs_read_sectors(lba, boot_sector.sectors_per_cluster, 
                               (uint8_t*)entries + bytes_read)) {
            return false;
        }
        
        bytes_read += bytes_per_cluster;
        entry_count += bytes_per_cluster / sizeof(fat16_dir_entry_t);
        
        if (entry_count >= max_entries) {
            break;
        }
        
        cluster = fat16_get_next_cluster(cluster);
    }
    
    // Clear any remaining entries
    if (entry_count < max_entries) {
        memset((uint8_t*)entries + bytes_read, 0, 
               (max_entries - entry_count) * sizeof(fat16_dir_entry_t));
    }
    
    return true;
}

// Function to change directory
bool fat16_change_directory(const char* path, uint16_t* current_cluster) {
    if (!path || !current_cluster) return false;

    // Handle root directory
    if (strcmp(path, "/") == 0) {
        *current_cluster = 0;  // 0 represents root directory
        return true;
    }

    // Handle parent directory
    if (strcmp(path, "..") == 0) {
        if (*current_cluster == 0) {
            // Already at root, can't go up
            return false;
        }

        // Read current directory to find .. entry
        fat16_dir_entry_t* dir_entries = (fat16_dir_entry_t*)malloc(root_dir_sectors * boot_sector.bytes_per_sector);
        if (!dir_entries) return false;

        bool success = false;
        if (fat16_read_directory(*current_cluster, dir_entries, boot_sector.root_entries)) {
            // Find the .. entry
            for (int i = 0; i < boot_sector.root_entries; i++) {
                if (dir_entries[i].filename[0] == 0x00) break;
                if (dir_entries[i].filename[0] == 0xE5) continue;
                if ((dir_entries[i].attributes & FAT16_ATTR_LONG_NAME) == FAT16_ATTR_LONG_NAME) continue;
                if (dir_entries[i].attributes & FAT16_ATTR_VOLUME_ID) continue;

                // Check if this is the .. entry
                if (dir_entries[i].filename[0] == '.' && 
                    dir_entries[i].filename[1] == '.' && 
                    dir_entries[i].filename[2] == ' ') {
                    *current_cluster = dir_entries[i].starting_cluster;
                    success = true;
                    break;
                }
            }
        }

        free(dir_entries);
        return success;
    }

    // Read current directory
    fat16_dir_entry_t* dir_entries = (fat16_dir_entry_t*)malloc(root_dir_sectors * boot_sector.bytes_per_sector);
    if (!dir_entries) return false;

    bool success = false;
    if (fat16_read_directory(*current_cluster, dir_entries, boot_sector.root_entries)) {
        // Find the directory entry
        fat16_dir_entry_t* entry = find_directory_entry(dir_entries, boot_sector.root_entries, path);
        if (entry && (entry->attributes & FAT16_ATTR_DIRECTORY)) {
            *current_cluster = entry->starting_cluster;
            success = true;
        }
    }

    free(dir_entries);
    return success;
} 

