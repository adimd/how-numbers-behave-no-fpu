#pragma once

// Phase 1: the float, constructed.
// Establish what a float actually IS on this chip before we break it.

// Decompose one float into sign/exponent/mantissa and rebuild it from the bits.
// `note` explains why this particular value is in the tour.
void probe_float_anatomy(float value, const char *note);