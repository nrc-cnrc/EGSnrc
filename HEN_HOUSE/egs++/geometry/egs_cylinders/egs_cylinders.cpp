/*
###############################################################################
#
#  EGSnrc egs++ cylinder geometry
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


/*! \file egs_cylinders.cpp
 *  \brief A set of concentric cylinders: implementation
 *  \author Declan Persram and Iwan Kawrakow
 */

#include "egs_cylinders.h"
#include "egs_input.h"

#ifndef SKIP_DOXYGEN
    string XProjector::type = "EGS_Xcylinders";
    string YProjector::type = "EGS_Ycylinders";
    string ZProjector::type = "EGS_Zcylinders";
    string Projector::type = "EGS_cylinders";
#endif

extern "C" {

    EGS_CYLINDERS_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {
        // check for valid input
        if (!input) {
            egsWarning("createGeometry(cylinders): null input?\n");
            return 0;
        }
        string type;
        int err = input->getInput("type",type);
        if (err) {
            egsWarning("createGeometry(cylinders): missing type key\n");
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
        vector<EGS_Float> radii;
        err = input->getInput("radii",radii);
        if (err) {
            egsWarning("createGeometry(cylinders): wrong/missing 'radii' input\n");
            return 0;
        }
        EGS_Float *r=new EGS_Float [radii.size()];
        for (int i=0; i<radii.size(); i++) {
            r[i]=radii[i];
        }

        // select geometry
        EGS_BaseGeometry *g;
        if (type == "EGS_XCylinders") {
            g = new EGS_CylindersX(radii.size(),r,xo,"",XProjector());
        }
        else if (type == "EGS_YCylinders") {
            g = new EGS_CylindersY(radii.size(),r,xo,"",YProjector());
        }
        else if (type == "EGS_ZCylinders") {
            g = new EGS_CylindersZ(radii.size(),r,xo,"",ZProjector());
        }
        else {
            vector<EGS_Float> a;
            err = input->getInput("axis",a);
            if (err || a.size() != 3) {
                egsWarning("createGeometry(cylinders): missing/wrong axis input\n");
                return 0;
            }
            egsWarning("got axis (%g,%g,%g)\n",a[0],a[1],a[2]);
            g = new EGS_Cylinders(radii.size(),r,xo,"",
                                  Projector(EGS_Vector(a[0],a[1],a[2])));
        }
        g->setName(input);
        g->setMedia(input);
        g->setLabels(input);
        return g;
    }

}
