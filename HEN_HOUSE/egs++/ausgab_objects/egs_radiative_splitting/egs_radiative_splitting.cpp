/*
###############################################################################
#
#  EGSnrc egs++ radiative splitting object
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


/*! \file egs_radiative_splitting.cpp
 *  \brief A radiative splitting ausgab object: implementation
 *  \EM
 */

#include <fstream>
#include <string>
#include <cstdlib>

#include "egs_radiative_splitting.h"
#include "egs_input.h"
#include "egs_functions.h"

EGS_RadiativeSplitting::EGS_RadiativeSplitting(const string &Name,
        EGS_ObjectFactory *f) :
    nsplit(1) {
    otype = "EGS_RadiativeSplitting";
}

EGS_RadiativeSplitting::~EGS_RadiativeSplitting() {
}

void EGS_RadiativeSplitting::setApplication(EGS_Application *App) {
    EGS_AusgabObject::setApplication(App);
    if (!app) {
        return;
    }

    char buf[32];

    // Set EGSnrc internal UBS + RR
    if (i_play_RR) {
        app->setRussianRoulette(nsplit);
        i_play_RR = true;
    }
    // Set EGSnrc internal radiative splitting number.
    else if (nsplit > 0) {
        app->setRadiativeSplitting(nsplit);
        i_play_RR = false;
    }

    description = "\n===========================================\n";
    description +=  "Radiative splitting Object (";
    description += name;
    description += ")\n";
    description += "===========================================\n";
    if (i_play_RR) {
        description +="\n - Splitting radiative events in ";
        sprintf(buf,"%d",nsplit);
        description += buf;
        description +="\n - Play RR with higher order e-/e+ with probability 1/";
        sprintf(buf,"%d\n\n",nsplit);
        description += buf;
    }
    else if (nsplit > 1) {
        description +="\n - Splitting radiative events in ";
        sprintf(buf,"%d\n\n",nsplit);
        description += buf;
    }
    else if (nsplit == 1) {
        description +="\n - NO radiative splitting";
    }
    else {
        description +="\n - BEWARE: Turning OFF radiative events !!!";
    }
    description += "\n===========================================\n\n";
}

//*********************************************************************
// Process input for this ausgab object
//
//**********************************************************************
extern "C" {

    EGS_RADIATIVE_SPLITTING_EXPORT EGS_AusgabObject *createAusgabObject(EGS_Input *input,
            EGS_ObjectFactory *f) {
        const static char *func = "createAusgabObject(radiative_splitting)";
        if (!input) {
            egsWarning("%s: null input?\n",func);
            return 0;
        }

        EGS_Float nsplit = 1.0;
        /*! Switch for splitting + RR. Negative nsplit value switches OFF RR. */
        int err = input->getInput("splitting",nsplit);

        //=================================================

        /* Setup radiative splitting object with input parameters */
        EGS_RadiativeSplitting *result = new EGS_RadiativeSplitting("",f);
        result->setSplitting(nsplit);
        result->setName(input);
        return result;
    }
}
