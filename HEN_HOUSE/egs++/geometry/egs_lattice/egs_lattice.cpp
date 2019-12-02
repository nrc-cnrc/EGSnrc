/*
###############################################################################
#
#  EGSnrc egs++ lattice geometry
#  Copyright (C) 2019 Rowan Thomson and Martin Martinov
#
#  This file is part of EGSnrc.
#
#  egs_lattice is free software: you can redistribute it and/or modify it under
#  the terms of the GNU Affero General Public License as published by the Free
#  Software Foundation, either version 3 of the License, or (at your option)
#  any later version.
#
#  egs_lattice is distributed in the hope that it will be useful, but WITHOUT
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
#  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License
#  for more details.
#
#  To see the GNU Affero General Public License at:
# <http://www.gnu.org/licenses/>.
#
################################################################################
#
#  When egs_lattice is used for publications, please cite the following paper:
#  Manuscript submission underway, to be replaced with title after publication
#
###############################################################################
#
#  Author:          Martin Martinov, 2019
#
#  Contributors:
#
###############################################################################
*/


/*! \file egs_lattice.cpp
 *  \brief A Bravais lattice geometry
 */

#include "egs_lattice.h"
#include "egs_input.h"
#include "egs_functions.h"

using namespace std;

void EGS_Lattice::setMedia(EGS_Input *,int,const int *) {
    egsWarning("EGS_Lattice::setMedia: don't use this method. Use the\n"
               " setMedia() methods of the geometry objects that make up this geometry\n");
}

void EGS_Lattice::setRelativeRho(int start, int end, EGS_Float rho) {
    setRelativeRho(0);
}

void EGS_Lattice::setRelativeRho(EGS_Input *) {
    egsWarning("EGS_Lattice::setRelativeRho(): don't use this method."
               " Use the\n setRelativeRho methods of the geometry objects that make up"
               " this geometry\n");
}

void EGS_Lattice::setBScaling(int start, int end, EGS_Float bf) {
    setBScaling(0);
}

void EGS_Lattice::setBScaling(EGS_Input *) {
    egsWarning("EGS_Lattice::setBScaling(): don't use this method. "
               "Use the\n setBScaling() methods of the geometry objects that make "
               "up this geometry\n");
}

EGS_Lattice::EGS_Lattice(EGS_BaseGeometry *B, EGS_BaseGeometry *S, int i, EGS_Float x,
                         EGS_Float y, EGS_Float z, const string &Name)
    : EGS_BaseGeometry(Name), base(B),
      sub(new EGS_TransformedGeometry(S,EGS_Vector(0,0,0))),
      ind(i), a(x), b(y), c(z) {
    type            = base->getType();
    type += " with a lattice of ";
    type += sub->getType();
    nreg            = base->regions() + sub->regions();
    has_rho_scaling = base->hasRhoScaling();
    maxStep         = base->regions()+1000000*sub->regions(); // Arbitrary step-length because I can't
};                                                            // think of an elegant way to do this

EGS_Lattice::~EGS_Lattice() {
    if (!sub->deref()) {
        delete sub;
    }
    if (!base->deref()) {
        delete base;
    }
};

void EGS_Lattice::printInfo() const {
    EGS_BaseGeometry::printInfo();
    egsInformation(" base geometry = %s (type %s)\n",base->getName().c_str(),
                   base->getType().c_str());
    egsInformation(" lattice subgeometry = %s (type %s)\n",sub->getName().c_str(),
                   sub->getType().c_str());
    egsInformation(" lattice region %d with an (x,y,z) spacing of (%d,%d,%d)",
                   ind,a,b,c);
    egsInformation("=======================================================\n");
}

void EGS_Hexagonal_Lattice::setMedia(EGS_Input *,int,const int *) {
    egsWarning("EGS_Hexagonal_Lattice::setMedia: don't use this method. Use the\n"
               " setMedia() methods of the geometry objects that make up this geometry\n");
}

void EGS_Hexagonal_Lattice::setRelativeRho(int start, int end, EGS_Float rho) {
    setRelativeRho(0);
}

void EGS_Hexagonal_Lattice::setRelativeRho(EGS_Input *) {
    egsWarning("EGS_Hexagonal_Lattice::setRelativeRho(): don't use this method."
               " Use the\n setRelativeRho methods of the geometry objects that make up"
               " this geometry\n");
}

void EGS_Hexagonal_Lattice::setBScaling(int start, int end, EGS_Float bf) {
    setBScaling(0);
}

void EGS_Hexagonal_Lattice::setBScaling(EGS_Input *) {
    egsWarning("EGS_Hexagonal_Lattice::setBScaling(): don't use this method. "
               "Use the\n setBScaling() methods of the geometry objects that make "
               "up this geometry\n");
}

EGS_Hexagonal_Lattice::EGS_Hexagonal_Lattice(EGS_BaseGeometry *B, EGS_BaseGeometry *S, int i, EGS_Float x,
        const string &Name)
    : EGS_BaseGeometry(Name), base(B),
      sub(new EGS_TransformedGeometry(S,EGS_Vector(0,0,0))),
      ind(i), a(x), d(4,0.0) {
    type            = base->getType();
    type += " with a hexagonal lattice of ";
    type += sub->getType();
    gap             = a*sqrt(3.0)/2.0;
    nreg            = base->regions() + sub->regions();
    has_rho_scaling = base->hasRhoScaling();
    maxStep         = base->regions()+1000000*sub->regions(); // Arbitrary step-length because I can't
};                                                            // think of an elegant way to do this

