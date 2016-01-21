/*
###############################################################################
#
#  EGSnrc egs++ symbol export control headers
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


#ifndef EGS_LIBCONFIG_
#define EGS_LIBCONFIG_

/*! \file egs_libconfig.h
    \brief Defines the EGS_EXPORT and EGS_LOCAL macros.
    \IK
*/

/*!  \def EGS_EXPORT
 \brief Export symbols from the egspp library

 This macro is set to __declspec(dllexport) or __declspec(dllimport)
 on Windows, depending on whether the library is being build or used.
 On Linux systems and a GCC compiler supporting the visibility attribute
 the HAVE_VISIBILITY macro should be defined to obtain an optimized
 library that only exports symbols marked as such.
*/

#ifdef MISSING
    #include "missing.h"
#endif

#include "egs_config1.h"

/*
     Macro to mark classes and functions as exports/imports
     on Windows or to assign default or hidden visibility
*/

#ifdef WIN32

    #ifdef BUILD_APP_LIB
        #define APP_EXPORT __declspec(dllexport)
    #else
        #define APP_EXPORT
    #endif

    #ifdef BUILD_DLL
        #define EGS_EXPORT __declspec(dllexport)
    #else
        #define EGS_EXPORT __declspec(dllimport)
    #endif
    #define EGS_LOCAL
    #define APP_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #ifdef BUILD_APP_LIB
            #define APP_EXPORT __attribute__ ((visibility ("default")))
        #else
            #define APP_EXPORT
        #endif
        #define EGS_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_LOCAL  __attribute__ ((visibility ("hidden")))
        #define APP_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define APP_EXPORT
        #define EGS_EXPORT
        #define EGS_LOCAL
        #define APP_LOCAL
    #endif

#endif

/*
      Use the following functions to print info, warnings, or to indicate
      a fatal error condition.
typedef void (*EGS_InfoFunction)(const char *,...);
extern EGS_EXPORT EGS_InfoFunction egsInformation;
extern EGS_EXPORT EGS_InfoFunction egsWarning;
extern EGS_EXPORT EGS_InfoFunction egsFatal;
*/

#endif
