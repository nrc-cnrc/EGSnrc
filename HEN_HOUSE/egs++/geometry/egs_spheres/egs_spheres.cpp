/*
###############################################################################
#
#  EGSnrc egs++ spheres geometry
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
#  Contributors:    Ernesto Mainegra-Hing
#                   Frederic Tessier
#                   Reid Townson
#                   Randle Taylor
#
###############################################################################
*/


/*! \file egs_spheres.cpp
 *  \brief A set of concentric spheres
 *  \author Declan Persram and Iwan Kawrakow
 */

#include "egs_spheres.h"
#include "egs_input.h"
#include "egs_functions.h"

#include <vector>
using std::vector;

string EGS_cSpheres::type = "EGS_cSpheres";

// generate the concentric spheres
EGS_cSpheres::EGS_cSpheres(int ns, const EGS_Float *radius,
                           const EGS_Vector &position, const string &Name) :
    EGS_BaseGeometry(Name), xo(position) {

    if (ns>0) {

        R2=new EGS_Float [ns];
        R=new EGS_Float [ns];

        // ... and sphere radii
        for (int i=0; i<ns; i++) {
            R2[i]=radius[i]*radius[i];
            R[i]=radius[i];
        }

        // for n-concentric spheres, we have n separate regions
        nreg=ns;
    }

    for (int ireg=0; ireg < ns; ireg++) {
        rbounds.push_back(radius[ireg]);
        EGS_Float router = rbounds[ireg];
        EGS_Float rinner = ireg > 0 ? rbounds[ireg-1] : 0;
        EGS_Float volume = (4./3.)*M_PI*(router*router*router - rinner*rinner*rinner);
        mass.push_back(getRelativeRho(ireg) * volume);
    }
}

bool EGS_cSpheres::isInside(const EGS_Vector &x) {
    EGS_Vector tmp(x-xo);
    EGS_Float r_sq=tmp.length2();
    if (r_sq>R2[nreg-1]) {
        return false;
    }
    return true;
}

int EGS_cSpheres::isWhere(const EGS_Vector &x) {
    EGS_Vector tmp(x-xo);
    EGS_Float r_sq=tmp.length2();
    if (r_sq>R2[nreg-1]) {
        return -1;
    }
    if (r_sq<R2[0]) {
        return 0;
    }
    return findRegion(r_sq,nreg-1,R2)+1;
}

// method to determine which spheres we are in(between)
int EGS_cSpheres::inside(const EGS_Vector &x) {

    EGS_Vector tmp(x-xo);
    EGS_Float r_sq=tmp.length2();

    // are we outside off all spheres? If so return that region number
    if (r_sq>R2[nreg-1]) {
        return -1;
    }

    /*
     * algorithm below fails for particle in central sphere ...ugh!
     */if (r_sq<R2[0]) {
        return 0;
    }

    // search for region containing particle
    int is=0,os=nreg,ms;
    while (os-is>1) {
        ms=(is+os)/2;
        if (r_sq<=R2[ms]) {
            os=ms;
        }
        else {
            is=ms;
        }
    }
    return os;
}

// howfar is particle trajectory from sphere boundary
/* note that in general we will be between two spheres (if inside a sphere at
 * all... so we need to check if the flight path will intersect the inner or
 * outer sphere
 */

#ifdef SPHERES_DEBUG
    EGS_Vector last_x, last_u;
    int last_ireg;
    EGS_Float last_d,last_t,last_aa,last_bb2,last_R2b2,last_tmp;
#endif

EGS_Float EGS_cSpheres::howfarToOutside(int ireg, const EGS_Vector &x,
                                        const EGS_Vector &u) {
    if (ireg < 0) {
        return 0;
    }
    EGS_Vector xp(x - xo);
    EGS_Float aa = xp*u, aa2 = aa*aa;
    EGS_Float bb2 = xp.length2();
    EGS_Float R2b2 = R2[nreg-1] - bb2;
    if (R2b2 <= 0) {
        return 0;    // outside within precision
    }
    EGS_Float tmp = sqrt(aa2 + R2b2);
    return aa > 0 ? R2b2/(tmp + aa) : tmp - aa;
}

