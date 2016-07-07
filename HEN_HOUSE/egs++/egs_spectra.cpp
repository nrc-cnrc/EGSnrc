/*
###############################################################################
#
#  EGSnrc egs++ spectra
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


/*! \file egs_spectra.cpp
 *  \brief EGS_BaseSpectrum implementation and several concrete spectra
 *  \IK
 */

#include "egs_base_source.h"
#include "egs_rndm.h"
#include "egs_alias_table.h"
#include "egs_input.h"
#include "egs_math.h"
#include "egs_ensdf.h"
#include "egs_application.h"

#include <cstdio>
#include "egs_math.h"
#include <fstream>
#ifndef NO_SSTREAM
    #include <sstream>
    #define S_STREAM std::istringstream
#else
    #include <strstream>
    #define S_STREAM std::istrstream
#endif



using namespace std;

void EGS_BaseSpectrum::reportAverageEnergy() const {
    egsInformation("expected average energy: %g\n",expectedAverage());
    EGS_Float e=0,de=0;
    getSampledAverage(e,de);
    egsInformation("sampled  average energy: %g +/- %g\n",e,de);
}

/*! \brief A monoenergetic particle spectrum.
  \ingroup egspp_main


  A monoenergetic spectrum is defined in the input file via
  \verbatim
  :start spectrum:
      type = monoenergetic
      energy = the kinetic energy of the particles
   :stop spectrum:
   \endverbatim
 */
class EGS_EXPORT EGS_MonoEnergy : public EGS_BaseSpectrum {

public:

    /*! \brief Construct a monoenergetic spectrum with energy \a energy. */
    EGS_MonoEnergy(EGS_Float energy) : EGS_BaseSpectrum(), E(energy) {
        char buf[1024];
        sprintf(buf,"monoenergetic %g MeV",E);
        type = buf;
    };
    ~EGS_MonoEnergy() {};
    EGS_Float expectedAverage() const {
        return E;
    };
    EGS_Float maxEnergy() const {
        return E;
    };

protected:

    EGS_Float sample(EGS_RandomGenerator *) {
        return E;
    };

    EGS_Float E; //!< The spectrum energy.

};

static inline EGS_Float getGaussianRN(EGS_RandomGenerator *rndm) {
    static bool have_x = false;
    static EGS_Float the_x;
    if (have_x) {
        have_x = false;
        return the_x;
    }
    EGS_Float r = sqrt(-2*log(1-rndm->getUniform()));
    EGS_Float cphi, sphi;
    rndm->getAzimuth(cphi,sphi);
    the_x = r*sphi;
    have_x = true;
    return r*cphi;
};

/*! \brief A Gaussian spectrum
 \ingroup egspp_main


 A Gaussian spectrum is defined in the input file via
\verbatim
:start spectrum:
    type = Gaussian
    mean energy = the mean kinetic energy
    sigma = the sigma of the spectrum
    or
    fwhm = the full-width-at-half-maximum of the spectrum
:stop spectrum:
\endverbatim

 */
class EGS_EXPORT EGS_GaussianSpectrum : public EGS_BaseSpectrum {

public:

    /*! \brief Construct a Gaussian spectrum with mean energy \a mean_energy
     * and width \a Sigma.
     *
     * If \a Sigma is less than zero, it is interpreted as -FWHM.
     * The mean energy \em must be greater than zero.
     */
    EGS_GaussianSpectrum(EGS_Float mean_energy, EGS_Float Sigma) :
        EGS_BaseSpectrum(), Eo(mean_energy), sigma(Sigma) {
        if (Eo <= 0) egsFatal("EGS_GaussianSpectrum: attempt to construct "
                                  "a spectrum with a negative mean energy (%g)\n",Eo);
        if (sigma < 0) {
            sigma = -sigma*0.4246609;    // i.e. assume
        }
        // the user has specified FWHM
        char buf[1024];
        sprintf(buf,"Gaussian spectrum with Eo = %g and sigma = %g",Eo,sigma);
        type = buf;
        if (Eo - 5*sigma > 0) {
            Emax = Eo + 5*sigma;
        }
        else {
            Emax = 2*Eo;
        }
    };
    ~EGS_GaussianSpectrum() {};
    EGS_Float expectedAverage() const {
        return Eo;
    };
    EGS_Float maxEnergy() const {
        return Emax;
    };

protected:

    EGS_Float sample(EGS_RandomGenerator *rndm) {
        EGS_Float E;
        do {
            E = Eo + sigma*getGaussianRN(rndm);
        }
        while (E <= 0 || E > Emax);
        return E;
    };

