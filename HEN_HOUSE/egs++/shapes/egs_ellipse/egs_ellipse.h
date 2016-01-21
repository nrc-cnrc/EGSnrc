/*
###############################################################################
#
#  EGSnrc egs++ ellipse shape headers
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


/*! \file egs_ellipse.h
 *  \brief An elliptical shape
 *  \IK
 */

#ifndef EGS_ELLIPSE_
#define EGS_ELLIPSE_

#include "egs_shapes.h"
#include "egs_rndm.h"
#include "egs_math.h"

#ifdef WIN32

    #ifdef BUILD_ELLIPSE_DLL
        #define EGS_ELLIPSE_EXPORT __declspec(dllexport)
    #else
        #define EGS_ELLIPSE_EXPORT __declspec(dllimport)
    #endif
    #define EGS_ELLIPSE_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_ELLIPSE_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_ELLIPSE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_ELLIPSE_EXPORT
        #define EGS_ELLIPSE_LOCAL
    #endif

#endif

/*! \brief An elliptical shape.

\ingroup Shapes
\ingroup SurfaceS

An ellipse is specified via
\verbatim
:start shape:
    library = egs_ellipse
    halfaxis = the two half axis of the ellipse
    midpoint = Ox, Oy (optional)
:stop shape:
\endverbatim
and delivers points uniformly distributed within an
ellipse in the xy-plane at z=0. As with the \link
EGS_CircleShape circle \endlink, attach
an \link EGS_AffineTransform affine transformation \endlink
to the ellipse to get points in other
planes.

*/
class EGS_ELLIPSE_EXPORT EGS_EllipseShape : public EGS_SurfaceShape {

public:

    /*! \brief Construct an ellipse with midpoint at \a Xo,\a Yo and
      half axis \a A and \a B. */
    EGS_EllipseShape(EGS_Float Xo, EGS_Float Yo, EGS_Float A, EGS_Float B,
                     const string &Name="",EGS_ObjectFactory *f=0) :
        EGS_SurfaceShape(Name,f), xo(Xo), yo(Yo), a(A), b(B) {
        otype = "ellipse";
        A = M_PI*a*b;
    };
    ~EGS_EllipseShape() {};
    EGS_Vector getPoint(EGS_RandomGenerator *rndm) {
        EGS_Float r = sqrt(rndm->getUniform());
        EGS_Float cphi, sphi;
        rndm->getAzimuth(cphi,sphi);
        return EGS_Vector(xo + r*a*cphi, yo + r*b*sphi, 0);
    };

protected:

    EGS_Float xo, yo, a, b;
};

#endif
