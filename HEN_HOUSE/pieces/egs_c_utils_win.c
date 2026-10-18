/*
###############################################################################
#
#  EGSnrc file locking functions for windows
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
#  Author:          Iwan Kawrakow, 2003
#
#  Contributors:    Ernesto Mainegra-Hing
#
###############################################################################
*/

#include "egs_c_utils.h"
#include "egs_c_locking.c"

#include <io.h>
#include <stdio.h>
#include <stdlib.h>

void egsSleep(const int *secs) {
  unsigned int msecs = *secs * 1000;
  _sleep(msecs);
}

#ifdef MAKE_WIN_DISTRIBUTION
void C_CONVENTION egs_sleep_(const int *secs) { egs_sleep(secs); }
void C_CONVENTION egs_sleep__(const int *secs) { egs_sleep(secs); }
void C_CONVENTION EGS_SLEEP(const int *secs) { egs_sleep(secs); }
void C_CONVENTION EGS_SLEEP_(const int *secs) { egs_sleep(secs); }
void C_CONVENTION EGS_SLEEP__(const int *secs) { egs_sleep(secs); }
#endif

void egsPerror(const char *str, int len) {
  (void)len;
  perror(str);
}

#ifdef MAKE_WIN_DISTRIBUTION
void C_CONVENTION egs_perror_(const char *str, int len) { perror(str); }
void C_CONVENTION egs_perror__(const char *str, int len) { perror(str); }
void C_CONVENTION EGS_PERROR(const char *str, int len) { perror(str); }
void C_CONVENTION EGS_PERROR_(const char *str, int len) { perror(str); }
void C_CONVENTION EGS_PERROR__(const char *str, int len) { perror(str); }
#endif

void egsRemoveFile(const char *fname, int *status, int len) {
  (void)len;
  *status = _unlink(fname);
}

#ifdef MAKE_WIN_DISTRIBUTION
void C_CONVENTION egs_remove_file_(const char *fname, int *status, int len) {
  egs_remove_file(fname, status, len);
}
void C_CONVENTION egs_remove_file__(const char *fname, int *status, int len) {
  egs_remove_file(fname, status, len);
}
void C_CONVENTION EGS_REMOVE_FILE(const char *fname, int *status, int len) {
  egs_remove_file(fname, status, len);
}
void C_CONVENTION EGS_REMOVE_FILE_(const char *fname, int *status, int len) {
  egs_remove_file(fname, status, len);
}
void C_CONVENTION EGS_REMOVE_FILE__(const char *fname, int *status, int len) {
  egs_remove_file(fname, status, len);
}
#endif