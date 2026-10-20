#!/usr/bin/env zsh
# ============================================================================
# Mithril - Automated Test Runner
# File: scripts/testing/run_all_tests.zsh
# ============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
BUILD_DIR="${REPO_ROOT}/build"

if [ ! -d "${BUILD_DIR}" ] || [ ! -f "${BUILD_DIR}/CTestTestfile.cmake" ]; then
    echo "[info] Configuring unified CMake build..."
    cmake -B "${BUILD_DIR}" -S "${REPO_ROOT}" -DMITHRIL_BUILD_TESTS=ON
    cmake --build "${BUILD_DIR}" --parallel
fi

echo "[test] Running CTest suite..."
ctest --test-dir "${BUILD_DIR}" --output-on-failure

echo "[test] Running Basic Examples Ops validations..."
"${BUILD_DIR}/examples/basic/test_rollout_toggles"
"${BUILD_DIR}/examples/basic/test_crypto_telemetry"

echo "[SUCCESS] All tests and validations PASSED!"
