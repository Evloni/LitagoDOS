#ifndef STRING_H
#define STRING_H

#include <stddef.h>

// Convert integer to string in the given base
char* itoa(int value, char* str, int base);

// Compare two strings
int strcmp(const char* s1, const char* s2);

// Set memory to a specific value
void* memset(void* dest, int val, size_t count);

#endif // STRING_H 