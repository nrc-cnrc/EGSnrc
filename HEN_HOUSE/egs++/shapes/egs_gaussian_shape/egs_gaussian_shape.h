/*
###############################################################################
#
#  EGSnrc egs++ guassian shape headers
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


/*! \file egs_gaussian_shape.h
 *  \brief A shape smeared by a Gaussian
 *  \IK
 */

#ifndef EGS_GAUSSIAN_SHAPE_
#define EGS_GAUSSIAN_SHAPE_

#include "egs_shapes.h"
#include "egs_rndm.h"

#ifdef WIN32

    #ifdef BUILD_GAUSSIAN_SHAPE_DLL
        #define EGS_GAUSSIAN_SHAPE_EXPORT __declspec(dllexport)
    #else
        #define EGS_GAUSSIAN_SHAPE_EXPORT __declspec(dllimport)
    #endif
    #define EGS_GAUSSIAN_SHAPE_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_GAUSSIAN_SHAPE_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_GAUSSIAN_SHAPE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_GAUSSIAN_SHAPE_EXPORT
        #define EGS_GAUSSIAN_SHAPE_LOCAL
    #endif

#endif

/*! \brief A Gaussian shape

  \ingroup Shapes

A Gaussian shape is a shape that picks
points randomly within another shape and then adds
\f$\Delta \vec{x} = (\Delta x, \Delta y, \Delta z)\f$
to the points where \f$\Delta x, \Delta y\f$ and \f$\Delta z\f$ are
sampled from Gaussian distributions with widths \f$\sigma_x, \sigma_y\f$
and \f$\sigma_z\f$. A Gaussian shape is defined via
\verbatim
:start shape:
    library = egs_gaussian_shape
    :start shape:
        definition of the shape to be smeared
    :stop shape:
    sigma = 1 or 2 or 3 inputs
:stop shape:
\endverbatim
If a single input is found for the \c sigma key, then
\f$\sigma_y = \sigma_z = 0\f$, if two inputs are found, then
\f$\sigma_z = 0\f$.

*/
class EGS_GAUSSIAN_SHAPE_EXPORT EGS_GaussianShape : public EGS_BaseShape {

public:

    EGS_GaussianShape(EGS_BaseShape *Shape, EGS_Float sx, EGS_Float sy,
                      EGS_Float sz, const string &Name="",EGS_ObjectFactory *f=0) :
        EGS_BaseShape(Name,f), shape(Shape), sig_x(sx), sig_y(sy), sig_z(sz) {
        if (shape) {
            shape->ref();
            otype = shape->getObjectType();
            otype += "with Gaussian ditribution";
            if (sig_x < 0) {
                sig_x = -sig_x*0.4246609;
            }
            if (sig_y < 0) {
                sig_y = -sig_y*0.4246609;
            }
            if (sig_z < 0) {
                sig_z = -sig_z*0.4246609;
            }
        }
        else {
            otype = "Invalid GaussianShape";
        }
    };
    ~EGS_GaussianShape() {
        EGS_Object::deleteObject(shape);
    };
    EGS_Vector getPoint(EGS_RandomGenerator *rndm) {
        EGS_Vector v(shape->getPoint(rndm));
        if (sig_x > 0) {
            v.x += sig_x*rndm->getGaussian();
        }
        if (sig_y > 0) {
            v.y += sig_y*rndm->getGaussian();
        }
        if (sig_z > 0) {
            v.z += sig_z*rndm->getGaussian();
        }
        return v;
    };
    bool supportsDirectionMethod() const {
        return true;
    };
    void getPointSourceDirection(const EGS_Vector &Xo,
                                 EGS_RandomGenerator *rndm, EGS_Vector &u, EGS_Float &wt) {
        EGS_Vector xo = T ? Xo*(*T) : Xo;
        EGS_Vector x = getPoint(rndm);
        u = x - xo;
        EGS_Float d2i = 1/u.length2(), di = sqrt(d2i);
        u *= di;
        wt = 1; //wt = A*fabs(u.z)*d2i;
        if (T) {
            T->rotate(u);
        }
    };

protected:

    EGS_BaseShape  *shape;
    EGS_Float      sig_x, sig_y, sig_z;

};

#endif
