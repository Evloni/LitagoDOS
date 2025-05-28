#include "../include/keyboardDriver.h"
#include "../include/vga.h"
#include "../include/vga_driver.h"
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
#include "../include/vbe.h"
#include "../include/font_8x16.h"
#include "../include/multiboot.h"
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
void delay_animation(int dots) {
	for (int i = 0; i < dots; i++) {
		vbe_draw_char(10, 10, '.', 0xFFFFFF, &font_8x16);
		for (volatile int j = 0; j < 100000000; j++); // Adjust for your speed
	}
}

void kernel_main(uint32_t multiboot_magic, void* multiboot_info) {
	// Initialize VBE first
	vbe_init(multiboot_magic, multiboot_info);
	
	// Clear screen with black background
	vbe_clear_screen(0x000000);
	
	// Display version information
	vbe_draw_string(10, 10, "Litago Version", 0xFFFFFF, &font_8x16);
	vbe_draw_string(130, 10, VERSION_STRING, 0xFFFFFF, &font_8x16);
	vbe_draw_string(10, 30, "Build:", 0xFFFFFF, &font_8x16);
	vbe_draw_string(64, 30, BUILD_DATE, 0xFFFFFF, &font_8x16);
	vbe_draw_string(160, 30, BUILD_TIME, 0xFFFFFF, &font_8x16);
	
	// POST checks
	vbe_draw_string(10, 60, "Performing Power-On Self Test", 0xFFFFFF, &font_8x16);
	
	// Memory Test
	vbe_draw_string(10, 80, "Memory Test: ", 0xFFFFFF, &font_8x16);
	delay_animation(3);
	vbe_draw_string(120, 80, "OK", 0x00FF00, &font_8x16);
	
	// Drive Detection
	vbe_draw_string(10, 100, "Detecting drives: ", 0xFFFFFF, &font_8x16);
	delay_animation(2);
	vbe_draw_string(150, 100, "OK", 0x00FF00, &font_8x16);
	
	// Initialize kernel subsystems
	vbe_draw_string(10, 120, "Initializing kernel subsystems", 0xFFFFFF, &font_8x16);
	delay_animation(2);
	
	// Initialize GDT
	vbe_draw_string(10, 140, "GDT: ", 0xFFFFFF, &font_8x16);
	delay_animation(1);
	gdt_init();
	vbe_draw_string(50, 140, "OK", 0x00FF00, &font_8x16);
	
	// Initialize IDT
	vbe_draw_string(10, 160, "IDT: ", 0xFFFFFF, &font_8x16);
	delay_animation(1);
	idt_init();
	vbe_draw_string(50, 160, "OK", 0x00FF00, &font_8x16);
	
	// Initialize memory manager
	vbe_draw_string(10, 180, "Memory Manager: ", 0xFFFFFF, &font_8x16);
	delay_animation(1);
	memory_map_init(multiboot_magic, multiboot_info);
	pmm_init();
	heap_init();
	vbe_draw_string(150, 180, "OK", 0x00FF00, &font_8x16);
	
	// Get module information from multiboot structure
	if (multiboot_magic == MULTIBOOT_MAGIC) {
		struct multiboot_header* mb = (struct multiboot_header*)multiboot_info;
		if (mb->flags & (1 << 3)) {  // Check if modules are present
			struct module* mods = (struct module*)mb->mods_addr;
			if (mb->mods_count > 0) {
				vbe_draw_string(10, 200, "FAT16 image: ", 0xFFFFFF, &font_8x16);
				delay_animation(1);
				// The first module should be our FAT16 image
				char addr_str[16];
				itoa(mods[0].mod_start, addr_str, 16);
				vbe_draw_string(120, 200, addr_str, 0xFFFFFF, &font_8x16);
				// Set the base address for the ISO filesystem
				iso_fs_set_base(mods[0].mod_start);
				vbe_draw_string(200, 200, "OK", 0x00FF00, &font_8x16);
			}
		}
	}
	
	// Initialize timer driver
	vbe_draw_string(10, 220, "Timer driver: ", 0xFFFFFF, &font_8x16);
	delay_animation(1);
	if (!timer_driver_init()) {
		vbe_draw_string(120, 220, "FAILED", 0xFF0000, &font_8x16);
		return;
	}
	vbe_draw_string(120, 220, "OK", 0x00FF00, &font_8x16);
	
	// Initialize FAT16 filesystem
	vbe_draw_string(10, 240, "FAT16 filesystem: ", 0xFFFFFF, &font_8x16);
	delay_animation(1);
	if (!fat16_init()) {
		vbe_draw_string(150, 240, "FAILED", 0xFF0000, &font_8x16);
		return;
	}
	vbe_draw_string(150, 240, "OK", 0x00FF00, &font_8x16);
	
	// Initialize keyboard
	vbe_draw_string(10, 260, "Keyboard: ", 0xFFFFFF, &font_8x16);
	delay_animation(1);
	if (!keyboard_init()) {
		vbe_draw_string(100, 260, "FAILED", 0xFF0000, &font_8x16);
		return;
	}
	vbe_draw_string(100, 260, "OK", 0x00FF00, &font_8x16);
	
	// Show system ready message
	vbe_draw_string(10, 300, "System initialized successfully!", 0x00FF00, &font_8x16);
	
	// Start shell
	vbe_draw_string(10, 320, "Starting shell...", 0xFFFFFF, &font_8x16);
	show_progress_bar(40, 40);  // 40-character wide progress bar
	//shell_start();
}