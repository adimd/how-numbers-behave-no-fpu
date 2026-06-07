#include <stdio.h>
#include "phase1_float_built.h"

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

    printf("\n--- end of run ---\n");
}