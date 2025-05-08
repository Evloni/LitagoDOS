#include "../include/keyboardDriver.h"
#include "../include/vga.h"
#include "../include/vga_driver.h"
#include "../include/io.h"
#include "../include/idt.h"
#include "../include/gdt.h"
#include "../include/driver.h"
#include "../include/shell.h"
#include "../include/timerDriver.h"
#include <stddef.h>

void kernel_main() {
	// Initialize GDT
	gdt_init();	
	terminal_setcolor(VGA_COLOR_GREEN);
	terminal_writestring("GDT initialized\n");
	terminal_setcolor(VGA_COLOR_WHITE);

	// Initialize IDT
	idt_init();
	terminal_setcolor(VGA_COLOR_GREEN);
	terminal_writestring("IDT initialized\n");
	terminal_setcolor(VGA_COLOR_WHITE);

	// Register and initialize VGA driver
	if (!register_driver("vga", vga_driver_init, vga_shutdown, NULL)) {
		terminal_writestring("Failed to register VGA driver\n");
		// If we can't register the VGA driver, we can't continue
		return;
	}
	terminal_setcolor(VGA_COLOR_GREEN);
	terminal_writestring("VGA driver registered\n");
	terminal_setcolor(VGA_COLOR_WHITE);

	// Register and initialize keyboard driver
	if (!register_driver("keyboard", keyboard_init, keyboard_shutdown, NULL)) {
		terminal_writestring("Failed to register keyboard driver\n");
		// If we can't register the keyboard driver, we can't continue
		return;
	}

	// Register and initialize timer driver
	if (!register_driver("timer", timer_driver_init, timer_driver_shutdown, NULL)) {
		terminal_writestring("Failed to register timer driver\n");
		return;
	}

	// Initialize all registered drivers
	if (!init_drivers()) {
		terminal_writestring("Failed to initialize all drivers\n");
		return;
	}
	terminal_setcolor(VGA_COLOR_GREEN);
	terminal_writestring("All drivers initialized successfully\n");
	terminal_setcolor(VGA_COLOR_WHITE);

	terminal_writestring("Initializing shell...\n");

	timer_delay_ms(1000);

	shell_init();
	shell_run();  // Start the shell command processing loop

	// Main kernel loop
	while(1) {
		__asm__("hlt");
	}
}