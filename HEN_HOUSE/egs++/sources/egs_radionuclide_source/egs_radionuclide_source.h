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
#include "egs_ensdf.cpp"

#include <algorithm>
#include <complex>


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

/*! \brief Beta spectrum generation for \ref EGS_RadionuclideSpectrum
 *
 * \ingroup egspp_main
 *
 *  Based on code by L. VanderZwan March 7, 1985.
 *
 *  Patrick Saull improved the code and ported from fortran to c++ in 2014.
 *
 *  Reid Townson integrated spectrum generation into egs++ in 2016.
*/
class EGS_EXPORT EGS_RadionuclideBetaSpectrum {

public:
    /*! \brief Construct beta spectra for a radionuclide
     */
    EGS_RadionuclideBetaSpectrum(EGS_Ensdf *decays, const string outputBetaSpectra) {

        EGS_Application *app = EGS_Application::activeApplication();
        rm = app->getRM();

        vector<BetaRecordLeaf *> myBetas = decays->getBetaRecords();

        for (vector<BetaRecordLeaf *>::iterator beta = myBetas.begin();
                beta != myBetas.end(); beta++) {

            // Skip electron capture records
            if ((*beta)->getCharge() == 1 &&
                    (*beta)->getPositronIntensity() == 0) {
                continue;
            }

            unsigned short int daughterZ = (*beta)->getZ();

            egsInformation("EGS_RadionuclideBetaSpectrum: "
                           "Energy, Z, A, forbidden: %f %d %d %d\n",
                           (*beta)->getFinalEnergy(), daughterZ,
                           (*beta)->getAtomicWeight(), (*beta)->getForbidden()
                          );

            const int nbin=1000;
            EGS_Float *e = new EGS_Float [nbin];
            EGS_Float *spec = new EGS_Float [nbin];
            EGS_Float *spec_y = new EGS_Float [nbin];

            double de, s_y, factor, se_y;

            ncomps=1; // if we increase this, then we must fill the remainder
            area[0]=1.0;
            rel[0]=1.0;

            emax = (*beta)->getFinalEnergy();
            zzz[0] = (double)daughterZ;
            rmass = (*beta)->getAtomicWeight();

            // These are some special cases where fudge factors are used!
            // Specify lamda[0]=4 for special shape factors

            // For Cl-36 (ref: nuc. phys. 99a,  625,(67))
            // Only the beta- spectrum
            if (daughterZ == 18 && (*beta)->getCharge() == -1) {
                lamda[0] = 4;
            }
            // For I-129 (ref: phys. rev. 95, 458, 54))
            // The beta- spectrum with 151 keV endpoint
            else if (daughterZ == 54 && emax < 0.154 && emax > 0.150) {
                lamda[0] = 4;
            }
            // For Cs-137 (ref: nuc. phys. 112a, 156, (68))
            // The beta- spectrum with 1175 keV endpoint
            else if (daughterZ == 56 && emax > 1.173 && emax < 1.177) {
                lamda[0] = 4;
            }
            // For Tl-204 (ref: can. j. phys., 45, 2621, (67))
            // There is only 1 beta- spectrum
            else if (daughterZ == 82) {
                lamda[0] = 4;
            }
            // For Bi-210 (ref: nuc. phys., 31, 293, (62))
            // There is only 1 beta- spectrum
            else if (daughterZ == 84) {
                lamda[0] = 4;
            }
            else {
                lamda[0] = (*beta)->getForbidden();
            }

            // For positrons from zzz negative (just how the spectrum code
            // was designed)
            if ((*beta)->getCharge() == 1) {
                zzz[0] *= -1;
            }

            etop[0]=emax;

            // prbs july 9, 2007 moved here from before src loop.
            // also, now tabulate based on
            // endpoint, using roughly nbin bins across spectrum. Do this by
            // rounding
            // endpoint E0 up to nearest 100 keV, and dividing by NBIN to get
            // binwidth.

            de=((int)(etop[0]*10.0+1)/10.)/nbin; // round up to nearest 100kev;
            // /=     NBIN
            //cout << "Binwidth " << de << endl;

            for (int ib=0; ib<nbin; ib++) {
                e[ib]=de+ib*de;
//                 egsInformation("%.12f, %.12f\n", e[ib], etop[0]);
            }

            s_y=0.0;
            se_y=0.0;
            for (int ib=0; ib<nbin; ib++) {

                if (e[ib]<=emax) {
                    sp(e[ib],spec_y[ib],factor);
                }
                else {
                    spec_y[ib]=0.0;
                }

                s_y=s_y+spec_y[ib];
                se_y=se_y+spec_y[ib]*e[ib];
            }

            for (int ib=0; ib<nbin; ib++) {
                spec[ib]=1/de*(spec_y[ib]/s_y);
//                 cout << e[ib] << " " << spec[ib] << endl;
            }

            EGS_AliasTable *bspec = new EGS_AliasTable(nbin,e,spec,1);
            (*beta)->setSpectrum(bspec);

            // Write the spectrum to a file
            if (outputBetaSpectra == "yes") {

                ostringstream ostr;
                ostr << decays->radionuclide << "_" << emax << ".spec";

                egsInformation("EGS_RadionuclideBetaSpectrum: Outputting beta spectrum to file: %s\n", ostr.str().c_str());

                ofstream specStream;
                specStream.open(ostr.str().c_str());
                for (int ib=0; ib<nbin; ib++) {
                    spec[ib]=1/de*(spec_y[ib]/s_y);
                    specStream << e[ib] << " " << spec[ib] << endl;
                }
                specStream.close();
            }
        }
    }

protected:

