# Native implementation v0.1.1 numeric amendment

This additive implementation amendment leaves every frozen v0.1 file and
QBC0 0.1 encoding unchanged.

## Comparisons

Two int operands compare as signed 64-bit integers for all six operators,
without binary64 conversion. Mixed int/real comparisons retain v0.1
int-to-real promotion, including binary64 rounding.

## Overflow

Addition, subtraction, multiplication, division and unary negation producing
an integer outside [-2^63, 2^63-1] stop with:

    QNOVA-E6006 file:line:column integer overflow

E6006 is new in v0.1.1; it is absent from the frozen v0.1 registry.
Existing codes and diagnostic prose are unchanged. No wraparound,
saturation or host-language undefined overflow is used.

INT64_MIN % -1 returns zero without evaluating overflowing host division.
Division/remainder by zero retain E6001. Ordinary integer division truncates
toward zero; remainder retains the dividend's sign.

Represent INT64_MIN in source as (-9223372036854775807 - 1); the positive
literal 9223372036854775808 remains outside the accepted literal range.

## Compatibility and tests

In-range arithmetic and bytecode layout are unchanged. An older VM can
decode the bytecode but does not acquire these fixes: use this release's VM.

test_comparisons.py checks 125 comparisons through source and standalone
bytecode, including exact integer boundaries and mixed-real promotion.
test_arithmetic.py checks 26 arithmetic cases through both execution paths,
including overflow rejection and INT64_MIN remainder.
