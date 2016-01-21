/*
###############################################################################
#
#  EGSnrc egs++ iplanes geometry
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
#
###############################################################################
*/


/*! \file egs_iplanes.cpp
 *  \brief Intersecting planes: implementation
 *  \IK
 */

#include "egs_iplanes.h"
#include "egs_input.h"
#include "egs_transformations.h"
#include "egs_functions.h"
#include "egs_math.h"

#include <vector>

using namespace std;

string EGS_IPlanes::type = "EGS_IPlanes";
string EGS_RadialRepeater::type = "EGS_RadialRepeater";

EGS_IPlanes::EGS_IPlanes(const EGS_Vector &Xo, const EGS_Vector &A, int np,
                         const EGS_Float *angles, const string &Name, bool degree) :
    EGS_BaseGeometry(Name), xo(Xo), axis(A) {
    nreg = 2*np;
    a = new EGS_Vector[nreg];
    d = new EGS_Float[nreg];
    int j;
    EGS_Float phi180 = M_PI;
    if (degree) {
        phi180 = 180;
    }
    EGS_RotationMatrix R(axis);
    for (j=0; j<nreg; j++) {
        EGS_Float phi;
        if (j < np) {
            phi = angles[j];
        }
        else {
            phi = angles[j-np] + phi180;
        }
        if (degree) {
            phi *= (M_PI/180);
        }
        EGS_Float cphi = cos(phi), sphi = sin(phi);
        a[j] = EGS_Vector(-sphi,cphi,0)*R;
        d[j] = a[j]*xo;
    }
}

EGS_IPlanes::EGS_IPlanes(const EGS_Vector &Xo, const EGS_Vector &A,
                         int np, const EGS_Vector *aj, const EGS_Float *dj,
                         const string &Name) : EGS_BaseGeometry(Name), xo(Xo), axis(A) {
    nreg = np;
    a = new EGS_Vector[nreg];
    d = new EGS_Float[nreg];
    int j;
    for (j=0; j<nreg; j++) {
        a[j] = aj[j];
        d[j] = dj[j];
    }
}

EGS_IPlanes::EGS_IPlanes(const EGS_Vector &Xo, const EGS_Vector &A,
                         int np, EGS_Float first, const string &Name) :
    EGS_BaseGeometry(Name), xo(Xo), axis(A) {
    nreg = np;
    EGS_Float dphi = 2*M_PI/nreg;
    EGS_RotationMatrix R(axis);
    a = new EGS_Vector[nreg];
    d = new EGS_Float[nreg];
    for (int j=0; j<nreg; j++) {
        EGS_Float phi = first + dphi*j;
        EGS_Float cphi = cos(phi), sphi = sin(phi);
        a[j] = EGS_Vector(-sphi,cphi,0)*R;
        d[j] = a[j]*xo;
    }
}

EGS_IPlanes::~EGS_IPlanes() {
    delete [] a;
    delete [] d;
}

int EGS_IPlanes::isWhere(const EGS_Vector &x) {
    EGS_Float aux_old = a[0]*x;
    for (int j=1; j<nreg; j++) {
        EGS_Float aux = a[j]*x;
        if (aux_old >= d[j-1] && aux < d[j]) {
            return j-1;
        }
        aux_old = aux;
    }
    return nreg-1;
}

int EGS_IPlanes::inside(const EGS_Vector &x) {
    EGS_Float aux_old = a[0]*x;
    for (int j=1; j<nreg; j++) {
        EGS_Float aux = a[j]*x;
        if (aux_old >= d[j-1] && aux < d[j]) {
            return j-1;
        }
        aux_old = aux;
    }
    return nreg-1;
}

int EGS_IPlanes::howfar(int ireg, const EGS_Vector &x,
                        const EGS_Vector &u, EGS_Float &t, int *newmed, EGS_Vector *normal) {
    if (ireg < 0) {
        egsFatal("EGS_IPlanes::howfar: ireg (%d) can not be negative\n",ireg);
    }
    EGS_Float t1 = t, t2 = t;
    EGS_Float up = a[ireg]*u;
    int inew = ireg;
    if (up < 0) {
        t1 = (d[ireg] - a[ireg]*x)/up;
        if (t1 <= t) {
            t = t1;
            inew = ireg-1;
            if (inew < 0) {
                inew = nreg-1;
            }
            if (newmed) {
                *newmed = medium(inew);
            }
            if (normal) {
                *normal = a[ireg];
            }
        }
    }
    int j = ireg+1;
    if (j >= nreg) {
        j = 0;
    }
    up = a[j]*u;
    if (up > 0) {
        t2 = (d[j] - a[j]*x)/up;
        if (t2 < t) {
            t = t2;
            inew = j;
            if (newmed) {
                *newmed = medium(inew);
            }
            if (normal) {
                *normal = a[j]*(-1);
            }
        }
    }
    return inew;
}

