# QBIT NOVA Native C v0.1

This directory contains the first native C implementation of the frozen
QBIT NOVA Language C-implementation specification v0.1.

## Implemented pipeline

    .qnova / .nova source
      -> deterministic UTF-8 lexer
      -> recursive-descent parser
      -> source-spanned AST
      -> semantic/type/effect validation
      -> typed linear QIR
      -> deterministic QBC 0.1 encoder/loader
      -> QVM

The QVM implements classical expressions, lexical scopes, nested functions,
recursion, when, bounded repeat, guard, observe, transactional patch,
cloned-state simulate, and the v0.1 virtual quantum operations: qbit, H, X,
Z, CNOT, probabilities, measurement/collapse, and reset.

The quantum runtime is a classical statevector simulator. It makes no claim
that Raspberry Pi or other host hardware is a physical quantum processor.

## Build

    make

Strict compiler check:

    gcc -Iinclude -std=c11 -Wall -Wextra -Wpedantic -Werror       src/qnova.c src/main.c -lm -o build/qnova

## CLI

    qnova check program.qnova
    qnova run program.qnova
    qnova run program.qnova --rng test:1
    qnova tokens program.qnova
    qnova ast program.qnova
    qnova qir program.qnova
    qnova compile program.qnova -o program.qbc
    qnova disasm program.qbc
    qnova run-qbc program.qbc --rng test:1
    qnova limits

--rng test:<seed> is the named deterministic random source for repeatable
measurement tests. Normal execution seeds the simulator from system time.

## Verification

    make test
    make check-spec

The test suite covers the frozen EX001-EX015 expectations, functions,
recursion, simulation rollback, deterministic bytecode, the documented
diagnostic families, and malformed QBC loader cases.

Generated binaries and test QBC files are intentionally ignored by Git.
