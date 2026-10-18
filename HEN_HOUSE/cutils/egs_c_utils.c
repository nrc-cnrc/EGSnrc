/*
###############################################################################
#
#  EGSnrc C utility functions
#  Copyright (C) 2015 National Research Council Canada
#
#  This file is part of EGSnrc.
#
#  EGSnrc is free software: you can redistribute it and/or modify it under
#  the terms of the GNU Affero General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  EGSnrc is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#  FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
#  more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with EGSnrc. If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################
#
#  Author:          Iwan Kawrakow, 2004
#
#  Contributors:    Cody Crewson
#
###############################################################################
#
#  Control-file APIs are implemented once in HEN_HOUSE/pieces/egs_c_locking.c.
#  Build paths copy that source beside this file before compilation.
#
###############################################################################
*/

#if !defined(EGS_LOCK_WAIT_BLOCKING) && !defined(EGS_LOCK_WAIT_UNBLOCKING)
#define EGS_LOCK_WAIT_BLOCKING
#endif

#if !defined(WIN32) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include "egs_c_utils.h"
#include "egs_c_locking.c"

#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#include <io.h>

void egsSleep(const int *secs) {
    Sleep((DWORD)(*secs * 1000));
}

void egsPerror(const char *str, int len) {
    (void)len;
    perror(str);
}

void egsRemoveFile(const char *fname, int *status, int len) {
    (void)len;
    *status = _unlink(fname);
}

#else
#include <unistd.h>

void egsSleep(const int *secs) {
    sleep(*secs);
}

void egsFtoString(const int *size, int *n, char *str, void *a, int len) {
    (void)len;
    if (*size == 4) {
        *n = sprintf(str, "%g", *(float *)a);
    }
    else if (*size == 8) {
        *n = sprintf(str, "%lg", *(double *)a);
    }
    else {
        *n = 0;
    }
}

void egsPerror(const char *str, int len) {
    (void)len;
    perror(str);
}

void egsRemoveFile(const char *fname, int *status, int len) {
    (void)len;
    *status = unlink(fname);
}
#endif