# QBIT NOVA Language Lineage

This repository is the semantic/specification home for QBIT NOVA Language development.

History is preserved. New contracts do not rewrite old contracts to make them appear identical.

## Existing public lineage

Earlier work remains in `Universal-Dragon-Core`, including:

- QBIT NOVA Language DNA v0.1
- QBIT NOVA Mission Lock v0.1
- QBIT NOVA Self-Hosting Roadmap v0.1
- QBIT NOVA Grammar Contract v0.2 (`.ud`)
- NOVA/QBIT Language Specification and Grammar v0.3 (`.nova`)
- NOVA/QBIT v0.3 experimental contract release
- QBIT NOVA Token Parser Pipeline v0.9 (Python bootstrap host)
- earlier AST, IR, QVM and QBC concept contracts

These prove lineage and intent. They are not silently treated as current V10 syntax.

## Historical frozen C-specification layer

`c-implementation/specification/v0.1/` remains frozen as the first native-implementation specification contract.

Important preserved laws include:

- compact source may deterministically expand into larger AST/QIR/QBC,
- UTF-8 source with deterministic lexical behavior,
- guard / observe / simulate / patch effect semantics,
- typed AST/QIR direction,
- bounded resource limits,
- deterministic diagnostics,
- classical virtual quantum simulation without physical-QPU claims,
- clean-room implementation rather than mechanical Python translation.

The V10 contract does not modify those historical files.

## Native implementation lineage

QBIT NOVA Native became the separate C17 reference implementation/runtime track. Its lineage added real implementation proof for deterministic lexer/parser/AST/QIR/QBC/QVM, capability guard, signed approvals, trust/replay/revocation security, bounded execution, runtime inputs, control flow, functions, GPU routing/proof, tensor frontend work, and atomic bytecode publishing.

Frozen Stage7 Step9 remains a legacy reference boundary. V10 development proceeds after that boundary without reinterpreting frozen QBC v8/v9 bytes.

## V10 canonical reconciliation

`canonical/v10/v0.1/` resolves the previously pending language identity questions.

Key V10 decisions:

- canonical source extension: `.qn`
- `.ud`, `.nova`, `.qnova`: historical/migration lineage
- canonical classical core: `u32`, `f32`, `bool`, `string`, `bytes`, `unit`
- canonical quantum direction: `qbit`, `qreg<N>`, `result`
- canonical output keyword: `emit`
- canonical classical conditional: `if`
- canonical controlled-X spelling: `cx`
- canonical transactional local mutation: `patch`
- source `guard` cannot grant runtime capability or approval
- QBC v8/v9 are immutable
- QBC v10 is explicit `QBCN` version 10
- DATA_ONLY typed-data profile is retained as V10 foundation
- executable typed PROGRAM opcode ABI requires its own freeze before typed QVM execution
- self-hosting remains a long-term goal, not a current claim

## Version law

Version labels from different historical layers are not numerically collapsed into one fake sequence.

`QBIT NOVA V10 Canonical Contract v0.1` means:

```text
language generation = V10
contract revision    = 0.1
```

Future semantic changes require a new contract revision. Historical documents retain their original names and meanings.
