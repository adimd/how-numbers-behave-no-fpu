#pragma once
#include <stdint.h>

// Reinterpret a float's 32 bits as an integer (no strict-aliasing UB).
uint32_t f32_bits(float f);

// Print a 32-bit value as binary, grouped as IEEE-754 fields:
//   sign(1) | exponent(8) | mantissa(23)
void print_f32_fields_binary(uint32_t bits);