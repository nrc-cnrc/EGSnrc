#!/bin/sh
###############################################################################
#  Smoke tests for XDG EGSnrc profile layout (issue #1211)
###############################################################################

set -e
_my_dir=$(cd "$(dirname "$0")" && pwd)
. "$_my_dir/../egsnrc_config_paths"

_fail=0
_ok() { echo "OK: $*"; }
_bad() { echo "FAIL: $*"; _fail=1; }

_tmp=$(mktemp -d)
trap 'rm -rf "$_tmp"' EXIT INT TERM

export XDG_CONFIG_HOME="$_tmp/config"
export HOME="$_tmp"
_fake_hh="$_tmp/egsnrc/HEN_HOUSE"
mkdir -p "$_fake_hh/scripts/xdg" "$_fake_hh/specs"
cp "$_my_dir/../egsnrc_config_paths" "$_fake_hh/scripts/"
cp "$_my_dir/../egsnrc" "$_fake_hh/scripts/"
cp "$_my_dir/../egsnrc_bashrc_additions" "$_fake_hh/scripts/"
chmod +x "$_fake_hh/scripts/egsnrc"
cp "$_my_dir/../xdg/EGSnrc.bash" "$_fake_hh/scripts/xdg/"
touch "$_fake_hh/specs/unix.spec"

egsnrc_ensure_profile_layout "testprof" "$_fake_hh"
_specs=$(egsnrc_user_specs_dir "testprof")
printf 'my_machine = test\nHEN_HOUSE = %s/\n' "$_fake_hh" > "$_specs/test.conf"
egsnrc_write_profile_env "testprof" "test" "$_tmp/egs_home"
egsnrc_set_active_profile "testprof"
egsnrc_install_xdg_shell_files "$_fake_hh"

_cfg=$(egsnrc_config_home)
test -f "$_cfg/EGSnrc.bash" && _ok 'EGSnrc.bash installed' || _bad 'EGSnrc.bash missing'
test -f "$_cfg/profiles/testprof/env" && _ok 'profile env written' || _bad 'profile env missing'
test -f "$_cfg/profiles/testprof/specs/test.conf" && _ok 'spec copied' || _bad 'spec missing'

_out=$(XDG_CONFIG_HOME="$_tmp/config" HOME="$_tmp" "$_fake_hh/scripts/egsnrc" list)
echo "$_out" | grep -q testprof && _ok 'egsnrc list shows profile' || _bad "egsnrc list: $_out"

# Second profile (parallel install simulation)
_fake2="$_tmp/egsnrc2/HEN_HOUSE"
mkdir -p "$_fake2/scripts" "$_fake2/specs"
cp "$_fake_hh/scripts/egsnrc" "$_fake2/scripts/"
cp "$_fake_hh/scripts/egsnrc_config_paths" "$_fake2/scripts/"
_specs2=$(egsnrc_user_specs_dir "other")
mkdir -p "$_specs2"
printf 'my_machine = other\nHEN_HOUSE = %s/\n' "$_fake2" > "$_specs2/other.conf"
egsnrc_write_profile_env "other" "other" "$_tmp/egs_home2"
egsnrc_ensure_profile_layout "other" "$_fake2"

_out2=$(XDG_CONFIG_HOME="$_tmp/config" HOME="$_tmp" "$_fake_hh/scripts/egsnrc" use other)
echo "$_out2" | grep -q 'Switched active profile' && _ok 'egsnrc use other' || _bad "egsnrc use: $_out2"

_active=$(cat "$_cfg/active_profile" | tr -d '[:space:]')
test "$_active" = other && _ok 'active_profile updated' || _bad "active_profile is $_active"

# sync makefiles: install stale Makefile in fake EGS_HOME, sync from HEN_HOUSE
_uc_sync="$_tmp/egs_home_sync"
mkdir -p "$_uc_sync/egs_app" "$_fake_hh/user_codes/egs_app"
printf 'include $(SPEC_DIR)egspp_\n' > "$_uc_sync/egs_app/Makefile"
printf 'include $(USER_SPEC_DIR)egspp_\n' > "$_fake_hh/user_codes/egs_app/Makefile"
egsnrc_write_profile_env "testprof" "test" "$_uc_sync"
_out3=$(XDG_CONFIG_HOME="$_tmp/config" HOME="$_tmp" "$_fake_hh/scripts/egsnrc" sync --profile testprof makefiles egs_app 2>&1)
echo "$_out3" | grep -q 'synced Makefile: egs_app' && _ok 'egsnrc sync makefiles' || _bad "egsnrc sync: $_out3"
grep -q 'USER_SPEC_DIR' "$_uc_sync/egs_app/Makefile" && _ok 'sync updated Makefile content' || _bad 'sync makefile content wrong'

# config use --apply updates active_conf and env
egsnrc_set_active_profile "testprof"
printf 'my_machine = test2\nHEN_HOUSE = %s/\n' "$_fake_hh" > "$_specs/test2.conf"
_out4=$(XDG_CONFIG_HOME="$_tmp/config" HOME="$_tmp" "$_fake_hh/scripts/egsnrc" config use test2 --apply 2>&1)
echo "$_out4" | grep -q 'EGS_CONFIG' && _ok 'config use --apply' || _bad "config use --apply: $_out4"
_active_conf=$(tr -d '[:space:]' < "$_cfg/profiles/testprof/active_conf")
test "$_active_conf" = test2 && _ok 'active_conf after config use' || _bad "active_conf is $_active_conf"

# profile delete: remove non-active profile, refuse active without --force
_out5=$(XDG_CONFIG_HOME="$_tmp/config" HOME="$_tmp" "$_fake_hh/scripts/egsnrc" profile delete other 2>&1)
echo "$_out5" | grep -q 'Deleted profile' && _ok 'profile delete other' || _bad "profile delete: $_out5"
if XDG_CONFIG_HOME="$_tmp/config" HOME="$_tmp" "$_fake_hh/scripts/egsnrc" profile delete testprof 2>&1 | grep -q 'refusing'; then
    _ok 'profile delete refuses active'
else
    _bad 'profile delete should refuse active profile'
fi
_out6=$(XDG_CONFIG_HOME="$_tmp/config" HOME="$_tmp" "$_fake_hh/scripts/egsnrc" profile delete testprof --force 2>&1)
echo "$_out6" | grep -q 'Deleted profile' && _ok 'profile delete --force' || _bad "profile delete --force: $_out6"
test ! -d "$_cfg/profiles/testprof" && _ok 'profile dir removed' || _bad 'profile dir still exists'

if test $_fail -eq 0; then
    echo ""
    echo "All XDG config tests passed."
    exit 0
fi
echo ""
echo "Some tests failed."
exit 1
