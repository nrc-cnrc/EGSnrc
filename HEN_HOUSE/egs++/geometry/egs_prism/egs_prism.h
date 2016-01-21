/*
###############################################################################
#
#  EGSnrc egs++ prism geometry headers
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


/*! \file egs_prism.h
 *  \brief A prism geometry: header
 *  \IK
 */

#ifndef EGS_PRISM_
#define EGS_PRISM_

#include "egs_base_geometry.h"
#include "egs_polygon.h"
#include "egs_functions.h"

#ifdef WIN32

    #ifdef BUILD_PRISM_DLL
        #define EGS_PRISM_EXPORT __declspec(dllexport)
    #else
        #define EGS_PRISM_EXPORT __declspec(dllimport)
    #endif
    #define EGS_PRISM_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_PRISM_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_PRISM_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_PRISM_EXPORT
        #define EGS_PRISM_LOCAL
    #endif

#endif

/*! \brief A class for modeling prisms

\ingroup Geometry
\ingroup ElementaryG

This template class is used to model prisms with specializations
for prisms having their bases in the
X-, Y-, Z- and any plane.
It can be used to model
"open" prisms (prisms that extend to infinity along the axis
perpendicular to the prism base) or closed prisms. The prism
bases can be arbitrary closed polygons. Prism geometries
have a single region (inside the prism). They are defined via
the following set of keys:

\verbatim
library = egs_prism
type = EGS_PrismX or EGS_PrismY or EGS_PrismZ or EGS_Prism
points = list of 2D or 3D positions
closed = two inputs
\endverbatim
When a X-, Y- or Z-prism is being constructed the list of inputs
for the \c points key is considered to be a list of 2D positions
in the X-, Y- or Z-planes through the origin (and therefore
the number of floating point numbers input must be even). When a
EGS_Prism object is constructed, the list of inputs
for the \c points key is considered to be a list of 3D positions
(and therefore, the number of floating point numbers input must be divisible
by 3). In this case, all points must be on a single plane, otherwise
the construction of an EGS_Prism is rejected.
The \c closed key is optional and if missing, the prism becomes
"open". If present, the two inputs define the distance from the
top and bottom prism plane to the plane used to define the polygon.
Note that at least 3 different 2D or 3D positions are required for the
\c points key in order to construct a prism. Also note that if the
last point of the polygon is not the same as the first point, the polygon
is automatically closed.

*/
template <class T>
class EGS_PRISM_EXPORT EGS_PrismT : public EGS_BaseGeometry {

protected:

    T      *p;             //!< The base polygon
    EGS_Vector a;          //!< The normal vector to the base plane
    EGS_Float
    d1, //!< Distance of the top plane to the base (for closed prisms)
    d2; //!< Distance of the bottom plane to the base (for closed prisms)

    bool   open; //!< Is the prism open ?

public:

    /*! \brief Construct an open prism using \a P as the base polygon

    The object takes ownership of the polygon pointed to by \a P
    (\em i.e. no copy is made).
    */
    EGS_PrismT(T *P, const string &Name="") :
        EGS_BaseGeometry(Name), p(P), a(p->getNormal()), open(true) {
        is_convex = p->isConvex();
    };

    /*! \brief Construct a closed prism using \a P as the base polygon

    The object takes ownership of the polygon pointed to by \a P
    (\em i.e. no copy is made). The distances of the yop and bottom planes
    from the base are given by \a D1 and \a D2.
    */
    EGS_PrismT(T *P, EGS_Float D1, EGS_Float D2, const string &Name="") :
        EGS_BaseGeometry(Name), p(P), a(p->getNormal()),
        d1(D1), d2(D2), open(false) {
        if (d1 > d2) {
            d1 = D2;
            d2 = D1;
        }
        is_convex = p->isConvex();
    };

    /*! \brief Desctructor, deletes the base polygon */
    ~EGS_PrismT() {
        delete p;
    };

