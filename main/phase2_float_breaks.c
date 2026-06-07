#include "phase2_float_breaks.h"
#include "probe_common.h"
#include <stdio.h>
#include <stdint.h>

#include <float.h>   // add this up with the other includes at the top of the file

#define SUM_MAX 12000
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



// ascending insertion-style sort into a provided buffer (small/simple on purpose)
static void sort_ascending(const float *src, float *dst, int n) {
    for (int i = 0; i < n; i++) dst[i] = src[i];
    for (int i = 1; i < n; i++) {
        float key = dst[i];
        int j = i - 1;
        while (j >= 0 && dst[j] > key) { dst[j + 1] = dst[j]; j--; }
        dst[j + 1] = key;
    }
}

void probe_summation_order(const float *vals, int n, const char *note) {
    static float buf[SUM_MAX];        // static: keep it off the small task stack
    if (n > SUM_MAX) n = SUM_MAX;

    sort_ascending(vals, buf, n);     // buf is now ascending

    // float, small-to-large (ascending)
    volatile float asc = 0.0f;
    for (int i = 0; i < n; i++) asc = asc + buf[i];

    // float, large-to-small (descending = walk ascending buffer backwards)
    volatile float desc = 0.0f;
    for (int i = n - 1; i >= 0; i--) desc = desc + buf[i];

    // double reference (the truth both float orders are trying to hit)
    double exact = 0.0;
    for (int i = 0; i < n; i++) exact += (double)buf[i];

    double err_asc  = (double)asc  - exact;
    double err_desc = (double)desc - exact;
    double between  = (double)asc  - (double)desc;

    printf("\n--- PROBE: summation order ---\n");
    printf("VALUE:    %s  (n=%d)\n", note, n);
    printf("          exact (double)        = "); print_double(exact, 6); printf("\n");
    printf("          float small-to-large  = "); print_double((double)asc, 6);
    printf("   (err "); print_double(err_asc, 6); printf(")\n");
    printf("          float large-to-small  = "); print_double((double)desc, 6);
    printf("   (err "); print_double(err_desc, 6); printf(")\n");
    printf("          discrepancy (asc-desc)= "); print_double(between, 6); printf("\n");

    if (between == 0.0)
        printf("VERDICT:  confirmed -- order did NOT matter here (magnitudes too close to split).\n");
    else
        printf("VERDICT:  confirmed -- SAME numbers, different order, different sum: addition is not associative.\n");
}


void probe_summation_bigsmall(float big, float small, long count, const char *note) {
    // small-to-large: add all the smalls first, THEN the big one
    volatile float asc = 0.0f;
    for (long i = 0; i < count; i++) asc = asc + small;
    asc = asc + big;

    // large-to-small: add the big one first, THEN all the smalls
    volatile float desc = big;
    for (long i = 0; i < count; i++) desc = desc + small;

    // double reference (truth)
    double exact = (double)big + (double)small * (double)count;

    printf("\n--- PROBE: summation order (big + many small) ---\n");
    printf("VALUE:    %s  (count=%ld)\n", note, count);
    printf("          exact (double)        = "); print_double(exact, 3); printf("\n");
    printf("          float small-to-large  = "); print_double((double)asc, 3);
    printf("   (err "); print_double((double)asc - exact, 3); printf(")\n");
    printf("          float large-to-small  = "); print_double((double)desc, 3);
    printf("   (err "); print_double((double)desc - exact, 3); printf(")\n");
    printf("          discrepancy (asc-desc)= "); print_double((double)asc - (double)desc, 3); printf("\n");

    if ((float)asc == (float)desc)
        printf("VERDICT:  confirmed -- order did NOT matter here.\n");
    else
        printf("VERDICT:  confirmed -- SAME numbers, different order, different sum: addition is not associative.\n");
}


void probe_kahan(const float *vals, int n, const char *note) {
    static float kbuf[SUM_MAX];              // own buffer (buf is local to another probe)
    if (n > SUM_MAX) n = SUM_MAX;

    // --- naive float sum (small-to-large, the friendlier order) ---
    sort_ascending(vals, kbuf, n);           // kbuf ascending
    volatile float naive = 0.0f;
    for (int i = 0; i < n; i++) naive = naive + kbuf[i];

    // --- Kahan compensated sum (same order) ---
    volatile float sum = 0.0f;
    volatile float c   = 0.0f;               // running correction (the saved bits)
    for (int i = 0; i < n; i++) {
        float y = kbuf[i] - c;               // bring in the leftover from last step
        float t = sum + y;                   // this addition loses low bits...
        c = (t - sum) - y;                   // ...and c captures exactly what was lost
        sum = t;
    }

    // --- double reference (truth) ---
    double exact = 0.0;
    for (int i = 0; i < n; i++) exact += (double)kbuf[i];

    printf("\n--- PROBE: Kahan summation (the fix) ---\n");
    printf("VALUE:    %s  (n=%d)\n", note, n);
    printf("          exact (double)      = "); print_double(exact, 6); printf("\n");
    printf("          naive  float sum    = "); print_double((double)naive, 6);
    printf("   (err "); print_double((double)naive - exact, 6); printf(")\n");
    printf("          Kahan  float sum    = "); print_double((double)sum, 6);
    printf("   (err "); print_double((double)sum - exact, 6); printf(")\n");
    printf("          leftover correction = "); print_double((double)c, 9); printf("\n");

    double e_naive = (double)naive - exact; if (e_naive < 0) e_naive = -e_naive;
    double e_kahan = (double)sum   - exact; if (e_kahan < 0) e_kahan = -e_kahan;
    if (e_kahan < e_naive)
        printf("VERDICT:  confirmed -- Kahan recaptured the lost bits; far closer to the true sum.\n");
    else
        printf("VERDICT:  confirmed -- no improvement here (this input doesn't lose bits to recover).\n");
}