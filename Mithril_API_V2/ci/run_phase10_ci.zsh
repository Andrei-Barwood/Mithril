#!/bin/zsh
set -euo pipefail

ROOT="/Users/kirtantegsingh/documents/mithril/Mithril_API_V2"
BUILD_DIR="${ROOT}/build-cmake-flint"

if ! pkg-config --exists flint; then
  echo "[ERROR] FLINT no está disponible (pkg-config flint)." >&2
  echo "Instala FLINT o ejecuta CI en un entorno con FLINT." >&2
  exit 1
fi

cmake -S "${ROOT}" -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DMITHRIL_ENABLE_TESTS=ON \
  -DMITHRIL_USE_LIBSODIUM=ON \
  -DMITHRIL_ENABLE_PROVIDER_FLINT=ON \
  -DMITHRIL_USE_FLINT=ON

cmake --build "${BUILD_DIR}" --parallel
ctest --test-dir "${BUILD_DIR}" --output-on-failure

echo "Phase 10 CI run complete (FLINT mode)"
