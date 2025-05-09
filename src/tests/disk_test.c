#include "../include/disk.h"
#include "../include/fat16.h"
#include "../include/vga.h"
#include "../include/string.h"
#include <stdint.h>

// Test function to read and display boot sector information
void test_disk_boot_sector(void) {
    uint8_t boot_sector[512];
    
    terminal_writestring("Reading boot sector...\n");
    
    // Read the first sector (boot sector)
    if (disk_read_sectors(0, 0, 1, boot_sector) != 0) {
        terminal_setcolor(VGA_COLOR_RED);
        terminal_writestring("Failed to read boot sector\n");
        terminal_setcolor(VGA_COLOR_WHITE);
        return;
    }

    // Display some basic boot sector information
    FAT16_BootSector* bpb = (FAT16_BootSector*)boot_sector;
    
    terminal_writestring("Boot Sector Information:\n");
    terminal_writestring("------------------------\n");
    
    // Display bytes per sector
    char temp[32];
    uint16_t bytes_per_sector = bpb->BPB_BytsPerSec;
    terminal_writestring("Bytes per sector: ");
    // Simple number to string conversion
    int i = 0;
    if (bytes_per_sector == 0) {
        temp[i++] = '0';
    } else {
        uint16_t n = bytes_per_sector;
        char t[32];
        int c = 0;
        while (n > 0) {
            t[c++] = (n % 10) + '0';
            n /= 10;
        }
        for (int z = c - 1; z >= 0; z--) {
            temp[i++] = t[z];
        }
    }
    temp[i] = '\0';
    terminal_writestring(temp);
    terminal_writestring("\n");

    // Display sectors per cluster
    terminal_writestring("Sectors per cluster: ");
    i = 0;
    if (bpb->BPB_SecPerClus == 0) {
        temp[i++] = '0';
    } else {
        uint8_t n = bpb->BPB_SecPerClus;
        char t[32];
        int c = 0;
        while (n > 0) {
            t[c++] = (n % 10) + '0';
            n /= 10;
        }
        for (int z = c - 1; z >= 0; z--) {
            temp[i++] = t[z];
        }
    }
    temp[i] = '\0';
    terminal_writestring(temp);
    terminal_writestring("\n");

    // Display number of FATs
    terminal_writestring("Number of FATs: ");
    i = 0;
    if (bpb->BPB_NumFATs == 0) {
        temp[i++] = '0';
    } else {
        uint8_t n = bpb->BPB_NumFATs;
        char t[32];
        int c = 0;
        while (n > 0) {
            t[c++] = (n % 10) + '0';
            n /= 10;
        }
        for (int z = c - 1; z >= 0; z--) {
            temp[i++] = t[z];
        }
    }
    temp[i] = '\0';
    terminal_writestring(temp);
    terminal_writestring("\n");

    // Display root directory entries
    terminal_writestring("Root directory entries: ");
    i = 0;
    if (bpb->BPB_RootEntCnt == 0) {
        temp[i++] = '0';
    } else {
        uint16_t n = bpb->BPB_RootEntCnt;
        char t[32];
        int c = 0;
        while (n > 0) {
            t[c++] = (n % 10) + '0';
            n /= 10;
        }
        for (int z = c - 1; z >= 0; z--) {
            temp[i++] = t[z];
        }
    }
    temp[i] = '\0';
    terminal_writestring(temp);
    terminal_writestring("\n");

    // Display file system type
    terminal_writestring("File system type: ");
    for (int i = 0; i < 8; i++) {
        if (bpb->BS_FilSysType[i] != ' ') {
            char c[2] = {bpb->BS_FilSysType[i], '\0'};
            terminal_writestring(c);
        }
    }
    terminal_writestring("\n\n");
}

// Test function to initialize and list FAT16 filesystem
void test_fat16_filesystem(void) {
    FAT16_VolumeInfo vol_info;
    
    terminal_writestring("Initializing FAT16 filesystem...\n");
    
    if (fat16_init(0, &vol_info) != 0) {
        terminal_setcolor(VGA_COLOR_RED);
        terminal_writestring("Failed to initialize FAT16 filesystem\n");
        terminal_setcolor(VGA_COLOR_WHITE);
        return;
    }

    terminal_writestring("FAT16 filesystem initialized successfully\n");
    terminal_writestring("Listing root directory contents:\n");
    terminal_writestring("--------------------------------\n");

    // List root directory contents
    fat16_test_ls_root(&vol_info);
}

// Main test function
void run_disk_tests(void) {
    terminal_writestring("\n=== Disk Driver Tests ===\n\n");
    
    // Test boot sector reading
    test_disk_boot_sector();
    
    // Test FAT16 filesystem
    test_fat16_filesystem();
    
    terminal_writestring("\n=== Disk Tests Complete ===\n\n");
} 