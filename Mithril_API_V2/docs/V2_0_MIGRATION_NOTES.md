# Mithril v2.0.0 Migration Notes

Date: 2026-03-18

## Release Scope

`Mithril_API_V2` is now the primary stable cryptographic API line for the repository.

Version markers:

1. `MITHRIL_API_VERSION_MAJOR=2`
2. `MITHRIL_API_VERSION_MINOR=0`
3. `MITHRIL_API_VERSION_PATCH=0`
4. `mithril_version_string() == "2.0.0"`

## What Changed from v1

1. Provider-based architecture:
   - API entrypoints are backend-agnostic.
   - Backends are activated explicitly via provider registry.
2. Typed status model:
   - Explicit verification errors (`MITHRIL_ERR_AEAD_AUTH_FAILED`, `MITHRIL_ERR_SIGNATURE_INVALID`).
   - Internal/provider failures normalized as internal errors.
3. Expanded crypto surface in v2 runtime path:
   - RNG, HASH, AEAD, KEX, SIGN
   - Bigint/modarith on `c23` and `flint`.
4. CI and smoke coverage:
   - FLINT-enabled matrix.
   - Consumer migration smoke and ops toggle tests.

## Backward Compatibility Position

1. v1 wrappers exist for selected legacy flows (`compat` layer).
2. v1 is not the default integration target anymore.
3. New development must use v2 API directly.

## Migration Steps

1. Integrate headers from:
   - `/Users/kirtantegsingh/documents/mithril/Mithril_API_V2/include/mithril/`
2. Replace legacy init and provider assumptions with:
   - `mithril_init`
   - `mithril_provider_activate`
3. Replace legacy calls using:
   - `/Users/kirtantegsingh/documents/mithril/Mithril_API_V2/docs/V1_TO_V2_FUNCTION_MAPPING.md`
4. Validate with CI-equivalent local checks:
   - `cmake --build ...`
   - `ctest --output-on-failure`
   - `make -C /Users/kirtantegsingh/documents/mithril/examples/basic test`

## Ops Guidance for Progressive Rollout

Use Sprint 8 runbook:

1. `/Users/kirtantegsingh/documents/mithril/Mithril_API_V2/docs/ROLLOUT_ROLLBACK_RUNBOOK.md`

This defines canary and rollback toggles plus telemetry counters for runtime observation.

## Acceptance Criteria for v2 Stable Line

1. Version string remains `2.0.0`.
2. `Mithril_API_V2` CI gates pass.
3. Consumers run on v2 default path.
4. v1 line is flagged deprecated and kept in maintenance-only mode.

