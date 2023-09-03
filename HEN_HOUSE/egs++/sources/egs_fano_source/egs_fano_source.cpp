/*
###############################################################################
#
#  EGSnrc egs++ Fano source
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
#  Authors:         Ernesto Mainegra-Hing, 2016
#                   Hugo Bouchard, 2016
#
#  Contributors:    Reid Townson
#
###############################################################################
*/


/*! \file egs_fano_source.cpp
 *  \brief An Fano source
 *  \EM
 *  The Fano option allows have uniform particles per unit mass
 */

#include "egs_fano_source.h"
#include "egs_input.h"
#include "egs_math.h"
#include <sstream>

EGS_FanoSource::EGS_FanoSource(EGS_Input *input,
                               EGS_ObjectFactory *f) :
    EGS_BaseSimpleSource(input,f), shape(0), geom(0),
    regions(0), nrs(0), min_theta(0), max_theta(M_PI), min_phi(0), max_phi(2*M_PI),
    max_mass_density(0.0) {

    vector<EGS_Float> pos;
    EGS_Input *ishape = input->takeInputItem("shape");
    if (ishape) {
        egsWarning("EGS_FanoSource: trying to construct the shape\n");
        shape = EGS_BaseShape::createShape(ishape);
        delete ishape;
    }
    if (!shape) {
        string sname;
        int err = input->getInput("shape name",sname);
        if (err)
            egsWarning("EGS_FanoSource: missing/wrong inline shape "
                       "definition and missing wrong 'shape name' input\n");
        else {
            shape = EGS_BaseShape::getShape(sname);
            if (!shape) egsWarning("EGS_FanoSource: a shape named %s"
                                       " does not exist\n");
        }
    }
    string geom_name;
    int err = input->getInput("geometry",geom_name);
    if (!err) {
        geom = EGS_BaseGeometry::getGeometry(geom_name);
        if (!geom) egsFatal("EGS_FanoSource: no geometry named %s in input file!\n",
                                geom_name.c_str());
        else {
            int errF = input->getInput("max mass density", max_mass_density);
            if (errF) {
                egsFatal("EGS_FanoSource: A Fano source requires a maximum density input.\n");
            }
        }
    }
    else {
        egsFatal("EGS_FanoSource: A Fano source requires a valid geometry name.\n");
    }

    EGS_Float tmp_theta;
    err = input->getInput("min theta", tmp_theta);
    if (!err) {
        min_theta = tmp_theta/180.0*M_PI;
    }

    err = input->getInput("max theta", tmp_theta);
    if (!err) {
        max_theta = tmp_theta/180.0*M_PI;
    }

    err = input->getInput("min phi", tmp_theta);
    if (!err) {
        min_phi = tmp_theta/180.0*M_PI;
    }

    err = input->getInput("max phi", tmp_theta);
    if (!err) {
        max_phi = tmp_theta/180.0*M_PI;
    }

    buf_1 = cos(min_theta);
    buf_2 = cos(max_theta);

    setUp();
}

void EGS_FanoSource::setUp() {
    otype = "EGS_FanoSource";
    if (!isValid()) {
        description = "Invalid Fano source";
    }
    else {
        description = "Fano source from a shape of type ";
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
        ostringstream str_density;
        str_density << scientific << max_mass_density;
        description += "\n maximum density = " + str_density.str() + "  g/cm3";
        description += "\n Fano geometry   = " + geom->getName();
        if (geom) {
            geom->ref();
        }
    }
}

extern "C" {

    EGS_FANO_SOURCE_EXPORT EGS_BaseSource *createSource(EGS_Input *input,
            EGS_ObjectFactory *f) {
        return createSourceTemplate<EGS_FanoSource>(input, f, "fano source");
    }

}
