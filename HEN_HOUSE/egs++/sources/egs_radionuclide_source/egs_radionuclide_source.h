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
#  Contributors:    Martin Martinov
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
#include "egs_spectra.cpp"

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
        #define EGS_RADIONUCLIDE_SOURCE_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_RADIONUCLIDE_SOURCE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_RADIONUCLIDE_SOURCE_EXPORT
        #define EGS_RADIONUCLIDE_SOURCE_LOCAL
    #endif

#endif

/*! \brief A radionuclide source.

\ingroup Sources

A radionuclide source is a source that delivers particles according to a
radionuclide decay scheme. It follows events decay-by-decay, so that internal
transitions and atomic relaxations from decay events are produced in a
correlated fashion from the same location.

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
- Internal pair production is sampled but the electron positron pair is not
produced.
- Due to limitations in the current LNHB ensdf format, electron shell vacancy
creation due to disintegrations is sampled per-shell rather than
considering sub-shell probabilities. I.e. probabilities for vacancy creation
in the K,L,M,N,O shells are considered, but not L1, L2, etc. The actual
sub-shell in which the vacancy created is sampled uniformly for the given
shell. This is an approximation, and only relevant when
'<code>atomic relaxations = eadl</code>' in
\ref EGS_RadionuclideSpectrum.
- Alpha particles are not transported.
- Atomic motion & recoil from emissions is not modeled.
- For some radionuclides the decay intensities do not add to exactly 100%, due to
uncertainties on the intensity values in the data files. To normalize the decay intensities and
allow for modeling, the discrepancy from 100% is divided over all decays,
scaled by the uncertainty of each decay.

Emissions are based on decays from the chosen radionuclide and can be a mix of
beta+-, electron capture and alpha decays. Metastable radionuclides are supported.
Internal transitions are
modeled event-by-event. The \ref EGS_RadionuclideSpectrum keeps track of
the energy level of excited daughter nuclides so that subsequent transitions and
electron shell cascades are correlated with specific disintegration events.

Each emission is assigned a shower index <b> \c ishower </b> and <b> \c time </b>
to allow for coincidence counting. These properties can be accessed for the
most recent emission from the source using something like the following. Note
that the time does not include any time-of-flight measures.

<code>source->getShowerIndex();</code>

<code>source->getTime();</code>

Auger and fluorescent photon radiations are also modeled as a part of the
source, using the EADL relaxation scheme by default. Alternatively, the user can
request to use Auger and fluorescent photons from the ensdf file comments by
setting the spectrum input parameter '<code>atomic relaxations = ensdf</code>'.
For more information, see \ref EGS_RadionuclideSpectrum.

Note that results are usually normalized by the source fluence, which is
returned by the \ref getFluence() function of the base source. The calculation of the fluence
depends on the selected base source, but the <b> \c N </b> used depends
on the number of disintegration events (tracked by <b> \c ishower </b>). This
is distinct from the <b> \c ncase </b> input parameter, which is the
number of particles returned by the source (includes relaxations etc.).

A radionuclide source is defined using the following input. It imports a base
source and uses the base source getNextParticle() invocation to determine decay
location and direction. The energy spectrum of the base source is not used, but it
is still a required input so must be specified. This implementation leads to
increased random number sampling than
that strictly required in the simulation, due to the information generated in
the base source beyond particle position/direction that is not used in
egs_radionuclide_source. For this reason it's recommended to set the base source
to use a monoenergetic spectrum.

\verbatim
:start source:
    name                = my_mixture
    library             = egs_radionuclide_source
    base source         = name of the source used to generate decay locations
    activity            = [optional, default=1] total activity of mixture,
                          assumed constant. The activity only affects the
                          emission times assigned to particles.
    charge              = [optional] list including at least one of -1, 0, 1, 2
                          to include electrons, photons, positrons and alphas.
                          Filtering is applied to ALL emissions (including
                          relaxation particles).
                          Omit this option to include all charges - this is
                          recommended.
    experiment time     = [optional, default=0] time length of the experiment,
                          set to 0 for no time limit. Source particles generated
                          after the experiment time are not transported.

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
        name                = my_base_source
        library             = egs_isotropic_source
        geometry            = my_envelope
        region selection    = IncludeSelected
        selected regions    = 1 2
        :start shape:
            type        = box
            box size    = 1 2 3
        :stop shape:
        :start spectrum: # This will not actually be used, but is required
            type = monoenergetic
            energy = 1
        :stop spectrum:
    :stop source:
    :start source:
        name                = my_radionuclide
        library             = egs_radionuclide_source
        base source         = my_base_source
        :start spectrum:
            type        = radionuclide
            nuclide     = Ir-192
        :stop spectrum:
    :stop source:
    simulation source = my_radionuclide
:stop source definition:
\endverbatim
\image html egs_radionuclide_source.png "A simple example of two spheres emitting Ir-192 radiations"
*/

