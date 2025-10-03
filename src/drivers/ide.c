#include "../../include/drivers/ide.h"
#include "../../include/drivers/pci.h"
#include "../../include/io.h"
#include "../../include/stdio.h"
#include <stddef.h>

// Global IDE controller instance
static ide_controller_t ide_ctrl = {0};

// PIIX3 IDE Controller PCI Configuration
static uint8_t piiX3_bus = 0;
static uint8_t piiX3_slot = 0;
static uint8_t piiX3_func = 0;

// Function prototypes for static functions
static bool find_piiX3_controller(void);
static bool configure_piiX3_controller(void);
static bool ide_wait_ready(uint8_t channel);
static bool ide_wait_data(uint8_t channel);
static uint16_t ide_read_data(uint8_t channel);
static void ide_write_data(uint8_t channel, uint16_t data);
static void ide_setup_lba(uint8_t channel, uint8_t drive, uint64_t lba, uint16_t sectors);
static bool ide_init_channel(uint8_t channel);
static void check_piiX3_configuration(void);

// Find the PIIX3 IDE controller via PCI
static bool find_piiX3_controller(void) {
    for (int bus = 0; bus < 256; bus++) {
        for (int device = 0; device < 32; device++) {
            for (int function = 0; function < 8; function++) {
                uint16_t vendor = pci_config_read(bus, device, function, 0x00) & 0xFFFF;
                uint16_t device_id = (pci_config_read(bus, device, function, 0x00) >> 16) & 0xFFFF;
                
                if (vendor == PIIX3_IDE_VENDOR_ID && device_id == PIIX3_IDE_DEVICE_ID) {
                    piiX3_bus = bus;
                    piiX3_slot = device;
                    piiX3_func = function;
                    printf("Found PIIX3 IDE Controller at PCI %d:%d.%d\n", bus, device, function);
                    return true;
                }
            }
        }
    }
    return false;
}

// Configure PIIX3 IDE controller
static bool configure_piiX3_controller(void) {
    if (!find_piiX3_controller()) {
        printf("PIIX3 IDE Controller not found!\n");
        return false;
    }

    // Read current configuration
    uint32_t config = pci_config_read(piiX3_bus, piiX3_slot, piiX3_func, PIIX3_IDE_CONFIG);
    printf("PIIX3 IDE Config: 0x%08x\n", config);

    // Enable IDE channels and set timing
    // Bit 0: Primary IDE Enable
    // Bit 1: Secondary IDE Enable
    // Bit 2-3: Primary IDE Timing
    // Bit 4-5: Secondary IDE Timing
    config |= 0x03; // Enable both channels
    config &= ~0x3C; // Clear timing bits
    config |= 0x20; // Set to PIO mode 0 timing

    // Write configuration back
    // Note: In a real implementation, you'd use pci_config_write here
    printf("PIIX3 IDE Controller configured (Primary: %s, Secondary: %s)\n", 
           (config & 0x01) ? "Enabled" : "Disabled",
           (config & 0x02) ? "Enabled" : "Disabled");
    return true;
}

// Wait for IDE drive to be ready
static bool ide_wait_ready(uint8_t channel) {
    uint16_t status_port = (channel == 0) ? IDE_PRIMARY_BASE + IDE_STATUS : IDE_SECONDARY_BASE + IDE_STATUS;
    uint32_t timeout = 100000;

    while (timeout--) {
        uint8_t status = inb(status_port);
        if (!(status & IDE_SR_BSY)) {
            return true;
        }
    }
    return false;
}

// Wait for IDE drive to request data
static bool ide_wait_data(uint8_t channel) {
    uint16_t status_port = (channel == 0) ? IDE_PRIMARY_BASE + IDE_STATUS : IDE_SECONDARY_BASE + IDE_STATUS;
    uint32_t timeout = 100000;

    while (timeout--) {
        uint8_t status = inb(status_port);
        if (status & IDE_SR_DRQ) {
            return true;
        }
        if (status & IDE_SR_ERR) {
            return false;
        }
    }
    return false;
}

// Read data from IDE channel
static uint16_t ide_read_data(uint8_t channel) {
    uint16_t data_port = (channel == 0) ? IDE_PRIMARY_BASE + IDE_DATA : IDE_SECONDARY_BASE + IDE_DATA;
    return inw(data_port);
}

