/*
###############################################################################
#
#  EGSnrc egs++ circle perpendicular shape headers
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
#  Author:          Reid Townson, 2020
#
#  Contributors:
#
###############################################################################
*/


/*! \file egs_circle_perpendicular.h
 *  \brief A circular shape perpendicular to source particles
 *  \RT
 */

#ifndef EGS_CIRCLE_PERPENDICULAR_
#define EGS_CIRCLE_PERPENDICULAR_

#include "egs_shapes.h"
#include "egs_rndm.h"
#include "egs_math.h"

#ifdef WIN32

    #ifdef BUILD_CIRCLE_PERPENDICULAR_DLL
        #define EGS_CIRCLE_PERPENDICULAR_EXPORT __declspec(dllexport)
    #else
        #define EGS_CIRCLE_PERPENDICULAR_EXPORT __declspec(dllimport)
    #endif
    #define EGS_CIRCLE_PERPENDICULAR_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_CIRCLE_PERPENDICULAR_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_CIRCLE_PERPENDICULAR_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_CIRCLE_PERPENDICULAR_EXPORT
        #define EGS_CIRCLE_PERPENDICULAR_LOCAL
    #endif

#endif

/*! \brief A circle shape perpendicular to source particles.

\ingroup Shapes
\ingroup SurfaceS

This shape is specified via
\verbatim
:start shape:
    library = egs_circle_perpendicular
    radius = the circle radius
    midpoint = Ox, Oy (optional)
    inner radius = the inner radius (optional)
:stop shape:
\endverbatim
and delivers points uniformly distributed within a circle.
The surface of this circle is always perpendicular to a ray based on the
source particle position,
when this shape is used as the target in a source that calls the
getPointSourceDirection() function. For example, in a
\link EGS_CollimatedSource collimated source \endlink
when a source shape and target shape are defined, the special properties
of this circle only apply when it is used as the target shape. Otherwise,
it behaves like an \link EGS_CircleShape egs_circle \endlink.

For each source particle, when getPointSourceDirection() is called, the
circle shape is rotated so that the circle surface is perpendicular to the
vector between the source particle location and the origin of the circle. In
effect this means that the source particles see a sphere instead of a
circle, except that for a given particle, the possible target locations
are on the surface of a circle rather than within a sphere.

If you place a transformation block inside this shape, to perform
an \link EGS_AffineTransform affine transformation \endlink, only
the translation part of the transformation will be applied. Rotations
will be un-done when the surface is rotated to be perpendicular
to the source particle.

If an inner radius is specified,
the points will be within the ring between the <code>inner radius</code>
and the \c radius. Points within a circle in planes other
than the xy-plane at z=0 can be obtained by attaching an
\link EGS_AffineTransform affine transformation \endlink
to the circle shape.
*/
class EGS_CIRCLE_PERPENDICULAR_EXPORT EGS_CirclePerpendicularShape : public EGS_SurfaceShape {

public:

    /*! \brief Conctruct a circle with midpoint given by \a Xo and \a Yo,
    radius \a R and innder radius \a R_i */
    EGS_CirclePerpendicularShape(EGS_Float Xo, EGS_Float Yo, EGS_Float R, EGS_Float R_i = 0,
                                 const string &Name="",EGS_ObjectFactory *f=0) :
        EGS_SurfaceShape(Name,f), xo(Xo), yo(Yo), ro(R_i), dr(R-R_i) {
        otype = "circle";
        if (dr < 0) {
            ro = R;
            dr = R_i - R;
        }
        A = M_PI*dr*(dr + 2*ro);
    };
    ~EGS_CirclePerpendicularShape() {};
    EGS_Vector getPoint(EGS_RandomGenerator *rndm) {
        EGS_Float r = ro + dr*sqrt(rndm->getUniform());
        EGS_Float cphi, sphi;
        rndm->getAzimuth(cphi,sphi);
        return EGS_Vector(xo + r*cphi, yo + r*sphi, 0);
    };

    void getPointSourceDirection(const EGS_Vector &Xo,
                                 EGS_RandomGenerator *rndm, EGS_Vector &u, EGS_Float &wt) {

        // Perform user requested transformations to a point on the source surface
        // This is effectively the same as transforming the target position
        // because we're just calculating the direction between the two
        EGS_Vector xo = T ? Xo*(*T) : Xo;

        // Get a point on the circle target surface
        EGS_Vector x = getPoint(rndm);
        u = x - xo;
        EGS_Float d2i = 1/u.length2(), di = sqrt(d2i);
        u *= di;

        // Calculate the angle between the normal to the circle surface and the u vector between points
        EGS_Vector perpToCircle = EGS_Vector(0, 0, 1);
        EGS_Float angleBetween = std::acos(u * perpToCircle / (perpToCircle.length() * u.length()));

        // We will rotate about a vector perpendicular to u and the target surface normal
        // Check against fabs(u.z) to account for both parallel and anti-parallel cases
        EGS_Vector rotateAbout;
        if ((fabs(u.z) - perpToCircle.z) < epsilon) {
            rotateAbout = perpToCircle;
        }
        else {
            rotateAbout = u.times(perpToCircle);
        }
        EGS_RotationMatrix rotation = EGS_RotationMatrix::rotV(-angleBetween, rotateAbout);
        EGS_AffineTransform *transform = new EGS_AffineTransform(rotation);
        if (transform) {
            // Transform the point on the target surface by this rotation
            transform->rotate(x);

            // Calculate the new u vector
            u = x - xo;
        }
        delete transform;

        d2i = 1/u.length2(), di = sqrt(d2i);
        u *= di;
        wt = A*u.length()*d2i;
        if (T) {
            T->rotate(u);
        }
    };

protected:

    EGS_Float xo, yo, ro, dr;
};

#endif
