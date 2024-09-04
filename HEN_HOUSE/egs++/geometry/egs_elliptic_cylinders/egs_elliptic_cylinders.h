/*
###############################################################################
#
#  EGSnrc egs++ elliptic cylinders geometry headers
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
#  Author:          Iwan Kawrakow, 2006
#
#  Contributors:    Reid Townson
#                   Hubert Ho
#                   Marc Chamberland
#
###############################################################################
#
#  A first version of a set of elliptic cylinders. Already works in the
#  geometry viewer, but still need to run it through the geometry tester. The
#  hard part for that type of geometry is hownear() (requires to solve a 4'th
#  order equation).
#
###############################################################################
*/


/*! \file egs_cylinders.h
 *  \brief A set of concentric cylinders: header
 *  \author Declan Persram and Iwan Kawrakow
 */

#ifndef EGS_CYLINDERS_
#define EGS_CYLINDERS_

#include "egs_base_geometry.h"
#include "egs_math.h"
#include "egs_functions.h"
#include "egs_projectors.h"

#include <vector>
using namespace std;

#ifdef WIN32

    #ifdef BUILD_ELLIPTIC_CYLINDERS_DLL
        #define EGS_ELLIPTIC_CYLINDERS_EXPORT __declspec(dllexport)
    #else
        #define EGS_ELLIPTIC_CYLINDERS_EXPORT __declspec(dllimport)
    #endif
    #define EGS_ELLIPTIC_CYLINDERS_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_ELLIPTIC_CYLINDERS_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_ELLIPTIC_CYLINDERS_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_ELLIPTIC_CYLINDERS_EXPORT
        #define EGS_ELLIPTIC_CYLINDERS_LOCAL
    #endif

#endif

