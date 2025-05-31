#include "../../include/memory/program.h"
#include "../../include/memory/pmm.h"
#include "../../include/drivers/vbe.h"
#include <string.h>

// Program loading address (1MB after kernel)
#define PROGRAM_LOAD_ADDRESS 0x100000

bool program_load(void* code_addr, size_t code_size, void* data_addr, size_t data_size, program_entry_t entry, struct program* prog) {
    if (!prog) return false;
    
    // Calculate number of pages needed
    size_t code_pages = (code_size + PAGE_SIZE - 1) / PAGE_SIZE;
    size_t data_pages = (data_size + PAGE_SIZE - 1) / PAGE_SIZE;
    
    // Allocate code segment
    void* code_segment = pmm_alloc_page();
    if (!code_segment) {
        terminal_writestring("Failed to allocate code segment\n");
        return false;
    }
    
    // Allocate data segment
    void* data_segment = pmm_alloc_page();
    if (!data_segment) {
        pmm_free_page(code_segment);
        terminal_writestring("Failed to allocate data segment\n");
        return false;
    }
    
    // Copy code and data
    memcpy(code_segment, code_addr, code_size);
    memcpy(data_segment, data_addr, data_size);
    
    // Set up program structure
    prog->code_segment = code_segment;
    prog->data_segment = data_segment;
    prog->code_size = code_size;
    prog->data_size = data_size;
    prog->entry = entry;
    
    return true;
}

void program_unload(struct program* prog) {
    if (!prog) return;
    
    // Calculate number of pages
    size_t code_pages = (prog->code_size + PAGE_SIZE - 1) / PAGE_SIZE;
    size_t data_pages = (prog->data_size + PAGE_SIZE - 1) / PAGE_SIZE;
    
    // Free code segment
    for (size_t i = 0; i < code_pages; i++) {
        pmm_free_page((void*)((uint32_t)prog->code_segment + i * PAGE_SIZE));
    }
    
    // Free data segment
    for (size_t i = 0; i < data_pages; i++) {
        pmm_free_page((void*)((uint32_t)prog->data_segment + i * PAGE_SIZE));
    }
    
    // Clear program structure
    memset(prog, 0, sizeof(struct program));
}

bool program_execute(struct program* prog) {
    if (!prog || !prog->entry) return false;
    
    // Check if program is properly loaded
    if (!prog->code_segment || !prog->data_segment) return false;
    
    // Execute program
    prog->entry();
    
    return true;
} 