    EGS_Float  Eo;    //!< The mean energy
    EGS_Float  sigma; //!< The Gaussian width
    /*! \brief The maximum energy
     *
     * If \f$E_0\f$ is too close to zero (less than 5 standard deviations)
     * The Gaussian spectrum is truncated to \f$E_{\rm max}\f$ so that it
     * is still symmetric around \f$E_0\f$.
     */
    EGS_Float  Emax;
};

/*! \brief A double-Gaussian spectrum
 *
 * \ingroup egspp_main
 *
 *
 * A double-Gaussian spectrum is a spectrum that consists of two Gaussian
 * distributions with the same mean energy \f$E_0\f$ but different widths
 * smoothly joined together at \f$E_0\f$ so that for \f$E < E_0\f$ the
 * probability distribution is given by the first Gaussian and for
 * \f$E < E_0\f$ by the second. This type of spectrum has been used to
 * describe the spectrum of electrons impinging on the bremsstrahlung target
 * of some medical linear accelerators. It is defined via
\verbatim
:start spectrum:
    type = Double Gaussian
    mean energy = the mean kinetic energy Eo
    sigma = the two sigmas of the spectrum
    or
    fwhm = the two fwhms of the spectrum
:stop spectrum:
\endverbatim
 */
class EGS_EXPORT EGS_DoubleGaussianSpectrum : public EGS_BaseSpectrum {

public:

    /*! \brief Construct a double-Gaussian spectrum with mean energy
     * \a mean_energy and widths \a sig_left and \a sig_right.
     */
    EGS_DoubleGaussianSpectrum(EGS_Float mean_energy, EGS_Float sig_left,
                               EGS_Float sig_right) : EGS_BaseSpectrum(), Eo(mean_energy),
        sleft(sig_left), sright(sig_right) {
        if (sleft < 0) {
            sleft = -sleft*0.4246609;
        }
        if (sright< 0) {
            sright= -sright*0.4246609;
        }
        Emax = Eo + 4*sright;
        if (Eo - 4*sleft < 0) egsWarning("EGS_DoubleGaussianSpectrum: "
                                             "for Eo=%g, sigma=%g there will be negative energy sampled\n");
        p = sleft/(sleft + sright);
        char buf[1024];
        sprintf(buf,"Double Gaussian spectrum with Eo = %g sig(left) = %g"
                " and sig(right) = %g",Eo,sleft,sright);
        type = buf;
    };
    ~EGS_DoubleGaussianSpectrum() {};
    EGS_Float maxEnergy() const {
        return Emax;
    };
    EGS_Float expectedAverage() const {
        return Eo + sqrt(2/M_PI)*(sright-sleft);
    };

protected:

    EGS_Float Eo;     //!< The mean energy
    EGS_Float Emax;   //!< The maximum energy
    EGS_Float sleft;  //!< The width of the spectrum left of Eo
    EGS_Float sright; //!< The width of the spectrum right of Eo
    EGS_Float p;      /*!< The probability for picking energies from the left
                           or right Gaussian */

    EGS_Float sample(EGS_RandomGenerator *rndm) {
        EGS_Float E;
        if (rndm->getUniform() < p) {
            do {
                E = Eo-sleft*fabs(getGaussianRN(rndm));
            }
            while (E <= 0);
        }
        else {
            do {
                E = Eo+sright*fabs(getGaussianRN(rndm));
            }
            while (E > Emax);
        }
        return E;
    };

};

/*! \brief A uniform energy spectrum.
 *
 * \ingroup egspp_main
 *
 * A uniform energy spectrum is a spectrum where the probability distribution
 * is uniform between a minimum and a maximum energy and zero outside of
 * the energy interval. This type of spectrum is defined via
\verbatim
:start spectrum:
    type = uniform
    range = minimum and maximum energy
    or
    minimum energy = Emin
    maximum energy = Emax
:stop spectrum:
\endverbatim

 */
class EGS_EXPORT EGS_UniformSpectrum : public EGS_BaseSpectrum {

public:

    /*! \brief Construct a uniform spectrum between \a emin and \a emax. */
    EGS_UniformSpectrum(EGS_Float emin, EGS_Float emax) : EGS_BaseSpectrum(),
        Emin(emin), Emax(emax), de(emax - emin) {
        char buf[1024];
        sprintf(buf,"Uniform spectrum with Emin = %g Emax = %g",Emin,Emax);
        type = buf;
    };
    ~EGS_UniformSpectrum() {};
    EGS_Float maxEnergy() const {
        return Emax;
    };
    EGS_Float expectedAverage() const {
        return (Emin+Emax)/2;
    };

protected:

    EGS_Float sample(EGS_RandomGenerator *rndm) {
        return Emin + rndm->getUniform()*de;
    };

