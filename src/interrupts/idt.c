#include "idt.h"
#include "../include/io.h"
#include "../include/vga.h"
struct idt_entry idt[256];
struct idt_ptr idt_pointer;

void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt[num].base_low = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = selector;
    idt[num].zero = 0;
    idt[num].flags = flags;
}

void idt_init(void) {
    idt_pointer.limit = (sizeof(struct idt_entry) * 256) - 1;
    idt_pointer.base = (uint32_t)&idt;

    // Clear the IDT
    for(int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    // Remap PIC
    outb(PIC1_COMMAND, 0x11);  // ICW1
    outb(PIC2_COMMAND, 0x11);
    outb(PIC1_DATA, 0x20);     // ICW2: IRQ 0-7 -> interrupts 32-39
    outb(PIC2_DATA, 0x28);     // ICW2: IRQ 8-15 -> interrupts 40-47
    outb(PIC1_DATA, 0x04);     // ICW3
    outb(PIC2_DATA, 0x02);
    outb(PIC1_DATA, 0x01);     // ICW4
    outb(PIC2_DATA, 0x01);

    // Set up keyboard interrupt (IRQ 1)
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);

    // Load IDT
    idt_load();

    // Enable interrupts
    asm volatile("sti");
    terminal_writestring("IDT initialized\n");
}