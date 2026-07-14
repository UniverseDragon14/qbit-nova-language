# QBIT NOVA Language

> Status: **Experimental — active development — unfinished**

QBIT NOVA Language is a Universal Dragon language project created by
**Aslam / Universal Dragon**.

This public repository is a development preview. It is not currently a
finished general-purpose language, stable compiler, or production runtime.

## Current publication

This repository currently contains the frozen:

**QBIT NOVA Language C-implementation specification v0.1**

It defines:

- token vocabulary
- formal grammar
- core syntax and semantic rules
- guard, observe, simulate and patch boundaries
- QBIT gate and measurement semantics
- AST and IR boundaries
- QBC bytecode model
- deterministic error behavior
- minimal examples
- implementation roadmap

No C compiler or C runtime has been published yet.

## Project identity

These projects remain separate:

- **QBIT NOVA Language** — language specification and future implementation
- **QBIT NOVA C** — separate C/QCPU project
- **NOVA Language / Universal Dragon Core** — earlier runtime and language lineage

Nothing is automatically merged between these projects.

## Version clarification

Earlier experimental QBIT/NOVA documents and Python-hosted parser/runtime
proofs already exist in Universal Dragon Core.

The `v0.1` in this repository refers specifically to the newly frozen
**C-implementation specification**, not a downgrade of the overall language
history.

See [LINEAGE.md](LINEAGE.md).

## Accuracy notice

QBIT examples currently represent symbolic or virtual quantum-state
simulation. A Raspberry Pi can host the runtime, but it is not being claimed
as a physical quantum processing unit.

## Official page

[install.universaldragon.com](https://install.universaldragon.com/)

The site currently documents the existing NOVA Termux runtime. A separate
development-repository link will be added after this repository is verified.

## Integrity check

Run these commands from the repository root:

    cd c-implementation/specification/v0.1
    sha256sum -c SPEC_MANIFEST.sha256

## License

A reuse license has not yet been selected. Public visibility is currently for
development transparency and review.
