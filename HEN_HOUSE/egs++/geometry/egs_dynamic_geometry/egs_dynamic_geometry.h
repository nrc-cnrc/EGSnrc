/*
###############################################################################
#
#  EGSnrc egs++ dynamic_geometry geometry headers
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
#  Author:          Alex Demelo 2023
#
#  Contributors:    Reid Townson
#
###############################################################################
*/

/*! \file egs_dynamic_geometry.h
 *  \brief A dynamic geometry: header
 *  \RT
 */

#ifndef EGS_DYNAMIC_GEOMETRY_
#define EGS_DYNAMIC_GEOMETRY_

#include "egs_base_geometry.h"
#include "egs_transformations.h"
#include "egs_rndm.h"
#include "egs_vector.h"
#include "egs_application.h"
#include <vector>
#include <string>
#include <sstream>

#ifdef WIN32
    #ifdef BUILD_DYNAMIC_GEOMETRY_DLL
        #define EGS_DYNAMIC_GEOMETRY_EXPORT __declspec(dllexport)
    #else
        #define EGS_DYNAMIC_GEOMETRY_EXPORT __declspec(dllimport)
    #endif
    #define EGS_DYNAMIC_GEOMETRY_LOCAL
#else
    #ifdef HAVE_VISIBILITY
        #define EGS_DYNAMIC_GEOMETRY_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_DYNAMIC_GEOMETRY_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_DYNAMIC_GEOMETRY_EXPORT
        #define EGS_DYNAMIC_GEOMETRY_LOCAL
    #endif
#endif

