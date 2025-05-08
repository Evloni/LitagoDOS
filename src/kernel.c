#include "../include/keyboardDriver.h"
#include "../include/vga.h"
#include "../include/vga_driver.h"
#include "../include/io.h"
#include "../include/idt.h"
#include "../include/gdt.h"
#include "../include/driver.h"
#include "../include/shell.h"
#include "../include/timerDriver.h"
#include "../include/memory/pmm.h"
#include "../include/memory/memory_map.h"
#include <stddef.h>

// Multiboot magic number
#define MULTIBOOT_MAGIC 0x2BADB002

void kernel_main(uint32_t multiboot_magic, void* multiboot_info) {
	// Clear screen first
	terminal_initialize();
	terminal_clear();
	terminal_setcolor(VGA_COLOR_WHITE);
	terminal_writestring("Initializing kernel...\n\n");

	// Initialize GDT
	gdt_init();

	// Initialize IDT
	idt_init();

	// Initialize memory manager
	memory_map_init(multiboot_magic, multiboot_info);
	pmm_init();

	// Register and initialize keyboard driver
	if (!register_driver("keyboard", keyboard_init, keyboard_shutdown, NULL)) {
		terminal_setcolor(VGA_COLOR_RED);
		terminal_writestring("Failed to register keyboard driver\n");
		return;
	}

	// Register and initialize timer driver
	if (!register_driver("timer", timer_driver_init, timer_driver_shutdown, NULL)) {
		terminal_setcolor(VGA_COLOR_RED);
		terminal_writestring("Failed to register timer driver\n");
		return;
	}

	// Initialize all registered drivers
	if (!init_drivers()) {
		terminal_setcolor(VGA_COLOR_RED);
		terminal_writestring("Failed to initialize all drivers\n");
		return;
	}

	// Show system ready message
	terminal_setcolor(VGA_COLOR_GREEN);
	terminal_writestring("System initialized successfully\n");
	terminal_setcolor(VGA_COLOR_WHITE);
	terminal_writestring("Starting shell...\n\n");

	timer_delay_ms(10000);

	shell_init();
	shell_run();  // Start the shell command processing loop

	// Main kernel loop
	while(1) {
		__asm__("hlt");
	}
}