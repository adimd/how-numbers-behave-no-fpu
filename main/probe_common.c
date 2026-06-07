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