/*
###############################################################################
#
#  EGSnrc egs++ gtransformed geometry
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
#                   Ernesto Mainegra-Hing
#                   Hubert Ho
#                   Marc Chamberland
#                   Hannah Gallop
#
###############################################################################
*/


/*! \file egs_gtransformed.cpp
 *  \brief A transformed geometry: implementation
 *  \IK
 */


#include "egs_gtransformed.h"
#include "egs_input.h"
#include "egs_functions.h"

static bool EGS_GTRANSFORMED_LOCAL inputSet = false;

void EGS_TransformedGeometry::setMedia(EGS_Input *,int,const int *) {
    egsWarning("EGS_TransformedGeometry::setMedia: don't use this method. Use the\n"
               " setMedia() methods of the geometry objects that make up this geometry\n");
}

void EGS_TransformedGeometry::setRelativeRho(int start, int end, EGS_Float rho) {
    setRelativeRho(0);
}

void EGS_TransformedGeometry::setRelativeRho(EGS_Input *) {
    egsWarning("EGS_TransformedGeometry::setRelativeRho(): don't use this "
               "method. Use the \n setRelativeRho() methods of the underlying "
               "geometry\n");
}

void EGS_TransformedGeometry::setBScaling(int start, int end, EGS_Float rho) {
    setBScaling(0);
}

void EGS_TransformedGeometry::setBScaling(EGS_Input *) {
    egsWarning("EGS_TransformedGeometry::setBScaling(): don't use this "
               "method. Use the \n setBScaling() methods of the underlying "
               "geometry\n");
}

extern "C" {

    static void setInputs() {
        inputSet = true;

        setBaseGeometryInputs(false);

        geomBlockInput->getSingleInput("library")->setValues({"EGS_GTransformed"});

        // Format: name, isRequired, description, vector string of allowed values
        geomBlockInput->addSingleInput("my geometry", true, "The name of a previously defined geometry");

        auto blockPtr = geomBlockInput->addBlockInput("transformation");
        blockPtr->addSingleInput("translation", false, "The translation for the geometry (x, y ,z)");
        auto rotPtr = blockPtr->addSingleInput("rotation", false, "2, 3, or 9 floating point numbers");
        auto vectPtr = blockPtr->addSingleInput("rotation vector", false, "3 floating point numbers");
        // Can either have "rotation" or "rotation vector"
        rotPtr->addDependency(vectPtr, "", true);
        vectPtr->addDependency(rotPtr, "", true);
    }

    EGS_GTRANSFORMED_EXPORT string getExample(string type) {
        string example;
        example = {
            R"(
    # Example of egs_gtransformed
    #:start geometry:
        name = my_gtransform
        library = egs_gtransformed
        my geometry = geom
        # created a geometry called geom
        :start transformation:
            translation = 0 0.5 0
            rotation = 0.05 0 -1
        :stop transformation:
    :stop geometry:
)"};
        return example;
    }

    EGS_GTRANSFORMED_EXPORT shared_ptr<EGS_BlockInput> getInputs() {
        if(!inputSet) {
            setInputs();
        }
        return geomBlockInput;
    }

    EGS_GTRANSFORMED_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {
        EGS_BaseGeometry *g = 0;
        EGS_Input *ij = input->takeInputItem("geometry",false);
        if (ij) {
            g = EGS_BaseGeometry::createSingleGeometry(ij);
            delete ij;
            if (!g) {
                egsWarning("createGeometry(gtransformed): got a null pointer"
                           " as a geometry?\n");
                return 0;
            }
        }
        if (!g) {
            string gname;
            int err = input->getInput("my geometry",gname);
            if (err) {
                egsWarning(
                    "createGeometry(gtransformed): my geometry must be defined\n"
                    "  either inline or using 'my geometry = some_name'\n");
                return 0;
            }
            g = EGS_BaseGeometry::getGeometry(gname);
            if (!g) {
                egsWarning("createGeometry(gtransformed): no geometry named %s"
                           " is defined\n",gname.c_str());
                return 0;
            }
        }
        g->ref();
        EGS_AffineTransform *t = EGS_AffineTransform::getTransformation(input);
        EGS_BaseGeometry *result;
        if (!t) {
            egsWarning("createGeometry(gtransformed): null transformation."
                       " I hope you know what you are doing\n");
            result = new EGS_TransformedGeometry(g,EGS_AffineTransform());
        }
        else {
            if (t->isI()) egsWarning("createGeometry(gtransformed): "
                                         "unity transformation. I hope you know what you are doing\n");
            result = new EGS_TransformedGeometry(g,*t);
            delete t;
        }
        result->setName(input);
        result->setBoundaryTolerance(input);
        result->setLabels(input);
        return result;

    }


    void EGS_TransformedGeometry::getLabelRegions(const string &str, vector<int> &regs) {

        // label defined in the geometry being transformed
        g->getLabelRegions(str, regs);

        // label defined in self (transformation input block)
        EGS_BaseGeometry::getLabelRegions(str, regs);

    }


}
