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

// Helper function for animated dots
void delay_animation(int dots) {
	for (int i = 0; i < dots; i++) {
		terminal_writestring(".");
		for (volatile int j = 0; j < 100000000; j++); // Adjust for your speed
	}
}

void kernel_main(uint32_t multiboot_magic, void* multiboot_info) {
	terminal_initialize();
	terminal_clear();
	if (ansi_is_enabled()) {
		terminal_writestring("\x1B[37m"); // Light grey
	} else {
		terminal_setcolor(VGA_COLOR_LIGHT_GREY);
	}

	terminal_writestring("Litago Version ");
	terminal_writestring(VERSION_STRING);
	terminal_writestring("\nBuild: ");
	terminal_writestring(BUILD_DATE);
	terminal_writestring(" ");
	terminal_writestring(BUILD_TIME);
	terminal_writestring("\n\n");

	// POST checks
	terminal_writestring("Performing Power-On Self Test\n");
	terminal_writestring("Memory Test: ");
	delay_animation(3); // Print dots with delay
	if (ansi_is_enabled()) {
		terminal_writestring("\x1B[32mOK\x1B[37m");
	} else {
		terminal_setcolor(VGA_COLOR_GREEN);
		terminal_writestring("OK");
		terminal_setcolor(VGA_COLOR_LIGHT_GREY);
	}
	terminal_writestring("\n");

	terminal_writestring("Detecting drives: ");
	delay_animation(2);
	if (ansi_is_enabled()) {
		terminal_writestring("\x1B[32mOK\x1B[37m");
	} else {
		terminal_setcolor(VGA_COLOR_GREEN);
		terminal_writestring("OK");
		terminal_setcolor(VGA_COLOR_LIGHT_GREY);
	}
	terminal_writestring("\n");

	terminal_writestring("Initializing kernel subsystems\n");
	delay_animation(2);

	// Display version information
	const struct version_info* info = get_version_info();
	terminal_writestring("Litago Version ");
	terminal_writestring(info->version_string);
	terminal_writestring("\nBuild: ");
	terminal_writestring(info->build_date);
	terminal_writestring(" ");
	terminal_writestring(info->build_time);
	terminal_writestring("\n\n");

	// Enable ANSI support explicitly
	ansi_set_enabled(true);
	if (ansi_is_enabled()) {
		terminal_writestring("\x1B[32mANSI support enabled\x1B[37m");
	} else {
		terminal_setcolor(VGA_COLOR_GREEN);
		terminal_writestring("ANSI support enabled");
		terminal_setcolor(VGA_COLOR_LIGHT_GREY);
	}
	terminal_writestring("\n");

	terminal_writestring("Initializing kernel...\n\n");

	// Initialize GDT
	terminal_writestring("GDT: ");
	delay_animation(1);
	gdt_init();
	if (ansi_is_enabled()) {
		terminal_writestring("\x1B[32mOK\x1B[37m");
	} else {
		terminal_setcolor(VGA_COLOR_GREEN);
		terminal_writestring("OK");
		terminal_setcolor(VGA_COLOR_LIGHT_GREY);
	}
	terminal_writestring("\n");

	// Initialize IDT
	terminal_writestring("IDT: ");
	delay_animation(1);
	idt_init();
	if (ansi_is_enabled()) {
		terminal_writestring("\x1B[32mOK\x1B[37m");
	} else {
		terminal_setcolor(VGA_COLOR_GREEN);
		terminal_writestring("OK");
		terminal_setcolor(VGA_COLOR_LIGHT_GREY);
	}
	terminal_writestring("\n");

	// Initialize memory manager
	terminal_writestring("Memory Manager: ");
	delay_animation(1);
	memory_map_init(multiboot_magic, multiboot_info);
	pmm_init();
	heap_init();  // Initialize heap after PMM
	if (ansi_is_enabled()) {
		terminal_writestring("\x1B[32mOK\x1B[37m");
	} else {
		terminal_setcolor(VGA_COLOR_GREEN);
		terminal_writestring("OK");
		terminal_setcolor(VGA_COLOR_LIGHT_GREY);
	}
	terminal_writestring("\n");

	// Get module information from multiboot structure
	if (multiboot_magic == MULTIBOOT_MAGIC) {
		struct multiboot_header* mb = (struct multiboot_header*)multiboot_info;
		if (mb->flags & (1 << 3)) {  // Check if modules are present
			struct module* mods = (struct module*)mb->mods_addr;
			if (mb->mods_count > 0) {
				terminal_writestring("FAT16 image: ");
				delay_animation(1);
				// The first module should be our FAT16 image
				char addr_str[16];
				itoa(mods[0].mod_start, addr_str, 16);
				terminal_writestring(addr_str);
				terminal_writestring(" ");
				// Set the base address for the ISO filesystem
				iso_fs_set_base(mods[0].mod_start);
				if (ansi_is_enabled()) {
					terminal_writestring("\x1B[32mOK\x1B[37m");
				} else {
					terminal_setcolor(VGA_COLOR_GREEN);
					terminal_writestring("OK");
					terminal_setcolor(VGA_COLOR_LIGHT_GREY);
				}
				terminal_writestring("\n");
			}
		}
	}

	// Initialize timer driver
	terminal_writestring("Timer driver: ");
	delay_animation(1);
	if (!timer_driver_init()) {
		if (ansi_is_enabled()) {
			terminal_writestring("\x1B[31mFAILED\x1B[37m");
		} else {
			terminal_setcolor(VGA_COLOR_RED);
			terminal_writestring("FAILED");
			terminal_setcolor(VGA_COLOR_LIGHT_GREY);
		}
		terminal_writestring("\n");
		return;
	}
	if (ansi_is_enabled()) {
		terminal_writestring("\x1B[32mOK\x1B[37m");
	} else {
		terminal_setcolor(VGA_COLOR_GREEN);
		terminal_writestring("OK");
		terminal_setcolor(VGA_COLOR_LIGHT_GREY);
	}
	terminal_writestring("\n");

	// Initialize FAT16 filesystem
	terminal_writestring("FAT16 filesystem: ");
	delay_animation(1);
	if (!fat16_init()) {
		if (ansi_is_enabled()) {
			terminal_writestring("\x1B[31mFAILED\x1B[37m");
		} else {
			terminal_setcolor(VGA_COLOR_RED);
			terminal_writestring("FAILED");
			terminal_setcolor(VGA_COLOR_LIGHT_GREY);
		}
		terminal_writestring("\n");
		return;
	}
	if (ansi_is_enabled()) {
		terminal_writestring("\x1B[32mOK\x1B[37m");
	} else {
		terminal_setcolor(VGA_COLOR_GREEN);
		terminal_writestring("OK");
		terminal_setcolor(VGA_COLOR_LIGHT_GREY);
	}
	terminal_writestring("\n");

	// Initialize keyboard
	terminal_writestring("Keyboard: ");
	delay_animation(1);
	if (!keyboard_init()) {
		if (ansi_is_enabled()) {
			terminal_writestring("\x1B[31mFAILED\x1B[37m");
		} else {
			terminal_setcolor(VGA_COLOR_RED);
			terminal_writestring("FAILED");
			terminal_setcolor(VGA_COLOR_LIGHT_GREY);
		}
		terminal_writestring("\n");
		return;
	}
	if (ansi_is_enabled()) {
		terminal_writestring("\x1B[32mOK\x1B[37m");
	} else {
		terminal_setcolor(VGA_COLOR_GREEN);
		terminal_writestring("OK");
		terminal_setcolor(VGA_COLOR_LIGHT_GREY);
	}
	terminal_writestring("\n");

	// Show system ready message
	if (ansi_is_enabled()) {
		terminal_writestring("\n\x1B[32mSystem initialized successfully!\x1B[37m\n");
	} else {
		terminal_setcolor(VGA_COLOR_GREEN);
		terminal_writestring("\nSystem initialized successfully!\n");
		terminal_setcolor(VGA_COLOR_LIGHT_GREY);
	}

	// Start shell
	terminal_writestring("\nStarting shell...\n");
	show_progress_bar(40, 40);  // 40-character wide progress bar
	shell_start();
}