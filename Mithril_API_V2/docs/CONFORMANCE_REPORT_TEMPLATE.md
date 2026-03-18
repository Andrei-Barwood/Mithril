# Conformance Report Template (c23 vs flint)

Use this template to document Sprint 2 conformance results between `c23` and `flint`.

## 1. Metadata

- Date (UTC): `YYYY-MM-DDTHH:MM:SSZ`
- Branch: `codex/mithril-v2-sprints`
- Commit: `<git-hash>`
- Platform: `<OS / compiler / arch>`
- Build flags:
  - `MITHRIL_ENABLE_PROVIDER_FLINT=ON`
  - `MITHRIL_USE_FLINT=ON`
  - `MITHRIL_ENABLE_TESTS=ON`

## 2. Test Commands

```zsh
cmake -S . -B build-cmake-flint \
  -DCMAKE_BUILD_TYPE=Debug \
  -DMITHRIL_ENABLE_TESTS=ON \
  -DMITHRIL_USE_LIBSODIUM=ON \
  -DMITHRIL_ENABLE_PROVIDER_FLINT=ON \
  -DMITHRIL_USE_FLINT=ON

cmake --build build-cmake-flint --parallel
ctest --test-dir build-cmake-flint -R "conformance_vectors|conformance_randomized" --output-on-failure
```

## 3. Deterministic Vectors

- Test name: `conformance_vectors`
- Status: `PASS | FAIL`
- Divergences: `<number>`
- Notes:
  - `<optional>`

## 4. Randomized Parity Summary

- Test name: `conformance_randomized`
- Status: `PASS | FAIL`
- Master seed: `<hex-seed>`
- Cases per operation: `<n>`

| Operation | Cases | Divergences |
|---|---:|---:|
| `bigint_add` | `<n>` | `<d>` |
| `bigint_sub` | `<n>` | `<d>` |
| `bigint_mul` | `<n>` | `<d>` |
| `modarith_add_mod` | `<n>` | `<d>` |
| `modarith_mul_mod` | `<n>` | `<d>` |
| **TOTAL** | `<5*n>` | `<sum>` |

## 5. Sprint 2 DoD Checklist

- [ ] 1000+ cases green per operation
- [ ] Divergences report = 0
- [ ] `conformance_vectors` passed
- [ ] `conformance_randomized` passed

## 6. Failure Handling (if applicable)

If divergences are non-zero, include:

1. Reproduction seed and operation.
2. First failing case index.
3. Input vectors (`a`, `b`, `modulus` if any).
4. `c23` status/output/written.
5. `flint` status/output/written.
6. Root cause hypothesis and next action.

## 7. Sign-off

- Implementer: `<name>`
- Reviewer: `<name>`
- Decision: `APPROVED | BLOCKED`
- Follow-ups: `<links to issues/PRs>`