    EGS_Float Emin;  //!< The minimum energy
    EGS_Float Emax;  //!< The maximum energy
    EGS_Float de;    //!< Emax - Emin
};

/*! \brief A tabulated spectrum.
 *
 * \ingroup egspp_main
 *
 * A tabulated spectrum can be used to simulate any type of energy
 * distribution. Three different types of a tabulated spectrum are
 * available:
 *  - A line spectrum, \em i.e. the spectrum consists of a series
 *    of discrete energies \f$E_i\f$ with probabilities \f$p_i\f$.
 *    This type of spectrum is useful for instance for the simulation of
 *    some radioactive sources.
 *  - A histogram spectrum. In this case the spectrum consists of a series
 *    of energy bins \f$E_i ... E_{i+1}\f$ with probabilities \f$p_i\f$, where
 *    the distribution within a bin is uniform. This is the type of spectrum
 *    implemented by \c ensrc.mortran and used in the standard set of
 *    EGSnrc mortran user codes.
 *  - An 'interpolated' spectrum. This is similar to a histogram spectrum,
 *    however, the probabilities are considered to be at the bin edges and
 *    a linear interpolation is used between the bin edges. As a result,
 *    an interpolated spectrum with \f$N\f$ bins requires \f$N+1\f$ energies and
 *    \f$N+1\f$ probabilities, whereas a histogram spectrum with \f$N\f$ bins
 *    requires \f$N+1\f$ energies but \f$N\f$ probabilities.
 *
 * Internally the tabulated spectrum object uses
 * an \link EGS_AliasTable alias table object\endlink to
 * sample energies efficiently.

There are two ways for defining a tabulated spectrum by specifying the
name of a file containing a spectrum definition, or inline. In the former
case the keys needed to define a spectrum are
\verbatim
:start spectrum:
    type = tabulated spectrum
    spectrum file = the name of a spectrum file
:stop spectrum:
\endverbatim
The syntax of the spectrum file is the same as for the standard set
of EGSnrc user codes (see the example spectra distributed with the
system in $HEN_HOUSE/spectra), except that now the mode of the file can also
be 2 (line spectrum) or 3 (interpolated spectrum).
A spectrum is defined inline as follows:
\verbatim
:start spectrum:
    type = tabulated spectrum
    energies = list of discrete energies or bin edges
    probabilities = list of probabilities
    spectrum type = 0 or 1 or 2 or 3
:stop spectrum:
\endverbatim
where the meaning of the spectrum type is the same as the mode of a
spectrum file.
 */
class EGS_EXPORT EGS_TabulatedSpectrum : public EGS_BaseSpectrum {

public:

    /*! \brief Construct a tabulated spectrum from the energies \a x and
     *  the probabilities \a f.
     *
     * The interpretation of \a x and \a f depends on the spectrum type
     * specified by \a Type. For \a Type=0,1 it is considered to be a
     * histogram spectrum with N-1 bins defined by the N edge energies \a x.
     * (the difference between a \a Type=0 and a \a Type=1 spectrum is that
     * in the one case the probabilities \a f are considered to be counts
     * per bin whereas in the latter case they are counts per MeV). For
     * \a Type=2 the spectrum is considered to be a line spectrum with
     * \a N discrete energies and probabilities given by \a x and \a f.
     * For \a Type=3 the spectrum is considered to be an interpolated
     * spectrum.
     */
    EGS_TabulatedSpectrum(int N, const EGS_Float *x, const EGS_Float *f,
                          int Type = 1, const char *fname = 0) : EGS_BaseSpectrum(),
        table(new EGS_AliasTable(N,x,f,Type)) {
        setType(Type,fname);
    };

    ~EGS_TabulatedSpectrum() {
        delete table;
    };
    EGS_Float maxEnergy() const {
        return table->getMaximum();
    };
    EGS_Float expectedAverage() const {
        return table->getAverage();
    };

protected:

    EGS_AliasTable *table; //!< The alias table object used to sample energies.
    void setType(int Type,const char *fname) {
        if (Type == 0) {
            type = "tabulated line spectrum";
        }
        else if (Type == 1) {
            type = "tabulated histogram spectrum";
        }
        else {
            type = "tabulated spectrum";
        }
        if (fname) {
            type += " defined in ";
            type += fname;
        }
        else {
            type += " defined inline";
        }
    };

    EGS_Float sample(EGS_RandomGenerator *rndm) {
        return table->sample(rndm);
    };

};

/*! \brief A radionuclide spectrum.
 *
 * \ingroup egspp_main
 *



 */
class EGS_EXPORT EGS_RadionuclideSpectrum : public EGS_BaseSpectrum {

public:

