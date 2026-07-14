# QBIT NOVA V0.1 Effect Semantics

## guard

```qnova
guard owner_approved {
    emit "allowed"
} else {
    emit "denied"
}
```

`guard` requires a Boolean expression.

When true, the first block executes. When false, the optional `else` block executes. Without `else`, a false guard raises `QNOVA-E7001`.

The guard condition must be free of mutation and external effects.

## observe

```qnova
observe count
observe probs(dragon)
```

`observe` evaluates a value and emits an immutable observation record.

Observation of `probs(qbit)` returns probabilities without collapsing the qbit. `observe` cannot mutate classical or quantum state.

## simulate

```qnova
simulate {
    h dragon
    observe probs(dragon)
}
```

`simulate` executes inside a cloned state:

- classical bindings are copied
- qbit state is copied
- output is marked as simulated
- mutations are discarded on block exit
- file, process, network and hardware effects are forbidden

A failure inside simulation cannot partially modify outer state.

## patch

```qnova
patch count = count + 1
```

`patch` is a transactional update of one existing classical binding.

The new expression is evaluated first. The binding changes only when evaluation and type validation both succeed.

V0.1 forbids patching:

- qbits
- function names
- files
- processes
- network state
- services
- hardware state
