/*
###############################################################################
#
#  EGSnrc egs++ library
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
#  Contributors:    Ernesto Mainegra-Hing
#
###############################################################################
*/


/*! \file egs_library.cpp
 *  \brief EGS_Librarymplementation
 *  \IK
 */

#include "egs_library.h"
#include "egs_functions.h"

#ifdef WIN32

    #include <windows.h>

    #define DLL_HANDLE HMODULE
    #define LOAD_LIBRARY(fname) LoadLibrary(fname)
    #define FREE_LIBRARY(lib)   FreeLibrary(lib);
    #define RESOLVE_SYMBOL(lib,symb) (void *) GetProcAddress(lib,symb)

#else

    #include <dlfcn.h>

    #define DLL_HANDLE void*
    #define LOAD_LIBRARY(fname) dlopen(fname,RTLD_LAZY)
    #define FREE_LIBRARY(lib)   !dlclose(lib)
    #define RESOLVE_SYMBOL(lib,symb) dlsym(lib,symb)

#endif

#include <string>

using namespace std;

#ifndef SKIP_DOXYGEN
/*! \brief EGS_Library implementation.

  \internwarning
*/
class EGS_LOCAL EGS_PrivateLibrary {
public:
    DLL_HANDLE lib;
    string name, fname;
    bool au;
    EGS_PrivateLibrary(const char *lib_name, const char *path = 0);
    ~EGS_PrivateLibrary();
    bool load();
    void *resolve(const char *symb);
    bool unload();
    static char fs;
    static const char *lib_prefix;
    static const char *lib_suffix;
};

#ifdef WIN32
    #ifdef CYGWIN
        char EGS_PrivateLibrary::fs = '/';
    #else
        char EGS_PrivateLibrary::fs = '\\';
    #endif
    const char *EGS_PrivateLibrary::lib_prefix = "";
    const char *EGS_PrivateLibrary::lib_suffix = ".dll";
#else
    char EGS_PrivateLibrary::fs = '/';
    const char *EGS_PrivateLibrary::lib_prefix = "lib";
    const char *EGS_PrivateLibrary::lib_suffix = ".so";
#endif


EGS_PrivateLibrary::EGS_PrivateLibrary(const char *lib_name, const char *path) {
    au = true;
    lib = 0;
    if (!lib_name) {
        egsWarning("EGS_Library::EGS_Library: null library name?\n");
        return;
    }
    name = lib_name;
    if (path) {
        fname = path;
        if (fname[fname.size()-1] != fs) {
            fname += fs;
        }
    }
    fname += lib_prefix;
    fname += lib_name;
    fname += lib_suffix;
#ifdef LIB_DEBUG
    egsInformation("EGS_Library::EGS_Library: file name is <%s>\n",fname.c_str());
#endif
}

EGS_PrivateLibrary::~EGS_PrivateLibrary() {
    if (au) {
        unload();
    }
}

bool EGS_PrivateLibrary::load() {
    if (lib) {
        return true;
    }
    lib = LOAD_LIBRARY(fname.c_str());
    /*
    if (!lib ) {
      const char *tmp = dlerror(); egsWarning("load library: %s\n",tmp);
    }
    */
#ifdef DLL_DEBUG
    egsInformation("In EGS_PrivateLibrary::load(): name = %s lib = 0x%x\n",
                   name.c_str(),lib);
#endif
    if (lib) {
        return true;
    }
    else {
        egsWarning("EGS_Library::load(): failed to load library %s\n",
                   fname.c_str());
#ifdef WIN32
        egsWarning("  error was: %d\n",GetLastError());
#else
        egsWarning("  error was: %s\n",dlerror());
#endif
        return false;
    }
}

void *EGS_PrivateLibrary::resolve(const char *symb) {
    if (!lib) {
        if (!load()) {
            return 0;
        }
    }
    void *result = RESOLVE_SYMBOL(lib,symb);
#ifdef DLL_DEBUG
    egsInformation("In EGS_PrivateLibrary::resolve: symbol = %s result = 0x%x\n",
                   symb,result);
#endif
    return result;
}

bool EGS_PrivateLibrary::unload() {
    if (!lib) {
        return true;
    }
    bool result = FREE_LIBRARY(lib);
    if (result) {
        lib = 0;
    }
    return result;
}
#endif

EGS_Library::EGS_Library(const char *lib_name, const char *path) {
    pl = new EGS_PrivateLibrary(lib_name,path);
}

EGS_Library::~EGS_Library() {
    delete pl;
}

bool EGS_Library::load() {
    return pl->load();
}

void *EGS_Library::resolve(const char *symb) {
    return pl->resolve(symb);
}

bool EGS_Library::unload() {
    return pl->unload();
}

bool EGS_Library::isLoaded() const {
    return (bool) pl->lib;
}

bool EGS_Library::autoUnload() const {
    return pl->au;
}

void EGS_Library::setUnload(bool u) {
    pl->au = u;
}

const char *EGS_Library::libraryName() const {
    return pl->name.c_str();
}

const char *EGS_Library::libraryFile() const {
    return pl->fname.c_str();
}

void *EGS_Library::resolve(const char *lname, const char *func,
                           const char *path) {
    EGS_PrivateLibrary p(lname,path);
    p.au = false;
    if (!p.load()) {
        return 0;
    }
    return p.resolve(func);
}
