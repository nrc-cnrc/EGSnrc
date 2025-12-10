/*
###############################################################################
#
#  EGSnrc egs++ range rejection ausgab object headers
#  Copyright (C) 2023 National Research Council Canada
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
#  Author:          Ernesto Mainegra-Hing, 2023
#
#  Contributors:
#
###############################################################################
#
#  A general range rejection tool.
#
#  TODO:
#
#  - Add Russian roulette (RR)
#
###############################################################################
*/


/*! \file egs_range_rejection.h
 *  \brief A range rejection ausgab object: header
 *  \EM
 */

#ifndef EGS_RANGE_REJECTION_
#define EGS_RANGE_REJECTION_

#include "egs_ausgab_object.h"
#include "egs_application.h"
#include "egs_scoring.h"
#include "egs_base_geometry.h"

#ifdef WIN32

    #ifdef BUILD_RANGE_REJECTION_DLL
        #define EGS_RANGE_REJECTION_EXPORT __declspec(dllexport)
    #else
        #define EGS_RANGE_REJECTION_EXPORT __declspec(dllimport)
    #endif
    #define EGS_RANGE_REJECTION_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_RANGE_REJECTION_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_RANGE_REJECTION_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_RANGE_REJECTION_EXPORT
        #define EGS_RANGE_REJECTION_LOCAL
    #endif

#endif

/*! \brief A range rejection object: header

\ingroup AusgabObjects

This ausgab object can be used to increase the efficiency of radiative
events. This ausgab object is specified via:
\verbatim
:start ausgab object:
    library        = egs_range_rejection
    name           = some_name
    maximum energy = E # Energy below which to reject charged particles [MeV]
:stop ausgab object:
\endverbatim

TODO:
 - Add Russian roulette (RR)

*/

class EGS_RANGE_REJECTION_EXPORT EGS_RangeRejection : public EGS_AusgabObject {

public:

    /*! Rejection algorithm type */
    enum AlgorithmType {
        rr,  // EGSnrc intrinsic range rejection (approximation)
        rrRR // Range rejection with Russin roulette (true VRT)
    };

    EGS_RangeRejection(const string &Name="", EGS_ObjectFactory *f = 0);
    EGS_RangeRejection(const string &Name="", EGS_ObjectFactory *f = 0, const EGS_Float &E = 0);

    ~EGS_RangeRejection();

    void setApplication(EGS_Application *App);

    int processEvent(EGS_Application::AusgabCall iarg) {
        return 0;
    };

    int processEvent(EGS_Application::AusgabCall iarg, int ir) {
        return 0;
    };

protected:
    /*! Maximum energy below which to reject charged particles. Rest mass included. */
    EGS_Float E_max;
    AlgorithmType algorithm;

};

#endif
