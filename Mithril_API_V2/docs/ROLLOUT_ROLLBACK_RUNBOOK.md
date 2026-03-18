# Sprint 8 Ops Runbook: Rollout and Rollback

## Scope

This runbook defines the operational procedure for Mithril v2 consumer rollout with:

1. canary gating
2. crypto failure telemetry
3. explicit rollback switch

Applies to:

1. `examples/basic/secure_client`
2. `examples/basic/accepting_connection_sodium`
3. `network/src/boost_asio_compat.cpp`
4. `network/include/mithril/mithril_sodium.hpp`

## Rollout Toggles

### `MITHRIL_V2_ROLLOUT_MODE`

Values:

1. `full` (default): v2 allowed for all processes
2. `canary`: enable v2 only for selected cohort
3. `rollback`: block v2 startup immediately

### `MITHRIL_V2_CANARY_PERCENT`

Range: integer `0..100` (default `100`)

Used only when `MITHRIL_V2_ROLLOUT_MODE=canary`.

### `MITHRIL_V2_CANARY_KEY`

Deterministic cohort key. If not set, `HOSTNAME` is used. If `HOSTNAME` is absent, fallback key is `default`.

### `MITHRIL_CRYPTO_PATH`

Existing path selector from Sprint 7:

1. `v2`
2. `v1`
3. `legacy`

Current migrated consumers still enforce v2-only runtime path.

### `MITHRIL_CRYPTO_TELEMETRY_STDERR`

If set to `1`, `y`, or `t`, every crypto failure is emitted to stderr:

```text
[mithril-telemetry] op=<operation> status=<MITHRIL_ERR_*>
```

## Telemetry Metrics

Per-process counters available through `mithril::sodium::telemetry`:

1. `total_operations`
2. `total_failures`
3. `verification_failures`
4. `internal_failures`
5. `failure_rate = total_failures / total_operations`

Both basic client and server print a telemetry summary before process exit.

## Canary Procedure

Suggested initial internal canary window: 30 minutes.

1. Deploy canary instances with:
   - `MITHRIL_V2_ROLLOUT_MODE=canary`
   - `MITHRIL_V2_CANARY_PERCENT=10`
   - `MITHRIL_V2_CANARY_KEY=<node-or-pod-id>`
2. Keep stable fleet unchanged.
3. Observe logs:
   - startup line with `rollout: mode=canary ... decision=enabled|blocked`
   - telemetry summary line at process exit
4. Promotion criteria for next window:
   - `internal_failures = 0`
   - no unexpected startup block on intended canary nodes
   - integration smoke remains green in CI
5. Increase canary percentage gradually (for example 10 -> 25 -> 50 -> 100).

## Rollback Procedure

Rollback is fail-closed from runtime policy plus traffic shift to prior stable deployment.

1. Set `MITHRIL_V2_ROLLOUT_MODE=rollback` on canary deployment.
2. Confirm process exits with startup error due rollout policy.
3. Route traffic back to previous stable deployment group.
4. Keep rollback mode until incident triage is complete.
5. After remediation, re-enter canary mode from a low percentage.

## Operational Validation Commands

Run local ops tests:

```zsh
make -C /Users/kirtantegsingh/documents/mithril/examples/basic test-sprint8
```

Force rollback block:

```zsh
cd /Users/kirtantegsingh/documents/mithril/examples/basic
MITHRIL_V2_ROLLOUT_MODE=rollback ./secure_client localhost 3333
```

Expected: startup failure with rollout policy message.

Force canary exclusion:

```zsh
cd /Users/kirtantegsingh/documents/mithril/examples/basic
MITHRIL_V2_ROLLOUT_MODE=canary \
MITHRIL_V2_CANARY_PERCENT=0 \
MITHRIL_V2_CANARY_KEY=node-a \
./secure_client localhost 3333
```

Expected: startup failure with canary decision `blocked`.

## CI Coverage

`consumers-smoke` job executes:

1. migrated consumer build and integration test
2. Sprint 8 ops tests via `make test` (includes `test-sprint8`)

This guarantees rollout toggle parser and telemetry hook behavior remain regression-tested.

