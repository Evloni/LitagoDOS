#ifndef FAT16_H
#define FAT16_H

#include <stdint.h> // For fixed-width integer types like uint8_t, uint16_t, uint32_t

// FAT16 Boot Sector Structure
typedef struct {
    uint8_t     BS_jmpBoot[3];          // Jump instruction to boot code
    uint8_t     BS_OEMName[8];          // OEM name/version
    uint16_t    BPB_BytsPerSec;         // Bytes per sector (usually 512)
    uint8_t     BPB_SecPerClus;         // Sectors per cluster (must be a power of 2)
    uint16_t    BPB_RsvdSecCnt;         // Number of reserved sectors (usually 1 for FAT12/16)
    uint8_t     BPB_NumFATs;            // Number of File Allocation Tables (usually 2)
    uint16_t    BPB_RootEntCnt;         // Max number of root directory entries (e.g., 512)
    uint16_t    BPB_TotSec16;           // Total sectors (if zero, use BPB_TotSec32)
    uint8_t     BPB_Media;              // Media descriptor (e.g., 0xF8 for hard disk)
    uint16_t    BPB_FATSz16;            // Sectors per FAT
    uint16_t    BPB_SecPerTrk;          // Sectors per track (for geometry)
    uint16_t    BPB_NumHeads;           // Number of heads (for geometry)
    uint32_t    BPB_HiddSec;            // Number of hidden sectors
    uint32_t    BPB_TotSec32;           // Total sectors (if BPB_TotSec16 is zero)
    uint8_t     BS_DrvNum;              // Drive number (e.g., 0x80 for hard disk)
    uint8_t     BS_Reserved1;           // Reserved (used by Windows NT)
    uint8_t     BS_BootSig;             // Boot signature (0x29)
    uint32_t    BS_VolID;               // Volume serial number
    uint8_t     BS_VolLab[11];          // Volume label
    uint8_t     BS_FilSysType[8];       // File system type (e.g., "FAT16   ")
    // ... (boot code follows)
} __attribute__((packed)) FAT16_BootSector;

// FAT16 Directory Entry Structure
typedef struct {
    uint8_t     DIR_Name[11];           // File name (8.3 format)
    uint8_t     DIR_Attr;               // File attributes
    uint8_t     DIR_NTRes;              // Reserved for Windows NT
    uint8_t     DIR_CrtTimeTenth;       // Creation time (tenths of a second)
    uint16_t    DIR_CrtTime;            // Creation time
    uint16_t    DIR_CrtDate;            // Creation date
    uint16_t    DIR_LstAccDate;         // Last access date
    uint16_t    DIR_FstClusHI;          // High word of first cluster number (0 for FAT12/16)
    uint16_t    DIR_WrtTime;            // Last write time
    uint16_t    DIR_WrtDate;            // Last write date
    uint16_t    DIR_FstClusLO;          // Low word of first cluster number
    uint32_t    DIR_FileSize;           // File size in bytes
} __attribute__((packed)) FAT16_DirectoryEntry;

// File Attributes (for DIR_Attr)
#define ATTR_READ_ONLY      0x01
#define ATTR_HIDDEN         0x02
#define ATTR_SYSTEM         0x04
#define ATTR_VOLUME_ID      0x08
#define ATTR_DIRECTORY      0x10
#define ATTR_ARCHIVE        0x20
#define ATTR_LONG_NAME      (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)

// Structure to hold parsed FAT16 volume information
typedef struct {
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t  num_fats;
    uint16_t root_entry_count;
    uint16_t sectors_per_fat;
    uint32_t total_sectors;

    // Calculated values
    uint32_t fat_start_sector;
    uint32_t root_dir_start_sector;
    uint32_t root_dir_sectors;
    uint32_t data_start_sector; // First data sector, where cluster 2 begins

    // Identification of the disk/partition
    int      disk_id; // Or some other identifier for the block device
} FAT16_VolumeInfo;

// Function to retrieve a FAT entry for a given cluster number
// Returns the 16-bit FAT entry, or a special value on error (e.g. 0xFFFF if read fails,
// though FAT entries themselves can be 0xFFFF for EOC). Error handling needs care.
// A more robust approach might return an error code via a parameter and the entry via another.
// For now, let's assume 0xFFFF can indicate an error if the cluster number is out of bounds
// or read fails, distinguishing from valid EOC markers (0xFFF8-0xFFFF) requires context.
// Let's aim to return the raw FAT entry and let the caller interpret it.
// A separate error status could be better.
uint16_t fat16_get_fat_entry(FAT16_VolumeInfo* vol_info, uint16_t cluster_number);

// Finds a directory entry by its 8.3 name in the ROOT directory.
// vol_info: Pointer to the initialized FAT16 volume information.
// name_8_3: The 11-byte (space-padded, uppercase) 8.3 filename to search for.
// entry_out: Pointer to a FAT16_DirectoryEntry where the found entry will be copied.
// Returns 0 on success, -1 if not found, other negative for errors.
int fat16_find_entry_in_root(FAT16_VolumeInfo* vol_info, const char name_8_3[11], FAT16_DirectoryEntry* entry_out);

// Lists all valid entries in the ROOT directory (skips LFNs, deleted, volume ID).
// vol_info: Pointer to the initialized FAT16 volume information.
// entries_array: Array (caller-allocated) to store found directory entries.
// max_entries: Maximum capacity of entries_array.
// num_entries_found: Output, actual number of entries found and stored in entries_array.
// Returns 0 on success (even if array fills up), negative for errors.
int fat16_list_root_directory(FAT16_VolumeInfo* vol_info, FAT16_DirectoryEntry entries_array[], int max_entries, int* num_entries_found);

// Reads all sectors of a given data cluster into a buffer.
// vol_info: Pointer to the initialized FAT16 volume information.
// cluster_number: The data cluster number to read (must be >= 2).
// buffer: Caller-allocated buffer to store cluster data. Must be large enough for
//         vol_info->sectors_per_cluster * vol_info->bytes_per_sector bytes.
// Returns 0 on success, negative for errors.
int fat16_read_cluster_data(FAT16_VolumeInfo* vol_info, uint16_t cluster_number, uint8_t* buffer);

// Reads the content of a file into a buffer.
// vol_info: Pointer to the initialized FAT16 volume information.
// file_entry: Pointer to the directory entry of the file to read.
// out_buffer: Caller-allocated buffer to store the file's content.
// buffer_capacity: The maximum size of out_buffer in bytes.
// bytes_actually_read: Output, actual number of bytes read into out_buffer.
// Returns 0 on success, negative for errors.
int fat16_read_file(FAT16_VolumeInfo* vol_info, FAT16_DirectoryEntry* file_entry,
                    uint8_t* out_buffer, uint32_t buffer_capacity, uint32_t* bytes_actually_read);

// Test function to list root directory contents
void fat16_test_ls_root(FAT16_VolumeInfo* vol_info);

#endif // FAT16_H 