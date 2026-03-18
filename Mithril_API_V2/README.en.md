# 🚀 Mithril API v2 (C23) — Complete English Guide

> A modular, scalable cryptographic API for macOS Sequoia, designed to evolve in phases with a provider-based architecture (`sodium`, `c23`, `flint`).

---

## 🧭 Table of Contents

1. [What is Mithril API v2?](#-what-is-mithril-api-v2)
2. [Current project status](#-current-project-status)
3. [Architecture (quick view)](#-architecture-quick-view)
4. [Requirements](#-requirements)
5. [Tutorial 1: Build + test with CMake (recommended)](#-tutorial-1-build--test-with-cmake-recommended)
6. [Tutorial 2: Quick script with libsodium](#-tutorial-2-quick-script-with-libsodium)
7. [Tutorial 3: Xcode integration (Sequoia)](#-tutorial-3-xcode-integration-sequoia)
8. [How the provider system works](#-how-the-provider-system-works)
9. [Public API currently available](#-public-api-currently-available)
10. [Current tests](#-current-tests)
11. [Troubleshooting (common errors)](#-troubleshooting-common-errors)
12. [Suggested roadmap](#-suggested-roadmap)
13. [Security best practices](#-security-best-practices)
14. [v1 -> v2 Migration](#-v1---v2-migration)

---

## 🔐 What is Mithril API v2?

`Mithril_API_V2` is the next-generation cryptographic layer built in **C23**, with these goals:

- ✅ Stable, predictable API for C/C++ clients
- ✅ Decoupled backend architecture via providers
- ✅ Easy scaling to new algorithms and libraries
- ✅ Reproducible builds on macOS Sequoia
- ✅ Strong foundation for CI and security hardening

---

## 📌 Current project status

Implemented status (phases 1 → 10):

- ✅ Stable line declared: `v2.0.0`
- ✅ Core API/context/config/capabilities
- ✅ Active-provider dispatch layer
- ✅ Memory utilities and constant-time helpers
- ✅ `sodium` provider for RNG/HASH/AEAD/KEX/SIGN (strict mode)
- ✅ `c23` provider for `bigint/modarith` (arbitrary length)
- ✅ `flint` provider implemented with real bigint/modarith backend
- ✅ Unit tests for `c23` bigint/modarith
- ✅ Working CMake + ctest pipeline
- ✅ macOS build scripts

Formal release and migration notes:

- `/Users/kirtantegsingh/documents/mithril/Mithril_API_V2/docs/V2_0_MIGRATION_NOTES.md`
- `/Users/kirtantegsingh/documents/mithril/CHANGELOG.md`

---

## 🧱 Architecture (quick view)

```text
include/mithril/*.h       -> Public API contracts
src/api/*.c               -> API entry layer
src/core/*.c              -> Registry/dispatch/validation
src/providers/provider_*  -> Backend implementations
src/platform/apple/*.c    -> Apple platform utilities (Sequoia)
tests/unit/*.c            -> Unit tests
```

Typical execution flow:

1. `mithril_init()`
2. provider registration
3. provider activation
4. API call (e.g. hash/sign/modarith)
5. dispatch to active provider

---

## 🛠 Requirements

### macOS

- macOS Sequoia (or recent compatible)
- `clang` / `cc`
- `cmake >= 3.20`
- `pkg-config`
- `libsodium`

### Quick dependency install (Homebrew)

```zsh
brew install cmake pkg-config libsodium
```

Verification:

```zsh
pkg-config --modversion libsodium
cmake --version
cc --version
```

---

## 🧪 Tutorial 1: Build + test with CMake (recommended)

> This is the official day-to-day workflow.

### Step 1: Go to project root

```zsh
cd /Users/kirtantegsingh/documents/mithril/Mithril_API_V2
```

### Step 2: Configure

```zsh
cmake -S . -B build-cmake \
  -DCMAKE_BUILD_TYPE=Debug \
  -DMITHRIL_ENABLE_TESTS=ON \
  -DMITHRIL_USE_LIBSODIUM=ON \
  -DMITHRIL_ENABLE_PROVIDER_FLINT=OFF
```

### Step 3: Build

```zsh
cmake --build build-cmake --parallel
```

### Step 4: Run tests

```zsh
ctest --test-dir build-cmake --output-on-failure
```

Expected output:

- `c23_bigint` ✅
- `c23_modarith` ✅

---

## ⚡ Tutorial 2: Quick script with libsodium

For a one-shot run:

```zsh
/Users/kirtantegsingh/documents/mithril/Mithril_API_V2/scripts/build_and_run_phase82_with_sodium.zsh
```

This script will:

1. build test binaries
2. enable `MITHRIL_USE_LIBSODIUM`
3. link `Security` framework on Apple
4. run tests

---

## 🍎 Tutorial 3: Xcode integration (Sequoia)

Available configs:

- `/Users/kirtantegsingh/documents/mithril/Mithril_API_V2/xcode/xcconfig/Base.xcconfig`
- `/Users/kirtantegsingh/documents/mithril/Mithril_API_V2/xcode/xcconfig/Debug.xcconfig`
- `/Users/kirtantegsingh/documents/mithril/Mithril_API_V2/xcode/xcconfig/Release.xcconfig`
- `/Users/kirtantegsingh/documents/mithril/Mithril_API_V2/xcode/xcconfig/Warnings.xcconfig`
- `/Users/kirtantegsingh/documents/mithril/Mithril_API_V2/xcode/xcconfig/Sanitizers.xcconfig`

### Step-by-step

1. Create/open your `.xcodeproj` or `.xcworkspace`.
2. Apply `Base.xcconfig` to your library target.
3. Reference Debug/Release xcconfig files accordingly.
4. Confirm `HEADER_SEARCH_PATHS` includes `Mithril_API_V2/include`.
5. Confirm `OTHER_LDFLAGS` includes `-lsodium`.
6. On macOS, link `Security.framework`.

---

## 🔌 How the provider system works

Default provider on startup:

- `sodium` (strict)

Additional providers:

- `c23` (pure C23 bigint/modarith)
- `flint` (optional registration via macro, implementation pending)

### Activate provider manually

```c
mithril_provider_activate(ctx, "c23");
```

### Query active provider

```c
const char *name = mithril_provider_active_name(ctx);
```

Why this matters:

- clean API/implementation separation
- simpler backend migrations
- easy cross-provider testing

---

## 📚 Public API currently available

Main headers:

- `mithril_api.h` (init/shutdown/context)
- `mithril_rng.h`
- `mithril_hash.h`
- `mithril_aead.h`
- `mithril_kex.h`
- `mithril_sign.h`
- `mithril_bigint.h`
- `mithril_modarith.h`
- `mithril_provider.h`
- `mithril_memory.h`
- `mithril_util.h`
- `mithril_platform.h`

---

## ✅ Current tests

Implemented tests:

- `tests/unit/test_c23_bigint.c`
- `tests/unit/test_c23_modarith.c`

Current coverage:

- bigint add/sub/mul (`c23`)
- modular add/mul with reduction (`c23`)
- invalid modulus edge case
- non-trivial input lengths

---

## 🔁 v1 -> v2 Migration

Official function-by-function mapping guide:

- `docs/V1_TO_V2_FUNCTION_MAPPING.md`

It includes:

- legacy function status (`implemented`, `pending`, `deferred`)
- recommended v2 replacements
- contract differences (endianness, error model, context/provider)
- progressive migration examples

---

## 🧯 Troubleshooting (common errors)

### 1) `MITHRIL_ERR_NOT_IMPLEMENTED` during `mithril_init`

Typical cause:

- build without `MITHRIL_USE_LIBSODIUM`
- missing libsodium link

Fix:

- enable `-DMITHRIL_USE_LIBSODIUM`
- verify `pkg-config --libs libsodium`

### 2) Link error with `SecRandomCopyBytes`

Cause:

- missing `Security.framework`

Fix:

- add `-framework Security` on macOS

### 3) Tests fail due to wrong provider

Cause:

- test running on a different active provider

Fix:

- force `mithril_provider_activate(ctx, "c23")` for bigint/modarith tests

### 4) `ld: library 'sodium' not found`

Cause:

- linker cannot resolve libsodium path

Fix:

- use `pkg-config` in CMake (`PkgConfig::LIBSODIUM`)
- verify Homebrew installation

---

## 🗺 Suggested roadmap

1. Implement real `flint` provider for high-performance bigint/modarith.
2. Add conformance tests (`c23` vs `flint`).
3. Add fuzzing around API boundaries.
4. Add reproducible benchmarks.
5. Define explicit ABI/API policy (`v2.x`).
6. Publish build artifacts for external integration.

---

## 🛡 Security best practices

- 🔒 Never ship unvalidated crypto fallbacks to production.
- 🔒 Keep `libsodium` updated.
- 🔒 Avoid logging secrets/keys.
- 🔒 Use secure memory clearing for sensitive buffers.
- 🔒 Enable sanitizers in Debug builds.
- 🔒 Keep dev and prod settings clearly separated.

---

## 🤝 Quick contribution flow

1. Create a branch `codex/<feature>`.
2. Implement focused, reviewable changes.
3. Run `ci/run_phase10_ci.zsh`.
4. Attach test evidence in your PR.
5. Review with security/regression priority.

---

## 🧩 Super-quick usage example

```c
#include <stdint.h>
#include "mithril/mithril_api.h"
#include "mithril/mithril_provider.h"
#include "mithril/mithril_modarith.h"

int main(void) {
    mithril_context *ctx = 0;
    if (mithril_init(&ctx, 0) != MITHRIL_OK) return 1;

    if (mithril_provider_activate(ctx, "c23") != MITHRIL_OK) return 2;

    uint8_t a[] = {0x12};
    uint8_t b[] = {0x0E};
    uint8_t m[] = {0x17};
    uint8_t out[8] = {0};
    size_t written = 0;

    if (mithril_modarith_mul_mod(ctx, a, 1, b, 1, m, 1, out, sizeof(out), &written) != MITHRIL_OK) return 3;

    mithril_shutdown(ctx);
    return 0;
}
```

---

## 🎯 Final note

`Mithril_API_V2` now has a practical base to evolve into a production-grade cryptographic API: modular, testable, and extensible.

If you continue right away, the highest-impact next step is: **conformance tests + real `provider_flint` implementation**. 🔥

---

## 📚 Sprint 1 FLINT Technical Document

Detailed implementation notes, usage examples, and industry-facing rationale:

- `docs/SPRINT1_FLINT_IMPLEMENTACION_Y_ENERGIA_LIMPIA.md`
