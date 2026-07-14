# QBIT NOVA V0.1 QBIT Semantics

## State model

A single qbit state is:

```text
alpha|0> + beta|1>
```

where `alpha` and `beta` are complex amplitudes satisfying:

```text
|alpha|² + |beta|² = 1
```

The implementation shall use binary64 real and imaginary components.

## Initialization

```qnova
qbit dragon = |0>
qbit light = |1>
```

Only `|0>` and `|1>` source literals are valid in V0.1.

## Gates

```qnova
h dragon
x dragon
z dragon
cnot dragon, light
```

Matrices:

```text
H = 1/sqrt(2) * [[1, 1], [1, -1]]
X = [[0, 1], [1, 0]]
Z = [[1, 0], [0, -1]]
```

`cnot control, target` applies controlled-X. Control and target must be different qbits.

## Probabilities

```qnova
observe probs(dragon)
```

returns:

```text
p0 = |alpha|²
p1 = |beta|²
```

and does not collapse state.

## Measurement

```qnova
measure dragon -> result
```

Measurement:

1. computes `p0` and `p1`
2. samples one classical bit
3. stores the bit when a target is supplied
4. collapses the measured qbit
5. renormalizes the complete register

## Reset

```qnova
reset dragon
```

Reset deterministically places the qbit in `|0>`.

## Canonical display

Probability and amplitude displays use three decimal places:

```text
dragon: |0>=1.000+0.000i |1>=0.000+0.000i
dragon: |0>=0.707+0.000i |1>=0.707+0.000i
```

The historical Python `j` suffix is not canonical V0.1 syntax. Native QBIT NOVA uses `i`.

## Hardware claim boundary

The V0.1 QVM is a classical virtual quantum simulator. It must never report that Raspberry Pi hardware is a physical quantum processor.
