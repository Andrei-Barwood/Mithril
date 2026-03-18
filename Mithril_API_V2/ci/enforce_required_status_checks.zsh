#!/bin/zsh
set -euo pipefail

OWNER="${OWNER:-Andrei-Barwood}"
REPO="${REPO:-Mithril}"
BRANCH="${BRANCH:-main}"
REQUIRED_CONTEXTS_CSV="${REQUIRED_CONTEXTS_CSV:-ci-required-gate}"
REQUIRED_APPROVALS="${REQUIRED_APPROVALS:-1}"
STRICT="${STRICT:-true}"

API="https://api.github.com/repos/${OWNER}/${REPO}"

require_cmd() {
  if ! command -v "$1" >/dev/null 2>&1; then
    echo "[ERROR] Missing required command: $1" >&2
    exit 2
  fi
}

require_cmd curl
require_cmd jq

if [ -z "${GITHUB_TOKEN:-}" ]; then
  echo "[ERROR] GITHUB_TOKEN is required." >&2
  exit 2
fi

if ! [[ "${REQUIRED_APPROVALS}" =~ '^[0-9]+$' ]]; then
  echo "[ERROR] REQUIRED_APPROVALS must be a non-negative integer." >&2
  exit 2
fi

context_json="$(
  printf '%s' "${REQUIRED_CONTEXTS_CSV}" | tr ',' '\n' | sed 's/^ *//; s/ *$//' | sed '/^$/d' | jq -R . | jq -s .
)"

if [ "$(printf '%s' "${context_json}" | jq 'length')" -eq 0 ]; then
  echo "[ERROR] REQUIRED_CONTEXTS_CSV resolved to an empty context list." >&2
  exit 2
fi

protection_tmp="$(mktemp)"
trap 'rm -f "${protection_tmp}"' EXIT

http_code="$(
  curl -sS -o "${protection_tmp}" -w '%{http_code}' \
    -H "Authorization: Bearer ${GITHUB_TOKEN}" \
    -H 'Accept: application/vnd.github+json' \
    "${API}/branches/${BRANCH}/protection"
)"

if [ "${http_code}" = "404" ]; then
  echo "[ERROR] Branch ${BRANCH} is not protected. Configure protection first, then rerun." >&2
  exit 1
fi

if [ "${http_code}" != "200" ]; then
  echo "[ERROR] Unable to read branch protection: HTTP ${http_code}" >&2
  jq -r '.message // "unknown error"' "${protection_tmp}" >&2 || true
  exit 1
fi

strict_bool="false"
if [ "${STRICT}" = "true" ]; then
  strict_bool="true"
fi

status_payload="$(
  jq -n \
    --argjson strict "${strict_bool}" \
    --argjson contexts "${context_json}" \
    '{strict: $strict, contexts: $contexts}'
)"

reviews_payload="$(
  jq -n \
    --argjson approvals "${REQUIRED_APPROVALS}" \
    '{dismiss_stale_reviews: false, require_code_owner_reviews: false, required_approving_review_count: $approvals}'
)"

curl -fsSL -X PATCH \
  -H "Authorization: Bearer ${GITHUB_TOKEN}" \
  -H 'Accept: application/vnd.github+json' \
  -H 'Content-Type: application/json' \
  "${API}/branches/${BRANCH}/protection/required_status_checks" \
  -d "${status_payload}" >/dev/null

curl -fsSL -X PATCH \
  -H "Authorization: Bearer ${GITHUB_TOKEN}" \
  -H 'Accept: application/vnd.github+json' \
  -H 'Content-Type: application/json' \
  "${API}/branches/${BRANCH}/protection/required_pull_request_reviews" \
  -d "${reviews_payload}" >/dev/null

verify_json="$(
  curl -fsSL \
    -H "Authorization: Bearer ${GITHUB_TOKEN}" \
    -H 'Accept: application/vnd.github+json' \
    "${API}/branches/${BRANCH}/protection"
)"

missing_contexts="$(
  jq -n \
    --argjson required "${context_json}" \
    --argjson actual "$(printf '%s' "${verify_json}" | jq '.required_status_checks.contexts // []')" \
    '$required - $actual'
)"

missing_count="$(printf '%s' "${missing_contexts}" | jq 'length')"
approvals_now="$(printf '%s' "${verify_json}" | jq -r '.required_pull_request_reviews.required_approving_review_count // 0')"

if [ "${missing_count}" -ne 0 ] || [ "${approvals_now}" -lt "${REQUIRED_APPROVALS}" ]; then
  echo "[ERROR] Required checks enforcement verification failed." >&2
  echo "Missing contexts: ${missing_contexts}" >&2
  echo "Approvals configured: ${approvals_now}" >&2
  exit 1
fi

echo "[OK] Required status checks enforced on ${OWNER}/${REPO}:${BRANCH}"
echo "- Contexts: ${REQUIRED_CONTEXTS_CSV}"
echo "- Required approvals: ${REQUIRED_APPROVALS}"
