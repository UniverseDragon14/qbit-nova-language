# QBIT NOVA V10 Canonical Contract v0.1

Status: `FROZEN_FOR_V10_IMPLEMENTATION`

Contract revision: `0.1`
Language generation: `V10`
Canonical semantic home: `QBIT NOVA Language`
Reference native implementation: `QBIT NOVA Native`

This document reconciles the historical QBIT NOVA / NOVA language lineage with the current native C17 implementation direction. It is a design lock for continued V10 implementation. It is not a claim that the complete V10 runtime, QBC executable profile, self-hosting compiler, physical QPU, audio engine, TTS engine, agent system, or stable production release already exists.

## 1. Authority and project separation

For V10 work, authority is ordered as follows:

1. This V10 canonical contract defines what V10 means.
2. Versioned V10 sub-specifications may refine a surface only when they do not contradict this contract.
3. QBIT NOVA Native is the reference implementation of this contract.
4. Older QBIT NOVA Language, NOVA/QBIT, `.ud`, `.nova`, `.qnova`, Python bootstrap, and symbolic syntax documents are lineage and migration evidence.
5. QBIT NOVA C remains a separate frozen project. No source, bytecode, runtime, or release compatibility with QBIT NOVA C is implied unless separately specified.

Historical frozen artifacts are never rewritten to pretend they always meant V10.

## 2. Identity

QBIT NOVA is its own language and execution contract.

Host languages such as C and historical Python bootstrap code are implementation tools, not the public language identity.

Long-term direction remains:

```text
source
-> tokenizer / lexer
-> parser
-> AST
-> semantic analysis
-> typed QIR
-> QBC
-> QVM
-> guarded backend execution
-> evidence receipt
```

The long-term self-hosting goal remains valid: QBIT NOVA tooling should eventually be implementable in QBIT NOVA itself. Self-hosting must never be claimed before it is real.

## 3. Canonical source identity

### 3.1 Source extension

V10 canonical source extension:

```text
.qn
```

Lineage status:

- `.ud` = historical QBIT NOVA v0.2 lineage.
- `.nova` = historical NOVA/QBIT lineage.
- `.qnova` = historical/bootstrap/specification lineage.
- `.qn` = V10 canonical source extension.

Migration tooling may read historical extensions, but a compiler must not silently reinterpret historical files as V10 when semantics differ.

QBIT NOVA C also historically uses `.qn`. That does not make the projects identical. Toolchain/project identity and language contract version determine semantics; extension alone never proves compatibility.

### 3.2 Encoding and identifiers

- Source encoding is UTF-8.
- Invalid UTF-8 fails closed.
- V10 identifiers are ASCII letters, digits, and underscore.
- First character must be a letter or underscore.
- Identifiers are case-sensitive.
- Locale-sensitive token classification is forbidden.
- A V10 compiler must not lowercase or uppercase identifiers during semantic identity resolution.

### 3.3 Trivia and termination

Canonical line comments:

```text
# comment
// comment
```

Newline terminates a statement unless grammar context keeps it open. `;` is an explicit statement terminator.

## 4. Canonical lexical keywords

V10 core keyword direction:

```text
let fn call return
if else repeat
emit
guard observe simulate patch
requires input
qbit qreg h x z cx measure reset probs
true false
u32 f32 bool string bytes unit
```

Reserved for versioned V10 extensions:

```text
list tensor i8
import export module
agent on event
adapter native
async await
map capability
voice audio
```

Historical spellings are not automatically canonical:

- `say` -> historical output spelling; V10 uses `emit`.
- `when` -> historical condition spelling; V10 reserves it for future pattern/event semantics. Classical branching uses `if`.
- `cnot` -> historical gate spelling; V10 uses `cx`.
- `set` -> current legacy Native mutation spelling; canonical transactional mutation is `patch`.

Compatibility aliases, if implemented, must be explicit and must lower to identical canonical semantics.

## 5. Canonical declaration and control syntax

### 5.1 Typed bindings

```qbit
let count: u32 = 3
let gain: f32 = 0.75
let ready: bool = true
let greeting: string = "Hi bro 😊"
let packet: bytes = b"QBIT\x00NOVA"
```

A V10 binding is declared explicitly with `let`. Implicit declaration by assignment is not canonical V10 syntax.

### 5.2 Functions

Initial V10 canonical call form keeps the proven Native explicit-call boundary:

