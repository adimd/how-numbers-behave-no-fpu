#include "phase5_fixed_point.h"
#include "probe_common.h"
#include <stdio.h>
#include <stdint.h>

// Q16.16: 16 integer bits, 16 fraction bits, stored in an int32. Scale = 2^16.
#define Q 16
#define Q_SCALE (1 << Q)          // 65536

// Convert a real value to its Q16.16 integer representation (round to nearest).
static int32_t to_q16(double r) {
    return (int32_t)(r * Q_SCALE + (r >= 0 ? 0.5 : -0.5));
}
// Read a Q16.16 integer back as a real value (for display).
static double from_q16(int32_t q) {
    return (double)q / Q_SCALE;
}

void probe_fixed_point_repr(double value, const char *note) {
    int32_t q = to_q16(value);           // the stored integer -- this is ALL the chip holds
    double back = from_q16(q);
    double error = back - value;

    printf("\n--- PROBE: fixed-point representation (Q16.16) ---\n");
    printf("VALUE:    %s\n", note);
    printf("          requested        = "); print_double(value, 6); printf("\n");
    printf("          stored integer   = %ld   (this is just an int: value x 65536)\n", (long)q);
    printf("          read back        = "); print_double(back, 6);
    printf("   (= %ld / 65536)\n", (long)q);
    printf("          error            = "); print_double(error, 8); printf("\n");

    // Prove the arithmetic is pure integer underneath.
    int32_t half = to_q16(0.5);
    int32_t sum  = q + half;                                  // add: just integer +
    int32_t prod = (int32_t)(((int64_t)q * half) >> Q);       // mul: integer *, then >>16 rescale
    printf("          + 0.5 (integer add)      -> %ld  = ", (long)sum); print_double(from_q16(sum), 6); printf("\n");
    printf("          x 0.5 (integer mul, >>16)-> %ld  = ", (long)prod); print_double(from_q16(prod), 6); printf("\n");

    if (error == 0.0)
        printf("VERDICT:  confirmed -- exact on the Q16.16 grid; pure integers carried the fraction.\n");
    else
        printf("VERDICT:  confirmed -- snapped to the nearest Q16.16 grid point; still just an integer underneath.\n");
}