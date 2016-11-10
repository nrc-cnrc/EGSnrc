/*
###############################################################################
#
#  EGSnrc egs++ radionuclide source headers
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
#  Author:          Reid Townson, 2016
#
#  Contributors:
#
###############################################################################
*/


/*! \file egs_radionuclide_source.h
 *  \brief A radionuclide source
 *  \RT
 */

#ifndef EGS_RADIONUCLIDE_SOURCE_
#define EGS_RADIONUCLIDE_SOURCE_

#include "egs_vector.h"
#include "egs_base_source.h"
#include "egs_rndm.h"
#include "egs_shapes.h"
#include "egs_base_geometry.h"
#include "egs_math.h"

#include <algorithm>


#ifdef WIN32

#ifdef BUILD_RADIONUCLIDE_SOURCE_DLL
    #define EGS_RADIONUCLIDE_SOURCE_EXPORT __declspec(dllexport)
#else
    #define EGS_RADIONUCLIDE_SOURCE_EXPORT __declspec(dllimport)
#endif
#define EGS_RADIONUCLIDE_SOURCE_LOCAL

#else

#ifdef HAVE_VISIBILITY
#define EGS_RADIONUCLIDE_SOURCE_EXPORT __attribute__ ((visibility
("default")))
#define EGS_RADIONUCLIDE_SOURCE_LOCAL  __attribute__ ((visibility
("hidden")))
#else
#define EGS_RADIONUCLIDE_SOURCE_EXPORT
#define EGS_RADIONUCLIDE_SOURCE_LOCAL
#endif

#endif

/*! \brief A radionuclide source.

\ingroup Sources

A radionuclide source is a source that delivers particles with
directions uniformly distributed in \f$4 \pi\f$ emitted from
\link EGS_BaseShape any shape. \endlink

Emissions are based on decays from the radionuclide isotope and can be a mix of
beta decays, X-radiations, etc.

It is defined using the following input
\verbatim
:start source:
    name                = my_mixture
    library             = egs_radionuclide_source
    activity            = total activity of mixture, assumed constant
    charge              = list including at least one of -1, 0, 1 to
                          include electrons, photons and positrons
    geometry            = my_geometry # see egs_isotropic_source
    region selection    = geometry confinement option, one of IncludeAll,
                          ExcludeAll, IncludeSelected, ExcludeSelected
    selected regions    = regions to apply geometry confinement
    :start shape:
        definition of the shape
    :stop shape:
    :start spectrum:
        type            = radionuclide
        isotope         = name of the isotope (e.g. Sr-90), used to look up the
                          ensdf file as $HEN_HOUSE/spectra/lnhb/{isotope}.ensdf
                          if ensdf file not provided below
        ensdf file      = [optional] path to a spectrum file in ensdf format,
                          including extension
        weight          = [optional] the relative activity (sampling
                          probability) for this isotope in a mixture
    :stop spectrum:
    :start spectrum:
        type            = radionuclide
        isotope         = name of next isotope in mixture (e.g. Y-90)
        weight          = ...
    :stop spectrum:
:stop source:
\endverbatim

The emission spectrum generation is described in \ref EGS_RadionuclideSpectrum.

Proceed with caution - \ref EGS_RadionuclideSource is in the development stages
and has not been thoroughly tested.

*/

