/*
###############################################################################
#
#  EGSnrc egs_exit functions
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
#  Contributors:
#
###############################################################################
*/


#ifdef STDCALL
#define C_CONVENTION __stdcall
#else
#define C_CONVENTION
#endif

#ifdef __cplusplus
extern "C" {
#endif
void my_exit(int *status){
   int s = *status;
   exit(s);
}
void C_CONVENTION egs_exit(int *status)  {my_exit(status);}
void C_CONVENTION egs_exit_(int *status) {my_exit(status);}
void C_CONVENTION egs_exit__(int* status){my_exit(status);}
void C_CONVENTION EGS_EXIT(int *status)  {my_exit(status);}
void C_CONVENTION EGS_EXIT_(int *status) {my_exit(status);}
void C_CONVENTION EGS_EXIT__(int *status){my_exit(status);}
#ifdef __cplusplus
}
#endif