    complex<double> cgamma(complex<double> z) {

        static const int g=7;
        static const double pi = 3.1415926535897932384626433832795028841972;
        static const double p[g+2] = {0.99999999999980993, 676.5203681218851,
                                      -1259.1392167224028,
                                      771.32342877765313,
                                      -176.61502916214059,
                                      12.507343278686905,
                                      -0.13857109526572012,
                                      9.9843695780195716e-6,
                                      1.5056327351493116e-7
                                     };

        if (real(z)<0.5) {
            return pi / (sin(pi*z)*cgamma(double(1.0)-z));
        }

        z -= 1.0;
        complex<double> x=p[0];
        for (int i=1; i<g+2; i++) {
            x += p[i]/(z+complex<double>(i,0));
        }
        complex<double> t = z + (g + double(0.5));

        return double(sqrt(2.*pi)) * pow(t,z+double(0.5)) * exp(-t) * x;
    }

    complex<double> clgamma(complex<double> z) {
        complex<double> u, v, h, p, r;

        static const double pi = 3.1415926535897932384626433832795028841972;
        static const double c1 = 9.189385332046727e-1;
        static const double c2 = 1.144729885849400;
        static const double c[10] = {8.333333333333333e-2,
                                     -2.777777777777777e-3,
                                     7.936507936507936e-4,
                                     -5.952380952380952e-4,
                                     8.417508417508417e-4,
                                     -1.917526917526917e-3,
                                     6.410256410256410e-3,
                                     -2.955065359477124e-2,
                                     1.796443723688305e-1,
                                     -1.392432216905901
                                    };

        static const double hf = 0.5;

        double x = real(z);
        double y = imag(z);
        h = 0;

        if (y == 0 && -abs(x) == int(x)) {
            return 0;
        }
        else {
            double ya = abs(y);
            if (x < 0) {
                u = double(1.) - complex<double>(x, ya);
            }
            else {
                u = complex<double>(x, ya);
            }

            h = 0;
            double ur = real(u);
            double ui, a;
            if (ur < 7.) {
                ui = imag(u);
                a = atan2(ui,ur);
                h = u;
                for (int i=1; i<=6-int(ur); i++) {
                    ur = ur + 1;
                    u = complex<double>(ur, ui);
                    h = h * u;
                    a = a + atan2(ui, ur);
                }
                h = complex<double>(hf * log(pow(real(h),2) + pow(imag(h),2)),
                                    a);

                u = double(1.) + u;
            }

            r = double(1.) / pow(u,2);
            p = r * c[9];

            for (int i=8; i>=1; i--) {
                p = r * (c[i] + p);
            }

            h = c1 + (u-hf)*log(u) - u + (c[0]+p) / u - h;

            if (x < 0.) {
                ur = double(int(x)) - 1.;
                ui = pi * (x-ur);
                x = pi * ya;
                double t = exp(-x-x);
                a = sin(ui);
                t = x + hf * log(t*pow(a,2)+pow(hf*(1.-t),2));
                a = atan2(cos(ui)*tanh(x),a) - ur*pi;
                h = c2 - complex<double>(t,a) - h;
            }
            if (y < 0) {
                h = conj(h);
            }
        }

        return h;
    }

