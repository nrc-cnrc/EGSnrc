/*
###############################################################################
#
#  EGSnrc egs++ functions headers
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


#ifndef EGS_FUNCTIONS_
#define EGS_FUNCTIONS_

#include "egs_libconfig.h"

#include <iostream>
#include <string>
using namespace std;

/*! \file egs_functions.h
 *  \brief Global egspp functions.
 *  \IK
 */

/*! \brief Writes the 64 bit integer \a n to the output stream data
 * and returns \c true on success, \c false on failure.
 *
 * \ingroup egspp_main
 *
 * This function is required because some C++ compilers don't have
 * an implementation of the << operator for 64 bit integers (\em e.g.
 * MSVC). The 64 bit integer is written as two space separated 32 bit
 * integers.
 */
bool EGS_EXPORT egsStoreI64(ostream &data, EGS_I64 n);

/*! \brief Reads a 64 bit integer from the stream \a data and assigns it
 * to \a n. Returns \c true on success, \c false on failure.
 *
 * \ingroup egspp_main
 *
 * This function is required because some C++ compilers don't have
 * an implementation of the >> operator for 64 bit integers (\em e.g.
 * MSVC). The 64 bit integer is read as two space separated 32 bit
 * integers as written by egsStoreI64().
 */
bool EGS_EXPORT egsGetI64(istream &data, EGS_I64 &n);

/*! \brief Defines a function <code>printf</code>-like prototype for
 * functions to be used to report info, warnings, or errors.
 */
typedef void (*EGS_InfoFunction)(const char *,...);

/*! \brief Always use this function for reporting the progress of a simulation
 * and any other type of information.
 *
 * \ingroup egspp_main
 *
 * By defualt the output goes to the standard output. This behaviour
 * can be changed using egsSetInfoFunction(). This is used, for instance,
 * to write output generated in the C++ part of an EGSnrc application to
 * a string and then pass it to the EGSnrc mortran back-end using
 * the \c egs_write_string subroutine.
 *
 * \sa egsWarning and egsFatal.
 */
extern EGS_EXPORT EGS_InfoFunction egsInformation;

/*! \brief Always use this function for reporting warnings.
 *
 * \ingroup egspp_main
 *
 * By defualt the output goes to standard error. This behaviour
 * can be changed using egsSetInfoFunction().
 *
 * \sa egsInformation, egsFatal
 */
extern EGS_EXPORT EGS_InfoFunction egsWarning;

/*! \brief Always use this function for reporting fatal errors.
 *
 * \ingroup egspp_main
 *
 * By defualt the output goes to standard error and the \c exit function
 * is called. This behaviour can be changed using egsSetInfoFunction().
 * However, keep in mind that there may be code that is not prepared to
 * deal with the situation that egsFatal actually returns.
 */
extern EGS_EXPORT EGS_InfoFunction egsFatal;

enum EGS_InfoType { Information, Warning, Fatal };

/*! \brief Set a function to be used for outputing information, warning
 * messages or reporting fatal errors.
 *
 * \ingroup egspp_main
 *
 * Sets the function of type \a t to \a func and returns the previously
 * used function for messages of that type.
 */
extern EGS_InfoFunction EGS_EXPORT
egsSetInfoFunction(EGS_InfoType t, EGS_InfoFunction func);

/*! \brief Reset I/O functions to their defaults
 *
 * \ingroup egspp_main
*/
extern void EGS_EXPORT egsSetDefaultIOFunctions();

/*! \brief Remove the $'s from a CVS key
 *
 * \ingroup egspp_main
 *
 * This is handy in cases when one wants to output the value of a CVS key
 * such as Id, Revision, etc., without the surrounding dollar signs.
 */
string EGS_EXPORT egsSimplifyCVSKey(const string &key);

/*! \brief Swap the bytes of 32 bit integers.
 *
 * \ingroup egspp_main
 *
 */
void EGS_EXPORT egsSwapBytes(int *);

/*! \brief Swap the bytes of 16 bit integers.
 *
 * \ingroup egspp_main
 *
 */
void EGS_EXPORT egsSwapBytes(short *);

/*! \brief Swap the bytes of 32 bit reals.
 *
 * \ingroup egspp_main
 *
 */
void EGS_EXPORT egsSwapBytes(float *);

/*! \brief Join two path variables (or a path and a file name)
  using the platform specific directory separator and return the result.
 *
 * \ingroup egspp_main
 */
string EGS_EXPORT egsJoinPath(const string &first, const string &second);

/*! \brief Strip the path from a file name and return the result.
 *
 * \ingroup egspp_main
 */
string EGS_EXPORT egsStripPath(const string &fname);

/*! \brief Get the name of the host the program is running on.
 *
 * \ingroup egspp_main
 */
string EGS_EXPORT egsHostName();

/*! \brief Get the process id.
 *
 * \ingroup egspp_main
 */
int EGS_EXPORT egsGetPid();

/*! \brief Get the endianess of the machine.
 *
 * Returns 0, of the machine is big-endian, 1 if it is little endian,
 * and -1, if the endianess could not be determined.
 * \ingroup egspp_main
 */
int EGS_EXPORT egsGetEndian();

/*! \brief Does the string \a path represent an absolute path name?
 *
 * \ingroup egspp_main
 */
bool EGS_EXPORT egsIsAbsolutePath(const string &path);


#endif
