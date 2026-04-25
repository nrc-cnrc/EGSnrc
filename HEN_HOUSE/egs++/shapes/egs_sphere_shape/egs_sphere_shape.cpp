/*
###############################################################################
#
#  EGSnrc egs++ sphere shape
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

#include "egs_sphere_shape.h"
#include "egs_input.h"

extern "C" {

    EGS_SPHERE_SHAPE_EXPORT EGS_BaseShape *createShape(EGS_Input *input,
            EGS_ObjectFactory *f) {
        (void)f;
        if (!input) {
            egsWarning("createShape(sphere_shape): null input?\n");
            return 0;
        }
        EGS_Float r;
        int err = input->getInput("radius",r);
        if (err) {
            egsWarning("createShape(sphere_shape): wrong/missing 'radius' input\n");
            return 0;
        }
        vector<EGS_Float> xo;
        EGS_SphereShape *shape = 0;
        err = input->getInput("midpoint",xo);
        if (!err && xo.size() == 3) {
            shape = new EGS_SphereShape(r,EGS_Vector(xo[0],xo[1],xo[2]));
        }
        else {
            shape = new EGS_SphereShape(r);
        }
        shape->setName(input);
        return shape;
    }
}
