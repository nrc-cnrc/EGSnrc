/*
###############################################################################
#
#  EGSnrc egs++ union geometry
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
#                   Ernesto Mainegra-Hing
#                   Hubert Ho
#                   Marc Chamberland
#
###############################################################################
*/


/*! \file egs_union_geometry.cpp
 *  \brief A geometry union
 *  \IK
 */

#include "egs_union_geometry.h"
#include "egs_input.h"
#include "egs_functions.h"

using namespace std;

string EGS_UNIONG_LOCAL EGS_UnionGeometry::type = "EGS_UnionGeometry";

void EGS_UnionGeometry::setMedia(EGS_Input *,int,const int *) {
    egsWarning("EGS_UnionGeometry::setMedia: don't use this method. Use the\n"
               " setMedia() methods of the geometry objects that make up this geometry\n");
}

void EGS_UnionGeometry::setRelativeRho(int start, int end, EGS_Float rho) {
    setRelativeRho(0);
}

void EGS_UnionGeometry::setRelativeRho(EGS_Input *) {
    egsWarning("EGS_UnionGeometry::setRelativeRho(): don't use this method. "
               "Use the\n setRelativeRho() methods of the geometry objects that make "
               "up this geometry\n");
}

void EGS_UnionGeometry::setBScaling(int start, int end, EGS_Float bf) {
    setBScaling(0);
}

void EGS_UnionGeometry::setBScaling(EGS_Input *) {
    egsWarning("EGS_UnionGeometry::setBScaling(): don't use this method. "
               "Use the\n setBScaling() methods of the geometry objects that make "
               "up this geometry\n");
}

EGS_UnionGeometry::EGS_UnionGeometry(const vector<EGS_BaseGeometry *> &geoms,
                                     const int *priorities, const string &Name) :
    EGS_BaseGeometry(Name) {
    ng = geoms.size();
    if (ng <= 0) egsFatal("EGS_UnionGeometry::EGS_UnionGeometry: attempt "
                              " to construct a union geometry from zero geometries\n");
    is_convex = false;
    if (ng == 1) egsWarning("EGS_UnionGeometry::EGS_UnionGeometry: why "
                                "do you want to make a union out of a single geometry?\n");
    g = new EGS_BaseGeometry* [ng];
    nmax = 0;
    int j;
    int *order = new int [ng];
    if (priorities) {
        // user has definied priorities
        // order them using a very simplistic algorithm
        bool *is_used = new bool [ng];
        for (j=0; j<ng; j++) {
            is_used[j] = false;
        }
        for (j=0; j<ng; j++) {
            int imax;
            for (imax=0; imax<ng-1; imax++) if (!is_used[imax]) {
                    break;
                }
            int pmax = priorities[imax];
            for (int i=0; i<ng; i++) {
                if (!is_used[i] && priorities[i] > pmax) {
                    imax = i;
                    pmax = priorities[i];
                }
            }
            order[j] = imax;
            is_used[imax] = true;
        }
        delete [] is_used;
    }
    else {
        // user has not definied priorities
        for (j=0; j<ng; j++) {
            order[j] = j;
        }
    }
    has_rho_scaling = false;
    // now put the geometries into the array of geometries in
    // decreasing priority order.
    for (j=0; j<ng; j++) {
        int i = order[j];
        g[i] = geoms[j];
        g[i]->ref();
        int n = g[i]->regions();
        if (n > nmax) {
            nmax = n;
        }
        if (!has_rho_scaling) {
            has_rho_scaling = g[i]->hasRhoScaling();
        }
    }
    has_B_scaling = false;
    // now put the geometries into the array of geometries in
    // decreasing priority order.
    for (j=0; j<ng; j++) {
        int i = order[j];
        g[i] = geoms[j];
        g[i]->ref();
        int n = g[i]->regions();
        if (n > nmax) {
            nmax = n;
        }
        if (!has_B_scaling) {
            has_B_scaling = g[i]->hasBScaling();
        }
    }
    delete [] order;
    if (!nmax) egsFatal("EGS_UnionGeometry::EGS_UnionGeometry: all geometries"
                            " have zero regions?\n");
    nreg = nmax*ng;
}

EGS_UnionGeometry::~EGS_UnionGeometry() {
    for (int j=0; j<ng; j++) {
        if (!g[j]->deref()) {
            delete g[j];
        }
    }
    delete [] g;
}

void EGS_UnionGeometry::printInfo() const {
    EGS_BaseGeometry::printInfo();
    egsInformation(" geometries:\n");
    for (int j=0; j<ng; j++) egsInformation("   %s (type %s)\n",
                                                g[j]->getName().c_str(),g[j]->getType().c_str());
    egsInformation(
        "=======================================================\n");
}

extern "C" {

    EGS_UNIONG_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {
        if (!input) {
            egsWarning("createGeometry(union): null input?\n");
            return 0;
        }
        vector<EGS_BaseGeometry *> geoms;
        vector<string> gnames;
        int err = input->getInput("geometries",gnames);
        if (err || gnames.size() < 1) {
            egsWarning("createGeometry(union): missing/wrong 'geometries' input\n");
            return 0;
        }
        for (unsigned int j=0; j<gnames.size(); j++) {
            EGS_BaseGeometry *gj = EGS_BaseGeometry::getGeometry(gnames[j]);
            if (!gj) egsWarning("createGeometry(union): no geometry named %s "
                                    "defined\n",gnames[j].c_str());
            else {
                geoms.push_back(gj);
            }
        }
        if (geoms.size() < 1) {
            egsWarning("createGeometry(union): must have at least one geometry\n");
            return 0;
        }
        vector<int> pri;
        err = input->getInput("priorities",pri);
        int *p = 0;
        if (!err) {
            if (pri.size() == geoms.size()) {
                p = new int [pri.size()];
                for (int i=0; i<pri.size(); i++) {
                    p[i] = pri[i];
                }
            }
            else egsWarning("createGeometry(union): the number of priorities (%d)"
                                " is not the same as the number of geometries (%d) => ignoring\n",
                                pri.size(),geoms.size());
        }
        EGS_BaseGeometry *result = new EGS_UnionGeometry(geoms,p);
        result->setName(input);
        result->setBoundaryTolerance(input);
        result->setLabels(input);
        if (p) {
            delete [] p;
        }
        return result;
    }

    void EGS_UnionGeometry::getLabelRegions(const string &str, vector<int> &regs) {

        // label defined in the sub-geometries
        vector<int> gregs;
        for (int i=0; i<ng; i++) {

            // add regions from set geometries
            gregs.clear();
            if (g[i]) {
                g[i]->getLabelRegions(str, gregs);
            }

            // shift region numbers according to indexing style
            for (int j=0; j<gregs.size(); j++) {
                gregs[j] += i*nmax;
            }

            // add regions to the list
            regs.insert(regs.end(), gregs.begin(), gregs.end());

        }

        // label defined in self (union geometry input block)
        EGS_BaseGeometry::getLabelRegions(str, regs);

    }

}
