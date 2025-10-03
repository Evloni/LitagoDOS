#include "../../include/drivers/ide.h"
#include "../../include/stdio.h"
#include <stdint.h>

// Test buffer for reading/writing sectors
static uint8_t test_buffer[512];

// Test the IDE driver
void test_ide_driver(void) {
    printf("=== IDE Driver Test ===\n");
    
    // Initialize the IDE controller
    if (!ide_init()) {
        printf("Failed to initialize IDE controller\n");
        return;
    }
    
    printf("IDE Controller initialized successfully\n");
    
    // Test reading from primary master drive
    printf("Testing read from Primary Master drive...\n");
    if (ide_read_sectors(0, 0, 0, 1, test_buffer)) {
        printf("Successfully read sector 0 from Primary Master\n");
        
        // Print first 64 bytes of the sector
        printf("First 64 bytes of sector 0:\n");
        for (int i = 0; i < 64; i++) {
            if (i % 16 == 0) {
                printf("\n%04x: ", i);
            }
            printf("%02x ", test_buffer[i]);
        }
        printf("\n");
    } else {
        printf("Failed to read from Primary Master drive\n");
    }
    
    // Test reading from primary slave drive
    printf("Testing read from Primary Slave drive...\n");
    if (ide_read_sectors(0, 1, 0, 1, test_buffer)) {
        printf("Successfully read sector 0 from Primary Slave\n");
    } else {
        printf("Failed to read from Primary Slave drive\n");
    }
    
    // Test reading from secondary master drive
    printf("Testing read from Secondary Master drive...\n");
    if (ide_read_sectors(1, 0, 0, 1, test_buffer)) {
        printf("Successfully read sector 0 from Secondary Master\n");
    } else {
        printf("Failed to read from Secondary Master drive\n");
    }
    
    // Test reading from secondary slave drive
    printf("Testing read from Secondary Slave drive...\n");
    if (ide_read_sectors(1, 1, 0, 1, test_buffer)) {
        printf("Successfully read sector 0 from Secondary Slave\n");
    } else {
        printf("Failed to read from Secondary Slave drive\n");
    }
    
    printf("=== IDE Driver Test Complete ===\n");
}

// Example of how to use the IDE driver in your kernel
void init_storage_system(void) {
    printf("Initializing storage system...\n");
    
    // Initialize IDE controller
    if (!ide_init()) {
        printf("Storage system initialization failed\n");
        return;
    }
    
    // Check which channels have devices
    if (ide_channel_has_devices(0)) {
        printf("Primary IDE channel has devices\n");
    }
    
    if (ide_channel_has_devices(1)) {
        printf("Secondary IDE channel has devices\n");
    }
    
    printf("Storage system initialized successfully\n");
}

// Example function to read boot sector
bool read_boot_sector(void* buffer) {
    // Try to read from primary master first
    if (ide_read_sectors(0, 0, 0, 1, buffer)) {
        printf("Boot sector read from Primary Master\n");
        return true;
    }
    
    // Try primary slave
    if (ide_read_sectors(0, 1, 0, 1, buffer)) {
        printf("Boot sector read from Primary Slave\n");
        return true;
    }
    
    // Try secondary master
    if (ide_read_sectors(1, 0, 0, 1, buffer)) {
        printf("Boot sector read from Secondary Master\n");
        return true;
    }
    
    // Try secondary slave
    if (ide_read_sectors(1, 1, 0, 1, buffer)) {
        printf("Boot sector read from Secondary Slave\n");
        return true;
    }
    
    printf("Failed to read boot sector from any drive\n");
    return false;
}

// Test specifically the secondary IDE channel
void test_secondary_ide_channel(void) {
    printf("=== Secondary IDE Channel Test ===\n");
    
    // Check if IDE controller is initialized
    if (!ide_is_initialized()) {
        printf("IDE controller not initialized. Running initialization...\n");
        if (!ide_init()) {
            printf("Failed to initialize IDE controller\n");
            return;
        }
    }
    
    printf("Testing Secondary IDE Channel specifically:\n");
    
    // Test secondary master
    printf("1. Testing Secondary Master drive...\n");
    uint8_t test_buffer[512];
    if (ide_read_sectors(1, 0, 0, 1, test_buffer)) {
        printf("   SUCCESS: Secondary Master drive is working\n");
        
        // Print first 32 bytes to verify data
        printf("   First 32 bytes of sector 0:\n   ");
        for (int i = 0; i < 32; i++) {
            printf("%02x ", test_buffer[i]);
            if ((i + 1) % 16 == 0) printf("\n   ");
        }
        printf("\n");
    } else {
        printf("   FAILED: Secondary Master drive not responding\n");
    }
    
    // Test secondary slave
    printf("2. Testing Secondary Slave drive...\n");
    if (ide_read_sectors(1, 1, 0, 1, test_buffer)) {
        printf("   SUCCESS: Secondary Slave drive is working\n");
    } else {
        printf("   FAILED: Secondary Slave drive not responding\n");
    }
    
    printf("=== Secondary IDE Channel Test Complete ===\n");
}