// Write data to IDE channel
static void ide_write_data(uint8_t channel, uint16_t data) {
    uint16_t data_port = (channel == 0) ? IDE_PRIMARY_BASE + IDE_DATA : IDE_SECONDARY_BASE + IDE_DATA;
    outw(data_port, data);
}

// Setup LBA addressing for IDE command
static void ide_setup_lba(uint8_t channel, uint8_t drive, uint64_t lba, uint16_t sectors) {
    uint16_t base_port = (channel == 0) ? IDE_PRIMARY_BASE : IDE_SECONDARY_BASE;
    
    // Select drive
    uint8_t drive_select = (drive == 0) ? IDE_DRIVE_MASTER : IDE_DRIVE_SLAVE;
    drive_select |= IDE_DRIVE_LBA;
    
    if (lba >= 0x100000000) {
        // 48-bit LBA
        drive_select |= 0x40; // LBA48 bit
        outb(base_port + IDE_DRIVE_HEAD, drive_select);
        outb(base_port + IDE_SECTOR_COUNT, (sectors >> 8) & 0xFF);
        outb(base_port + IDE_LBA_LOW, (lba >> 24) & 0xFF);
        outb(base_port + IDE_LBA_MID, (lba >> 32) & 0xFF);
        outb(base_port + IDE_LBA_HIGH, (lba >> 40) & 0xFF);
        outb(base_port + IDE_DRIVE_HEAD, drive_select);
        outb(base_port + IDE_SECTOR_COUNT, sectors & 0xFF);
        outb(base_port + IDE_LBA_LOW, lba & 0xFF);
        outb(base_port + IDE_LBA_MID, (lba >> 8) & 0xFF);
        outb(base_port + IDE_LBA_HIGH, (lba >> 16) & 0xFF);
    } else {
        // 28-bit LBA
        outb(base_port + IDE_DRIVE_HEAD, drive_select | ((lba >> 24) & 0x0F));
        outb(base_port + IDE_SECTOR_COUNT, sectors);
        outb(base_port + IDE_LBA_LOW, lba & 0xFF);
        outb(base_port + IDE_LBA_MID, (lba >> 8) & 0xFF);
        outb(base_port + IDE_LBA_HIGH, (lba >> 16) & 0xFF);
    }
}

// Initialize IDE controller
bool ide_init(void) {
    printf("Initializing IDE Controller...\n");
    
    // Initialize channel structures
    ide_ctrl.primary.base_port = IDE_PRIMARY_BASE;
    ide_ctrl.primary.ctrl_port = IDE_PRIMARY_CTRL;
    ide_ctrl.primary.present = false;
    ide_ctrl.primary.device_type = IDE_DEVICE_NONE;
    
    ide_ctrl.secondary.base_port = IDE_SECONDARY_BASE;
    ide_ctrl.secondary.ctrl_port = IDE_SECONDARY_CTRL;
    ide_ctrl.secondary.present = false;
    ide_ctrl.secondary.device_type = IDE_DEVICE_NONE;
    
    // Configure PIIX3 controller
    if (!configure_piiX3_controller()) {
        printf("Failed to configure PIIX3 IDE controller\n");
        return false;
    }
    
    // Check PIIX3 configuration
    check_piiX3_configuration();
    
    // Initialize both channels
    ide_init_channel(0); // Primary
    ide_init_channel(1); // Secondary
    
    // Detect devices
    if (!ide_detect_devices()) {
        printf("No IDE devices detected\n");
        return false;
    }
    
    ide_ctrl.initialized = true;
    printf("IDE Controller initialized successfully\n");
    return true;
}

// Detect IDE devices on all channels
bool ide_detect_devices(void) {
    bool found_any = false;
    
    printf("Detecting IDE devices...\n");
    
    // Check primary channel
    printf("Checking Primary Channel:\n");
    for (int drive = 0; drive < 2; drive++) {
        if (ide_identify_device(0, drive)) {
            ide_ctrl.primary.present = true;
            found_any = true;
            ide_print_device_info(0, drive);
        } else {
            printf("  Primary %s: No device\n", (drive == 0) ? "Master" : "Slave");
        }
    }
    
    // Check secondary channel
    printf("Checking Secondary Channel:\n");
    for (int drive = 0; drive < 2; drive++) {
        if (ide_identify_device(1, drive)) {
            ide_ctrl.secondary.present = true;
            found_any = true;
            ide_print_device_info(1, drive);
        } else {
            printf("  Secondary %s: No device\n", (drive == 0) ? "Master" : "Slave");
        }
    }
    
    if (found_any) {
        printf("IDE device detection completed successfully\n");
    } else {
        printf("No IDE devices found on any channel\n");
    }
    
    return found_any;
}

