/*
###############################################################################
#
#  EGSnrc egs++ angular spread source
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
#  Author:          Iwan Kawrakow, 2009
#
#  Contributors:
#
###############################################################################
*/


/*! \file  egs_angular_spread_source.cpp
 *  \brief Adds angular spread to the directions of particles from some other source
 *  \IK
 */

#include "egs_angular_spread_source.h"
#include "egs_input.h"
#include "egs_math.h"

EGS_AngularSpreadSource::EGS_AngularSpreadSource(EGS_Input *input,
        EGS_ObjectFactory *f) : EGS_BaseSource(input,f), source(0), sigma(0) {
    EGS_Input *isource = input->takeInputItem("source",false);
    if (isource) {
        source = EGS_BaseSource::createSource(isource);
        delete isource;
    }
    if (!source) {
        string sname;
        int err = input->getInput("source name",sname);
        if (err)
            egsWarning("EGS_AngularSpreadSource: missing/wrong inline source "
                       "definition and missing wrong 'source name' input\n");
        else {
            source = EGS_BaseSource::getSource(sname);
            if (!source) egsWarning("EGS_AngularSpreadSource: a source named %s"
                                        " does not exist\n");
        }
    }
    int err = input->getInput("sigma",sigma);
    if (!err) {
        if (sigma < 0) {
            sigma = -0.4246609001440095285*sigma;
        }
        sigma *= M_PI/180;
        sigma *= sigma;
    }
    setUp();
}

void EGS_AngularSpreadSource::setUp() {
    otype = "EGS_AngularSpreadSource";
    if (!isValid()) {
        description = "Invalid angular spread source";
        return;
    }
    description = "Angular spread added to a ";
    description += source->getSourceDescription();
    source->ref();
}

extern "C" {

    EGS_ANGULAR_SPREAD_SOURCE_EXPORT EGS_BaseSource *createSource(EGS_Input *input,
            EGS_ObjectFactory *f) {
        return
            createSourceTemplate<EGS_AngularSpreadSource>(input,f,"angular spread source");
    }

}
