#include "../include/fat16.h"
#include <stddef.h> // For NULL
#include "../include/string.h"
#include "../include/disk.h"

// Replace the placeholder with our actual disk driver
static int read_sectors_placeholder(int disk_id, uint32_t lba, uint16_t count, void* buffer) {
    return disk_read_sectors(disk_id, lba, count, buffer);
}

// Initialize FAT16 file system on a given disk/partition.
// Parses the boot sector and populates vol_info.
// Returns 0 on success, or a negative error code on failure.
int fat16_init(int disk_id, FAT16_VolumeInfo* vol_info) {
    if (!vol_info) {
        return -1; // Invalid argument
    }

    uint8_t sector_buffer[512]; // Assuming 512 bytes per sector for the boot sector
                               // A more robust implementation would use BPB_BytsPerSec after reading it,
                               // but the first read must assume a size.

    // Read the first sector (LBA 0 of the partition) which is the Boot Sector
    if (read_sectors_placeholder(disk_id, 0, 1, sector_buffer) != 0) {
        // kprintf("Error: Could not read boot sector from disk %d\n", disk_id);
        return -2; // Disk read error
    }

    FAT16_BootSector* bpb = (FAT16_BootSector*)sector_buffer;

    // --- Basic Validations ---
    if (bpb->BPB_BytsPerSec == 0 || (512 % bpb->BPB_BytsPerSec != 0) ) {
        // kprintf("Error: Invalid BPB_BytsPerSec: %u\n", bpb->BPB_BytsPerSec);
        return -3; // Invalid sector size
    }
    // Typically 512, 1024, 2048, 4096. FAT specification states it must be a power of 2.
    // And at least 512.

    if (bpb->BS_BootSig != 0x29 && bpb->BS_BootSig != 0x28) { // 0x28 is for earlier versions
        // While some systems might not have 0x29, it's a good sanity check for the Extended BPB.
        // Some minimal FAT12/16 might not have it.
        // kprintf("Warning: Boot signature 0x%x is not 0x29 or 0x28. May not be an Extended BPB.\n", bpb->BS_BootSig);
        // For simplicity, we can choose to proceed or fail here.
        // If BS_FilSysType is reliable, that's better.
    }

    // Check for FAT16 signature (less reliable than checking cluster count later)
    // Example: if (strncmp((const char*)bpb->BS_FilSysType, "FAT16   ", 8) != 0) {
    //    kprintf("Error: File system type is not FAT16 ('%.8s')\n", bpb->BS_FilSysType);
    //    return -4; // Not FAT16
    // }
    // A more robust check for FAT16 involves calculating the data cluster count.

    if (bpb->BPB_NumFATs == 0) {
        // kprintf("Error: Number of FATs is zero.\n");
        return -5;
    }

    if (bpb->BPB_SecPerClus == 0 || (bpb->BPB_SecPerClus & (bpb->BPB_SecPerClus - 1)) != 0) {
        // kprintf("Error: Sectors per cluster (%u) is not a power of 2 or is zero.\n", bpb->BPB_SecPerClus);
        return -6; // Invalid SecPerClus
    }


    // --- Populate vol_info ---
    vol_info->disk_id = disk_id;
    vol_info->bytes_per_sector = bpb->BPB_BytsPerSec;
    vol_info->sectors_per_cluster = bpb->BPB_SecPerClus;
    vol_info->reserved_sector_count = bpb->BPB_RsvdSecCnt;
    vol_info->num_fats = bpb->BPB_NumFATs;
    vol_info->root_entry_count = bpb->BPB_RootEntCnt;
    vol_info->sectors_per_fat = bpb->BPB_FATSz16; // For FAT16, BPB_FATSz16 is used

    if (bpb->BPB_TotSec16 != 0) {
        vol_info->total_sectors = bpb->BPB_TotSec16;
    } else {
        vol_info->total_sectors = bpb->BPB_TotSec32;
    }

    // --- Calculate critical offsets and sizes ---

    // The first FAT starts immediately after the reserved sectors.
    // LBA = Logical Block Address (sector number)
    vol_info->fat_start_sector = vol_info->reserved_sector_count;

    // The Root Directory starts immediately after all FAT copies.
    vol_info->root_dir_start_sector = vol_info->fat_start_sector +
                                     (vol_info->num_fats * vol_info->sectors_per_fat);

    // Calculate how many sectors the Root Directory occupies.
    // Each directory entry is 32 bytes.
    uint32_t root_dir_size_bytes = vol_info->root_entry_count * 32;
    vol_info->root_dir_sectors = (root_dir_size_bytes + vol_info->bytes_per_sector - 1) / vol_info->bytes_per_sector; // Ceil division

    // The Data Region (where file clusters begin) starts immediately after the Root Directory.
    vol_info->data_start_sector = vol_info->root_dir_start_sector + vol_info->root_dir_sectors;

    // --- FAT Type Determination (more robustly than BS_FilSysType) ---
    // Number of clusters in the data region determines FAT type.
    // First, calculate total data sectors
    uint32_t data_sectors = vol_info->total_sectors -
                           (vol_info->reserved_sector_count +
                            (vol_info->num_fats * vol_info->sectors_per_fat) +
                            vol_info->root_dir_sectors);

    // Then, calculate total clusters
    // Cluster numbers start from 2. Cluster 0 and 1 are reserved.
    uint32_t total_clusters = data_sectors / vol_info->sectors_per_cluster;

    // According to Microsoft's FAT spec:
    // FAT12: if CountofClusters < 4085
    // FAT16: if CountofClusters >= 4085 and < 65525
    // FAT32: if CountofClusters >= 65525
    // (Note: Max cluster number for FAT16 is 0xFFF4 (65524). Some sources say up to 0xFFF6)
    // Values 0xFFF0-0xFFF6 are typically reserved, 0xFFF7 is bad cluster, 0xFFF8-0xFFFF is EOC.
    // So, effective max data clusters is slightly less than 65525.
    if (total_clusters < 4085) {
        // kprintf("Warning: Calculated cluster count (%u) suggests FAT12, not FAT16.\n", total_clusters);
        // return -7; // Or handle as FAT12 if you plan to support it.
    } else if (total_clusters >= 65525) {
        // kprintf("Warning: Calculated cluster count (%u) suggests FAT32, not FAT16.\n", total_clusters);
        // return -8; // Or handle as FAT32.
    }
    // If BS_FilSysType was checked and matched "FAT16   ", this gives further confirmation.

    // kprintf("FAT16 Volume Initialized:\n");
    // kprintf("  Bytes/Sector: %u\n", vol_info->bytes_per_sector);
    // kprintf("  Sectors/Cluster: %u\n", vol_info->sectors_per_cluster);
    // kprintf("  Total Sectors: %u\n", vol_info->total_sectors);
    // kprintf("  FAT Start Sector: %u\n", vol_info->fat_start_sector);
    // kprintf("  Root Dir Start Sector: %u\n", vol_info->root_dir_start_sector);
    // kprintf("  Root Dir Sectors: %u\n", vol_info->root_dir_sectors);
    // kprintf("  Data Start Sector: %u\n", vol_info->data_start_sector);
    // kprintf("  Total Data Clusters: %u\n", total_clusters);

    return 0; // Success
}

