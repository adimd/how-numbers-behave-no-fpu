#pragma once

// Phase 5: fixed-point -- the deliberate engineering answer.
// An integer with a hidden scale: value = stored_integer * 2^-n.
// Fast and native on a no-FPU chip; a uniform grid instead of the float's uneven one.

void probe_fixed_point_repr(double value, const char *note);   // build Q16.16, show it carries fractions
// The headline tradeoff: float spacing GROWS with magnitude (uneven, vast range);
// Q16.16 spacing is CONSTANT everywhere (uniform) but hits a hard range wall.
void probe_grid_compare(void);