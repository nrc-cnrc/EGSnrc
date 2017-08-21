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
#include "egs_application.h"

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

Note that \ref EGS_RadionuclideSource is an experimental source and only a
subset of the available radionuclides have been tested against measurement.
Please be aware of the known caveats and how they may affect your results.
Report any discrepancies on the
<a href="https://github.com/nrc-cnrc/EGSnrc/issues">EGSnrc github issues page</a>.

<em>Known Caveats:</em>

- Beta+- spectra have only been checked for a subset of radionuclides. Use
'<code>output beta spectra = yes</code>' in \ref EGS_RadionuclideSpectrum
and perform a quick
simulation to obtain beta spectrum files for the nuclide of interest.
- Gamma-gamma angular correlations are currently not modeled; all emissions
are considered isotropic.
- Due to limitations in the current LNHB ensdf format, electron shell vacancy
creation due to disintegrations is sampled per-shell rather than
considering sub-shell probabilities. I.e. probabilities for vacancy creation
in the K,L,M,N,O shells are considered, but not L1, L2, etc. The actual
sub-shell in which the vacancy created is sampled uniformly for the given
shell. This is an approximation, but only relevant when
'<code>atomic relaxations = ensdf</code>' in
\ref EGS_RadionuclideSpectrum.
- Alpha particles are absorbed immediately in the source region, and
not transported.
- Atomic motion & recoil from emissions is not modeled.

Emissions are based on decays from the chosen radionuclide and can be a mix of
beta+-, electron capture and alpha decays. Metastable radionuclides are supported.
Internal transition gamma emissions are
modeled event-by-event. The \ref EGS_RadionuclideSpectrum tracks
the energy level of excited daughter nuclides so that subsequent transitions and
electron shell cascades are correlated with specific disintegration events.

Each emission is assigned a shower index <b> \c ishower </b> and <b> \c time </b>
to allow for coincidence counting. These properties can be accessed for the
most recent emission from the source using something like:

<code>source->getShowerIndex();</code>

<code>source->getTime();</code>

Auger and fluorescent photon radiations are also modeled as a part of the
source, using the EADL relaxation scheme by default. Alternatively, the user can
request to use Auger and fluorescent photons from the ensdf file comments by
setting the spectrum input parameter '<code>atomic relaxations = ensdf</code>'.
For more information, see \ref EGS_RadionuclideSpectrum.

A radionuclide source is defined using the following input. Notice that the
format is similar to \ref EGS_IsotropicSource.
\verbatim
:start source:
    name                = my_mixture
    library             = egs_radionuclide_source
    activity            = total activity of mixture, assumed constant
    charge              = [optional] list including at least one of -1, 0, 1, 2
                          to include electrons, photons, positrons and alphas.
                          Filtering is applied to ALL emissions (including
                          relaxation particles).
                          Omit this option to include all charges - this is
                          recommended.
    geometry            = [optional] my_geometry # see egs_isotropic_source
    region selection    = [optional] geometry confinement option
                          one of IncludeAll, ExcludeAll,
                          IncludeSelected, ExcludeSelected
    selected regions    = [required for IncludeSelected, ExcludeSelected]
                          regions to apply geometry confinement
    :start shape:
        definition of the shape
    :stop shape:
    :start spectrum:
        definition of an EGS_RadionuclideSpectrum (see link below)
    :stop spectrum:
:stop source:
\endverbatim

The emission spectrum generation is described in \ref EGS_RadionuclideSpectrum.

<em>Note about emission times: </em>

The <b> \c time </b> of disintegration is sampled based on the
total activity of the <b> \c mixture </b> in \ref EGS_RadionuclideSource.
For uniform random number <b><code>u</code></b>, we sample the time to the
next disintegration, and increment the wall clock as follows:

<code>time += -log(1-u) / activity;</code>

The time of emission of a transition photon is determined by sampling
the delay that occurs after disintegration, according to the transition
<b> \c halflife </b>.

