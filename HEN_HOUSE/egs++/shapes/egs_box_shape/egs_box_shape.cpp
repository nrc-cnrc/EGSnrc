/*
###############################################################################
#
#  EGSnrc egs++ box shape
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

#include "egs_box_shape.h"
#include "egs_input.h"

extern "C" {

    EGS_BOX_SHAPE_EXPORT EGS_BaseShape *createShape(EGS_Input *input,
            EGS_ObjectFactory *f) {
        (void)f;
        if (!input) {
            egsWarning("createShape(box_shape): null input?\n");
            return 0;
        }
        vector<EGS_Float> s;
        int err = input->getInput("box size",s);
        if (err) {
            egsWarning("createShape(box_shape): no 'box size' input?\n");
            return 0;
        }
        EGS_AffineTransform *t = EGS_AffineTransform::getTransformation(input);
        EGS_BoxShape *shape = 0;
        if (s.size() == 1) {
            shape = new EGS_BoxShape(s[0],t);
        }
        else if (s.size() == 3) {
            shape = new EGS_BoxShape(s[0],s[1],s[2],t);
        }
        else {
            egsWarning("createShape(box_shape): invalid 'box size' input\n");
        }
        if (t) {
            delete t;
        }
        if (!shape) {
            return 0;
        }
        shape->setName(input);
        return shape;
    }
}