    void slfact(double p, double z, double radf, double xl[4]) {

        double ff[4];
        double dfac[4]= {1.0, 3.0, 15.0, 105.0};
        double pi,c137,az,w,rad,pr,y,x1,gk,bb,cc,dd,x2;

        complex<double> aa;

        pi  = acos(-1.0);
        c137= 137.036; // 1/ fine structure constant
        az  = z/c137;
        w   = sqrt(p*p+1.0);
        rad = radf/386.159;
        pr  = p*rad;
        y   = az*w/p;

        for (int k=1; k<=4; k++) {

            gk = sqrt(k*k-az*az);
            x1 = pow((pow(p,k-1)/dfac[k-1]) ,2);

            aa = clgamma(complex<double>(gk,y));
            double aa_real = real(aa);

            bb=lgamma((double)k);
            cc=lgamma(2.0*k +1.0);
            dd=lgamma(2.0*gk+1.0);

            ff[k-1] =
                pow(2.0*pr, 2.0*(gk-k)) *
                exp(pi*y+2.0*(aa_real+cc-bb-dd)) *
                (k+gk)/(2.0*k);

            x2 =
                1.0 -
                az*pr*(2.0*w*(2.0*k+1.0)/(p*k*(2.0*gk+1.0)) -
                       2.0*p*gk/(w*k*(2.0*gk+1.0))) -
                2.0*k*pr*pr/((2.0*k+1.0)*(k+gk));
            xl[k-1] = x1*ff[k-1]/ff[0]*x2;
        }

        return;
    }

    void bsp(double e, double &bspec, double &factor) {

        // *****************************************************************
        //     Calculates n(e) (unnormalized) for one spectral component,
        //     specified by:zz,emax,lamda, where
        //
        //     lamda = 1  first  forbidden
        //     lamda = 2  second forbidden
        //     lamda = 3  third  forbidden
        //     lamda = 0  otherwise
        //     lamda = 4  for a few nuclides whose experimental shape don't
        //                fit theory. fudge factors are applied to the allowed
        //                shape.
        //     zz         charge of the daughter nucleus
        //     emax       maximum beta energy in mev.
        //     w and tsq   in mc**2 units
        //     e and emax  in mev   units.
        //     v is the screening correction for atomic electrons
        //     for the thomas-fermi model of the atom
        //     v = 1.13 * (alpha)**2  * z**(4/3)
        //
        //

        double pi,c137,zab,v,z,x,w,psq,p,y,qsq,g,cab,f,radf;

        double xl[4];
        complex<double> c;
        complex<double> a;

        bspec=0.0;
        if (e>emax) {
            return;
        }

        pi  =acos(-1.);
        c137=137.036;           // 1/ fine structure constant

        zab = abs(zz);
        v   = 1.13*pow(zab,1.333)/pow(c137,2); // Screening correction
        v   = copysign(v,zz);
        z   = zab/c137;
        x   = sqrt(1.0-z*z);        // s parameter
        w   = 1.0+(e/rm)-v;    // Total energy of b particle
        if (w<1.0000001) {
            bspec = 0.;
            return;
        }
        //if(w<1.00001) w=1.00001;
        psq = w*w-double(1.0);
        p   = sqrt(psq);          // Momemtum of beta particle
        y   = z*w/p;              // eta = alpha * z * e / p
        y   = copysign(y,zz);
        qsq = 3.83*pow(emax-e,2);

        if (e <= 1.0e-5) {
            g=0.0;              // Low energy approximation
            if (zz>=0.0) {
                g = qsq*2.0*pi*pow(z,(2.0*x-1.0));
            }
        }
        else {
            a   = complex<double>(x,y);
            c   = cgamma(a);
            cab = abs(c);
            f   = pow(psq,x-1.0)*exp(pi*y)*pow(cab,2);
            g   = f*p*w*qsq;
        }

        factor  = 1.0; // Necessary to calculate kurie plot (not done)
        bspec   = g;
        if (lam == 0) {
            return;
        }

        radf = 1.2*pow(rmass,0.333); // Nuclear radius
        slfact(p,zz,radf,xl);

        if (lam==1) {
            bspec = g*(qsq*xl[0]+9.0*xl[1]);
            return;
        }
        else if (lam==2) {
            bspec = g*(pow(qsq,2)*xl[0]+30.0*qsq*xl[1]+225.0*xl[2]);
            return;
        }
        else if (lam==3) {
            bspec = g*(pow(qsq,3.0)*xl[0]+63.0*pow(qsq,2)*xl[1]+
                       1575.0*qsq*xl[2] + 11025.0*xl[3]);
            return;
        }
        else { // lam==4

            // Fudge factors for nuclides whose experimental spectra don't
            // seem to fit theory.

            //     for cl36 (ref: nuc. phys. 99a,  625,(67))
            if (zab == 18.0) {
                bspec = bspec*(qsq*xl[0]+20.07*xl[1]);
            }

            //     for i129 (ref: phys. rev. 95, 458, 54))
            if (zab == 54.) {
                bspec = bspec*(psq+10.0*qsq);
            }

            //     for cs-ba137 (ref: nuc. phys. 112a, 156, (68))
            if (zab == 56.) {
                bspec = bspec*(qsq*xl[0]+0.045*xl[1]);
            }

            //     for tl204 (ref: can. j. phys., 45, 2621, (67))
            if (zab == 82.) {
                bspec = bspec*(1.0-1.677*e+ 2.77*e*e);
            }

            //     for bi210 (ref: nuc. phys., 31, 293, (62))
            if (zab == 84.) {
                bspec = bspec*(1.78-2.35*e+e*e);
            }

            return;
        }
    }

