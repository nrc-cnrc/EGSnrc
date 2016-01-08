/*
###############################################################################
#
#  EGSnrc egs++ elliptic cylinders geometry
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
#  Author:          Iwan Kawrakow, 2006
#
#  Contributors:
#
###############################################################################
#
#  A first version of a set of elliptic cylinders. Already works in the
#  geometry viewer, but still need to run it through the geometry tester. The
#  hard part for that type of geometry is hownear() (requires to solve a 4'th
#  order equation).
#
###############################################################################
*/


/*! \file egs_cylinders.cpp
 *  \brief A set of concentric cylinders: implementation
 *  \author Declan Persram and Iwan Kawrakow
 */

#include "egs_elliptic_cylinders.h"
#include "egs_input.h"

extern "C" {

    EGS_ELLIPTIC_CYLINDERS_EXPORT
    EGS_BaseGeometry *createGeometry(EGS_Input *input) {
        // check for valid input
        if (!input) {
            egsWarning("createGeometry(elliptic cylinders): null input?\n");
            return 0;
        }
        string type;
        int err = input->getInput("type",type);
        if (err) {
            egsWarning("createGeometry(elliptic cylinders): missing type key\n");
            return 0;
        }

        // point on cylinder axis
        EGS_Vector xo;
        vector<EGS_Float> Xo;
        err = input->getInput("midpoint",Xo);
        if (!err && Xo.size() == 3) {
            xo = EGS_Vector(Xo[0],Xo[1],Xo[2]);
        }

        // cylinder radii
        vector<EGS_Float> x_radii, y_radii;
        err = input->getInput("x-radii",x_radii);
        if (err) {
            egsWarning("createGeometry(elliptic cylinders): wrong/missing "
                       "'x-radii' input\n");
            return 0;
        }
        err = input->getInput("y-radii",y_radii);
        if (err) {
            egsWarning("createGeometry(elliptic cylinders): wrong/missing "
                       "'y-radii' input\n");
            return 0;
        }
        if (x_radii.size() != y_radii.size()) {
            egsWarning("createGeometry(elliptic cylinders): expecting the same "
                       "number of x- and y-radii, your input is %d %d\n",
                       x_radii.size(),y_radii.size());
            return 0;
        }

        // select geometry
        EGS_BaseGeometry *g;
        if (type == "EGS_EllipticCylindersXY")
            g = new EGS_EllipticCylindersXY(x_radii,y_radii,xo,"",
                                            EGS_XProjector("X"),EGS_YProjector("Y"));
        else if (type == "EGS_EllipticCylindersXZ")
            g = new EGS_EllipticCylindersXZ(x_radii,y_radii,xo,"",
                                            EGS_XProjector("X"),EGS_ZProjector("Z"));
        else if (type == "EGS_EllipticCylindersYZ")
            g = new EGS_EllipticCylindersYZ(x_radii,y_radii,xo,"",
                                            EGS_YProjector("Y"),EGS_ZProjector("Z"));
        else {
            vector<EGS_Float> ax, ay;
            err = input->getInput("x-axis",ax);
            if (err || ax.size() != 3) {
                egsWarning("createGeometry(elliptic cylinders): missing/wrong "
                           "'x-axis' input\n");
                return 0;
            }
            err = input->getInput("y-axis",ay);
            if (err || ay.size() != 3) {
                egsWarning("createGeometry(elliptic cylinders): missing/wrong "
                           "'y-axis' input\n");
                return 0;
            }
            g = new EGS_EllipticCylinders(x_radii,y_radii,xo,"",
                                          EGS_Projector(EGS_Vector(ax[0],ax[1],ax[2]),"Any"),
                                          EGS_Projector(EGS_Vector(ay[0],ay[1],ay[2]),""));
        }
        g->setName(input);
        g->setMedia(input);
        return g;
    }

}
