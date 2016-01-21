/*
###############################################################################
#
#  EGSnrc egs++ space geometry
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


/*! \file egs_space.cpp
 *  \brief The entire space as a geometry
 *  \IK
 */

#include "egs_space.h"
#include "egs_input.h"

string EGS_Space::type = "EGS_Space";

extern "C" {

    EGS_SPACE_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {
        EGS_Space *g = new EGS_Space("");
        g->setName(input);
        g->setMedia(input);
        g->setLabels(input);
        return g;
    }

}
