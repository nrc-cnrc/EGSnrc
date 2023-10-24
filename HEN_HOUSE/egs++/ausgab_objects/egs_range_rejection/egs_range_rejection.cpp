/*
###############################################################################
#
#  EGSnrc egs++ range rejection asugab object
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
#  A general range rejection tool.
#
#  TODO:
#
#  - Add Russian roulette (RR)
#
###############################################################################
*/


/*! \file egs_range_rejection.cpp
 *  \brief A range rejection ausgab object: implementation
 *  \EM
 */

#include <fstream>
#include <string>
#include <cstdlib>

#include "egs_range_rejection.h"
#include "egs_input.h"
#include "egs_functions.h"

EGS_RangeRejection::EGS_RangeRejection(const string &Name,
        EGS_ObjectFactory *f) :
    algorithm(rr), E_max(0) {
    otype = "EGS_RangeRejection";
}

EGS_RangeRejection::EGS_RangeRejection(const string &Name,
        EGS_ObjectFactory *f, const EGS_Float &E) :
    algorithm(rr), E_max(E) {
    otype = "EGS_RangeRejection";
}

EGS_RangeRejection::~EGS_RangeRejection() {
}

void EGS_RangeRejection::setApplication(EGS_Application *App) {
    EGS_AusgabObject::setApplication(App);
    if (!app) {
        return;
    }

    char buf[32];

    EGS_Float ecut = app->getEcut();

    description = "\n===========================================\n";
    description +=  "range rejection Object (";
    description += name;
    description += ")\n";
    description += "===========================================\n";
    if (E_max >= ecut) {
        description +="\n - Range rejection below ";
        sprintf(buf,"%g MeV\n\n",E_max);
        description += buf;
        // Set EGSnrc internal range rejection number .
        app->setRangeRejection(E_max);
    }
    else {
        description +="\n - NO range rejection";
    }
    description += "\n===========================================\n\n";
}

//*********************************************************************
// Process input for this ausgab object
//
//**********************************************************************
extern "C" {

    EGS_RANGE_REJECTION_EXPORT EGS_AusgabObject *createAusgabObject(EGS_Input *input,
            EGS_ObjectFactory *f) {
        const static char *func = "createAusgabObject(range_rejection)";
        if (!input) {
            egsWarning("%s: null input?\n",func);
            return 0;
        }

        EGS_Float E_rr = 0.0;
        int err = input->getInput("maximum energy",E_rr);

        //=================================================

        /* Setup range rejection object with input parameters */
        EGS_RangeRejection *result = new EGS_RangeRejection("",f,E_rr);
        result->setName(input);
        return result;
    }
}
