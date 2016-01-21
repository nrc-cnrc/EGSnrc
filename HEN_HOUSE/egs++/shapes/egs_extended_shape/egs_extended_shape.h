/*
###############################################################################
#
#  EGSnrc egs++ extended shape headers
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


/*! \file egs_extended_shape.h
 *  \brief An extended shape
 *  \IK
 */

#ifndef EGS_EXTENDED_SHAPE_
#define EGS_EXTENDED_SHAPE_

#include "egs_shapes.h"
#include "egs_rndm.h"

#ifdef WIN32

    #ifdef BUILD_EXTENDED_SHAPE_DLL
        #define EGS_EXTENDED_SHAPE_EXPORT __declspec(dllexport)
    #else
        #define EGS_EXTENDED_SHAPE_EXPORT __declspec(dllimport)
    #endif
    #define EGS_EXTENDED_SHAPE_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_EXTENDED_SHAPE_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_EXTENDED_SHAPE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_EXTENDED_SHAPE_EXPORT
        #define EGS_EXTENDED_SHAPE_LOCAL
    #endif

#endif

/*! \brief An extended shape

\ingroup Shapes

An extended shape is a shape that
takes a random point from another shape and then
adds \f$\Delta z\f$ to the z-component of the position vector,
where \f$\Delta z\f$ is picked uniformly between \f$z_1$ and $z_2\f$,
which are user inputs. Using this type of shape it is possible to
obtain \em e.g. a cylindrical shape from a circle, a box from a
rectangle, a prism from a polygon, etc. An extended shape is defined
using
\verbatim
:start shape:
    library = egs_extended_shape
    :start shape:
        definition of the shape to be 'extended'
    :stop shape:
    extension = z1 z2
:stop shape:
\endverbatim

*/
class EGS_EXTENDED_SHAPE_EXPORT EGS_ExtendedShape : public EGS_BaseShape {

public:

    EGS_ExtendedShape(EGS_BaseShape *Shape, EGS_Float H1, EGS_Float H2,
                      const string &Name="",EGS_ObjectFactory *f=0) :
        EGS_BaseShape(Name,f), shape(Shape), h1(H1), h2(H2), dh(h2-h1) {
        if (shape) {
            shape->ref();
            otype = "Extended ";
            otype += shape->getObjectType();
        }
        else {
            otype = "Invalid ExtendedShape";
        }
    };
    ~EGS_ExtendedShape() {
        EGS_Object::deleteObject(shape);
    };
    EGS_Vector getPoint(EGS_RandomGenerator *rndm) {
        EGS_Vector v(shape->getPoint(rndm));
        v.z += h1 + dh*rndm->getUniform();
        return v;
    };

protected:

    EGS_BaseShape  *shape;
    EGS_Float      h1, h2, dh;

};

#endif
