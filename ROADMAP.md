# Development Roadmap

## Phase 1 — Public foundation

- [x] Preserve frozen C-implementation specification v0.1
- [x] Publish dedicated language repository
- [x] Preserve links to earlier public lineage
- [ ] Select documentation and source-code licenses

## Phase 2 — V10 canonical language contract

- [x] Reconcile `.ud`, `.nova`, `.qnova`, and current `.qn` lineage
- [x] Select `.qn` as canonical V10 source extension
- [x] Define canonical project separation: Language = semantics, Native = reference implementation, QBIT NOVA C = separate frozen project
- [x] Reconcile core keyword direction (`emit`, `if`, `cx`, transactional `patch`)
- [x] Lock V10 core value direction (`u32`, `f32`, `bool`, `string`, `bytes`, `unit`)
- [x] Lock quantum semantic direction (`qbit`, `qreg<N>`, `result`)
- [x] Lock AST and typed-QIR invariants
- [x] Lock V8/V9 immutability and explicit QBC v10 version law
- [x] Adopt the existing V10 DATA_ONLY layout as typed-data ABI foundation
- [x] Define effect/capability/approval/receipt separation
- [x] Define compatibility and migration law
- [x] Define conformance levels and truth boundaries

Contract: `canonical/v10/v0.1/00_CANONICAL_CONTRACT_V0_1.md`

## Phase 3 — Native reconciliation and executable QBC V10

- [ ] Complete Step2D full regression certification
- [ ] Fix V10 identifier case-sensitivity conformance
- [ ] Canonicalize f32 negative zero and deterministic literal conversion
- [ ] Enforce canonical string NUL rule consistently
- [ ] Bring `u32` and `bool` into the unified V10 typed frontend/QIR
- [ ] Freeze V10 executable PROGRAM opcode/value-slot sub-specification
- [ ] Implement full f32 source -> AST -> QIR -> QBC v10 PROGRAM -> QVM vertical slice
- [ ] Add strict executable V10 loader validation
- [ ] Add deterministic executable V10 receipt proof

## Phase 4 — Core language completion

- [ ] Executable string/bytes operations where semantically defined
- [ ] Canonical `guard` language semantics
- [ ] Canonical immutable `observe`
- [ ] Canonical cloned-state `simulate`
- [ ] Canonical transactional local `patch`
- [ ] Compiler capability inference + explicit `requires`
- [ ] Runtime approval/replay/revocation/receipt binding for V10 programs
- [ ] Runtime input type expansion beyond proven u32 only when specified

## Phase 5 — Structured and quantum profiles

- [ ] `list<T>` specification + implementation
- [ ] Tensor QBC v10 serialization profile
- [ ] Tensor QVM/backend execution profile
- [ ] Quantum grammar reconciliation for qbit/qreg operations
- [ ] Deterministic seeded measurement test contract
- [ ] Optional future real-QPU adapter contract without physical-hardware misrepresentation

## Phase 6 — Media, voice and external profiles

- [ ] Audio value/profile specification
- [ ] Voice request language/profile specification
- [ ] Native audio playback contract
- [ ] TTS backend contract
- [ ] Microphone/capture capability contract
- [ ] Media receipts and resource limits

These are later profiles. The existing media ABI foundation is not itself a TTS/playback claim.

## Phase 7 — Agent / DRAGON / NOVA integration

- [ ] Versioned agent capability-request contract
- [ ] Event/trigger contract
- [ ] DRAGON policy/orchestration boundary
- [ ] NOVA reasoning/tool boundary
- [ ] Multi-node runtime contract
- [ ] Remote gateway/edge contract

Agents must request capability and cannot self-authorize.

## Phase 8 — Verification and release

- [ ] Full legacy + V10 regression green
- [ ] Golden grammar tests
- [ ] Negative/fail-closed tests
- [ ] Deterministic QBC golden hashes
- [ ] Sanitizer/Valgrind proof where applicable
- [ ] Pi5 V10 proof
- [ ] Termux packaging
- [ ] Documentation site update
- [ ] Stable V10 release review

## Phase 9 — Self-hosting

- [ ] Specify the minimum V10 subset required to implement compiler tooling
- [ ] Implement QBIT NOVA tokenizer/lexer in QBIT NOVA
- [ ] Implement parser/AST tooling in QBIT NOVA
- [ ] Implement QIR/QBC tooling in QBIT NOVA
- [ ] Compile the compiler with a previous trusted compiler
- [ ] Reproducible bootstrap comparison
- [ ] Remove bootstrap dependency only after proof
- [ ] Self-hosting GREEN

No self-hosting or independence claim is made until those proofs exist.
