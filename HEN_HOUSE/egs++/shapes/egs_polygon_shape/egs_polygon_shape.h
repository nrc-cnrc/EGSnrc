/*
###############################################################################
#
#  EGSnrc egs++ polygon shape headers
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
#  Contributors:    Randle Taylor
#
###############################################################################
*/


/*! \file egs_polygon_shape.h
 *  \brief A polygon shape
 *  \IK
 */

#ifndef EGS_POLYGON_SHAPE_
#define EGS_POLYGON_SHAPE_

#include "egs_shapes.h"
#include "egs_rndm.h"
#include "egs_alias_table.h"

#ifdef WIN32

    #ifdef BUILD_POLYGON_SHAPE_DLL
        #define EGS_POLYGON_SHAPE_EXPORT __declspec(dllexport)
    #else
        #define EGS_POLYGON_SHAPE_EXPORT __declspec(dllimport)
    #endif
    #define EGS_POLYGON_SHAPE_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_POLYGON_SHAPE_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_POLYGON_SHAPE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_POLYGON_SHAPE_EXPORT
        #define EGS_POLYGON_SHAPE_LOCAL
    #endif

#endif

/*! \brief A triangular shape.

  \ingroup Shapes
  \ingroup SurfaceS

A triangle shape is a special case of a \link EGS_PolygonShape
polygon shape. \endlink
*/
class EGS_POLYGON_SHAPE_EXPORT EGS_TriangleShape : public EGS_SurfaceShape {

public:

    EGS_TriangleShape(const vector<EGS_Float> &points, const string &Name="",
                      EGS_ObjectFactory *f=0);
    EGS_TriangleShape(const EGS_Float *points, const string &Name="",
                      EGS_ObjectFactory *f=0);
    ~EGS_TriangleShape() {};
    EGS_Vector getPoint(EGS_RandomGenerator *rndm) {
        EGS_Float eta_a = sqrt(rndm->getUniform());
        EGS_Float eta_b = eta_a*rndm->getUniform();
        eta_a = 1 - eta_a;
        return EGS_Vector(xo + ax*eta_a + bx*eta_b,
                          yo + ay*eta_a + by*eta_b,
                          0);
    };

protected:

    EGS_Float xo, yo, ax, ay, bx, by;

};


/*! \brief A polygon shape.

  \ingroup Shapes
  \ingroup SurfaceS

A polygon shape is defined using
\verbatim
:start shape:
    library = egs_polygon_shape
    points = list of at least 3 2D points (i.e. at least 6 floating numbers)
:stop shape:
\endverbatim
and delivers points uniformely distributed
within a polygon in the xy-plane at z=0.
When 3 2D points are given, the actual shape being
constructed will be EGS_TriangleShape.
For polygons in other planes, attach an \link EGS_AffineTransform affine
transformation\endlink to the shape.
*/
class EGS_POLYGON_SHAPE_EXPORT EGS_PolygonShape : public EGS_SurfaceShape {

public:

    EGS_PolygonShape(const vector<EGS_Float> &points, const string &Name="",
                     EGS_ObjectFactory *f=0);
    ~EGS_PolygonShape() {
        if (n > 0) {
            for (int j=0; j<n-2; j++) {
                delete triangle[j];
            }
            delete [] triangle;
            delete table;
        }
    };
    EGS_Vector getPoint(EGS_RandomGenerator *rndm) {
        int j = table->sampleBin(rndm);
        return triangle[j]->getPoint(rndm);
    };

protected:

    int  n;  // number of triangles
    EGS_TriangleShape **triangle;
    EGS_AliasTable    *table;

};

#endif