int EGS_cSpheres::howfar(int ireg, const EGS_Vector &x,
                         const EGS_Vector &u, EGS_Float &t, int *newmed, EGS_Vector *normal) {
    int direction_flag=-1;  /* keep track of direction entering or exiting a
                             sphere boundary */
    double d=veryFar*1e5;   // set a maximum distance from a boundary

    EGS_Vector xp(x - xo);
    double aa = xp*u, aa2 = aa*aa;
    double bb2 = xp.length2();

    double rad=0, R2b2, tmp;

    // check if we are inside of any regions at all? ...
    if (ireg>=0) {

        /* check if particle is moving towards or away from the sphere(s) centre.
         * we loose here if the particle is in the centre sphere as we don't need
         * to check this condition. see next 'if' statement
         */
        if (aa >= 0 || !ireg) {

            /* ie. particle moving away from center of
               spheres, OR it is IN the innermost sphere
               => we must check the outer sphere only
             */
            R2b2 = R2[ireg] - bb2;
            if (R2b2 <= 0 && aa > 0) {
                d = halfBoundaryTolerance;    // hopefully a truncation problem
            }
            else {
                tmp = aa2 + R2b2;
                if (tmp > 0) {
                    tmp = sqrt(tmp);
                }
                else {
                    if (tmp < -boundaryTolerance) {
                        egsWarning("EGS_cSpheres::howfar: something is wrong\n");
                        egsWarning("  we think we are in region %d, but R2b2=%g",
                                   ireg,R2b2);
                    }
                    tmp = 0;
                }
                d = aa > 0 ? R2b2/(tmp + aa) : tmp - aa;
                // the above reduces roundoff, which is significant
                // when aa2 is large compared to R2b2 and aa>0
            }
            rad = -R[ireg];
            direction_flag=ireg+1;
            if (direction_flag >= nreg) {
                direction_flag = -1;
            }
        }
        else {

            /* so now we know the particle is moving towards the centre of the
             * spheres.  check to see if its trajectory will intersect the nested
             * sphere - we are guaranteed there is one - we checked that already!
             */
            R2b2 = R2[ireg-1] - bb2;
            tmp = aa2 + R2b2;
            if (tmp <= 0) {   // we will not intersect the nested sphere
                R2b2 = R2[ireg] - bb2;
                tmp = aa2 + R2b2;
                if (tmp > 0) {
                    d = sqrt(tmp) - aa;
                }
                else {
                    d = -aa;
                }
                rad = -R[ireg];
                direction_flag=ireg+1;
                if (direction_flag >= nreg) {
                    direction_flag = -1;
                }
            }
            else {
                // we're hitting the inner sphere (from the outside)
                tmp = sqrt(tmp);
                d = -R2b2/(tmp - aa);
                direction_flag=ireg-1;
                rad = R[direction_flag];
            }
        }
    }
    else {
        // we are not inside any of the spherical regions of interest
        if (aa<0) { // we _might_ intersect the largest sphere
            R2b2 = R2[nreg-1] - bb2;
            tmp = aa2 + R2b2;
            if (tmp > 0) {  // we *will* intersect the largest sphere
                d = -R2b2/(sqrt(tmp) - aa);
                direction_flag=nreg-1;
                rad = R[direction_flag];
            }
        }
    }

#ifdef SPHERES_DEBUG
    if (isnan(d)) {
        egsWarning("\nGot nan\n");
    }

    if (d < -boundaryTolerance) {
        egsWarning("\nNegative step?: %g\n",d);
        egsWarning("ireg=%d inew=%d aa=%g bb2=%g\n",ireg,direction_flag,aa,bb2);
        //exit(1);
    }

    last_x = x;
    last_u = u;
    last_ireg = ireg;
    last_d = d;
    last_t = t;
    last_aa = aa;
    last_bb2 = bb2;
    last_R2b2 = R2b2;
    last_tmp = tmp;
#endif

    // check desired step size against this d
    if (d<=t) {
        t=d;
        if (newmed) {
            if (direction_flag >= 0) {
                *newmed = medium(direction_flag);
            }
            else {
                *newmed = -1;
            }
        }
        if (normal) {
            EGS_Vector n(xp + u*d);
            *normal = n*(1/rad);
        }
        return direction_flag;
    }
    return ireg;
}

