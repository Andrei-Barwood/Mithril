#!/bin/zsh
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${(%):-%N}")" && pwd)"
ROOT="${MITHRIL_ROOT:-$(cd "${SCRIPT_DIR}/.." && pwd)}"
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

"${SCRIPT_DIR}/generate_conformance_report.zsh" "${ROOT}" "${BUILD_DIR}"

echo "Phase 10 CI run complete (FLINT mode). Report: ${BUILD_DIR}/conformance_report.md"
