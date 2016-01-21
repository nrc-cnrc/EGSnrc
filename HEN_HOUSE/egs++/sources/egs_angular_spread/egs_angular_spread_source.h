/*
###############################################################################
#
#  EGSnrc egs++ angular spread source headers
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
#  Author:          Iwan Kawrakow, 2009
#
#  Contributors:
#
###############################################################################
*/


/*! \file  egs_angular_spread_source.h
 *  \brief An angular spread source: header
 *  \IK
 */

#ifndef EGS_ANGULAR_SPREAD_SOURCE_
#define EGS_ANGULAR_SPREAD_SOURCE_

#include "egs_vector.h"
#include "egs_base_source.h"
#include "egs_rndm.h"
#include "egs_functions.h"


#ifdef WIN32

    #ifdef BUILD_ANGULAR_SPREAD_SOURCE_DLL
        #define EGS_ANGULAR_SPREAD_SOURCE_EXPORT __declspec(dllexport)
    #else
        #define EGS_ANGULAR_SPREAD_SOURCE_EXPORT __declspec(dllimport)
    #endif
    #define EGS_ANGULAR_SPREAD_SOURCE_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_ANGULAR_SPREAD_SOURCE_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_ANGULAR_SPREAD_SOURCE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_ANGULAR_SPREAD_SOURCE_EXPORT
        #define EGS_ANGULAR_SPREAD_SOURCE_LOCAL
    #endif

#endif

/*! \brief A source that adds additional Gaussian angular spread to another source

  \ingroup Sources

An angular spread source is a source that takes a particle from any
other source and then applies a rotation to the particle direction
by an angle sampled from a Gaussian distribution with user defined width.
This source is defined as follows:
\verbatim
:start source:
    library = egs_angular_spread_source
    name = some_name
    source name = the name of a previously defined source
    sigma = angular_spread_in_degrees
:stop source:
\endverbatim
The \c sigma input can be positive or negative.
If it is positive, it is considered to be the sigma of the Gaussian distribution
in degrees, if negative, the FWHM of the distribution.

*/
class EGS_ANGULAR_SPREAD_SOURCE_EXPORT EGS_AngularSpreadSource :
    public EGS_BaseSource {

public:

    EGS_AngularSpreadSource(EGS_Input *, EGS_ObjectFactory *f=0);
    ~EGS_AngularSpreadSource() {
        EGS_Object::deleteObject(source);
    };

    EGS_I64 getNextParticle(EGS_RandomGenerator *rndm,
                            int &q, int &latch, EGS_Float &E, EGS_Float &wt,
                            EGS_Vector &x, EGS_Vector &u) {
        EGS_I64 c = source->getNextParticle(rndm,q,latch,E,wt,x,u);
        //egsInformation("\nGot u=(%g,%g,%g)\n",u.x,u.y,u.z);
        if (sigma > 0) {
            EGS_Float cost;
            do {
                cost = 1 + sigma*log(1 - rndm->getUniform());
            }
            while (cost <= -1);
            EGS_Float cphi, sphi;
            rndm->getAzimuth(cphi,sphi);
            EGS_Float sint = sqrt(1-cost*cost);
            u.rotate(cost,sint,cphi,sphi);
            //egsInformation("sampled cost=%g -> unew=(%g,%g,%g)\n",cost,u.x,u.y,u.z);
        }
        return c;
    };
    EGS_Float getEmax() const {
        return source->getEmax();
    };
    EGS_Float getFluence() const {
        return source->getFluence();
    };
    bool storeState(ostream &data) const {
        return source->storeState(data);
    };
    bool setState(istream &data) {
        return source->setState(data);
    };
    bool addState(istream &data_in) {
        return source->addState(data_in);
    };
    void resetCounter() {
        source->resetCounter();
    };

    bool isValid() const {
        return (source != 0);
    };

protected:

    EGS_BaseSource     *source; //!< The source being transformed
    EGS_Float           sigma;

    void setUp();

};

#endif