    // Sums weighted, normalized spectral components to give total spectrum
    void sp(double e, double &spec, double &factor) {

        double bspec;

        spec=0.0;
        for (int icomp=0; icomp<ncomps; icomp++) {
            zz  = zzz[icomp];
            emax= etop[icomp];
            lam = lamda[icomp];
            bsp(e,bspec,factor);
            spec= spec+bspec*rel[icomp]/area[icomp];
        }
    }

private:
    EGS_Float rm;
    double zz,emax,rmass;
    double zzz[9],etop[9],rel[9],area[9],lamda[9];
    int lam, ncomps;
};

/*! \brief A radionuclide spectrum.

\ingroup egspp_main

Generates spectra for radionuclide emissions. This spectrum type is used by
\ref EGS_RadionuclideSource.
Currently spectrum data is obtained from ENSDF format data files using
\ref EGS_Ensdf. These files can be found in
<code>$HEN_HOUSE/spectra/lnhb/ensdf/</code>.
For more information about the ENSDF format and how it is processed,
see \ref EGS_Ensdf.

It is defined using the following input
\verbatim
:start spectrum:
    type            = radionuclide
    nuclide         = name of the nuclide (e.g. Sr-90), used to look up the
                        ensdf file as $HEN_HOUSE/spectra/lnhb/ensdf/{nuclide}.ensdf
                        if ensdf file not provided below
    ensdf file      = [optional] path to a spectrum file in ensdf format,
                        including extension
    relative activity = [optional] the relative activity (sampling
                        probability) for this nuclide in a mixture
    atomic relaxations = [optional, default=eadl] eadl, ensdf or off
                                 By default, 'eadl' relaxations use the EGSnrc
                                 algorithm for emission correlated with
                                 disintegration events. Alternatively, 'ensdf'
                                 relaxations statistically sample fluorescent
                                 photons and Auger emission using comments
                                 in the ensdf file. Turning this option off
                                 disables all relaxations resulting from
                                 radionuclide disintegration events.
    output beta spectra = [optional, default=no] yes or no
                            whether or not to output beta spectra to files.
                            Files will be named based on the nuclide and
                            maximum energy of the beta decay:
                            {nuclide}_{energy}.spec
    alpha scoring       = [optional, default=none] none or local
                            Whether or not to deposit alpha particles locally.
                            Since alpha particles are not transported in EGSnrc,
                            there are only two options. Either discard the alpha
                            particles and their energy completely, or deposit
                            the energy immediately after creation in the
                            local region.
    extra transition approximation = [optional, default=off] off or on
                            If the intensity away from a level in a radionuclide
                            daughter is larger than the intensity feeding the
                            level (e.g. decays to that level), then additional
                            transitions away from that level will be sampled if
                            this approximation is on.
                            They will not be correlated with decays, but the
                            spectrum will produce emission rates to match both
                            the decay intensities and the internal transition
                            intensities from the ensdf file.
:stop spectrum:
:start spectrum:
    type                = radionuclide
    nuclide             = name of next nuclide in mixture (e.g. Y-90)
    relative activity   = ...
:stop spectrum:
\endverbatim

The source spectrum determines which emission type occurs,
and proceeds to sample the emission parameters. In some cases, no particle
is emitted - this state is signaled by returning zero. The spectrum either
models a disintegration, an internal transition or the emission of
Auger/fluorescence from relaxations.

- Beta- and beta+ disintegrations emit an electron or positron,
respectively. The energies of beta particles are sampled from spectra
generated by \ref EGS_RadionuclideBetaSpectrum. Electron capture events
are also sampled and result in shell vacancies / relaxations, but neutrinos are not modeled. In this case,
\ref EGS_RadionuclideSpectrum returns zero. Disintegrations may set
the energy level of the daughter nuclide to an excited state, leading to the
emission of transition photons, conversion electrons and relaxation emissions
on subsequent calls to <b>\c sample()</b>.

- Alpha disintegrations may also set the energy level of the daughter to an
excited state. Alpha particles themselves are not modeled, so a zero energy
particle is returned. Use the 'alpha scoring = local' option to force alpha particles
to deposit energy in the same region as their creation. By default, no energy
is deposited and the alpha particles are discarded immediately.

- Internal transitions are sampled following a disintegration
if the daughter nuclide is in an excited state. The energy level of the
daughter is then reset according to the internal transition that took place.
Note that there are cases where a transition is not guaranteed.
In such a case, the daughter level is set to zero and zero energy is returned
from <b>\c sample()</b>. There are also cases where multiple transitions occur after
a single disintegration, if the 'extra transition approximation' option is set
to 'on'. Internal transitions may also result in
conversion electrons and relaxation emissions. Internal pair production is currently
modelled in the sampling routine, but no particles are produced.

- Relaxations that result in X-Ray fluorescence and Auger emissions return photons
or electrons until the cascade is complete. Note that X-Ray and Auger emissions
are not disintegrations so are
not counted in the fluence (or the <b>\c ishower </b> parameter). These particles are
assigned the same shower index as the most recent
disintegration and the same time of emission as the most recent disintegration
or internal transition, whichever was last. Relaxations may result in local
energy depositions - this can be obtained using <b> \c getEdep() </b>.

The <b>\c printSampledEmissions()</b> function is provided for evaluating
how the decay emissions actually end up sampled by the source.
The function prints the signature energy and emission intensity of each
emission type for the radionuclide. The intensity is the ratio of the number
of emissions of that type sampled and the total fluence, multiplied by 100. This
provides units comparable to intensities in ensdf files
(intensity per 100 disintegrations).
To add this
output following the simulation, it will be necessary to edit
<b>\c runSimulation()</b> in the application (e.g. in egs_chamber.cpp). At the
end of <b>\c runSimulation()</b>, add a line such as
<code>source->printSampledEmissions();</code>.

 */
