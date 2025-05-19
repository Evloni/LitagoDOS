#include "../../include/drivers/iso_fs.h"
#include "../../include/vga.h"
#include <stdint.h>

void test_iso_fs(void) {
    terminal_writestring("Testing ISO filesystem driver...\n");
    
    // Initialize the filesystem
    if (!iso_fs_init()) {
        terminal_writestring("Failed to initialize ISO filesystem\n");
        return;
    }
    terminal_writestring("ISO filesystem initialized successfully\n");
    
    // Test reading a sector
    uint8_t buffer[512];
    if (!iso_fs_read_sectors(0, 1, buffer)) {
        terminal_writestring("Failed to read sector 0\n");
        return;
    }
    terminal_writestring("Successfully read sector 0\n");
    
    // Print first few bytes of the sector
    terminal_writestring("First 16 bytes of sector 0: ");
    for (int i = 0; i < 16; i++) {
        uint8_t byte = buffer[i];
        // Print high nibble
        uint8_t high = (byte >> 4) & 0x0F;
        terminal_putchar(high < 10 ? '0' + high : 'A' + (high - 10));
        // Print low nibble
        uint8_t low = byte & 0x0F;
        terminal_putchar(low < 10 ? '0' + low : 'A' + (low - 10));
        terminal_putchar(' ');
    }
    terminal_putchar('\n');
} 