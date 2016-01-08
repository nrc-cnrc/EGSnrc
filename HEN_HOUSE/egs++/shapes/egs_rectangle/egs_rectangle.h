/*
###############################################################################
#
#  EGSnrc egs++ rectangle shape headers
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


/*! \file egs_rectangle.h
 *  \brief Rectangular shape
 *  \IK
 */

#ifndef EGS_RECTANGLE_
#define EGS_RECTANGLE_

#include "egs_shapes.h"
#include "egs_rndm.h"

#ifdef WIN32

    #ifdef BUILD_RECTANGLE_DLL
        #define EGS_RECTANGLE_EXPORT __declspec(dllexport)
    #else
        #define EGS_RECTANGLE_EXPORT __declspec(dllimport)
    #endif
    #define EGS_RECTANGLE_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_RECTANGLE_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_RECTANGLE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_RECTANGLE_EXPORT
        #define EGS_RECTANGLE_LOCAL
    #endif

#endif

/*! \brief A rectangular shape

  \ingroup Shapes
  \ingroup SurfaceS

A rectangular shape is defined via
\verbatim
:start shape:
    library = egs_rectangle
    rectangle = x1 y1 x2 y2
    inner rectangle = xp1 yp1 xp2 yp2 (optional)
:stop shape:
\endverbatim
If the optional <code>inner rectangle</code> input is missing, the
points will be uniformly distributed in the rectangle with
the left-upper corner <code>x1,y1</code> and the right-lower corner
<code>x2,y2</code>. If the <code>inner rectangle</code> key is present,
<code>xp1,yp1</code> and <code>xp2,yp2</code> are the left-upper corner and
right-lower corner of the inner rectangle so that the points will
be distributed within the ``rectangular ring'' defined by the two rectangles.
In this later case the actual object being constructed will be of type
EGS_RectangularRing.
*/
class EGS_RECTANGLE_EXPORT EGS_RectangleShape : public EGS_SurfaceShape {

public:

    EGS_RectangleShape(EGS_Float Xmin, EGS_Float Xmax, EGS_Float Ymin,
                       EGS_Float Ymax, const string &Name="",EGS_ObjectFactory *f=0) :
        EGS_SurfaceShape(Name,f), xmin(Xmin), xmax(Xmax), ymin(Ymin),
        ymax(Ymax) {
        EGS_Float tmp;
        if (xmin > xmax) {
            tmp = xmax;
            xmax = xmin;
            xmin = tmp;
        }
        if (ymin > ymax) {
            tmp = ymax;
            ymax = ymin;
            ymin = tmp;
        }
        dx = xmax - xmin;
        dy = ymax - ymin;
        otype = "rectangle";
        A = dx*dy;
    };
    ~EGS_RectangleShape() {};
    EGS_Vector getPoint(EGS_RandomGenerator *rndm) {
        return EGS_Vector(xmin + dx*rndm->getUniform(),
                          ymin + dy*rndm->getUniform(),
                          0);
    };

protected:

    EGS_Float xmin, xmax, ymin, ymax, dx, dy;
};

/*! \brief A "rectangular ring"

  \ingroup Shapes
  \ingroup SurfaceS

  See EGS_RectangleShape for more details.
*/
class EGS_RECTANGLE_EXPORT EGS_RectangularRing : public EGS_SurfaceShape {

public:

    EGS_RectangularRing(EGS_Float Xmin, EGS_Float Xmax, EGS_Float Ymin,
                        EGS_Float Ymax, EGS_Float Xmin_i, EGS_Float Xmax_i, EGS_Float Ymin_i,
                        EGS_Float Ymax_i, const string &Name="",EGS_ObjectFactory *f=0);
    ~EGS_RectangularRing();
    bool isValid() const {
        return valid;
    };
    EGS_Vector getPoint(EGS_RandomGenerator *rndm) {
        EGS_Float eta = rndm->getUniform();
        int j=0;
        while (eta > p[j]) {
            j++;
        }
        return r[j]->getPoint(rndm);
    };

protected:

    EGS_RectangleShape  *r[4];
    EGS_Float            p[4];
    bool                 valid;

};

#endif
