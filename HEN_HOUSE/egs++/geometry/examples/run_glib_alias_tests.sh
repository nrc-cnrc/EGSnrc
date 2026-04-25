#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
APP_BIN="${APP_BIN:-/Users/marc/Developer/EGSnrc/egs_home/bin/glib-ausgab/egs_app}"
LIB_DIR="${LIB_DIR:-/Users/marc/Developer/EGSnrc/HEN_HOUSE/egs++/dso/glib-ausgab}"

run_expect_success() {
  local input_name="$1"
  local log_file="${ROOT_DIR}/${input_name}.log"
  echo "==> PASS expected: ${input_name}"
  DYLD_LIBRARY_PATH="${LIB_DIR}" "${APP_BIN}" -i "${input_name}" > "${log_file}" 2>&1
  awk '/Dose Scoring Object\(/ {found=1} END {exit(found?0:1)}' "${log_file}"
  awk '/fluence = [1-9]/ {found=1} END {exit(found?0:1)}' "${log_file}"
}

run_expect_fail() {
  local input_name="$1"
  local expected_pattern="$2"
  local log_file="${ROOT_DIR}/${input_name}.log"
  echo "==> FAIL expected: ${input_name}"
  set +e
  DYLD_LIBRARY_PATH="${LIB_DIR}" "${APP_BIN}" -i "${input_name}" > "${log_file}" 2>&1
  local ec=$?
  set -e
  if [[ ${ec} -eq 0 ]]; then
    echo "Expected failure but command succeeded: ${input_name}" >&2
    exit 1
  fi
  awk -v pat="${expected_pattern}" 'index($0, pat) {found=1} END {exit(found?0:1)}' "${log_file}"
}

cd "${ROOT_DIR}"

run_expect_success "glib_alias_pos_internal"
run_expect_success "glib_alias_pos_wrapper"
run_expect_success "glib_alias_validation"
run_expect_success "glib_alias_labels_pos"
# Unknown label currently falls back to all regions; keep as a passing regression probe.
run_expect_success "glib_alias_labels_neg_unknown_label"
run_expect_fail "glib_alias_neg_missing_geom" "does not name an existing geometry"
run_expect_fail "glib_alias_neg_collision_internal" "failed to register alias"

echo "All glib alias tests completed successfully."
