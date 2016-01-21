/*
###############################################################################
#
#  EGSnrc egs++ line shape
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


/*! \file egs_line_shape.cpp
 *  \brief A line shape
 *  \IK
 */

#include "egs_line_shape.h"
#include "egs_input.h"
#include "egs_functions.h"

EGS_LineShape::EGS_LineShape(const vector<EGS_Float> &points,
                             const string &Name, EGS_ObjectFactory *f) : EGS_BaseShape(Name,f) {
    int np = points.size();
    n = np/2;
    if (n <= 1) {
        n = 0;
        return;
    }
    x = new EGS_Float[n];
    y = new EGS_Float[n];
    EGS_Float *w = new EGS_Float [n-1];
    for (int j=0; j<n; j++) {
        x[j] = points[2*j];
        y[j] = points[2*j+1];
        if (j > 0) {
            EGS_Float ax = x[j]-x[j-1], ay = y[j]-y[j-1];
            w[j-1] = sqrt(ax*ax+ay*ay);
        }
    }
    table = new EGS_AliasTable(n-1,x,w,0);
    otype = "line";
}

extern "C" {

    EGS_LINE_SHAPE_EXPORT EGS_BaseShape *createShape(EGS_Input *input,
            EGS_ObjectFactory *f) {
        if (!input) {
            egsWarning("createShape(line): null input?\n");
            return 0;
        }
        vector<EGS_Float> points;
        int err = input->getInput("points",points);
        if (err) {
            egsWarning("createShape(line): no 'points' input\n");
            return 0;
        }
        int np = points.size();
        if ((np%2) != 0) {
            egsWarning("createShape(line): you must input an even number of"
                       " floating numbers\n");
            return 0;
        }
        if (np < 4) {
            egsWarning("createShape(line): you must input at least 2 2D points"
                       " to form a line\n");
            return 0;
        }
        EGS_LineShape *shape = new EGS_LineShape(points,"",f);
        shape->setName(input);
        shape->setTransformation(input);
        return shape;
    }

}
