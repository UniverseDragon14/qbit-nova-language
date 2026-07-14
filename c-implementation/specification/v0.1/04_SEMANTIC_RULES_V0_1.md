# QBIT NOVA V0.1 Semantic Rules

## Types

V0.1 defines:

- `int`: signed 64-bit integer
- `real`: IEEE 754 binary64
- `bool`: `true` or `false`
- `string`: immutable UTF-8 string
- `bit`: classical value `0` or `1`
- `qbit`: non-copyable quantum-state reference
- `unit`: no value

## Evaluation order

Expressions evaluate strictly from left to right.

## Numeric conversion

`int` may promote to `real`. No other implicit conversion is permitted.

## Scope

- A block creates a lexical scope.
- Duplicate declarations in the same scope are errors.
- Shadowing an outer classical binding is allowed.
- Shadowing a qbit binding is forbidden.

## Mutability

- `let` creates a stable binding.
- `patch` performs a transactional classical update.
- qbit state changes only through quantum operations.
- qbit values cannot be copied, compared, serialized as ordinary values or returned from a function.

## Determinism

Compilation, AST generation, IR generation and QBC encoding must be deterministic.

Measurement is probabilistic in normal execution. Test harnesses must inject a named deterministic random source without changing language syntax.

## Resource limits

The runtime shall expose deterministic limits for:

- source bytes
- token count
- AST nodes
- call depth
- loop iterations
- qbit count
- QBC instructions
- execution steps

Exceeding a limit produces a defined error, never silent truncation.
