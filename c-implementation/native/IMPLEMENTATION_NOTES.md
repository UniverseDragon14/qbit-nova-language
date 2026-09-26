# Native v0.1 Implementation Notes

## Determinism

Lexing, AST construction, semantic resolution, QIR lowering and QBC encoding
are deterministic for the same source bytes, compiler version and options.
QBC reproducibility is checked by the test suite.

## Resource policy

Default limits are exposed by qnova limits:

- source bytes: 1 MiB
- tokens: 100,000
- AST nodes: 100,000
- call depth: 64
- repeat iterations: 1,000,000
- qbits: 20
- QBC instructions: 200,000
- execution steps: 10,000,000

## Quantum representation

All qbits share one pure-state binary64-complex statevector, so CNOT and Bell
states preserve entanglement. Measurement uses marginal probabilities,
collapses the complete register, and renormalizes it.

reset is implemented as measurement followed by conditional X. This
guarantees the target qbit becomes |0> while preserving a valid pure-state
trajectory for the remaining simulated register.

Qbits declared inside a function remain allocated internally until program
termination after that call returns. They become unreachable from source,
but retaining their hidden subsystem avoids inventing an undefined
deallocation rule for entangled pure states. The global qbit resource limit
still applies.

## Effects

simulate snapshots classical frames and the complete quantum state. Its
mutations are rolled back at exit and its output is prefixed [sim].

guard conditions and observe expressions reject effectful function calls.
patch updates only an existing classical binding after value evaluation and
type conversion succeed.
