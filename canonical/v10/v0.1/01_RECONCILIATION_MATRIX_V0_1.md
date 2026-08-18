# QBIT NOVA V10 Reconciliation Matrix v0.1

This matrix records how the V10 canonical contract reconciles preserved lineage with current Native implementation reality. Historical artifacts are not deleted or rewritten.

| Surface | Historical lineage | Current Native lineage | V10 canonical decision | Migration / implementation consequence |
|---|---|---|---|---|
| Project identity | QBIT NOVA, NOVA/QBIT, Universal Dragon language experiments | QBIT NOVA Native C17 | QBIT NOVA Language defines semantics; QBIT NOVA Native implements them | Keep Language and Native separate repos/roles |
| QBIT NOVA C | Separate C/QCPU project | Explicitly kept separate by Native docs | Separate frozen project | No automatic merge or compatibility claim |
| Source extension | `.ud`, `.nova`, `.qnova` | `.qn` | `.qn` canonical V10 | Historical extensions require explicit migration/dialect handling |
| Output keyword | `say`, later `emit` | `emit` | `emit` | `say` is historical compatibility only |
| Conditional keyword | `when` | `if` | `if` | Reserve `when` for future pattern/event semantics |
| Mutation keyword | historical local `patch`; older v0.3 also used file `patch`; Native uses `set` | `set` for scalar mutation | `patch` = transactional local language-state update | File mutation moves to capability-gated external API; `set` legacy alias only |
| Controlled-X spelling | `cnot` | `cx` | `cx` | `cnot` may be explicit migration alias |
| Declarations | `let name [: type] = expr` | `let` for u32; V10 Step2 uses typed `let` | explicit typed `let` is canonical | No implicit declaration by assignment in canonical V10 |
| Functions | normal function concept; call expressions in C-spec | `fn`, explicit `call ... -> ...` | explicit `call` statement for initial V10 | Expression-call sugar can be added later without changing core call semantics |
| Integer type | historical `int` = signed 64-bit | `u32` proven | `u32` core | No silent `int -> u32` mapping |
| Real type | historical `real` = binary64 | V10 Step2 `f32` | `f32` core finite binary32 | No silent `real -> f32` mapping |
| Boolean | `bool` | bool represented in typed scalar path | `bool` | Preserve explicit Boolean semantics |
| String | immutable UTF-8 in C-spec | Step1/Step2 UTF-8 validators and constant pool | immutable UTF-8, core NUL forbidden | Decoder/validator must enforce canonical NUL rule |
| Bytes | not core in early language spec | Step1/Step2 bounded bytes | `bytes` core | No implicit string/bytes conversion |
| List | not implemented | not implemented | `list<T>` reserved structured V10 type | Requires separate implementation profile |
| Tensor | not in early core C-spec | Stage7 Step9 frontend, f32/i8 metadata, V10 serialization fail-closed | `tensor<dtype>[shape...]` reserved/canonical direction | QBC/QVM tensor profile required before execution claim |
| Qbit syntax | single `qbit` and `cnot` in early docs | `qreg`, indexed targets, `cx`, GHZ support | `qbit` and `qreg<N>` semantic types; `cx` gate | Preserve compact quantum semantics; no physical-QPU claim |
| Guard | Boolean/effect boundary in C-spec; safe-output concept in v0.3 | separate runtime capability guard/policy | source `guard` is language control/effect boundary only | Source guard cannot grant runtime capability or approval |
| Observe | immutable observation; probabilities non-collapsing | not fully integrated in current main Native language | canonical effect | Implement without state mutation |
| Simulate | cloned state, discard mutation, external effects forbidden | runtime has quantum simulation but no canonical source `simulate` block | canonical effect | Add after core PROGRAM execution slice |
| Patch | transactional classical update in C-spec | scalar `set` operations | transactional local update | External file patch requires `file.write` capability contract |
| Capabilities | safety/approval ideas in lineage | explicit capability mask and deny-by-default guard | explicit compiler/runtime security metadata | Infer + declare; never under-declare; unknown fails closed |
| Approval | owner approval concepts | Ed25519 signed approval, trust, replay, revocation | runtime authorization layer | Not ordinary language value; blocked capability cannot be approved |
| QIR | early symbolic IR, later typed linear IR | typed QIR with qbit/qreg/result/u32/bool and tensor metadata | typed linear semantic boundary | Add f32/string/bytes and effect/capability metadata without host pointers |
| Historical QBC | early symbolic formats and C-spec `QBC0` concept | Native QBC v1-v9 `QBCN`; v8/v9 frozen | never reinterpret old bytes | V8/V9 remain immutable |
| QBC V10 | previously pending | Step2B `QBCN` v10 DATA_ONLY typed-value layout | V10 explicit; DATA_ONLY profile retained | PROGRAM typed opcode profile must be separately frozen before typed QVM execution |
| Diagnostics | `QNOVA-E####` in early C-spec | `QN-E...` in Native | `QN-E####` machine namespace direction | Build explicit registry; no silent code reuse |
| Backend | future CPU/QPU bridge concepts | CPU/auto/Vulkan routing, evidence | backend-independent semantics | Backend/fallback recorded; Pi5 not physical QPU |
| Audio/voice | not core in old contract | Step1 bounded media/voice ABI foundation | reserved V10 extension | No TTS/playback/mic claim until separate profile |
| Agents/events | future direction only | not language runtime feature | reserved future extension | Agents request capability; cannot self-authorize |
| Self-hosting | explicit mission goal | Native currently C17 | long-term conformance goal | Do not claim before compiler/tooling is actually written in QBIT NOVA |

## Decisions that intentionally override earlier syntax direction

V10 deliberately selects implementation-aligned, lower-ambiguity choices where lineage disagrees:

```text
.ud / .nova / .qnova -> .qn
say                  -> emit
when                 -> if
cnot                 -> cx
set                   -> patch (canonical transactional mutation)
```

These are versioned V10 decisions, not claims that older documents were wrong at the time they were written.

## Decisions where historical semantics remain stronger than current implementation

The following historical design laws remain valuable and are promoted into V10 even when current Native does not yet fully implement them:

- `guard` condition purity,
- immutable `observe`,
- cloned-state `simulate`,
- transactional `patch`,
- deterministic AST/QIR/QBC,
- bounded resource use,
- compact-to-expanded lowering,
- no fake physical quantum claim,
- self-hosting as a long-term target.

## Current implementation corrections required by the V10 lock

Known conformance work includes:

1. V10 native-data identifier handling must become case-sensitive instead of normalizing identifier spelling.
2. `f32 -0.0` must canonicalize to positive zero before canonical QBC serialization.
3. Core V10 string validation must reject U+0000 consistently at encode and decode boundaries.
4. V10 `u32` and `bool` must join the typed V10 frontend/QIR rather than remain only in the legacy lane.
5. Canonical `patch` semantics must be integrated without confusing it with historical file-patch syntax or legacy `set` spelling.
6. The executable V10 PROGRAM typed opcode/value-slot profile must be frozen before typed QVM execution is enabled.
7. Full regression evidence must be green before broadening the runtime surface.
