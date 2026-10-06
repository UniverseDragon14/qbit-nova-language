# QBIT NOVA Language native candidate

Requires a C11 compiler, make, and libm. Tests also require Python 3 and Bash.
No AI API key or Python runtime is needed to execute the compiled VM.

From the repository root:

    cd c-implementation/native
    make
    make test
    make check-spec
    build/qnova run examples/recursion.qnova

The last command prints 120. Compile and execute standalone bytecode:

    build/qnova compile examples/recursion.qnova -o build/recursion.qbc
    build/qnova run-qbc build/recursion.qbc

Canonical source extension: .qnova; .nova is accepted with the same grammar.
Earlier .ud and separate Native .qn dialects require explicit migration.

Strict and sanitizer verification:

    make BUILD=build-strict CFLAGS="-O2 -g -std=c11 -Wall -Wextra -Wpedantic -Werror" test
    ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 make BUILD=build-sanitize CFLAGS="-O1 -g -std=c11 -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer" LDLIBS="-lm -fsanitize=address,undefined" test

Mixed int/real arithmetic retains floating-point rounding. Int/int comparisons
are exact, and integer arithmetic overflow reports E6006. See the
[numeric amendment](../c-implementation/specification/v0.1.1/NUMERIC_CORRECTIONS.md).

This candidate is not a general-purpose ecosystem comparable in maturity to
Python/C. It is not self-hosted or a physical quantum processor. Existing
resource limits bound execution; this review is not a security certification
for executing arbitrary hostile bytecode.