// hownear - closest perpendicular distance to sphere surface
EGS_Float EGS_cSpheres::hownear(int ireg, const EGS_Vector &x) {
    EGS_Vector xp(x-xo);
    EGS_Float r=xp.length();
    //EGS_Float r_sq=x.x*x.x+x.y*x.y+x.z*x.z;
    EGS_Float d;

    if (ireg>=0) {
        d=R[ireg]-r;
        if (ireg) {
            EGS_Float dd=r-R[ireg-1];
            if (dd<d) {
                d=dd;
            }
        }
    }
    else {
        d=r-R[nreg-1];
    }

    return d;
}

void EGS_cSpheres::printInfo() const {
    EGS_BaseGeometry::printInfo();
    egsInformation(" midpoint of spheres = (%g,%g,%g)\n",xo.x,xo.y,xo.z);
    egsInformation(" sphere radii = ");
    for (int j=0; j<nreg; j++) {
        egsInformation("%g ",R[j]);
    }
    egsInformation(
        "\n=======================================================\n");
}

EGS_Float EGS_cSpheres::getBound(int idir, int ind) {
    if (idir == RDIR && ind ==0) {
        return 0.;
    }
    else if (idir == RDIR && ind>0 && ind <= nreg) {
        return rbounds[ind-1];
    }
    return EGS_BaseGeometry::getBound(idir, ind);
}


int EGS_cSpheres::getNRegDir(int dir) {
    if (dir == RDIR) {
        return nreg;
    }
    return EGS_BaseGeometry::getNRegDir(dir);
}


EGS_Float EGS_cSpheres::getMass(int ireg) {
    if (ireg >= 0 && ireg < nreg) {
        return mass[ireg];
    }
    return 1;
}

/********************** Shell *************************************/

string EGS_cSphericalShell::type = "EGS_cSphericalShell";

// generate the concentric spheres
EGS_cSphericalShell::EGS_cSphericalShell(int ns, const EGS_Float *radius,
        const EGS_Vector &position, const string &Name) :
    EGS_BaseGeometry(Name), xo(position) {

    is_convex = false;

    if (ns>0) {

        R2 = new EGS_Float [ns];
        R = new EGS_Float [ns];

        for (int i=0; i<ns; i++) {
            R2[i] = radius[i]*radius[i];
            R[i] = radius[i];
        }

        // for n-concentric spheres (with hollow centre), we have n - 1 separate regions
        nreg = ns - 1;
    }

    for (int ireg=0; ireg < nreg; ireg++) {
        EGS_Float rinner = R[ireg];
        EGS_Float router = R[ireg + 1];
        EGS_Float volume = (4./3.)*M_PI*(router*router*router - rinner*rinner*rinner);
        mass.push_back(getRelativeRho(ireg) * volume);
    }
}

bool EGS_cSphericalShell::isInside(const EGS_Vector &x) {
    EGS_Float r_sq = (x - xo).length2();
    return (r_sq >= R2[0]) && (r_sq <= R2[nreg]);
}

int EGS_cSphericalShell::isWhere(const EGS_Vector &x) {

    EGS_Float r_sq = (x - xo).length2();

    if ((r_sq < R2[0]) || (r_sq > R2[nreg])) {
        return -1;
    }
    return findRegion(r_sq, nreg, R2);
}

// method to determine which spheres we are in(between)
int EGS_cSphericalShell::inside(const EGS_Vector &x) {

    EGS_Vector tmp(x-xo);
    EGS_Float r_sq=tmp.length2();

    // are we outside off all spheres? If so return that region number
    if (r_sq > R2[nreg] || r_sq < R2[0]) {
        return -1;
    }

    for (int shell = 1; shell < nreg + 1; shell++) {
        if (r_sq <= R2[shell]) {
            return shell - 1;
        }
    }

    return -1;

}

