#!/bin/zsh
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${(%):-%N}")" && pwd)"
ROOT="${MITHRIL_ROOT:-$(cd "${SCRIPT_DIR}/.." && pwd)}"
BUILD_DIR="${MITHRIL_BUILD_DIR:-${ROOT}/build-cmake-flint-nightly}"
CASES_PER_OPERATION="${MITHRIL_CONFORMANCE_CASES_PER_OPERATION:-10000}"

if ! [[ "${CASES_PER_OPERATION}" =~ '^[1-9][0-9]*$' ]]; then
  echo "[ERROR] MITHRIL_CONFORMANCE_CASES_PER_OPERATION must be a positive integer." >&2
  exit 2
fi

if ! pkg-config --exists flint; then
  echo "[ERROR] FLINT not found via pkg-config. Install FLINT to run extended conformance nightly." >&2
  exit 1
fi

cmake -S "${ROOT}" -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DMITHRIL_ENABLE_TESTS=ON \
  -DMITHRIL_USE_LIBSODIUM=ON \
  -DMITHRIL_ENABLE_PROVIDER_FLINT=ON \
  -DMITHRIL_USE_FLINT=ON \
  -DMITHRIL_CONFORMANCE_CASES_PER_OPERATION="${CASES_PER_OPERATION}"

cmake --build "${BUILD_DIR}" --parallel
ctest --test-dir "${BUILD_DIR}" -R "conformance_vectors|conformance_randomized" --output-on-failure

"${SCRIPT_DIR}/generate_conformance_report.zsh" "${ROOT}" "${BUILD_DIR}"

echo "Extended FLINT conformance nightly complete."
echo "- Cases per operation: ${CASES_PER_OPERATION}"
echo "- Report: ${BUILD_DIR}/conformance_report.md"
