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
 *  \IK
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

/*! \brief A radionuclide source

  \ingroup Sources

A radionuclide source is a source that delivers particles with
directions uniformly distributed in \f$4 \pi\f$ emitted from
\link EGS_BaseShape any shape. Emissions are based on decays from the
radionuclide isotope and can be a mix of beta decays, X-radiations, etc.
\endlink
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
ensdf file in $HEN_HOUSE/spectra/lnhb if ensdf file not provided
        ensdf file = [optional] path to a spectrum file in ensdf format
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

//     EGS_I64 getNextParticle(EGS_RandomGenerator *rndm,
//                             int &q, int &latch, EGS_Float &E, EGS_Float &wt,
//                             EGS_Vector &x, EGS_Vector &u, EGS_I64 &ishower,
//                             EGS_Float &time);

    EGS_Float getEmax() const {
        return Emax;
    };

    EGS_Float getFluence() const {
        return count;
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

        EGS_Float sinz = 1-u.z*u.z;
        if (sinz > 1e-15) {
            sinz = sqrt(sinz);
            EGS_Float cphi, sphi;
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
    bool storeState(ostream &data_out) const {
        if (!egsStoreI64(data_out,count)) {
            return false;
        }
        for (unsigned int i=0; i<decays.size(); ++i) {
            if (!decays[i]->storeState(data_out)) {
                return false;
            }
        }
        if (!storeFluenceState(data_out)) {
            return false;
        }
        return true;
    };

    /*! \brief Add the source state from the stream \a data to the
     * current state.
     *
     * Uses the \link EGS_BaseSpectrum::addState() addState() \endlink
     * of the spectrum object and the addFluenceData() virtual function.
     */
    bool addState(istream &data) {
        EGS_I64 count_save = count;
        if (!egsGetI64(data,count)) {
            return false;
        }
        for (unsigned int i=0; i<decays.size(); ++i) {
            if (!decays[i]->addState(data)) {
                return false;
            }
        }
        if (!addFluenceData(data)) {
            return false;
        }
        count += count_save;
        return true;
    };

    /*! \brief Reset the source to a state with zero sampled particles.
     *
     * Uses the \link EGS_BaseSpectrum::resetCounter() resetCounter() \endlink
     * function of the spectrum object and the virtual function
     * resetFluenceCounter().
     */
    void resetCounter() {
        count = 0;
        for (unsigned int i=0; i<decays.size(); ++i) {
            decays[i]->resetCounter();
        }
        resetFluenceCounter();
    };

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
    bool setState(istream &data) {
        if (!egsGetI64(data,count)) {
            return false;
        }
        for (unsigned int i=0; i<decays.size(); ++i) {
            if (!decays[i]->setState(data)) {
                return false;
            }
        }
        if (!setFluenceState(data)) {
            return false;
        }
        return true;
    };

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
    vector<int>         q_allowed;
    EGS_Float           activity;
    double              time;
    EGS_I64             ishower;
};

#endif
