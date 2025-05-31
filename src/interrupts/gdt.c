#include "../../include/gdt.h"
#include "../../include/drivers/vbe.h"
#include <stddef.h>

// GDT entries
struct gdt_entry gdt[5];
struct gdt_ptr gdt_ptr;

// Set a GDT entry
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

// Initialize GDT
void gdt_init() {
    gdt_ptr.limit = (sizeof(struct gdt_entry) * 5) - 1;
    gdt_ptr.base = (uint32_t)&gdt;

    // NULL descriptor
    gdt_set_gate(0, 0, 0, 0, 0);

    // Code segment descriptor
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // Data segment descriptor
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    // User mode code segment
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

    // User mode data segment
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    // Load GDT
    asm volatile("lgdt (%0)" : : "r" (&gdt_ptr));

    // Reload segment registers
    asm volatile("movw $0x10, %ax");
    asm volatile("movw %ax, %ds");
    asm volatile("movw %ax, %es");
    asm volatile("movw %ax, %fs");
    asm volatile("movw %ax, %gs");
    asm volatile("movw %ax, %ss");
    asm volatile("ljmp $0x08, $1f");
    asm volatile("1:");
} 