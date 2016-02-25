/*
###############################################################################
#
#  EGSnrc egs++ shape collection headers
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


/*! \file egs_shape_collection.h
 *  \brief A shape collection
 *  \IK
 */

#ifndef EGS_SHAPE_COLLECTION_
#define EGS_SHAPE_COLLECTION_

#include "egs_shapes.h"
#include "egs_alias_table.h"
#include "egs_rndm.h"

#ifdef WIN32

    #ifdef BUILD_SHAPE_COLLECTION_DLL
        #define EGS_SHAPE_COLLECTION_EXPORT __declspec(dllexport)
    #else
        #define EGS_SHAPE_COLLECTION_EXPORT __declspec(dllimport)
    #endif
    #define EGS_SHAPE_COLLECTION_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_SHAPE_COLLECTION_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_SHAPE_COLLECTION_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_SHAPE_COLLECTION_EXPORT
        #define EGS_SHAPE_COLLECTION_LOCAL
    #endif

#endif

/*! \brief A shape collection

  \ingroup Shapes

This is a shape made from
other shapes of any type \f$s_1, s_2, ..., s_n\f$ where the random position
is picked from the \f$s_j\f$'s with probabilities \f$p_j\f$ defined by
the user. A shape collection is defined using
\verbatim
:start shape:
    library = egs_shape_collection
    :start shape:
        definition of the first shape in the collection
    :stop shape:
    :start shape:
        definition of the second shape in the collection
    :stop shape:
    ...
    :start shape:
        definition of the last shape in the collection
    :stop shape:
    probabilities = p1 p2 ... pn
:stop shape:
\endverbatim

A shape collection can be used to pick surface points
if all of the shapes in the collection support the
getPointSourceDirection() method.

*/
class EGS_SHAPE_COLLECTION_EXPORT EGS_ShapeCollection : public EGS_BaseShape {

public:

    EGS_ShapeCollection(const vector<EGS_BaseShape *> &Shapes,
                        const vector<EGS_Float> &Probs, const string &Name="",
                        EGS_ObjectFactory *f=0);
    ~EGS_ShapeCollection() {
        if (nshape > 0) {
            for (int j=0; j<nshape; j++) {
                EGS_Object::deleteObject(shapes[j]);
            }
            delete [] shapes;
            delete table;
        }
    };
    EGS_Vector getPoint(EGS_RandomGenerator *rndm) {
        int j = table->sampleBin(rndm);
        return shapes[j]->getRandomPoint(rndm);
    };

    bool supportsDirectionMethod() const {
        for (int j=0; j<nshape; j++) {
            if (!shapes[j]->supportsDirectionMethod()) {
                return false;
            }
        }
        return true;
    };

    EGS_Float area() const {
        EGS_Float A = 0;
        for (int j=0; j<nshape; j++) {
            A += shapes[j]->area();
        }
        return A;
    };

    void getPointSourceDirection(const EGS_Vector &Xo,
                                 EGS_RandomGenerator *rndm, EGS_Vector &u, EGS_Float &wt) {
        EGS_Vector xo = T ? Xo*(*T) : Xo;
        int j = table->sampleBin(rndm);
        shapes[j]->getPointSourceDirection(xo,rndm,u,wt);
        if (T) {
            T->rotate(u);
        }
    };

protected:

    int            nshape;
    EGS_BaseShape  **shapes;
    EGS_AliasTable *table;

};

#endif
