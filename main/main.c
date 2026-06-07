#include <stdio.h>
#include "phase1_float_built.h"
#include "phase2_float_breaks.h"

static void phase_banner(const char *title) {
    printf("\n#########################################\n");
    printf("# %s\n", title);
    printf("#########################################\n");
}

void app_main(void)
{
    printf("\n========================================\n");
    printf("  how numbers behave on a chip with no FPU\n");
    printf("  baseline: compiled at -O3, CPU 160 MHz\n");
    printf("========================================\n");

    // =====================================================================
    phase_banner("PHASE 1: THE FLOAT, CONSTRUCTED");
    // =====================================================================

    // 1a. Anatomy -- what a float is made of.
    probe_float_anatomy(1.0f,  "1.0  -- cleanest case: mantissa all zeros, exponent = bias");
    probe_float_anatomy(6.75f, "6.75 -- an exact binary fraction, reconstructs perfectly");
    probe_float_anatomy(0.1f,  "0.1  -- CANNOT be represented exactly; mantissa repeats");
    probe_float_anatomy(0.0f,  "0.0  -- special case: exponent field all zeros, no hidden 1");
    probe_float_anatomy(-2.0f, "-2.0 -- flips the sign bit so you can see it act");
    probe_float_anatomy(1e7f,  "1e7  -- large value: watch the exponent climb");

    // 1b. Rounding -- what happens when a value isn't on the grid.
    probe_rounding(0.1, "0.1 -- the classic: snaps slightly");
    probe_rounding(0.2, "0.2 -- also off-grid");
    probe_rounding(0.3, "0.3 -- snaps too; this is what sets up 0.1+0.2 later");
    probe_rounding(0.5, "0.5 -- exact (a power of two): lands with NO snap");

    // =====================================================================
    phase_banner("PHASE 2: THE FLOAT, BREAKING DOWN");
    // =====================================================================

    // 2a. The gap grows -- past 2^24, the spacing exceeds 1.
    probe_gap_grows();

     // 2b. Machine epsilon -- the gap at 1.0, measured.
    probe_epsilon();

    // 2c. Accumulation -- when additions stick, creep, or vanish entirely.
    probe_accumulate(1.0f,        1.0f,       1000000, "inc >> gap: moves perfectly");
    probe_accumulate(1.0f,        0.00000005f, 1000000, "inc < eps/2: truly FROZEN near 1.0");
    probe_accumulate(1.0f,        0.00001f,   1000000, "inc > epsilon: moves but creeps");
    probe_accumulate(16777216.0f, 1.0f,       1000000, "2^24, gap=2: +1 frozen");
    probe_accumulate(8388608.0f,  1.0f,       1000000, "2^23, gap=1: +1 still works");


    // 2d. Summation order -- same numbers, different order, different answer.

// Case A: big ABOVE 2^24 + many small, array-free (no RAM cost).
    // 1e8 >> 2^24, so its gap is large; adding 1.0 large-first vanishes.
    probe_summation_bigsmall(1.0e8f, 1.0f, 1000000, "1e8 plus 1,000,000 x 1.0 (big above 2^24)");

    // Case B: many equal mid-size values (generated). No giant; subtler drift.
    static float many_equal[10000];
    for (int i = 0; i < 10000; i++) many_equal[i] = 0.1f;       // 10000 x 0.1
    probe_summation_order(many_equal, 10000, "10000 x 0.1");

    // Case C: explicit small illustrative set -- mixed magnitudes, fully visible.
    static const float mixed[] = { 1.0e6f, 0.5f, 0.5f, 0.25f, 0.25f };
    probe_summation_order(mixed, 5, "1e6 + 0.5 + 0.5 + 0.25 + 0.25 (spelled out)");

    // Case D: benign control -- similar magnitudes, order should NOT matter.
    static const float benign[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };
    probe_summation_order(benign, 5, "1+2+3+4+5 (control: no magnitude spread)");

    // 2e. Kahan summation -- recover the bits naive summation throws away.
    static float ten_k_tenths[10000];
    for (int i = 0; i < 10000; i++) ten_k_tenths[i] = 0.1f;
    probe_kahan(ten_k_tenths, 10000, "10000 x 0.1 -- naive creeps; Kahan should recover");

    static const float kahan_mixed[] = { 1.0e6f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f };
    probe_kahan(kahan_mixed, 6, "1e6 + five 0.1 -- big swamps small unless compensated");


    printf("\n--- end of run ---\n");
}