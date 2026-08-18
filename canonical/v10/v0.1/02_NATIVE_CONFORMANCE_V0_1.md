# QBIT NOVA V10 Native Conformance Map v0.1

This file records the implementation state observed while freezing the V10 canonical contract. It is deliberately conservative. `IMPLEMENTED` means the reviewed Native lineage contains the surface; it does not mean final V10 release approval.

## Reference checkpoints reviewed

- Frozen Native Stage7 Step9: `f13ccf4e279a792261a5f2cabbd6cd545cc86f0a`
- V10 media/voice foundation Step1 lineage
- V10 native-data Step2A lineage
- V10 QBC data-layout Step2B lineage
- V10 main CLI integration Step2C lineage
- Step2D regression-gate lineage

The complete V10 development state remains separate from a stable release claim.

## Conformance table

| Contract surface | Observed Native state | V10 conformance status |
|---|---|---|
| Native C17 implementation | Exists | IMPLEMENTED foundation |
| Deterministic legacy lexer/parser/AST | Exists | LEGACY IMPLEMENTED |
| Legacy typed QIR | qbit/qreg/result/u32/bool | LEGACY IMPLEMENTED |
| Legacy QBC v8/v9 | Frozen compatibility | FROZEN / PRESERVE |
| Legacy QVM | Quantum + bounded scalar/control/function paths | IMPLEMENTED for legacy profile |
| Capability guard | Deny-by-default mask/policy | IMPLEMENTED |
| Ed25519 approval | Signed source/capability binding | IMPLEMENTED |
| Trust store | Exists | IMPLEMENTED |
| Replay ledger | Atomic consume before VM | IMPLEMENTED |
| Revocation checks | Wired before replay in reviewed current docs | IMPLEMENTED lineage |
| Evidence receipt | Runtime receipt API/result metadata exists | IMPLEMENTED foundation |
| GPU routing | CPU/auto/Vulkan routing/proof lineage | IMPLEMENTED bounded profile |
| Runtime u32 inputs | ABI v1 | IMPLEMENTED legacy profile |
| Tensor frontend | f32/i8 metadata; V10 build boundary | PARTIAL; frontend only |
| V10 media ABI | f32/string/bytes/audio/voice request descriptors | IMPLEMENTED foundation only |
| V10 f32 source literal | Step2A | IMPLEMENTED gated/frontend |
| V10 string source literal | Step2A | IMPLEMENTED gated/frontend |
| V10 bytes source literal | Step2A | IMPLEMENTED gated/frontend |
| V10 typed data QIR | Step2A | IMPLEMENTED gated lane |
| V10 QBC DATA_ONLY | Step2B | IMPLEMENTED development profile |
| V10 normal CLI check/qir/build route | Step2C | IMPLEMENTED |
| V10 typed QVM execution | Explicitly fail-closed | NOT IMPLEMENTED |
| V10 PROGRAM typed opcode ABI | Not frozen in reviewed evidence | NOT IMPLEMENTED / REQUIRED NEXT |
| Step2D full regression PASS | CI status reviewed as empty/no success evidence | NOT CERTIFIED |
| Pi5 V10 proof | Not run in reviewed V10 steps | NOT RUN |
| Audio playback | No | NOT IMPLEMENTED |
| TTS synthesis | No | NOT IMPLEMENTED |
| Microphone capture | No | NOT IMPLEMENTED |
| Agent/event runtime | No | NOT IMPLEMENTED |
| Self-hosting compiler | No | NOT IMPLEMENTED |
| Physical QPU | Explicitly not claimed | NOT CLAIMED |

## Conformance gaps created by the canonical lock

The canonical contract intentionally exposes several places where a development proof is not yet the final V10 semantic contract:

### Identifier case

The reviewed Step2A V10 data lexer normalizes identifier text to lowercase. V10 canonical identifiers are case-sensitive. This must be corrected before V10-C0 conformance is claimed for that lane.

### f32 negative zero

The reviewed Step2A/Step2B path preserves raw f32 bits. V10 canonical serialization normalizes negative zero to positive zero. The encoder/lowering path needs an explicit canonicalization test.

### String NUL rule

The reviewed frontend rejects NUL for semantic string validation, while the Step2B QBC validation path did not consistently require that rule. V10 core strings forbid U+0000, so encode/decode validation must agree.

### V10 u32/bool unification

The reviewed frozen Native pipeline already implements u32 and bool, but Step2A V10 typed data is a separate lane. V10 must converge these into one typed semantic system instead of maintaining permanent language islands.

### Effects

Historical `guard`, `observe`, `simulate`, and transactional `patch` semantics are part of the canonical language direction, but the current Native main language does not yet implement that complete effect model.

### Executable QBC V10

Step2B proves a strict typed `DATA_ONLY` file. It is not evidence that an executable typed PROGRAM profile exists. The next bytecode milestone must freeze typed execution opcodes/value slots and then prove a full f32 vertical slice.

## Next proof target

The next decisive milestone after regression is not more surface syntax. It is:

```text
canonical f32 source
-> canonical AST
-> canonical typed QIR
-> QBC V10 PROGRAM profile
-> strict loader
-> QVM f32 execution
-> deterministic result/receipt
```

That vertical slice is the point where executable QBC V10 becomes real rather than only a typed-data container.
