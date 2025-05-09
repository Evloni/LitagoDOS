#ifndef _DISK_TEST_H
#define _DISK_TEST_H

// Test function to read and display boot sector information
void test_disk_boot_sector(void);

// Test function to initialize and list FAT16 filesystem
void test_fat16_filesystem(void);

// Run all disk driver tests
void run_disk_tests(void);

#endif // _DISK_TEST_H 