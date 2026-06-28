# EGSnrc XDG fish entry (installed to ~/.config/EGSnrc/EGSnrc.fish)

set -l _egsnrc_cfg_home $XDG_CONFIG_HOME/EGSnrc
if set -q EGSNRC_CONFIG_HOME
    set _egsnrc_cfg_home $EGSNRC_CONFIG_HOME
else if not set -q XDG_CONFIG_HOME
    set _egsnrc_cfg_home $HOME/.config/EGSnrc
end

if not test -d $_egsnrc_cfg_home
    echo "No EGSnrc configuration found in $_egsnrc_cfg_home" >&2
    return 1
end

set -l _egsnrc_profile default
if test -f $_egsnrc_cfg_home/active_profile
    set _egsnrc_profile (string trim (cat $_egsnrc_cfg_home/active_profile))
end

set -l _egsnrc_profdir $_egsnrc_cfg_home/profiles/$_egsnrc_profile
if not test -d $_egsnrc_profdir
    echo "EGSnrc profile '$_egsnrc_profile' not found." >&2
    return 1
end

set -l _egsnrc_hh ""
if test -f $_egsnrc_profdir/hen_house
    set _egsnrc_hh (string trim (cat $_egsnrc_profdir/hen_house))
end

if test -f $_egsnrc_profdir/env
    source $_egsnrc_profdir/env
end

set -l _egsnrc_active_conf ""
if test -f $_egsnrc_profdir/active_conf
    set _egsnrc_active_conf (string trim (cat $_egsnrc_profdir/active_conf))
end

if test -n "$_egsnrc_active_conf"
    set -gx EGS_CONFIG $_egsnrc_profdir/specs/$_egsnrc_active_conf.conf
end

if test -z "$_egsnrc_hh"; and test -f $EGS_CONFIG
    set _egsnrc_hh (grep 'HEN_HOUSE =' $EGS_CONFIG | sed 's/HEN_HOUSE = //' | string trim)
end

if not test -f $_egsnrc_hh/scripts/egsnrc_fishrc_additions
    echo "Could not locate HEN_HOUSE for profile '$_egsnrc_profile'." >&2
    return 1
end

source $_egsnrc_hh/scripts/egsnrc_fishrc_additions
