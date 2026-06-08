# How Numbers Behave on a Chip Without a Floating-Point Unit

An empirical investigation, run entirely on an ESP32-C3-mini board, into how
floating-point and integer arithmetic actually behave on real silicon, with a clear
verdict on each behavior.

> A longer write-up of this investigation, with the full story and the surprises
> found along the way, is here: **[How Numbers Behave on a Chip With No FPU](https://adithyamd.com/Projects/how-numbers-behave-no-fpu.html)**

## Objective

Computer arithmetic is usually taught as a set of clean rules. This project does not
trust the rules in the abstract. It treats the chip as a measuring instrument and
checks them.

The goals, in order:

1. Take each claim about how numbers work on a machine (how floats are built, how
   they round and break down, what happens at the limits of the range, how integers
   wrap, what fixed-point buys you) and turn each one into a minimal program that
   runs on the board.
2. Observe what this specific chip actually does when pushed to each edge, recording
   the raw bits where it matters rather than a tidied-up decimal.
3. Deliver a verdict for every behavior: confirmed exactly, confirmed with a
   hardware-specific twist, or diverges from the standard story.

The chip is an ESP32-C3-mini (single-core 32-bit RISC-V, no floating-point unit),
running at 160 MHz. The missing FPU is the whole reason this is worth doing: floats
are emulated in software while integer and fixed-point math is the fast, native path,
so the costs and edge cases that a desktop hides are forced into the open here.

## Why it is laid out this way

The investigation is split into five phases, and the order is the argument, not just
a filing system. It walks the number system from the top down:

1. First it builds the float and shows what it is.
2. Then it shows the float breaking down under ordinary use.
3. Then it drives the float off both ends of the range.
4. Then it drops underneath the float to the integers the chip actually prefers.
5. Then it builds fixed-point as the deliberate engineering answer to everything the
   earlier phases diagnosed.

So the float is constructed, then broken, then pushed to its cliffs, which motivates
going underneath to integers, which in turn motivates fixed-point as the principled
choice on a chip with no FPU. Each phase is one source file, and `main.c` calls the
probes in that order, so flashing once prints the whole story top to bottom.

| Phase | File | What it establishes |
|-------|------|---------------------|
| 1. The float, constructed | `phase1_float_built.c` | float anatomy (sign, exponent, mantissa, the hidden 1); rounding snaps a value to the nearest grid point |
| 2. The float, breaking down | `phase2_float_breaks.c` | the gap that grows past 2^24; machine epsilon; additions that freeze and that creep; summation order; Kahan summation as the fix |
| 3. The edges | `phase3_edges.c` | overflow saturating to infinity; underflow through subnormals to zero; NaN |
| 4. Underneath the float | `phase4_integers.c` | two's-complement wrap; division truncation; signed overflow as undefined behavior |
| 5. Fixed-point | `phase5_fixed_point.c` | Q16.16 (an integer with a hidden scale); the uniform fixed-point grid versus the float's uneven one |

Shared bit-level helpers (reading a float's raw bits, printing fields, a decimal
formatter) live in `probe_common.c`, so every phase reports results the same way.

## What it finds

A few of the results, to show the kind of thing the probes turn up:

- `0.1` cannot be stored: it snaps to the nearest grid point, and you can see the
  repeating mantissa cut off and rounded in the raw bits.
- `16777216 + 1` equals `16777216`: past 2^24 the gap between representable floats is
  larger than 1, so adding 1 changes nothing.
- The same 1,000,001 values summed in two different orders differ by exactly
  1,000,000, because float addition stops being associative once the magnitudes span
  the gap. Kahan summation recovers the lost bits.
- `NaN != NaN`: the one value that fails its own equality test, is unordered against
  everything, and contaminates any arithmetic it touches.
- `int8: 100 + 50 = -106`: signed integers are a wrap-around clock, not a saturating
  one.
- Signed overflow produces two contradictory answers in a single run: forced to
  execute, the chip wraps `INT_MAX + 1` to `INT_MIN`, but the optimizer, free to
  assume overflow never happens, folds `(x + 1) > x` to true. Same expression, same
  silicon, two legal answers, because the behavior is undefined.
- Fixed-point keeps one constant fine step everywhere the float's spacing grows with
  magnitude, but hits a hard range wall where the float keeps going (coarsely) out to
  about 3.4e38.

## Build and run

Built with ESP-IDF v5.5.3. From the project root:

    idf.py set-target esp32c3
    idf.py -p <YOUR_PORT> flash monitor

`<YOUR_PORT>` is typically `COMx` on Windows, `/dev/ttyACM0` on Linux, or
`/dev/cu.usbmodem*` on macOS. Exit the monitor with `Ctrl-]`. Output is serial only,
at 115200 baud, and needs no extra hardware.

### The optimization baseline

The project is compiled at `-O3`, on purpose. This is not the ESP-IDF default. It is
chosen so the optimizer is maximally free to exploit undefined behavior, which is
what makes the signed-overflow result in Phase 4 visible at all. Every verdict in the
output should be read against this baseline, and the banner prints it on each run.

## Reading the output

Each probe prints a small block:

    --- PROBE: <name> ---
    CLAIM:    what the standard account predicts
    OBSERVED: what the chip actually produced (often the raw bits)
    VERDICT:  confirmed, confirmed with a twist, or diverges

Floats are shown as raw IEEE-754 bit patterns and through an integer-based decimal
formatter, not through `printf("%f")`, which prints unreliably under the default
nano-libc on this target. When a printed decimal and the bits appear to disagree, the
bits are the ground truth.

## Notes

These are deliberate choices made so the output stays honest on this specific chip:

- No `%f`. Float values are formatted from their bits or via integer math, so the
  output is correct regardless of the libc float-formatting setting.
- Scheduler yields. The long software-float loops are slow enough on a no-FPU chip
  that the run yields to the scheduler between heavy probes, so the task watchdog
  stays satisfied instead of resetting the board.
- Decimal printer range. The integer-based printer refuses to print values beyond
  2^31 (it prints `(>2^31, see bits)` rather than a wrapped, misleading number), so
  read the bit pattern for those.

## License

MIT. You are free to use, modify, and share this, including for your own teaching or
projects, as long as the copyright notice is kept. See the `LICENSE` file for the
full text.

Copyright (c) 2026 adimd