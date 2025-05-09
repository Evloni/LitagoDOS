#include "../include/string.h"

// Reverse a string in place
static void reverse(char* str, int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

// Convert integer to string in the given base
char* itoa(int value, char* str, int base) {
    int i = 0;
    int is_negative = 0;

    // Handle 0 explicitly
    if (value == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    // Handle negative numbers only for base 10
    if (value < 0 && base == 10) {
        is_negative = 1;
        value = -value;
    }

    // Process individual digits
    while (value != 0) {
        int rem = value % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        value = value / base;
    }

    // Add negative sign if needed
    if (is_negative)
        str[i++] = '-';

    str[i] = '\0';

    // Reverse the string
    reverse(str, i);

    return str;
}

// Compare two strings
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

// Set memory to a specific value
void* memset(void* dest, int val, size_t count) {
    unsigned char* ptr = (unsigned char*)dest;
    while (count-- > 0) {
        *ptr++ = (unsigned char)val;
    }
    return dest;
}

// Find first occurrence of character in string
char* strchr(const char* str, int c) {
    while (*str != '\0') {
        if (*str == (char)c) {
            return (char*)str;
        }
        str++;
    }
    if (c == '\0') {
        return (char*)str;
    }
    return NULL;
}

// Compare two strings up to n characters
int strncmp(const char* s1, const char* s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) {
        return 0;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

// Copy memory from source to destination
void* memcpy(void* dest, const void* src, size_t count) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    while (count-- > 0) {
        *d++ = *s++;
    }
    return dest;
}

// Compare memory regions
int memcmp(const void* ptr1, const void* ptr2, size_t count) {
    const unsigned char* p1 = (const unsigned char*)ptr1;
    const unsigned char* p2 = (const unsigned char*)ptr2;
    while (count-- > 0) {
        if (*p1++ != *p2++) {
            return p1[-1] < p2[-1] ? -1 : 1;
        }
    }
    return 0;
}

// Get string length
size_t strlen(const char* str) {
    const char* s = str;
    while (*s) s++;
    return s - str;
}

// Copy string with length limit
char* strncpy(char* dest, const char* src, size_t count) {
    char* d = dest;
    while (count > 0 && *src != '\0') {
        *d++ = *src++;
        count--;
    }
    while (count > 0) {
        *d++ = '\0';
        count--;
    }
    return dest;
}

// Concatenate strings
char* strcat(char* dest, const char* src) {
    char* d = dest;
    while (*d) d++;
    while (*src) {
        *d++ = *src++;
    }
    *d = '\0';
    return dest;
}

// Concatenate strings with length limit
char* strncat(char* dest, const char* src, size_t count) {
    char* d = dest;
    while (*d) d++;
    while (count > 0 && *src != '\0') {
        *d++ = *src++;
        count--;
    }
    *d = '\0';
    return dest;
} 