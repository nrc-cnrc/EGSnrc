/*
###############################################################################
#
#  EGSnrc Fortran name decoration scheme checker
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
#  Author:          Ernesto Mainegra-Hing, 2003
#
#  Contributors:    Iwan Kawrakow
#
###############################################################################
#
#  It turns out we don't need 3 object files for windows, but only two: One
#  using the cdecl naming convention and another (for Windooze) using the
#  __stdcall naming convention. To this end, a macro C_CONVENTION was added
#  to each function in the C files.
#
###############################################################################
*/


#include <stdio.h>

#ifdef STDCALL
#define C_CONVENTION __stdcall
#else
#define C_CONVENTION
#endif

void C_CONVENTION junk() {
   printf("lower case, ");
}
void C_CONVENTION junk_() {
   printf("lower case, _ ");
}
void C_CONVENTION JUNK() {
   printf("upper case, ");
}
void C_CONVENTION JUNK_() {
   printf("upper case, _ ");
}
void C_CONVENTION junk_junk() {
   printf("no underscores\n");
}
void C_CONVENTION junk_junk_() {
   printf("and _\n");
}
void C_CONVENTION junk_junk__() {
    printf("and __\n");
}
void C_CONVENTION JUNK_JUNK() {
   printf("no underscores\n");
}
void C_CONVENTION JUNK_JUNK_() {
    printf("and _\n");
}
void C_CONVENTION JUNK_JUNK__() {
    printf("and __\n");
}


