#pragma once

// Phase 2: the float, breaking down.
// The consequences of the grid: gaps grow, small additions vanish,
// order matters, and Kahan summation claws the lost bits back.

void probe_gap_grows(void);   // 2^24 + 1 collapses; the gap exceeds 1
void probe_epsilon(void);   // machine epsilon near 1.0, measured directly

// Run `count` additions of `inc` onto a float starting at `start`.
// Reveals when additions vanish because inc is smaller than the local gap.
void probe_accumulate(float start, float inc, long count, const char *note);