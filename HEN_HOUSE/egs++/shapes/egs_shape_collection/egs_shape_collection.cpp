/*
###############################################################################
#
#  EGSnrc egs++ shape collection
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


/*! \file egs_shape_collection.cpp
 *  \brief A shape collection
 *  \IK
 */

#include "egs_shape_collection.h"
#include "egs_input.h"
#include "egs_functions.h"

EGS_ShapeCollection::EGS_ShapeCollection(const vector<EGS_BaseShape *> &Shapes,
        const vector<EGS_Float> &Probs, const string &Name, EGS_ObjectFactory *f) :
    EGS_BaseShape(Name,f), nshape(0) {
    int n1 = Shapes.size(), n2 = Probs.size();
    if (n1 < 2 || n2 < 2 || n1 != n2) {
        egsWarning("EGS_ShapeCollection::EGS_ShapeCollection: invalid input\n");
        return;
    }
    nshape = n1;
    shapes = new EGS_BaseShape* [nshape];
    EGS_Float *dum = new EGS_Float [nshape], *p = new EGS_Float [nshape];
    for (int j=0; j<nshape; j++) {
        shapes[j] = Shapes[j];
        shapes[j]->ref();
        p[j] = Probs[j];
        dum[j] = 1;
    }
    table = new EGS_AliasTable(nshape,dum,p,0);
    delete [] dum;
    delete [] p;
}

extern "C" {

    EGS_SHAPE_COLLECTION_EXPORT EGS_BaseShape *createShape(EGS_Input *input,
            EGS_ObjectFactory *f) {
        if (!input) {
            egsWarning("createShape(shape collection): null input?\n");
            return 0;
        }
        vector<EGS_BaseShape *> shapes;
        vector<EGS_Float> probs;
        EGS_Input *ishape;
        while ((ishape = input->takeInputItem("shape",false))) {
            EGS_BaseShape *shape = EGS_BaseShape::createShape(ishape);
            if (!shape) {
                egsWarning("createShape(shape collection): got null shape\n");
            }
            else {
                shapes.push_back(shape);
            }
            delete ishape;
        }
        vector<string> snames;
        int err = input->getInput("shape names",snames);
        if (!err && snames.size() > 0) {
            for (unsigned int j=0; j<snames.size(); j++) {
                EGS_BaseShape *shape = EGS_BaseShape::getShape(snames[j]);
                if (!shape) egsWarning("createShape(shape collection): no shape "
                                           "named %s exists\n",snames[j].c_str());
                else {
                    shapes.push_back(shape);
                }
            }
        }
        err = input->getInput("probabilities",probs);
        bool ok = true;
        if (err) {
            egsWarning("createShape(shape collection): no 'probabilities' input\n");
            ok = false;
        }
        if (shapes.size() < 2) {
            egsWarning("createShape(shape collection): at least 2 shapes are "
                       "needed for a shape collection, you defined %d\n",shapes.size());
            ok = false;
        }
        if (shapes.size() != probs.size()) {
            egsWarning("createShape(shape collection): the number of shapes (%d)"
                       " is not the same as the number of input probabilities (%d)\n");
            ok = false;
        }
        for (unsigned int i=0; i<probs.size(); i++) {
            if (probs[i] < 0) {
                egsWarning("createShape(shape collection): probabilities must not"
                           " be negative, but your input probability %d is %g\n",
                           i,probs[i]);
                ok = false;
            }
        }
        if (!ok) {
            for (unsigned int j=0; j<shapes.size(); j++) {
                EGS_Object::deleteObject(shapes[j]);
            }
            return 0;
        }
        EGS_ShapeCollection *s = new EGS_ShapeCollection(shapes,probs,"",f);
        s->setName(input);
        s->setTransformation(input);
        return s;
    }

}
