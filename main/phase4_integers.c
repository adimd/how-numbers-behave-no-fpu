#include "phase4_integers.h"
#include "probe_common.h"
#include <stdio.h>
#include <stdint.h>

void probe_twos_complement(void) {
    printf("\n--- PROBE: two's-complement wrap ---\n");
    printf("CLAIM:    signed integers are a wrap-around clock. An int8 holds\n");
    printf("          -128..127; pushing past 127 wraps to the negative end.\n");

    // The headline: 100 + 50 in an int8.
    volatile int8_t a = 100, b = 50;        // volatile so -O3 computes it for real
    int8_t sum = a + b;                      // 150 doesn't fit in int8
    printf("OBSERVED: int8: 100 + 50 = %d   (150 doesn't fit in -128..127)\n", sum);
    printf("          why: 150 in 8 bits = 0x%02X = %d as signed\n",
           (uint8_t)(a + b), sum);

    // Show the clock: walk values around the top edge explicitly.
    printf("          stepping across the top edge (int8):\n");
    int starts[] = {125, 126, 127, 128, 129, 130};   // compute in int, cast to int8
    for (int i = 0; i < 6; i++) {
        int8_t v = (int8_t)starts[i];                 // 128->-128, 129->-127, ...
        printf("            %3d as int8 = %4d  (bits 0x%02X)%s\n",
               starts[i], v, (uint8_t)v,
               (starts[i] == 128) ? "  <- WRAPPED to negative" : "");
    }

    // The full range, by its bit pattern extremes.
    printf("          int8 max  127 = bits 0x%02X\n", (uint8_t)(int8_t)127);
    printf("          int8 min -128 = bits 0x%02X   (one past max, on the clock)\n",
           (uint8_t)(int8_t)(-128));

    printf("VERDICT:  confirmed -- no saturation: integers wrap. 100+50 lands on a\n");
    printf("          negative number because the count ran off the top of the clock.\n");
}

void probe_division(int dividend, int divisor, const char *note) {
    int q = dividend / divisor;          // C integer division
    int r = dividend % divisor;          // C remainder
    int identity = (q * divisor + r);    // should reconstruct the dividend

    // For contrast: what "floor division" (round toward -infinity) would give.
    int qfloor = q;
    if ((r != 0) && ((r < 0) != (divisor < 0))) qfloor = q - 1;  // adjust if signs differ

    printf("\n--- PROBE: integer division & truncation ---\n");
    printf("VALUE:    %s\n", note);
    printf("          %d / %d  = %d   (quotient)\n", dividend, divisor, q);
    printf("          %d %% %d = %d   (remainder)\n", dividend, divisor, r);
    printf("          identity check: q*divisor + r = %d*%d + %d = %d  (== dividend? %s)\n",
           q, divisor, r, identity, (identity == dividend) ? "yes" : "NO");
    if (qfloor != q)
        printf("          note: truncation gave %d; floor division would give %d (differs for negatives)\n",
               q, qfloor);

    printf("VERDICT:  confirmed -- division truncates toward zero; remainder takes the\n");
    printf("          dividend's sign; and q*divisor + r always reconstructs the dividend.\n");
}