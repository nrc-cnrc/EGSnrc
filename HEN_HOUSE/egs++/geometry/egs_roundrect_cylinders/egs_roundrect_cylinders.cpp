/*
###############################################################################
#
#  EGSnrc egs++ roundrect cylinders geometry
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
#  Author:          Manuel Stoeckl, 2016
#
#  Contributors:    Hannah Gallop
#
###############################################################################
#
#  A set of concentric rounded rectangle cylinders.
#
###############################################################################
*/


/*! \file egs_roundrect_cylinders.cpp
 *  \brief A set of concentric rounded rectangular cylinders: implementation
 *  \author Manuel Stoeckl
 */

#include "egs_roundrect_cylinders.h"
#include "egs_input.h"

static bool EGS_ROUNDRECT_CYLINDERS_LOCAL inputSet = false;

extern "C" {
    static void setInputs() {
        inputSet = true;

        setBaseGeometryInputs(false);

        geomBlockInput->getSingleInput("library")->setValues({"EGS_RoundRect_Cylinders"});

        // Format: name, isRequired, description, vector string of allowed inputs
        auto typePtr = geomBlockInput->addSingleInput("type", true, "The type of rounded rectangle cylinder", {"EGS_RoundRectCylinders", "EGS_RoundRectCylindersXY", "EGS_RoundRectCylindersYZ", "EGS_RoundRectCylindersXZ"});

        geomBlockInput->addSingleInput("x-widths", true, "A list of cylinder half-widths in the x-direction, must be in increasing order");
        geomBlockInput->addSingleInput("y-widths", true, "A list of cylinder half-widths in the y-direction, must be in increasing order");
        geomBlockInput->addSingleInput("radii", true, "A list of fillet radii, must be in increasing order");
        geomBlockInput->addSingleInput("midpoint", false, "The position of the midpoint (x, y, z)");

        // EGS_RoundRectCylinders
        auto inpPtr = geomBlockInput->addSingleInput("x-axis", true, "x-axis of rounded rectangle (x, y, z)");
        inpPtr->addDependency(typePtr, "EGS_RoundRectCylinders");
        auto yinpPtr = geomBlockInput->addSingleInput("y-axis", true, "y-axis of rounded rectangle (x, y, z)");
        yinpPtr->addDependency(typePtr, "EGS_RoundRectCylinders");
    }

    EGS_ROUNDRECT_CYLINDERS_EXPORT string getExample(string type) {
        string example;
        example = {
            R"(
    # Example of rounded reactangle cylinder
    #:start geometry:
        name = my_roundedrectcylinder
        library = egs_roundrect_cylinders
        type = EGS_RoundRectCylindersXY
        x-widths = 1 2
        y-widths = 0.5 1
        radii = 0.1 0.5
        :start media input:
            media = air water
            set medium = 1 1
        :stop media input:
    :stop geometry:
)"};
        return example;
    }

    EGS_ROUNDRECT_CYLINDERS_EXPORT shared_ptr<EGS_BlockInput> getInputs() {
        if(!inputSet) {
            setInputs();
        }
        return geomBlockInput;
    }

    EGS_ROUNDRECT_CYLINDERS_EXPORT
    EGS_BaseGeometry *createGeometry(EGS_Input *input) {
        // check for valid input
        if (!input) {
            egsWarning("createGeometry(round rectangular cylinders): null input?\n");
            return 0;
        }
        string type;
        int err = input->getInput("type",type);
        if (err) {
            egsWarning("createGeometry(round rectangular cylinders): missing type key\n");
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
        vector<EGS_Float> x_widths, y_widths, radii;
        err = input->getInput("x-widths",x_widths);
        if (err) {
            egsWarning("createGeometry(round rectangular cylinders): wrong/missing "
                       "'x-widths' input\n");
            return 0;
        }
        err = input->getInput("y-widths",y_widths);
        if (err) {
            egsWarning("createGeometry(round rectangular cylinders): wrong/missing "
                       "'y-widths' input\n");
            return 0;
        }
        err = input->getInput("radii",radii);
        if (err) {
            egsWarning("createGeometry(round rectangular cylinders): wrong/missing "
                       "'radii' input\n");
            return 0;
        }
        if (x_widths.size() != y_widths.size() || x_widths.size() != radii.size()) {
            egsWarning("createGeometry(round rectangular cylinders): expecting the same "
                       "number of x- and y-widths and radii, your input is %d %d %d\n",
                       x_widths.size(),y_widths.size(), radii.size());
            return 0;
        }

        for (int i=0; i<x_widths.size(); i++) {
            if (i >= 1) {
                if (x_widths[i-1] > x_widths[i] || x_widths[i] <= 0 || x_widths[i-1] <= 0) {
                    egsWarning("createGeometry(round rectangular cylinders): x-widths must all be positive and in sorted order\n");
                    return 0;
                }
                if (y_widths[i-1] > y_widths[i] || y_widths[i] <= 0 || y_widths[i-1] <= 0) {
                    egsWarning("createGeometry(round rectangular cylinders): y-widths must all be positive and in sorted order\n");
                    return 0;
                }
                if (radii[i] > radii[i-1]) {
                    if (x_widths[i] - radii[i] >= x_widths[i-1] - radii[i-1]) {
                    }
                    else if (x_widths[i] - radii[i] >= x_widths[i-1] - radii[i-1]) {
                    }
                    else {
                        // We have an intersection if the furthest distance on the
                        // smaller circle away from the center of the larger is more
                        // than the larger radius
                        EGS_Float dx = (x_widths[i] - radii[i]) - (x_widths[i-1] - radii[i-1]);
                        EGS_Float dy = (y_widths[i] - radii[i]) - (y_widths[i-1] - radii[i-1]);
                        EGS_Float maxdist = sqrt(dx*dx+dy*dy);
                        if (radii[i] - radii[i-1] < maxdist) {
                            egsWarning("createGeometry(round rectangular cylinders): rounded rectangles may not intersect (but (hx,hy,r) = (%f,%f,%f) and (%f,%f,%f) do)\n",
                                       x_widths[i-1], y_widths[i-1], radii[i-1],
                                       x_widths[i], y_widths[i], radii[i]);
                            return 0;
                        }
                    }
                }
            }
            if (radii[i] < 0) {
                egsWarning("createGeometry(round rectangular cylinders): radii must all be positive\n");
                return 0;
            }
            if (radii[i] > x_widths[i] || radii[i] > y_widths[i]) {
                egsWarning("createGeometry(round rectangular cylinders): radii cannot be larger than half-widths\n");
                return 0;
            }
        }

        // select geometry
        EGS_BaseGeometry *g;
        if (type == "EGS_RoundRectCylindersXY")
            g = new EGS_RoundRectCylindersT<EGS_XProjector,EGS_YProjector>(x_widths,y_widths,radii,xo,"",
                    EGS_XProjector("X"),EGS_YProjector("Y"));
        else if (type == "EGS_RoundRectCylindersXZ")
            g = new EGS_RoundRectCylindersT<EGS_XProjector,EGS_ZProjector>(x_widths,y_widths,radii,xo,"",
                    EGS_XProjector("X"),EGS_ZProjector("Z"));
        else if (type == "EGS_RoundRectCylindersYZ")
            g = new EGS_RoundRectCylindersT<EGS_YProjector,EGS_ZProjector>(x_widths,y_widths,radii,xo,"",
                    EGS_YProjector("Y"),EGS_ZProjector("Z"));
        else {
            vector<EGS_Float> ax, ay;
            err = input->getInput("x-axis",ax);
            if (err || ax.size() != 3) {
                egsWarning("createGeometry(round rectangular cylinders): missing/wrong "
                           "'x-axis' input\n");
                return 0;
            }
            err = input->getInput("y-axis",ay);
            if (err || ay.size() != 3) {
                egsWarning("createGeometry(round rectangular cylinders): missing/wrong "
                           "'y-axis' input\n");
                return 0;
            }
            g = new EGS_RoundRectCylindersT<EGS_Projector,EGS_Projector>(x_widths,y_widths,radii,xo,"",
                    EGS_Projector(EGS_Vector(ax[0],ax[1],ax[2]),"Any"),
                    EGS_Projector(EGS_Vector(ay[0],ay[1],ay[2]),""));
        }
        g->setName(input);
        g->setLabels(input);
        g->setMedia(input);
        return g;
    }

}
