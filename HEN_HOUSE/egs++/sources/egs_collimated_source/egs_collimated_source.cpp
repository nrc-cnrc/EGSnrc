/*
###############################################################################
#
#  EGSnrc egs++ collimated source
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
#  Contributors:    Hubert Ho
#                   Reid Townson
#                   Hannah Gallop
#
###############################################################################
*/


/*! \file egs_collimated_source.cpp
 *  \brief A collimated isotropic source
 *  \IK
 */

#include "egs_collimated_source.h"
#include "egs_input.h"

static bool EGS_COLLIMATED_SOURCE_LOCAL inputSet = false;

EGS_CollimatedSource::EGS_CollimatedSource(EGS_Input *input,
        EGS_ObjectFactory *f) : EGS_BaseSimpleSource(input,f),
    source_shape(0), target_shape(0), ctry(0), dist(1) {
    EGS_Input *ishape = input->takeInputItem("source shape");
    if (ishape) {
        source_shape = EGS_BaseShape::createShape(ishape);
        delete ishape;
    }
    if (!source_shape) {
        string sname;
        int err = input->getInput("source shape name",sname);
        if (err)
            egsWarning("EGS_CollimatedSource: missing/wrong inline source "
                       "shape definition and missing/wrong 'source shape name' input\n");
        else {
            source_shape = EGS_BaseShape::getShape(sname);
            if (!source_shape)
                egsWarning("EGS_CollimatedSource: a shape named %s"
                           " does not exist\n",sname.c_str());
        }
    }
    ishape = input->takeInputItem("target shape");
    if (ishape) {
        target_shape = EGS_BaseShape::createShape(ishape);
        delete ishape;
    }
    if (!target_shape) {
        string sname;
        int err = input->getInput("target shape name",sname);
        if (err)
            egsWarning("EGS_CollimatedSource: missing/wrong inline target"
                       "shape definition and missing/wrong 'target shape name' input\n");
        else {
            target_shape = EGS_BaseShape::getShape(sname);
            if (!target_shape)
                egsWarning("EGS_CollimatedSource: a shape named %s"
                           " does not exist\n",sname.c_str());
        }
    }
    if (target_shape) {
        if (!target_shape->supportsDirectionMethod())
            egsWarning("EGS_CollimatedSource: the target shape %s, which is"
                       " of type %s, does not support the getPointSourceDirection()"
                       " method\n",target_shape->getObjectName().c_str(),
                       target_shape->getObjectType().c_str());
    };
    EGS_Float auxd;
    int errd = input->getInput("distance",auxd);
    if (!errd) {
        dist = auxd;
    }
    setUp();
}

void EGS_CollimatedSource::setUp() {
    otype = "EGS_CollimatedSource";
    if (!isValid()) {
        description = "Invalid collimated source";
    }
    else {
        description = "Collimated source from a shape of type ";
        description += source_shape->getObjectType();
        description += " onto a shape of type ";
        description += target_shape->getObjectType();
        description += " with ";
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

        srcBlockInput->getSingleInput("library")->setValues({"EGS_Collimated_Source"});

        // Format: name,  isRequired, description, vector string of allowed values
        auto source_shapePtr = srcBlockInput->addBlockInput("source shape");
        auto target_shapePtr = srcBlockInput->addBlockInput("target shape");

        setShapeInputs(source_shapePtr);
        setShapeInputs(target_shapePtr);

        srcBlockInput->addSingleInput("distance", false, "source-target minimum distance");
    }

    EGS_COLLIMATED_SOURCE_EXPORT string getExample() {
        string example;
        example = {
            R"(
    # Example of egs_collimated_source
    #:start source:
        library = egs_collimated_source
        name = my_source
        :start source shape:
            type = point
            position = 0 0 5
        :stop source shape:
        :start target shape:
            library = egs_rectangle
            rectangle = -1 -1 1 1
        :stop target shape:
        distance = 5
        charge = -1
        :start spectrum:
            type = monoenergetic
            energy = 20
        :stop spectrum:
    :stop source:
)"};
        return example;
    }

    EGS_COLLIMATED_SOURCE_EXPORT shared_ptr<EGS_BlockInput> getInputs() {
        if(!inputSet) {
            setInputs();
        }
        return srcBlockInput;
    }

    EGS_COLLIMATED_SOURCE_EXPORT EGS_BaseSource *createSource(EGS_Input *input,
            EGS_ObjectFactory *f) {
        return
            createSourceTemplate<EGS_CollimatedSource>(input,f,"collimated source");
    }

}
