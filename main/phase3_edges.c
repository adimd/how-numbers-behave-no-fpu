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

void probe_underflow(void) {
    printf("\n--- PROBE: underflow through subnormals to zero ---\n");
    printf("CLAIM:    below FLT_MIN (smallest NORMAL float) the format keeps going\n");
    printf("          through 'subnormals' (no hidden 1), losing precision bit by bit,\n");
    printf("          until it finally flushes to true zero.\n");

    printf("OBSERVED: FLT_MIN (smallest normal) = ");
    print_double((double)FLT_MIN, 0);    // ~1.18e-38, prints 0.000... at low dec
    printf("\n          FLT_MIN bits             = ");
    print_f32_fields_binary(f32_bits(FLT_MIN));
    printf("   (exp = 1, the smallest normal exponent)\n");

    // Halve repeatedly and watch it cross from normal -> subnormal -> zero.
    volatile float v = FLT_MIN;
    printf("          halving from FLT_MIN:\n");
    for (int step = 0; step < 26; step++) {
        v = v / 2.0f;
        uint32_t b = f32_bits(v);
        uint32_t exp_raw = (b >> 23) & 0xFF;
        // only print the interesting transitions, not all 26 lines
        if (step < 3 || v == 0.0f || (step % 5 == 0)) {
            printf("            step %2d: bits = ", step + 1);
            print_f32_fields_binary(b);
            if (v == 0.0f)            printf("   -> ZERO (flushed)");
            else if (exp_raw == 0)    printf("   -> subnormal (no hidden 1)");
            else                      printf("   -> still normal");
            printf("\n");
        }
        if (v == 0.0f) break;
    }

    printf("VERDICT:  confirmed -- underflow is gradual: normal floats give way to\n");
    printf("          subnormals (exp=0, precision shrinking) before reaching zero.\n");
}

void probe_nan(void) {
    printf("\n--- PROBE: NaN (not a number) ---\n");
    printf("CLAIM:    NaN is a reserved value (exp all ones, mantissa NONzero).\n");
    printf("          It fails its own equality test, and contaminates any arithmetic.\n");

    // Make a NaN honestly: 0.0/0.0 is undefined. volatile so -O3 can't fold it.
    volatile float zero = 0.0f;
    float nan = zero / zero;

    printf("OBSERVED: 0.0/0.0 bits = ");
    print_f32_fields_binary(f32_bits(nan));
    printf("\n          isnan(result) = %d\n", isnan(nan));

    // The defining weirdness: NaN is not equal to itself.
    printf("          nan == nan ? %d   (every other value equals itself)\n", (nan == nan));
    printf("          nan != nan ? %d   (the ONE value where this is true)\n", (nan != nan));

    // Comparisons all return false -- NaN is unordered.
    printf("          nan <  1.0 ? %d\n", (nan < 1.0f));
    printf("          nan >  1.0 ? %d   (both false: NaN is unordered, not just 'big')\n", (nan > 1.0f));

    // Contamination: any arithmetic touching NaN yields NaN.
    printf("          nan + 1.0  -> %s\n", isnan(nan + 1.0f) ? "NaN" : "a number");
    printf("          nan * 0.0  -> %s   (not 0! NaN wins)\n", isnan(nan * 0.0f) ? "NaN" : "a number");

    // Contrast with infinity (also exp all ones, but mantissa ZERO) -- a finding:
    float inf = INFINITY;
    printf("          (contrast) inf == inf ? %d   (infinity DOES equal itself)\n", (inf == inf));

    printf("VERDICT:  confirmed -- NaN fails self-equality, is unordered against all values,\n");
    printf("          and propagates through arithmetic. Infinity, the other all-ones\n");
    printf("          exponent, is well-behaved by contrast -- the mantissa tells them apart.\n");
}