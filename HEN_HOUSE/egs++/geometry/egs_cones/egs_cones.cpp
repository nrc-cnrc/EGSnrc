/*
###############################################################################
#
#  EGSnrc egs++ cones geometry
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
#                   Randle Taylor
#                   Ernesto Mainegra-Hing
#
###############################################################################
*/


/*! \file egs_cones.cpp
 *  \brief Various cone geometries: implementation
 *  \IK
 */

#include "egs_cones.h"
#include "egs_input.h"

#include <vector>
using std::vector;

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

string EGS_SimpleCone::type = "EGS_SimpleCone";
string EGS_ParallelCones::type = "EGS_ParallelCones";
string EGS_ConeSet::type = "EGS_ConeSet";
string EGS_ConeStack::type = "EGS_ConeStack";

void EGS_ConeStack::clear(bool all) {
    if (nltot > 0) {
        if (all) {
            for (int j=0; j<nl; j++) {
                for (int i=0; i<nr[j]; i++)
                    if (!cones[j][i]->deref()) {
                        delete cones[j][i];
                    }
                delete [] cones[j];
            }
            nl = 0;
        }
        delete [] cones;
        delete [] pos;
        delete [] nr;
        delete [] flag;
        nltot = 0;
    }
}

#define N_CS_GROW 50
void EGS_ConeStack::resize() {
    int nnew = nltot + N_CS_GROW;
    int *new_nr = new int [nnew], *new_flag = new int [nnew];
    EGS_Float *new_pos = new EGS_Float [nnew+1];
    EGS_SimpleCone ***new_cones = new EGS_SimpleCone **[nnew];
    for (int j=0; j<nl; j++) {
        new_nr[j] = nr[j];
        new_flag[j] = flag[j];
        new_pos[j] = pos[j];
        new_cones[j] = cones[j];
    }
    if (nl > 0) {
        new_pos[nl] = pos[nl];
    }
    clear(false);
    nr = new_nr;
    pos = new_pos;
    flag = new_flag;
    cones = new_cones;
    nltot = nnew;
}

void EGS_ConeStack::addLayer(EGS_Float thick, const vector<EGS_Float> &rtop,
                             const vector<EGS_Float> &rbottom,
                             const vector<string> &med_names) {
    int this_nr = rbottom.size();
    if (!this_nr) {
        egsWarning("EGS_ConeStack::addLayer: no bottom radii?\n");
        egsWarning("  --> ignoring layer\n");
        return;
    }
    if ((int)med_names.size() != this_nr) {
        egsWarning("EGS_ConeStack::addLayer: number of cone radii (%d) is"
                   " different from number of media (%d)\n",this_nr,med_names.size());
        egsWarning("  --> ignoting layer\n");
        return;
    }
    bool same_radii = false;
    if (rtop.size() != rbottom.size()) {
        if (!rtop.size()) {
            if (!nl) {
                egsWarning("EGS_ConeStack::addLayer: zero top radii does not"
                           " work for the first layer\n");
                egsWarning("  --> ignoting layer\n");
                return;
            }
            same_radii = true;
        }
        else {
            egsWarning("EGS_ConeStack::addLayer: number of bottom radii (%d)"
                       " is different from number of top radii (%d)\n",this_nr,
                       rtop.size());
            egsWarning("  --> ignoting layer\n");
            return;
        }
    }
    if (nl >= nltot) {
        resize();
    }
    if (!nl) {
        pos[nl] = xo*a;
        Rout = rbottom[this_nr-1];
        Rout2 = Rout*Rout;
        if (fabs(rtop[this_nr-1]-Rout) > boundaryTolerance) {
            same_Rout = false;
            is_convex = false;
        }
    }

    cones[nl] = new EGS_SimpleCone * [this_nr];
    nr[nl] = this_nr;
    pos[nl+1] = pos[nl] + thick;
    EGS_Vector x(xo+a*(pos[nl]-pos[0]));
    //egsInformation(" layer %d x = (%g,%g,%g)\n",nl,x.x,x.y,x.z);
    for (int ir=0; ir<this_nr; ir++) {
        EGS_Float Rtop = same_radii ? cones[nl-1][ir]->getRadius(x) : rtop[ir];
        cones[nl][ir] = new EGS_SimpleCone(x,a,thick,Rtop,rbottom[ir]);
        cones[nl][ir]->setMedium(med_names[ir]);
        cones[nl][ir]->ref();
        if (ir == this_nr-1) {
            if (fabs(rbottom[this_nr-1]-Rout) > boundaryTolerance) {
                same_Rout = false;
                is_convex = false;
            }
            if (fabs(Rtop-Rout) > boundaryTolerance) {
                same_Rout = false;
                is_convex = false;
            }
        }
    }
    if (same_radii) {
        flag[nl-1] += 2;
        flag[nl] = 1;
    }
    else {
        flag[nl] = 0;
    }
    if (this_nr > nmax) {
        nmax = this_nr;
    }
    ++nl;
    nreg = nl*nmax;
    //egsInformation("addLayer: the following layers are defined:\n");
    //for(int j=0; j<nl; j++) egsInformation("  %g %g %d %d\n",pos[j],pos[j+1],
    //        nr[j],flag[j]);
}