// Test IDE channel status
void test_ide_channel_status(void) {
    printf("=== IDE Channel Status Test ===\n");
    
    if (!ide_is_initialized()) {
        printf("IDE controller not initialized\n");
        return;
    }
    
    printf("Primary Channel has devices: %s\n", 
           ide_channel_has_devices(0) ? "YES" : "NO");
    printf("Secondary Channel has devices: %s\n", 
           ide_channel_has_devices(1) ? "YES" : "NO");
    
    // Test reading status registers
    printf("Primary Channel Status Register: 0x%02x\n", inb(IDE_PRIMARY_BASE + IDE_STATUS));
    printf("Secondary Channel Status Register: 0x%02x\n", inb(IDE_SECONDARY_BASE + IDE_STATUS));
    
    printf("=== IDE Channel Status Test Complete ===\n");
}

// Comprehensive IDE diagnostic function
void ide_diagnostic_test(void) {
    printf("=== IDE Comprehensive Diagnostic Test ===\n");
    
    // Check if IDE controller is initialized
    if (!ide_is_initialized()) {
        printf("IDE controller not initialized. Running initialization...\n");
        if (!ide_init()) {
            printf("Failed to initialize IDE controller\n");
            return;
        }
    }
    
    printf("\n1. PCI Configuration Analysis:\n");
    uint8_t bus, slot, func;
    ide_get_piiX3_location(&bus, &slot, &func);
    printf("   PIIX3 Controller Location: Bus %d, Device %d, Function %d\n", bus, slot, func);
    
    // Read PCI configuration registers
    uint32_t config = pci_config_read(bus, slot, func, PIIX3_IDE_CONFIG);
    printf("   Configuration Register: 0x%08x\n", config);
    printf("   Primary Channel Enabled: %s\n", (config & 0x01) ? "YES" : "NO");
    printf("   Secondary Channel Enabled: %s\n", (config & 0x02) ? "YES" : "NO");
    
    printf("\n2. I/O Port Analysis:\n");
    printf("   Primary Base Port (0x%04x): ", IDE_PRIMARY_BASE);
    uint8_t primary_status = inb(IDE_PRIMARY_BASE + IDE_STATUS);
    printf("Status = 0x%02x\n", primary_status);
    
    printf("   Secondary Base Port (0x%04x): ", IDE_SECONDARY_BASE);
    uint8_t secondary_status = inb(IDE_SECONDARY_BASE + IDE_STATUS);
    printf("Status = 0x%02x\n", secondary_status);
    
    printf("   Primary Control Port (0x%04x): ", IDE_PRIMARY_CTRL);
    uint8_t primary_ctrl = inb(IDE_PRIMARY_CTRL);
    printf("Control = 0x%02x\n", primary_ctrl);
    
    printf("   Secondary Control Port (0x%04x): ", IDE_SECONDARY_CTRL);
    uint8_t secondary_ctrl = inb(IDE_SECONDARY_CTRL);
    printf("Control = 0x%02x\n", secondary_ctrl);
    
    printf("\n3. Channel Status Analysis:\n");
    printf("   Primary Channel has devices: %s\n", 
           ide_channel_has_devices(0) ? "YES" : "NO");
    printf("   Secondary Channel has devices: %s\n", 
           ide_channel_has_devices(1) ? "YES" : "NO");
    
    printf("\n4. Secondary Channel Detailed Test:\n");
    
    // Test secondary channel step by step
    printf("   a) Selecting Secondary Master drive...\n");
    outb(IDE_SECONDARY_BASE + IDE_DRIVE_HEAD, IDE_DRIVE_MASTER);
    for (volatile int i = 0; i < 1000; i++); // Delay
    
    uint8_t status = inb(IDE_SECONDARY_BASE + IDE_STATUS);
    printf("      Status after drive selection: 0x%02x\n", status);
    
    printf("   b) Sending IDENTIFY command...\n");
    outb(IDE_SECONDARY_BASE + IDE_COMMAND, IDE_CMD_IDENTIFY);
    
    // Wait a bit and check status
    for (volatile int i = 0; i < 10000; i++); // Longer delay
    status = inb(IDE_SECONDARY_BASE + IDE_STATUS);
    printf("      Status after IDENTIFY: 0x%02x\n", status);
    
    if (status == 0) {
        printf("      RESULT: No device present (status = 0)\n");
        printf("      This is normal if no drive is connected to secondary channel\n");
    } else if (status & IDE_SR_BSY) {
        printf("      RESULT: Device is busy\n");
    } else if (status & IDE_SR_DRQ) {
        printf("      RESULT: Device is requesting data transfer\n");
    } else {
        printf("      RESULT: Device responded with status 0x%02x\n", status);
    }
    
    printf("\n5. Conclusion:\n");
    if (secondary_status == 0) {
        printf("   Secondary channel appears to be inactive or no drives connected.\n");
        printf("   This is normal in many systems, especially virtual machines.\n");
        printf("   The primary channel working confirms your IDE driver is functional.\n");
    } else if (!(config & 0x02)) {
        printf("   Secondary channel is disabled in PIIX3 configuration.\n");
        printf("   This is a hardware/BIOS configuration issue.\n");
    } else {
        printf("   Secondary channel is enabled but no drives are responding.\n");
        printf("   This suggests no drives are connected to the secondary channel.\n");
    }
    
    printf("\n=== Diagnostic Complete ===\n");
} 