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
#  Contributors:
#
###############################################################################
*/


/*! \file egs_collimated_source.cpp
 *  \brief A collimated isotropic source
 *  \IK
 */

#include "egs_collimated_source.h"
#include "egs_input.h"

EGS_CollimatedSource::EGS_CollimatedSource(EGS_Input *input,
        EGS_ObjectFactory *f) : EGS_BaseSimpleSource(input,f),
    source_shape(0), target_shape(0), dist(1), ctry(0) {
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
                       "shape definition and missing wrong 'source shape name' input\n");
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
                       "shape definition and missing wrong 'target shape name' input\n");
        else {
            target_shape = EGS_BaseShape::getShape(sname);
            if (!source_shape)
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

    EGS_COLLIMATED_SOURCE_EXPORT EGS_BaseSource *createSource(EGS_Input *input,
            EGS_ObjectFactory *f) {
        return
            createSourceTemplate<EGS_CollimatedSource>(input,f,"collimated source");
    }

}
