#pragma once

// Phase 4: underneath the float -- the integer world the chip prefers.
// Two's-complement wrap, division truncation, and signed overflow as UB.

void probe_twos_complement(void);   // 100+50 goes negative; the wrap-around clock