#include <stdio.h>
#include "phase1_float_built.h"

void app_main(void)
{
    printf("\n========================================\n");
    printf("  how numbers behave on a chip with no FPU\n");
    printf("  baseline: compiled at -O3\n");
    printf("========================================\n");

    // ---- Phase 1: the float, constructed ----
    // A curated tour: each value exposes a different corner of the format.
    probe_float_anatomy(1.0f,   "1.0  -- cleanest case: mantissa all zeros, exponent = bias");
    probe_float_anatomy(6.75f,  "6.75 -- an exact binary fraction, reconstructs perfectly");
    probe_float_anatomy(0.1f,   "0.1  -- CANNOT be represented exactly; mantissa repeats");
    probe_float_anatomy(0.0f,   "0.0  -- special case: exponent field all zeros, no hidden 1");
    probe_float_anatomy(-2.0f,  "-2.0 -- flips the sign bit so you can see it act");
    probe_float_anatomy(1e7f,   "1e7  -- large value: watch the exponent climb");

    printf("\n--- end of run ---\n");
}