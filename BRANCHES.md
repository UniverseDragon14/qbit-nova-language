# QBIT NOVA Language Branch Map

Inspected on **2026-09-01**. All **16 reachable branches** are accounted for.

| Branch | Inspected tip | Purpose / state |
|---|---|---|
| `agent/add-qbit-nova-landing-page` | `5c597124d102` | Original landing-page change |
| `agent/add-qbit-nova-landing-page-2` | `eb0a7448ec46` | Development-preview publication snapshot |
| `agent/add-qbit-nova-landing-page-3` | `eb0a7448ec46` | Duplicate pointer to the same preview snapshot |
| `agent/add-qbit-nova-landing-page-v2` | `51c7d02a0cf4` | Landing-page merge snapshot |
| `agent/add-qbit-nova-landing-page-v3` | `51c7d02a0cf4` | Duplicate pointer to landing-page merge |
| `agent/qbit-nova-landing-page-final` | `51c7d02a0cf4` | Duplicate pointer to landing-page merge |
| `agent/qbit-nova-landing-page-v1` | `51c7d02a0cf4` | Duplicate pointer to landing-page merge |
| `agent/qbit-nova-site` | `51c7d02a0cf4` | Duplicate pointer to landing-page merge |
| `agent/qbit-site-project-hub` | `3cabe00c3c40` | Makes QBIT NOVA C primary on the project hub |
| `aslam/qbit-nova-landing-page` | `51c7d02a0cf4` | Duplicate pointer to landing-page merge |
| `aslam/qbit-site-final` | `51c7d02a0cf4` | Duplicate pointer to landing-page merge |
| `dev/v10-canonical-contract-v0.1` | `661ae9ecc0c6` | Frozen V10 canonical contract/reconciliation docs |
| `landing-page-work` | `51c7d02a0cf4` | Duplicate pointer to landing-page merge |
| `main` | `190d4b1037f8` | Default specification/site branch; latest backup sync was reverted |
| `qbit-landing-page-approved` | `51c7d02a0cf4` | Duplicate pointer to landing-page merge |
| `qbit-landing-page-pr` | `51c7d02a0cf4` | Duplicate pointer to landing-page merge |

## What the pointers mean

Nine branch names currently point to the same commit `51c7d02a0cf4`. They do not contain nine separate implementations; they are duplicate names for one landing-page merge snapshot. Two more names point to `eb0a7448ec46`.

Every Git branch points to a commit. A branch can have **no unique commits**, but it cannot exist with literally no commit.

## Current source boundary

The default `main` branch publishes the frozen C-implementation specification v0.1 and the landing page. Its inspected tip is a revert of a private backup sync. Direct current-branch checks confirm that the transient `c-implementation/Makefile`, token/error C sources and tests from that reverted sync are **not present now**.

The separate `dev/v10-canonical-contract-v0.1` branch contains V10 canonical contract, reconciliation, native conformance and provenance-review documents. It is development documentation, not a complete compiler/runtime release.

## Accuracy boundary

QBIT/NOVA quantum syntax and examples describe symbolic or software simulation paths. Raspberry Pi hosting does not imply a physical QPU.