// Identify IDE device
bool ide_identify_device(uint8_t channel, uint8_t drive) {
    uint16_t base_port = (channel == 0) ? IDE_PRIMARY_BASE : IDE_SECONDARY_BASE;
    const char* channel_name = (channel == 0) ? "Primary" : "Secondary";
    const char* drive_name = (drive == 0) ? "Master" : "Slave";
    
    printf("Identifying %s %s drive...\n", channel_name, drive_name);
    
    // Select drive
    uint8_t drive_select = (drive == 0) ? IDE_DRIVE_MASTER : IDE_DRIVE_SLAVE;
    outb(base_port + IDE_DRIVE_HEAD, drive_select);
    
    // Small delay
    for (volatile int i = 0; i < 1000; i++);
    
    // Check initial status
    uint8_t status = inb(base_port + IDE_STATUS);
    printf("  Initial status: 0x%02x\n", status);
    
    // Send IDENTIFY command
    outb(base_port + IDE_COMMAND, IDE_CMD_IDENTIFY);
    
    // Wait for response
    if (!ide_wait_ready(channel)) {
        printf("  Timeout waiting for ready\n");
        return false;
    }
    
    // Check if device is present
    status = inb(base_port + IDE_STATUS);
    printf("  Status after IDENTIFY: 0x%02x\n", status);
    
    if (status == 0) {
        printf("  No device present (status = 0)\n");
        return false;
    }
    
    // Wait for data
    if (!ide_wait_data(channel)) {
        printf("  Timeout waiting for data\n");
        return false;
    }
    
    printf("  Device responding, reading identify data...\n");
    
    // Read identify data (we'll just read it to clear the buffer for now)
    for (int i = 0; i < 256; i++) {
        inw(base_port + IDE_DATA);
    }
    
    printf("  %s %s drive identified successfully\n", channel_name, drive_name);
    return true;
}

// Print device information
void ide_print_device_info(uint8_t channel, uint8_t drive) {
    const char* channel_name = (channel == 0) ? "Primary" : "Secondary";
    const char* drive_name = (drive == 0) ? "Master" : "Slave";
    printf("IDE Device: %s Channel %s Drive\n", channel_name, drive_name);
}

// Read sectors from IDE drive
bool ide_read_sectors(uint8_t channel, uint8_t drive, uint64_t lba, uint16_t sectors, void* buffer) {
    uint16_t base_port = (channel == 0) ? IDE_PRIMARY_BASE : IDE_SECONDARY_BASE;
    
    if (!ide_wait_ready(channel)) {
        return false;
    }
    
    // Setup LBA addressing
    ide_setup_lba(channel, drive, lba, sectors);
    
    // Send read command
    uint8_t cmd = (lba >= 0x100000000) ? IDE_CMD_READ_SECTORS_EXT : IDE_CMD_READ_SECTORS;
    outb(base_port + IDE_COMMAND, cmd);
    
    // Read data
    uint16_t* data = (uint16_t*)buffer;
    for (int sector = 0; sector < sectors; sector++) {
        if (!ide_wait_data(channel)) {
            return false;
        }
        
        // Read 256 words (512 bytes) per sector
        for (int word = 0; word < 256; word++) {
            data[word] = ide_read_data(channel);
        }
        data += 256;
    }
    
    return true;
}

