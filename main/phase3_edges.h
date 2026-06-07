#pragma once

// Phase 3: the edges.
// Drive a float off both ends of the line -- overflow to infinity,
// underflow through subnormals to zero -- and meet NaN.

void probe_overflow(void);   // climb until the float saturates to +infinity
void probe_underflow(void);   // descend through subnormals to zero
void probe_nan(void);   // the value that isn't equal to itself, and contaminates everything