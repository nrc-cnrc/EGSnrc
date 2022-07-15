/*
###############################################################################
#
#  EGSnrc egs++ collimated source headers
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
#  Contributors:    Ernesto Mainegra-Hing
#                   Reid Townson
#                   Hubert Ho
#                   Reid Townson
#
###############################################################################
*/


/*! \file egs_collimated_source.h
 *  \brief A collimated isotropic source
 *  \IK
 */

#ifndef EGS_COLLIMATED_SOURCE_
#define EGS_COLLIMATED_SOURCE_

#include "egs_vector.h"
#include "egs_base_source.h"
#include "egs_rndm.h"
#include "egs_shapes.h"
#include "egs_functions.h"


#ifdef WIN32

    #ifdef BUILD_COLLIMATED_SOURCE_DLL
        #define EGS_COLLIMATED_SOURCE_EXPORT __declspec(dllexport)
    #else
        #define EGS_COLLIMATED_SOURCE_EXPORT __declspec(dllimport)
    #endif
    #define EGS_COLLIMATED_SOURCE_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_COLLIMATED_SOURCE_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_COLLIMATED_SOURCE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_COLLIMATED_SOURCE_EXPORT
        #define EGS_COLLIMATED_SOURCE_LOCAL
    #endif

#endif

/*! \brief A collimated particle source

\ingroup Sources

A collimated source is an isotropic source collimated in
such a way as to irradiate only a certain area or solid angle.
The particle positions \f$\vec{x}\f$ for this source are picked from
any \link EGS_BaseShape shape \endlink (the source shape).
The particle directions are determined by picking a position \f$\vec{x}'\f$
from another shape (the target shape) and setting
the direction to be \f$\vec{u}=(\vec{x}'-\vec{x})/|\vec{x}'-\vec{x}|\f$.
In order to get a proper isotropic source collimated to the
area of the target shape, the statistical weight of the particles
must be adjusted accordingly (basically set to the scalar product
of the surface normal and \f$\vec{x}'-\vec{x}\f$ divided by the
distance between \f$\vec{x}'\f$ and \f$\vec{x}\f$ to the third power and
multiplied by the surface area from which points \f$\vec{x}'\f$
are being picked). This is done in a special method of the target shape
called \link EGS_BaseShape::getPointSourceDirection()
getPointSourceDirection() \endlink. Not all shapes support this
method. The shapes that support this method and can therefore be used
as a target shape in a collimated source are
\link EGS_BoxShape boxes, \endlink
\link EGS_SphereShape spheres, \endlink
\link EGS_CylinderShape cylinders, \endlink
\link EGS_CircleShape circles, \endlink
\link EGS_EllipseShape ellipses, \endlink
\link EGS_RectangleShape rectangles \endlink or
\link EGS_RectangularRing "rectangular rings", \endlink
\link EGS_PolygonShape polygons \endlink and
\link EGS_TriangleShape triangles \endlink.

The fluence calculated by getFluence() is determined as \f$N/d^2\f$,
where \c N is the number of source particles and \c d is provided by
the user as \c distance, the minimum distance between the source and target
shapes. This accounts for the solid angle within which particles are generated.
If \c distance is not provided, it defaults to 1, which will likely be
an incorrect normalization.

A collimated source is defined as follows:
\verbatim
:start source:
    library = egs_collimated_source
    name = some_name
    :start source shape:
        definition of the source shape
    :stop source shape:
    :start target shape:
        definition of the target shape
    :stop target shape:
    :start spectrum:
        definition of the spectrum
    :stop spectrum:
    distance = source-target shape min. distance
    charge = -1 or 0 or 1 for electrons or photons or positrons
:stop source:
\endverbatim
It is worth noting that the functionality of sources 1, 11, 12, 14, 15 and 16
from the RZ series of user codes and source 3 in DOSXYZnrc can be
reproduced with the collimated source from the EGSnrc C++ class library.

A simple example:
\verbatim
:start source definition:
    :start source:
        library = egs_collimated_source
        name = my_source
        :start source shape:
            type = point
            position = 0 0 5
        :stop source shape:
        :start target shape:
            library   = egs_rectangle
            rectangle = -1 -1 1 1
        :stop target shape:
        distance = 5
        charge = -1
        :start spectrum:
            type = monoenergetic
            energy = 20
        :stop spectrum:
    :stop source:

    simulation source = my_source

:stop source definition:
\endverbatim
\image html egs_collimated_source.png "A simple example"
*/
class EGS_COLLIMATED_SOURCE_EXPORT EGS_CollimatedSource :
    public EGS_BaseSimpleSource {

public:

    /*! Constructor

    Construct a collimated source with charge \a Q, spectrum \a Spec,
    source shape \a sshape and target shape \a tshape.
    */
    EGS_CollimatedSource(int Q, EGS_BaseSpectrum *Spec,
                         EGS_BaseShape *sshape, EGS_BaseShape *tshape,
                         const string &Name="", EGS_ObjectFactory *f=0) :
        EGS_BaseSimpleSource(Q,Spec,Name,f), source_shape(sshape),
        target_shape(tshape), ctry(0), dist(1) {
        setUp();
    };

    /*! Constructor

    Construct a collimated source from the information pointed to by \a inp.
    */
    EGS_CollimatedSource(EGS_Input *, EGS_ObjectFactory *f=0);
    ~EGS_CollimatedSource() {
        EGS_Object::deleteObject(source_shape);
        EGS_Object::deleteObject(target_shape);
    };

    void getPositionDirection(EGS_RandomGenerator *rndm,
                              EGS_Vector &x, EGS_Vector &u, EGS_Float &wt) {
        //x = source_shape->getPoint(rndm);
        x = source_shape->getRandomPoint(rndm);
        int ntry = 0;
        do {
            target_shape->getPointSourceDirection(x,rndm,u,wt);
            ntry++;
            if (ntry > 10000)
                egsFatal("EGS_CollimatedSource::getPositionDirection:\n"
                         "  my target shape %s, which is of type %s, failed to\n"
                         "  return a positive weight after 10000 attempts\n",
                         target_shape->getObjectName().c_str(),
                         target_shape->getObjectType().c_str());
        }
        while (wt <= 0);
        //egsInformation("got x=(%g,%g,%g) u=(%g,%g,%g) wt = %g ntry = %d\n",
        //        x.x,x.y,x.z,u.x,u.y,u.z,wt,ntry);
        ctry += ntry;
    };

    EGS_Float getFluence() const {
        double res = ctry;
        return res/(dist*dist);
    };

    bool storeFluenceState(ostream &data) const {
        return egsStoreI64(data,ctry);
    };

    bool setFluenceState(istream &data) {
        return egsGetI64(data,ctry);
    };

    bool addFluenceData(istream &data) {
        EGS_I64 tmp;
        bool ok = egsGetI64(data,tmp);
        if (!ok) {
            return false;
        }
        ctry += tmp;
        return true;
    };

    bool isValid() const {
        return (s != 0 && source_shape != 0 && target_shape != 0 &&
                target_shape->supportsDirectionMethod());
    };

    void resetFluenceCounter() {
        ctry = 0;
    };

protected:

    EGS_BaseShape *source_shape,  //!< the source shape
                  *target_shape;  //!< the target shape
    EGS_I64       ctry;           //!< number of attempts to sample a particle
    EGS_Float     dist;           //!< source-target shape min. distance

    void setUp();

};

#endif
