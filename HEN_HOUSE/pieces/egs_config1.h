/*
###############################################################################
#
#  EGSnrc parallel processing C functions
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


#ifndef EGS_CONFIG1_
#define EGS_CONFIG1_

#ifdef WIN32
#include <windows.h>
#endif
/* Without the above include file, gcc on Windows does not know about
   __int64
 */

/*! Use the following macro for functions that don't have an underscore in
    in their name.
 */
__f77_function1__

/*! Use the following macro for functions with underscore(s) in their name
 */
__f77_function2__

/*! The name of the configuration
 */
#define CONFIG_NAME "__config_name__"

#ifdef __cplusplus
#define __extc__ "C"
#else
#define __extc__
#endif

#ifdef SINGLE
typedef float  EGS_Float;
#else
typedef double EGS_Float;
#endif

typedef short EGS_I16;
typedef int   EGS_I32;
#ifdef WIN32
typedef __int64 EGS_I64;
#else
typedef long long EGS_I64;
#endif

#endif
