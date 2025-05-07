#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../include/vga.h"
#include "interrupts/idt.h"



void kernel_main(void) 
{
	/* Initialize terminal interface */
	terminal_initialize();
	idt_init();

	terminal_writestring("Hello, kernel World!\n");
}