```qbit
fn add(left: u32, right: u32) -> u32 {
    let result: u32 = left + right
    return result
}

call add(a, b) -> answer
```

Parameters are typed immutable bindings. Function return type is explicit except for `unit` functions where omission may be allowed by a later grammar sub-specification.

### 5.3 Classical control flow

```qbit
if ready {
    emit count
} else {
    emit 0
}

repeat 3 {
    emit count
}
```

Repeat count is evaluated once, is bounded, and cannot create unbounded execution.

### 5.4 Transactional patch

```qbit
patch count = count + 1
```

`patch` means transactional update of one existing language-level classical binding:

1. evaluate the new expression,
2. validate its type and bounds,
3. commit only on success.

V10 core `patch` does not mean file modification. Historical file-patch syntax belongs to lineage only. File mutation is an external capability-gated effect and requires a separate API/module contract.

## 6. Canonical type system

### 6.1 Core classical values

| Type | Canonical V10 meaning |
|---|---|
| `u32` | unsigned 32-bit integer |
| `f32` | finite IEEE-754 binary32 |
| `bool` | `true` or `false` |
| `string` | immutable valid UTF-8 byte sequence, U+0000 forbidden in V10 core |
| `bytes` | immutable arbitrary byte sequence |
| `unit` | no value |

### 6.2 Quantum values

| Type | Meaning |
|---|---|
| `qbit` | one non-copyable quantum-state reference |
| `qreg<N>` | fixed-width quantum register |
| `result` | classical measurement result |

Quantum amplitudes are runtime/backend state and are not represented by the source `f32` type. State-vector implementations may use higher internal precision. Source `f32` does not reduce the required quantum-state numerical precision.

### 6.3 Structured V10 type constructors

Canonical reserved direction:

```text
list<T>
tensor<dtype>[shape...]
```

`list<T>` is specified as a V10 structured-type direction but is not yet an implemented Native type.

Tensor frontend work already exists in Native lineage. V10 tensor serialization/execution requires a separate versioned QBC/QVM profile before it is considered conforming.

Initial tensor dtype direction includes `f32` and `i8`; `i8` is not promoted to a general V10 scalar type by this contract.

### 6.4 Historical type reconciliation

Historical C-specification types do not receive unsafe silent aliases:

- historical `int` was signed 64-bit; it is not silently mapped to `u32`.
- historical `real` was binary64; it is not silently mapped to `f32`.
- historical `bit` must not be guessed into `bool` or `result` without migration context.

A future signed-integer or binary64 source type must use an explicit new canonical type name and specification.

## 7. Numeric semantics

### 7.1 `u32`

- Range: `0..4294967295`.
- Arithmetic is checked.
- Overflow, underflow, and division by zero fail closed; silent wraparound is not canonical language behavior.
- No implicit numeric conversion is allowed unless a future contract explicitly adds one.

### 7.2 `f32`

- IEEE-754 binary32.
- Only finite values are valid core V10 values.
- NaN and positive/negative infinity are rejected at language/runtime boundaries.
- Decimal literal conversion must be locale-independent and deterministic.
- Canonical rounding is IEEE round-to-nearest, ties-to-even.
- `-0.0` is semantically canonicalized to positive zero before QBC serialization.
- QBC canonical bits for zero are `0x00000000`.

This means a development implementation that preserves negative-zero source bits must be updated before claiming full V10 conformance.

## 8. String and bytes semantics

### 8.1 `string`

- Must be structurally valid UTF-8.
- Overlong encodings, surrogate code points, and values above U+10FFFF are invalid.
- U+0000 is forbidden in core V10 strings. Use `bytes` for arbitrary NUL-containing data.
- No Unicode normalization is performed silently.
- Source bytes are preserved after escape decoding.
- Length is explicit in bytes; C NUL termination is not part of the semantic value.

### 8.2 `bytes`

Canonical literal direction:

```qbit
b"QBIT\x00NOVA"
```

- Arbitrary bytes are allowed.
- `\xNN` uses exactly two hexadecimal digits.
- Length is explicit and bounded.
- `bytes` is never implicitly converted to `string`.

## 9. Quantum semantics

Canonical direction preserves the proven compact quantum model:

```qbit
qreg pair[2] := |00>
h pair[0]
cx pair[0] pair[1]
measure pair -> result
emit result
```

A single-qbit form may be represented by `qbit` as a width-one quantum binding.

Core laws:

