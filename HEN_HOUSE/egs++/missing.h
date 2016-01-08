/*
###############################################################################
#
#  EGSnrc egs++ compiler incompatibility headers
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
#  Author:          Iwan Kawrakow, 2005
#
#  Contributors:
#
###############################################################################
*/


/*! \file missing.h
 *  \brief Fixes header incompatibilities between the PGI 4.0 compiler
 *         and newer Linux systems.
 *  \IK
 */

#ifndef MISSING_H_
#define MISSING_H_

#ifndef __CHAR_BIT__
    #define __CHAR_BIT__ 8
#endif

#ifndef __SCHAR_MAX__
    #define __SCHAR_MAX__ 127
#endif

#ifndef __SHRT_MAX__
    #define __SHRT_MAX__ 32767
#endif

#ifndef __INT_MAX__
    #define __INT_MAX__ 2147483647
#endif

#ifndef __LONG_MAX__
    #define __LONG_MAX__ 2147483647L
#endif

#ifndef __FLT_MANT_DIG__
    #define __FLT_MANT_DIG__ 24
#endif

#ifndef __FLT_DIG__
    #define __FLT_DIG__ 6
#endif

#ifndef __FLT_MIN_EXP__
    #define __FLT_MIN_EXP__ (-125)
#endif

#ifndef __FLT_MAX_EXP__
    #define __FLT_MAX_EXP__ 128
#endif

#ifndef __FLT_MIN_10_EXP__
    #define __FLT_MIN_10_EXP__ (-125)
#endif

#ifndef __FLT_MAX_10_EXP__
    #define __FLT_MAX_10_EXP__ 128
#endif

#ifndef __FLT_RADIX__
    #define __FLT_RADIX__ 2
#endif

#ifndef __FLT_MIN__
    #define __FLT_MIN__ 1.17549435e-38F
#endif

#ifndef __FLT_MAX__
    #define __FLT_MAX__ 3.40282347e+38F
#endif

#ifndef __FLT_RADIX__
    #define __FLT_RADIX__ 2
#endif

#ifndef __FLT_EPSILON__
    #define __FLT_EPSILON__ 1.19209290e-7F
#endif

#ifndef __DBL_MANT_DIG__
    #define __DBL_MANT_DIG__ 53
#endif

#ifndef __DBL_DIG__
    #define __DBL_DIG__ 15
#endif

#ifndef __DBL_MIN_EXP__
    #define __DBL_MIN_EXP__ (-1021)
#endif

#ifndef __DBL_MAX_EXP__
    #define __DBL_MAX_EXP__ 1024
#endif

#ifndef __DBL_MIN_10_EXP__
    #define __DBL_MIN_10_EXP__ (-1021)
#endif

#ifndef __DBL_MAX_10_EXP__
    #define __DBL_MAX_10_EXP__ 1024
#endif

#ifndef __DBL_MIN__
    #define __DBL_MIN__ 2.2250738585072014e-308
#endif

#ifndef __DBL_MAX__
    #define __DBL_MAX__ 1.7976931348623157e+308
#endif

#ifndef __DBL_EPSILON__
    #define __DBL_EPSILON__ 2.2204460492503131e-16
#endif

#endif
