#include "../../include/memory/heap.h"
#include <stddef.h>

// Wrapper for kmalloc
void* malloc(size_t size) {
    return kmalloc(size);
}

// Wrapper for kfree
void free(void* ptr) {
    kfree(ptr);
} 