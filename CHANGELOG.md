# Changelog

All notable changes to Mithril are documented in this file.

## [2.0.0] - 2026-03-18

### Added

1. Stable `Mithril_API_V2` line with provider architecture (`sodium`, `c23`, `flint`).
2. Runtime rollout controls for consumer deployment:
   - `MITHRIL_V2_ROLLOUT_MODE=full|canary|rollback`
   - `MITHRIL_V2_CANARY_PERCENT`
   - `MITHRIL_V2_CANARY_KEY`
3. Crypto failure telemetry hooks and counters in C++ wrapper layer.
4. CI smoke coverage for migrated consumers and Sprint 8 ops checks.

### Changed

1. v2 path is the default consumer path.
2. Error model differentiates cryptographic verification failures from internal/provider failures.
3. Build/test automation aligned for cross-platform CI and FLINT-enabled variants.

### Deprecated

1. Legacy v1 API line (`Mithril_Criptography_API`) is now deprecated for new development.
2. v1 remains in limited maintenance mode for critical fixes only.

### Migration

1. Use `Mithril_API_V2` as the primary integration target.
2. Start migration from legacy calls using:
   - `/Users/kirtantegsingh/documents/mithril/Mithril_API_V2/docs/V1_TO_V2_FUNCTION_MAPPING.md`
   - `/Users/kirtantegsingh/documents/mithril/Mithril_API_V2/docs/V2_0_MIGRATION_NOTES.md`