EGS_Float EGS_cSphericalShell::howfarToOutside(int ireg, const EGS_Vector &x,
        const EGS_Vector &u) {
    if (ireg < 0) {
        return 0;
    }
    EGS_Vector xp(x - xo);
    EGS_Float aa = xp*u;
    EGS_Float aa2 = aa*aa;
    EGS_Float bb2 = xp.length2();
    EGS_Float R2b2 = R2[nreg] - bb2;
    EGS_Float R2b2in = R2[0] - bb2;
    if (R2b2in <= 0 || R2b2 <= 0) {
        return 0;    // outside within precision
    }

    EGS_Float d, tmp;
    if (aa >= 0) {

        /* ie. particle moving away from center of
            spheres we must check the outer sphere only
            */
        if (R2b2 <= 0 && aa > 0) {
            d = 1e-15;    // hopefully a truncation problem
        }
        else {
            tmp = aa2 + R2b2;
            if (tmp > 0) {
                tmp = sqrt(tmp);
            }
            else {
                if (tmp < -1e-2) {
                    egsWarning("EGS_cSphericalShell::howfarToOutside: something is wrong\n");
                    egsWarning("  we think we are in region %d, but R2b2=%g", ireg,R2b2);
                }
                tmp = 0;
            }
            d = aa > 0 ? R2b2/(tmp + aa) : tmp - aa;
        }
    }
    else {

        R2b2 = R2[0] - bb2;
        tmp = aa2 + R2b2;
        if (tmp <= 0) {   // we will not intersect the nested sphere
            R2b2 = R2[nreg] - bb2;
            tmp = aa2 + R2b2;
            if (tmp > 0) {
                d = sqrt(tmp) - aa;
            }
            else {
                d = -aa;
            }
        }
        else {
            // we're hitting the inner sphere (from the outside)
            tmp = sqrt(tmp);
            d = -R2b2/(tmp - aa);
        }

    }
    return d;

}


// howfar is particle trajectory from sphere boundary
/* note that in general we will be between two spheres (if inside a sphere at
 * all... so we need to check if the flight path will intersect the inner or
 * outer sphere
 */
int EGS_cSphericalShell::howfar(int ireg, const EGS_Vector &x,
                                const EGS_Vector &u, EGS_Float &t, int *newmed, EGS_Vector *normal) {
    int direction_flag=-1;  /* keep track of direction entering or exiting a
                             sphere boundary */
    double d=1e35;      // set a maximum distance from a boundary

    EGS_Vector xp(x - xo);
    double aa = xp*u, aa2 = aa*aa;
    double bb2 = xp.length2();

    double rad=0, R2b2, tmp;

    // check if we are inside of any regions at all? ...
    if (ireg >= 0) {

        /* check if particle is moving towards or away from the sphere(s) centre.
         * we loose here if the particle is in the centre sphere as we don't need
         * to check this condition. see next 'if' statement
         */
        if (aa >= 0) {

            /* ie. particle moving away from center of
               spheres we must check the outer sphere only
             */
            R2b2 = R2[ireg + 1] - bb2;
            if (R2b2 <= 0 && aa > 0) {
                d = 1e-15;    // hopefully a truncation problem
            }
            else {
                tmp = aa2 + R2b2;
                if (tmp > 0) {
                    tmp = sqrt(tmp);
                }
                else {
                    if (tmp < -1e-2) {
                        egsWarning("EGS_cSphericalShell::howfar: something is wrong\n");
                        egsWarning("  we think we are in region %d, but R2b2=%g",
                                   ireg,R2b2);
                    }
                    tmp = 0;
                }
                d = aa > 0 ? R2b2/(tmp + aa) : tmp - aa;
                // the above reduces roundoff, which is significant
                // when aa2 is large compared to R2b2 and aa>0
            }
            rad = -R[ireg + 1];
            direction_flag = ireg + 1;
            if (direction_flag >= nreg) {
                direction_flag = -1;
            }
        }
        else {

            /* so now we know the particle is moving towards the centre of the
             * spheres.  check to see if its trajectory will intersect the nested
             * sphere - we are guaranteed there is one - we checked that already!
             */
            R2b2 = R2[ireg] - bb2;
            tmp = aa2 + R2b2;
            if (tmp <= 0) {   // we will not intersect the nested sphere
                R2b2 = R2[ireg+1] - bb2;
                tmp = aa2 + R2b2;
                if (tmp > 0) {
                    d = sqrt(tmp) - aa;
                }
                else {
                    d = -aa;
                }
                rad = -R[ireg + 1];
                direction_flag= ireg + 1;
                if (direction_flag >= nreg) {
                    direction_flag = -1;
                }
            }
            else {
                // we're hitting the inner sphere (from the outside)
                tmp = sqrt(tmp);
                d = -R2b2/(tmp - aa);
                direction_flag= ireg - 1;
                rad = R[ireg];
            }
        }
    }
    else {
        // if we're in here, we know we're not in the shell, so bb2
        // can be equal to R2[0], it just means we're sitting at the
        // boundary
        if (bb2 <= R2[0] + epsilon) { //epsilon needed for round off errors
            // we are in the hollow center
            R2b2 = R2[0] - bb2;
            tmp = sqrt(aa2 + R2b2);
            d = aa > 0 ? R2b2/(tmp + aa) : tmp - aa;
            direction_flag = 0;
            rad = -R[direction_flag];
        }
        else {
            // we are not inside any of the spherical regions of interest
            if (aa<0) { // we _might_ intersect the largest sphere
                R2b2 = R2[nreg] - bb2;
                tmp = aa2 + R2b2;
                if (tmp > 0) {  // we *will* intersect the largest sphere
                    d = -R2b2/(sqrt(tmp) - aa);
                    direction_flag=nreg-1;
                    rad = R[nreg];
                }
            }
        }
    }

    // check desired step size against this d
    if (d <= t) {
        t=d;
        if (newmed) {
            if (direction_flag >= 0) {
                *newmed = medium(direction_flag);
            }
            else {
                *newmed = -1;
            }
        }
        if (normal) {
            EGS_Vector n(xp + u*d);
            *normal = n*(1/rad);
        }
        return direction_flag;
    }
    return ireg;
}

