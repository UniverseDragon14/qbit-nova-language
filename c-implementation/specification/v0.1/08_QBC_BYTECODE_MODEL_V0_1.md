# QBIT NOVA Bytecode Model V0.1

## Byte order

All multibyte integers use little-endian encoding.

## File header

The fixed 24-byte header is:

```text
offset size field
0      4    magic = "QBC0"
4      1    major_version = 0
5      1    minor_version = 1
6      2    flags
8      4    constant_pool_count
12     4    instruction_count
16     4    entry_instruction
20     4    payload_crc32
```

## Constant pool

Each entry uses:

```text
u8  tag
u8  flags
u16 reserved
u32 payload_length
u8  payload[payload_length]
```

Tags:

```text
0x01 INT64
0x02 REAL64
0x03 UTF8
0x04 SYMBOL
0x05 SOURCE_FILE
0x06 COMPLEX64_PAIR
```

## Instruction encoding

```text
u16 opcode
u8  operand_count
u8  flags
u32 operands[operand_count]
```

Operands reference registers, symbols, blocks or constant-pool indexes as defined by the opcode.

## Core opcodes

```text
0x0000 NOP
0x0001 CONST
0x0002 LOAD
0x0003 STORE
0x0004 ADD
0x0005 SUB
0x0006 MUL
0x0007 DIV
0x0008 MOD

0x0010 CMP_EQ
0x0011 CMP_NE
0x0012 CMP_LT
0x0013 CMP_LE
0x0014 CMP_GT
0x0015 CMP_GE

0x0020 JUMP
0x0021 JUMP_IF_FALSE

0x0030 CALL
0x0031 RETURN

0x0040 EMIT

0x0050 GUARD_BEGIN
0x0051 GUARD_END
0x0052 OBSERVE
0x0053 SIMULATE_BEGIN
0x0054 SIMULATE_END
0x0055 PATCH

0x0100 QALLOC
0x0101 QH
0x0102 QX
0x0103 QZ
0x0104 QCNOT
0x0105 QPROBS
0x0106 QMEASURE
0x0107 QRESET
```

## Validation

Before execution the loader must reject:

- incorrect magic
- unsupported version
- truncated data
- invalid constant-pool indexes
- unknown opcodes
- invalid operand counts
- entry point outside the instruction stream
- checksum mismatch
- resource limits exceeding runtime policy

## QBC determinism

Equivalent source compiled with the same compiler version and options must produce byte-identical QBC.
