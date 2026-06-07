#include "probe_common.h"
#include <stdio.h>
#include <string.h>

uint32_t f32_bits(float f) {
    uint32_t u;
    memcpy(&u, &f, sizeof u);
    return u;
}

void print_f32_fields_binary(uint32_t bits) {
    putchar((bits >> 31) & 1 ? '1' : '0');   // sign
    putchar(' ');
    for (int i = 30; i >= 23; i--)            // exponent, 8 bits
        putchar((bits >> i) & 1 ? '1' : '0');
    putchar(' ');
    for (int i = 22; i >= 0; i--)             // mantissa, 23 bits
        putchar((bits >> i) & 1 ? '1' : '0');
}

void print_double(double x, int dec) {
    if (x < 0) { putchar('-'); x = -x; }
    if (x >= 2147483647.0) {            // beyond what 32-bit long can hold honestly
        printf("(>2^31, see bits)");
        return;
    }
    long int_part = (long)x;
    double frac = x - (double)int_part;
    printf("%ld.", int_part);
    for (int i = 0; i < dec; i++) {
        frac *= 10.0;
        int digit = (int)frac;
        if (digit > 9) digit = 9;
        putchar('0' + digit);
        frac -= digit;
    }
}