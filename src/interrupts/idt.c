#include "../../include/idt.h"
#include "../../include/io.h"
#include "../../include/system.h"
#include "../../include/vga.h"
#include <stddef.h>

// IDT pointer structure
struct idt_pointer {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// IDT entries array
static struct idt_entry idt[256];

// IDT pointer
struct idt_pointer idt_ptr;

// External assembly functions
extern void irq0();
extern void irq1();
extern void idt_load(void);
extern void timer_handler(struct regs *r);

// Helper to print a byte as two hex digits
static void print_hex(uint8_t value) {
    const char *hex = "0123456789ABCDEF";
    terminal_putchar(hex[(value >> 4) & 0x0F]);
    terminal_putchar(hex[value & 0x0F]);
}

// Simple memset implementation
static void* memset(void* dest, int val, size_t count) {
    unsigned char* ptr = (unsigned char*)dest;
    while (count-- > 0) {
        *ptr++ = val;
    }
    return dest;
}

// Set an IDT entry
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = selector;
    idt[num].zero = 0;
    idt[num].flags = flags;
    
    // Debug output for keyboard interrupt gate
    if (num == 33) {  // IRQ1 (keyboard)
        terminal_writestring("\nSetting up keyboard interrupt gate (IRQ1):\n");
        terminal_writestring("Base: 0x");
        print_hex((base >> 24) & 0xFF);
        print_hex((base >> 16) & 0xFF);
        print_hex((base >> 8) & 0xFF);
        print_hex(base & 0xFF);
        terminal_putchar('\n');
        terminal_writestring("Selector: 0x");
        print_hex((selector >> 8) & 0xFF);
        print_hex(selector & 0xFF);
        terminal_putchar('\n');
        terminal_writestring("Flags: 0x");
        print_hex(flags);
        terminal_putchar('\n');
    }
}

// Initialize the IDT
void idt_init() {
    terminal_writestring("\n=== Initializing IDT ===\n");
    
    // Set up IDT pointer
    idt_ptr.limit = (sizeof(struct idt_entry) * 256) - 1;
    idt_ptr.base = (uint32_t)&idt;
    
    // Clear IDT
    for (size_t i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0x08, 0x8E);  // Present, Ring 0, 32-bit Interrupt Gate
    }
    
    // Remap PIC
    // ICW1 - Start initialization
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    
    // ICW2 - Set vector offsets
    outb(0x21, 0x20);  // IRQ0-7: 0x20-0x27
    outb(0xA1, 0x28);  // IRQ8-15: 0x28-0x2F
    
    // ICW3 - Set up cascading
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    
    // ICW4 - Set mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    
    // Mask all interrupts except keyboard and timer
    outb(0x21, 0xFC);  // Enable IRQ0 (timer) and IRQ1 (keyboard)
    outb(0xA1, 0xFF);  // Disable all IRQs on slave PIC
    
    // Set up timer interrupt
    idt_set_gate(0x20, (uint32_t)irq0, 0x08, 0x8E);
    
    // Set up keyboard interrupt
    idt_set_gate(0x21, (uint32_t)irq1, 0x08, 0x8E);
    
    // Load IDT
    asm volatile("lidt (%0)" : : "r" (&idt_ptr));
    
    // Enable interrupts
    asm volatile("sti");
    
    // Enable keyboard interrupt (IRQ1) on PIC1
    uint8_t pic_mask = inb(0x21);
    pic_mask &= ~(1 << 1);  // Clear bit 1 to enable keyboard interrupt
    outb(0x21, pic_mask);
    
    terminal_writestring("\nFinal PIC1 mask: ");
    print_hex(pic_mask);
    terminal_putchar('\n');
    terminal_writestring("(Bit 1 should be 0 for keyboard interrupt to be enabled)\n");
    
    terminal_writestring("=== IDT Initialization Complete ===\n\n");
}