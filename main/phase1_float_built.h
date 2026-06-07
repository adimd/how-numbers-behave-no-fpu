#pragma once

// Phase 1: the float, constructed.
// Establish what a float actually IS on this chip before we break it.

void probe_float_anatomy(float value, const char *note);   // decompose & rebuild
void probe_rounding(double requested, const char *note);    // the snap to the grid