- qbit/qreg values are non-copyable language resources.
- quantum state mutation occurs only through quantum operations.
- `probs` / observation must not collapse state.
- measurement may collapse state and is probabilistic unless a deterministic test seed/source is injected.
- identical compiler inputs must still produce deterministic AST, QIR, and QBC even when runtime measurement is probabilistic.
- ordinary CPU/GPU/Pi hardware is never described as a physical QPU.

## 10. Effect model

V10 separates language effects from execution authority.

### 10.1 `guard`

`guard` evaluates a pure Boolean condition and controls language execution. The condition cannot mutate state or perform external effects.

A source `guard` does not grant runtime capability, does not replace owner approval, and cannot override a blocked policy.

### 10.2 `observe`

`observe` produces immutable observation/evidence from a value without mutating language state. Quantum probability observation must not collapse state.

### 10.3 `simulate`

`simulate` executes in cloned language state:

- classical state cloned,
- quantum state cloned where supported,
- mutations discarded on exit,
- external file/process/network/device effects forbidden,
- failure cannot partially mutate outer state.

### 10.4 `patch`

`patch` is transactional local classical mutation as defined above.

### 10.5 External effects

External effects are outside the authority of `guard`, `observe`, `simulate`, and `patch`. They require explicit capabilities and runtime policy enforcement.

## 11. Capability contract

Capability metadata is a security boundary, not decoration.

Current canonical capability names inherited from the Native security lineage include:

```text
quantum.simulate
evidence.emit
compute.u32.add
compute.u32.scalar
model.exec
file.write
network
device.control
shell.exec
```

Default policy direction:

- safe computational/evidence capabilities may be allowed by policy,
- model/file/network/device capabilities require explicit approval by default,
- `shell.exec` remains blocked by default,
- unknown capabilities fail closed.

Canonical source declaration direction:

```qbit
requires model.exec
```

Compiler responsibility:

- infer the capabilities required by operations,
- combine inferred and explicitly declared requirements,
- never emit QBC that under-declares the actual effect surface.

Runtime responsibility:

- validate capability metadata before execution,
- apply deny-by-default policy,
- require valid approval when policy says approval is required,
- never allow source code to self-approve.

## 12. Approval contract

Approval is a runtime authorization layer outside ordinary language expression semantics.

The current Native Ed25519 approval lineage is retained as the V10 reference direction:

- approval bound to exact source digest,
- stable capability identity,
- trusted issuer identity,
- issue/expiry time,
- nonce/context,
- cryptographic signature,
- revocation check,
- replay prevention before execution.

A valid signature cannot approve a capability that policy classifies as blocked.

Approval tokens are never source literals and must never be embedded into QBC as secret authority.

## 13. Receipt contract

Every privileged or externally meaningful execution path should be able to produce an immutable evidence receipt.

V10 receipt semantics require, when applicable:

- contract/runtime version identity,
- exact source digest,
- exact QBC digest,
- requested and selected backend,
- requested capabilities,
- guard/policy decision,
- approval scheme and token digest without secret key material,
- issuer fingerprint when present,
- revocation/replay status when present,
- deterministic runtime-input digest when inputs are used,
- output/result digest,
- bounded-execution facts sufficient to audit the run.

Receipt serialization format is a separately versioned ABI. Human-readable prose is not a substitute for machine-verifiable fields.

## 14. AST canonical boundary

The AST preserves source meaning and source spans. It must not contain host-runtime pointers as semantic identity.

Required V10 node families include:

```text
Program
Block
LetDecl
FunctionDecl
Parameter
CallStmt
ReturnStmt
EmitStmt
IfStmt
RepeatStmt
GuardStmt
ObserveStmt
SimulateStmt
PatchStmt
RequiresStmt
InputDecl
QbitDecl
QregDecl
GateStmt
MeasureStmt
ResetStmt
TensorDecl
BinaryExpr
UnaryExpr
LiteralExpr
NameExpr
```

Every semantic source node carries at least:

```text
node_kind
source_file_id
start_line
start_column
end_line
end_column
```

Semantic analysis resolves symbols, types, scopes, quantum ownership, effect permissions, capability requirements, control-flow validity, and deterministic resource estimates.

## 15. QIR canonical boundary

Typed QIR is the canonical semantic boundary between AST and bytecode.

V10 QIR is a typed linear representation; SSA is not required by this contract.

Each instruction conceptually carries:

```text
instruction_id
basic_block_id
opcode
typed operands
result id
source span
effect class
```

Required invariants:

