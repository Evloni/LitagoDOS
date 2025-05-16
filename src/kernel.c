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
#include "../include/version.h"
#include <stddef.h>

// Multiboot magic number
#define MULTIBOOT_MAGIC 0x2BADB002

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

	// Initialize timer driver
	if (!timer_driver_init()) {
		terminal_setcolor(VGA_COLOR_RED);
		terminal_writestring("Failed to initialize timer driver\n");
		return;
	}

	// Initialize filesystem
	if (fat16_init() != 0) {
		terminal_setcolor(VGA_COLOR_RED);
		terminal_writestring("Failed to initialize filesystem\n");
		return;
	}

	// Show system ready message
	terminal_setcolor(VGA_COLOR_GREEN);
	terminal_writestring("System initialized successfully\n");
	terminal_setcolor(VGA_COLOR_WHITE);
	terminal_writestring("Starting shell...\n\n");

	timer_delay_ms(1000);

	shell_init();
	shell_run();  // Start the shell command processing loop

	// Main kernel loop
	while(1) {
		__asm__("hlt");
	}
}