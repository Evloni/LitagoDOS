#include <stdint.h>

// 64-bit unsigned division
uint64_t __udivdi3(uint64_t num, uint64_t den) {
    uint64_t quot = 0;
    uint64_t qbit = 1;

    if (den == 0) {
        return 0; // Division by zero
    }

    // Left-justify denominator and count shift
    while ((int64_t)den >= 0) {
        den <<= 1;
        qbit <<= 1;
    }

    while (qbit) {
        if (den <= num) {
            num -= den;
            quot += qbit;
        }
        den >>= 1;
        qbit >>= 1;
    }

    return quot;
}

// 64-bit unsigned modulo
uint64_t __umoddi3(uint64_t num, uint64_t den) {
    uint64_t quot = 0;
    uint64_t qbit = 1;

    if (den == 0) {
        return 0; // Division by zero
    }

    // Left-justify denominator and count shift
    while ((int64_t)den >= 0) {
        den <<= 1;
        qbit <<= 1;
    }

    while (qbit) {
        if (den <= num) {
            num -= den;
            quot += qbit;
        }
        den >>= 1;
        qbit >>= 1;
    }

    return num;
} 