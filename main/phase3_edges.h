#pragma once

// Phase 3: the edges.
// Drive a float off both ends of the line -- overflow to infinity,
// underflow through subnormals to zero -- and meet NaN.

void probe_overflow(void);   // climb until the float saturates to +infinity