#!/usr/bin/env bash
# Run glib geometry-alias regression tests via egs_app.
#
# Environment (optional unless noted):
#   HEN_HOUSE    — path to HEN_HOUSE (default: inferred from this script’s location)
#   EGS_HOME     — user application tree (default: dirname(HEN_HOUSE)/egs_home if present)
#   EGS_CONFIG   — e.g. $HEN_HOUSE/specs/<machine>.conf; used to read my_machine if set
#   EGS_MACHINE  — config/machine name (default: from EGS_CONFIG, else glib-ausgab)
#   APP_BIN      — override path to egs_app (default: $EGS_HOME/bin/$EGS_MACHINE/egs_app)
#   LIB_DIR      — DSO directory for DYLD_LIBRARY_PATH (default:
#                  $HEN_HOUSE/egs++/dso/$EGS_MACHINE)
#
# egs_app always opens inputs from $EGS_HOME/egs_app/<name>.egsinp. This script
# symlinks the glib_alias example .egsinp and .geom files from this directory
# into that folder so the tests do not need manual copies.

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Infer HEN_HOUSE when unset: this file lives under .../HEN_HOUSE/egs++/geometry/examples
if [[ -z "${HEN_HOUSE:-}" ]]; then
  _hh="$(cd "${ROOT_DIR}/../../.." && pwd)"
  if [[ "$(basename "${_hh}")" == "HEN_HOUSE" ]] || [[ -d "${_hh}/egs++/dso" ]]; then
    HEN_HOUSE="${_hh}"
  fi
fi
if [[ -z "${HEN_HOUSE:-}" ]]; then
  echo "error: set HEN_HOUSE or install this script under HEN_HOUSE/egs++/geometry/examples" >&2
  exit 1
fi
HEN_HOUSE="${HEN_HOUSE%/}"

# Machine name: explicit env, or parse from EGS_CONFIG (my_machine = ...), or default
EGS_MACHINE="${EGS_MACHINE:-}"
if [[ -z "${EGS_MACHINE}" && -n "${EGS_CONFIG:-}" && -f "${EGS_CONFIG}" ]]; then
  EGS_MACHINE="$(grep -m1 '^[[:space:]]*my_machine[[:space:]]*=' "${EGS_CONFIG}" | sed -e 's/^[^=]*=[[:space:]]*//' -e 's/[[:space:]]*$//' -e "s/^['\"]//" -e "s/['\"]$//")"
fi
EGS_MACHINE="${EGS_MACHINE:-glib-ausgab}"

# EGS_HOME: sibling of HEN_HOUSE by convention
if [[ -z "${EGS_HOME:-}" ]]; then
  _parent="$(dirname "${HEN_HOUSE}")"
  if [[ -d "${_parent}/egs_home" ]]; then
    EGS_HOME="${_parent}/egs_home"
  fi
fi

LIB_DIR="${LIB_DIR:-${HEN_HOUSE}/egs++/dso/${EGS_MACHINE}}"
APP_BIN="${APP_BIN:-${EGS_HOME:-}/bin/${EGS_MACHINE}/egs_app}"

if [[ ! -x "${APP_BIN}" ]]; then
  echo "error: egs_app not found or not executable: ${APP_BIN}" >&2
  echo "Set APP_BIN, EGS_HOME, and/or EGS_MACHINE (or EGS_CONFIG so my_machine can be read)." >&2
  exit 1
fi

if [[ -z "${EGS_HOME:-}" || ! -d "${EGS_HOME}/egs_app" ]]; then
  echo "error: EGS_HOME must be set and contain egs_app/ (egs_app loads -i from there only)." >&2
  exit 1
fi

# egs_app resolves -i as $EGS_HOME/egs_app/<input>.egsinp (not cwd)
link_glib_alias_inputs() {
  local f base
  shopt -s nullglob
  for f in "${ROOT_DIR}"/glib_alias_*.egsinp "${ROOT_DIR}"/glib_alias_*.geom; do
    base="$(basename "${f}")"
    ln -sf "${f}" "${EGS_HOME}/egs_app/${base}"
  done
  shopt -u nullglob
}
link_glib_alias_inputs

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
run_expect_success "glib_alias_pos_both"
run_expect_success "glib_alias_labels_pos"
# Unknown label currently falls back to all regions; keep as a passing regression probe.
run_expect_success "glib_alias_labels_neg_unknown_label"
run_expect_fail "glib_alias_neg_missing_geom" "does not name an existing geometry"
run_expect_fail "glib_alias_neg_collision_internal" "failed to register alias"

echo "All glib alias tests completed successfully."