class EGS_RADIONUCLIDE_SOURCE_EXPORT EGS_RadionuclideSource :
    public EGS_BaseSource {

public:

    /*! \brief Constructor from input file */
    EGS_RadionuclideSource(EGS_Input *, EGS_ObjectFactory *f=0);

    /*! \brief Destructor */
    ~EGS_RadionuclideSource() {
        if (baseSource)
            if (!baseSource->deref()) {
                delete baseSource;
            }

        for (vector<EGS_RadionuclideSpectrum * >::iterator it =
                    decays.begin();
                it!=decays.end(); it++) {
            delete *it;
            *it=0;
        }
        decays.clear();
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
        return (ishower+1)*(baseSource->getFluence()/sCount); //!< Scale ishower+1 return by fluence ratio returned by file
    };

    /*! \brief Returns the emission time of the most recent particle */
    double getTime() const {
        return time;
    };

    /*! \brief Get the total possible length of the experiment that is being modelled
     *
     * This method returns the total experiment time specified by the user.
     * This is used to exclude time-delayed source emissions that would occur
     * after the modelled experiment.
     */
    double getExperimentTime() const {
        return experimentTime;
    };

    /*! \brief Returns the shower index of the most recent particle */
    EGS_I64 getShowerIndex() const {
        return ishower;
    };

    unsigned int getEmissionType() const {
        return emissionType;
    }

    /*! \brief Outputs the emission stats of the spectra */
    void printSampledEmissions() {
        egsInformation("\n======================================================\n");
        egsInformation("Start of source emissions statistics:\n");
        for (unsigned int i=0; i<decays.size(); ++i) {
            decays[i]->printSampledEmissions();
        }
        egsInformation("End of source emissions statistics\n");
        egsInformation("======================================================\n\n");
    };

    /*! \brief Checks the validity of the source */
    bool isValid() const {
        return baseSource;
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

private:
    EGS_Application *app;

    EGS_I64             count; //!< Number of times the spectrum was sampled
    EGS_Float           Emax; //!< Maximum energy the spectrum may return

    void setUp();

    string sName; //!< Name of the base source
    EGS_I64 sCount; //!< Name of the base source
    EGS_BaseSource *baseSource; //!< Pointer to the base source

    vector<int>         q_allowed; //!< A list of allowed charges
    vector<EGS_RadionuclideSpectrum *> decays; //!< The radionuclide decay structure
    EGS_Float           activity; //!< The activity of the source

    bool                q_allowAll; //!< Whether or not to allow all charges
    bool                disintegrationOccurred; //!< Whether or not a disintegration occurred while generating the most recent source particle
    EGS_Float           time, //!< The time of emission of the most recently generated particle
                        experimentTime, //!< The time length of the experiment that is being modelled
                        lastDisintTime; //!< The time of emission of the last disintegration
    EGS_I64             ishower; //!< The shower index (disintegration number) of the most recently generated particle
    EGS_Vector          xOfDisintegration; //!< The position of the last disintegration

    unsigned int emissionType;
};

#endif
