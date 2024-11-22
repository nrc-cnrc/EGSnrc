/*
###############################################################################
#
#  EGSnrc egs++ rz geometry
#  Copyright (C) 2016 Randle E. P. Taylor, Rowan M. Thomson,
#  Marc J. P. Chamberland, Dave W. O. Rogers
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
#  Author:          Randle Taylor, 2016
#
#  Contributors:    Marc Chamberland
#                   Rowan Thomson
#                   Dave Rogers
#                   Martin Martinov
#
###############################################################################
#
#  egs_rz was developed for the Carleton Laboratory for Radiotherapy Physics.
#
###############################################################################
*/


/*! \file egs_rz.cpp
 *  \brief An egs_nd_geometry wrapper to simplify RZ geometry creation
 *  \author Randle Taylor
 */

#include <map>
#include "egs_input.h"
#include "egs_rz.h"
#include "../egs_cylinders/egs_cylinders.h"
#include "../egs_planes/egs_planes.h"


string EGS_RZGeometry::RZType = "EGS_RZ";

EGS_RZGeometry::EGS_RZGeometry(vector<EGS_BaseGeometry *> geoms,
                               vector<EGS_Float> rads, vector<EGS_Float> zbs, const string &name) :
    EGS_NDGeometry(geoms, name), radii(rads), zbounds(zbs) {

    /* we always set inner most radii to 0 (simplifies volume calcs) */
    if (radii[0] != 0) {
        radii.insert(radii.begin(), 0.);
    }

    vector<EGS_Float> vol;
    for (size_t r=0; r < radii.size()-1; r++) {

        EGS_Float rmin = radii[r];
        EGS_Float rmax = radii[r+1];

        EGS_Float area = M_PI*(rmax*rmax - rmin*rmin);

        for (size_t plane = 0; plane < zbounds.size()-1; plane++) {
            EGS_Float zmin = zbounds[plane];
            EGS_Float zmax = zbounds[plane+1];
            reg_vol.push_back((zmax-zmin)*area);
        }
    }

};

EGS_Float EGS_RZGeometry::getBound(int idir, int ind) {
    if (idir == ZDIR && ind >= 0 && ind < (int)zbounds.size()) {
        return zbounds[ind];
    }
    else if (idir == RDIR && ind >= 0 && ind < (int)radii.size()) {
        return radii[ind];
    }
    return 0.;
}


int EGS_RZGeometry::getNRegDir(int dir) {
    if (dir == ZDIR) {
        return zbounds.size() - 1;
    }
    else if (dir == RDIR) {
        return radii.size() - 1;
    }
    return 0;
}

EGS_Float EGS_RZGeometry::getVolume(int ireg) {
    if (ireg < 0 || ireg >= nreg) {
        return -1;
    }
    return reg_vol[ireg];
}


/* Initialization helpers for EGS_RZ */
namespace egs_rz {

vector<EGS_Float> getRadiiByShells(EGS_Input *input) {

    vector<EGS_Float> radii;

    vector<int> nshells;
    int err = input->getInput("number of shells", nshells);
    if (err) {
        return radii;
    }

    vector<EGS_Float> thick;
    err = input->getInput("shell thickness", thick);
    if (err) {
        return radii;
    }

    EGS_Float cur_r = 0;
    for (size_t shell_group=0; shell_group < min(thick.size(), nshells.size()); shell_group++) {

        for (int shell = 0; shell < nshells[shell_group]; shell++) {
            cur_r += thick[shell_group];
            radii.push_back(cur_r);

        }
    }

    return radii;

}

vector<EGS_Float> getRadii(EGS_Input *input) {

    vector<EGS_Float> radii;
    int err = input->getInput("radii", radii);
    if (err) {
        /* no explicit radii input so assume user will be specifying using shells */
        radii = getRadiiByShells(input);
    }

    return radii;

}


vector<EGS_Float> EGS_RZ_LOCAL getZPlanesBySlabs(EGS_Input *input) {

    vector<EGS_Float> zplanes;

    EGS_Float zo = 0;

    int err = input->getInput("first plane", zo);
    if (err) {
        egsWarning("RZ: missing 'first plane' input. Assuming zo=0");
        zo  = 0;
    }

    vector<int> nslabs;
    err = input->getInput("number of slabs", nslabs);
    if (err) {
        return zplanes;
    }

    vector<EGS_Float> thick;
    err = input->getInput("slab thickness", thick);
    if (err) {
        return zplanes;
    }

    EGS_Float cur_z = zo;
    zplanes.push_back(zo);
    for (size_t slab_group=0; slab_group < min(thick.size(), nslabs.size()); slab_group++) {

        for (int slab = 0; slab < nslabs[slab_group]; slab++) {
            cur_z += thick[slab_group];
            zplanes.push_back(cur_z);
        }
    }

    return zplanes;

}

vector<EGS_Float> EGS_RZ_LOCAL getZPlanes(EGS_Input *input) {

    vector<EGS_Float> zplanes;
    int err = input->getInput("z-planes", zplanes);
    if (err) {
        zplanes = getZPlanesBySlabs(input);
    }

    return zplanes;

}

bool allIncreasing(vector<EGS_Float> vec) {

    if (vec.size() == 0) {
        return true;
    }

    EGS_Float last = vec[0];
    for (size_t i=1; i < vec.size(); i++) {
        if (vec[i] <= last) {
            return false;
        }
        last = vec[i];
    }

    return true;

}


};

extern "C" {

    EGS_RZ_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {

        if (!input) {
            egsWarning("createGeometry(egs_rz): null input?\n");
            return 0;
        }

        vector<EGS_Float> radii = egs_rz::getRadii(input);
        if (radii.size() == 0) {
            egsWarning("createGeometry(rz): wrong/missing radii inputs\n");
            return 0;
        }
        if (!egs_rz::allIncreasing(radii)) {
            egsWarning("createGeometry(rz): radii must be monotonically increasing\n");
            return 0;
        }

        vector<EGS_Float> zplanes = egs_rz::getZPlanes(input);
        if (zplanes.size() == 0) {
            egsWarning("createGeometry(rz): wrong/missing z plane inputs\n");
            return 0;
        }

        EGS_CylindersZ *cyl = new EGS_CylindersZ(radii, EGS_Vector(), EGS_BaseGeometry::getUniqueName(), EGS_ZProjector(""));
        EGS_PlanesZ *planes = new EGS_PlanesZ(zplanes, EGS_BaseGeometry::getUniqueName(), EGS_ZProjector("z-planes"));

        vector<EGS_BaseGeometry *> rz_geoms;
        rz_geoms.push_back(planes);
        rz_geoms.push_back(cyl);

        EGS_BaseGeometry *rz = new EGS_RZGeometry(rz_geoms, radii, zplanes);

        if (!rz) {
            egsWarning("createGeometry(rz): failed to create nd geometry\n");
            return 0;
        }

        rz->setName(input);
        rz->setMedia(input);
        rz->setLabels(input);

        return rz;

    }

}
