/*
###############################################################################
#
#  EGSnrc egs++ box geometry
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


/*! \file egs_box.cpp
 *  \brief A box geometry: implementation
 *  \IK
 */

#include "egs_box.h"
#include "egs_input.h"

void EGS_Box::printInfo() const {
    EGS_BaseGeometry::printInfo();
    egsInformation(" box size = %g %g %g\n",ax,ay,az);
    egsInformation("=======================================================\n");
}

string EGS_Box::type("EGS_Box");

static char EGS_BOX_LOCAL ebox_message1[] = "createGeometry(box): %s\n";
static char EGS_BOX_LOCAL ebox_message2[] = "null input?";
static char EGS_BOX_LOCAL ebox_message3[] = "wrong/missing 'box size' input?";
static char EGS_BOX_LOCAL ebox_message4[] =
    "expecting 1 or 3 float inputs for 'box size'";
static char EGS_BOX_LOCAL ebox_key1[] = "box size";

extern "C" {

    EGS_BOX_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {
        if (!input) {
            egsWarning(ebox_message1,ebox_message2);
            return 0;
        }
        vector<EGS_Float> s;
        int err = input->getInput(ebox_key1,s);
        if (err) {
            egsWarning(ebox_message1,ebox_message3);
            return 0;
        }
        EGS_AffineTransform *t = EGS_AffineTransform::getTransformation(input);
        EGS_Box *result;
        if (s.size() == 1) {
            result = new EGS_Box(s[0],t);
        }
        else if (s.size() == 3) {
            result = new EGS_Box(s[0],s[1],s[2],t);
        }
        else {
            egsWarning(ebox_message1,ebox_message4);
            if (t) {
                delete t;
            }
            return 0;
        }
        if (t) {
            delete t;
        }
        result->setName(input);
        result->setMedia(input);
        result->setLabels(input);
        return result;
    }

}
