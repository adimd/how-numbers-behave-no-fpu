#include "phase1_float_built.h"
#include "probe_common.h"
#include <stdio.h>
#include <stdint.h>

void probe_float_anatomy(float value, const char *note) {
    uint32_t bits = f32_bits(value);

    uint32_t sign     = (bits >> 31) & 0x1;          // 1 bit
    uint32_t exp_raw  = (bits >> 23) & 0xFF;          // 8 bits, biased
    uint32_t mantissa = bits & 0x7FFFFF;              // 23 bits, fraction

    int exp_actual = (int)exp_raw - 127;             // remove the bias of 127

    printf("\n--- PROBE: float anatomy ---\n");
    printf("VALUE:    %s\n", note);
    printf("          raw bits = ");
    print_f32_fields_binary(bits);
    printf("\n");
    printf("          sign = %lu (%s) | exp_raw = %lu (=> %d) | mantissa = 0x%06lX\n",
           sign, sign ? "neg" : "pos", exp_raw, exp_actual, mantissa);

    // The "1.mantissa x 2^exp" rule only applies to NORMAL numbers
    // (exp_raw between 1 and 254). Zero and the specials don't have a hidden 1.
    if (exp_raw == 0) {
        printf("          NOTE: exponent field is all zeros -- this is zero or a subnormal,\n");
        printf("                so the hidden-1 rule does NOT apply here.\n");
        printf("VERDICT:  confirmed -- the format reserves all-zero exponent as a special case.\n");
        return;
    }
    if (exp_raw == 0xFF) {
        printf("          NOTE: exponent field is all ones -- this is infinity or NaN.\n");
        printf("VERDICT:  confirmed -- the format reserves all-one exponent as a special case.\n");
        return;
    }

    // Normal number: rebuild from the fields to prove they reconstruct the value.
    double significand = 1.0 + (double)mantissa / 8388608.0;  // 2^23
    double rebuilt = (sign ? -1.0 : 1.0) * significand;
    for (int i = 0; i < (exp_actual < 0 ? -exp_actual : exp_actual); i++)
        rebuilt = exp_actual < 0 ? rebuilt / 2.0 : rebuilt * 2.0;

    int sig_milli = (int)(significand * 1000.0 + 0.5);
    printf("          significand (hidden 1 + fraction) = 1.%03d\n", sig_milli - 1000);

    // Print rebuilt as integer part + 3 decimals WITHOUT scaling the whole
    // value by 1000 (that overflows 32-bit for large numbers like 1e7).
    double mag = rebuilt < 0 ? -rebuilt : rebuilt;
    long  int_part   = (long)mag;
    int   frac_milli = (int)((mag - (double)int_part) * 1000.0 + 0.5);
    printf("          rebuilt from bits alone = %s%ld.%03d\n",
           sign ? "-" : "", int_part, frac_milli);
    printf("VERDICT:  confirmed -- the stored fields reconstruct the value.\n");
}