// You might also want a function to "mount" a FAT16 volume, which could
// call fat16_init and store the FAT16_VolumeInfo in a global or filesystem-specific structure.
// For example:
// static FAT16_VolumeInfo mounted_volume;
// int fat16_mount(int disk_id, const char* mount_point_name) {
//     int result = fat16_init(disk_id, &mounted_volume);
//     if (result == 0) {
//         // Store mount_point_name and associate with mounted_volume
//         // kprintf("FAT16 volume from disk %d mounted at %s\n", disk_id, mount_point_name);
//     }
//     return result;
// }

// Reads a 16-bit entry from the FAT.
// vol_info: Pointer to the initialized FAT16 volume information.
// cluster_number: The cluster whose FAT entry is to be read.
// Returns: The 16-bit FAT entry.
// IMPORTANT: Error handling for disk reads needs to be robust.
// This simplified version assumes read_sectors_placeholder can signal error,
// and might return an ambiguous value if not handled carefully by the caller.
// A production version might use a status pointer argument.
uint16_t fat16_get_fat_entry(FAT16_VolumeInfo* vol_info, uint16_t cluster_number) {
    if (!vol_info) {
        // kprintf("fat16_get_fat_entry: vol_info is NULL\n");
        return 0xFFFF; // Error indication (ambiguous with EOC, needs careful handling)
    }

    // FAT16 entries are 2 bytes.
    uint32_t fat_offset_bytes = cluster_number * 2;

    // Calculate which sector of the FAT contains this entry.
    uint32_t fat_sector_for_entry = vol_info->fat_start_sector + (fat_offset_bytes / vol_info->bytes_per_sector);
    uint32_t offset_in_fat_sector = fat_offset_bytes % vol_info->bytes_per_sector;

    // Ensure we don't read past the end of a FAT.
    // (vol_info->sectors_per_fat * vol_info->bytes_per_sector) is the size of one FAT in bytes.
    if (fat_offset_bytes >= (vol_info->sectors_per_fat * vol_info->bytes_per_sector)) {
        // kprintf("fat16_get_fat_entry: Cluster number %u out of FAT bounds\n", cluster_number);
        return 0xFFFF; // Error: cluster number too high
    }

    // Buffer to hold one sector from the FAT
    // Should be dynamically sized or use vol_info->bytes_per_sector
    uint8_t fat_sector_buffer[512]; // Assuming 512 for now, use vol_info->bytes_per_sector

    if (vol_info->bytes_per_sector > sizeof(fat_sector_buffer)) {
        // kprintf("fat16_get_fat_entry: Sector size %u too large for buffer\n", vol_info->bytes_per_sector);
        // This indicates a need for dynamic allocation or a larger static buffer
        return 0xFFFF; // Error
    }

    // Read the required FAT sector (from the first FAT)
    // TODO: Implement FAT sector caching for performance if needed later.
    if (read_sectors_placeholder(vol_info->disk_id, fat_sector_for_entry, 1, fat_sector_buffer) != 0) {
        // kprintf("fat16_get_fat_entry: Failed to read FAT sector %u for cluster %u\n", fat_sector_for_entry, cluster_number);
        return 0xFFFF; // Disk read error (ambiguous)
    }

    // Extract the 16-bit entry (FAT is little-endian)
    // The pointer arithmetic here directly accesses the 16-bit value at the correct offset.
    uint16_t entry_value = *((uint16_t*)&fat_sector_buffer[offset_in_fat_sector]);

    return entry_value;
}


