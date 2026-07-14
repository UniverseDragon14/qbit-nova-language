# QBIT NOVA V0.1 AST and IR Boundaries

## AST responsibility

The AST preserves source structure and source locations.

Required node families:

```text
Program
Block
LetDecl
FunctionDecl
Parameter
ReturnStmt
EmitStmt
WhenStmt
RepeatStmt
GuardStmt
ObserveStmt
SimulateStmt
PatchStmt
QbitDecl
GateStmt
MeasureStmt
ResetStmt
BinaryExpr
UnaryExpr
CallExpr
ProbabilityExpr
LiteralExpr
NameExpr
```

Every AST node contains:

```text
node_kind
source_file_id
start_line
start_column
end_line
end_column
```

The AST contains no runtime pointers and no Python-specific objects.

## Semantic analysis output

Semantic analysis resolves:

- symbol identifiers
- inferred and declared types
- lexical scope
- qbit ownership
- effect permissions
- control-flow validity
- resource estimates

## QIR responsibility

QIR is a typed linear instruction representation.

QIR removes surface syntax while retaining:

```text
instruction id
basic-block id
opcode
typed operands
result id
source span
effect class
```

V0.1 does not require SSA form.

## Compact-to-expanded invariant

A compact statement may lower into multiple QIR instructions.

Example:

```qnova
measure dragon -> result
```

may lower into:

```text
QPROBS q0 -> p0, p1
QRANDOM p0, p1 -> b0
QCOLLAPSE q0, b0
STORE result, b0
```

The transformation must be:

- deterministic for a fixed compiler version
- bounded by configured expansion limits
- reproducible byte-for-byte
- independent of Python implementation details
