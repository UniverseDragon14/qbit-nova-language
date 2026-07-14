# QBIT NOVA V0.1 Deterministic Error Model

## Format

```text
QNOVA-E#### file:line:column message
```

The code and location are stable. Diagnostic prose may only change in a new language specification version.

## Lexical errors

```text
QNOVA-E1001 invalid source character
QNOVA-E1002 unterminated string literal
QNOVA-E1003 malformed numeric literal
QNOVA-E1004 invalid UTF-8 sequence
```

## Parser errors

```text
QNOVA-E2001 unexpected token
QNOVA-E2002 expected token missing
QNOVA-E2003 invalid qbit declaration
QNOVA-E2004 unmatched delimiter
QNOVA-E2005 invalid statement terminator
```

## Semantic errors

```text
QNOVA-E3001 undefined name
QNOVA-E3002 duplicate declaration
QNOVA-E3003 type mismatch
QNOVA-E3004 condition is not Boolean
QNOVA-E3005 illegal qbit copy
QNOVA-E3006 invalid patch target
QNOVA-E3007 return type mismatch
QNOVA-E3008 invalid effect in context
```

## Quantum errors

```text
QNOVA-E4001 unknown qbit
QNOVA-E4002 duplicate qbit declaration
QNOVA-E4003 CNOT control equals target
QNOVA-E4004 invalid or non-normalized state
QNOVA-E4005 invalid measurement target
QNOVA-E4006 qbit resource limit exceeded
```

## Bytecode errors

```text
QNOVA-E5001 invalid QBC magic
QNOVA-E5002 unsupported QBC version
QNOVA-E5003 invalid constant-pool index
QNOVA-E5004 unknown opcode
QNOVA-E5005 truncated bytecode
QNOVA-E5006 checksum mismatch
```

## Runtime errors

```text
QNOVA-E6001 division by zero
QNOVA-E6002 call-depth limit exceeded
QNOVA-E6003 return outside function
QNOVA-E6004 instruction limit exceeded
QNOVA-E6005 repeat limit exceeded
```

## Guard and effect errors

```text
QNOVA-E7001 guard denied
QNOVA-E7002 forbidden effect inside simulate
QNOVA-E7003 mutation attempted by observe
QNOVA-E7004 patch transaction failed
```

## Selection rule

When multiple errors are possible, report the earliest source location. At one location, phase priority is:

```text
lexer -> parser -> semantic -> quantum -> bytecode -> runtime
```
