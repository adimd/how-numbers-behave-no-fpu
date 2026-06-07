#include "phase3_edges.h"
#include "probe_common.h"
#include <stdio.h>
#include <stdint.h>
#include <float.h>     // FLT_MAX
#include <math.h>      // isinf, INFINITY

void probe_overflow(void) {
    printf("\n--- PROBE: overflow to infinity ---\n");
    printf("CLAIM:    floats have a largest finite value (FLT_MAX). Past it,\n");
    printf("          the format has no bigger number, so it saturates to +infinity.\n");

    printf("OBSERVED: FLT_MAX        = ");
    print_double((double)FLT_MAX, 0);
    printf("\n          FLT_MAX bits   = ");
    print_f32_fields_binary(f32_bits(FLT_MAX));
    printf("\n");

    // Climb: keep doubling from a large value and watch it cross the edge.
    volatile float v = FLT_MAX / 2.0f;
    printf("          climbing by doubling from FLT_MAX/2:\n");
    for (int step = 0; step < 4; step++) {
        float next = v * 2.0f;
        printf("            step %d: x2 -> ", step);
        if (isinf(next)) printf("+infinity");
        else             print_double((double)next, 0);
        printf("   bits = ");
        print_f32_fields_binary(f32_bits(next));
        printf("\n");
        v = next;
    }

    // Now interrogate infinity itself -- it's a real value with rules.
    float inf = INFINITY;
    printf("          is it really infinity? isinf = %d\n", isinf(inf));
    printf("          infinity + 1   == infinity ? %d\n", (inf + 1.0f) == inf);
    printf("          infinity - infinity        -> %s\n",
           isnan(inf - inf) ? "NaN (undefined)" : "a number");

    printf("VERDICT:  confirmed -- past FLT_MAX the float saturates to +infinity,\n");
    printf("          a reserved value (exp all ones, mantissa zero) that absorbs +1\n");
    printf("          but yields NaN when you subtract it from itself.\n");
}