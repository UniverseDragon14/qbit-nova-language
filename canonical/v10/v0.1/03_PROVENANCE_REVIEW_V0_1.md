# QBIT NOVA V10 Canonical Review Provenance v0.1

The V10 contract was produced by cross-reading preserved GitHub lineage instead of treating one historical document as the entire language.

## QBIT NOVA Language repository reviewed

Root lineage/status:

- `README.md`
- `LINEAGE.md`
- `ROADMAP.md`

Frozen C-implementation specification v0.1:

- `00_SPEC_FREEZE_V0_1.md`
- `01_TOKEN_VOCABULARY_V0_1.md`
- `02_GRAMMAR_V0_1.ebnf`
- `03_CORE_SYNTAX_V0_1.md`
- `04_SEMANTIC_RULES_V0_1.md`
- `05_GUARD_OBSERVE_SIMULATE_PATCH_V0_1.md`
- `06_QBIT_GATE_AND_MEASUREMENT_V0_1.md`
- `07_AST_AND_IR_BOUNDARIES_V0_1.md`
- `08_QBC_BYTECODE_MODEL_V0_1.md`
- `09_DETERMINISTIC_ERROR_MODEL_V0_1.md`
- `11_IMPLEMENTATION_ROADMAP_V0_1.md`
- `FREEZE_STATUS.txt`
- `PROVENANCE_REFERENCES.tsv`

The frozen spec is preserved unchanged by this V10 branch.

## Universal Dragon Core lineage reviewed

Key historical language intent/contract documents:

- `docs/QBIT_NOVA_LANGUAGE_DNA_V0_1.md`
- `docs/QBIT_NOVA_MISSION_LOCK_V0_1.md`
- `docs/QBIT_NOVA_SELF_HOSTING_ROADMAP_V0_1.md`
- `docs/QBIT_NOVA_GRAMMAR_CONTRACT_V0_2.md`
- `docs/QBIT_NOVA_SYNTAX_V0_2.md`
- `docs/spec/NOVA_QBIT_LANGUAGE_SPEC_V0_3.md`
- `docs/spec/NOVA_QBIT_GRAMMAR_V0_3.md`
- `docs/releases/NOVA_QBIT_V0_3_RELEASE_MANIFEST.md`
- `docs/QBIT_NOVA_TOKEN_PARSER_PIPELINE_V0_9.md`
- `docs/QBIT_NOVA_AST_V0_1.md`
- `docs/QBIT_NOVA_IR_V0_1.md`
- `docs/QBIT_NOVA_QBC_V0_1.md`
- `docs/QBIT_NOVA_QVM_V0_1.md`

These establish the compact-language, safety-first, bootstrap-truth, and self-hosting lineage but contain mutually incompatible historical syntax/version choices. V10 therefore reconciles them instead of copying one verbatim.

## QBIT NOVA Native lineage reviewed

Public Native documentation reviewed includes:

- root `README.md`
- `docs/ARCHITECTURE.md`
- `docs/TYPED_QIR_V02.md`
- `docs/CAPABILITY_GUARD_V03.md`
- signed approval / trust / replay / revocation lineage

Current Native implementation contracts reviewed include the lexer token model, AST model, typed QIR model, QBC model, guard/capability model, VM result/receipt surface, legacy QBC v8/v9 record sizes, frozen Stage7 Step9 truth boundary, and the V10 media/data/QBC/CLI development steps.

Important Native facts used by V10 reconciliation:

- `.qn` is the active Native source extension.
- Native already proves `u32`, `bool`, control flow, bounded repeat, functions, runtime inputs, quantum simulation, capability guard, approval, replay/revocation lineage, and evidence receipts.
- frozen Stage7 Step9 preserves QBC v8/v9 and fails tensor serialization closed before V10.
- V10 development proves bounded `f32/string/bytes`, typed data QIR, a strict QBC v10 DATA_ONLY typed-value profile, and normal CLI check/qir/build routing.
- typed V10 QVM execution remains intentionally unavailable in the reviewed evidence.

## QBIT NOVA C reviewed as a separation boundary

The QBIT NOVA C repository was reviewed to confirm that it is an existing separate C/QCPU/virtual-quantum project with its own `.qn` history and release lineage.

V10 does not import QBIT NOVA C history into QBIT NOVA Language semantics. Shared naming or file extension does not imply compatibility.

## Reconciliation method

Where sources agreed, the V10 contract preserves the shared law.

Where sources disagreed, V10 chooses one explicit rule and records the losing forms as historical/migration lineage.

Where current implementation exists but the canonical contract strengthens correctness, the implementation is marked as requiring conformance work instead of weakening the contract to match an accidental implementation detail.

Where no reviewed source proves an implementation, the V10 contract marks that surface as future/reserved rather than pretending it exists.