EGS_Float EGS_IPlanes::hownear(int ireg, const EGS_Vector &x) {
    if (ireg < 0) {
        egsFatal("EGS_IPlanes::hownear: ireg (%d) can not be negative\n",ireg);
    }
    EGS_Float t1 = a[ireg]*x - d[ireg];
    int j = ireg+1;
    if (j >= nreg) {
        j = 0;
    }
    EGS_Float t2 = d[j] - a[j]*x;
    return min(t1,t2);
}

void EGS_IPlanes::printInfo() const {
    EGS_BaseGeometry::printInfo();
    egsInformation("  axis: Xo = (%g,%g,%g) a = (%g,%g,%g)\n",xo.x,xo.y,xo.z,
                   axis.x,axis.y,axis.z);
    egsInformation(
        "\n=======================================================\n");
}

/***************************************************************************/

EGS_RadialRepeater::EGS_RadialRepeater(const EGS_Vector &Xo,
                                       const EGS_Vector &A, int np, EGS_BaseGeometry *G,
                                       EGS_Float first, const string &Name) : EGS_BaseGeometry(Name), g(G) {
    EGS_Float dphi = 2*M_PI/np;
    iplanes = new EGS_IPlanes(Xo,A,np,first-dphi/2);
    //iplanes = new EGS_IPlanes(Xo,A,np,first);
    //iplanes = new EGS_IPlanes(Xo,A,np,(EGS_Float)0);
    g->ref();
    iplanes->ref();
    nrep = np;
    ng = g->regions();
    nreg = nrep*ng + 1;
    //EGS_Float dphi = 2*M_PI/np;
    R = new EGS_RotationMatrix [nrep];
    EGS_RotationMatrix Ro(A);
    for (int j=0; j<nrep; j++) {
        //EGS_Float phi = first + dphi*(0.5+j);
        //EGS_Float phi = dphi*(0.5+j);
        EGS_Float phi = dphi*j;
        R[j] = Ro.inverse()*EGS_RotationMatrix::rotZ(-phi)*Ro;
    }
    phi_o = first;
    xo = Xo;
}

EGS_RadialRepeater::~EGS_RadialRepeater() {
    delete [] R;
    if (!iplanes->deref()) {
        delete iplanes;
    }
    if (!g->deref()) {
        delete g;
    }
}

void EGS_RadialRepeater::printInfo() const {
    EGS_BaseGeometry::printInfo();
    egsInformation("%d uniformely rotated replicas of geometry %s with "
                   "phi_o=%g degrees\n",nrep,g->getName().c_str(),
                   phi_o*180./M_PI);
    EGS_Vector xo(iplanes->getAxisXo()), axis(iplanes->getAxisDirection());
    egsInformation("Axis of rotation is xo=(%g,%g,%g) a=(%g,%g,%g)\n",
                   xo.x,xo.y,xo.z,axis.x,axis.y,axis.z);
    const char *med_name = getMediumName(med);
    if (med_name) {
        egsInformation("space is filled with %s\n",med_name);
    }
    else {
        egsInformation("space is filled with vacuum\n");
    }
    egsInformation("Repeated geometry:\n");
    g->printInfo();
}

