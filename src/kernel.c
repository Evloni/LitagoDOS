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
#include <stddef.h>

// Multiboot magic number
#define MULTIBOOT_MAGIC 0x2BADB002

// Multiboot header structure
struct multiboot_header {
	uint32_t flags;
	uint32_t mem_lower;
	uint32_t mem_upper;
	uint32_t boot_device;
	uint32_t cmdline;
	uint32_t mods_count;
	uint32_t mods_addr;
	uint32_t syms[4];
	uint32_t mmap_length;
	uint32_t mmap_addr;
	uint32_t drives_length;
	uint32_t drives_addr;
	uint32_t config_table;
	uint32_t boot_loader_name;
	uint32_t apm_table;
	uint32_t vbe_control_info;
	uint32_t vbe_mode_info;
	uint32_t vbe_mode;
	uint32_t vbe_interface_seg;
	uint32_t vbe_interface_off;
	uint32_t vbe_interface_len;
};

// Module structure
struct module {
	uint32_t mod_start;
	uint32_t mod_end;
	uint32_t string;
	uint32_t reserved;
};

void kernel_main(uint32_t multiboot_magic, void* multiboot_info) {
	// Clear screen first
	terminal_initialize();
	terminal_clear();
	terminal_setcolor(VGA_COLOR_WHITE);


	// Display version information
	const struct version_info* info = get_version_info();
	terminal_writestring("Litago Version ");
	terminal_writestring(info->version_string);
	terminal_writestring("\nBuild: ");
	terminal_writestring(info->build_date);
	terminal_writestring(" ");
	terminal_writestring(info->build_time);
	terminal_writestring("\n\n");
	
	terminal_writestring("Initializing kernel...\n\n");

	// Initialize GDT
	gdt_init();

	// Initialize IDT
	idt_init();

	// Initialize memory manager
	memory_map_init(multiboot_magic, multiboot_info);
	pmm_init();
	heap_init();  // Initialize heap after PMM

	// Get module information from multiboot structure
	if (multiboot_magic == MULTIBOOT_MAGIC) {
		struct multiboot_header* mb = (struct multiboot_header*)multiboot_info;
		if (mb->flags & (1 << 3)) {  // Check if modules are present
			struct module* mods = (struct module*)mb->mods_addr;
			if (mb->mods_count > 0) {
				// The first module should be our FAT16 image
				terminal_writestring("Found FAT16 image at ");
				char addr_str[16];
				itoa(mods[0].mod_start, addr_str, 16);
				terminal_writestring(addr_str);
				terminal_writestring("\n");
				
				// Set the base address for the ISO filesystem
				iso_fs_set_base(mods[0].mod_start);
			}
		}
	}

	// Initialize timer driver
	if (!timer_driver_init()) {
		terminal_setcolor(VGA_COLOR_RED);
		terminal_writestring("Failed to initialize timer driver\n");
		return;
	}

	// Initialize FAT16 filesystem
	if (!fat16_init()) {
		terminal_setcolor(VGA_COLOR_RED);
		terminal_writestring("Failed to initialize FAT16 filesystem\n");
		return;
	}

	// Initialize keyboard
	if (!keyboard_init()) {
		terminal_setcolor(VGA_COLOR_RED);
		terminal_writestring("Failed to initialize keyboard\n");
		return;
	}

	// Show system ready message
	terminal_setcolor(VGA_COLOR_GREEN);
	terminal_writestring("System initialized successfully!\n");
	terminal_setcolor(VGA_COLOR_WHITE);

	// Start shell
	terminal_writestring("Starting shell...\n\n");
	shell_start();
}