void EGS_ConeStack::printInfo() const {
    EGS_BaseGeometry::printInfo();
    egsInformation("number of layers: %d\n",nl);
    for (int il=0; il<nl; il++) {
        egsInformation("*** layer %d: top = %g bottom = %g\n",il,
                       pos[il],pos[il+1]);
        egsInformation("    top radii: ");
        int i;
        EGS_Vector x(xo+a*(pos[il]-pos[0]));
        for (i=0; i<nr[il]; i++) {
            egsInformation("%g ",cones[il][i]->getRadius(x));
        }
        egsInformation("\n    bottom radii: ");
        x = xo + a*(pos[il+1]-pos[0]);
        for (i=0; i<nr[il]; i++) {
            egsInformation("%g ",cones[il][i]->getRadius(x));
        }
        egsInformation("\n    media: ");
        for (i=0; i<nr[il]; i++) {
            egsInformation("%d ",cones[il][i]->medium(0));
        }
        egsInformation("\n");
    }
    egsInformation("=====================================================\n");
}

void EGS_SimpleCone::printInfo() const {
    EGS_BaseGeometry::printInfo();
    egsInformation(" cone apex = (%g,%g,%g)\n",xo.x,xo.y,xo.z);
    egsInformation(" cone axis = (%g,%g,%g)\n",a.x,a.y,a.z);
    egsInformation(" opening angle = %g degrees\n",180*atan(gamma)/M_PI);
    if (open) {
        egsInformation(" cone is open\n");
    }
    else {
        egsInformation(" cone height %g\n",-d1);
    }
    egsInformation("=====================================================\n");
}

void EGS_ParallelCones::printInfo() const {
    EGS_BaseGeometry::printInfo();
    egsInformation(" cone axis = (%g,%g,%g)\n",a.x,a.y,a.z);
    egsInformation(" opening angle = %g degrees\n",180*atan(gamma)/M_PI);
    egsInformation(" number of cones = %d\n",nc);
    egsInformation(" cone apexes:\n      ");
    for (int j=0; j<nc; j++) {
        egsInformation("(%g,%g,%g) ",xo[j].x,xo[j].y,xo[j].z);
    }
    egsInformation("\n=====================================================\n");
}

void EGS_ConeSet::printInfo() const {
    EGS_BaseGeometry::printInfo();
    egsInformation(" cone apex = (%g,%g,%g)\n",xo.x,xo.y,xo.z);
    egsInformation(" cone axis = (%g,%g,%g)\n",a.x,a.y,a.z);
    egsInformation(" flag = %d\n",flag);
    egsInformation(" opening angles (degrees)=\n    ");
    for (int j=0; j<nc; j++) {
        egsInformation("%g ",180*atan(gamma[j])/M_PI);
    }
    egsInformation("\n=====================================================\n");
}