    /*! \brief Construct a radionuclide spectrum.
     */
    EGS_RadionuclideSpectrum(const string isotope, const string ensdf_file,
                             const EGS_Float weight) :
        EGS_BaseSpectrum() {

        // Read in the data file for the isotope
        // and build the decay structure
        decays = new EGS_Ensdf(isotope, ensdf_file);

        // Normalize the emission and transition intensities
        decays->normalizeIntensities();

        // Get the particle records from the decay scheme
        myBetas = decays->getBetaRecords();
        myAlphas = decays->getAlphaRecords();
        myGammas = decays->getGammaRecords();
        myLevels = decays->getLevelRecords();
        xrayIntensities = decays->getXRayIntensities();
        xrayEnergies = decays->getXRayEnergies();
        augerIntensities = decays->getAugerIntensities();
        augerEnergies = decays->getAugerEnergies();

        // Initialization
        currentLevel = 0;
        Emax = 0;
        currentTime = 0;
        ishower = -1; // Start with ishower -1 so first shower has index 0

        // Get the maximum energy for emissions
        for (vector<BetaRecordLeaf *>::iterator beta = myBetas.begin();
                beta != myBetas.end(); beta++) {

            double energy = (*beta)->getFinalEnergy();
            if (Emax < energy) {
                Emax = energy;
            }
        }
        for (vector<AlphaRecord *>::iterator alpha = myAlphas.begin();
                alpha != myAlphas.end(); alpha++) {

            double energy = (*alpha)->getFinalEnergy();
            if (Emax < energy) {
                Emax = energy;
            }
        }
        for (vector<GammaRecord *>::iterator gamma = myGammas.begin();
                gamma != myGammas.end(); gamma++) {

            double energy = (*gamma)->getDecayEnergy();
            if (Emax < energy) {
                Emax = energy;
            }
        }
        for (unsigned int i=0; i < xrayEnergies.size(); ++i) {
            numSampledXRay.push_back(0);
            if (Emax < xrayEnergies[i]) {
                Emax = xrayEnergies[i];
            }
        }
        for (unsigned int i=0; i < augerEnergies.size(); ++i) {
            numSampledAuger.push_back(0);
            if (Emax < augerEnergies[i]) {
                Emax = augerEnergies[i];
            }
        }

        // Set the weight of the spectrum
        spectrumWeight = weight;

        printf("EGS_RadionuclideSpectrum: Emax: %f\n",Emax);
        printf("EGS_RadionuclideSpectrum: Weight: %f\n",weight);
    };

    ~EGS_RadionuclideSpectrum() {
        delete decays;
    };

    int getCharge() const {
        return currentQ;
    }

    double getTime() const {
        return currentTime;
    }

    void setMaximumTime(double maxTime) {
        Tmax = maxTime;
    }

    EGS_I64 getShowerIndex() const {
        return ishower;
    }

    EGS_Float getSpectrumWeight() const {
        return spectrumWeight;
    }

    void setSpectrumWeight(EGS_Float newWeight) {
        spectrumWeight = newWeight;
    }

