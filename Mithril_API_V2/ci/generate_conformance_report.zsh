#!/bin/zsh
set -euo pipefail

if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <root-dir> <build-dir>" >&2
  exit 2
fi

ROOT="$1"
BUILD_DIR="$2"
TEMPLATE_FILE="${ROOT}/docs/CONFORMANCE_REPORT_TEMPLATE.md"
REPORT_FILE="${BUILD_DIR}/conformance_report.md"
VECTORS_BIN="${BUILD_DIR}/test_conformance_vectors"
RANDOM_BIN="${BUILD_DIR}/test_conformance_randomized"

if [ ! -f "${TEMPLATE_FILE}" ]; then
  echo "[ERROR] Missing template: ${TEMPLATE_FILE}" >&2
  exit 1
fi

if [ ! -x "${VECTORS_BIN}" ] || [ ! -x "${RANDOM_BIN}" ]; then
  echo "[ERROR] Missing conformance test binaries in ${BUILD_DIR}" >&2
  exit 1
fi

VECTORS_OUTPUT="$("${VECTORS_BIN}")"
RANDOM_OUTPUT="$("${RANDOM_BIN}")"

DATE_UTC="$(date -u '+%Y-%m-%dT%H:%M:%SZ')"
HOST_OS="$(uname -s)"
HOST_ARCH="$(uname -m)"
COMPILER_INFO="$(cc --version | head -n 1)"

if git -C "${ROOT}/.." rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  BRANCH="$(git -C "${ROOT}/.." branch --show-current)"
  COMMIT="$(git -C "${ROOT}/.." rev-parse --short HEAD)"
else
  BRANCH="unknown"
  COMMIT="unknown"
fi

SUMMARY_LINE="$(printf "%s\n" "${RANDOM_OUTPUT}" | sed -n 's/^\(\[CONFORMANCE\] summary.*\)$/\1/p')"
CASES_PER_OPERATION="$(printf "%s\n" "${SUMMARY_LINE}" | sed -n 's/.*cases_per_operation=\([0-9][0-9]*\).*/\1/p')"
OPERATIONS_COUNT="$(printf "%s\n" "${SUMMARY_LINE}" | sed -n 's/.*operations=\([0-9][0-9]*\).*/\1/p')"
SEEDS_COUNT="$(printf "%s\n" "${SUMMARY_LINE}" | sed -n 's/.*seeds=\([0-9][0-9]*\).*/\1/p')"
TOTAL_CASES="$(printf "%s\n" "${SUMMARY_LINE}" | sed -n 's/.*total_cases=\([0-9][0-9]*\).*/\1/p')"
TOTAL_DIVERGENCES="$(printf "%s\n" "${SUMMARY_LINE}" | sed -n 's/.*total_divergences=\([0-9][0-9]*\).*/\1/p')"

if [ -z "${CASES_PER_OPERATION}" ] || [ -z "${OPERATIONS_COUNT}" ] || [ -z "${SEEDS_COUNT}" ] || [ -z "${TOTAL_CASES}" ] || [ -z "${TOTAL_DIVERGENCES}" ]; then
  echo "[ERROR] Failed to parse conformance summary from randomized output." >&2
  exit 1
fi

TABLE_ROWS="$(
  printf "%s\n" "${RANDOM_OUTPUT}" | awk '
    /^\[CONFORMANCE\] op=/{
      op=""
      cases=""
      divergences=""
      for (i = 1; i <= NF; ++i) {
        if ($i ~ /^op=/) {
          op = $i
          sub(/^op=/, "", op)
        } else if ($i ~ /^cases=/) {
          cases = $i
          sub(/^cases=/, "", cases)
        } else if ($i ~ /^divergences=/) {
          divergences = $i
          sub(/^divergences=/, "", divergences)
        }
      }
      if (op != "" && cases != "" && divergences != "") {
        if (!(op in seen)) {
          order[++n] = op
          seen[op] = 1
        }
        case_sum[op] += cases + 0
        div_sum[op] += divergences + 0
      }
    }
    END{
      for (i = 1; i <= n; ++i) {
        op = order[i]
        printf("| `%s` | %d | %d |\n", op, case_sum[op], div_sum[op])
      }
    }
  '
)"

{
  cat "${TEMPLATE_FILE}"
  echo ""
  echo "## 8. Generated Run Data"
  echo ""
  echo "- Generated at (UTC): \`${DATE_UTC}\`"
  echo "- Branch: \`${BRANCH}\`"
  echo "- Commit: \`${COMMIT}\`"
  echo "- Host: \`${HOST_OS} ${HOST_ARCH}\`"
  echo "- Compiler: \`${COMPILER_INFO}\`"
  echo "- Build dir: \`${BUILD_DIR}\`"
  echo ""
  echo "### Conformance Summary"
  echo ""
  echo "- Cases per operation: \`${CASES_PER_OPERATION}\`"
  echo "- Operations: \`${OPERATIONS_COUNT}\`"
  echo "- Seeds: \`${SEEDS_COUNT}\`"
  echo "- Total cases: \`${TOTAL_CASES}\`"
  echo "- Total divergences: \`${TOTAL_DIVERGENCES}\`"
  echo ""
  echo "### Operation Table"
  echo ""
  echo "| Operation | Cases | Divergences |"
  echo "|---|---:|---:|"
  printf "%s\n" "${TABLE_ROWS}"
  echo "| **TOTAL** | ${TOTAL_CASES} | ${TOTAL_DIVERGENCES} |"
  echo ""
  echo "### Raw Output: conformance_vectors"
  echo ""
  echo '```text'
  printf "%s\n" "${VECTORS_OUTPUT}"
  echo '```'
  echo ""
  echo "### Raw Output: conformance_randomized"
  echo ""
  echo '```text'
  printf "%s\n" "${RANDOM_OUTPUT}"
  echo '```'
} > "${REPORT_FILE}"

echo "[CONFORMANCE] report generated: ${REPORT_FILE}"
