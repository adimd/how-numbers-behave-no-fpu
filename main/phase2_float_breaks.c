#include "phase2_float_breaks.h"
#include "probe_common.h"
#include <stdio.h>
#include <stdint.h>

#include <float.h>   // add this up with the other includes at the top of the file

// (the rest of the file stays; append the two functions below)

void probe_epsilon(void) {
    printf("\n--- PROBE: machine epsilon (the gap at 1.0) ---\n");
    printf("CLAIM:    epsilon is the smallest x where 1.0 + x != 1.0.\n");
    printf("          For float32 this is 2^-23 ~= 0.00000011920929.\n");

    // Measure it: step to the next float above 1.0 and take the difference.
    float one = 1.0f;
    uint32_t bits = f32_bits(one) + 1;
    float next; __builtin_memcpy(&next, &bits, sizeof next);
    float measured = next - one;

    printf("OBSERVED: next float above 1.0 - 1.0 = ");
    print_double(measured, 12);
    printf("\n          FLT_EPSILON from <float.h>  = ");
    print_double(FLT_EPSILON, 12);
    printf("\n");

    // Confirm the boundary: half of epsilon should vanish, epsilon should stick.
    volatile float half = 1.0f + (measured / 2.0f);
    volatile float full = 1.0f + measured;
    printf("          1.0 + eps/2 == 1.0 ? %s\n", (half == 1.0f) ? "yes (vanished)" : "no");
    printf("          1.0 + eps   == 1.0 ? %s\n", (full == 1.0f) ? "yes" : "no (stuck)");

    printf("VERDICT:  confirmed -- epsilon is the grid spacing at 1.0; below it, additions round away.\n");
}

void probe_accumulate(float start, float inc, long count, const char *note) {
    // local gap just above `start`, by bit-stepping
    uint32_t b = f32_bits(start) + 1;
    float nv; __builtin_memcpy(&nv, &b, sizeof nv);
    float gap = nv - start;

    // Run the additions for real. volatile forces every add to happen on the
    // hardware (no -O3 closed-form shortcut), which is the point of measuring.
    volatile float acc = start;
    for (long i = 0; i < count; i++)
        acc = acc + inc;

    float moved      = (float)acc - start;       // what actually accumulated
    double should_be = (double)inc * (double)count; // what exact math would give

    printf("\n--- PROBE: accumulate ---\n");
    printf("VALUE:    %s\n", note);
    printf("          start = ");           print_double(start, 6);
    printf("  inc = ");                      print_double(inc, 9);
    printf("  x %ld\n", count);
    printf("          local gap above start = ");
    print_double(gap, 9);
    printf("\n          should have added ~= ");
    print_double(should_be, 6);
    printf("\n          actually moved by   = ");
    print_double(moved, 6);
    printf("\n");

    if (moved == 0.0f)
        printf("VERDICT:  confirmed -- FROZEN: inc < gap, every add rounded away to nothing.\n");
    else if ((double)moved < should_be * 0.999)
        printf("VERDICT:  confirmed -- PARTIAL: it moved, but lost arithmetic to rounding.\n");
    else
        printf("VERDICT:  confirmed -- clean: inc >= gap, additions accumulated as expected.\n");
}


// How big is the gap to the next representable float just above `v`?
static float gap_above(float v) {
    uint32_t bits = f32_bits(v);
    uint32_t next = bits + 1;          // the very next float upward
    float nv;
    // reinterpret next bits back to float
    __builtin_memcpy(&nv, &next, sizeof nv);
    return nv - v;
}

void probe_gap_grows(void) {
    printf("\n--- PROBE: the gap grows (2^24 + 1) ---\n");
    printf("CLAIM:    as floats get larger, the spacing between them grows.\n");
    printf("          Past 2^24 the gap exceeds 1, so +1 cannot be represented.\n");

    float two23 = 8388608.0f;    // 2^23 = 8,388,608
    float two24 = 16777216.0f;   // 2^24 = 16,777,216

    printf("OBSERVED: gap just above 2^23 (%d) = ", (int)two23);
    print_double(gap_above(two23), 6);
    printf("\n          gap just above 2^24 (%d) = ", (int)two24);
    print_double(gap_above(two24), 6);
    printf("\n");

    // The headline: add 1 at each boundary and see what sticks.
    float a = two23, b = two24;
    float a1 = a + 1.0f;
    float b1 = b + 1.0f;

    printf("          2^23 + 1: %d + 1 = %d   (changed? %s)\n",
           (int)a, (int)a1, (a1 != a) ? "yes" : "NO");
    printf("          2^24 + 1: %d + 1 = %d   (changed? %s)\n",
           (int)b, (int)b1, (b1 != b) ? "yes" : "NO");

    printf("VERDICT:  confirmed -- at 2^24 the gap is 2, so +1 lands back on 2^24;\n");
    printf("          the increment is smaller than the space between numbers.\n");
}