    void printSampledEmissions() {
        printf("\nSampled %s emissions:\n", decays->radionuclide.c_str());
        printf("========================\n");
        printf("Energy | Intensity per 100 emissions\n");
        if (myBetas.size() > 0) {
            printf("Beta records:\n");
        }
        for (vector<BetaRecordLeaf *>::iterator beta = myBetas.begin();
                beta != myBetas.end(); beta++) {

            printf("%f %f\n", (*beta)->getFinalEnergy(),
                   ((EGS_Float)(*beta)->getNumSampled()/ishower)*100);
        }
        if (myAlphas.size() > 0) {
            printf("Alpha records:\n");
        }
        for (vector<AlphaRecord *>::iterator alpha = myAlphas.begin();
                alpha != myAlphas.end(); alpha++) {

            printf("%f %f\n", (*alpha)->getFinalEnergy(),
                   ((EGS_Float)(*alpha)->getNumSampled()/ishower)*100);
        }
        if (myGammas.size() > 0) {
            printf("Gamma records:\n");
        }
        for (vector<GammaRecord *>::iterator gamma = myGammas.begin();
                gamma != myGammas.end(); gamma++) {

            printf("%f %f\n", (*gamma)->getDecayEnergy(),
                   ((EGS_Float)(*gamma)->getNumSampled()/ishower)*100);
        }
        if (xrayEnergies.size() > 0) {
            printf("X-Ray records:\n");
        }
        for (unsigned int i=0; i < xrayEnergies.size(); ++i) {
            printf("%f %f\n", xrayEnergies[i],
                   ((EGS_Float)numSampledXRay[i]/ishower)*100);
        }
        if (augerEnergies.size() > 0) {
            printf("Auger records:\n");
        }
        for (unsigned int i=0; i < augerEnergies.size(); ++i) {
            printf("%f %f\n", augerEnergies[i],
                   ((EGS_Float)numSampledAuger[i]/ishower)*100);
        }
        printf("\n");
    }

protected:
    EGS_Float sample(EGS_RandomGenerator *rndm) {

        // Sample a uniform random number
        EGS_Float u = rndm->getUniform();

        // The energy of the sampled particle
        EGS_Float E;

        // If the daughter is in an excited state
        // Check for transitions
        if (currentLevel && currentLevel->getEnergy() > 0) {
//             printf("EGS_RadionuclideSpectrum:sample: excited daughter "
//                 "%f\n",currentLevel->getEnergy());

            for (vector<GammaRecord *>::iterator gamma = myGammas.begin();
                    gamma != myGammas.end(); gamma++) {

                if ((*gamma)->getLevelRecord() == currentLevel) {

                    if (u < (*gamma)->getTransitionIntensity()) {

                        (*gamma)->incrNumSampled();
                        currentQ = (*gamma)->getCharge();

                        currentTime += currentLevel->getHalfLife() /
                                       0.693147180559945309417232121458176568075500134360255254120680009493393
                                       * log(rndm->getUniform());

                        currentLevel = (*gamma)->getFinalLevel();



                        E = (*gamma)->getDecayEnergy();
                        return E;
                    }
                }
            }
        }
        else {
            // Incremember the shower number
            ishower++;

            // Uniformly distribute decays over the experiment time
            currentTime = rndm->getUniform() * Tmax;

            // Sample which decay occurs
            // Betas
            for (vector<BetaRecordLeaf *>::iterator beta = myBetas.begin();
                    beta != myBetas.end(); beta++) {
                if (u < (*beta)->getBetaIntensity()) {

                    (*beta)->incrNumSampled();
                    currentQ = (*beta)->getCharge();
                    //printf("EGS_RadionuclideSpectrum: q: %d\n",currentQ);

                    // Set the energy level of the daughter
                    currentLevel = (*beta)->getLevelRecord();

                    // TODO: Generate beta- spectrum
                    // TODO: Need to implement electron capture

                    // For now just uniform up to max!
                    E = u * (*beta)->getFinalEnergy();
                    //printf("\nEGS_RadionuclideSpectrum: E: %f\n",E);
                    return E;
                }
            }

            // Alphas
            for (vector<AlphaRecord *>::iterator alpha = myAlphas.begin();
                    alpha != myAlphas.end(); alpha++) {
                if (u < (*alpha)->getAlphaIntensity()) {

                    (*alpha)->incrNumSampled();
                    currentQ = (*alpha)->getCharge();

                    // Set the energy level of the daughter
                    currentLevel = (*alpha)->getLevelRecord();

                    // For alphas we simulate a disintegration but the
                    // transport will not be performed
                    return 0;
                }
            }

            // XRays
            for (unsigned int i=0; i < xrayIntensities.size(); ++i) {
                if (u < xrayIntensities[i]) {

                    numSampledXRay[i]++;
                    currentQ = 0;

                    E = xrayEnergies[i];

                    return E;
                }
            }

            // Auger electrons
            for (unsigned int i=0; i < augerIntensities.size(); ++i) {
                if (u < augerIntensities[i]) {

                    numSampledAuger[i]++;
                    currentQ = -1;

                    E = augerEnergies[i];

                    return E;
                }
            }
        }

        // Shouldn't get here
        return 0;
    };

    EGS_Float maxEnergy() const {
        return Emax;
    };

    EGS_Float expectedAverage() const {
        return 0;
    };

private:

    EGS_Ensdf                   *decays;
    vector<BetaRecordLeaf *>    myBetas;
    vector<AlphaRecord *>       myAlphas;
    vector<GammaRecord *>       myGammas;
    vector<LevelRecord *>       myLevels;
    vector<double>              xrayIntensities,
           xrayEnergies,
           augerIntensities,
           augerEnergies;
    vector<EGS_I64>             numSampledXRay,
           numSampledAuger;
    const LevelRecord           *currentLevel;
    int                         currentQ;
    EGS_Float                   currentTime,
                                Emax,
                                Tmax,
                                spectrumWeight;
    EGS_I64                     ishower;
};

//
// The following function is used to skip potentially present
// commas as separators in spectrum files. With it we can write
//   stream >> input1 >> skipsep >> input2;
// and it will work for white space and for comma separated input.
//
istream &skipsep(istream &in) {
    char c;
    for (EGS_I64 loopCount=0; loopCount<=loopMax; ++loopCount) {
        if (loopCount == loopMax) {
            egsFatal("skipsep: Too many iterations were required! Input may be invalid, or consider increasing loopMax.");
            return in;
        }
        in.get(c);
        if (in.eof() || in.fail() || !in.good()) {
            break;
        }
        if (c == ',') {
            break;
        }
        if (!isspace(c)) {
            in.putback(c);
            break;
        }
    }
    return in;
}