<code>time += -halflife * log(1-u) / ln(2);</code>

If you are using '<code>atomic relaxations = ensdf</code>' in the spectrum, then
the relaxation emissions are not correlated with disintegration events. This
means that getShowerIndex() and getTime() will not produce correct
results for non-disintegration emissions.

<em>A simple example:</em>
\verbatim
:start run control:
    ncase = 1e6
:stop run control:
:start geometry definition:
    :start geometry:
        name        = my_box
        library     = egs_box
        box size    = 1 2 3
        :start media input:
            media   = H2O521ICRU
        :stop media input:
    :stop geometry:
    :start geometry:
        name        = sphere1
        library     = egs_spheres
        midpoint    = 0 0 1
        radii       = 0.3
        :start media input:
            media   = AIR521ICRU
        :stop media input:
    :stop geometry:
    :start geometry:
        name        = sphere2
        library     = egs_spheres
        midpoint    = 0 0 -1
        radii       = 0.3
        :start media input:
            media   = AIR521ICRU
        :stop media input:
    :stop geometry:
    :start geometry:
        name        = my_envelope
        library     = egs_genvelope
        base geometry           = my_box
        inscribed geometries    = sphere1 sphere2
    :stop geometry:
    simulation geometry = my_envelope
:stop geometry definition:
:start source definition:
    :start source:
        name                = my_source
        library             = egs_radionuclide_source
        activity            = 28e6
        geometry            = my_envelope
        region selection    = IncludeSelected
        selected regions    = 1 2
        :start shape:
            type        = box
            box size    = 1 2 3
            :start media input:
                media   = H2O521ICRU
            :stop media input:
        :stop shape:
        :start spectrum:
            type        = radionuclide
            nuclide     = Ir-192
        :stop spectrum:
    :stop source:
    simulation source = my_source
:stop source definition:
\endverbatim
\image html egs_radionuclide_source.png "A simple example of two spheres emitting Ir-192 radiations"
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

    /*! \brief Gets the next particle from the radionuclide spectra */
    EGS_I64 getNextParticle(EGS_RandomGenerator *rndm,
                            int &q, int &latch, EGS_Float &E, EGS_Float &wt,
                            EGS_Vector &x, EGS_Vector &u);

    /*! \brief Returns the maximum energy out of all the spectra */
    EGS_Float getEmax() const {
        return Emax;
    };

    /*! \brief Returns the current fluence (number of disintegrations) */
    EGS_Float getFluence() const {
        return ishower+1;
    };

    /*! \brief Returns the emission time of the most recent particle */
    double getTime() const {
        return time;
    };

    /*! \brief Returns the shower index of the most recent particle */
    EGS_I64 getShowerIndex() const {
        return ishower;
    };

    /*! \brief Outputs the emission stats of the spectra */
    void printSampledEmissions() {
        for (unsigned int i=0; i<decays.size(); ++i) {
            decays[i]->printSampledEmissions();
        }
    }

    /*! \brief Calculates the position and direction of a new source particle */
    void getPositionDirection(EGS_RandomGenerator *rndm,
                              EGS_Vector &x, EGS_Vector &u, EGS_Float &wt);

    /*! \brief Checks the validity of the source */
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
    EGS_Float buf_1, buf_2;
    EGS_Float min_phi, max_phi;
    int       nrs;
    GeometryConfinement gc;

    vector<EGS_BaseSpectrum *> decays; //!< The radionuclide decay structure
    vector<int>         q_allowed; //!< A list of allowed charges
    bool                q_allowAll; //!< Whether or not to allow all charges
    bool                disintegrationOccurred; //!< Whether or not a disintegration occurred while generating the most recent source particle
    EGS_Float           activity, //!< The activity of the source
                        time; //!< The time of emission of the most recently generated particle
    EGS_I64             ishower; //!< The shower index (disintegration number) of the most recently generated particle
    EGS_Vector          xOfDisintegration; //!< The position of the last disintegration

private:
    EGS_Application *app;
};

#endif
