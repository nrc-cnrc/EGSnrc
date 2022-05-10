/*
###############################################################################
#
#  EGSnrc egs++ ellipse shape
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
#  Contributors:    Hannah Gallop
#
###############################################################################
*/


/*! \file egs_ellipse.cpp
 *  \brief An elliptical shape
 *  \IK
 */

#include "egs_ellipse.h"
#include "egs_input.h"
#include "egs_functions.h"

static bool EGS_ELLIPSE_LOCAL inputSet = false;
static shared_ptr<EGS_BlockInput> EGS_ELLIPSE_LOCAL shapeBlockInput = make_shared<EGS_BlockInput>("shape");

extern "C" {

    static void setInputs() {
        inputSet = true;

        shapeBlockInput->addSingleInput("library", true, "The type of shape, loaded by shared library in egs++/dso.", {"EGS_Ellipse"});
        shapeBlockInput->addSingleInput("halfaxis", true, "The two half axis of the ellipse.");
        shapeBlockInput->addSingleInput("midpoint", false, "The midpoint of the ellipse, (x, y).");
        setShapeInputs(shapeBlockInput);
    }

    EGS_ELLIPSE_EXPORT string getExample() {
        string example;
        example = {
            R"(
    # Example fo egs_ellipse
    #:start shape:
        library = egs_ellipse
        halfway = the two half axis of the ellipse
        midpoint = Ox, Oy (optional)
    :stop shape:
)"};
        return example;
    }

    EGS_ELLIPSE_EXPORT shared_ptr<EGS_BlockInput> getInputs() {
       if(!inputSet) {
           setInputs();
       }
       return shapeBlockInput;
    }

    EGS_ELLIPSE_EXPORT EGS_BaseShape *createShape(EGS_Input *input,
            EGS_ObjectFactory *f) {
        if (!input) {
            egsWarning("createShape(ellipse): null input?\n");
            return 0;
        }
        vector<EGS_Float> axis;
        int err = input->getInput("halfaxis",axis);
        if (err) {
            egsWarning("createShape(ellipse): no 'halfaxis' input\n");
            return 0;
        }
        if (axis.size() != 2) {
            egsWarning("createShape(ellipse): 2 instead of %d inputs expected"
                       " for keyword 'halfaxis'.\n",axis.size());
            return 0;
        }
        vector<EGS_Float> pos;
        err = input->getInput("midpoint",pos);
        if (err) {
            pos.clear();
            pos.push_back(0);
            pos.push_back(0);
        }
        else {
            if (pos.size() != 2) {
                egsWarning("createShape(ellipse): 2 instead of %d inputs expected"
                           " for keyword 'midpoint'. Reseting midpoint to (0,0)\n",
                           pos.size());
                pos.clear();
                pos.push_back(0);
                pos.push_back(0);
            }
        }
        EGS_EllipseShape *shape = new EGS_EllipseShape(pos[0],pos[1],
                axis[0],axis[1],"",f);
        shape->setName(input);
        shape->setTransformation(input);
        return shape;
    }

}
