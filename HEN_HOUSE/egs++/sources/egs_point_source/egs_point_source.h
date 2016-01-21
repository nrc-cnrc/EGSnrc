/*
###############################################################################
#
#  EGSnrc egs++ point source headers
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


/*! \file egs_point_source.h
 *  \brief A point source
 *  \IK
 */

#ifndef EGS_POINT_SOURCE_
#define EGS_POINT_SOURCE_

#include "egs_base_source.h"
#include "egs_vector.h"
#include "egs_rndm.h"


#ifdef WIN32

    #ifdef BUILD_POINT_SOURCE_DLL
        #define EGS_POINT_SOURCE_EXPORT __declspec(dllexport)
    #else
        #define EGS_POINT_SOURCE_EXPORT __declspec(dllimport)
    #endif
    #define EGS_POINT_SOURCE_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_POINT_SOURCE_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_POINT_SOURCE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_POINT_SOURCE_EXPORT
        #define EGS_POINT_SOURCE_LOCAL
    #endif

#endif

/*! \brief A point source

  \ingroup Sources

A point source is a special case of an
\link EGS_IsotropicSource isotropic source \endlink.
The only reasons it is provided as a separate source
is that, being a very simple source, it was implemented first
to test the framework of dynamically loading particle sources
and that its definition is slightly simpler than the definition
of an isotropic source:
\verbatim
:start source:
    library = egs_point_source
    name = some_name
    position = Px, Py, Pz
    :start spectrum:
        definition of the spectrum
    :stop spectrum:
    charge = -1 or 0 or 1 for electrons or photons or positrons
:stop source:
\endverbatim

*/
class EGS_POINT_SOURCE_EXPORT EGS_PointSource : public EGS_BaseSimpleSource {

    EGS_Vector xo;      //!< The point source position
    bool       valid;   //!< Is the object a valid source?

public:

    /*! \brief Constructor

    Construct a point source with charge \a Q, spectrum \a Spec and
    position \a Xo. The source object takes ownership of the spectrum.
    */
    EGS_PointSource(int Q, EGS_BaseSpectrum *Spec, const EGS_Vector &Xo,
                    const string &Name="", EGS_ObjectFactory *f=0) :
        EGS_BaseSimpleSource(Q,Spec,Name,f), xo(Xo), valid(true) {
        setUp();
    };

    /*! \brief Constructor

    Construct a point source from the information pointed to by \a inp.
    */
    EGS_PointSource(EGS_Input *, EGS_ObjectFactory *f=0);
    ~EGS_PointSource() {};

    void getPositionDirection(EGS_RandomGenerator *rndm,
                              EGS_Vector &x, EGS_Vector &u, EGS_Float &wt) {
        x = xo;
        u.z = 2*rndm->getUniform()-1;
        EGS_Float sinz = 1-u.z*u.z;
        if (sinz > 1e-15) {
            sinz = sqrt(sinz);
            EGS_Float cphi, sphi;
            rndm->getAzimuth(cphi,sphi);
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
        return (valid && s != 0);
    };

protected:

    /*! \brief Sets up the source type and description */
    void setUp();

};

#endif
