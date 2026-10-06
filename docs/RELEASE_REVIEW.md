# Native v0.1.1 candidate review

Date: 2026-10-06, Asia/Colombo.
Baseline: 5665806852acece2601fd8d9e8d216ef9efc072f.
Candidate identifier: qbit-nova-language-native-v0.1.1-rc.1.

## Changes

- Exact signed64 comparisons for all six operators.
- Checked signed64 arithmetic and defined E6006 overflow diagnostic.
- Fresh-checkout creation of tests/out, fail-fast test setup, nonempty
  deterministic hashes, and configurable test binary for sanitizer builds.
- Regression tests, explicit migration checks, lineage/provenance review.
- CI definition now includes strict and ASan/UBSan builds.
- Frozen v0.1 specification and QBC0 0.1 format remain unchanged.
  The additive numeric amendment is recorded separately under v0.1.1.

## Observed local verification

| Check | Result |
|---|---|
| New comparison regression against old binary | Fails as expected, reproducing precision loss |
| Strict C11 build, warnings as errors | PASS |
| Original expectations/error/bytecode tests | 54 PASS |
| Comparison expectations, source and source-free QBC | 250 PASS |
| Arithmetic boundary checks, source and source-free QBC | 52 PASS |
| Explicit migration and legacy-rejection checks | 8 PASS |
| Deterministic comparison bytecode | PASS |
| AddressSanitizer / UndefinedBehaviorSanitizer with the above suite | PASS |
| Frozen v0.1 specification manifest | PASS |
| Historical Python v0.9 zero/one pipeline contract check | PASS |

These are 54 original plus 310 additional checks; the deterministic bytecode
check is separate. Sanitizers validate the exercised paths, not all possible
inputs. Hosted CI has not been claimed as passing by this local review.

## Release gates

Technical candidate review is complete within this scope. Public stable
publication is withheld pending the owner-selected code/documentation
license and provenance scope approval; see LICENSE_DECISION.md.
Repository documentation is prepared; no live documentation site or
portfolio has been redeployed as part of this repair.

The dedicated local release signing key is distinct from UDNOS and
authorization keys. A valid signature proves possession of that key,
not independent owner review or GitHub Verified status. Verification
instructions and the public key are under release/.

## Preserved boundaries

No C17 Native/GPU repository, SD-card backup, API key, DNS record, live
service, installed system executable, or portfolio is modified.
Historical dialects are not automatically translated. No general matrix
GPU execution, full legacy equivalence, self-hosting, or Python/C maturity
claim is made.
