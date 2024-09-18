/*
###############################################################################
#
#  EGSnrc egs++ circle shape
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
#  Contributors:    Reid Townson
#
###############################################################################
*/


/*! \file egs_circle.cpp
 *  \brief A circular shape
 *  \IK
 */

#include "egs_circle.h"
#include "egs_input.h"
#include "egs_functions.h"

static string EGS_CIRCLE_LOCAL typeStr("EGS_Circle");
static bool EGS_CIRCLE_LOCAL inputSet = false;
static shared_ptr<EGS_BlockInput> EGS_CIRCLE_LOCAL shapeBlockInput = make_shared<EGS_BlockInput>("shape");

extern "C" {

    static void setInputs() {
        inputSet = true;
        shapeBlockInput->addSingleInput("library", true, "The type of shape, loaded by shared library in egs++/dso.", vector<string>(1, typeStr));
        shapeBlockInput->addSingleInput("radius", false, "The radius of the circle.");
        shapeBlockInput->addSingleInput("midpoint", false, "The x, y midpoint of the circle, which is in the x-y plane located at z=0. Use an EGS_AffineTransform block to translate or rotate the shape.");
        shapeBlockInput->addSingleInput("inner radius", false, "The inner radius, to define a ring. Points will only be sampled within the ring between the 'inner radius' and 'radius'.");
        setShapeInputs(shapeBlockInput);
    }

    EGS_CIRCLE_EXPORT string getExample() {
        string example {
            R"(
        :start shape:
            library = egs_circle
            radius = the circle radius
            midpoint = Ox, Oy (optional)
            inner radius = the inner radius (optional)
        :stop shape:
)"};
        return example;
    }

    EGS_CIRCLE_EXPORT shared_ptr<EGS_BlockInput> getInputs() {
        if(!inputSet) {
            setInputs();
        }
        return shapeBlockInput;
    }

    EGS_CIRCLE_EXPORT EGS_BaseShape *createShape(EGS_Input *input,
            EGS_ObjectFactory *f) {
        if (!input) {
            egsWarning("createShape(circle): null input?\n");
            return 0;
        }
        EGS_Float radius;
        int err = input->getInput("radius",radius);
        if (err) {
            egsWarning("createShape(circle): no 'radius' input\n");
            return 0;
        }
        EGS_Float Ro;
        err = input->getInput("inner radius",Ro);
        if (err) {
            Ro = 0;
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
                egsWarning("createShape(circle): 2 instead of %d inputs expected"
                           " for keyword 'midpoint'. Reseting midpoint to (0,0)\n",
                           pos.size());
                pos.clear();
                pos.push_back(0);
                pos.push_back(0);
            }
        }
        EGS_CircleShape *shape = new EGS_CircleShape(pos[0],pos[1],radius,Ro,"",f);
        shape->setName(input);
        shape->setTransformation(input);
        return shape;
    }

}