EGS_Hexagonal_Lattice::~EGS_Hexagonal_Lattice() {
    if (!sub->deref()) {
        delete sub;
    }
    if (!base->deref()) {
        delete base;
    }
};

void EGS_Hexagonal_Lattice::printInfo() const {
    EGS_BaseGeometry::printInfo();
    egsInformation(" base geometry = %s (type %s)\n",base->getName().c_str(),
                   base->getType().c_str());
    egsInformation(" hexagonal lattice subgeometry = %s (type %s)\n",sub->getName().c_str(),
                   sub->getType().c_str());
    egsInformation(" hexagonal lattice region %d with a spacing of %d",ind,a);
    egsInformation("=======================================================\n");
}
extern "C" {

    EGS_LATTICE_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {
        int err = 0;

        // Get base geometry
        EGS_Input *i = input->getInputItem("base geometry");
        if (!i) {
            egsWarning("createGeometry(lattice): base geometry must be defined\n"
                       " using 'base geometry = some_geom'?\n");
            return 0;
        }
        EGS_Input *ig = i->getInputItem("geometry");
        EGS_BaseGeometry *b;
        if (ig) {
            b = EGS_BaseGeometry::createSingleGeometry(ig);
            delete ig;
            if (!b) {
                egsWarning("createGeometry(lattice): incorrect base geometry definition\n");
                delete i;
                return 0;
            }
        }
        else {
            string bgname;
            err = i->getInput("base geometry",bgname);
            delete i;
            if (err) {
                egsWarning("createGeometry(lattice): missing/incorrect 'base geometry' input\n");
                return 0;
            }
            b = EGS_BaseGeometry::getGeometry(bgname);
            if (!b) {
                egsWarning("createGeometry(lattice): no geometry with name %s defined\n",
                           bgname.c_str());
                return 0;
            }
        }

        // Get subgeometry
        i = input->getInputItem("subgeometry");
        if (!i) {
            egsWarning("createGeometry(lattice): subgeometry must be defined\n"
                       " using 'subgeometry = some_geom'?\n");
            return 0;
        }
        ig = i->getInputItem("geometry");
        EGS_BaseGeometry *s;
        if (ig) {
            s = EGS_BaseGeometry::createSingleGeometry(ig);
            delete ig;
            if (!s) {
                egsWarning("createGeometry(lattice): incorrect subgeometry definition\n");
                delete i;
                return 0;
            }
        }
        else {
            string bgname;
            err = i->getInput("subgeometry",bgname);
            delete i;
            if (err) {
                egsWarning("createGeometry(lattice): missing/incorrect 'subgeometry' input\n");
                return 0;
            }
            s = EGS_BaseGeometry::getGeometry(bgname);
            if (!s) {
                egsWarning("createGeometry(lattice): no geometry with name %s defined\n",
                           bgname.c_str());
                return 0;
            }
        }

        // Get base region in which to place subgeometries
        int ind = -1;
        i = input->getInputItem("subgeometry index");
        if (!i) {
            egsWarning("createGeometry(lattice): subgeometry index must be defined\n"
                       "  using 'subgeometry index = some_index'\n");
            return 0;
        }
        else {
            err = i->getInput("subgeometry index",ind);
            delete i;
            if (err) {
                egsWarning("createGeometry(lattice): missing/incorrect 'subgeometry index' input\n");
                return 0;
            }
            else if (ind < 0 || ind >= b->regions()) { // Not a real index
                egsWarning("createGeometry(lattice): subgeometry index %d"
                           " is not valid, must be a region in base geometry\n",ind);
                return 0;
            }
        }

        // Get one subgeometry spacing (hexagonal or cubic) or three (Bravais)
        vector<EGS_Float> space;
        err = input->getInput("spacing",space);
        if (!err) {
            for (int i=0; i < space.size(); i++) {
                if (space[i] <= 0) {
                    egsWarning("createGeometry(lattice): spacing"
                               " is not valid, spacings must be greater than zero\n");
                    return 0;
                }
            }
            if (space.size() != 1 && space.size() != 3) {
                egsWarning("createGeometry(lattice): spacing"
                           " is not valid, input either one or three"
                           "x, y, and z spacings\n");
                return 0;
            }
        }

        // Check for hexagonal lattice
        string type;
        input->getInput("type",type);

        // Final build
        EGS_BaseGeometry *result;
        if (space.size() == 3) {
            result = new EGS_Lattice(b, s, ind, space[0], space[1], space[2]);
        }
        else if (input->compare("hexagonal",type)) {
            result = new EGS_Hexagonal_Lattice(b, s, ind, space[0]);
        }
        else {
            result = new EGS_Lattice(b, s, ind, space[0], space[0], space[0]);
        }
        result->setName(input);
        return result;
    }

    void EGS_Lattice::getLabelRegions(const string &str, vector<int> &regs) {
        // labels defined in base geometry (matching indices)
        base->getLabelRegions(str, regs);
        int index = regs.size();

        // labels defined in sub geometries (shifting by base nreg)
        EGS_BaseGeometry::getLabelRegions(str, regs);
        for (; index<regs.size(); index++) {
            regs[index] += base->regions();
        }
    }

    void EGS_Hexagonal_Lattice::getLabelRegions(const string &str, vector<int> &regs) {
        // labels defined in base geometry (matching indices)
        base->getLabelRegions(str, regs);
        int index = regs.size();

        // labels defined in sub geometries (shifting by base nreg)
        EGS_BaseGeometry::getLabelRegions(str, regs);
        for (; index<regs.size(); index++) {
            regs[index] += base->regions();
        }
    }
}