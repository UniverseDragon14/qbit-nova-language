# QBIT NOVA V0.1 Native Implementation Roadmap

## Phase 0: freeze verification

- Review every V0.1 specification file.
- Assign specification test IDs.
- Confirm no recovered Python implementation was copied.
- Confirm the compact-to-expanded contract.

## Phase 1: C foundation

Planned components only:

```text
include/qnova/token.h
include/qnova/lexer.h
include/qnova/ast.h
include/qnova/parser.h
include/qnova/type.h
include/qnova/ir.h
include/qnova/qbc.h
include/qnova/qvm.h
include/qnova/error.h
```

No C source is created during specification freeze.

## Phase 2: lexer

- UTF-8 validation
- deterministic source locations
- tokens and literals
- lexical error tests

## Phase 3: parser and AST

- recursive-descent parser
- arena-owned AST nodes
- source-span preservation
- parser recovery restricted to test tooling

## Phase 4: semantic analysis

- scopes and symbols
- type checking
- qbit ownership
- effect checking
- resource estimates

## Phase 5: QIR

- typed linear instructions
- deterministic lowering
- compact-to-expanded limits
- source mapping

## Phase 6: QBC

- portable encoder
- strict loader
- checksum validation
- byte-identical reproducibility tests

## Phase 7: QVM

- classical execution
- guard/observe/simulate/patch transactions
- statevector simulator
- H, X, Z and CNOT
- measurement and collapse
- deterministic test RNG injection

## Phase 8: compatibility tests

Recovered examples become behavioural fixtures. Expected behaviour is re-authored as language tests; Python internals are not ported.

## Phase 9: adapters

Adapters, OpenQASM export and hardware bridges are out of V0.1 runtime scope. They require a separate specification and owner approval.

## Repository status

- Git initialized: `NO`
- GitHub repository created: `NO`
- C implementation started: `NO`
- Recovered runtime executed: `NO`