/*! \brief A dynamic geometry.

\ingroup Geometry
\ingroup CompositeG

An dynamic geometry is a geometry that
takes a random point from another geometry and then
applies a transformation, using a time sampling and interpolating between
control points.

An dynamic geometry is defined using
\verbatim
:start geometry:
    name        = ...
    library     = egs_dynamic_geometry
    my geometry = name of a predefined geometry that we want to add motion to
    :start motion: # units of cm and degrees
       control point = time(1) xtrans(1) ytrans(1) ztrans(1) xrot(1) yrot(1) zrot(1)
       control point = time(2) xtrans(2) ytrans(2) ztrans(2) xrot(2) yrot(2) zrot(2)
       .
       .
       .
       control point = time(N) xtrans(N) ytrans(N) ztrans(N) xrot(N) yrot(N) zrot(N)
    :stop motion:
:stop geometry:
\endverbatim

Control points must be defined such that time(i+1)>=time(i), where time(i)
is the value of time for control point i. The time(i) are automatically
normalized by time(N), where N is the number of control points.

A translation from the starting position of the geometry is applied according to
x, y and z. A rotation follows the same rotation technique as in
EGS_AffineTransform, using the rotation input parameter for 2 or 3 values.
Angles are in degrees and translations in cm.

Continuous, dynamic motion between control points is simulated by choosing a random
number, R, on (0,1] and, for time(i)<R<=time(i+1), setting the translation or
rotation parameter P by interpolation:
P=P(i)+[P(i+1)-P(i)]/[time(i+1)-time(i)]*[R-time(i)]

Note that this scheme for generating incident source coordinates really
only makes sense if time(1)=0.0.  However, the geometry can function
with time(1)>0.0, in the case where a user desires to eliminate particles
associated with a range of time values, but there will be a lot of
warning messages.

*/
class EGS_DYNAMIC_GEOMETRY_EXPORT EGS_DynamicGeometry :
    public EGS_BaseGeometry {

public:

    /*!
     * \struct EGS_ControlPoint
     * \brief Structure to store control point information for dynamic geometry.
     */
    struct EGS_ControlPoint {
        EGS_Float time;             //!< Time index for control point
        vector<EGS_Float> trnsl;    //!< Vector specifying x, y, z translation
        vector<EGS_Float> rot;      //!< Vector specifying rotation
    };

    /*! \brief Construct a dynamic geometry using \a G as the geometry and \a cpts as the control points.
     *
     * \param G Pointer to the base geometry to be transformed dynamically.
     * \param dyninp Input containing dynamic geometry specifications.
     * \param Name Name of the dynamic geometry.
     */
    EGS_DynamicGeometry(EGS_BaseGeometry *G, EGS_Input *dyninp, const string &Name = "") : EGS_BaseGeometry(Name), g(G) {
        type = g->getType() + "D";
        nreg = g->regions();
        is_convex = g->isConvex();
        has_rho_scaling = g->hasRhoScaling();
        has_B_scaling = g->hasBScaling();
        EGS_DynamicGeometry::buildDynamicGeometry(g, dyninp);

        if (cpts.size() < 2) {
            egsWarning("EGS_DynamicGeometry: not enough or missing control points.\n");
        }
        else {
            if (cpts[0].time > 0.0) {
                egsWarning("EGS_DynamicGeometry: time index of control point 1 > 0.0. This will generate many warning messages.\n");
            }
            int npts = cpts.size();
            for (int i = 0; i < npts; i++) {
                if (i > 0 && cpts[i].time < cpts[i - 1].time - epsilon) {
                    egsWarning("EGS_DynamicGeometry: time index of control point %i < time index of control point %i\n", i, i - 1);
                }
                if (cpts[i].time < 0.0) {
                    egsWarning("EGS_DynamicGeometry: time index of control point %i < 0.0\n", i);
                }
            }
            // Normalize time values
            for (int i = 0; i < npts - 1; i++) {
                cpts[i].time /= cpts[npts - 1].time;
            }
        }
    };

    /*! Destructor. */
    ~EGS_DynamicGeometry() {
        if (!g->deref()) {
            delete g;
        }
    };

    /*!
     * Sets the current state transform of the geometry. This is called when checking location. Same as gtransformed.
     *
     * \param t Affine transformation to set.
     */
    void setTransformation(EGS_AffineTransform t) {
        T = t;
    };

    /*!
     * Computes intersections of a particle with the dynamic geometry.
     *
     * \param ireg Region index.
     * \param n Number of particles.
     * \param x Particle position.
     * \param u Particle direction.
     * \param isections Geometry intersections.
     * \return Number of intersections.
     */
    int computeIntersections(int ireg, int n, const EGS_Vector &x,
                             const EGS_Vector &u, EGS_GeometryIntersections *isections);

    /*!
     * Checks if a region is real.
     *
     * \param ireg Region index.
     * \return True if the region is real, false otherwise.
     */
    bool isRealRegion(int ireg) const;

    /*!
     * Checks if a point is inside the dynamic geometry.
     *
     * \param x Point to check.
     * \return True if the point is inside, false otherwise.
     */
    bool isInside(const EGS_Vector &x);

    /*!
     * Checks the location of a point.
     *
     * \param x Point to check.
     * \return Index of the region containing the point.
     */
    int isWhere(const EGS_Vector &x);

    /*!
     * Alias for isWhere method.
     *
     * \param x Point to check.
     * \return Index of the region containing the point.
     */
    int inside(const EGS_Vector &x);

    /*!
     * Returns the medium index of a region.
     *
     * \param ireg Region index.
     * \return Medium index.
     */
    int medium(int ireg) const;

    /*!
     * Computes the distance to the outside of the dynamic geometry.
     *
     * \param ireg Region index.
     * \param x Particle position.
     * \param u Particle direction.
     * \return Distance to the outside.
     */
    EGS_Float howfarToOutside(int ireg, const EGS_Vector &x, const EGS_Vector &u);

    /*!
     * Computes the distance to the nearest boundary.
     *
     * \param ireg Region index.
     * \param x Particle position.
     * \param u Particle direction.
     * \param t Output: distance to the nearest boundary.
     * \param newmed Output: new medium index after crossing the boundary.
     * \param normal Output: normal vector at the intersection point.
     * \return Index of the new region.
     */
    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u, EGS_Float &t, int *newmed=0, EGS_Vector *normal=0);

    /*!
     * Computes the distance to the nearest boundary.
     *
     * \param ireg Region index.
     * \param x Particle position.
     * \return Distance to the nearest boundary.
     */
    EGS_Float hownear(int ireg, const EGS_Vector &x);

    /*!
     * Returns the maximum step allowed for the dynamic geometry.
     *
     * \return Maximum allowed step.
     */
    int getMaxStep() const;

    /*!
     * Checks if the dynamic geometry has a specific boolean property in a region.
     *
     * \param ireg Region index.
     * \param prop Boolean property type.
     * \return True if the region has the specified property, false otherwise.
     */
    bool hasBooleanProperty(int ireg, EGS_BPType prop) const;

    /*!
     * Sets a boolean property for the dynamic geometry.
     *
     * \param prop Boolean property type.
     */
    void setBooleanProperty(EGS_BPType prop);

    /*!
     * Adds a boolean property for the dynamic geometry.
     *
     * \param bit Bit index of the property to add.
     */
    void addBooleanProperty(int bit);

    /*!
     * Sets a boolean property for a range of regions in the dynamic geometry.
     *
     * \param prop Boolean property type.
     * \param start Start index of the range.
     * \param end End index of the range.
     * \param step Step size for the range.
     */
    void setBooleanProperty(EGS_BPType prop, int start, int end, int step=1);

    /*!
     * Adds a boolean property for a range of regions in the dynamic geometry.
     *
     * \param bit Bit index of the property to add.
     * \param start Start index of the range.
     * \param end End index of the range.
     * \param step Step size for the range.
     */
    void addBooleanProperty(int bit, int start, int end, int step=1);

    /*!
     * Returns the type of the dynamic geometry.
     *
     * \return Type of the dynamic geometry.
     */
    const string &getType() const;

    /*!
     * Gets the relative density of a region in the dynamic geometry.
     *
     * \param ireg Region index.
     * \return Relative density of the region.
     */
    EGS_Float getRelativeRho(int ireg) const;

    /*!
     * Sets the relative density for a range of regions in the dynamic geometry.
     *
     * \param start Start index of the range.
     * \param end End index of the range.
     * \param rho Relative density to set.
     */
    void setRelativeRho(int start, int end, EGS_Float rho);

    /*!
     * Sets the relative density for a range of regions in the dynamic geometry using input specifications.
     *
     * \param input Input containing relative density specifications.
     */
    void setRelativeRho(EGS_Input *);

    /*!
     * Gets the magnetic field scaling factor for a region in the dynamic geometry.
     *
     * \param ireg Region index.
     * \return Magnetic field scaling factor.
     */
    EGS_Float getBScaling(int ireg) const;

    /*!
     * Sets the magnetic field scaling factor for a range of regions in the dynamic geometry.
     *
     * \param start Start index of the range.
     * \param end End index of the range.
     * \param bf Magnetic field scaling factor to set.
     */
    void setBScaling(int start, int end, EGS_Float bf);

    /*!
     * Sets the magnetic field scaling factor for a range of regions in the dynamic geometry using input specifications.
     *
     * \param input Input containing magnetic field scaling factor specifications.
     */
    void setBScaling(EGS_Input *);

    /*!
     * Retrieves regions labeled with a given string.
     *
     * \param str Label to search for.
     * \param regs Output: List of region indices with the specified label.
     */
    void getLabelRegions(const string &str, vector<int> &regs);

    /*!
     * Updates the next particle state for geometries. It is tasked with determining the next state of the dynamic geometry.
     *
     * \param rndm Random number generator.
     */
    void getNextGeom(EGS_RandomGenerator *rndm);

    /*!
     * Updates the position of the dynamic geometry to the specified time.
     *
     * \param time Time index to update to.
     */
    void updatePosition(EGS_Float time);

    /*!
     * Determines whether the simulation geometry contains a dynamic geometry.
     *
     * \param hasdynamic Output: True if the simulation contains a dynamic geometry, false otherwise.
     */
    void containsDynamic(bool &hasdynamic);

