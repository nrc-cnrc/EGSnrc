/*
###############################################################################
#
#  EGSnrc egs++ library headers
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
#  Contributors:    Reid Townson
#
###############################################################################
*/


/*! \file egs_library.h
 *  \brief EGS_Library class header file
 *  \IK
 */

#ifndef EGS_LIBRARY_
#define EGS_LIBRARY_

#include "egs_libconfig.h"

class EGS_PrivateLibrary;

/*! \brief A class for dynamically loading shared libraries.

  \ingroup egspp_main

  This class provides a platform-independent interface to the system specific
  process of loading dynamic shared objects (DSO), a.k.a. DLLs and resolving
  the addresses of symbols exported by the library.
*/
class EGS_EXPORT EGS_Library {

    //! Pointer to the private class implementing the functionality.
    EGS_PrivateLibrary *pl;

public:

    /*! \brief Constructs the library object and sets the DSO name to \a lib_name.

        \a lib_name should not contain platform specific prefixes or extensions,
        this will be handled by the library object (\em i.e.
        <code>lib_name = mylib</code>
        will result in \c mylib.dll under Windows and \c libmylib.so under Unix.)
        If \a path is \c null, the library must be in the standard search
        path for DSOs. If path contains a valid path specification,
        the library name will be constracted from \a path and \a lib_name.
     */
    EGS_Library(const char *lib_name, const char *path = 0);

    /*! \brief Destructs the library object.

        The library will be unloaded
        unless the \a auto_unload flag is set to \c false using setUnload().
     */
    ~EGS_Library();

    /*! \brief Loads the library.

        It is not necessary to call this function
        before using resolve(). Will return \c true on success and
        \c false otherwise.
     */
    bool load();

    /*! \brief Returns the address of the exported symbol \a func.

        Calls the load() function if necessary. Returns the address of
        the symbol on success or \c null if the symbol could not
        be resolved or the library could not be loaded.
     */
    void *resolve(const char *func);

    /*! \brief Unloads the library.

        Returns \c true on success, \c false otherwise.
        This function is called by the destructor if the library object
        was set to automatically unload the library with setUnload().
     */
    bool unload();

    /*! \brief Returns \c true if the library is loaded, \c false otherwise
     */
    bool isLoaded() const;

    /*! \brief Returns \c true if the library automatically unloads when
        the object is destructed, \c false otherwise.

        \sa setUnload().
     */
    bool autoUnload() const;

    /*! \brief Set automatic unloading to \a u. */
    void setUnload(bool u);

    /*! \brief Returns the name of the library object as given in the
      constructor. */
    const char *libraryName() const;

    /*! \brief Returns the name of the DSO, including full path and
      platform-specific prefix and extension. */
    const char *libraryFile() const;

    /*! \brief Resolve the address of the symbol \a func from the DSO \a lname.

    This static function is provided for convenience. It loads the library
    \a lname
    and returns the address of the symbol \a func on success, \c null if
    the library could not be loaded or if it does not export a symbol named
    \a func. If \a path is null, the library will be searched for in the
    standard set of library search paths, otherwise the library file name
    will be constructed from \a path and \a lname.

    \sa EGS_Library::EGS_Library(), load(), resolve().
    */
    static void *resolve(const char *lname, const char *func,
                         const char *path = 0);
};

#endif
