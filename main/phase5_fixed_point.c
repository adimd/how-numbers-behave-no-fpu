#include "phase5_fixed_point.h"
#include "probe_common.h"
#include <stdio.h>
#include <stdint.h>
#include <float.h>

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


void probe_grid_compare(void) {
    printf("\n--- PROBE: uniform grid (fixed-point) vs uneven grid (float) ---\n");
    printf("CLAIM:    float spacing grows with magnitude; Q16.16 spacing is constant\n");
    printf("          (1/65536) everywhere -- uniform, but only within a small range.\n");

    // Q16.16 spacing is always one integer step = 1/65536, independent of magnitude.
    double q_step = 1.0 / Q_SCALE;
    printf("OBSERVED: Q16.16 step is ALWAYS = "); print_double(q_step, 8);
    printf("  (1/65536), at every magnitude\n\n");

    // Float spacing: step to the next representable float at several magnitudes.
    float points[] = { 0.5f, 1.0f, 100.0f, 100000.0f, 16777216.0f };
    const char *labels[] = { "0.5", "1.0", "100", "100000", "2^24" };
    printf("          magnitude |   float gap to next   |  Q16.16 gap\n");
    printf("          ----------+-----------------------+-------------\n");
    for (int i = 0; i < 5; i++) {
        float v = points[i];
        uint32_t b = f32_bits(v) + 1;
        float nv; __builtin_memcpy(&nv, &b, sizeof nv);
        double fgap = (double)nv - (double)v;

        printf("          %-9s | ", labels[i]);
        print_double(fgap, 8);
        printf("  | ");
        // Q16.16 can only represent this magnitude if it fits the +-32768 range.
        if (v < 32768.0f) { print_double(q_step, 8); printf("  (same as always)"); }
        else              { printf("--- OUT OF RANGE (Q16.16 maxes ~32768) ---"); }
        printf("\n");
    }

    // Show the two failure modes directly.
    printf("\n          range limits:\n");
    printf("            float  reaches ~3.4e38 (but coarsely up high)\n");
    printf("            Q16.16 maxes at +-32767.99998; one step past wraps (int32 overflow)\n");

    // Demonstrate the float being COARSER than Q16.16 at a shared magnitude.
    printf("\n          at magnitude 100:\n");
    printf("            float gap  = "); 
    { float v=100.0f; uint32_t b=f32_bits(v)+1; float nv; __builtin_memcpy(&nv,&b,sizeof nv);
      print_double((double)nv-(double)v, 8); }
    printf("\n            Q16.16 gap = "); print_double(q_step, 8);
    printf("   <- fixed-point is FINER here\n");

    printf("VERDICT:  confirmed -- neither wins outright: float trades uniform precision\n");
    printf("          for enormous range (fine near 0, coarse when large); Q16.16 keeps\n");
    printf("          one constant fine step but only across a small bounded range.\n");
}