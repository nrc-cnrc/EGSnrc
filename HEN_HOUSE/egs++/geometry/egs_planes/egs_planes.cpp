/*
###############################################################################
#
#  EGSnrc egs++ planes geometry
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
#                   Reid Townson
#
###############################################################################
*/


/*! \file egs_planes.cpp
 *  \brief Sets of parallel planes and a plane collection
 *  \IK
 */


#include "egs_planes.h"
#include "egs_input.h"

//string XProjector::type = "EGS_Xplanes";
//string YProjector::type = "EGS_Yplanes";
//string ZProjector::type = "EGS_Zplanes";
//string Projector::type = "EGS_Planes";
const string EGS_PLANES_LOCAL xproj_type("EGS_Xplanes");
const string EGS_PLANES_LOCAL yproj_type("EGS_Yplanes");
const string EGS_PLANES_LOCAL zproj_type("EGS_Zplanes");
const string EGS_PLANES_LOCAL proj_type("EGS_Planes");

string EGS_PlaneCollection::type = "EGS_PlaneCollection";

EGS_PlaneCollection::EGS_PlaneCollection(int Np, const EGS_Float *pos,
        const EGS_Vector *norm, const string &Name) : EGS_BaseGeometry(Name) {
    if (Np < 2) egsFatal("EGS_PlaneCollection::EGS_PlaneCollection: "
                             " you nead at least 2 planes\n");
    np = Np;
    nreg=np-1;
    planes = new EGS_Planes* [np];
    for (int j=0; j<np; j++) {
        planes[j] = new EGS_Planes(1,&pos[j],"",
                                   EGS_Projector(norm[j],proj_type));
        planes[j]->ref();
    }
}

EGS_PlaneCollection::~EGS_PlaneCollection() {
    //egsWarning("Deleting ~EGS_PlaneCollection at 0x%x\n",this);
    for (int j=0; j<np; j++) if (!planes[j]->deref()) {
            delete planes[j];
        }
    delete [] planes;
}

void EGS_PlaneCollection::printInfo() const {
    EGS_BaseGeometry::printInfo();
    int j;
    egsInformation("  plane positions: ");
    for (j=0; j<np; j++) {
        egsInformation("  %g ",planes[j]->position(0));
    }
    egsInformation("  \nplane normals: ");
    for (j=0; j<np; j++) {
        EGS_Vector a = planes[j]->normal();
        egsInformation("  (%g,%g,%g) ",a.x,a.y,a.z);
    }
    egsInformation("\n=====================================================\n");
}

extern "C" {

    EGS_PLANES_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {
        string type;
        int err = input->getInput("type",type);
        if (err) {
            egsWarning("createGeometry(planes): missing type key\n");
            return 0;
        }
        EGS_Float first;
        vector<EGS_Float> thick;
        vector<int> nthick;
        vector<EGS_Float> pos;
        int err_first = input->getInput("first plane",first);
        int err_thick = input->getInput("slab thickness",thick);
        int err_nthick = input->getInput("number of slabs",nthick);
        if (!err_first && !err_thick && !err_nthick) {
            if (thick.size() != nthick.size() || !thick.size()) {
                egsWarning("createGeometry(planes): number of 'slab thickness' and"
                           "\n    'number of slabs' inputs must be the same and not zero\n");
                egsWarning("  got %d and %d inputs --> input ignored\n",
                           thick.size(),nthick.size());
            }
            else {
                pos.push_back(first);
                int j=0;
                for (int i=0; i<thick.size(); i++) {
                    for (int l=0; l<nthick[i]; ++l) {
                        pos.push_back(pos[j++]+thick[i]);
                    }
                }
            }
        }
        if (!pos.size()) {
            err = input->getInput("positions",pos);
            if (err) {
                egsWarning("createGeometry(planes): missing/wrong 'positions' "
                           "input and missing/wrong multiple plane input\n");
                return 0;
            }
        }
        EGS_BaseGeometry *g;
        if (type == "EGS_Xplanes") g = new EGS_PlanesX(pos,"",
                    EGS_XProjector(xproj_type));
        else if (type == "EGS_Yplanes") g = new EGS_PlanesY(pos,"",
                    EGS_YProjector(yproj_type));
        else if (type == "EGS_Zplanes") g = new EGS_PlanesZ(pos,"",
                    EGS_ZProjector(zproj_type));
        else if (type == "EGS_Planes") {
            vector<EGS_Float> a;
            err = input->getInput("normal",a);
            if (err || a.size() != 3) {
                egsWarning("createGeometry(planes): missing/wrong normal input\n");
                return 0;
            }
            g = new EGS_Planes(pos,"",EGS_Projector(EGS_Vector(a[0],a[1],a[2]),
                                                    proj_type));
        }
        else if (type == "EGS_PlaneCollection") {
            vector<EGS_Float> a;
            err = input->getInput("normals",a);
            if (err || a.size() < 6) {
                egsWarning("createGeometry(planes): missing/wrong normal input\n");
                return 0;
            }
            int np = a.size()/3;
            if (np != pos.size()) {
                egsWarning("createGeometry(planes): number of plane normals (%d)\n"
                           " is not the same as number of plane positions (%d) for a"
                           " plane collection\n",np,pos.size());
                return 0;
            }
            EGS_Float *p = new EGS_Float [np];
            EGS_Vector *normal = new EGS_Vector [np];
            for (int j=0; j<np; j++) {
                p[j] = pos[j];
                normal[j] = EGS_Vector(a[3*j],a[3*j+1],a[3*j+2]);
            }
            g = new EGS_PlaneCollection(np,p,normal);
        }
        else {
            egsWarning("createGeometry(planes): unknown type %s\n",type.c_str());
            return 0;
        }
        g->setName(input);
        g->setBoundaryTolerance(input);
        g->setMedia(input);
        g->setLabels(input);
        return g;
    }

}
