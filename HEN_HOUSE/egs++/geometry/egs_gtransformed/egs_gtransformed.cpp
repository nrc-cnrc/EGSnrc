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

extern "C" {

    EGS_GTRANSFORMED_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {
        int error = 0;
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
