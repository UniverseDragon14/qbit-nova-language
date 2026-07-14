# QBIT NOVA V0.1 Core Syntax

## Variables

```qnova
let count = 1
let ratio: real = 0.707
let ready: bool = true
```

The compiler infers a type when no annotation is present.

## Functions

```qnova
fn add(a: int, b: int) -> int {
    return a + b
}
```

Functions use lexical scope. Parameters are immutable bindings. Local `let` bindings may be changed only through `patch`.

## Conditional execution

```qnova
when count == 1 {
    emit "ready"
} else {
    emit "waiting"
}
```

## Repetition

```qnova
repeat 3 {
    emit "dragon"
}
```

The repeat count is evaluated once and must be a non-negative integer.

## Compact quantum source

```qnova
qbit dragon = |0>
h dragon
observe probs(dragon)
measure dragon -> result
emit result
```

The source remains compact. The compiler may lower it into multiple AST, QIR and QBC records. Expansion must be deterministic and bounded.
