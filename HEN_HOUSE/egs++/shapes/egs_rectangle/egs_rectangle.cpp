/*
###############################################################################
#
#  EGSnrc egs++ rectangle shape
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
#  Contributors:
#
###############################################################################
*/


/*! \file egs_rectangle.cpp
 *  \brief Rectangular shapes
 *  \IK
 */

#include "egs_rectangle.h"
#include "egs_input.h"
#include "egs_functions.h"

EGS_RectangularRing::EGS_RectangularRing(EGS_Float xmin, EGS_Float xmax,
        EGS_Float ymin, EGS_Float ymax, EGS_Float xmin_i, EGS_Float xmax_i,
        EGS_Float ymin_i, EGS_Float ymax_i, const string &Name,
        EGS_ObjectFactory *f) : EGS_SurfaceShape(Name,f), valid(true) {
    EGS_Float tmp;
    if (xmin > xmax) {
        tmp = xmax;
        xmax = xmin;
        xmin = tmp;
    }
    if (ymin > ymax) {
        tmp = ymax;
        ymax = ymin;
        ymin = tmp;
    }
    if (xmin_i > xmax_i) {
        tmp = xmax_i;
        xmax_i = xmin_i;
        xmin_i = tmp;
    }
    if (ymin_i > ymax_i) {
        tmp = ymax_i;
        ymax_i = ymin_i;
        ymin_i = tmp;
    }
    if (xmin_i < xmin || ymin_i < ymin || xmax_i > xmax || ymax_i > ymax) {
        valid = false;
        return;
    }
    r[0] = new EGS_RectangleShape(xmin,xmin_i,ymin,ymax);
    r[1] = new EGS_RectangleShape(xmax_i,xmax,ymin,ymax);
    r[2] = new EGS_RectangleShape(xmin_i,xmax_i,ymin,ymin_i);
    r[3] = new EGS_RectangleShape(xmin_i,xmax_i,ymax_i,ymax);
    p[0] = (xmin_i - xmin)*(ymax - ymin);
    p[1] = (xmax - xmax_i)*(ymax - ymin);
    p[2] = (xmax_i - xmin_i)*(ymin_i - ymin);
    p[3] = (xmax_i - xmin_i)*(ymax - ymax_i);
    A = p[0] + p[1] + p[2] + p[3];
    p[0] /= A;
    p[1] = p[1]/A + p[0];
    p[2] = p[2]/A + p[1];
    p[3] = 1.1;

    otype = "rectangular ring";

}

EGS_RectangularRing::~EGS_RectangularRing() {
    if (valid) {
        delete r[0];
        delete r[1];
        delete r[2];
        delete r[3];
    };
}


extern "C" {

    EGS_RECTANGLE_EXPORT EGS_BaseShape *createShape(EGS_Input *input,
            EGS_ObjectFactory *f) {
        if (!input) {
            egsWarning("createShape(rectangle): null input?\n");
            return 0;
        }
        vector<EGS_Float> pos;
        int err = input->getInput("rectangle",pos);
        if (err) {
            egsWarning("createShape(rectangle): no 'rectangle' input\n");
            return 0;
        }
        if (pos.size() != 4) {
            egsWarning("createShape(rectangle): found only %d inputs instead"
                       " of 4\n");
            return 0;
        }
        EGS_BaseShape *shape;
        vector<EGS_Float> posi;
        err = input->getInput("inner rectangle",posi);
        if (!err && posi.size() == 4) {
            EGS_RectangularRing *s = new EGS_RectangularRing(pos[0],pos[2],pos[1],
                    pos[3],posi[0],posi[2],posi[1],posi[3],"",f);
            if (!s->isValid()) {
                egsWarning("createShape(rectangle): your input did not result in"
                           " a valid \"rectangular ring\"\n");
                delete s;
            }
            else {
                shape = s;
            }
        }
        else {
            shape = new EGS_RectangleShape(pos[0],pos[2],pos[1],pos[3],"",f);
        }
        if (shape) {
            shape->setName(input);
            shape->setTransformation(input);
        }
        return shape;
    }

}
