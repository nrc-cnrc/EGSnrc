/*
###############################################################################
#
#  EGSnrc egs++ point shape
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
*/

#ifndef EGS_POINT_SHAPE_
#define EGS_POINT_SHAPE_

#include "egs_shapes.h"

#ifdef WIN32
    #ifdef BUILD_POINT_SHAPE_DLL
        #define EGS_POINT_SHAPE_EXPORT __declspec(dllexport)
    #else
        #define EGS_POINT_SHAPE_EXPORT __declspec(dllimport)
    #endif
    #define EGS_POINT_SHAPE_LOCAL
#else
    #ifdef HAVE_VISIBILITY
        #define EGS_POINT_SHAPE_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_POINT_SHAPE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_POINT_SHAPE_EXPORT
        #define EGS_POINT_SHAPE_LOCAL
    #endif
#endif

#endif