protected:
    EGS_BaseGeometry *g;   //!< The geometry undergoing dynamic motion
    string type;           //!< The geometry type
    EGS_AffineTransform T; //!< Affine transformation representing the current state
    vector<EGS_ControlPoint> cpts;  //!< Control points for dynamic motion
    int ncpts;             //!< Number of control points
    EGS_Float ptime;       //!< Time index corresponding to the particle

    /*!
     * \brief Don't define media in the transformed geometry definition.
     *
     * This function is re-implemented to warn the user not to define media in the definition of a transformed geometry.
     * Instead, media should be defined when specifying the geometry to be transformed.
     */
    void setMedia(EGS_Input *inp, int, const int *);

    /*!
     * \brief Extract coordinates for the next dynamic geometry position.
     *
     * \param rand Random number for time sampling.
     * \param gipt EGS_ControlPoint structure to store the coordinates.
     * \return 0 if successful, otherwise 1.
     */
    int getCoordGeom(EGS_Float rand, EGS_ControlPoint &gipt);

    /*!
     * \brief Builds the dynamic geometry using input specifications.
     *
     * \param g Pointer to the base geometry to be transformed dynamically.
     * \param dyninp Input containing dynamic geometry specifications.
     */
    void buildDynamicGeometry(EGS_BaseGeometry *g, EGS_Input *dyninp);
};
#endif