// Finds a directory entry by its 8.3 name in the ROOT directory.
int fat16_find_entry_in_root(FAT16_VolumeInfo* vol_info, const char name_8_3[11], FAT16_DirectoryEntry* entry_out) {
    if (!vol_info || !name_8_3 || !entry_out) {
        return -2; // Invalid arguments
    }

    uint8_t sector_buffer[512]; // Assuming 512 bytes/sector, adjust if necessary or use vol_info
    if (vol_info->bytes_per_sector > sizeof(sector_buffer)) {
        // kprintf("fat16_find_entry_in_root: Sector size too large for buffer.\n");
        return -3; // Buffer too small
    }

    unsigned int entries_per_sector = vol_info->bytes_per_sector / sizeof(FAT16_DirectoryEntry);
    unsigned int root_entries_processed = 0;

    for (uint32_t i = 0; i < vol_info->root_dir_sectors; ++i) {
        uint32_t current_sector = vol_info->root_dir_start_sector + i;
        if (read_sectors_placeholder(vol_info->disk_id, current_sector, 1, sector_buffer) != 0) {
            // kprintf("fat16_find_entry_in_root: Failed to read root directory sector %u\n", current_sector);
            return -4; // Disk read error
        }

        FAT16_DirectoryEntry* dir_entries = (FAT16_DirectoryEntry*)sector_buffer;

        for (unsigned int j = 0; j < entries_per_sector; ++j) {
            if (root_entries_processed >= vol_info->root_entry_count) {
                return -1; // Searched all possible root entries
            }
            root_entries_processed++;

            FAT16_DirectoryEntry current_entry = dir_entries[j];

            if (current_entry.DIR_Name[0] == 0x00) { // End of directory
                return -1; // Not found, and no more entries
            }
            if ((uint8_t)current_entry.DIR_Name[0] == 0xE5) { // Unused entry
                continue;
            }
            if ((current_entry.DIR_Attr & ATTR_LONG_NAME) == ATTR_LONG_NAME) { // LFN entry
                continue;
            }

            // Compare the 11-byte name
            if (strncmp((const char*)current_entry.DIR_Name, name_8_3, 11) == 0) {
                *entry_out = current_entry; // Copy the found entry
                return 0; // Success
            }
        }
    }
    return -1; // Not found after checking all root directory sectors
}