/*! \brief A set of concentric elliptic cylinders

  \ingroup Geometry
  \ingroup ElementaryG

This template class implements the geometry methods for a set of
\f$N\f$ concentric elliptic
cylinders that define a geometry with \f$N\f$ regions with
specializations for a elliptic cylinders along the x-, y-, z- and any axis.
Similar comments as in the case of
\link EGS_PlanesT planes \endlink
apply for the motivation for having 4 different elliptic cylinder classes.
Note that in all cases the cylinder axis passes through the origin.
Sets of cylinders at another location can be obtained by using a
\link EGS_TransformedGeometry
transformed geometry. \endlink The convention
for region numbering is that the region inside the inner-most cylinder
is region 0 with region index increasing towards the last cylinder in the
set. Note also that the geometry defined by a set of cylinders is unlimited
in the direction along the cylinder axis.

The additional keys needed to construct a set of cylinders are
\verbatim

type = EGS_XCylinders or EGS_YCylinders or EGS_ZCylinders or EGS_Cylinders
radii = list of cylinder radii
axis  = ax, ay, az

\endverbatim
The \c axis key is only needed for an object of type EGS_Cylinders
(and in fact is ignored for x-, y- or z-cylinders). The list of radii must
be given in increasing order. The components of the axis vector don't
need to be normalized to unity.

Examples of the usage of cylinder sets can be found in
\c I6702.inp, \c rounded_ionchamber.geom, \c rz.geom and \c rz_phi.geom
example geometry files.

A simple example:
\verbatim
:start geometry definition:
    :start geometry:
        name        = my_cylinders
        library     = egs_elliptic_cylinders
        type        = EGS_EllipticCylindersXZ
        midpoint    = 0
        x-radii     = 1 2 3
        y-radii     = 1 4 7
        :start media input:
            media = water air water
            set medium = 1 1
            set medium = 2 2
        :stop media input:
    :stop geometry:

    simulation geometry = my_cylinders

:stop geometry definition:
\endverbatim
\image html egs_elliptic_cylinders.png "A simple example with clipping plane 0,1,0,0"

\todo Get rid off the local projector classes, use the egspp classes
instead.

*/
template <class Tx, class Ty>
class EGS_ELLIPTIC_CYLINDERS_EXPORT EGS_EllipticCylindersT :
    public EGS_BaseGeometry {

protected:

    EGS_Float *ax,  //!< Radii along the axis defined by Ax,
              *ay,  //!< Radii along the axis defined by Ay,
              *axi, //!< Inverse of ax
              *ayi, //!< Inverse of ay
              *sx,  //!< Used for hownear calculations
              *sy;  //!< Used for hownear calculations
    bool  *is_cyl;  //!< True if the two radii are equal
    EGS_Vector xo;  //!< A point on the cylinder axis
    Tx Ax;          //!< The projection operator for the first ('x') axis
    Ty Ay;          //!< The projection operator for the second ('y') axis
    string mytype;  //!< The cylinder type
    int    nwarn;

public:

    /*! \brief Desctructor.

    Deallocates the arrays
    */
    ~EGS_EllipticCylindersT() {
        if (nreg > 0) {
            delete [] ax;
            delete [] axi;
            delete [] ay;
            delete [] ayi;
            delete [] sx;
            delete [] sy;
            delete [] is_cyl;
        }
    };

    /*! \brief Construct a set of concentric cylinders.

    Construct a set of elliptic cylinders named \a Name using the projection
    operators \a A_x and \a A_y.
    \a nc is the number of cylinders, \a x_rad, \a y_rad their radii
    along the two axis defined by \a A_x and \a A_y,
    \a position is a point on the cylinder axis
    */
    EGS_EllipticCylindersT(int nc, const EGS_Float *x_rad,
                           const EGS_Float *y_rad,
                           const EGS_Vector &position, const string &Name,
                           const Tx &A_x, const Ty &A_y) : EGS_BaseGeometry(Name),
        Ax(A_x), Ay(A_y), xo(position), nwarn(0) {
        if (nc>0) {
            ax = new EGS_Float [nc];
            axi = new EGS_Float [nc];
            ay = new EGS_Float [nc];
            ayi = new EGS_Float [nc];
            sx = new EGS_Float [nc];
            sy = new EGS_Float [nc];
            is_cyl = new bool [nc];
            for (int i=0; i<nc; i++) {
                ax[i]=x_rad[i];
                axi[i] = 1/ax[i];
                ay[i]=y_rad[i];
                ayi[i] = 1/ay[i];
                EGS_Float z = ay[i]/ax[i];
                if (fabs(z-1) > boundaryTolerance) {
                    sx[i] = 1/(z*z-1)/ax[i];
                    sy[i] = z*sx[i];
                    is_cyl[i] = false;
                }
                else {
                    is_cyl[i] = true;
                }
            }
            nreg=nc;
        }
        mytype = Ax.getType() + Ay.getType();
    };

    /*! \brief \overload */
    EGS_EllipticCylindersT(const vector<EGS_Float> &x_rad,
                           const vector<EGS_Float> &y_rad,
                           const EGS_Vector &position, const string &Name,
                           const Tx &A_x, const Ty &A_y) : EGS_BaseGeometry(Name),
        xo(position), Ax(A_x), Ay(A_y), nwarn(0) {
        int nc = x_rad.size();
        if (nc>0) {
            ax = new EGS_Float [nc];
            axi = new EGS_Float [nc];
            ay = new EGS_Float [nc];
            ayi = new EGS_Float [nc];
            sx = new EGS_Float [nc];
            sy = new EGS_Float [nc];
            is_cyl = new bool [nc];
            for (int i=0; i<nc; i++) {
                ax[i]=x_rad[i];
                axi[i] = 1/ax[i];
                ay[i]=y_rad[i];
                ayi[i] = 1/ay[i];
                EGS_Float z = ay[i]/ax[i];
                if (fabs(z-1) > boundaryTolerance) {
                    sx[i] = 1/(z*z-1)/ax[i];
                    sy[i] = z*sx[i];
                    is_cyl[i] = false;
                }
                else {
                    is_cyl[i] = true;
                }
            }
            nreg=nc;
        }
        mytype = Ax.getType() + Ay.getType();
    };

    bool isInside(const EGS_Vector &x) {
        EGS_Vector rc(x-xo);
        EGS_Float rx = Ax*rc*axi[nreg-1], ry = Ay*rc*ayi[nreg-1];
        return rx*rx + ry*ry <= 1 ? true : false;
    };

    int isWhere(const EGS_Vector &x) {
        if (!isInside(x)) {
            return -1;
        }
        EGS_Vector rc(x-xo);
        for (int j=0; j<nreg-1; j++) {
            EGS_Float rx = Ax*rc*axi[j], ry = Ay*rc*ayi[j];
            if (rx*rx + ry*ry <= 1) {
                return j;
            }
        }
        return nreg-1;
    };

    int inside(const EGS_Vector &x) {
        return isWhere(x);
    };

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed = 0, EGS_Vector *normal = 0) {

        EGS_Vector xp(x-xo);
        int inew = ireg;
        int iold = inew < 0 ? nreg : ireg;
        int ihit = ireg;
        if (ireg < 0 || ireg > 0) {
            // Particle is outside or inside but not in the inner-most
            // cylinder. Check for intersection with an inner cylinder.
            int n = iold-1;
            EGS_Float ux = Ax*u*axi[n], uy = Ay*u*ayi[n];
            EGS_Float u2 = ux*ux + uy*uy;
            if (u2 > 0) {
                EGS_Float xx = Ax*xp*axi[n], xy = Ay*xp*ayi[n];
                EGS_Float xu = xx*ux + xy*uy;
                if (xu < 0) {
                    EGS_Float r2 = xx*xx + xy*xy - 1;
                    if (r2 > 0) {
                        EGS_Float D = xu*xu - r2*u2;
                        if (D >= 0) {
                            EGS_Float d = r2/(sqrt(D) - xu);
                            if (d <= t) {
                                t = d;
                                inew = n;
                                ihit = inew;
                            }
                        }
                    }
                    else {
                        t = 0;
                        inew = n;
                        ihit = inew;
                    }
                }
            }
        }

        if (ireg >= 0 && t > 0) {
            // If here, particle is inside. Check for intersection with
            // the cylinder the particle is in.
            EGS_Float ux = Ax*u*axi[ireg], uy = Ay*u*ayi[ireg];
            EGS_Float u2 = ux*ux + uy*uy;
            if (u2 > 0) {
                EGS_Float xx = Ax*xp*axi[ireg], xy = Ay*xp*ayi[ireg];
                EGS_Float xu = xx*ux + xy*uy;
                EGS_Float r2 = xx*xx + xy*xy - 1;
                EGS_Float d = veryFar;
                if (r2 >= 0) {
                    // we think we are inside but the math shows we are
                    // outside. Hopefully a truncation problem.
                    if (xu > 0) { // moving outwards.
                        d = 0;
                    }
                    else {
                        // moving inwards => assume we are at the boundary
                        // and r2 >= 0 is a truncation problem.
                        d = -2*xu/u2;
                    }
                }
                else {
                    EGS_Float D = sqrt(xu*xu - r2*u2);
                    d = xu < 0 ? (D-xu)/u2 : -r2/(D+xu);
                }
                if (d <= t) {
                    t = d;
                    inew = ireg+1;
                    ihit = ireg;
                }
            }
        }
        if (inew == ireg) {
            return inew;
        }

        if (normal) {
            EGS_Vector xhit = xp + u*t;
            EGS_Float xx = Ax*xhit, xy = Ay*xhit;
            *normal = Ax.normal()*(xx*ay[ihit]*ay[ihit]) +
                      Ay.normal()*(xy*ax[ihit]*ax[ihit]);
            normal->normalize();
            if (inew > iold) {
                *normal *= (-1);
            }
        }
        if (inew >= nreg) {
            return -1;
        }
        if (newmed) {
            *newmed = medium(inew);
        }
        return inew;
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        EGS_Float xx = fabs(Ax*x), xy = fabs(Ay*x);
        EGS_Float tperp = ireg >= 0 ? hownear(ireg,xx,xy) : veryFar;
        if (ireg && tperp > 0) {
            int n = ireg < 0 ? nreg-1 : ireg-1;
            EGS_Float t1 = hownear(n,xx,xy);
            if (t1 < tperp) {
                tperp = t1;
            }
        }
        return tperp;
    };

    const string &getType() const {
        return mytype;
    };

    void printInfo() const {
        EGS_BaseGeometry::printInfo();
        egsInformation("Type = %s\n",mytype.c_str());
        egsInformation(" midpoint of cylinders = (%g,%g,%g)\n",xo.x,xo.y,xo.z);
        int j;
        egsInformation(" radii along 'x' =");
        for (j=0; j<nreg; j++) {
            egsInformation(" %g",ax[j]);
        }
        egsInformation("\n");
        egsInformation(" radii along 'y' =");
        for (j=0; j<nreg; j++) {
            egsInformation(" %g",ay[j]);
        }
        egsInformation("\n");
        egsInformation("===================================================\n");
    };

