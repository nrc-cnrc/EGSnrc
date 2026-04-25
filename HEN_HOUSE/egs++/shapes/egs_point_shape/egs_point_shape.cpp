/*
###############################################################################
#
#  EGSnrc egs++ point shape
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

#include "egs_point_shape.h"
#include "egs_input.h"

extern "C" {

    EGS_POINT_SHAPE_EXPORT EGS_BaseShape *createShape(EGS_Input *input,
            EGS_ObjectFactory *f) {
        (void)f;
        if (!input) {
            egsWarning("createShape(point_shape): null input?\n");
            return 0;
        }
        vector<EGS_Float> pos;
        int err = input->getInput("position",pos);
        if (err) {
            egsWarning("createShape(point_shape): no 'position' input\n");
            return 0;
        }
        if (pos.size() != 3) {
            egsWarning("createShape(point_shape): found %d inputs instead of 3\n",
                       pos.size());
            return 0;
        }
        EGS_PointShape *shape = new EGS_PointShape(EGS_Vector(pos[0],pos[1],pos[2]));
        shape->setName(input);
        return shape;
    }
}
