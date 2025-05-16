#include "../../include/memory/heap.h"
#include "../../include/memory/pmm.h"
#include <stddef.h>
#include <stdint.h>

#define HEAP_START 0x1000000  // 16MB
#define HEAP_SIZE  0x1000000  // 16MB
#define BLOCK_SIZE 4096       // 4KB pages

typedef struct heap_block {
    struct heap_block* next;
    size_t size;
    int used;
} heap_block_t;

static heap_block_t* heap_start = NULL;

void heap_init(void) {
    // Allocate initial heap space
    heap_start = (heap_block_t*)HEAP_START;
    heap_start->next = NULL;
    heap_start->size = HEAP_SIZE - sizeof(heap_block_t);
    heap_start->used = 0;
}

void* malloc(size_t size) {
    if (size == 0) return NULL;

    // Round up size to block size
    size = (size + BLOCK_SIZE - 1) & ~(BLOCK_SIZE - 1);

    // Find a free block
    heap_block_t* current = heap_start;
    while (current != NULL) {
        if (!current->used && current->size >= size) {
            // Split block if it's too large
            if (current->size > size + sizeof(heap_block_t) + BLOCK_SIZE) {
                heap_block_t* new_block = (heap_block_t*)((char*)current + sizeof(heap_block_t) + size);
                new_block->next = current->next;
                new_block->size = current->size - size - sizeof(heap_block_t);
                new_block->used = 0;
                current->next = new_block;
                current->size = size;
            }
            current->used = 1;
            return (void*)((char*)current + sizeof(heap_block_t));
        }
        current = current->next;
    }

    return NULL;  // No free block found
}

void free(void* ptr) {
    if (ptr == NULL) return;

    // Find the block header
    heap_block_t* block = (heap_block_t*)((char*)ptr - sizeof(heap_block_t));
    block->used = 0;

    // Coalesce adjacent free blocks
    heap_block_t* current = heap_start;
    while (current != NULL && current->next != NULL) {
        if (!current->used && !current->next->used) {
            current->size += current->next->size + sizeof(heap_block_t);
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
} 