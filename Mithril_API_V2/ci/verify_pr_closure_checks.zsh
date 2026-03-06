#!/bin/zsh
set -euo pipefail

OWNER="${OWNER:-Andrei-Barwood}"
REPO="${REPO:-Mithril}"
HEAD_OWNER="${HEAD_OWNER:-${OWNER}}"
HEAD_BRANCH="${HEAD_BRANCH:-codex/mithril-v2-sprints}"
BASE_BRANCH="${BASE_BRANCH:-main}"
CHECKLIST_TEXT="${CHECKLIST_TEXT:-Merge solo al cerrar Sprint 9}"
REQUIRED_CHECK_CONTEXT="${REQUIRED_CHECK_CONTEXT:-ci-required-gate}"

API="https://api.github.com/repos/${OWNER}/${REPO}"

require_cmd() {
  if ! command -v "$1" >/dev/null 2>&1; then
    echo "[ERROR] Missing required command: $1" >&2
    exit 2
  fi
}

require_cmd curl
require_cmd jq

AUTH_HEADERS=()
if [ -n "${GITHUB_TOKEN:-}" ]; then
  AUTH_HEADERS=( -H "Authorization: Bearer ${GITHUB_TOKEN}" )
fi

pr_list_json="$(curl -fsSL \
  "${AUTH_HEADERS[@]}" \
  -H 'Accept: application/vnd.github+json' \
  "${API}/pulls?state=open&head=${HEAD_OWNER}:${HEAD_BRANCH}")"

pr_count="$(printf '%s' "${pr_list_json}" | jq 'length')"
if [ "${pr_count}" -eq 0 ]; then
  echo "[FAIL] No open PR found for head ${HEAD_OWNER}:${HEAD_BRANCH}" >&2
  exit 1
fi

pr_json="$(printf '%s' "${pr_list_json}" | jq '.[0]')"
pr_number="$(printf '%s' "${pr_json}" | jq -r '.number')"
pr_url="$(printf '%s' "${pr_json}" | jq -r '.html_url')"
pr_draft="$(printf '%s' "${pr_json}" | jq -r '.draft')"
pr_base="$(printf '%s' "${pr_json}" | jq -r '.base.ref')"
pr_head="$(printf '%s' "${pr_json}" | jq -r '.head.ref')"

check1_status="FAIL"
if [ "${pr_base}" = "${BASE_BRANCH}" ] && [ "${pr_head}" = "${HEAD_BRANCH}" ]; then
  check1_status="PASS"
fi

has_checklist="$(
  printf '%s' "${pr_json}" | jq -r \
    --arg text "${CHECKLIST_TEXT}" \
    '((.body // "") | ascii_downcase) | contains(($text | ascii_downcase))'
)"
check2_status="FAIL"
if [ "${has_checklist}" = "true" ]; then
  check2_status="PASS"
fi

check3_status="FAIL"
check3_detail=""
protection_tmp="$(mktemp)"
trap 'rm -f "${protection_tmp}"' EXIT

http_code="$(curl -sS -o "${protection_tmp}" -w '%{http_code}' \
  "${AUTH_HEADERS[@]}" \
  -H 'Accept: application/vnd.github+json' \
  "${API}/branches/${BASE_BRANCH}/protection")"

if [ "${http_code}" = "200" ]; then
  require_reviews="$(jq -r '.required_pull_request_reviews != null' "${protection_tmp}")"
  approvals="$(jq -r '.required_pull_request_reviews.required_approving_review_count // 0' "${protection_tmp}")"
  require_checks="$(jq -r '.required_status_checks != null' "${protection_tmp}")"
  has_required_context="$(jq -r --arg ctx "${REQUIRED_CHECK_CONTEXT}" '(.required_status_checks.contexts // []) | index($ctx) != null' "${protection_tmp}")"

  if [ "${require_reviews}" = "true" ] && [ "${require_checks}" = "true" ] && [ "${approvals}" -ge 1 ] && [ "${has_required_context}" = "true" ]; then
    check3_status="PASS"
    check3_detail="PR required, status checks required, approvals=${approvals}, required_context=${REQUIRED_CHECK_CONTEXT}"
  else
    check3_status="FAIL"
    check3_detail="required_reviews=${require_reviews}, required_status_checks=${require_checks}, approvals=${approvals}, has_required_context=${has_required_context}, required_context=${REQUIRED_CHECK_CONTEXT}"
  fi
elif [ "${http_code}" = "401" ] || [ "${http_code}" = "403" ]; then
  check3_status="UNVERIFIED"
  check3_detail="GitHub auth required to read branch protection (set GITHUB_TOKEN)."
else
  api_message="$(jq -r '.message // "unknown error"' "${protection_tmp}")"
  check3_status="FAIL"
  check3_detail="HTTP ${http_code}: ${api_message}"
fi

echo "PR verification report"
echo "- Repository: ${OWNER}/${REPO}"
echo "- PR: #${pr_number} (${pr_url})"
echo "- Draft: ${pr_draft}"
echo ""
echo "[1] Base/head expected => ${check1_status}"
echo "    expected: base=${BASE_BRANCH}, head=${HEAD_BRANCH}"
echo "    actual:   base=${pr_base}, head=${pr_head}"
echo ""
echo "[2] Checklist text in description => ${check2_status}"
echo "    text: \"${CHECKLIST_TEXT}\""
echo ""
echo "[3] Branch protection on ${BASE_BRANCH} => ${check3_status}"
echo "    detail: ${check3_detail}"

overall=0
if [ "${check1_status}" != "PASS" ] || [ "${check2_status}" != "PASS" ]; then
  overall=1
fi
if [ "${check3_status}" = "FAIL" ]; then
  overall=1
elif [ "${check3_status}" = "UNVERIFIED" ]; then
  overall=2
fi

if [ "${overall}" -eq 0 ]; then
  echo ""
  echo "[OK] All PR closure checks passed."
elif [ "${overall}" -eq 2 ]; then
  echo ""
  echo "[WARN] PR checks partially verified. Branch protection could not be validated without auth."
else
  echo ""
  echo "[FAIL] One or more PR closure checks failed."
fi

exit "${overall}"
