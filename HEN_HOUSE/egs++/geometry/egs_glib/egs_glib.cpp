/*
###############################################################################
#
#  EGSnrc egs++ glib geometry
#  Copyright (C) 2016 Randle E. P. Taylor, Rowan M. Thomson,
#  Marc J. P. Chamberland, D. W. O. Rogers
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
#  Author:          Randle Taylor, 2016
#
#  Contributors:    Marc Chamberland
#                   Rowan Thomson
#                   Dave Rogers
#
###############################################################################
#
#  egs_glib was developed for the Carleton Laboratory for Radiotherapy Physics.
#
###############################################################################
*/


/*! \file egs_glib.cpp
 *  \brief The createGeometry function for the library shim
 *  \author Randle Taylor (randle.taylor@gmail.com)
 **/

#include <map>
#include <iostream>
#include <istream>
#include <sstream>
#include <fstream>
#include "egs_input.h"
#include "egs_functions.h"
#include "egs_glib.h"


extern "C" {

    /*! createGeometry function for glib shim */
    EGS_GLIB_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {

        if (!input) {
            egsWarning("createGeometry(egs_glib): null input?\n");
            return 0;
        }


        EGS_Input *egs_geom_input = input->takeInputItem("geometry definition");
        if (!egs_geom_input) {
            egsWarning("createGeometry(egs_glib): missing `geometry definition` input\n");
            return 0;
        }

        EGS_BaseGeometry *final = EGS_BaseGeometry::createGeometry(egs_geom_input);

        delete egs_geom_input;

        if (!final) {
            egsWarning("createGeometry(egs_glib): unable to create geometry\n");
            return 0;
        }

        final->setName(input);

        return final;

    }

}
