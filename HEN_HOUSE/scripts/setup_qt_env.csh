#!/bin/csh
###############################################################################
#
#  EGSnrc csh script to set Qt build environment on macOS
#  Copyright (C) 2026 National Research Council Canada
#
#  This file is part of EGSnrc.
#
#  Sourced from egsnrc_cshrc_additions on Darwin systems.
#
###############################################################################

if ( ! $?QTDIR ) then
    if ( `which brew >& /dev/null` ) then
        set qt_prefix = `brew --prefix qt@5`
        if ( -d "$qt_prefix/bin" ) then
            setenv QTDIR "$qt_prefix"
        endif
    endif
endif

setenv QMAKESPEC "${HEN_HOUSE}mkspecs/macx-g++-egsnrc"

if ( ! $?EGSNRC_CXX ) then
    set egspp_conf = "${HEN_HOUSE}specs/egspp_${my_machine}.conf"
    if ( -f "$egspp_conf" ) then
        set EGSNRC_CXX = `grep '^CXX *=' "$egspp_conf" | sed 's/CXX *= *//' | head -1`
        if ( "$EGSNRC_CXX" != "" ) setenv EGSNRC_CXX "$EGSNRC_CXX"
    endif
endif

if ( ! $?EGSNRC_CC ) then
    if ( -f "$EGS_CONFIG" ) then
        set EGSNRC_CC = `grep '^CC *=' "$EGS_CONFIG" | sed 's/CC *= *//' | head -1`
    endif
    if ( "$EGSNRC_CC" == "" && $?EGSNRC_CXX ) then
        set EGSNRC_CC = `echo "$EGSNRC_CXX" | sed 's/g++/gcc/'`
    endif
    if ( "$EGSNRC_CC" != "" ) setenv EGSNRC_CC "$EGSNRC_CC"
endif
