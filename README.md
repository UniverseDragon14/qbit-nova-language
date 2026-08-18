# QBIT NOVA Language

> Status: **Experimental — active development — V10 canonical reconciliation in progress**

QBIT NOVA Language is a Universal Dragon language project created by **Aslam / Universal Dragon**.

This repository is the semantic/specification home for QBIT NOVA. It defines what the language means. The separate QBIT NOVA Native project is the native reference implementation/runtime track.

## V10 canonical contract

The V10 reconciliation lock is authored under:

```text
canonical/v10/v0.1/
```

Start with:

- `00_CANONICAL_CONTRACT_V0_1.md`
- `01_RECONCILIATION_MATRIX_V0_1.md`
- `02_NATIVE_CONFORMANCE_V0_1.md`
- `FREEZE_STATUS.txt`

The V10 contract reconciles the preserved `.ud`, `.nova`, `.qnova`, C-specification, and current `.qn` Native lineages without rewriting history.

Core V10 direction includes:

```text
.qn source
-> lexer / parser
-> canonical AST
-> semantic validation
-> typed QIR
-> QBC v10
-> guarded QVM/backend execution
-> evidence receipt
```

The canonical V10 classical core is `u32`, `f32`, `bool`, `string`, `bytes`, and `unit`, with quantum `qbit`, `qreg<N>`, and `result` semantics. Structured/media/agent surfaces remain versioned later profiles.

QBC v8/v9 remain immutable historical Native formats. QBC v10 is explicit and must never reinterpret old bytecode.

## Preserved frozen C-implementation specification v0.1

The repository also preserves the earlier frozen:

**QBIT NOVA Language C-implementation specification v0.1**

It defines historical/reference contracts for:

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

That specification remains frozen evidence. V10 does not edit it in place.

## Project identity

These projects remain separate:

- **QBIT NOVA Language** — canonical language meaning/specification
- **QBIT NOVA Native** — native reference implementation/runtime
- **QBIT NOVA C** — separate frozen C/QCPU project
- **NOVA Language / Universal Dragon Core** — earlier runtime and language lineage

Nothing is automatically merged between these projects.

## Version clarification

Earlier experimental QBIT/NOVA documents and Python-hosted parser/runtime proofs already exist in Universal Dragon Core.

The historical `v0.1` under `c-implementation/specification/` refers to that frozen C-implementation specification revision.

`QBIT NOVA V10 Canonical Contract v0.1` means language generation V10, contract revision 0.1. It is not a downgrade or rewrite of earlier version history.

See [LINEAGE.md](LINEAGE.md).

## Accuracy notice

QBIT examples represent symbolic or software virtual quantum-state simulation unless a future adapter contract explicitly targets real quantum hardware. A Raspberry Pi can host the runtime, but it is not a physical quantum processing unit.

## Integrity check for the historical frozen spec

From the repository root:

```bash
cd c-implementation/specification/v0.1
sha256sum -c SPEC_MANIFEST.sha256
```

## License

A reuse license has not yet been selected. Public visibility is currently for development transparency and review.
