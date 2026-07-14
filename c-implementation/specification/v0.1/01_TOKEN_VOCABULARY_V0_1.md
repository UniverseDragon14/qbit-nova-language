# QBIT NOVA V0.1 Token Vocabulary

## Source encoding

Source files use UTF-8. V0.1 identifiers are restricted to ASCII letters, digits and underscore for deterministic portable compilation.

## Trivia

- Space and horizontal tab separate tokens.
- Newline terminates a statement unless inside parentheses or braces.
- `;` may explicitly terminate a statement.
- `#` begins a line comment.

## Identifiers

```text
identifier = letter (letter | digit | "_")*
letter     = "A".."Z" | "a".."z" | "_"
digit      = "0".."9"
```

Identifiers are case-sensitive.

## Literals

- Integer: `0`, `1`, `42`, `-7`
- Real: `1.0`, `0.707`, `-3.5`
- Boolean: `true`, `false`
- String: `"dragon"`
- Ket initializer: `|0>`, `|1>`

## Keywords

```text
let fn return when else repeat true false emit
guard observe simulate patch
qbit bit h x z cnot measure probs reset
```

Reserved for later versions:

```text
adapter native import export async await
```

## Operators

```text
+ - * / % == != < <= > >= and or not
= -> : , . ( ) { } [ ]
```

## Token determinism

The lexer shall emit exactly one token stream for a given UTF-8 byte sequence. No locale-sensitive classification is permitted.
