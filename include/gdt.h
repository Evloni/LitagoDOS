#ifndef GDT_H
#define GDT_H

#include <stdint.h>

// 32-bit GDT entry structure
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

// 32-bit GDT pointer structure
struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// 64-bit GDT entry structure
struct gdt_entry_64bit {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
    uint32_t base_upper;
    uint32_t reserved;
} __attribute__((packed));

// 64-bit GDT pointer structure
struct gdt_ptr_64bit {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

// Function declarations
void gdt_init(void);
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

// 64-bit function declarations
void gdt_64bit_init(void);
void gdt_64bit_set_gate(int num, uint64_t base, uint64_t limit, uint8_t access, uint8_t gran);
const struct gdt_ptr_64bit* gdt_64bit_get_ptr(void);
void gdt_64bit_setup_tss(uint64_t tss_address, uint64_t tss_size);
void gdt_64bit_print_info(void);

#endif // GDT_H 