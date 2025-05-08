#ifndef STRING_H
#define STRING_H

#include <stddef.h>

// Convert integer to string in the given base
char* itoa(int value, char* str, int base);

// Compare two strings
int strcmp(const char* s1, const char* s2);

#endif // STRING_H 