// hownear - closest perpendicular distance to sphere surface
EGS_Float EGS_cSphericalShell::hownear(int ireg, const EGS_Vector &x) {

    EGS_Vector xp(x-xo);
    EGS_Float r = xp.length();
    EGS_Float d, dout,din;

    if (ireg >= 0) {
        dout = R[ireg+1] - r;
        din = r - R[ireg];
        d = min(dout, din);
    }
    else if (r <= R[0] + epsilon) { //epsilon needed for round off errors
        d = R[0] - r;
    }
    else {
        d = r - R[nreg];
    }

    return d;
}

EGS_Float EGS_cSphericalShell::getBound(int idir, int ind) {
    if (idir == RDIR && ind >= 0 && ind <= nreg) {
        return R[ind];
    }
    return EGS_BaseGeometry::getBound(idir, ind);
}


int EGS_cSphericalShell::getNRegDir(int dir) {
    if (dir == RDIR) {
        return nreg;
    }
    return EGS_BaseGeometry::getNRegDir(dir);
}


EGS_Float EGS_cSphericalShell::getMass(int ireg) {
    if (ireg >= 0 && ireg < nreg) {
        return mass[ireg];
    }
    return 1;
}


void EGS_cSphericalShell::printInfo() const {
    EGS_BaseGeometry::printInfo();
    egsInformation(" midpoint of spheres = (%g,%g,%g)\n",xo.x,xo.y,xo.z);
    egsInformation(" sphere radii = ");
    for (int j=0; j<nreg+1; j++) {
        egsInformation("%g ",R[j]);
    }
    egsInformation(
        "\n=======================================================\n");
}

extern "C" {

    EGS_SPHERES_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {
        if (!input) {
            egsWarning("createGeometry(spheres): null input?\n");
            return 0;
        }
        EGS_Vector xo;
        vector<EGS_Float> Xo;
        int err = input->getInput("midpoint",Xo);
        if (!err && Xo.size() == 3) {
            xo = EGS_Vector(Xo[0],Xo[1],Xo[2]);
        }

        string type= "";
        input->getInput("type", type);

        vector<EGS_Float> radii;
        err = input->getInput("radii",radii);
        if (err) {
            egsWarning("createGeometry(spheres): wrong/missing 'radii' input\n");
            return 0;
        }
        else if ((type == "shell") && (radii.size() < 2)) {
            egsWarning("createGeometry(spheres): You must specify at least two radii for a spherical shell\n");
            return 0;
        }

        EGS_Float *r = new EGS_Float [radii.size()];
        for (int j=0; j<radii.size(); j++) {
            r[j] = radii[j];
        }

        EGS_BaseGeometry *result;
        if (type != "shell") {
            result = new EGS_cSpheres(radii.size(),r,xo);
        }
        else {
            result = new EGS_cSphericalShell(radii.size(), r, xo);
        }
        result->setName(input);
        result->setBoundaryTolerance(input);
        result->setMedia(input);
        result->setLabels(input);
        return result;
    }

}
