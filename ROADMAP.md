# Development Roadmap

## Phase 1 — Public foundation

- [x] Preserve frozen C-implementation specification v0.1
- [x] Publish dedicated development repository
- [x] Preserve links to earlier public lineage
- [ ] Select documentation and source-code licenses

## Phase 2 — Canonical language contract

- [x] Reconcile .ud, .nova and .qnova behavior for native v0.1
- [x] Define canonical v0.1 keywords and grammar
- [ ] Complete behavioral mapping of v0.2, v0.3 and v0.9 lineage
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
- [ ] Documentation site publication/update
- [ ] Stable release review
- [ ] Select final reuse license before stable public release

Current native verification: 54/54 tests pass on Raspberry Pi 5.