- deterministic instruction and value numbering,
- explicit value types,
- explicit control flow,
- bounded lowering expansion,
- capability/effect information cannot be lost,
- backend choice must not silently change language semantics,
- no host pointers in serialized representations.

## 16. QBC version law

### 16.1 Immutable historical bytecode

QBC v8 and QBC v9 are frozen historical Native formats.

They must never be reinterpreted as V10.

No V10 compiler may emit V10-only values into a file marked v8 or v9.

### 16.2 V10 identity

Canonical V10 identity:

```text
magic   = QBCN
version = 10
byte_order = little-endian
```

The existing 128-byte V10 header extension and 80-byte typed-value record form the canonical V10 typed-data ABI v1 foundation.

### 16.3 V10 DATA_ONLY profile

The existing Step2B development profile is retained as canonical `DATA_ONLY` profile v1:

- 128-byte header,
- `DATA_ONLY` flag,
- zero executable instruction/register/function/input counts,
- declaration-order typed-value table,
- 80-byte typed-value records,
- bounded contiguous constant pool,
- explicit source SHA-256,
- canonical offset/range validation,
- unknown/reserved fields fail closed.

`f32`, `string`, and `bytes` records follow the type semantics in this contract. Where the Step2B proof differs from this contract, this contract wins for future conformance, including negative-zero normalization and core-string NUL rejection.

### 16.4 V10 executable PROGRAM profile

`PROGRAM` is a separate V10 profile and is not considered released merely because `DATA_ONLY` exists.

Non-negotiable PROGRAM-profile rules are locked now:

- magic/version remain `QBCN` / `10`,
- header is explicit and versioned,
- legacy v8/v9 bytes are never reinterpreted,
- section offsets/sizes are validated before allocation or execution,
- typed values use explicit canonical type metadata,
- capabilities are explicit metadata,
- instructions are bounded and validated before VM entry,
- unknown opcodes/types/flags/reserved bits fail closed,
- output publishing is atomic,
- equivalent source + compiler version + options produces byte-identical QBC.

The exact new V10 typed-execution opcode table is a required versioned sub-specification before V10 QVM execution can be declared conforming. Until that sub-specification is frozen, V10 executable typed programs must fail closed rather than borrow an accidental legacy encoding.

## 17. Deterministic diagnostics

V10 canonical machine diagnostic namespace is:

```text
QN-E####
```

Historical `QNOVA-E####` codes remain lineage and must not be silently reused with changed meaning.

Rules:

- machine code is stable within a frozen contract revision,
- source location is stable and deterministic,
- no silent truncation,
- earliest source-location error wins when phases compete,
- phase order remains lexer -> parser -> semantic -> QIR/QBC validation -> runtime/policy,
- human prose may improve only when tooling does not depend on exact prose.

Existing Native diagnostic ranges may be grandfathered into the V10 registry only through an explicit registry document. V10 media/data development currently uses the `78xx` area; that does not authorize uncoordinated reuse.

## 18. Runtime input contract

Runtime inputs are explicit, typed, bounded, and receipt-bound.

Initial proven Native input type is `u32`.

Canonical direction:

```qbit
input price: u32
```

No missing, duplicate, unknown, malformed, or type-invalid runtime binding may be silently accepted.

## 19. Backend contract

Language semantics are backend-independent.

Runtime may select among versioned backends such as:

```text
cpu
auto
vulkan
```

Rules:

- backend selection cannot alter source meaning,
- unsupported backend requests fail closed or use an explicitly documented safe fallback,
- fallback must be recorded in the receipt,
- GPU use is an execution optimization, not a new language meaning,
- Pi5 is a classical trusted local node/lab, not a physical QPU,
- future real-QPU adapters require a separate backend/adapter contract.

## 20. Media and voice boundary

The existing Native media ABI foundation for `f32`, UTF-8 strings, bytes, normalized interleaved f32 audio buffers, and typed voice requests is valid V10 implementation research.

However:

- `audio` and `voice` are reserved extension surfaces, not core scalar types in this contract revision,
- TTS synthesis is not implied by a voice request descriptor,
- microphone capture is not implied,
- playback is not implied,
- no external voice service is part of core language identity,
- all future media effects need capability/resource rules and deterministic metadata contracts.

## 21. Agent, NOVA, DRAGON and external orchestration boundary

QBIT NOVA V10 is the language/execution contract. It must not conflate language semantics with a specific AI model or orchestration product.

