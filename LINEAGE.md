# QBIT NOVA Language Lineage

This repository is the dedicated development home for future QBIT NOVA
Language work.

## Existing public lineage

Earlier work remains in
[Universal-Dragon-Core](https://github.com/UniverseDragon14/Universal-Dragon-Core):

- [QBIT NOVA Grammar Contract v0.2](https://github.com/UniverseDragon14/Universal-Dragon-Core/blob/nova-v1.4.0-dev/docs/QBIT_NOVA_GRAMMAR_CONTRACT_V0_2.md)
- [NOVA/QBIT Language Specification v0.3](https://github.com/UniverseDragon14/Universal-Dragon-Core/blob/nova-v1.4.0-dev/docs/spec/NOVA_QBIT_LANGUAGE_SPEC_V0_3.md)
- [NOVA/QBIT v0.3 Release Manifest](https://github.com/UniverseDragon14/Universal-Dragon-Core/blob/nova-v1.4.0-dev/docs/releases/NOVA_QBIT_V0_3_RELEASE_MANIFEST.md)
- [QBIT NOVA Token Parser Pipeline v0.9](https://github.com/UniverseDragon14/Universal-Dragon-Core/blob/nova-v1.4.0-dev/docs/QBIT_NOVA_TOKEN_PARSER_PIPELINE_V0_9.md)
- [QBIT NOVA installer v0.2](https://github.com/UniverseDragon14/Universal-Dragon-Core/blob/nova-v1.4.0-dev/tools/install_qnova_v02.sh)

## Current frozen layer

`c-implementation/specification/v0.1/` is a new implementation-contract
freeze. It is not yet declared to replace the earlier language/runtime work.

## Reconciliation pending

Before a stable release, development must reconcile:

- `.ud`, `.nova`, and `.qnova` source-extension history
- earlier Python bootstrap runtime behavior
- canonical grammar and keyword set
- QBIT simulation semantics
- AST, IR, and QBC formats
- final version-number sequence
- migration and compatibility rules