static char spec_msg1[] = "EGS_BaseSpectrum::createSpectrum:";

EGS_BaseSpectrum *EGS_BaseSpectrum::createSpectrum(EGS_Input *input) {
    if (!input) {
        egsWarning("%s got null input?\n",spec_msg1);
        return 0;
    }
    EGS_Input *inp = input;
    bool delete_it = false;
    if (!input->isA("spectrum")) {
        inp = input->takeInputItem("spectrum");
        if (!inp) {
            egsWarning("%s no 'spectrum' input!\n",spec_msg1);
            return 0;
        }
        delete_it = true;
    }
    string stype;
    int err = inp->getInput("type",stype);
    if (err) {
        egsWarning("%s wrong/missing 'type' input\n",spec_msg1);
        if (delete_it) {
            delete inp;
        }
        return 0;
    }
    EGS_BaseSpectrum *spec = 0;
    if (inp->compare(stype,"monoenergetic")) {
        EGS_Float Eo;
        err = inp->getInput("energy",Eo);
        if (err) egsWarning("%s wrong/missing 'energy' input for a "
                                "monoenergetic spectrum\n",spec_msg1);
        else {
            spec = new EGS_MonoEnergy(Eo);
        }
    }
    else if (inp->compare(stype,"Gaussian")) {
        EGS_Float Eo, sig, fwhm;
        int err1 = inp->getInput("mean energy",Eo);
        int err2 = inp->getInput("sigma",sig);
        int err3 = inp->getInput("fwhm",fwhm);
        if (err1 || (err2 && err3)) {
            if (err1) egsWarning("%s wrong/missing 'mean energy' input"
                                     " for a Gaussian spectrum\n",spec_msg1);
            else egsWarning("%s wrong/missing 'sigma' and 'FWHM' input for a "
                                "Gaussian spectrum\n",spec_msg1);

        }
        else {
            if (Eo <= 0) egsWarning("%s mean energy must be positive but your"
                                        " input was %g\n",Eo);
            else {
                if (!err2) {
                    if (sig <= 0) egsWarning("%s sigma must be positive"
                                                 " but your input was %g\n",spec_msg1,sig);
                    else {
                        spec = new EGS_GaussianSpectrum(Eo,sig);
                    }
                }
                else {
                    if (fwhm <= 0)  egsWarning("%s FWHM must be positive"
                                                   " but your input was %g\n",spec_msg1,fwhm);
                    else {
                        spec = new EGS_GaussianSpectrum(Eo,-fwhm);
                    }
                }
            }
        }
    }
    else if (inp->compare(stype,"Double Gaussian")) {
        EGS_Float Eo;
        vector<EGS_Float> sig, fwhm;
        int err1 = inp->getInput("mean energy",Eo);
        int err2 = inp->getInput("sigma",sig);
        int err3 = inp->getInput("fwhm",fwhm);
        if (!err1 && Eo <= 0) {
            err1 = 1;
        }
        if (!err2 && sig.size() != 2) {
            err2 = 1;
        }
        if (!err2 && (sig[0] <= 0 || sig[1] <= 0)) {
            err2 = 1;
        }
        if (!err3 && fwhm.size() != 2) {
            err3 = 1;
        }
        if (!err3 && (fwhm[0] <= 0 || fwhm[1] <= 0)) {
            err3 = 1;
        }
        if (err1 || (err2 && err3)) {
            if (err1) egsWarning("%s wrong/missing 'mean energy' input"
                                     " for a Double Gaussian spectrum\n",spec_msg1);
            if (err2 && err3) egsWarning("%s wrong/missing 'sigma' and 'FWHM'"
                                             " input for a Double Gaussian spectrum\n",spec_msg1);
        }
        else {
            if (!err2 && !err3) egsWarning("%s found 'sigma' and 'FWHM' "
                                               "input, using 'sigma'\n",spec_msg1);
            if (!err2) {
                spec = new EGS_DoubleGaussianSpectrum(Eo,sig[0],sig[1]);
            }
            else {
                spec = new EGS_DoubleGaussianSpectrum(Eo,-fwhm[0],-fwhm[1]);
            }
        }
    }
    else if (inp->compare(stype,"uniform")) {
        vector<EGS_Float> range;
        EGS_Float Emin, Emax;
        int err1 = inp->getInput("range",range);
        int err2 = inp->getInput("minimum energy",Emin);
        int err3 = inp->getInput("maximum energy",Emax);
        if (!err2 && !err3 && Emin > Emax) {
            egsWarning("%s Emin (%g) is greater than Emax (%g)?\n",
                       spec_msg1,Emin,Emax);
            err2 = 1;
            err3 = 1;
        }
        if (err1 && err2 && err3) egsWarning("%s wrong/missing 'range' and"
                                                 " 'minimum/maximum energy' input\n",spec_msg1);
        else {
            if (!err2 && !err3) {
                spec = new EGS_UniformSpectrum(Emin,Emax);
            }
            else {
                if (range[0] < range[1]) {
                    spec = new EGS_UniformSpectrum(range[0],range[1]);
                }
                else {
                    spec = new EGS_UniformSpectrum(range[1],range[0]);
                }
            }
        }
    }
    else if (inp->compare(stype,"tabulated spectrum")) {
        string spec_file;
        err = inp->getInput("spectrum file",spec_file);
        if (!err) {
            ifstream sdata(spec_file.c_str());
            if (!sdata) egsWarning("%s failed to open spectrum file %s\n",
                                       spec_msg1,spec_file.c_str());
            else {
                char title[1024];
                sdata.getline(title,1023);
                if (sdata.eof() || sdata.fail() || !sdata.good()) {
                    egsWarning("%s error while reading title of spectrum file"
                               "%s\n",spec_msg1,spec_file.c_str());
                    if (delete_it) {
                        delete inp;
                    }
                    return 0;
                }
                if (sdata.eof() || sdata.fail() || !sdata.good()) {
                    egsWarning("%s error while reading spectrum type and "
                               "number of bins in spectrum file %s\n",
                               spec_msg1,spec_file.c_str());
                    if (delete_it) {
                        delete inp;
                    }
                    return 0;
                }
                EGS_Float dum;
                int nbin, mode;
                sdata >> nbin >> skipsep >> dum >> skipsep >> mode;
                if (sdata.eof() || sdata.fail() || !sdata.good()) {
                    egsWarning("%s error while reading spectrum type and "
                               "number of bins in spectrum file %s\n",
                               spec_msg1,spec_file.c_str());
                    if (delete_it) {
                        delete inp;
                    }
                    return 0;
                }
                if (nbin < 2) {
                    egsWarning("%s nbin in a spectrum must be at least 2\n"
                               "  you have %d in the spectrum file %s\n",
                               spec_msg1,nbin,spec_file.c_str());
                    if (delete_it) {
                        delete inp;
                    }
                    return 0;
                }
                if (mode < 0 || mode > 3) {
                    egsWarning("%s unknown spectrum type %d in spectrum file"
                               " %s\n",spec_msg1,mode,spec_file.c_str());
                    if (delete_it) {
                        delete inp;
                    }
                    return 0;
                }
                EGS_Float *en_array, *f_array;
                int ibin;
                f_array = new EGS_Float [nbin];
                if (mode == 0 || mode == 1) {
                    en_array = new EGS_Float [nbin+1];
                    en_array[0] = dum;
                    ibin=1;
                }
                else {
                    en_array = new EGS_Float [nbin];
                    ibin=0;
                }
                for (int j=0; j<nbin; j++) {
                    sdata >> en_array[ibin++] >> skipsep >> f_array[j];
                    if (sdata.eof() || sdata.fail() || !sdata.good()) {
                        egsWarning("%s error on line %d in spectrum file %s\n",
                                   spec_msg1,j+2,spec_file.c_str());
                        if (delete_it) {
                            delete inp;
                        }
                        delete [] en_array;
                        delete [] f_array;
                        return 0;
                    }
                    if (mode != 2 && ibin > 1) {
                        if (en_array[ibin-1] <= en_array[ibin-2]) {
                            egsWarning("%s energies must be in increasing "
                                       "order.\n   This is not the case for input on "
                                       "lines %d,%d in spectrum file %s\n",
                                       spec_msg1,j+2,j+1,spec_file.c_str());
                            if (delete_it) {
                                delete inp;
                            }
                            return 0;
                        }
                    }
                    if (mode == 0) {
                        f_array[j]/=(en_array[ibin-1]-en_array[ibin-2]);
                    }
                }
                int itype = 1;
                if (mode == 2) {
                    itype = 0;
                }
                else if (mode == 3) {
                    itype = 2;
                }
                int nb = itype == 1 ? nbin+1 : nbin;
                spec = new EGS_TabulatedSpectrum(nb,en_array,f_array,itype,
                                                 spec_file.c_str());
                delete [] en_array;
                delete [] f_array;
            }
        }
        else {
            vector<EGS_Float> eners, probs;
            int itype=1, mode;
            int err1 = inp->getInput("energies",eners);
            int err2 = inp->getInput("probabilities",probs);
            int err3 = inp->getInput("spectrum mode",mode);      // according to EGSnrc convention
            if (err3) {
                err3 = inp->getInput("spectrum type",mode);      // deprecated
            }
            if (err3) {
                egsWarning("%s wrong/missing 'spectrum mode' input\n",spec_msg1);
                if (delete_it) {
                    delete inp;
                }
                return 0;
            }
            else {
                if (mode < 0 || mode > 3) {
                    egsWarning("%s unknown spectrum 'mode' %d"
                               " %s\n",spec_msg1,mode,spec_file.c_str());
                    if (delete_it) {
                        delete inp;
                    }
                    return 0;
                }
                if (mode == 2) {
                    itype = 0;
                }
                else if (mode == 3) {
                    itype = 2;
                }
            }
            if (err1 || err2) {
                if (err1) egsWarning("%s wrong/missing 'energies' input\n",
                                         spec_msg1);
                if (err2) egsWarning("%s wrong/missing 'probabilities' "
                                         "input\n",spec_msg1);
            }
            else {
                if (itype == 1 && probs.size() != eners.size()-1)
                    egsWarning("%s for spectrum type 1 the number of energies"
                               " must be the number of probabilities + 1\n",
                               spec_msg1);
                else if ((itype == 0 || itype == 2) &&
                         probs.size() != eners.size())
                    egsWarning("%s for spectrum types 0 and 2 the number of "
                               "energies must be equal to the number of probabilities\n",
                               spec_msg1);
                else {
                    int nbin = eners.size();
                    int nbin1 = itype == 1 ? nbin-1 : nbin;
                    EGS_Float *x = new EGS_Float [nbin],
                    *f = new EGS_Float [nbin1];
                    int ibin = 0;
                    if (itype == 1) {
                        ibin = 1;
                        x[0] = eners[0];
                    }
                    for (int j=0; j<nbin1; j++) {
                        x[ibin] = eners[ibin];
                        ibin++;
                        f[j] = mode == 0 ? probs[j]/(eners[ibin-1]-eners[ibin-2]) : probs[j];
                        if (itype != 0 && ibin > 1) {
                            if (x[ibin-1] <= x[ibin-2]) {
                                egsWarning("%s energies must be given in "
                                           "increasing order\n  This is not the case"
                                           " for inputs %d and %d (%g,%g) %d\n",
                                           spec_msg1,ibin-2,ibin-1,x[ibin-2],x[ibin-1],
                                           j);
                                if (delete_it) {
                                    delete inp;
                                }
                                delete [] x;
                                delete [] f;
                                return 0;
                            }
                        }
                    }
                    spec = new EGS_TabulatedSpectrum(nbin,x,f,itype,0);
                    delete [] x;
                    delete [] f;
                }
            }
        }
    }
    else if (inp->compare(stype,"radionuclide")) {
        string isotope;
        err = inp->getInput("isotope",isotope);
        if (err) {
            egsWarning("%s wrong/missing 'isotope' input\n",spec_msg1);
            return 0;
        }

        EGS_Float weight;
        err = inp->getInput("weight",weight);
        if (err) {
            weight = 1;
        }

        // For ensdf input, first check for the input argument
        string ensdf_file;
        err = inp->getInput("ensdf file",ensdf_file);

        // If not passed as input, find the ensdf file in the
        // directory $HEN_HOUSE/spectra/lnhb
        if (err) {

            EGS_Application *app = EGS_Application::activeApplication();
            if (app) {
                ensdf_file = egsJoinPath(app->getHenHouse(),"spectra");
                ensdf_file = egsJoinPath(ensdf_file.c_str(),"lnhb");
            }
            else {
                char *hen_house = getenv("HEN_HOUSE");
                if (!hen_house) {

                    egsWarning("EGS_BaseSpectrum::createSpectrum: "
                               "No active application and HEN_HOUSE not defined.\n"
                               "Assuming local directory for spectra\n");
                    ensdf_file = "./";
                }
                else {
                    ensdf_file = egsJoinPath(hen_house,"spectra");
                    ensdf_file = egsJoinPath(ensdf_file.c_str(),"lnhb");
                }
            }
            ensdf_file = egsJoinPath(ensdf_file.c_str(),isotope.append(".txt"));
        }
        spec = new EGS_RadionuclideSpectrum(isotope, ensdf_file, weight);
    }
    else {
        egsWarning("%s unknown spectrum type %s\n",spec_msg1,stype.c_str());
    }
    if (delete_it) {
        delete inp;
    }
    return spec;
}
