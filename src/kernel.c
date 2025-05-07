#include "../include/keyboardDriver.h"
#include "../include/vga.h"
#include "../include/vga_driver.h"
#include "../include/io.h"
#include "../include/idt.h"
#include "../include/gdt.h"
#include "../include/driver.h"
#include <stddef.h>

void kernel_main() {
	// Initialize GDT
	gdt_init();
	terminal_writestring("GDT initialized\n");

	// Initialize IDT
	idt_init();
	terminal_writestring("IDT initialized\n");

	// Register and initialize VGA driver
	if (!register_driver("vga", vga_driver_init, vga_shutdown, NULL)) {
		terminal_writestring("Failed to register VGA driver\n");
		// If we can't register the VGA driver, we can't continue
		while(1) { __asm__("hlt"); }
	}
	terminal_writestring("VGA driver registered\n");

	// Register and initialize keyboard driver
	if (!register_driver("keyboard", keyboard_init, keyboard_shutdown, NULL)) {
		terminal_writestring("Failed to register keyboard driver\n");
		// If we can't register the keyboard driver, we can't continue
		while(1) { __asm__("hlt"); }
	}
	terminal_writestring("Keyboard driver registered\n");

	// Initialize all registered drivers
	if (!init_drivers()) {
		terminal_writestring("Failed to initialize all drivers\n");
		while(1) { __asm__("hlt"); }
	}
	terminal_writestring("All drivers initialized successfully\n");

	// Main kernel loop
	while(1) {
		__asm__("hlt");
	}
}