private:

    EGS_Float hownear(int ireg, EGS_Float xx, EGS_Float xy) {
        if (is_cyl[ireg]) {
            return fabs(ax[ireg]-sqrt(xx*xx+xy*xy));
        }
        double uo = sx[ireg]*xx, vo = sy[ireg]*xy;
        double x1 = (uo < 0 && uo > -2) ?
                    uo*(uo*uo-4*(vo*vo+3))/(16*(1+vo*vo)) :
                    1 - 0.5*vo*vo/(vo*vo+(uo+1)*(uo+1));
        int ntry=0;
        for (EGS_I64 loopCount=0; loopCount<=loopMax; ++loopCount) {
            if (loopCount == loopMax) {
                egsFatal("EGS_EllipticCylindersT::hownear: Too many iterations were required! Input may be invalid, or consider increasing loopMax.");
                return 0;
            }
            if (++ntry > 50) {
                if (++nwarn < 20) egsWarning("EGS_EllipticCylindersT::"
                                                 "hownear: failed to find solution\n");
                return 0;
            }
            double vox = vo*x1, uox = uo + x1, xm1 = 1 - x1*x1;
            double f = vox*vox - uox*uox*xm1;
            if (fabs(f) < boundaryTolerance) {
                break;
            }
            double fs = 2*(vox*vo + uox*(x1*uox-xm1));
            x1 -= f/fs;
        }
        double y1 = ay[ireg]*sqrt(1-x1*x1) - xy;
        x1 = ax[ireg]*x1-xx;
        return sqrt(x1*x1 + y1*y1);
    };
};

typedef EGS_EllipticCylindersT<EGS_XProjector,EGS_YProjector>
EGS_EllipticCylindersXY;
typedef EGS_EllipticCylindersT<EGS_XProjector,EGS_ZProjector>
EGS_EllipticCylindersXZ;
typedef EGS_EllipticCylindersT<EGS_YProjector,EGS_ZProjector>
EGS_EllipticCylindersYZ;
typedef EGS_EllipticCylindersT<EGS_Projector,EGS_Projector>
EGS_EllipticCylinders;

#endif
