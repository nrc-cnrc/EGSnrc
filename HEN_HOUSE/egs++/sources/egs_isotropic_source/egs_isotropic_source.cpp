/*
###############################################################################
#
#  EGSnrc egs++ isotropic source
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
#  Contributors:    Long Zhang
#                   Hubert Ho
#                   Reid Townson
#
###############################################################################
*/


/*! \file egs_isotropic_source.cpp
 *  \brief An isotropic source
 *  \IK
 */

#include "egs_isotropic_source.h"
#include "egs_input.h"
#include "egs_math.h"

static string EGS_ISOTROPIC_SOURCE_LOCAL typeStr("EGS_Isotropic_Source");
static bool EGS_ISOTROPIC_SOURCE_LOCAL inputSet = false;

EGS_IsotropicSource::EGS_IsotropicSource(EGS_Input *input,
        EGS_ObjectFactory *f) : EGS_BaseSimpleSource(input,f), shape(0), geom(0),
    regions(0), min_theta(0), max_theta(M_PI), min_phi(0), max_phi(2*M_PI),
    nrs(0), gc(IncludeAll) {
    vector<EGS_Float> pos;
    EGS_Input *ishape = input->takeInputItem("shape");
    if (ishape) {
        shape = EGS_BaseShape::createShape(ishape);
        delete ishape;
    }
    if (!shape) {
        string sname;
        int err = input->getInput("shape name",sname);
        if (err)
            egsWarning("EGS_IsotropicSource: missing/wrong inline shape "
                       "definition and missing wrong 'shape name' input\n");
        else {
            shape = EGS_BaseShape::getShape(sname);
            if (!shape) egsWarning("EGS_IsotropicSource: a shape named %s"
                                       " does not exist\n");
        }
    }
    string geom_name;
    int err = input->getInput("geometry",geom_name);
    if (!err) {
        geom = EGS_BaseGeometry::getGeometry(geom_name);
        if (!geom) egsWarning("EGS_IsotropicSource: no geometry named %s\n",
                                  geom_name.c_str());
        else {
            vector<string> reg_options;
            reg_options.push_back("IncludeAll");
            reg_options.push_back("ExcludeAll");
            reg_options.push_back("IncludeSelected");
            reg_options.push_back("ExcludeSelected");
            gc = (GeometryConfinement) input->getInput("region selection",reg_options,0);
            if (gc == IncludeSelected || gc == ExcludeSelected) {
                vector<int> regs;
                err = input->getInput("selected regions",regs);
                if (err || regs.size() < 1) {
                    egsWarning("EGS_IsotropicSource: region selection %d used "
                               "but no 'selected regions' input found\n",gc);
                    gc = gc == IncludeSelected ? IncludeAll : ExcludeAll;
                    egsWarning(" using %d\n",gc);
                }
                nrs = regs.size();
                regions = new int [nrs];
                for (int j=0; j<nrs; j++) {
                    regions[j] = regs[j];
                }
            }
        }
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

void EGS_IsotropicSource::setUp() {
    otype = "EGS_IsotropicSource";
    if (!isValid()) {
        description = "Invalid isotropic source";
    }
    else {
        description = "Isotropic source from a shape of type ";
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

        if (geom) {
            geom->ref();
        }
    }
}

extern "C" {

    static void setInputs() {
        inputSet = true;

        setBaseSourceInputs();

        srcBlockInput->getSingleInput("library")->setValues(vector<string>(1, typeStr));

        auto shapePtr = srcBlockInput->addBlockInput("shape");
        /* Commented out because I don't think this input is used
        Also at this point dependency of a block on an input hasn't been implemented
        auto shapeNamePtr = srcBlockInput->addSingleInput("shape name", false, "TODO");
        shapeNamePtr->addDependency(shapePtr, true);
        shapePtr->addDependency(shapeNamePtr, true);*/

        setShapeInputs(shapePtr);

        auto geomPtr = srcBlockInput->addSingleInput("geometry", false, "The name of a geometry, used for complex source shapes. Only particles generated inside the geometry or some of its regions are used.");
        auto regPtr = srcBlockInput->addSingleInput("region selection", false, "Include or exclude regions from the named geometry, to define a volume for source particle generation.", {"IncludeAll", "ExcludeAll","IncludeSelected","ExcludeSelected"});
        regPtr->addDependency(geomPtr);
        auto selPtr = srcBlockInput->addSingleInput("selected regions", false, "If region selection = IncludeSelected or ExcludeSelected, then this is a list of the regions in the named geometry to include or exclude.");
        selPtr->addDependency(geomPtr);
        selPtr->addDependency(regPtr);
        srcBlockInput->addSingleInput("min theta", false, "The minimum theta angle in degrees, to restrict the directions of source particles. Defaults to 0.");
        srcBlockInput->addSingleInput("max theta", false, "The maximum theta angle in degrees, to restrict the directions of source particles. Defaults to 180.");
        srcBlockInput->addSingleInput("min phi", false, "The minimum phi angle in degrees, to restrict the directions of source particles. Defaults to 0.");
        srcBlockInput->addSingleInput("max phi", false, "The maximum phi angle in degrees, to restrict the directions of source particles. Defaults to 360.");
    }

    EGS_ISOTROPIC_SOURCE_EXPORT string getExample() {
        string example
{R"(
    :start source:
        name                = my_source
        library             = egs_isotropic_source
        charge              = 0
        geometry            = my_envelope
        region selection    = IncludeSelected
        selected regions    = 1 2
        :start shape:
            type     = box
            box size    = 1 2 3
            :start media input:
                media = H2O521ICRU
            :stop media input:
        :stop shape:
        :start spectrum:
            type = monoenergetic
            energy = 1
        :stop spectrum:
    :stop source:
)"};
        return example;
    }

    EGS_ISOTROPIC_SOURCE_EXPORT shared_ptr<EGS_BlockInput> getInputs() {
        if(!inputSet) {
            setInputs();
        }
        return srcBlockInput;
    }

    EGS_ISOTROPIC_SOURCE_EXPORT EGS_BaseSource *createSource(EGS_Input *input,
            EGS_ObjectFactory *f) {
        return
            createSourceTemplate<EGS_IsotropicSource>(input,f,"isotropic source");
    }

}
