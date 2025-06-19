#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "../../include/drivers/vbe.h"

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

static int itoa(int num, char* str, int base) {
    int i = 0;
    bool isNegative = false;

    // Handle 0 explicitly
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return i;
    }

    // Handle negative numbers
    if (num < 0 && base == 10) {
        isNegative = true;
        // Use unsigned arithmetic to handle INT_MIN properly
        unsigned int abs_num = (unsigned int)(-num);
        while (abs_num != 0) {
            unsigned int rem = abs_num % base;
            str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
            abs_num = abs_num / base;
        }
    } else {
        // Process individual digits for non-negative numbers
        unsigned int abs_num = (unsigned int)num;
        while (abs_num != 0) {
            unsigned int rem = abs_num % base;
            str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
            abs_num = abs_num / base;
        }
    }

    // Append negative sign for base 10
    if (isNegative) {
        str[i++] = '-';
    }

    str[i] = '\0';
    reverse(str, i);
    return i;
}

int sprintf(char* str, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    int i = 0;
    int j = 0;
    
    while (format[i] != '\0') {
        if (format[i] == '%') {
            i++;
            switch (format[i]) {
                case 'd': {
                    int num = va_arg(args, int);
                    j += itoa(num, &str[j], 10);
                    break;
                }
                case 'x': {
                    int num = va_arg(args, int);
                    j += itoa(num, &str[j], 16);
                    break;
                }
                case 'c': {
                    str[j++] = (char)va_arg(args, int);
                    break;
                }
                case 's': {
                    const char* s = va_arg(args, const char*);
                    while (*s) {
                        str[j++] = *s++;
                    }
                    break;
                }
                case '%': {
                    str[j++] = '%';
                    break;
                }
            }
        } else {
            str[j++] = format[i];
        }
        i++;
    }
    
    str[j] = '\0';
    va_end(args);
    return j;
}

int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    char buffer[256];  // Buffer for formatted output
    int i = 0;
    int j = 0;
    int total_chars = 0;  // Track total characters written
    
    while (format[i] != '\0') {
        if (format[i] == '%') {
            i++;
            
            // Parse width specifier
            int width = 0;
            while (format[i] >= '0' && format[i] <= '9') {
                width = width * 10 + (format[i] - '0');
                i++;
            }
            
            switch (format[i]) {
                case 'd': {
                    int num = va_arg(args, int);
                    char num_str[32];
                    int len = itoa(num, num_str, 10);
                    
                    // Add padding if width is specified
                    while (len < width) {
                        buffer[j++] = ' ';
                        len++;
                    }
                    
                    // Copy the number
                    for (int k = 0; k < itoa(num, num_str, 10); k++) {
                        buffer[j++] = num_str[k];
                    }
                    break;
                }
                case 'x': {
                    int num = va_arg(args, int);
                    char num_str[32];
                    int len = itoa(num, num_str, 16);
                    
                    // Add zero padding if width is specified
                    while (len < width) {
                        buffer[j++] = '0';
                        len++;
                    }
                    
                    // Copy the number
                    for (int k = 0; k < itoa(num, num_str, 16); k++) {
                        buffer[j++] = num_str[k];
                    }
                    break;
                }
                case 'X': {
                    int num = va_arg(args, int);
                    char num_str[32];
                    int start_pos = j;
                    int len = itoa(num, num_str, 16);
                    
                    // Add zero padding if width is specified
                    while (len < width) {
                        buffer[j++] = '0';
                        len++;
                    }
                    
                    // Copy the number
                    for (int k = 0; k < itoa(num, num_str, 16); k++) {
                        buffer[j++] = num_str[k];
                    }
                    
                    // Convert to uppercase
                    for (int k = start_pos; k < j; k++) {
                        if (buffer[k] >= 'a' && buffer[k] <= 'z') {
                            buffer[k] = buffer[k] - 'a' + 'A';
                        }
                    }
                    break;
                }
                case 'c': {
                    buffer[j++] = (char)va_arg(args, int);
                    break;
                }
                case 's': {
                    const char* s = va_arg(args, const char*);
                    if (s == NULL) {
                        const char* null_str = "(null)";
                        while (*null_str) {
                            buffer[j++] = *null_str++;
                        }
                    } else {
                        while (*s) {
                            buffer[j++] = *s++;
                        }
                    }
                    break;
                }
                case '%': {
                    buffer[j++] = '%';
                    break;
                }
                case 'u': {
                    unsigned int num = va_arg(args, unsigned int);
                    char num_str[32];
                    int len = itoa((int)num, num_str, 10);
                    
                    // Add padding if width is specified
                    while (len < width) {
                        buffer[j++] = ' ';
                        len++;
                    }
                    
                    // Copy the number
                    for (int k = 0; k < itoa((int)num, num_str, 10); k++) {
                        buffer[j++] = num_str[k];
                    }
                    break;
                }
                case 'o': {
                    int num = va_arg(args, int);
                    char num_str[32];
                    int len = itoa(num, num_str, 8);
                    
                    // Add padding if width is specified
                    while (len < width) {
                        buffer[j++] = ' ';
                        len++;
                    }
                    
                    // Copy the number
                    for (int k = 0; k < itoa(num, num_str, 8); k++) {
                        buffer[j++] = num_str[k];
                    }
                    break;
                }
                case 'p': {
                    void* ptr = va_arg(args, void*);
                    buffer[j++] = '0';
                    buffer[j++] = 'x';
                    char num_str[32];
                    int len = itoa((int)(uintptr_t)ptr, num_str, 16);
                    
                    // Add zero padding if width is specified
                    while (len < width) {
                        buffer[j++] = '0';
                        len++;
                    }
                    
                    // Copy the number
                    for (int k = 0; k < itoa((int)(uintptr_t)ptr, num_str, 16); k++) {
                        buffer[j++] = num_str[k];
                    }
                    break;
                }
                default: {
                    // Unknown format specifier, just print it
                    buffer[j++] = '%';
                    buffer[j++] = format[i];
                    break;
                }
            }
        } else {
            buffer[j++] = format[i];
        }
        i++;
        
        // Flush buffer if it's getting full or we hit a newline
        if (j >= 254 || (j > 0 && buffer[j-1] == '\n')) {
            buffer[j] = '\0';
            for (int k = 0; k < j; k++) {
                terminal_putchar(buffer[k]);
            }
            total_chars += j;
            j = 0;
        }
    }
    
    // Flush any remaining characters
    if (j > 0) {
        buffer[j] = '\0';
        for (int k = 0; k < j; k++) {
            terminal_putchar(buffer[k]);
        }
        total_chars += j;
    }
    
    va_end(args);
    return total_chars;  // Return total number of characters written
} 