extern "C" {

    EGS_CONES_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {

        if (!input) {
            egsWarning("createGeometry(cones): null input?\n");
            return 0;
        }
        string type;
        int err = input->getInput("type",type);
        if (err) {
            egsWarning("createGeometry(cones): object type not specified\n");
            return 0;
        }
        if (type == "EGS_ConeStack") {
            vector<EGS_Float> axis;
            err = input->getInput("axis",axis);
            if (err) {
                egsWarning("createGeometry(EGS_ConeStack): no axis input\n");
                return 0;
            }
            if (axis.size() != 6) {
                egsWarning("createGeometry(EGS_ConeStack): wrong axis input"
                           " (expecting 6 instead of %d inputs)\n",axis.size());
                return 0;
            }
            EGS_ConeStack *g = new EGS_ConeStack(
                EGS_Vector(axis[0],axis[1],axis[2]),
                EGS_Vector(axis[3],axis[4],axis[5]),"");
            EGS_Input *layer;
            int nl = 0;
            vector<int> layerLabels;
            while ((layer = input->takeInputItem("layer"))) {
                vector<EGS_Float> rtop, rbottom;
                vector<string> media;
                EGS_Float thickness;
                err = layer->getInput("thickness",thickness);
                if (err)
                    egsWarning("createGeometry(EGS_ConeStack): missing 'thickness'"
                               " input for layer %d\n  --> layer ignored\n",nl);
                else {
                    err = layer->getInput("top radii",rtop);
                    err = layer->getInput("bottom radii",rbottom);
                    if (err) egsWarning("createGeometry(EGS_ConeStack): missing "
                                            "'bottom radii' input for layer %d\n",nl);
                    int err1 = layer->getInput("media",media);
                    if (err1) egsWarning("createGeometry(EGS_ConeStack): missing "
                                             "'media' input for layer %d\n",nl);
                    if (err || err1) {
                        egsWarning("  --> layer ignored\n");
                    }
                    else {
                        g->addLayer(thickness,rtop,rbottom,media);
                    }
                }
                layerLabels.push_back(g->setLabels(layer));
                delete layer;
                ++nl;
            }
            if (!g->nLayer()) {
                egsWarning("createGeometry(EGS_ConeStack): zero layers\n");
                delete g;
                return 0;
            }

            // adjust lable region numbering in each layer
            int count=0;
            for (size_t i=0; i<layerLabels.size(); i++) {
                for (int j=0; j<layerLabels[i]; j++) {
                    g->shiftLabelRegions(count,i);
                    count++;
                }
            }

            g->setName(input);
            g->setBoundaryTolerance(input);
            g->setLabels(input);
            g->setBScaling(input);  // Perhaps add density scaling as well?
            return g;
        }

        // get cone apex position
        vector<EGS_Float> tmp;
        EGS_Vector Xo;
        err = input->getInput("apex",tmp);
        if (err || tmp.size() != 3)
            egsWarning("createGeometry(cones): no 'apex' input, "
                       "assuming (0,0,0)\n");
        else {
            Xo.x = tmp[0];
            Xo.y = tmp[1];
            Xo.z = tmp[2];
        }
        tmp.clear();

        // get cone axis
        EGS_Vector axis(0,0,1);
        err = input->getInput("axis",tmp);
        if (err || tmp.size() != 3)
            egsWarning("createGeometry(cones): no 'axis' input, assuming "
                       "(0,0,1)\n");
        else {
            axis.x = tmp[0];
            axis.y = tmp[1];
            axis.z = tmp[2];
        }

        EGS_BaseGeometry *g;
        if (input->compare(type,"EGS_ConeSet")) {
            vector<EGS_Float> angles;
            err = input->getInput("opening angles",angles);
            bool is_radian = false;
            if (err) {
                angles.clear();
                err = input->getInput("opening angles in radian",angles);
                if (err) {
                    egsWarning("createGeometry(cones): no 'opening angles' or "
                               "'opening angles in radian' input\n");
                    return 0;
                }
                is_radian = true;
            }
            int flag = 0;
            err = input->getInput("flag",flag);
            int nc = angles.size();
            EGS_Float *gamma = new EGS_Float [nc];
            for (int j=0; j<nc; j++) {
                if (angles[j] <= 0) {
                    egsWarning("createGeometry(cones): opening angles must be"
                               " positive\n");
                    delete [] gamma;
                    return 0;
                }
                EGS_Float a = angles[j];
                if (!is_radian) {
                    a *= (M_PI/180);
                }
                if (a >= M_PI/2) {
                    egsWarning("createGeometry(cones): opening angles can not be"
                               " greater than Pi/2\n");
                    delete [] gamma;
                    return 0;
                }
                if (j > 0) {
                    if (angles[j] <= angles[j-1]) {
                        egsWarning("createGeometry(cones): opening angles must be"
                                   " in increasing order\n");
                        delete [] gamma;
                        return 0;
                    }
                }
                gamma[j] = tan(a);
            }
            g = new EGS_ConeSet(Xo,axis,nc,gamma,flag);
            delete [] gamma;
        }

        else {
            // get opening angle
            EGS_Float angle;
            err = input->getInput("opening angle",angle);
            if (err) {
                err = input->getInput("opening angle in radian",angle);
                if (err) {
                    egsWarning("createGeometry(cones): no 'opening angle' or "
                               "'opening angle in radian' input\n");
                    return 0;
                }
            }
            else {
                angle *= (M_PI/180);
            }
            if (angle >= M_PI/2) {
                egsWarning("createGeometry(cones): it is not allowed to have"
                           " an opening angle greater than Pi/2, your input was %g\n",angle);
                return 0;
            }
            EGS_Float gamma = tan(angle);
            if (input->compare(type,"EGS_SimpleCone")) {
                EGS_Float height;
                EGS_Float *h = 0;
                err = input->getInput("height",height);
                if (!err) {
                    if (height <= 0)
                        egsWarning("createGeometry(cones): the cone height"
                                   " must be greater than zero, your input was %g\n",height);
                    else {
                        h = &height;
                    }
                }
                g = new EGS_SimpleCone(Xo,axis,gamma,h,"");
            }
            else if (input->compare(type,"EGS_ParallelCones")) {
                vector<EGS_Float> d;
                err = input->getInput("apex distances",d);
                EGS_Float *dist=0;
                int nc=1;
                if (!err && d.size() > 0) {
                    dist = new EGS_Float [d.size()];
                    for (size_t j=0; j<d.size(); j++) {
                        dist[j] = d[j];
                    }
                    nc = 1 + d.size();
                }
                g = new EGS_ParallelCones(nc,Xo,axis,gamma,dist);
                if (dist) {
                    delete [] dist;
                }
            }
            else {
                egsWarning("createGeometry(cones): unknown object type %s\n",
                           type.c_str());
                return 0;
            }
        }

        g->setName(input);
        g->setMedia(input);
        g->setLabels(input);
        return g;
    }

}
