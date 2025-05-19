#ifndef STRING_H
#define STRING_H

#include <stddef.h>

// Convert integer to string in the given base
char* itoa(int value, char* str, int base);

// Compare two strings
int strcmp(const char* s1, const char* s2);

// Compare two strings up to n characters
int strncmp(const char* s1, const char* s2, size_t n);

// Find first occurrence of character in string
char* strchr(const char* str, int c);

// Set memory to a specific value
void* memset(void* dest, int val, size_t count);

// Move memory from one location to another
void* memmove(void* dest, const void* src, size_t n);

#endif // STRING_H 