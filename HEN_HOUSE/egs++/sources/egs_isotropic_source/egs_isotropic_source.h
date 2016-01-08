/*
###############################################################################
#
#  EGSnrc egs++ isotropic source headers
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
#  Contributors:    Frederic Tessier
#
###############################################################################
*/


/*! \file egs_isotropic_source.h
 *  \brief An isotropic source
 *  \IK
 */

#ifndef EGS_ISOTROPIC_SOURCE_
#define EGS_ISOTROPIC_SOURCE_

#include "egs_vector.h"
#include "egs_base_source.h"
#include "egs_rndm.h"
#include "egs_shapes.h"
#include "egs_base_geometry.h"
#include "egs_math.h"


#ifdef WIN32

    #ifdef BUILD_ISOTROPIC_SOURCE_DLL
        #define EGS_ISOTROPIC_SOURCE_EXPORT __declspec(dllexport)
    #else
        #define EGS_ISOTROPIC_SOURCE_EXPORT __declspec(dllimport)
    #endif
    #define EGS_ISOTROPIC_SOURCE_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_ISOTROPIC_SOURCE_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_ISOTROPIC_SOURCE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_ISOTROPIC_SOURCE_EXPORT
        #define EGS_ISOTROPIC_SOURCE_LOCAL
    #endif

#endif

/*! \brief An isotropic source

  \ingroup Sources

An isotropic source is a source that delivers particles with
directions uniformly distributed in \f$4 \pi\f$ emitted from
\link EGS_BaseShape any shape. \endlink
It is defined using the following input
\verbatim
:start source:
    library = egs_isotropic_source
    name = some_name
    :start shape:
        definition of the shape
    :stop shape:
    :start spectrum:
        definition of the spectrum
    :stop spectrum:
    charge = -1 or 0 or 1 for electrons or photons or positrons
    min theta = 80  (degree)
    max theta = 100 (degree)
:stop source:
\endverbatim
It is worth noting that the functionality of source 3 in the RZ series
of user codes or source 6 in DOSXYZnrc can be reproduced with
the isotropic source from the EGSnrc C++ class library.
*/

class EGS_ISOTROPIC_SOURCE_EXPORT EGS_IsotropicSource :
    public EGS_BaseSimpleSource {

public:

    /*! \brief Geometry confinement options */
    enum GeometryConfinement {
        IncludeAll      = 0,
        ExcludeAll      = 1,
        IncludeSelected = 2,
        ExcludeSelected = 3
    };

    /*! \brief Constructor

    Construct an isotropic source with charge \a Q, spectrum \a Spec
    and emitting particles from the shape \a Shape
    */
    EGS_IsotropicSource(int Q, EGS_BaseSpectrum *Spec, EGS_BaseShape *Shape,
                        EGS_BaseGeometry *geometry,
                        const string &Name="", EGS_ObjectFactory *f=0) :
        EGS_BaseSimpleSource(Q,Spec,Name,f), shape(Shape),
        min_theta(85.), max_theta(95.), min_phi(0), max_phi(2*M_PI),
        buf_1(1), buf_2(-1),
        geom(geometry), regions(0), nrs(0), gc(IncludeAll) {
        setUp();
    };

    /*! \brief Constructor

    Construct an isotropic source from the information pointed to by \a inp.
    */
    EGS_IsotropicSource(EGS_Input *, EGS_ObjectFactory *f=0);
    ~EGS_IsotropicSource() {
        egsWarning("destructing point source\n");
        EGS_Object::deleteObject(shape);
        if (geom) {
            if (!geom->deref()) {
                delete geom;
            }
        }
        if (nrs > 0 && regions) {
            delete [] regions;
        }
    };

    void getPositionDirection(EGS_RandomGenerator *rndm,
                              EGS_Vector &x, EGS_Vector &u, EGS_Float &wt) {
        bool ok = true;
        do {
            x = shape->getRandomPoint(rndm);
            if (geom) {
                if (gc == IncludeAll) {
                    ok = geom->isInside(x);
                }
                else if (gc == ExcludeAll) {
                    ok = !geom->isInside(x);
                }
                else if (gc == IncludeSelected) {
                    ok = false;
                    int ireg = geom->isWhere(x);
                    for (int j=0; j<nrs; ++j) {
                        if (ireg == regions[j]) {
                            ok = true;
                            break;
                        }
                    }
                }
                else {
                    ok = true;
                    int ireg = geom->isWhere(x);
                    for (int j=0; j<nrs; ++j) {
                        if (ireg == regions[j]) {
                            ok = false;
                            break;
                        }
                    }
                }
            }
        }
        while (!ok);

        u.z = rndm->getUniform()*(buf_1 - buf_2) - buf_1;

        //u.z = 2*rndm->getUniform()-1;
        EGS_Float sinz = 1-u.z*u.z;
        if (sinz > 1e-15) {
            sinz = sqrt(sinz);
            EGS_Float cphi, sphi;
            //rndm->getAzimuth(cphi,sphi);
            // sample phi, slower than rndm->getAzimuth
            EGS_Float phi = min_phi +(max_phi - min_phi)*rndm->getUniform();
            cphi = cos(phi);
            sphi = sin(phi);
            u.x = sinz*cphi;
            u.y = sinz*sphi;
        }
        else {
            u.x = 0;
            u.y = 0;
        }
        wt = 1;
    };

    EGS_Float getFluence() const {
        return count;
    };

    bool storeFluenceState(ostream &) const {
        return true;
    };

    bool setFluenceState(istream &) {
        return true;
    };

    bool isValid() const {
        return (s != 0 && shape != 0);
    };

protected:

    EGS_BaseShape *shape;  //!< The shape from which particles are emitted.
    EGS_BaseGeometry *geom;
    int              *regions;

    void setUp();

    EGS_Float min_theta, max_theta;
    EGS_Float buf_1, buf_2; //! avoid multi-calculating cos(min_theta) and cos(max_theta)
    EGS_Float min_phi, max_phi;
    int                 nrs;
    GeometryConfinement gc;
};

// change log:
// Long, Jan 15 2008 add polar angle limitation and macro support
//                  a word on the polar angle sampling:
//                        cos(theta) follows a uniform distribution

#endif