class EGS_RADIONUCLIDE_SOURCE_EXPORT EGS_RadionuclideSource :
    public EGS_BaseSource {

public:

    /*! \brief Geometry confinement options */
    enum GeometryConfinement {
        IncludeAll      = 0,
        ExcludeAll      = 1,
        IncludeSelected = 2,
        ExcludeSelected = 3
    };

    /*! \brief Constructor

    Construct a radionuclide source with charge array \a Q, spectra array
    \a Decays and emitting particles from the shape \a Shape
    */
    EGS_RadionuclideSource(vector<int> Q_allowed, vector<EGS_BaseSpectrum *>
                           Decays, EGS_Float Activity, EGS_BaseShape *Shape, EGS_BaseGeometry
                           *geometry, const string &Name="", EGS_ObjectFactory *f=0) :
        EGS_BaseSource(Name,f), shape(Shape),
        min_theta(85.), max_theta(95.), min_phi(0), max_phi(2*M_PI),
        buf_1(1), buf_2(-1),
        geom(geometry), regions(0), nrs(0), gc(IncludeAll),
        q_allowed(Q_allowed), decays(Decays), activity(Activity) {
        setUp();
    };

    /*! \brief Constructor

    Construct a radionuclide source from the information pointed to by \a inp.
    */
    EGS_RadionuclideSource(EGS_Input *, EGS_ObjectFactory *f=0);
    ~EGS_RadionuclideSource() {
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

    EGS_I64 getNextParticle(EGS_RandomGenerator *rndm,
                            int &q, int &latch, EGS_Float &E, EGS_Float &wt,
                            EGS_Vector &x, EGS_Vector &u);

    EGS_Float getEmax() const {
        return Emax;
    };

    EGS_Float getFluence() const {
        return ishower+1;
    };

    double getTime() const {
        return time;
    };

    EGS_I64 getShowerIndex() const {
        return ishower;
    };

    void printSampledEmissions() {
        for (unsigned int i=0; i<decays.size(); ++i) {
            decays[i]->printSampledEmissions();
        }
    }

    void getPositionDirection(EGS_RandomGenerator *rndm,
                              EGS_Vector &x, EGS_Vector &u, EGS_Float &wt);

    bool storeFluenceState(ostream &data) const {
        return true;
    };

    bool setFluenceState(istream &data) {
        return true;
    };

    bool isValid() const {
        return (decays.size() != 0 && shape != 0);
    };

    /*! \brief Store the source state to the data stream \a data_out.
     *
     * Uses the \link EGS_BaseSpectrum::storeState() storeState() \endlink
     * of the spectrum object and the storeFluenceState() virtual function.
     */
    bool storeState(ostream &data_out) const;

    /*! \brief Add the source state from the stream \a data to the
     * current state.
     *
     * Uses the \link EGS_BaseSpectrum::addState() addState() \endlink
     * of the spectrum object and the addFluenceData() virtual function.
     */
    bool addState(istream &data);

    /*! \brief Reset the source to a state with zero sampled particles.
     *
     * Uses the \link EGS_BaseSpectrum::resetCounter() resetCounter() \endlink
     * function of the spectrum object and the virtual function
     * resetFluenceCounter().
     */
    void resetCounter();

    /*! \brief Add fluence data from the stream \a data to the current state.
     *
     *
     * \sa storeFluenceState(), setFluenceState(), resetFluenceCounter(),
     * setState(), storeState(), resetCounter() and addState().
     */
    bool addFluenceData(istream &data) {
        return true;
    }

    /*! \brief Reset the data related to the sampling of positions and
     * directions to a state with zero sampled particles.
     *
     * \sa storeFluenceState(), setFluenceState(), addFluenceData(),
     * setState(), storeState(), resetCounter() and addState().
     */
    void resetFluenceCounter() { };

    /*! \brief Set the source state according to the data in the stream \a data.
     *
     * Uses the \link EGS_BaseSpectrum::setState() setState() \endlink
     * method of the spectrum object and the setFluenceState() virtual
     * function.
     */
    bool setState(istream &data);

protected:

    EGS_BaseShape *shape;  //!< The shape from which particles are emitted.
    EGS_BaseGeometry    *geom;
    int                 *regions;

    EGS_I64             count;
    EGS_Float           Emax;

    void setUp();

    EGS_Float min_theta, max_theta;
    EGS_Float buf_1, buf_2; //! avoid multi-calculating cos(min_theta) and
    // cos(max_theta)
    EGS_Float min_phi, max_phi;
    int       nrs;
    GeometryConfinement gc;

    vector<EGS_BaseSpectrum *> decays; //!< The radionuclide decay structure
    vector<int>         q_allowed; //!< A list of allowed charges
    bool                q_allowAll; //!< Whether or not to allow all charges
    EGS_Float           activity; //!< The activity of the source
    double              time; //!< The time of emission of the most recently generated particle
    EGS_I64             ishower; //!< The shower index (disintegration number) of the most recently generated particle
};

#endif
