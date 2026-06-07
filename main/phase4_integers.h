#pragma once

// Phase 4: underneath the float -- the integer world the chip prefers.
// Two's-complement wrap, division truncation, and signed overflow as UB.

void probe_twos_complement(void);   // 100+50 goes negative; the wrap-around clock

// Integer division: truncates toward zero, remainder sign follows the dividend.
// Verifies the identity  q*divisor + r == dividend  each time.
void probe_division(int dividend, int divisor, const char *note);

// Signed overflow is undefined behavior, not a value. The hardware wraps when
// forced to compute; the optimizer may assume it never happens. Same chip, two answers.
void probe_signed_overflow(void);
