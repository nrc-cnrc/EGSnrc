/*
###############################################################################
#
#  EGSnrc egs++ dynamic shape headers
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
#  Author:          Reid Townson
#
#  Contributors:
#
###############################################################################
*/


/*! \file egs_dynamic_shape.h
 *  \brief A dynamic shape
 *  \RT
 */

#ifndef EGS_DYNAMIC_SHAPE_
#define EGS_DYNAMIC_SHAPE_

#include "egs_shapes.h"
#include "egs_rndm.h"
#include "egs_application.h"

#ifdef WIN32

    #ifdef BUILD_DYNAMIC_SHAPE_DLL
        #define EGS_DYNAMIC_SHAPE_EXPORT __declspec(dllexport)
    #else
        #define EGS_DYNAMIC_SHAPE_EXPORT __declspec(dllimport)
    #endif
    #define EGS_DYNAMIC_SHAPE_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_DYNAMIC_SHAPE_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_DYNAMIC_SHAPE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_DYNAMIC_SHAPE_EXPORT
        #define EGS_DYNAMIC_SHAPE_LOCAL
    #endif

#endif

/*! \brief An dynamic shape

\ingroup Shapes

A dynamic shape provides the functionality to model motion of shapes during the simulation. This is automatically synchronized with the motion of sources and geometries. To do this, the dynamic shape applies transformations to any other egs++ shape. By sampling a time index, continuous motion is modelled by interpolating between user-defined control points.

A dynamic shape is defined using
\verbatim
:start shape:
    library = egs_dynamic_shape
    :start shape:
        definition of the shape to be 'dynamic'
    :stop shape:
    :start motion: # units of cm and degrees
        control point = timeIndex(1) xtrans(1) ytrans(1) ztrans(1) xrot(1) yrot(1) zrot(1)
        control point = timeIndex(2) xtrans(2) ytrans(2) ztrans(2) xrot(2) yrot(2) zrot(2)
        .
        .
        .
        control point = timeIndex(N) xtrans(N) ytrans(N) ztrans(N) xrot(N) yrot(N) zrot(N)
    :stop motion:
:stop shape:
\endverbatim

Control points must be defined such that timeIndex(i+1)>=timeIndex(i), where timeIndex(i)
is the value of time index for control point i. The timeIndex(i) are automatically
normalized by timeIndex(N), where N is the number of control points.

A translation from the starting position of the shape is applied according to
xtrans, ytrans and ztrans. A rotation follows the same rotation technique as in
EGS_AffineTransform, using the rotation input parameter for 2 or 3 values.
Angles are in degrees and translations in cm.

Continuous, dynamic motion between control points is simulated by choosing a random
number, R, on (0,1] and, for timeIndex(i)<R<=timeIndex(i+1), setting the translation or
rotation parameter P by interpolation:
P=P(i)+[P(i+1)-P(i)]/[timeIndex(i+1)-timeIndex(i)]*[R-timeIndex(i)]

It is generally expected that the user provide timeIndex(1)=0.0. However, the geometry can function with timeIndex(1)>0.0, in the case where a user desires to eliminate particles associated with a range of timeIndex values, but there will be a lot of warning messages.

*/
class EGS_DYNAMIC_SHAPE_EXPORT EGS_DynamicShape : public EGS_BaseShape {

public:

