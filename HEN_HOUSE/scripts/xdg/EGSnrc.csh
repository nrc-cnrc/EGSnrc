###############################################################################
#
#  EGSnrc XDG C-shell entry (installed to ~/.config/EGSnrc/EGSnrc.csh)
#
###############################################################################

set _egsnrc_cfg_home = "${XDG_CONFIG_HOME:-$HOME/.config}/EGSnrc"
if ( $?EGSNRC_CONFIG_HOME ) set _egsnrc_cfg_home = "$EGSNRC_CONFIG_HOME"

if ( ! -d "$_egsnrc_cfg_home" ) then
    echo "No EGSnrc configuration found in $_egsnrc_cfg_home" >&2
    exit 1
endif

set _egsnrc_profile = "default"
if ( -f "$_egsnrc_cfg_home/active_profile" ) set _egsnrc_profile = `cat "$_egsnrc_cfg_home/active_profile" | tr -d ' '`

set _egsnrc_profdir = "$_egsnrc_cfg_home/profiles/$_egsnrc_profile"
if ( ! -d "$_egsnrc_profdir" ) then
    echo "EGSnrc profile '$_egsnrc_profile' not found." >&2
    exit 1
endif

set _egsnrc_hh = ""
if ( -f "$_egsnrc_profdir/hen_house" ) set _egsnrc_hh = `cat "$_egsnrc_profdir/hen_house" | tr -d ' '`

if ( -f "$_egsnrc_profdir/env" ) source "$_egsnrc_profdir/env"

set _egsnrc_active_conf = ""
if ( -f "$_egsnrc_profdir/active_conf" ) set _egsnrc_active_conf = `cat "$_egsnrc_profdir/active_conf" | tr -d ' '`

if ( "$_egsnrc_active_conf" != "" ) then
    setenv EGS_CONFIG "$_egsnrc_profdir/specs/${_egsnrc_active_conf}.conf"
endif

if ( "$_egsnrc_hh" == "" && -f "$EGS_CONFIG" ) then
    set _egsnrc_hh = `grep 'HEN_HOUSE =' "$EGS_CONFIG" | sed 's/HEN_HOUSE = //' | tr -d ' '`
endif

if ( ! -f "$_egsnrc_hh/scripts/egsnrc_cshrc_additions" ) then
    echo "Could not locate HEN_HOUSE for profile '$_egsnrc_profile'." >&2
    exit 1
endif

source "$_egsnrc_hh/scripts/egsnrc_cshrc_additions"