// Lists all valid entries in the ROOT directory (skips LFNs, deleted, volume ID).
int fat16_list_root_directory(FAT16_VolumeInfo* vol_info, FAT16_DirectoryEntry entries_array[], int max_entries, int* num_entries_found) {
    if (!vol_info || !entries_array || !num_entries_found || max_entries <= 0) {
        if (num_entries_found) *num_entries_found = 0;
        return -2; // Invalid arguments
    }

    *num_entries_found = 0; // Initialize count

    // Buffer to hold one sector.
    // Using a fixed-size buffer. A more robust solution might use vol_info->bytes_per_sector
    // and potentially dynamic allocation if sector sizes can vary widely and be very large.
    uint8_t sector_buffer[512];
    if (vol_info->bytes_per_sector > sizeof(sector_buffer)) {
        // kprintf("fat16_list_root_directory: Sector size %u exceeds buffer size %u.\n",
        //         vol_info->bytes_per_sector, sizeof(sector_buffer));
        return -3; // Buffer too small for configured sector size
    }

    unsigned int entries_per_sector = vol_info->bytes_per_sector / sizeof(FAT16_DirectoryEntry);
    unsigned int entries_scanned_in_root = 0; // Counter for entries scanned against BPB_RootEntCnt

    for (uint32_t i = 0; i < vol_info->root_dir_sectors; ++i) {
        if (*num_entries_found >= max_entries) {
            break; // Output array is full, no need to read more sectors
        }

        uint32_t current_sector_lba = vol_info->root_dir_start_sector + i;
        if (read_sectors_placeholder(vol_info->disk_id, current_sector_lba, 1, sector_buffer) != 0) {
            // kprintf("fat16_list_root_directory: Failed to read root directory sector %u\n", current_sector_lba);
            return -4; // Disk read error
        }

        FAT16_DirectoryEntry* dir_entries_in_sector = (FAT16_DirectoryEntry*)sector_buffer;

        for (unsigned int j = 0; j < entries_per_sector; ++j) {
            // Stop if we've scanned the maximum number of entries allowed in the root directory
            if (entries_scanned_in_root >= vol_info->root_entry_count) {
                return 0; // All potential root entries processed
            }
            entries_scanned_in_root++;

            FAT16_DirectoryEntry current_entry = dir_entries_in_sector[j];

            if (current_entry.DIR_Name[0] == 0x00) { // End of directory marker
                return 0; // No more valid entries in the directory
            }
            if ((uint8_t)current_entry.DIR_Name[0] == 0xE5) { // Unused (deleted) entry
                continue;
            }
            if ((current_entry.DIR_Attr & ATTR_VOLUME_ID) == ATTR_VOLUME_ID) { // Volume label
                continue;
            }
            if ((current_entry.DIR_Attr & ATTR_LONG_NAME) == ATTR_LONG_NAME) { // Long File Name entry
                continue;
            }

            // This is a valid, short filename entry
            if (*num_entries_found < max_entries) {
                entries_array[*num_entries_found] = current_entry;
                (*num_entries_found)++;
            } else {
                // Array is full, but we successfully processed entries up to this point.
                // The caller can check if *num_entries_found == max_entries to know if the list might be truncated.
                return 0;
            }
        }
    }
    return 0; // Successfully processed all allocated root directory sectors
}

// Reads all sectors of a given data cluster into a buffer.
int fat16_read_cluster_data(FAT16_VolumeInfo* vol_info, uint16_t cluster_number, uint8_t* buffer) {
    if (!vol_info || !buffer) {
        return -1; // Invalid arguments
    }

    // Cluster numbers for data start at 2.
    // Clusters 0 and 1 are reserved and should not be passed to this function
    // for reading data content.
    if (cluster_number < 2) {
        // kprintf("fat16_read_cluster_data: Invalid cluster number %u (must be >= 2)\n", cluster_number);
        return -2; // Invalid cluster number for data region
    }

    // Calculate the LBA of the first sector of the specified cluster.
    // The data_start_sector points to where cluster 2 begins.
    // So, (cluster_number - 2) gives the offset in terms of number of clusters from the start.
    uint32_t first_sector_of_cluster = vol_info->data_start_sector +
                                       ((uint32_t)cluster_number - 2) * vol_info->sectors_per_cluster;

    // Number of sectors to read for this one cluster
    uint16_t sectors_to_read = vol_info->sectors_per_cluster;

    // Read all sectors for the cluster
    if (read_sectors_placeholder(vol_info->disk_id, first_sector_of_cluster, sectors_to_read, buffer) != 0) {
        // kprintf("fat16_read_cluster_data: Failed to read cluster %u (LBA: %u, Sectors: %u)\n",
        //         cluster_number, first_sector_of_cluster, sectors_to_read);
        return -3; // Disk read error
    }

    return 0; // Success
}

