/*
###############################################################################
#
#  EGSnrc egs++ Fano source headers
#  Copyright (C) 2016 National Research Council Canada
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
#  Authors:         Ernesto Mainegra-Hing, 2016
#                   Hugo Bouchard, 2016
#
#  Contributors:    Reid Townson
#
###############################################################################
*/


/*! \file egs_fano_source.h
 *  \brief A Fano source
 *  \EM
 */

#ifndef EGS_FANO_SOURCE_
#define EGS_FANO_SOURCE_

#include "egs_vector.h"
#include "egs_base_source.h"
#include "egs_rndm.h"
#include "egs_shapes.h"
#include "egs_base_geometry.h"
#include "egs_math.h"


#ifdef WIN32

    #ifdef BUILD_FANO_SOURCE_DLL
        #define EGS_FANO_SOURCE_EXPORT __declspec(dllexport)
    #else
        #define EGS_FANO_SOURCE_EXPORT __declspec(dllimport)
    #endif
    #define EGS_FANO_SOURCE_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_FANO_SOURCE_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_FANO_SOURCE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_FANO_SOURCE_EXPORT
        #define EGS_FANO_SOURCE_LOCAL
    #endif

#endif

/*! \brief A Fano source

  \ingroup Sources

An Fano source is a source that delivers particles proportional to the
mass in the current source region with directions uniformly distributed in
\f$4 \pi\f$ emitted from \link EGS_BaseShape any shape. \endlink
It is defined using the following input
\verbatim
:start source:
    library = egs_fano_source
    name = some_name
    :start shape:
        definition of the shape
    :stop shape:
    :start spectrum:
        definition of the spectrum
    :stop spectrum:
    charge = -1 or 0 or 1 for electrons or photons or positrons
    max mass density = 1.2
    geometry = some_name
:stop source:
\endverbatim
*/

class EGS_FANO_SOURCE_EXPORT EGS_FanoSource :
    public EGS_BaseSimpleSource {

public:

    /*! \brief Constructor

    Construct a Fano source with charge \a Q, spectrum \a Spec
    and emitting particles from the shape \a Shape
    */
    EGS_FanoSource(int Q, EGS_BaseSpectrum *Spec, EGS_BaseShape *Shape,
                   EGS_BaseGeometry *geometry,
                   const string &Name="", EGS_ObjectFactory *f=0) :
        EGS_BaseSimpleSource(Q,Spec,Name,f), shape(Shape),
        min_theta(85.), max_theta(95.), min_phi(0), max_phi(2*M_PI),
        buf_1(1), buf_2(-1),
        geom(geometry), regions(0), nrs(0) {
        setUp();
    };

    /*! \brief Constructor

    Construct a Fano source from the information pointed to by \a inp.
    */
    EGS_FanoSource(EGS_Input *, EGS_ObjectFactory *f=0);
    ~EGS_FanoSource() {
        egsWarning("destructing Fano source\n");
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
        bool ok = true, okfano = false;
        wt = 1;
        do {
            do {
                x = shape->getRandomPoint(rndm);
                ok = geom->isInside(x);
                if (ok) {
                    /*******************************************************************************
                     * Rejection technique generates particles proportional to the region's mass m.
                     * The joint probability of selecting the emission point is the product of the
                     * probability of the point being in volume V times the probability of surviving
                     * the rejection, which is proportional to the density in that volume.
                     *******************************************************************************/
                    if (rndm->getUniform()*max_mass_density > geom->getMediumRho(geom->medium(geom->isWhere(x)))) {
                        okfano = false;
                    }
                    else {
                        okfano = true;
                    }

                    /*********************************************************************
                     * Rather than rejecting, accept always and adjust weight accordingly
                     * A quick check didn't show improved efficiency as one would have expected!
                     * Perhaps the extra calculations? One should have used the values above!
                     *********************************************************************/
                    //wt = wt * geom->getMediumRho(geom->medium(geom->isWhere(x)))/max_mass_density;
                    //real_count += wt;
                    //okfano = true;
                }
            }
            while (!ok);
        }
        while (!okfano);
        u.z = buf_1 - rndm->getUniform()*(buf_1 - buf_2);
        EGS_Float sinz = 1-u.z*u.z;
        if (sinz > epsilon) {
            sinz = sqrt(sinz);
            EGS_Float cphi, sphi;
            EGS_Float phi = min_phi + (max_phi - min_phi)*rndm->getUniform();
            cphi = cos(phi);
            sphi = sin(phi);
            u.x = sinz*cphi;
            u.y = sinz*sphi;
        }
        else {
            u.x = 0;
            u.y = 0;
        }
    };

    EGS_Float getFluence() const {
        return count;
    };

    /**************************************************
     * Related to the implementation without rejection
     * To be further tested !!!
     **************************************************/
    //EGS_Float getFluence() const { return Fano_source ? real_count : count; };
    /*
        bool storeFluenceState(ostream & data) const {
             if (Fano_source) data << real_count;
             return Fano_source ? data.good() : true;
        };

        bool setFluenceState(istream & data) {
          if (Fano_source) data >> real_count;
          return Fano_source ? data.good() : true;
        };

        bool addFluenceData(istream &data) {
             if (Fano_source){
                EGS_Float tmp; data >> tmp;
                if( !data.good() ) return false;
                real_count += tmp;
             }
             return true;
        };

        void resetFluenceCounter() {real_count = 0.0; };
    */
    /*****************************************************/

    bool isValid() const {
        return (s != 0 && shape != 0);
    };

protected:

    EGS_BaseShape    *shape;    //!< The shape from which particles are emitted.
    EGS_BaseGeometry *geom;
    int              *regions;

    void setUp();

    EGS_Float min_theta, max_theta;
    EGS_Float buf_1, buf_2;//! avoid multi-calculating cos(min_theta) and cos(max_theta)
    EGS_Float min_phi, max_phi;
    //real_count;
    EGS_Float max_mass_density;
    int                 nrs;
};


#endif
