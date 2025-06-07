#include "../include/keyboardDriver.h"
#include "../include/io.h"
#include "../include/idt.h"
#include "../include/gdt.h"
#include "../include/shell.h"
#include "../include/timerDriver.h"
#include "../include/memory/pmm.h"
#include "../include/memory/memory_map.h"
#include "../include/memory/heap.h"
#include "../include/version.h"
#include "../include/fs/fat16.h"
#include "../include/drivers/iso_fs.h"
#include "../include/utils/progress.h"
#include "../include/drivers/vbe.h"
#include "../include/font_8x16.h"
#include "../include/multiboot.h"
#include "../include/drivers/vbe.h"
#include "../include/utils/boot_animation.h"
#include "../include/drivers/bdf_font.h"
#include "../include/drivers/font_loader.h"
#include <stddef.h>

// Multiboot magic number
#define MULTIBOOT_MAGIC 0x2BADB002

// Module structure
struct module {
	uint32_t mod_start;
	uint32_t mod_end;
	uint32_t string;
	uint32_t reserved;
};

// Helper function for animated dots
void delay_animation(int dots, int start_x, int y) {
	for (volatile int j = 0; j < 100000000; j++); // Reduced from 100,000,000 to 1,000,000
}

void kernel_main(uint32_t multiboot_magic, void* multiboot_info) {
	// Initialize VBE first
	vbe_initialize(multiboot_magic, multiboot_info);
	
	// Clear screen with black background
	terminal_clear();
	
	// Display version information
	terminal_writestring("Litago Version ");
	terminal_writestring(VERSION_STRING);
	terminal_writestring(" Build: ");
	terminal_writestring(BUILD_DATE);
	terminal_writestring(" ");
	terminal_writestring(BUILD_TIME);
	terminal_writestring("\n");
	
	// POST checks
	terminal_writestring("Performing Power-On Self Test\n");
	// Move cursor below header
	vbe_cursor_x = 0;
	vbe_cursor_y = 30; // 5 lines * 16px + a blank line
	
	// Memory Test
	terminal_writestring("Memory Test: ");
	delay_animation(3, 115, 80);
	terminal_writestring_color("OK\n", 0x00FF00);
	
	// Drive Detection
	terminal_writestring("Detecting drives: ");
	delay_animation(2, 150, 100);
	terminal_writestring_color("OK\n", 0x00FF00);
	
	// Initialize kernel subsystems
	terminal_writestring("Initializing kernel subsystems");
	delay_animation(2, 250, 120);
	terminal_writestring("\n");
	
	// Initialize GDT
	terminal_writestring("GDT: ");
	delay_animation(1, 50, 140);
	gdt_init();
	terminal_writestring_color("OK\n", 0x00FF00);
	
	// Initialize IDT
	terminal_writestring("IDT: ");
	delay_animation(1, 50, 160);
	idt_init();
	terminal_writestring_color("OK\n", 0x00FF00);
	
	// Initialize memory manager
	terminal_writestring("Memory Manager: ");
	delay_animation(1, 155, 180);
	memory_map_init(multiboot_magic, multiboot_info);
	pmm_init();
	heap_init();
	terminal_writestring_color("OK\n", 0x00FF00);
	
	// Get module information from multiboot structure
	if (multiboot_magic == MULTIBOOT_MAGIC) {
		struct multiboot_header* mb = (struct multiboot_header*)multiboot_info;
		if (mb->flags & (1 << 3)) {  // Check if modules are present
			struct module* mods = (struct module*)mb->mods_addr;
			if (mb->mods_count > 0) {
				terminal_writestring("FAT16 image: ");
				delay_animation(1, 120, 200);
				
				// Set the base address and size for the ISO filesystem
				terminal_writestring("Setting ISO filesystem base address: ");
				char addr_str[32];
				sprintf(addr_str, "0x%x\n", mods[0].mod_start);
				terminal_writestring(addr_str);
				
				terminal_writestring("Setting ISO filesystem size: ");
				sprintf(addr_str, "%d bytes\n", mods[0].mod_end - mods[0].mod_start);
				terminal_writestring(addr_str);
				
				iso_fs_set_base(mods[0].mod_start);
				iso_fs_set_size(mods[0].mod_end - mods[0].mod_start);
				terminal_writestring_color("OK\n", 0x00FF00);
			} else {
				terminal_writestring("No modules found!\n");
				return;
			}
		} else {
			terminal_writestring("No module info!\n");
			return;
		}
	} else {
		terminal_writestring("Invalid multiboot magic!\n");
		return;
	}
	
	// Initialize FAT16 filesystem
	terminal_writestring("FAT16 filesystem: ");
	delay_animation(1, 145, 240);
	if (!fat16_init()) {
		terminal_writestring("FAILED\n");
		return;
	}
	terminal_writestring_color("OK\n", 0x00FF00);

	// Initialize timer driver
	terminal_writestring("Timer driver: ");
	delay_animation(1, 120, 220);
	if (!timer_driver_init()) {
		terminal_writestring("FAILED\n");
		return;
	}
	terminal_writestring_color("OK\n", 0x00FF00);

	// Initialize keyboard
	terminal_writestring("Keyboard: ");
	delay_animation(1, 90, 260);
	if (!keyboard_init()) {
		terminal_writestring("FAILED\n");
		return;
	}
	terminal_writestring_color("OK\n", 0x00FF00);

	
	// Initialize font loader with the PSF font
	if (!font_loader_init("SYSTEM/FONTS/ZAPLIGHT.PSF")) {
		terminal_writestring("Warning: Could not load custom font, using embedded font\n");
	} else {
		PSF1Font* font = get_current_psf1_font();
		if (font) {
			if (font->glyph_count == 512) {
				terminal_writestring("Loaded font with extended ASCII support\n");
			} else {
				terminal_writestring("Warning: Loaded font does not support extended ASCII\n");
			}
		}
	}
	

	// Show system ready message
	terminal_writestring("System initialized successfully!\n");
	
	// Test box drawing
	
	// Show boot animation
	show_boot_animation();
	
	// Start shell
	//terminal_writestring("Starting shell...\n");
	//show_progress_bar(40, 10);  // 40-character wide progress bar
	//shell_start();
	
	// Clean up font resources when shutting down
}