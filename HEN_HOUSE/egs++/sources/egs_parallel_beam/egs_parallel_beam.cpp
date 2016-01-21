/*
###############################################################################
#
#  EGSnrc egs++ parallel beam source
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


/*! \file egs_parallel_beam.cpp
 *  \brief A parallel beam
 *  \IK
 */

#include "egs_parallel_beam.h"
#include "egs_input.h"

EGS_ParallelBeam::EGS_ParallelBeam(EGS_Input *input,
                                   EGS_ObjectFactory *f) : EGS_BaseSimpleSource(input,f), shape(0), uo(0,0,1) {
    vector<EGS_Float> dir;
    if (!input->getInput("direction",dir)) {
        if (dir.size() != 3) egsWarning("EGS_ParallelBeam: you must input"
                                            " 3 numbers in the 'direction' input\n"
                                            "  but I got %d => ignoring the input\n",dir.size());
        else {
            uo.x = dir[0];
            uo.y = dir[1];
            uo.z = dir[2];
            EGS_Float norm = uo.length();
            if (norm < 1e-6) {
                egsWarning("EGS_ParallelBeam: the length of the direction"
                           " vector can not be zero => ignoring your input\n");
                uo.x = 0;
                uo.y = 0;
                uo.z = 1;
            }
            else {
                uo *= (1./norm);
            }
        }
    }
    EGS_Input *ishape = input->takeInputItem("shape");
    if (ishape) {
        //egsWarning("EGS_ParallelBeam: trying to construct the shape\n");
        shape = EGS_BaseShape::createShape(ishape);
        delete ishape;
    }
    if (!shape) {
        string sname;
        int err = input->getInput("shape name",sname);
        if (err)
            egsWarning("EGS_ParallelBeam: missing/wrong inline shape "
                       "definition and missing wrong 'shape name' input\n");
        else {
            shape = EGS_BaseShape::getShape(sname);
            if (!shape) egsWarning("EGS_ParallelBeam: a shape named %s"
                                       " does not exist\n");
        }
    }
    setUp();
}

void EGS_ParallelBeam::setUp() {
    otype = "EGS_ParallelBeam";
    if (!isValid()) {
        description = "Invalid parallel beam";
    }
    else {
        description = "Parallel beam from a shape of type ";
        description += shape->getObjectType();
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

    EGS_PARALLEL_BEAM_EXPORT EGS_BaseSource *createSource(EGS_Input *input,
            EGS_ObjectFactory *f) {
        return createSourceTemplate<EGS_ParallelBeam>(input,f,"parallel beam");
    }

}
