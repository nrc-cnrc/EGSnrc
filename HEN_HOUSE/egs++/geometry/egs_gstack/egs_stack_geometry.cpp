/*
###############################################################################
#
#  EGSnrc egs++ stack geometry
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
#  Contributors:    Frederic Tessier
#                   Marc Chamberland
#                   Ernesto Mainegra-Hing
#                   Reid Townson
#
###############################################################################
*/


/*! \file egs_stack_geometry.cpp
 *  \brief A geometry stack: implementation
 *  \IK
 */


#include "egs_stack_geometry.h"
#include "egs_input.h"
#include "egs_functions.h"

string EGS_StackGeometry::type = "EGS_StackGeometry";

EGS_StackGeometry::EGS_StackGeometry(const vector<EGS_BaseGeometry *> &geoms,
                                     const string &Name) : EGS_BaseGeometry(Name) {
    if (geoms.size() < 2) egsFatal("EGS_StackGeometry::EGS_StackGeometry: "
                                       " less than 2 geometries is not mermitted\n");
    ng = geoms.size();
    g = new EGS_BaseGeometry* [ng];
    nmax = 1;
    has_rho_scaling = false;
    for (int j=0; j<ng; j++) {
        g[j] = geoms[j];
        g[j]->ref();
        int n = g[j]->regions();
        if (n > nmax) {
            nmax = n;
        }
        if (!has_rho_scaling) {
            has_rho_scaling = g[j]->hasRhoScaling();
        }
    }
    has_B_scaling = false;
    for (int j=0; j<ng; j++) {
        g[j] = geoms[j];
        g[j]->ref();
        int n = g[j]->regions();
        if (n > nmax) {
            nmax = n;
        }
        if (!has_B_scaling) {
            has_B_scaling = g[j]->hasBScaling();
        }
    }
    nreg = ng*nmax;
    is_convex = false;
}

EGS_StackGeometry::~EGS_StackGeometry() {
    for (int j=0; j<ng; j++)
        if (!g[j]->deref()) {
            delete g[j];
        }
    delete [] g;
}

void EGS_StackGeometry::printInfo() const {
    EGS_BaseGeometry::printInfo();
    egsInformation(" geometries:\n");
    for (int j=0; j<ng; j++) egsInformation("   %s (type %s)\n",
                                                g[j]->getName().c_str(),g[j]->getType().c_str());
    egsInformation(
        "=======================================================\n");
}

void EGS_StackGeometry::setMedia(EGS_Input *,int,const int *) {
    egsWarning("EGS_StackGeometry::setMedia: don't use this method. Use the\n"
               " setMedia() methods of the geometry objects that make up this geometry\n");
}

void EGS_StackGeometry::setRelativeRho(int start, int end, EGS_Float rho) {
    setRelativeRho(0);
}

void EGS_StackGeometry::setRelativeRho(EGS_Input *) {
    egsWarning("EGS_StackGeometry::setRelativeRho(): don't use this method.\n"
               " Use the setRelativeRho methods of the geometry objects that make up"
               " this geometry\n");
}

void EGS_StackGeometry::setBScaling(int start, int end, EGS_Float rho) {
    setBScaling(0);
}

void EGS_StackGeometry::setBScaling(EGS_Input *) {
    egsWarning("EGS_StackGeometry::setBScaling(): don't use this method.\n"
               " Use the setBScaling methods of the geometry objects that make up"
               " this geometry\n");
}

extern "C" {

    EGS_STACKG_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {
        if (!input) {
            egsWarning("createGeometry(stack): null input?\n");
            return 0;
        }
        vector<string> gnames;
        vector<EGS_BaseGeometry *> geoms;
        int err = input->getInput("geometries",gnames);
        if (err || gnames.size() < 2) {
            egsWarning("createGeometry(stack): missing/wrong 'geometries' input\n");
            return 0;
        }
        for (unsigned int j=0; j<gnames.size(); j++) {
            EGS_BaseGeometry *gj = EGS_BaseGeometry::getGeometry(gnames[j]);
            if (!gj) egsWarning("createGeometry(stack): no geometry named %s "
                                    " defined\n",gnames[j].c_str());
            else {
                geoms.push_back(gj);
            }
        }
        if (geoms.size() < 2) {
            egsWarning("createGeometry(stack): must have at least 2 geometries\n");
            return 0;
        }
        EGS_BaseGeometry *result = new EGS_StackGeometry(geoms);
        result->setName(input);
        result->setBoundaryTolerance(input);
        result->setLabels(input);
        EGS_Float tol = epsilon;
        err = input->getInput("tolerance",tol);
        if (!err) {
            result->setBoundaryTolerance(tol);
        }
        return result;
    }


    void EGS_StackGeometry::getLabelRegions(const string &str, vector<int> &regs) {

        vector<int> local_regs;

        // label defined in the stacked geometries
        for (int i=0; i<ng; i++) {
            local_regs.clear();
            g[i]->getLabelRegions(str, local_regs);
            for (int j=0; j<local_regs.size(); j++) {
                regs.push_back(i*nmax + local_regs[j]);
            }
        }

        // label defined in self (stack input block)
        EGS_BaseGeometry::getLabelRegions(str, regs);

    }


}
