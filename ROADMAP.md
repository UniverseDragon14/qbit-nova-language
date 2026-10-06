# Development Roadmap

## Phase 1 — Public foundation

- [x] Preserve frozen C-implementation specification v0.1
- [x] Publish dedicated development repository
- [x] Preserve links to earlier public lineage
- [ ] Select documentation and source-code licenses

## Phase 2 — Canonical language contract

- [x] Reconcile .ud, .nova and .qnova behavior for native v0.1
- [x] Define canonical v0.1 keywords and grammar
- [x] Map reviewed v0.2/v0.3/v0.9 contract surfaces; record explicit incompatibilities and bounded migration tests
- [x] Freeze v0.1 AST/IR boundaries and QBC format
- [x] Define native v0.1 compatibility and migration rules

See c-implementation/native/COMPATIBILITY.md.

## Phase 3 — Native implementation

- [x] Implement deterministic tokenizer and UTF-8 validation
- [x] Implement recursive-descent parser
- [x] Implement semantic/type/effect validation
- [x] Implement source-spanned AST and typed linear QIR
- [x] Implement deterministic QBC 0.1 encoder and strict loader
- [x] Implement deterministic diagnostics
- [x] Implement native C QVM runtime
- [x] Implement classical statevector H/X/Z/CNOT/probability/measurement/reset
- [x] Implement guard/observe/simulate/patch semantics
- [x] Implement functions, lexical scope, nested calls and recursion
- [x] Expose deterministic named test RNG and resource limits

## Phase 4 — Verification and release

- [x] Unit/runtime tests
- [x] Frozen EX001-EX015 golden expectations
- [x] Negative/error-model and malformed-QBC tests
- [x] Strict -Werror build
- [x] AddressSanitizer/UndefinedBehaviorSanitizer verification
- [x] GitHub CI workflow
- [x] Linux/Termux local installer
- [x] Prepare repository documentation, migration review and numeric amendment
- [ ] Publish stable documentation with the owner-approved release
- [x] Complete technical candidate review; see docs/RELEASE_REVIEW.md
- [ ] Owner license/provenance decision and public stable release
- [ ] Select final reuse license before stable public release

Current candidate verification: 54 original checks + 310 added comparison,
arithmetic and migration checks pass on Raspberry Pi 5; strict and ASan/UBSan
builds pass. This is bounded validation, not a claim of Python/C maturity.
