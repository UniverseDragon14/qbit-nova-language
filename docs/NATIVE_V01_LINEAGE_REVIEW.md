# Native v0.1 lineage and migration review

Reviewed 2026-10-06 (Asia/Colombo). This maps preserved experimental
dialects to the C11 implementation without merging QBIT NOVA C, C17 Native
or V10 into it.

## Evidence

Universal-Dragon-Core local HEAD:
`2967bc947de9025f4188baa533995192b9655b84`.
[SOURCE_PROVENANCE.tsv](SOURCE_PROVENANCE.tsv) identifies the actual bytes
reviewed and whether they match that commit.

Existing V10 reconciliation at Language commit
`661ae9ecc0c67834dd10e6667ec89c83867b588d`,
`canonical/v10/v0.1/`, was already present. Its `.qn`, `u32`,
`f32`, `if` and QBCN choices do not replace this C11 contract's
`.qnova`, signed64 `int`, binary64 `real`, `when` and QBC0.

## Behavioral mapping

| Historical behavior | Native C11 behavior | Migration decision |
|---|---|---|
| v0.2/v0.9 .ud identity declarations | Command selects native grammar, not filename | Explicit syntax migration; renaming is insufficient |
| nova/creator/team/brain declarations | No executable identity declarations | Preserve identity in project documentation |
| say "text" | emit "text" | Explicit output rewrite |
| qbit initialization, H and measurement | Statevector qbits; measure q -> result binds a bit | Bind and emit measurement explicitly |
| v0.9 grouped AST execution | Native statements execute in source order | Audit ordering; gates/output cannot be freely moved |
| v0.9 colon-based when on measured qbit name | Boolean when; qbit is not comparable; measurement yields bit | Redesign control flow using typed values |
| v0.3 decision-label simulate | Snapshot and rollback local state | Different meaning; manual redesign |
| v0.3 prob / observe q | observe probs(q) reports probabilities without collapse | Choose observation versus measurement explicitly |
| v0.3 state-dispatched guard | Boolean guard and effect checks | Rewrite conditions; retain external authorization separately |
| v0.3 file patch/replace | patch name = expression updates local binding | File mutation has no native migration |
| v0.3/v0.9 adapter contract output | Text output; no adapter executor | Emitted contracts do not grant permission |
| Earlier Python bootstrap | Independent Python parser/QVM; this compiler/VM is C11 | No runtime dependency or bytecode compatibility assumed |
| C17 Native Stage7 / V10 | Separate .qn, QBCN, GPU/capability line | Preserve separately; never reinterpret bytecode |

## Verification

- Historical tests/qbit_nova_v09_pipeline_check.py ran with bytecode writes
  disabled: QBIT_NOVA_V09_PIPELINE_CONTRACT_GREEN. This checks zero/one
  measurement examples and selected branches.
- Native tests/test_migration.py checks zero/one measurement rewrites with
  both .qnova and .nova (4 checks).
- It rejects legacy identity/output, brain/intent, file-patch, and
  colon-based branch forms (4 checks).
- Native guard/simulate/observe/patch remain covered by the original suite.

This completes the mapping of reviewed contract surfaces with bounded
runtime evidence. It does not claim all historical programs were ported,
all dialects are equivalent, or V10 execution is complete.

## Version naming

Candidate: qbit-nova-language-native-v0.1.1-rc.1.
The frozen baseline and QBC remain v0.1. The separate v0.1.1 amendment defines
overflow rejection. Earlier v0.2/v0.3/v0.9 labels are experimental lineage,
not upgrades or downgrades of this implementation release sequence.