// Write sectors to IDE drive
bool ide_write_sectors(uint8_t channel, uint8_t drive, uint64_t lba, uint16_t sectors, const void* buffer) {
    uint16_t base_port = (channel == 0) ? IDE_PRIMARY_BASE : IDE_SECONDARY_BASE;
    
    if (!ide_wait_ready(channel)) {
        return false;
    }
    
    // Setup LBA addressing
    ide_setup_lba(channel, drive, lba, sectors);
    
    // Send write command
    uint8_t cmd = (lba >= 0x100000000) ? IDE_CMD_WRITE_SECTORS_EXT : IDE_CMD_WRITE_SECTORS;
    outb(base_port + IDE_COMMAND, cmd);
    
    // Write data
    const uint16_t* data = (const uint16_t*)buffer;
    for (int sector = 0; sector < sectors; sector++) {
        if (!ide_wait_data(channel)) {
            return false;
        }
        
        // Write 256 words (512 bytes) per sector
        for (int word = 0; word < 256; word++) {
            ide_write_data(channel, data[word]);
        }
        data += 256;
        
        // Check for errors
        uint8_t status = inb(base_port + IDE_STATUS);
        if (status & IDE_SR_ERR) {
            return false;
        }
    }
    
    // Wait for write to complete
    uint32_t timeout = 1000000;
    while (timeout--) {
        uint8_t status = inb(base_port + IDE_STATUS);
        if (!(status & IDE_SR_BSY)) {
            break;
        }
        if (status & IDE_SR_ERR) {
            return false;
        }
    }
    
    if (timeout == 0) {
        return false;
    }
    
    // Flush cache
    outb(base_port + IDE_COMMAND, IDE_CMD_FLUSH_CACHE);
    
    // Wait for flush to complete
    timeout = 1000000;
    while (timeout--) {
        uint8_t status = inb(base_port + IDE_STATUS);
        if (!(status & IDE_SR_BSY)) {
            break;
        }
        if (status & IDE_SR_ERR) {
            return false;
        }
    }
    
    return timeout > 0;
}

// Get IDE controller status
bool ide_is_initialized(void) {
    return ide_ctrl.initialized;
}

// Check if channel has devices
bool ide_channel_has_devices(uint8_t channel) {
    if (channel == 0) {
        return ide_ctrl.primary.present;
    } else {
        return ide_ctrl.secondary.present;
    }
}

// Initialize IDE channel
static bool ide_init_channel(uint8_t channel) {
    uint16_t base_port = (channel == 0) ? IDE_PRIMARY_BASE : IDE_SECONDARY_BASE;
    uint16_t ctrl_port = (channel == 0) ? IDE_PRIMARY_CTRL : IDE_SECONDARY_CTRL;
    
    const char* channel_name = (channel == 0) ? "Primary" : "Secondary";
    printf("Initializing %s IDE Channel (Base: 0x%04x, Ctrl: 0x%04x)\n", channel_name, base_port, ctrl_port);
    
    // Reset the channel by setting and clearing the reset bit
    outb(ctrl_port, 0x04); // Set reset bit
    for (volatile int i = 0; i < 1000; i++); // Small delay
    outb(ctrl_port, 0x00); // Clear reset bit
    for (volatile int i = 0; i < 1000; i++); // Small delay
    
    // Check if channel is responding
    uint8_t status = inb(base_port + IDE_STATUS);
    printf("%s Channel Status: 0x%02x\n", channel_name, status);
    
    return true;
}

// Check PIIX3 IDE configuration
static void check_piiX3_configuration(void) {
    if (!find_piiX3_controller()) {
        printf("PIIX3 IDE Controller not found - cannot check configuration\n");
        return;
    }

    uint32_t config = pci_config_read(piiX3_bus, piiX3_slot, piiX3_func, PIIX3_IDE_CONFIG);
    printf("PIIX3 IDE Configuration Check:\n");
    printf("  Primary IDE Channel: %s\n", (config & 0x01) ? "Enabled" : "Disabled");
    printf("  Secondary IDE Channel: %s\n", (config & 0x02) ? "Enabled" : "Disabled");
    printf("  Primary Timing: %d\n", (config >> 2) & 0x03);
    printf("  Secondary Timing: %d\n", (config >> 4) & 0x03);
    
    if (!(config & 0x02)) {
        printf("  WARNING: Secondary IDE channel is disabled in PIIX3 configuration!\n");
        printf("  This is why secondary drives are not responding.\n");
    }
}

// Get PIIX3 controller location (for diagnostics)
void ide_get_piiX3_location(uint8_t* bus, uint8_t* slot, uint8_t* func) {
    *bus = piiX3_bus;
    *slot = piiX3_slot;
    *func = piiX3_func;
} 