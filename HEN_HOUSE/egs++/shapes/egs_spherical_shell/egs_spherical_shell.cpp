/*
###############################################################################
#
#  EGSnrc egs++ spherical shell shape
#  Copyright (C) 2016 Randle E. P. Taylor, Rowan M. Thomson,
#  Marc J. P. Chamberland, D. W. O. Rogers
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
#                   Hannah Gallop
#
###############################################################################
#
#  egs_spherical_shell was developed for the Carleton Laboratory for
#  Radiotherapy Physics.
#
###############################################################################
*/


/*! \file egs_spherical_shell.cpp
    \brief a spherical shell shape
    \author Randle Taylor (randle.taylor@gmail.com)
*/

#include "egs_spherical_shell.h"
#include "egs_input.h"
#include "egs_functions.h"

static bool EGS_SPHERICAL_SHELL_LOCAL inputSet = false;
static shared_ptr<EGS_BlockInput> EGS_SPHERICAL_SHELL_LOCAL shapeBlockInput = make_shared<EGS_BlockInput>("shape");

EGS_SphericalShellShape::EGS_SphericalShellShape(EGS_Float ri, EGS_Float ro, int hemisph, EGS_Float halfangle, const EGS_Vector &Xo,
        const string &Name,EGS_ObjectFactory *f) :
    EGS_BaseShape(Name, f), r_inner(ri), r_outer(ro), hemisphere(hemisph), half_angle(halfangle), xo(Xo) {
    otype = "sphericalShell";
    if (half_angle < 0) {
        sgn = -1;
        half_angle = fabs(half_angle);
    }
    else {
        sgn = 1;
    }
};


/*! \brief Returns a random point within the spherical shell. */
EGS_Vector EGS_SphericalShellShape::getPoint(EGS_RandomGenerator *rndm) {

    EGS_Float rnd1 = rndm->getUniform();
    EGS_Float rnd2 = rndm->getUniform();

    EGS_Float rad = r_inner + (r_outer - r_inner)*pow(rnd1, 1/3.);

    EGS_Float cos_max, cos_min;
    EGS_Float cos_th;

    if (fabs(half_angle) < 1E-4) {
        cos_th = 2*rnd2 -1;
    }
    else {
        /* conical section */
        cos_min = cos(half_angle);
        cos_th = cos_min + rnd2*(1 - cos_min);
        rad *= sgn;
    }

    EGS_Float sin_th = sqrt(1-cos_th*cos_th);

    EGS_Float cos_phi, sin_phi;
    rndm->getAzimuth(cos_phi, sin_phi);

    EGS_Float x = rad*sin_th*cos_phi;
    EGS_Float y = rad*sin_th*sin_phi;
    EGS_Float z = rad*cos_th;

    if (hemisphere != 0) {
        z = hemisphere*fabs(z);
    }

    return xo + EGS_Vector(x, y, z);

};


void EGS_SphericalShellShape::getPointSourceDirection(const EGS_Vector &Xo,
        EGS_RandomGenerator *rndm, EGS_Vector &u, EGS_Float &wt) {
    EGS_Vector xo = T ? Xo*(*T) : Xo;
    EGS_Float cost = 2*rndm->getUniform()-1;
    EGS_Float sint = 1-cost*cost;
    EGS_Vector x;
    if (sint > 1e-10) {
        EGS_Float cphi, sphi;
        rndm->getAzimuth(cphi,sphi);
        sint = r_outer*sqrt(sint);
        x.x = sint*cphi;
        x.y = sint*sphi;
        x.z = r_outer*cost;
    }
    else {
        x.z = r_outer*cost;
    }
    u = (x + this->xo) - xo;
    EGS_Float di = 1/u.length();
    u *= di;
    wt = u*x*4*M_PI*r_outer*di*di;
};

EGS_Float EGS_SphericalShellShape::area() const {
    return 4*M_PI*r_outer*r_outer;
};

extern "C" {

    static void setInputs() {
        inputSet = true;

        shapeBlockInput->addSingleInput("library", true, "The type of shape, loaded by shared library in egs++/dso.", {"EGS_Spherical_Shape"});
        shapeBlockInput->addSingleInput("midpoint", true, "The midpoint of the shape, (x y z)");
        shapeBlockInput->addSingleInput("inner radius", true, "The inner radius");
        shapeBlockInput->addSingleInput("outer radius", true, "The outer radius");
        shapeBlockInput->addSingleInput("hemisphere", false, "Hemisphere");
        shapeBlockInput->addSingleInput("hemisphere", false, "The half angle, in degrees");
        setShapeInputs(shapeBlockInput);
    }

    EGS_SPHERICAL_SHELL_EXPORT string getExample() {
        string example;
        example = {
            R"(
    # Example of egs_spherical_shell
    #:start shape:
        library = egs_spherical_shell
        midpoint = 0 0 0
        innner radius = 0.5
        outer radius = 1
        hemisphere = 1
        half angle = 35
    :stop shape:
)"};
        return example;
    }

    EGS_SPHERICAL_SHELL_EXPORT shared_ptr<EGS_BlockInput> getInputs() {
        if(!inputSet) {
            setInputs();
        }
        return shapeBlockInput;
    }

    EGS_SPHERICAL_SHELL_EXPORT EGS_BaseShape *createShape(EGS_Input *input, EGS_ObjectFactory *f) {

        if (!input) {
            egsWarning("createShape(sphericalShell): null input?\n");
            return 0;
        }

        EGS_Float ri, ro;
        int err = input->getInput("inner radius", ri);
        if (err) {
            egsWarning("createShape(sphericalShell): no 'inner radius' input\n");
            return 0;
        }
        else if (ri < 0) {
            egsWarning("createShape(sphericalShell): 'inner radius' must be >= 0\n");
            return 0;
        }

        err = input->getInput("outer radius", ro);
        if (err) {
            egsWarning("createShape(sphericalShell): no 'outer radius' input\n");
            return 0;
        }
        else if (ro < 0) {
            egsWarning("createShape(sphericalShell): 'outer radius' must be >= 0\n");
            return 0;
        }
        else if (ri > ro) {
            egsWarning("createShape(sphericalShell): 'inner radius' must be less than 'outer radius'\n");
            return 0;
        }

        int hemisphere;
        err = input->getInput("hemisphere", hemisphere);
        if (err) {
            hemisphere = 0;
        }
        else if ((hemisphere != 0) && (hemisphere != -1) && (hemisphere != 1)) {
            egsWarning("createShape(sphericalShell): 'hemisphere' must be 1, -1, 0\n");
            return 0;
        }

        EGS_Float half_angle=0, half_angle_deg= 0;
        err = input->getInput("half angle", half_angle_deg);
        if (err) {
            half_angle = 0;
        }
        else {
            half_angle = M_PI/180.*half_angle_deg;
        }

        if (half_angle && hemisphere) {
            egsWarning("createShape(sphericalShell): Hemisphere and half angle specified! Remove one and try again.");
            return 0;
        }


        EGS_SphericalShellShape *result;

        vector<EGS_Float> xo;
        err = input->getInput("midpoint",xo);
        if (err || xo.size() != 3) {
            xo.clear();
            xo.push_back(0);
            xo.push_back(0);
            xo.push_back(0);
        }

        result = new EGS_SphericalShellShape(ri, ro, hemisphere, half_angle, EGS_Vector(xo[0],xo[1],xo[2]));
        result->setName(input);
        return result;
    }

}