class EGS_EXPORT EGS_RadionuclideSpectrum {

public:
    /*! \brief Construct a radionuclide spectrum.
     */
    EGS_RadionuclideSpectrum(const string nuclide, const string ensdf_file,
                             const EGS_Float relativeActivity, const string relaxType, const string outputBetaSpectra, const bool scoreAlphasLocally, const bool allowMultiTransition);

    /*! \brief Destructor. */
    ~EGS_RadionuclideSpectrum() {
        if (decays) {
            delete decays;
        }
        if (betaSpectra) {
            delete betaSpectra;
        }
    };

    /*! \brief Returns the maximum energy that may be emitted.
     *
     * Returns the maximum energy expected from the radionuclide. If correlated
     * relaxations are turned on (e.g. eadl relaxations), note that they are not
     * considered in this. Relaxation emissions from the ENSDF file are
     * considered.
     */
    EGS_Float maxEnergy() const {
        return Emax;
    };

    /*! \brief Get the charge of the most recent emission. */
    int getCharge() const {
        return currentQ;
    }

    /*! \brief Get the emission time of the most recent emission. */
    double getTime() const {
        return currentTime;
    }

    /*! \brief Get the shower index of the most recent emission. */
    EGS_I64 getShowerIndex() const {
        return ishower;
    }

