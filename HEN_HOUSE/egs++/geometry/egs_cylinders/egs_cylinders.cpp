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
#                   Hannah Gallop
#
###############################################################################
*/


/*! \file egs_cylinders.cpp
 *  \brief A set of concentric cylinders: implementation
 *  \author Declan Persram and Iwan Kawrakow
 */

#include "egs_cylinders.h"
#include "egs_input.h"

static bool EGS_CYLINDERS_LOCAL inputSet = false;

extern "C" {

    static void setInputs() {
        inputSet = true;

        setBaseGeometryInputs(false);

        geomBlockInput->getSingleInput("library")->setValues({"EGS_Cylinders"});

        // Format: name, isRequired, description, vector string of allowd values
        auto typePtr = geomBlockInput->addSingleInput("type", true, "The type of cylinder.", {"EGS_XCylinders", "EGS_YCylinders", "EGS_ZCylinders", "EGS_Cylinders"});

        geomBlockInput->addSingleInput("radii", true, "A list of cylinder radii, must be in increasing order");
        geomBlockInput->addSingleInput("midpoint", false, "The position of the midpoint of the cylinder (x, y, z)");

        // EGS_Cylinders
        auto inpPtr = geomBlockInput->addSingleInput("axis", true, "The unit vector defining the axis along the length of the cylinder.");
        inpPtr->addDependency(typePtr, "EGS_Cylinders");
    }

    EGS_CYLINDERS_EXPORT string getExample(string type) {
        string example;
        example = {
            R"(
    # Examples of the egs_cylinders to follow

    # EGS_XCylinder example
    #:start geometry:
        library = egs_cylinders
        type = EGS_XCylinders
        name = my_xcylinders
        radii = 1 2 3
        midpoint = 0
        :start media input:
            media = water air water
            set medium = 1 1
            set medium = 2 2
        :stop media input:
    :stop geometry:

    # EGS_Cylinder example
    #:start geometry:
        library = egs_cylinders
        type = EGS_Cylinders
        name = my_cylinder
        radii = 7
        axis = 4 3 2
        midpoint = 0 0 0
        :start media input:
            media = water
        :stop media input:
    :stop geometry:
)"};
        return example;
    }

    EGS_CYLINDERS_EXPORT shared_ptr<EGS_BlockInput> getInputs() {
        if(!inputSet) {
            setInputs();
        }
        return geomBlockInput;
    }

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
            g = new EGS_CylindersX(radii.size(),r,xo,"",EGS_XProjector("EGS_XCylinders"));
        }
        else if (type == "EGS_YCylinders") {
            g = new EGS_CylindersY(radii.size(),r,xo,"",EGS_YProjector("EGS_YCylinders"));
        }
        else if (type == "EGS_ZCylinders") {
            g = new EGS_CylindersZ(radii.size(),r,xo,"",EGS_ZProjector("EGS_ZCylinders"));
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
                                  EGS_Projector(EGS_Vector(a[0],a[1],a[2]),""));
        }
        g->setName(input);
        g->setBoundaryTolerance(input);
        g->setMedia(input);
        g->setLabels(input);
        return g;
    }

}
