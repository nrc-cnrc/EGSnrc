/*
###############################################################################
#
#  EGSnrc egs++ cylinder shape
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
*/

#include "egs_cylinder_shape.h"
#include "egs_input.h"

extern "C" {

    EGS_CYLINDER_SHAPE_EXPORT EGS_BaseShape *createShape(EGS_Input *input,
            EGS_ObjectFactory *f) {
        (void)f;
        if (!input) {
            egsWarning("createShape(cylinder_shape): null input?\n");
            return 0;
        }
        EGS_Float r, H;
        int err = input->getInput("radius",r);
        if (err) {
            egsWarning("createShape(cylinder_shape): wrong/missing 'radius' input\n");
            return 0;
        }
        err = input->getInput("height",H);
        if (err) {
            egsWarning("createShape(cylinder_shape): wrong/missing 'height' input\n");
            return 0;
        }
        vector<EGS_Float> phi_range;
        bool set_phi = false;
        if (!input->getInput("phi range",phi_range) && phi_range.size() == 2) {
            set_phi = true;
            phi_range[0] *= M_PI/180;
            phi_range[1] *= M_PI/180;
        }
        EGS_AffineTransform *t = EGS_AffineTransform::getTransformation(input);
        if (t) {
            EGS_CylinderShape *shape = new EGS_CylinderShape(r,H,t);
            shape->setName(input);
            if (set_phi) {
                shape->setPhiRange(phi_range[0],phi_range[1]);
            }
            delete t;
            return shape;
        }
        vector<EGS_Float> Xo, A;
        int err1 = input->getInput("midpoint",Xo);
        int err2 = input->getInput("axis",A);
        bool has_Xo = (err1 == 0 && Xo.size() == 3);
        bool has_A = (err2 == 0 && A.size() == 3);
        EGS_CylinderShape *shape;
        if (has_Xo && has_A) {
            shape = new EGS_CylinderShape(r,H,EGS_Vector(Xo[0],Xo[1],Xo[2]),EGS_Vector(A[0],A[1],A[2]));
        }
        else if (has_Xo) {
            shape = new EGS_CylinderShape(r,H,EGS_Vector(Xo[0],Xo[1],Xo[2]));
        }
        else if (has_A) {
            shape = new EGS_CylinderShape(r,H,EGS_Vector(0,0,0),EGS_Vector(A[0],A[1],A[2]));
        }
        else {
            shape = new EGS_CylinderShape(r,H);
        }
        shape->setName(input);
        if (set_phi) {
            shape->setPhiRange(phi_range[0],phi_range[1]);
        }
        return shape;
    }
}
