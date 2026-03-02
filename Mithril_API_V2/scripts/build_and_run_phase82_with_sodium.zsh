#!/bin/zsh
set -euo pipefail

ROOT="/Users/kirtantegsingh/documents/mithril/Mithril_API_V2"
INC=("-I${ROOT}/include")
CFLAGS=("-std=c23" "-DMITHRIL_USE_LIBSODIUM")
APPLE_LIBS=("-framework" "Security")

if command -v pkg-config >/dev/null 2>&1 && pkg-config --exists libsodium; then
  SODIUM_CFLAGS=( ${(z)$(pkg-config --cflags libsodium)} )
  SODIUM_LIBS=( ${(z)$(pkg-config --libs libsodium)} )
else
  SODIUM_CFLAGS=()
  SODIUM_LIBS=("-lsodium")
fi

COMMON_SOURCES=(
  "${ROOT}/src/api/mithril_api.c"
  "${ROOT}/src/api/mithril_context.c"
  "${ROOT}/src/api/mithril_error.c"
  "${ROOT}/src/api/mithril_version.c"
  "${ROOT}/src/api/mithril_rng.c"
  "${ROOT}/src/api/mithril_hash.c"
  "${ROOT}/src/api/mithril_aead.c"
  "${ROOT}/src/api/mithril_kex.c"
  "${ROOT}/src/api/mithril_sign.c"
  "${ROOT}/src/api/mithril_bigint.c"
  "${ROOT}/src/api/mithril_modarith.c"
  "${ROOT}/src/core/core_config.c"
  "${ROOT}/src/core/core_capabilities.c"
  "${ROOT}/src/core/core_validation.c"
  "${ROOT}/src/core/core_registry.c"
  "${ROOT}/src/core/core_dispatch.c"
  "${ROOT}/src/core/core_memory.c"
  "${ROOT}/src/core/core_ct.c"
  "${ROOT}/src/core/core_util.c"
  "${ROOT}/src/providers/provider_sodium/provider_sodium_registry.c"
  "${ROOT}/src/providers/provider_sodium/provider_sodium_rng.c"
  "${ROOT}/src/providers/provider_sodium/provider_sodium_hash.c"
  "${ROOT}/src/providers/provider_sodium/provider_sodium_aead.c"
  "${ROOT}/src/providers/provider_sodium/provider_sodium_kex.c"
  "${ROOT}/src/providers/provider_sodium/provider_sodium_sign.c"
  "${ROOT}/src/providers/provider_c23/provider_c23_registry.c"
  "${ROOT}/src/providers/provider_c23/provider_c23_bigint.c"
  "${ROOT}/src/providers/provider_c23/provider_c23_modarith.c"
  "${ROOT}/src/providers/provider_flint/provider_flint_registry.c"
  "${ROOT}/src/providers/provider_flint/provider_flint_bigint.c"
  "${ROOT}/src/providers/provider_flint/provider_flint_modarith.c"
  "${ROOT}/src/platform/apple/apple_entropy.c"
  "${ROOT}/src/platform/apple/apple_clock.c"
  "${ROOT}/src/platform/apple/apple_memory.c"
)

cc "${CFLAGS[@]}" "${INC[@]}" "${SODIUM_CFLAGS[@]}" \
  "${ROOT}/tests/unit/test_c23_bigint.c" \
  "${COMMON_SOURCES[@]}" \
  "${SODIUM_LIBS[@]}" "${APPLE_LIBS[@]}" \
  -o /tmp/test_c23_bigint_phase82_sodium

cc "${CFLAGS[@]}" "${INC[@]}" "${SODIUM_CFLAGS[@]}" \
  "${ROOT}/tests/unit/test_c23_modarith.c" \
  "${COMMON_SOURCES[@]}" \
  "${SODIUM_LIBS[@]}" "${APPLE_LIBS[@]}" \
  -o /tmp/test_c23_modarith_phase82_sodium

/tmp/test_c23_bigint_phase82_sodium
/tmp/test_c23_modarith_phase82_sodium

echo "Phase 9 build+tests complete"
