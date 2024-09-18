/*
###############################################################################
#
#  EGSnrc egs++ point source
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


/*! \file egs_point_source.cpp
 *  \brief A point source
 *  \IK
 */

#include "egs_point_source.h"
#include "egs_input.h"

static bool EGS_POINT_SOURCE_LOCAL inputSet = false;

EGS_PointSource::EGS_PointSource(EGS_Input *input, EGS_ObjectFactory *f) :
    EGS_BaseSimpleSource(input,f), xo(), valid(true) {
    vector<EGS_Float> pos;
    int err = input->getInput("position",pos);
    if (!err && pos.size() == 3) {
        xo = EGS_Vector(pos[0],pos[1],pos[2]);
    }
    else {
        egsWarning("EGS_PointSource: missing/wrong 'position' input\n");
        valid = false;
    }
    setUp();
}

void EGS_PointSource::setUp() {
    otype = "EGS_PointSource";
    if (!isValid()) {
        description = "Invalid point source";
    }
    else {
        description = "Point source with ";
        description += s->getType();
        if (q == -1) {
            description += ", electrons";
        }
        else if (q == 0) {
            description += ", photons";
        }
        else if (q == 1) {
            description += ", positrons";
        }
        else {
            description += ", unknown particle type";
        }
    }
}


extern "C" {

    static void setInputs() {
        inputSet = true;

        setBaseSourceInputs();

        srcBlockInput->getSingleInput("library")->setValues({"EGS_Point_Source"});

        // Format: name, isRequired, description, vector string of allowed values
        srcBlockInput->addSingleInput("position", true, "The position of the point source, (x, y, z)");
    }

    EGS_POINT_SOURCE_EXPORT string getExample() {
        string example;
        example = {
            R"(
    # Example of egs_point_source
    #:start source:
        library = egs_point_source
        name = my_source
        position = 0 0 0
        :start spectrum:
            type = monoenergetic
            energy = 1
        :stop spectrum:
        charge = 0
    :stop source:
)"};
        return example;
    }

    EGS_POINT_SOURCE_EXPORT shared_ptr<EGS_BlockInput> getInputs() {
        if(!inputSet) {
            setInputs();
        }
        return srcBlockInput;
    }

    EGS_POINT_SOURCE_EXPORT EGS_BaseSource *createSource(EGS_Input *input,
            EGS_ObjectFactory *f) {
        return createSourceTemplate<EGS_PointSource>(input,f,"point source");
    }

}
