# QBIT NOVA v0.1 Compatibility Rules

## Canonical source form

- .qnova is the canonical source extension for this native v0.1 line.
- .nova is accepted as a source-file alias because the frozen v0.1 contract
  explicitly names both forms.
- .ud belongs to earlier Universal Dragon language lineage. It is not
  silently reinterpreted as v0.1 QBIT NOVA.

The CLI currently determines source language from the command used, not from
the filename suffix, so .nova and .qnova have identical v0.1 semantics.

## Keyword reconciliation

The frozen token vocabulary names h, x, and z as gate tokens while the frozen
minimal examples also use x as a classical identifier. The native parser
resolves that collision contextually:

- x q at statement position is the X gate.
- let x = 1, emit x, patch x = 2, function parameters, and equivalent name
  positions treat x as an identifier.

The lexer remains deterministic; only parser context resolves the role.

## Bytecode

Native v0.1 QBC uses magic QBC0, version 0.1, little-endian fields and a CRC32
payload checksum. Other historical bytecode/runtime formats are not silently
accepted as QBC v0.1.

## Earlier lineage

The repository's LINEAGE.md preserves links to grammar/runtime work called
v0.2, v0.3 and v0.9. Those version labels belong to earlier experimental
lines and are not ordered migrations from this C-implementation contract.
Migration must be explicit and source-tested; no automatic source rewrite is
performed by the v0.1 native CLI.
