#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>

// Initialize the heap
void heap_init(void);

// Allocate memory from the heap
void* malloc(size_t size);

// Free previously allocated memory
void free(void* ptr);

// Allocate memory from the kernel heap
void* kmalloc(size_t size);

// Free memory allocated from the kernel heap
void kfree(void* ptr);

#endif // HEAP_H 