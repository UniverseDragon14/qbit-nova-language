# QBIT NOVA Language

> Status: **Native C v0.1.1 release candidate — numeric fixes verified —
> public stable release awaits owner license decision**

All **16 branches** were inspected on 2026-09-01. See [BRANCHES.md](BRANCHES.md) for the complete map, duplicate branch pointers, and the current source boundary.

QBIT NOVA Language is a Universal Dragon language project created by
**Aslam / Universal Dragon**.

## Current repository

The repository now contains both:

1. the frozen QBIT NOVA Language C-implementation specification v0.1
2. the first native C compiler/runtime implementation under
   c-implementation/native/

The native pipeline implements:

- deterministic UTF-8 lexer and token stream
- recursive-descent parser
- source-spanned AST
- semantic/type/effect validation
- typed linear QIR
- deterministic QBC 0.1 bytecode encoding
- strict QBC loader with CRC validation
- classical QVM execution
- guard, observe, simulate and patch
- lexical variables, functions, nested functions and recursion
- virtual qbits with H, X, Z, CNOT, probabilities, measurement and reset
- deterministic named RNG for tests
- bounded source/runtime resources and deterministic diagnostics

On Raspberry Pi 5, 54 original checks plus 310 comparison, arithmetic and
migration checks pass under strict and ASan/UBSan builds. The original frozen
specification SHA manifest is unchanged. See [RELEASE_REVIEW.md](docs/RELEASE_REVIEW.md)
for scope and [the numeric amendment](c-implementation/specification/v0.1.1/NUMERIC_CORRECTIONS.md)
for the new E6006 overflow diagnostic.

## Build and run

    cd c-implementation/native
    make
    make test
    build/qnova run examples/ex010.qnova --rng test:1

See:

- c-implementation/native/README.md
- c-implementation/native/COMPATIBILITY.md
- c-implementation/native/IMPLEMENTATION_NOTES.md

## Project identity

These projects remain separate:

- QBIT NOVA Language — this language contract and native implementation
- QBIT NOVA C — separate C/QCPU project
- NOVA Language / Universal Dragon Core — earlier runtime/language lineage

Nothing is automatically merged between those projects.

## Version clarification

Earlier experimental QBIT/NOVA documents and Python-hosted parser/runtime
proofs exist in Universal Dragon Core. The v0.1 here identifies the frozen
native C-implementation contract. It is not a downgrade of the earlier
experimental version labels.

See LINEAGE.md and the native compatibility document before migrating older
source.

## Accuracy notice

The QVM is a classical virtual quantum-state simulator. Running it on a
Raspberry Pi or GPU does not make that hardware a physical quantum processor.

## Frozen-spec integrity check

    cd c-implementation/specification/v0.1
    sha256sum -c SPEC_MANIFEST.sha256

## Release status

The native implementation is a verified local candidate, not a declared
public stable release. The owner must select the reuse license and confirm
its scope before stable publication. See [LICENSE_DECISION.md](docs/LICENSE_DECISION.md).

- [Getting started](docs/GETTING_STARTED.md)
- [Lineage and migration review](docs/NATIVE_V01_LINEAGE_REVIEW.md)
- [Candidate release review](docs/RELEASE_REVIEW.md)
- [Release signature verification](docs/release/SIGNING.md)
