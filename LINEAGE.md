# QBIT NOVA Language Lineage

This repository is the dedicated development home for QBIT NOVA Language.

## Existing public lineage

Earlier work remains in Universal-Dragon-Core:

- [QBIT NOVA Grammar Contract v0.2](https://github.com/UniverseDragon14/Universal-Dragon-Core/blob/nova-v1.4.0-dev/docs/QBIT_NOVA_GRAMMAR_CONTRACT_V0_2.md)
- [NOVA/QBIT Language Specification v0.3](https://github.com/UniverseDragon14/Universal-Dragon-Core/blob/nova-v1.4.0-dev/docs/spec/NOVA_QBIT_LANGUAGE_SPEC_V0_3.md)
- [NOVA/QBIT v0.3 Release Manifest](https://github.com/UniverseDragon14/Universal-Dragon-Core/blob/nova-v1.4.0-dev/docs/releases/NOVA_QBIT_V0_3_RELEASE_MANIFEST.md)
- [QBIT NOVA Token Parser Pipeline v0.9](https://github.com/UniverseDragon14/Universal-Dragon-Core/blob/nova-v1.4.0-dev/docs/QBIT_NOVA_TOKEN_PARSER_PIPELINE_V0_9.md)
- [QBIT NOVA installer v0.2](https://github.com/UniverseDragon14/Universal-Dragon-Core/blob/nova-v1.4.0-dev/tools/install_qnova_v02.sh)

## Current frozen layer

c-implementation/specification/v0.1/ is the frozen native implementation
contract. c-implementation/native/ now contains a C implementation of that
contract. It is not declared to silently replace earlier experimental
language/runtime lines.

## Current compatibility decision

- .qnova is canonical for native v0.1.
- .nova is an accepted native v0.1 source alias.
- .ud remains legacy lineage and is not silently reinterpreted.
- QBC0 version 0.1 is the native bytecode contract.
- Earlier experimental version labels are not treated as ordered upgrades or
  downgrades from the native C-contract version.

See c-implementation/native/COMPATIBILITY.md.

## Native v0.1 reconciliation review

The contract mapping and bounded runtime checks are documented in
[the native lineage review](docs/NATIVE_V01_LINEAGE_REVIEW.md).
Source hashes distinguish inspected files from branch-name assumptions.
The existing V10 reconciliation remains a separate development contract.

Candidate version: native v0.1.1-rc.1; QBC stays QBC0 0.1.
Public stable publication still requires the owner license decision in
[LICENSE_DECISION.md](docs/LICENSE_DECISION.md).
