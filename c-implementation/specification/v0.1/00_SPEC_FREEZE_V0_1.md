# QBIT NOVA Language V0.1 Specification Freeze

## Status

- Version: `0.1`
- State: `FROZEN_FOR_FIRST_NATIVE_IMPLEMENTATION`
- Canonical implementation direction: `C`
- Recovered Python role: `READ_ONLY_BEHAVIOURAL_REFERENCE`
- Direct Python-to-C translation: `DENIED`
- Git initialization: `NOT APPROVED`
- C source implementation: `NOT STARTED`

## Purpose

QBIT NOVA is a compact, quantum-aware programming language designed to describe guarded classical logic, virtual quantum operations, observation, simulation and deterministic state transitions.

A short QBIT NOVA source program may lower into a larger AST, typed IR and QBC execution representation. This expansion is a compiler transformation. It is not evidence of physical quantum hardware or quantum speedup.

## V0.1 pipeline

```text
.nova / .qnova source
        |
        v
lexer
        |
        v
parser
        |
        v
AST
        |
        v
semantic analysis
        |
        v
typed QIR
        |
        v
QBC bytecode
        |
        v
QVM execution
```

## Clean-room contract

Recovered documents, examples and Python prototypes may define expected behaviour. They do not define the structure of the new C implementation.

The native implementation shall be written from this specification and its tests, not by mechanically translating Python source.