    /*! \brief Get energy that should be deposited locally from relaxations/alphas. */
    EGS_Float getEdep() const {
        return edep;
    }

    /*! \brief Get the relative weight assigned to this spectrum. */
    EGS_Float getSpectrumWeight() const {
        return spectrumWeight;
    }

    /*! \brief Set the relative weight assigned to this spectrum. */
    void setSpectrumWeight(EGS_Float newWeight) {
        spectrumWeight = newWeight;
    }

    /*! \brief Get the emission type of the most recent source particle.
     *
     * 0: No event occurred
     * 1: Atomic relaxation particle (correlated with decay)
     * 2: Internal transition gamma
     * 3: Conversion electron
     * 4: Electron capture
     * 5: Beta+ decay
     * 6: Beta- decay
     * 7: Alpha decay
     * 8: Metastable decay
     * 9: X-Ray (uncorrelated, from ENSDF)
     * 10: Auger electron (uncorrelated, from ENSDF)
     * 11: Uncorrelated internal transition gamma
     * 12: Uncorrelated conversion electron
     * 13: Internal pair production
     * 14: Uncorrelated internal pair production
     */
    unsigned int getEmissionType() const {
        return emissionType;
    }

    /*! \brief Print the sampled emission intensities.
     */
    void printSampledEmissions();

    EGS_Ensdf* getRadionuclideEnsdf() {
        return decays;
    }

    bool storeState(ostream &data) const {
        return egsStoreI64(data,ishower);
    }

    bool setState(istream &data) {
        return egsGetI64(data,ishower);
    }

    void resetCounter() {
        currentLevel = 0;
        currentTime = 0;
        ishower = -1;
        totalGammaEnergy = 0;
    }

    /*! \brief Sample an event from the spectrum, returns the energy of the emitted particle. */
    EGS_Float sample(EGS_RandomGenerator *rndm);

private:

    EGS_Ensdf                   *decays;
    vector<BetaRecordLeaf *>    myBetas;
    vector<AlphaRecord *>       myAlphas;
    vector<GammaRecord *>       myGammas,
           myMetastableGammas,
           myUncorrelatedGammas;
    vector<LevelRecord *>       myLevels;
    vector<double>              xrayIntensities,
           xrayEnergies,
           augerIntensities,
           augerEnergies;
    vector<EGS_I64>             numSampledXRay,
           numSampledAuger;
    vector<const LevelRecord *> multiTransitions;
    EGS_SimpleContainer<EGS_RelaxationParticle> relaxParticles;
    const LevelRecord           *currentLevel;
    int                         currentQ;
    unsigned int                emissionType;
    EGS_Float                   currentTime,
                                Emax,
                                spectrumWeight,
                                totalGammaEnergy,
                                edep;
    EGS_I64                     ishower;
    string                      relaxationType;
    bool                        scoreAlphasLocal;

    EGS_RadionuclideBetaSpectrum *betaSpectra;
    EGS_Application             *app;
};

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

    EGS_RadionuclideSpectrum* createSpectrum(EGS_Input *input);

    vector<EGS_Ensdf*> getRadionuclideEnsdf() {
        vector<EGS_Ensdf*> decayEnsdf;
        for(auto dec: decays) {
            decayEnsdf.push_back(dec->getRadionuclideEnsdf());
        }
        return decayEnsdf;
    }

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
