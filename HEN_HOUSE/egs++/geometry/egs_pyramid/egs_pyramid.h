/*
###############################################################################
#
#  EGSnrc egs++ pyramid geometry headers
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
#  Contributors:    Blake Walters
#
###############################################################################
*/


/*! \file egs_pyramid.h
 *  \brief A pyramid geometry: header
 *  \IK
 */

#ifndef EGS_PYRAMID_
#define EGS_PYRAMID_

#include "egs_base_geometry.h"
#include "egs_polygon.h"

#ifdef WIN32

    #ifdef BUILD_PYRAMID_DLL
        #define EGS_PYRAMID_EXPORT __declspec(dllexport)
    #else
        #define EGS_PYRAMID_EXPORT __declspec(dllimport)
    #endif
    #define EGS_PYRAMID_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_PYRAMID_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_PYRAMID_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_PYRAMID_EXPORT
        #define EGS_PYRAMID_LOCAL
    #endif

#endif

/*! \brief A template class for modeling pyramids.

\ingroup Geometry
\ingroup ElementaryG

This template class models pyramids with specializations for
pyramids having their bases in the X-, Y-, Z- and any plane.
The class can be used to model "open"
pyramids (pyramids that extend to infinity in the direction pointing
from the tip of the pyramid towards the plane used to define the pyramid
base) or closed pyramids. The pyramid bases can be arbitrary closed polygons.
Pyramid geometries have a single region (inside the pyramid).
They are defined via the following set of keys:

\verbatim
library = egs_pyramid
type = EGS_PyramidX or EGS_PyramidY or EGS_PyramidZ or EGS_Pyramid
points = list of 2D or 3D positions
tip    = Ox, Oy, Oz     # 3D position of the tip of the pyramid.
closed = 0 or 1         # optional
\endverbatim

As with \link EGS_PrismT prisms \endlink,
when a X-, Y- or Z-pyramid is being constructed,
the list of inputs
for the \c points key is considered to be a list of 2D positions
in the X-, Y- or Z-planes through the origin (and therefore
the number of floating point numbers input must be even). When a
EGS_Pyramid object is constructed, the list of inputs
for the \c points key is considered to be a list of 3D positions
(and therefore, the number of floating point numbers input must be divisible
by 3). In this case, all points must be on a single plane, otherwise
the construction of an EGS_Pyramid is rejected. The
\c tip key defines the 3D position of the tip of the pyramid.
The \c closed key is optional. When missing or set to 0, the
pyramid is open, otherwise it is closed by the plane in which the
pyramid base is defined.

The \c pyramid.geom example geometry file demonstrate the use
of a pyramid object.

*/
template <class T>
class EGS_PYRAMID_EXPORT EGS_PyramidT : public EGS_BaseGeometry {

protected:

    T             *p;   //!< the base polygon
    EGS_Vector    xo;   //!< the tip of the pyramid
    EGS_2DVector  xop;  //!< the tip projection on the base polygon
    EGS_Vector    a;    //!< the base normal vector.
    EGS_Float     d;    //!< distance from tip to base polygon (always positive)
    int           n;    //!< number of sides
    EGS_Polygon   **s;  //!< sides.
    bool          open; //!< is the pyramid open ?

public:

    /*! \brief Construct a pyramid using \a P as the base polygon and
    \a Xo as the position of the tip.

    If \a O is \c true, the pyramid is open (\em i.e. extends to infinity
    in the direction from the tip towards the pyramid base), otherwise
    it is closed by the base polygon.
    */
    EGS_PyramidT(T *P, const EGS_Vector &Xo, bool O=true, const string &N="");

    ~EGS_PyramidT() {
        delete p;
        for (int j=0; j<n; j++) {
            delete s[j];
        }
        delete [] s;
    };

    bool isInside(const EGS_Vector &x) {
        EGS_Vector xp(x-xo);
        EGS_Float axp = a*xp;
        if (axp > 0) {
            return false;
        }
        //if( axp > -1e-10 ) return true;
        if (!open  && d + axp < 0) {
            return false;
        }
        EGS_Float t = -d/axp;
        //return p->isInside2D(xop+p->getProjection(xp)*t);
        return p->isInside2D(p->getProjection(xo + t*xp));
    };

    int isWhere(const EGS_Vector &x) {
        if (isInside(x)) {
            return 0;
        }
        else {
            return -1;
        }
    };
    int inside(const EGS_Vector &x) {
        return isWhere(x);
    };

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed = 0, EGS_Vector *normal = 0) {
        int jhit=-1;
        if (ireg == 0) {
            int convex = p->isConvex();
            for (int j=0; j<n; j++) {
                if (convex || s[j]->isInside2D(x)) {
                    if (s[j]->isInside(x))
                        if (s[j]->howfar(true,x,u,t)) {
                            jhit = j;
                        }
                }
            }
            if (!open) {
                if (p->howfar(true,x,u,t)) {
                    jhit = n;
                }
            }
            if (jhit < 0) {
                return ireg;
            }
            if (newmed) {
                *newmed = -1;
            }
            if (normal) *normal = jhit < n ? s[jhit]->getNormal() :
                                      p->getNormal();
            return -1;
        }
        for (int j=0; j<n; j++) {
            //bool in = s[j]->isInside(x) && s[j]->isInside2D(x);
            EGS_Float up = u*s[j]->getNormal(), xp = s[j]->distance(x);
            if (up > 0 && xp < 0) {
                EGS_Float tt = -xp/up;
                if (tt <= t && s[j]->isInside2D(x+u*tt)) {
                    t = tt;
                    jhit = j;
                }
            }
        }
        if (!open) {
            EGS_Float up = u*p->getNormal(), xp = p->distance(x);
            if (up > 0 && xp < 0) {
                EGS_Float tt = -xp/up;
                if (tt <= t && p->isInside2D(x+u*tt)) {
                    t = tt;
                    jhit = n;
                }
            }
        }
        if (jhit < 0) {
            return ireg;
        }
        if (newmed) {
            *newmed = med;
        }
        if (normal) *normal = jhit < n ? s[jhit]->getNormal()*(-1) :
                                  p->getNormal()*(-1);
        return 0;
    };

    // TODO: optimize. this implementation is waaaay too slow.
    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        EGS_Float tperp = 1e30;
        for (int j=0; j<n; j++) {
            EGS_Float t = s[j]->hownear(true,x);
            if (t < tperp) {
                if (t <= 0) {
                    return 0;
                }
                tperp = t;
            }
        }
        if (!open) {
            EGS_Float t = p->hownear(true,x);
            if (t < tperp) {
                tperp = t;
            }
        }
        return tperp;
    };

    const string &getType() const {
        return p->getType();
    };

    void printInfo() const;

};

typedef EGS_PyramidT<EGS_PolygonYZ> EGS_PyramidX;
typedef EGS_PyramidT<EGS_PolygonXZ> EGS_PyramidY;
typedef EGS_PyramidT<EGS_PolygonXY> EGS_PyramidZ;
typedef EGS_PyramidT<EGS_Polygon> EGS_Pyramid;

#endif
