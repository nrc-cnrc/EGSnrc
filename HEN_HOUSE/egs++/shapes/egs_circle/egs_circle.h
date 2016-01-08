/*
###############################################################################
#
#  EGSnrc egs++ circle shape headers
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


/*! \file egs_circle.h
 *  \brief A circular shape
 *  \IK
 */

#ifndef EGS_CIRCLE_
#define EGS_CIRCLE_

#include "egs_shapes.h"
#include "egs_rndm.h"
#include "egs_math.h"

#ifdef WIN32

    #ifdef BUILD_CIRCLE_DLL
        #define EGS_CIRCLE_EXPORT __declspec(dllexport)
    #else
        #define EGS_CIRCLE_EXPORT __declspec(dllimport)
    #endif
    #define EGS_CIRCLE_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_CIRCLE_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_CIRCLE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_CIRCLE_EXPORT
        #define EGS_CIRCLE_LOCAL
    #endif

#endif

/*! \brief A circle shape.

\ingroup Shapes
\ingroup SurfaceS

This shape is specified via
\verbatim
:start shape:
    library = egs_circle
    radius = the circle radius
    midpoint = Ox, Oy (optional)
    inner radius = the inner radius (optional)
:stop shape:
\endverbatim
and delivers points uniformly distributed within a circle
in the xy-plane at z=0. If an inner radius is specified,
the points will be within the ring between the <code>inner radius</code>
and the \c radius. Points within a circle in planes other
than the xy-plane at z=0 can be obtained by attaching an
\link EGS_AffineTransform affine transformation \endlink
to the circle shape.
*/
class EGS_CIRCLE_EXPORT EGS_CircleShape : public EGS_SurfaceShape {

public:

    /*! \brief Conctruct a circle with midpoint given by \a Xo and \a Yo,
    radius \a R and innder radius \a R_i */
    EGS_CircleShape(EGS_Float Xo, EGS_Float Yo, EGS_Float R, EGS_Float R_i = 0,
                    const string &Name="",EGS_ObjectFactory *f=0) :
        EGS_SurfaceShape(Name,f), xo(Xo), yo(Yo), ro(R_i), dr(R-R_i) {
        otype = "circle";
        if (dr < 0) {
            ro = R;
            dr = R_i - R;
        }
        A = M_PI*dr*(dr + 2*ro);
    };
    ~EGS_CircleShape() {};
    EGS_Vector getPoint(EGS_RandomGenerator *rndm) {
        EGS_Float r = ro + dr*sqrt(rndm->getUniform());
        EGS_Float cphi, sphi;
        rndm->getAzimuth(cphi,sphi);
        return EGS_Vector(xo + r*cphi, yo + r*sphi, 0);
    };

protected:

    EGS_Float xo, yo, ro, dr;
};

#endif