// Helper for minimum of two uint32_t
static inline uint32_t min_u32(uint32_t a, uint32_t b) {
    return a < b ? a : b;
}

// Reads the content of a file.
int fat16_read_file(FAT16_VolumeInfo* vol_info, FAT16_DirectoryEntry* file_entry,
                    uint8_t* out_buffer, uint32_t buffer_capacity, uint32_t* bytes_actually_read) {

    if (!vol_info || !file_entry || !out_buffer || !bytes_actually_read) {
        if (bytes_actually_read) *bytes_actually_read = 0;
        return -1; // Invalid arguments
    }

    *bytes_actually_read = 0;
    uint32_t total_bytes_read_for_file = 0;

    // If file size is 0 or buffer capacity is 0, nothing to read.
    if (file_entry->DIR_FileSize == 0 || buffer_capacity == 0) {
        return 0; // Success, 0 bytes read
    }

    // Check if entry is a directory, which we can't "read" as a flat file this way.
    if ((file_entry->DIR_Attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
        // kprintf("fat16_read_file: Attempted to read a directory as a file.\n");
        return -2; // Is a directory
    }

    uint16_t current_cluster = file_entry->DIR_FstClusLO;

    // Buffer for reading one sector at a time.
    uint8_t sector_read_buffer[512]; // Assuming 512, common sector size.
    if (vol_info->bytes_per_sector > sizeof(sector_read_buffer)) {
        // kprintf("fat16_read_file: Sector size %u exceeds internal buffer %u.\n",
        //         vol_info->bytes_per_sector, sizeof(sector_read_buffer));
        return -3; // Sector size too large for static buffer
    }

    // Loop through cluster chain
    // Valid data clusters are 0x0002 to 0xFFF6 for FAT16.
    // 0xFFF7 is bad cluster. 0xFFF8-0xFFFF is End-Of-Chain.
    while (current_cluster >= 0x0002 && current_cluster <= 0xFFF6) {
        if (total_bytes_read_for_file >= file_entry->DIR_FileSize || total_bytes_read_for_file >= buffer_capacity) {
            break; // Reached end of file or filled buffer
        }

        uint32_t first_sector_of_cluster = vol_info->data_start_sector +
                                           ((uint32_t)current_cluster - 2) * vol_info->sectors_per_cluster;

        // Read sector by sector within the current cluster
        for (uint16_t s = 0; s < vol_info->sectors_per_cluster; ++s) {
            if (total_bytes_read_for_file >= file_entry->DIR_FileSize || total_bytes_read_for_file >= buffer_capacity) {
                break; // Double check: end of file or filled buffer
            }

            uint32_t lba_to_read = first_sector_of_cluster + s;
            if (read_sectors_placeholder(vol_info->disk_id, lba_to_read, 1, sector_read_buffer) != 0) {
                // kprintf("fat16_read_file: Failed to read sector %u from cluster %u\n", lba_to_read, current_cluster);
                // *bytes_actually_read = total_bytes_read_for_file; // Return partial read
                return -4; // Disk read error
            }

            uint32_t bytes_remaining_in_file = file_entry->DIR_FileSize - total_bytes_read_for_file;
            uint32_t space_left_in_out_buffer = buffer_capacity - total_bytes_read_for_file;
            
            uint32_t bytes_to_copy_this_sector = vol_info->bytes_per_sector;
            bytes_to_copy_this_sector = min_u32(bytes_to_copy_this_sector, bytes_remaining_in_file);
            bytes_to_copy_this_sector = min_u32(bytes_to_copy_this_sector, space_left_in_out_buffer);

            if (bytes_to_copy_this_sector > 0) {
                 // memcpy is in string.h, which is included as ../include/string.h
                memcpy(out_buffer + total_bytes_read_for_file, sector_read_buffer, bytes_to_copy_this_sector);
                total_bytes_read_for_file += bytes_to_copy_this_sector;
            }
        } // End of sector loop for this cluster

        // Get next cluster from FAT
        current_cluster = fat16_get_fat_entry(vol_info, current_cluster);
    } // End of cluster chain loop

    *bytes_actually_read = total_bytes_read_for_file;
    return 0; // Success
}

// --- Test/Utility functions (Example: ls for root) ---

// Helper to format 8.3 filename
// input_8_3_name: The 11-byte DIR_Name field from FAT16_DirectoryEntry
// output_buffer:  A char buffer to store the null-terminated formatted name (e.g., at least 13 bytes: 8 + '.' + 3 + '\0')
static void format_fat16_filename(const uint8_t input_8_3_name[11], char* output_buffer) {
    int out_idx = 0;
    // Copy name part, skip trailing spaces
    for (int i = 0; i < 8; ++i) {
        if (input_8_3_name[i] == ' ') {
            break; // Stop at first space in name part
        }
        output_buffer[out_idx++] = input_8_3_name[i];
    }

    // Check if there's an extension (first char of ext part is not space)
    if (input_8_3_name[8] != ' ') {
        output_buffer[out_idx++] = '.';
        for (int i = 0; i < 3; ++i) {
            if (input_8_3_name[8 + i] == ' ') {
                break; // Stop at first space in extension part
            }
            output_buffer[out_idx++] = input_8_3_name[8 + i];
        }
    }
    output_buffer[out_idx] = '\0'; // Null-terminate
}

// Example test function to list the root directory.
// Uses terminal_writestring for output.
void fat16_test_ls_root(FAT16_VolumeInfo* vol_info) {
    if (!vol_info || vol_info->bytes_per_sector == 0) { // Basic check if vol_info is initialized
        terminal_writestring("Error: FAT16 volume not initialized or invalid.\n");
        return;
    }

    terminal_writestring("Listing root directory:\n");
    terminal_writestring("Type    Size       Name\n");
    terminal_writestring("-------------------------------------\n");

    #define MAX_ROOT_ENTRIES_TO_LIST 64
    FAT16_DirectoryEntry entries[MAX_ROOT_ENTRIES_TO_LIST];
    int num_found = 0;

    int result = fat16_list_root_directory(vol_info, entries, MAX_ROOT_ENTRIES_TO_LIST, &num_found);

    if (result != 0) {
        terminal_writestring("Error listing root directory. Code: ");
        // TODO: Convert 'result' (int) to string and print here if needed, or handle error differently.
        terminal_writestring("\n"); 
        return;
    }

    if (num_found == 0) {
        terminal_writestring("(empty)\n");
        return;
    }

    char formatted_name[13]; // Max 8.3 name + null = 8 + . + 3 + \0 = 13
    char size_str[11]; // Buffer for file size string (max 10 digits for uint32_t + null)

    for (int i = 0; i < num_found; ++i) {
        format_fat16_filename(entries[i].DIR_Name, formatted_name);

        const char* type_str = "FILE "; // Added space for basic alignment
        if ((entries[i].DIR_Attr & ATTR_DIRECTORY) == ATTR_DIRECTORY) {
            type_str = "<DIR> ";
        }
        
        terminal_writestring(type_str);
        terminal_writestring("  "); // Spacing

        // TODO: Implement or use a uint32_to_string function here for entries[i].DIR_FileSize
        // Example: uint32_to_string(entries[i].DIR_FileSize, size_str, 10);
        // For now, printing a placeholder for size:
        if ((entries[i].DIR_Attr & ATTR_DIRECTORY) != ATTR_DIRECTORY) {
             // Crude way to show size for now, replace with itoa/sprintf functionality
            if (entries[i].DIR_FileSize < 100000) { // Simple placeholder
                char temp_size[6]; int k=0; 
                if(entries[i].DIR_FileSize == 0) { temp_size[k++] = '0'; }
                else { 
                   uint32_t n = entries[i].DIR_FileSize; 
                   char t[6]; int c=0; while(n>0){t[c++]=(n%10)+'0'; n/=10;} 
                   for(int z=c-1;z>=0;z--) temp_size[k++]=t[z];
                }
                temp_size[k]='\0'; terminal_writestring(temp_size); 
                for(int pad=0; pad < (10-k); ++pad) terminal_writestring(" ");
            } else { terminal_writestring(">100KB    "); }
        } else {
            terminal_writestring("          "); // Placeholder for <DIR> size
        }

        terminal_writestring(formatted_name);
        terminal_writestring("\n");
    }
} 