    /*!
     * \brief Constructor for EGS_DynamicShape
     * \param Shape Base shape to be made dynamic
     * \param dyninp Input containing dynamic shape specifications
     * \param Name Name of the dynamic shape
     * \param f EGS_ObjectFactory pointer
     */
    EGS_DynamicShape(EGS_BaseShape *Shape, EGS_Input *dyninp, const string &Name="",EGS_ObjectFactory *f=0) :
        EGS_BaseShape(Name,f), shape(Shape) {
        if (shape) {
            shape->ref();
            otype = "dynamic ";
            otype += shape->getObjectType();
        }
        else {
            otype = "Invalid DynamicShape";
        }

        // Build dynamic shape is where many of the geometry attributes (including control points) are extracted
        buildDynamicShape(dyninp);

        if (cpts.size() < 2) {
            egsWarning("DynamicShape: not enough or missing control points.\n");
        }
        else {
            if (cpts[0].time > 0.0) {
                egsWarning("DynamicShape: time index of control point 1 > 0.0.  This will generate many warning messages.\n");
            }
            int npts = cpts.size();
            for (int i=0; i<npts; i++) {
                if (i>0 && cpts[i].time < cpts[i-1].time-epsilon) {
                    egsWarning("DynamicShape: time index of control point %i < time index of control point %i\n",i,i-1);
                }
                if (cpts[i].time<0.0) {
                    egsWarning("DynamicShape: time index of control point %i < 0.0\n",i);
                }
            }

            // Normalize time values
            for (int i=0; i<npts-1; i++) {
                cpts[i].time /= cpts[npts-1].time;
            }
        }
    };

    /*!
     * \brief Destructor for EGS_DynamicShape
     */
    ~EGS_DynamicShape() {
        EGS_Object::deleteObject(shape);
    };

    /*!
     * \brief Get a random point from the dynamic shape
     * \param rndm Random number generator
     * \return Random point as an EGS_Vector
     */
    EGS_Vector getPoint(EGS_RandomGenerator *rndm) {
        EGS_Vector v(shape->getPoint(rndm));
        return v;
    };

    /*! \brief Returns a random 3D vector.
     *
     * Uses the function getPoint() to pick a random position and
     * then applies the affine transformation attached to the shape before
     * returning it.
     */
    EGS_Vector getRandomPoint(EGS_RandomGenerator *rndm) {
        getNextShapePosition(rndm);
        return shape->getRandomPoint(rndm);
    };

    /*!
     * \brief Structure representing a control point for dynamic motion
     */
    struct EGS_ControlPoint {
        EGS_Float time;     //!< Time index for control point
        vector<EGS_Float> trnsl; //!< Vector specifying x, y, z translation
        vector<EGS_Float> rot;   //!< Rotation vector
    };

    /*!
     * \brief Get the direction of the point source for a given position
     * \param Xo Position vector
     * \param rndm Random number generator
     * \param u Direction vector
     * \param wt Weight
     */
    void getPointSourceDirection(const EGS_Vector &Xo,
                                 EGS_RandomGenerator *rndm, EGS_Vector &u, EGS_Float &wt) {
        if (shape->supportsDirectionMethod()) {
            getNextShapePosition(rndm);
            shape->getPointSourceDirection(Xo, rndm, u, wt);
        }
    }

protected:

    EGS_BaseShape  *shape;    //!< Base shape made dynamic

    vector<EGS_ControlPoint> cpts; //!< Control points

    int ncpts; //!< Number of control points

    EGS_Float ptime; //!< Time index corresponding to particle

    /*!
     * \brief Get the next state of the dynamic shape
     * \param rndm Random number generator
     */
    void getNextShapePosition(EGS_RandomGenerator *rndm);

    /*!
     * \brief Determine whether the simulation geometry contains a dynamic shape
     * \param hasdynamic Boolean indicating if the simulation geometry contains a dynamic shape
     */
    void containsDynamic(bool &hasdynamic) {
        hasdynamic = true;
    }

    /*!
     * \brief Check if the shape supports the direction method
     * \return Boolean indicating if the shape supports the direction method
     */
    bool supportsDirectionMethod() const {
        return shape->supportsDirectionMethod();
    }

    /*!
     * \brief Extract coordinates for the next dynamic shape position
     * \param rand Random number for time sampling
     * \param gipt EGS_ControlPoint structure to store the coordinates
     * \return 0 if successful, otherwise 1
     */
    int getCoord(EGS_Float rand, EGS_ControlPoint &gipt);

    /*!
     * \brief Build the dynamic shape using input specifications
     * \param dyninp Input containing dynamic shape specifications
     */
    void buildDynamicShape(EGS_Input *dyninp);

};

#endif
