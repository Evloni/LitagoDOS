#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

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
        num = -num;
    }

    // Process individual digits
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
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