    bool isInside(const EGS_Vector &x) {
        if (!open) {
            EGS_Float d = p->distance(x);
            if (d < d1 || d > d2) {
                return false;
            }
        }
        return p->isInside2D(x);
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
        EGS_2DVector v;
        EGS_2DVector *pv = normal ? &v : 0;
        if (open) {
            bool hit = p->howfar2D(ireg == 0 ? true : false,x,u,t,pv);
            if (!hit) {
                return ireg;
            }
            if (newmed) {
                *newmed = !ireg ? -1 : med;
            }
            if (normal) {
                *normal = p->getNormal(v);
            }
            return !ireg ? -1 : 0;
        }
        EGS_Float up = a*u, d = p->distance(x);
        if (!ireg) {  // inside
            EGS_Float tt = 1e30;
            int inew = ireg;
            if (up > 0) {
                tt = (d2 - d)/up;
            }
            else {
                tt = (d1 - d)/up;
            }
            if (tt <= t) {
                t = tt;
                inew = -1;
                if (normal) {
                    *normal = up>0 ? a*(-1) : a;
                }
                if (newmed) {
                    *newmed = -1;
                }
            }
            bool hit = p->howfar2D(true,x,u,t,pv);
            if (!hit) {
                return inew;
            }
            if (newmed) {
                *newmed = -1;
            }
            if (normal) {
                *normal = p->getNormal(v);
            }
            return -1;
        }
        if (d < d1 || d > d2) {
            EGS_Float tt = 1e30;
            if (d < d1 && up > 0) {
                tt = (d1 - d)/up;
            }
            else if (d > d2 && up < 0) {
                tt = (d2 - d)/up;
            }
            if (tt < t) {
                EGS_Vector xp(x + u*tt);
                if (p->isInside2D(xp)) {
                    t = tt;
                    if (newmed) {
                        *newmed = med;
                    }
                    if (normal) {
                        *normal = up>0 ? a*(-1) : a;
                    }
                    return 0;
                }
            }
        }
        EGS_Float tt = t;
        bool hit = p->howfar2D(false,x,u,tt,pv);
        if (!hit) {
            return ireg;
        }
        d = p->distance(x+u*tt);
        if (d >= d1 && d <= d2) {
            t = tt;
            if (newmed) {
                *newmed = med;
            }
            if (normal) {
                *normal = p->getNormal(v);
            }
            return 0;
        }
        return ireg;
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        EGS_Float tperp = p->hownear2D(ireg == 0 ? true : false,x);
        if (open) {
            return tperp;
        }
        EGS_Float d = p->distance(x);
        if (!ireg) {  // inside
            EGS_Float t = d2 - d;
            if (t < tperp) {
                tperp = t;
            }
            t = d - d1;
            if (t < tperp) {
                tperp = t;
            }
        }
        else {
            EGS_Float t;
            if (d < d1) {
                t = d1 - d;
            }
            else if (d > d2) {
                t = d - d2;
            }
            else {
                return tperp;
            }
            if (p->isInside2D(x)) {
                tperp = t;
            }
            else {
                tperp = sqrt(tperp*tperp + t*t);
            }
        }
        return tperp;
    };

    //const string &getType() const { return type; };
    const string &getType() const {
        return p->getType();
    };

    void printInfo() const {
        EGS_BaseGeometry::printInfo();
        if (open) {
            egsInformation("  open\n");
        }
        else {
            egsInformation("  closed with planes at %g and %g\n",d1,d2);
        }
        egsInformation("===================================================\n");
    };

};

/*! \brief A prism with base in the X-plane. */
typedef EGS_PrismT<EGS_PolygonYZ> EGS_PrismX;
/*! \brief A prism with base in the Y-plane. */
typedef EGS_PrismT<EGS_PolygonXZ> EGS_PrismY;
/*! \brief A prism with base in the Z-plane. */
typedef EGS_PrismT<EGS_PolygonXY> EGS_PrismZ;
/*! \brief A prism with base in an arbitrary plane. */
typedef EGS_PrismT<EGS_Polygon> EGS_Prism;

#endif
