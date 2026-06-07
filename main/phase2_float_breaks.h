#pragma once

// Phase 2: the float, breaking down.
// The consequences of the grid: gaps grow, small additions vanish,
// order matters, and Kahan summation claws the lost bits back.

void probe_gap_grows(void);   // 2^24 + 1 collapses; the gap exceeds 1
void probe_epsilon(void);   // machine epsilon near 1.0, measured directly

// Run `count` additions of `inc` onto a float starting at `start`.
// Reveals when additions vanish because inc is smaller than the local gap.
void probe_accumulate(float start, float inc, long count, const char *note);

// Sum the same values in three ways: float ascending, float descending,
// and double (the reference truth). Reveals that order changes the float answer.
void probe_summation_order(const float *vals, int n, const char *note);
// Big + many small, summed both orders WITHOUT an array (no RAM blowup).
void probe_summation_bigsmall(float big, float small, long count, const char *note);
