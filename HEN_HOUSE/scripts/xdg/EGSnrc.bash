###############################################################################
#
#  EGSnrc XDG Bourne-shell entry (installed to ~/.config/EGSnrc/EGSnrc.bash)
#
###############################################################################

_egsnrc_cfg_home="${XDG_CONFIG_HOME:-$HOME/.config}/EGSnrc"
if test -n "$EGSNRC_CONFIG_HOME"; then
    _egsnrc_cfg_home="$EGSNRC_CONFIG_HOME"
fi

if ! test -d "$_egsnrc_cfg_home"; then
    echo "No EGSnrc configuration found in $_egsnrc_cfg_home" >&2
    echo "Run HEN_HOUSE/scripts/configure or egsnrc_migrate_config." >&2
    return 1 2>/dev/null || exit 1
fi

_egsnrc_profile="default"
if test -f "$_egsnrc_cfg_home/active_profile"; then
    _egsnrc_profile=$(tr -d '[:space:]' < "$_egsnrc_cfg_home/active_profile")
fi

_egsnrc_profdir="$_egsnrc_cfg_home/profiles/$_egsnrc_profile"
if ! test -d "$_egsnrc_profdir"; then
    echo "EGSnrc profile '$_egsnrc_profile' not found under $_egsnrc_cfg_home/profiles/" >&2
    return 1 2>/dev/null || exit 1
fi

_egsnrc_hh=""
if test -f "$_egsnrc_profdir/hen_house"; then
    _egsnrc_hh=$(tr -d '[:space:]' < "$_egsnrc_profdir/hen_house")
fi

if test -f "$_egsnrc_profdir/env"; then
    . "$_egsnrc_profdir/env"
fi

_egsnrc_active_conf=""
if test -f "$_egsnrc_profdir/active_conf"; then
    _egsnrc_active_conf=$(tr -d '[:space:]' < "$_egsnrc_profdir/active_conf")
fi
if test -z "$_egsnrc_active_conf" && test -d "$_egsnrc_profdir/specs"; then
    for _f in "$_egsnrc_profdir/specs"/*.conf; do
        case "$_f" in
            *egspp_*|*"$_egsnrc_profdir/specs"/*.conf) continue ;;
        esac
        if test -f "$_f"; then
            _egsnrc_active_conf=$(basename "$_f" .conf)
            break
        fi
    done
fi

if test -n "$_egsnrc_active_conf"; then
    EGS_CONFIG="$_egsnrc_profdir/specs/${_egsnrc_active_conf}.conf"
    export EGS_CONFIG
fi

if test -z "$_egsnrc_hh" && test -n "$EGS_CONFIG" && test -f "$EGS_CONFIG"; then
    _egsnrc_hh=$(grep 'HEN_HOUSE =' "$EGS_CONFIG" | sed 's/HEN_HOUSE = //' | tr -d '[:space:]')
fi

if test -z "$_egsnrc_hh" || ! test -f "$_egsnrc_hh/scripts/egsnrc_bashrc_additions"; then
    echo "Could not locate HEN_HOUSE for EGSnrc profile '$_egsnrc_profile'." >&2
    return 1 2>/dev/null || exit 1
fi

if test -z "$EGS_CONFIG" || ! test -f "$EGS_CONFIG"; then
    echo "EGSnrc configuration file not found for profile '$_egsnrc_profile'." >&2
    return 1 2>/dev/null || exit 1
fi

. "$_egsnrc_hh/scripts/egsnrc_bashrc_additions"
