/*
###############################################################################
#
#  EGSnrc egs++ math headers
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


/*! \file egs_math.h
 *  \brief Attempts to fix broken math header files.
 *  \IK
 */

#ifndef EGS_MATH_
#define EGS_MATH_

#ifdef MISSING
    #include "missing.h"
#endif

#ifdef NO_CMATH
    #include <math.h>
#else
    #include <cmath>
#endif

// M_PI is not defined after including cmath for the MS visual studio compiler?
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// Some C++ compilers don't have a proper implementation of abs() for
// all types => use a macro instead.
#ifdef DONT_HAVE_ABS
    #ifdef abs
        #undef abs
    #endif
    #define abs(a)     ((a) >= 0  ? (a) : -(a))
#endif

// Some C++ compilers don't know anything about min/max after including
// cmath.
#ifdef DONT_HAVE_MINMAX
    #ifdef min
        #undef min
    #endif
    #ifdef max
        #undef max
    #endif
    #define  max(a, b)  ((a) > (b) ? (a) :  (b))
    #define  min(a, b)  ((a) < (b) ? (a) :  (b))
#endif

// this one is needed for the SGI C++ compiler.
#ifdef STD_SQRT
    #define sqrt(a) std::sqrt(a)
#endif

#endif