extern "C" {

    EGS_IPLANES_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {
        if (!input) {
            egsWarning("createGeometry(iplanes): null input?\n");
            return 0;
        }
        vector<EGS_Float> axis;
        EGS_Vector xo, a(0,0,1);
        int err = input->getInput("axis",axis);
        if (!err && axis.size() == 6) {
            xo = EGS_Vector(axis[0],axis[1],axis[2]);
            a = EGS_Vector(axis[3],axis[4],axis[5]);
            a.normalize();
        }
        else egsWarning("createGeometry(iplanes): wrong/missing axis input\n"
                            "  using Xo=(0,0,0), a=(0,0,1)\n");
        string type;
        err = input->getInput("type",type);
        if (!err && (type == "EGS_RadialRepeater" || type == "repeater")) {
            int np;
            err = input->getInput("number of repetitions",np);
            if (err || np < 2) {
                egsWarning("createGeometry(iplanes): missing/wrong "
                           "'number of repetitions' input\n");
                return 0;
            }
            string gname;
            err = input->getInput("repeated geometry",gname);
            if (err) {
                egsWarning("createGeometry(iplanes): missing 'repeated geometry'"
                           " input\n");
                return 0;
            }
            EGS_BaseGeometry *g = EGS_BaseGeometry::getGeometry(gname);
            if (!g) {
                egsWarning("createGeometry(iplanes): no geometry named %s exists\n",
                           gname.c_str());
                return 0;
            }
            EGS_Float phi_o = 0;
            EGS_Float tmp1,tmp2;
            err = input->getInput("first angle",tmp1);
            int err1 = input->getInput("first angle in radians",tmp2);
            if (!err) {
                phi_o = tmp1*M_PI/180;
            }
            else if (!err1) {
                phi_o = tmp2;
            }
            EGS_RadialRepeater *result = new EGS_RadialRepeater(xo,a,np,g,phi_o);
            result->setName(input);
            string medium;
            err = input->getInput("medium",medium);
            if (!err) {
                result->setMedium(medium);
            }
            result->setRLabels(input);
            result->setLabels(input);
            return result;
        }

        vector<EGS_Float> angles;
        err = input->getInput("angles",angles);
        bool is_degree = true;
        EGS_Float max_angle = 180;
        if (err) {
            err = input->getInput("angles in radian",angles);
            if (err) {
                egsWarning("createGeometry(iplanes): wrong/missing 'angles' or "
                           "'angles in radian' input\n");
                return 0;
            }
            is_degree = false;
            max_angle = M_PI;
        }
        EGS_Float *ang = new EGS_Float [angles.size()];
        for (int j=0; j<angles.size(); j++) {
            ang[j] = angles[j];
            /*
            if( ang[j] < 0 || ang[j] > max_angle ) {
                egsWarning("createGeometry(iplanes): angle must be between 0 and "
                    "%g\n",max_angle); delete [] ang; return 0;
            }
            */
            if (j > 0) {
                if (ang[j] <= ang[j-1]) {
                    egsWarning("createGeometry(iplanes): angles must be ordered"
                               " in increasing order\n");
                    delete [] ang;
                    return 0;
                }
            }
        }
        int nang = angles.size();
        EGS_Float ang_diff = ang[nang-1]-ang[0];
        if (ang_diff > max_angle) {
            egsWarning("createGeometry(iplanes): difference between first and last"
                       " angle must be less then 180 degrees.\n");
            if (is_degree)
                egsWarning("  Your input: first angle=%g, last angle=%g,"
                           " difference=%g\n",ang[0],ang[nang-1],ang[nang-1]-ang[0]);
            else
                egsWarning("  Your input: first angle=%g, last angle=%g,"
                           " difference=%g\n",ang[0]*180/M_PI,ang[nang-1]*180/M_PI,
                           (ang[nang-1]-ang[0])*180/M_PI);
            delete [] ang;
            return 0;
        }
        EGS_BaseGeometry *g = new EGS_IPlanes(xo,a,angles.size(),ang,"",is_degree);
        delete [] ang;
        g->setName(input);
        g->setMedia(input);
        g->setLabels(input);
        return g;
    }

    void EGS_RadialRepeater::setRLabels(EGS_Input *input) {

        // radial repeater labels
        string inp;
        int err;
        err = input->getInput("set repeater label", inp);
        if (!err) {
            iplanes->setLabels(inp);
        }
    }


    void EGS_RadialRepeater::getLabelRegions(const string &str, vector<int> &regs) {

        vector<int> local_regs;

        // label in repeated geometry
        local_regs.clear();
        g->getLabelRegions(str, local_regs);
        for (int i=0; i<nrep; i++) {
            for (int r=0; r<local_regs.size(); r++) {
                regs.push_back(ng*i + local_regs[r]);
            }
        }

        // labels defined in iplanes
        local_regs.clear();
        iplanes->getLabelRegions(str, local_regs);
        for (int i=0; i<local_regs.size(); i++) {
            for (int r=0; r<ng; r++) {
                regs.push_back(ng*local_regs[i] + r);
            }
        }

        // label defined in self (repeater input block)
        EGS_BaseGeometry::getLabelRegions(str, regs);

    }

}