Future agent syntax, event triggers, NOVA reasoning, DRAGON policy/orchestration, device coordination, cloud gateways, and UI assistants must consume QBIT NOVA through explicit versioned contracts.

A future agent can request capability; it cannot grant itself capability.

Canonical security direction remains:

```text
agent / caller requests operation
-> compiled capability requirements
-> policy / guard evaluation
-> signed approval when required
-> bounded execution
-> immutable receipt
```

## 22. Determinism and boundedness law

Every V10 implementation must define hard limits for relevant resources, including:

- source bytes,
- tokens,
- declarations / AST nodes,
- QIR instructions,
- QBC bytes / sections,
- constant-pool bytes,
- scalar/value slots,
- functions and call depth,
- runtime inputs,
- repeat iterations,
- execution steps,
- qbits/register width,
- tensors and tensor bytes,
- strings/bytes/media buffers where supported.

Limit violations fail closed. Silent truncation is forbidden.

## 23. Compatibility law

Compatibility is explicit, never magical.

- Historical source migration must identify the source dialect/version.
- Historical `.ud`, `.nova`, and `.qnova` syntax is not silently V10.
- Legacy Native syntax may remain supported by a legacy compiler lane.
- V8/V9 QBC remains decode-compatible only according to its own frozen contract.
- V10-only features never serialize into V8/V9.
- QBIT NOVA C is separate and is not merged into V10 by this contract.
- A migration tool must report any semantic change, especially `int/real`, `when/if`, `set/patch`, file-patch, gate spelling, and extension changes.

## 24. Conformance levels

A tool must state which level it actually satisfies.

```text
V10-C0  source + lexer/parser + AST + semantic validation
V10-C1  typed QIR + deterministic lowering
V10-C2D QBC v10 DATA_ONLY profile
V10-C2P QBC v10 PROGRAM profile
V10-C3  QVM execution for the claimed V10 value/opcode set
V10-C4  capability + approval + replay/revocation + receipt integration
V10-C5  structured types / tensor profile as separately specified
V10-C6  media/voice profile as separately specified
V10-C9  self-hosting compiler/toolchain
```

Partial implementations must not advertise a higher level.

## 25. Required next implementation order

The canonical continuation order is now:

1. preserve frozen Stage7 Step9 and legacy V8/V9 hashes,
2. complete Step2D full regression proof,
3. make V10 lexer/parser semantics conform to this contract (especially case-sensitive identifiers, canonical `let`, string NUL rule, f32 zero normalization),
4. unify `u32` and `bool` into the V10 typed frontend/QIR lane,
5. freeze the V10 PROGRAM opcode/type-slot sub-specification,
6. implement a complete `f32` source -> AST -> QIR -> QBC V10 PROGRAM -> QVM vertical slice,
7. extend executable value support to `string` and `bytes` where meaningful,
8. add `list<T>` and versioned tensor serialization/runtime profiles,
9. integrate canonical effect semantics (`guard`, `observe`, `simulate`, `patch`),
10. integrate capability inference/declaration with the V10 compiler,
11. bind V10 execution to approval/replay/revocation/receipt contracts,
12. only then expand media/voice, agent/event, multi-node, or external adapter surfaces,
13. progress toward self-hosting only after the language and bytecode contracts are stable enough to compile their own tooling.

This order deliberately prioritizes a complete vertical execution proof over adding broad surface area.

## 26. Truth boundary

At this contract freeze:

- historical language lineage exists and remains preserved,
- QBIT NOVA C remains separate,
- frozen Native Stage7 Step9 remains the legacy reference checkpoint,
- V10 native media/data foundations exist,
- V10 `f32/string/bytes` frontend/QIR work exists,
- V10 DATA_ONLY QBC layout exists,
- V10 main CLI build routing exists,
- full V10 typed QVM execution does not yet exist,
- full V10 PROGRAM opcode ABI is not yet frozen,
- Step2D full regression has not been certified by a successful CI status in the evidence reviewed for this contract,
- Pi5 V10 proof is not claimed,
- physical QPU is not claimed,
- TTS/playback/microphone execution is not claimed,
- agent/DRAGON/NOVA orchestration is not claimed as language-runtime implementation,
- stable V10 release/final production freeze is not claimed.

## 27. Change-control law

This file is frozen as contract revision `v0.1` for continued implementation.

Future semantic changes require a new contract revision, for example `v0.2`. Do not edit history to make an old contract appear to contain a newer decision.
