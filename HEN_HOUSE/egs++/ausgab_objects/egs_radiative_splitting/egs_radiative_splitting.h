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
    /*! Switch for splitting + RR. Negative nsplit value switches OFF RR. */
    void setSplitting(const int &n_s) {
        nsplit = n_s;
        if (nsplit < 0) {
            nsplit *= -1;
            i_play_RR = false;
        }
        else if (nsplit > 1) {
            i_play_RR = true;
        }
        /* Avoid zero division. A zero value turns off brems */
        wthin = nsplit ? 1./nsplit : 1.0;
    };

    bool needsCall(EGS_Application::AusgabCall iarg) const {
        if (
            iarg == EGS_Application::BeforeBrems       ||
            iarg == EGS_Application::BeforeAnnihFlight ||
            iarg == EGS_Application::BeforeAnnihRest   ||
            iarg == EGS_Application::AfterBrems        ||
            iarg == EGS_Application::AfterAnnihFlight  ||
            iarg == EGS_Application::AfterAnnihRest    ||
            iarg == EGS_Application::FluorescentEvent) {
            return true;
        }
        else {
            return false;
        }
    };

    int processEvent(EGS_Application::AusgabCall iarg) {

        /* A fat particle's weight is larger than a thin particle's max weight */
        bool is_phat = (app->top_p.wt - wthin) > epsilon;
        bool is_primary = app->top_p.latch == 0 ? true : false;

        /* Split radiative events ONLY for primary and fat electrons */
        if (iarg == EGS_Application::BeforeBrems       ||
                iarg == EGS_Application::BeforeAnnihFlight ||
                iarg == EGS_Application::BeforeAnnihRest   &&
                (is_primary || is_phat)) {
            app->setRadiativeSplitting(nsplit);
        }
        /* Avoids higher order splitting of radiative events */
        else if (iarg == EGS_Application::AfterBrems       ||
                 iarg == EGS_Application::AfterAnnihFlight ||
                 iarg == EGS_Application::AfterAnnihRest) {
            app->setRadiativeSplitting(1);
            app->setLatch(app->getNpOld()+1,1);
        }
        /* Fluorescent photons created by charged particles surviving RR
           when radiative splitting ON should be split to avoid having heavy photons.
           This should happen in EGSnrc, but it is not implemented yet, so do it here!
           Note that when this is implemented in EGSnrc, the weight check will make sure
           photons aren't split again!
        */
        if (iarg == EGS_Application::FluorescentEvent && is_phat && nsplit > 1) {
            app->splitTopParticleIsotropically(nsplit);
        }


        return 0;
    };

protected:
    /* Max weight of thin particles */
    EGS_Float wthin;
    /* Maximum splitting limited to 2,147,483,647. Negative value switches OFF RR. */
    int nsplit;
    /* Switch for Russian Roulette */
    bool i_play_RR;

};

#endif
