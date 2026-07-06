#!/usr/bin/env bash
set -euo pipefail

repo_root="$(git rev-parse --show-toplevel)"
cd "$repo_root"

heartbeat_file="${KEEPALIVE_FILE:-.github/actions-keepalive.json}"
min_days="${KEEPALIVE_MIN_DAYS:-45}"
force="${KEEPALIVE_FORCE:-false}"
workflow_list="${KEEPALIVE_WORKFLOWS:-mithril-v2-conformance-nightly.yml}"

if [[ ! "$min_days" =~ ^[0-9]+$ ]]; then
  echo "KEEPALIVE_MIN_DAYS must be a positive integer, got: $min_days" >&2
  exit 64
fi

json_escape() {
  local value="$1"
  value="${value//\\/\\\\}"
  value="${value//\"/\\\"}"
  value="${value//$'\n'/\\n}"
  printf '%s' "$value"
}

workflow_json_array() {
  local first="true"
  local workflow

  printf '['
  for workflow in ${workflow_list//,/ }; do
    [[ -n "$workflow" ]] || continue
    if [[ "$first" == "true" ]]; then
      first="false"
    else
      printf ', '
    fi
    printf '"%s"' "$(json_escape "$workflow")"
  done
  printf ']'
}

enable_scheduled_workflows() {
  if [[ -z "${GITHUB_TOKEN:-}" || -z "${GITHUB_REPOSITORY:-}" ]]; then
    echo "GitHub token or repository context not found; skipping workflow enable check."
    return 0
  fi

  local api_url="${GITHUB_API_URL:-https://api.github.com}"
  local workflow
  local workflow_file
  local response_file
  local status

  for workflow in ${workflow_list//,/ }; do
    [[ -n "$workflow" ]] || continue
    workflow_file="${workflow##*/}"
    response_file="$(mktemp)"
    status="$(
      curl -sS -o "$response_file" -w "%{http_code}" \
        -X PUT \
        -H "Accept: application/vnd.github+json" \
        -H "Authorization: Bearer ${GITHUB_TOKEN}" \
        -H "X-GitHub-Api-Version: 2022-11-28" \
        "${api_url}/repos/${GITHUB_REPOSITORY}/actions/workflows/${workflow_file}/enable"
    )"

    if [[ "$status" == "204" ]]; then
      echo "Workflow '${workflow_file}' is enabled."
    else
      echo "::warning::Could not enable workflow '${workflow_file}' (HTTP ${status})."
      sed 's/^/::warning::GitHub API response: /' "$response_file" || true
    fi

    rm -f "$response_file"
  done
}

create_heartbeat_commit() {
  local now_epoch
  local last_commit_epoch
  local age_days
  local timestamp
  local head_sha
  local workflow_array
  local branch

  now_epoch="$(date +%s)"
  last_commit_epoch="$(git log -1 --format=%ct)"
  age_days="$(((now_epoch - last_commit_epoch) / 86400))"

  if [[ "$force" != "true" && "$force" != "1" && "$age_days" -lt "$min_days" ]]; then
    echo "Last commit is ${age_days} day(s) old; no heartbeat needed before ${min_days} days."
    return 0
  fi

  timestamp="$(date -u +"%Y-%m-%dT%H:%M:%SZ")"
  head_sha="$(git rev-parse HEAD)"
  workflow_array="$(workflow_json_array)"

  mkdir -p "$(dirname "$heartbeat_file")"
  cat > "$heartbeat_file" <<JSON
{
  "updated_at": "${timestamp}",
  "reason": "Keep scheduled GitHub Actions active before GitHub's 60-day inactivity window.",
  "min_days_without_commit": ${min_days},
  "previous_head": "${head_sha}",
  "workflow_files": ${workflow_array}
}
JSON

  git config user.name "${GIT_AUTHOR_NAME:-github-actions[bot]}"
  git config user.email "${GIT_AUTHOR_EMAIL:-41898282+github-actions[bot]@users.noreply.github.com}"
  git add "$heartbeat_file"

  if git diff --cached --quiet; then
    echo "Heartbeat file is unchanged; nothing to commit."
    return 0
  fi

  git commit -m "chore(ci): refresh scheduled workflow activity"

  if [[ -n "${GITHUB_ACTIONS:-}" ]]; then
    branch="${KEEPALIVE_BRANCH:-${GITHUB_REF_NAME:-$(git branch --show-current)}}"
    if [[ -z "$branch" ]]; then
      echo "Could not determine branch to push heartbeat commit." >&2
      exit 65
    fi

    # Authenticate the remote explicitly using the token. This is more reliable than
    # relying solely on actions/checkout credentials, especially with branch protection
    # rules or when a PAT is provided for bypassing PR requirements.
    if [[ -n "${GITHUB_TOKEN:-}" && -n "${GITHUB_REPOSITORY:-}" ]]; then
      git remote set-url origin "https://x-access-token:${GITHUB_TOKEN}@github.com/${GITHUB_REPOSITORY}.git"
    fi

    if ! git push origin "HEAD:${branch}"; then
      echo "::error::Failed to push heartbeat commit to ${branch}."
      echo "Common cause: branch protection rules (e.g. 'Require a pull request before merging')."
      echo "Fix: create a fine-grained PAT with Contents: Read and write, store as ACTIONS_KEEPALIVE_PAT secret."
      echo "Or, in branch protection settings, allow the workflow / github-actions to bypass PR requirements."
      exit 1
    fi
  else
    echo "Heartbeat commit created locally. Push it when you are ready."
  fi
}

enable_scheduled_workflows
create_heartbeat_commit
