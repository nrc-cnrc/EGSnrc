/*
###############################################################################
#
#  EGSnrc egs++ radiative splitting object headers
#  Copyright (C) 2018 National Research Council Canada
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
#  Author:          Ernesto Mainegra-Hing, 2018
#
#  Contributors:
#
###############################################################################
#
#  A general radiative splitting tool.
#
#  TODO:
#
#  - Add directional radiative splitting (DRS)
#
###############################################################################
*/


/*! \file egs_radiative_splitting.h
 *  \brief A radiative splitting ausgab object: header
 *  \EM
 */

#ifndef EGS_RADIATIVE_SPLITTING_
#define EGS_RADIATIVE_SPLITTING_

#include "egs_ausgab_object.h"
#include "egs_application.h"
#include "egs_scoring.h"
#include "egs_base_geometry.h"

#ifdef WIN32

    #ifdef BUILD_RADIATIVE_SPLITTING_DLL
        #define EGS_RADIATIVE_SPLITTING_EXPORT __declspec(dllexport)
    #else
        #define EGS_RADIATIVE_SPLITTING_EXPORT __declspec(dllimport)
    #endif
    #define EGS_RADIATIVE_SPLITTING_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_RADIATIVE_SPLITTING_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_RADIATIVE_SPLITTING_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_RADIATIVE_SPLITTING_EXPORT
        #define EGS_RADIATIVE_SPLITTING_LOCAL
    #endif

#endif

/*! \brief A radiative splitting object: header

\ingroup AusgabObjects

This ausgab object can be used to increase the efficiency of radiative
events. This ausgab object is specified via:
\verbatim
:start ausgab object:
    library   = egs_radiative_splitting
    name      = some_name
    splitting = n_split
:stop ausgab object:
\endverbatim

TODO:
 - Add directional radiative splitting (DRS)

*/

class EGS_RADIATIVE_SPLITTING_EXPORT EGS_RadiativeSplitting : public EGS_AusgabObject {

public:

    /*! Splitting algortihm type */
    enum Type {
        URS, // EGSnrc Uniform Radiative Splitting
        DRS, // Directional Radiative Splitting
        DRSf // Directional Radiative Splitting (BEAMnrc)
    };

    EGS_RadiativeSplitting(const string &Name="", EGS_ObjectFactory *f = 0);

    ~EGS_RadiativeSplitting();

    void setApplication(EGS_Application *App);

    void setSplitting(const int &n_s) {
        nsplit = n_s;
    };

    int processEvent(EGS_Application::AusgabCall iarg) {
        return 0;
    };
    int processEvent(EGS_Application::AusgabCall iarg, int ir) {
        return 0;
    };

protected:
    /* Maximum splitting limited to 2,147,483,647 */
    int nsplit;

};

#endif
