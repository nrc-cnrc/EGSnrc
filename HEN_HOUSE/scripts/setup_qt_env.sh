#!/bin/sh
###############################################################################
#
#  EGSnrc script to set Qt build environment on macOS
#  Copyright (C) 2026 National Research Council Canada
#
#  This file is part of EGSnrc.
#
#  Sourced from egsnrc_bashrc_additions on Darwin systems. Sets QTDIR,
#  QMAKESPEC, EGSNRC_CXX, and EGSNRC_CC so Qt GUIs use the same GNU
#  compilers as the EGSnrc/egspp build.
#
#  Requires EGS_CONFIG, HEN_HOUSE, and my_machine to be set by the caller.
#
###############################################################################

setup_qt_read_conf_var() {
    conf_file=$1
    var_name=$2
    if test -f "$conf_file"; then
        grep "^${var_name} *=" "$conf_file" | sed "s/^${var_name} *= *//" | head -1
    fi
}

setup_qt_gcc_from_gxx() {
    echo "$1" | sed 's/g++/gcc/'
}

# Qt installation prefix
if test "x$QTDIR" = x; then
    if command -v brew >/dev/null 2>&1; then
        qt_prefix=$(brew --prefix qt@5 2>/dev/null)
        if test "x$qt_prefix" != x && test -d "$qt_prefix/bin"; then
            QTDIR=$qt_prefix
            export QTDIR
        fi
    fi
fi

# EGSnrc mkspec (GUI Makefiles also pass -spec explicitly)
export QMAKESPEC="${HEN_HOUSE}mkspecs/macx-g++-egsnrc"

# C++ compiler from egspp config (written by configure_c++)
if test "x$EGSNRC_CXX" = x; then
    egspp_conf="${HEN_HOUSE}specs/egspp_${my_machine}.conf"
    EGSNRC_CXX=$(setup_qt_read_conf_var "$egspp_conf" CXX)
    if test "x$EGSNRC_CXX" != x; then
        export EGSNRC_CXX
    fi
fi

# C compiler from main EGSnrc config; fallback: derive from EGSNRC_CXX
if test "x$EGSNRC_CC" = x; then
    EGSNRC_CC=$(setup_qt_read_conf_var "$EGS_CONFIG" CC)
    if test "x$EGSNRC_CC" = x && test "x$EGSNRC_CXX" != x; then
        EGSNRC_CC=$(setup_qt_gcc_from_gxx "$EGSNRC_CXX")
    fi
    if test "x$EGSNRC_CC" != x; then
        export EGSNRC_CC
    fi
fi
