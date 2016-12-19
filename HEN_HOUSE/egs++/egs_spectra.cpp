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
#                   Reid Townson
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

#include <complex>
#include <limits>


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


/*! \brief Beta spectrum generation for \link EGS_RadionuclideSpectrum
 * radionuclide spectra \endlink
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
    EGS_RadionuclideBetaSpectrum(EGS_Ensdf *decays) {

        vector<BetaRecordLeaf *> myBetas = decays->getBetaRecords();

        for (vector<BetaRecordLeaf *>::iterator beta = myBetas.begin();
                beta != myBetas.end(); beta++) {

            // Skip electron capture records since they don't emit a particle
            if ((*beta)->getCharge() == 1 &&
                    (*beta)->getPositronIntensity() == 0) {
                continue;
            }

            egsInformation("EGS_RadionuclideBetaSpectrum: "
                           "Energy, Z, A, forbidden: %f %d %d %d\n",
                           (*beta)->getFinalEnergy(), (*beta)->getZ(),
                           (*beta)->getAtomicWeight(), (*beta)->getForbidden()
                          );

            const int nbin=1000;
            EGS_Float *e = new EGS_Float [nbin];
            EGS_Float *spec = new EGS_Float [nbin];
            EGS_Float *spec_y = new EGS_Float [nbin];
            EGS_Float *spec_sr = new EGS_Float [nbin];

            double de, s_y, s_sr, factor, e1, e2, se_y, se_sr;
            int isrc;

            ncomps=1; // if we increase this, then we must fill the remainder
            area[0]=1.0;
            rel[0]=1.0;

            emax = (*beta)->getFinalEnergy();
            zzz[0] = (double)(*beta)->getZ();
            rmass = (*beta)->getAtomicWeight();
            lamda[0] = (*beta)->getForbidden();

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

            e1=0.001;         // may be too low for some spectra (e.g. Tl-204)
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
            // This is just intended to be temporary to test the
            // spectrum generation code
            ostringstream ostr;
            ostr << decays->radionuclide << "_d_" << emax << ".spec";

            ofstream specStream;
            specStream.open(ostr.str().c_str());
            for (int ib=0; ib<nbin; ib++) {
                spec[ib]=1/de*(spec_y[ib]/s_y);
                specStream << e[ib] << " " << spec[ib] << endl;
            }
            specStream.close();
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
        double pi,c137,az,g1,w,rad,pr,y,x1,gk,bb,cc,dd,x2;

        complex<double> aa;

        pi  = acos(-1.0);
        c137= 137.036; // 1/ fine structure constant
        az  = z/c137;
        g1  = sqrt(1.0-az*az);
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
        //     v is the screening correction for atomi// electrons
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
        w   = 1.0+(e/0.51097)-v;    // Total energy of b particle
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

            //TODO: Currently no isotopes will get here

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
    double zz,emax,rmass;
    double zzz[9],etop[9],rel[9],area[9],lamda[9];
    int lam, ncomps;
};


/*! \brief A radionuclide spectrum.

\ingroup egspp_main

Generates spectra for radionuclide emissions. This spectrum type is used by
\ref EGS_RadionuclideSource.
Currently spectrum data is obtained from ENSDF format data files.
These files can be found in $HEN_HOUSE/spectra/lnhb/.
The decay structure is created by \ref EGS_Ensdf.

It is defined using the following input
\verbatim
:start spectrum:
    type            = radionuclide
    isotope         = name of the isotope (e.g. Sr-90), used to look up the
                        ensdf file as $HEN_HOUSE/spectra/lnhb/{isotope}.ensdf
                        if ensdf file not provided below
    ensdf file      = [optional] path to a spectrum file in ensdf format,
                        including extension
    relative activity = [optional] the relative activity (sampling
                        probability) for this isotope in a mixture
    use fluorescence and auger = [optional, default=yes] yes or no
                                 whether or not to model the x-ray and Auger
                                 emissions provided in the ensdf file
:stop spectrum:
:start spectrum:
    type                = radionuclide
    isotope             = name of next isotope in mixture (e.g. Y-90)
    relative activity   = ...
:stop spectrum:
\endverbatim

 */
class EGS_EXPORT EGS_RadionuclideSpectrum : public EGS_BaseSpectrum {

public:

    /*! \brief Construct a radionuclide spectrum.
     */
    EGS_RadionuclideSpectrum(const string isotope, const string ensdf_file,
                             const EGS_Float relativeActivity, const string useFluorescence) :
        EGS_BaseSpectrum() {

        // For now, hard-code verbose mode
        // 0 - minimal output
        // 1 - some output of ensdf data and normalized intensities
        // 2 - verbose output
        int verbose = 1;

        // Read in the data file for the isotope
        // and build the decay structure
        decays = new EGS_Ensdf(isotope, ensdf_file, useFluorescence, verbose);

        // Normalize the emission and transition intensities
        decays->normalizeIntensities();

        // Get the beta energy spectra
        betaSpectra = new EGS_RadionuclideBetaSpectrum(decays);

        // Get the particle records from the decay scheme
        myBetas = decays->getBetaRecords();
        myAlphas = decays->getAlphaRecords();
        myGammas = decays->getGammaRecords();
        myMetastableGammas = decays->getMetastableGammaRecords();
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
        totalGammaEnergy = 0;

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
        for (vector<GammaRecord *>::iterator gamma = myMetastableGammas.begin();
                gamma != myMetastableGammas.end(); gamma++) {

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
        spectrumWeight = relativeActivity;

        if (verbose) {
            egsInformation("EGS_RadionuclideSpectrum: Emax: %f\n",Emax);
            egsInformation("EGS_RadionuclideSpectrum: Relative activity: %f\n",relativeActivity);
        }
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

    EGS_I64 getShowerIndex() const {
        return ishower;
    }

    EGS_Float getSpectrumWeight() const {
        return spectrumWeight;
    }

    void setSpectrumWeight(EGS_Float newWeight) {
        spectrumWeight = newWeight;
    }

    /*! \brief Print the sampled emission intensities.
     */
    void printSampledEmissions() {

        egsInformation("\nSampled %s emissions:\n", decays->radionuclide.c_str());
        egsInformation("========================\n");

        if (ishower < 1) {
            egsWarning("EGS_RadionuclideSpectrum::printSampledEmissions: Warning: The number of disintegrations (tracked by `ishower`) is less than 1.\n");
            return;
        }

        egsInformation("Energy | Intensity per 100 emissions\n");
        if (myBetas.size() > 0) {
            egsInformation("Beta records:\n");
        }
        for (vector<BetaRecordLeaf *>::iterator beta = myBetas.begin();
                beta != myBetas.end(); beta++) {

            egsInformation("%f %f\n", (*beta)->getFinalEnergy(),
                           ((EGS_Float)(*beta)->getNumSampled()/ishower)*100);
        }
        if (myAlphas.size() > 0) {
            egsInformation("Alpha records:\n");
        }
        for (vector<AlphaRecord *>::iterator alpha = myAlphas.begin();
                alpha != myAlphas.end(); alpha++) {

            egsInformation("%f %f\n", (*alpha)->getFinalEnergy(),
                           ((EGS_Float)(*alpha)->getNumSampled()/ishower)*100);
        }
        if (myGammas.size() > 0) {
            egsInformation("Gamma records:\n");
        }
        EGS_I64 totalNumSampled = 0;
        for (vector<GammaRecord *>::iterator gamma = myGammas.begin();
                gamma != myGammas.end(); gamma++) {

            totalNumSampled += (*gamma)->getNumSampled();
            egsInformation("%f %f\n", (*gamma)->getDecayEnergy(),
                           ((EGS_Float)(*gamma)->getNumSampled()/ishower)*100);
        }
        if (myGammas.size() > 0) {
            if (totalNumSampled > 0) {
                egsInformation("Average gamma energy: %f\n",
                               totalGammaEnergy / totalNumSampled);
            }
            else {
                egsInformation("Zero gamma transitions occurred.\n");
            }
        }
        if (myMetastableGammas.size() > 0) {
            egsInformation("Metastable Gamma records:\n");
        }
        totalNumSampled = 0;
        for (vector<GammaRecord *>::iterator gamma = myMetastableGammas.begin();
                gamma != myMetastableGammas.end(); gamma++) {

            totalNumSampled += (*gamma)->getNumSampled();
            egsInformation("%f %f\n", (*gamma)->getDecayEnergy(),
                           ((EGS_Float)(*gamma)->getNumSampled()/ishower)*100);
        }
        if (xrayEnergies.size() > 0) {
            egsInformation("X-Ray records:\n");
        }
        for (unsigned int i=0; i < xrayEnergies.size(); ++i) {
            egsInformation("%f %f\n", xrayEnergies[i],
                           ((EGS_Float)numSampledXRay[i]/ishower)*100);
        }
        if (augerEnergies.size() > 0) {
            egsInformation("Auger records:\n");
        }
        for (unsigned int i=0; i < augerEnergies.size(); ++i) {
            egsInformation("%f %f\n", augerEnergies[i],
                           ((EGS_Float)numSampledAuger[i]/ishower)*100);
        }
        egsInformation("\n");
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

protected:
    /*! \brief Sample an event from the spectrum.

    Returns the energy of the emitted particle.

    Determines which emission type occurs, and proceeds to sample the
    emission parameters. If the daughter isotope is
    in an excited state, a transition gamma is emitted and
    the daughter transitions to a lower energy state. If the daughter
    isotope is not in an exited state, one of the following events occurs:

    Disintegration events may set the energy level of the daughter to an
    excited state. Beta- and beta+ disintegrations emit an electron or positron,
    respectively. The energies of beta particles are sampled from spectra
    generated by \ref EGS_RadionuclideBetaSpectrum. Electron capture events
    are also sampled, but neutrinos are not modeled so a zero energy
    particle is returned.

    Alpha disintegrations may set the energy level of the daughter to an
    excited state, leading to the emission of transition photons.
    Alpha particles themselves are not modeled, so a zero energy particle
    is returned.

    X-Ray and Auger events emit a photon or electron from a sampled energy line.
    X-Ray and Auger emissions are not disintegrations so are not counted in
    the fluence (or the ishower parameter).

    Transition gammas are sampled and emitted following a disintegration (if
    the daughter is created in an excited state). The energy level of the
    daughter is then set according to the transition that took place.
    Note that there are cases where a transition photon is not guaranteed.
    In such a case, the daughter level is set to zero and a zero energy
    particle is returned.

    Note about emission times: The time of emission of
    a transition photon is determined by sampling the delay that occurs after
    disintegration (according to the transition half life).
    The time of disintegration is calculated similarly based on the
    total activity of the mixture in \ref EGS_RadionuclideSource.
    X-Rays and Auger electrons are assumed to occur at the same time
    as the disintegration and will have both time and ishower corresponding
    to the most recent disintegration.

    Currently, it is possible for an X-Ray or Auger to be emitted before a
    disintegration has taken place. They are assigned currentTime=0 and
    ishower=-1.

     */
    EGS_Float sample(EGS_RandomGenerator *rndm) {

        // Sample a uniform random number
        EGS_Float u = rndm->getUniform();

        // The energy of the sampled particle
        EGS_Float E;

        // If the daughter is in an excited state
        // check for transitions
        if (currentLevel && currentLevel->levelCanDecay() && currentLevel->getEnergy() > 1e-10) {

            for (vector<GammaRecord *>::iterator gamma = myGammas.begin();
                    gamma != myGammas.end(); gamma++) {

                if ((*gamma)->getLevelRecord() == currentLevel) {

                    if (u < (*gamma)->getTransitionIntensity()) {

                        // Update the current time by sampling how long
                        // it took for this transition to occur
                        // time += halflife / ln(2) * log(u)
                        double hl = currentLevel->getHalfLife();
                        if (hl > 0) {
                            currentTime = -hl * log(1.-rndm->getUniform()) /
                                          0.693147180559945309417232121458176568075500134360255254120680009493393;
                        }

                        (*gamma)->incrNumSampled();
                        currentQ = (*gamma)->getCharge();

                        currentLevel = (*gamma)->getFinalLevel();

                        E = (*gamma)->getDecayEnergy();

                        totalGammaEnergy += E;

                        return E;
                    }
                }
            }
            currentLevel = 0;
            return 0;
        }

        // ============================
        // Sample which decay occurs
        // ============================
        currentTime = 0;

        // Beta-, beta+ and electron capture
        for (vector<BetaRecordLeaf *>::iterator beta = myBetas.begin();
                beta != myBetas.end(); beta++) {
            if (u < (*beta)->getBetaIntensity()) {

                // Increment the shower number
                ishower++;

                // Increment the counter of betas and get the charge
                (*beta)->incrNumSampled();
                currentQ = (*beta)->getCharge();

                // Set the energy level of the daughter
                currentLevel = (*beta)->getLevelRecord();

                // For beta+ records we decide between
                // branches for beta+ or electron capture
                if (currentQ == 1) {
                    // For positron emission, continue as usual
                    if ((*beta)->getPositronIntensity() > 1e-10 && rndm->getUniform() < (*beta)->getPositronIntensity()) {

                    }
                    else {
                        // For electron capture, there is no emitted particle
                        // (only a neutrino)
                        // so we return a 0 energy particle
                        return 0;
                    }
                }

                // Sample the energy from the spectrum alias table
                E = (*beta)->getSpectrum()->sample(rndm);

                return E;
            }
        }

        // Alphas
        for (vector<AlphaRecord *>::iterator alpha = myAlphas.begin();
                alpha != myAlphas.end(); alpha++) {
            if (u < (*alpha)->getAlphaIntensity()) {

                // Increment the shower number
                ishower++;

                // Increment the counter of alphas and get the charge
                (*alpha)->incrNumSampled();
                currentQ = (*alpha)->getCharge();

                // Set the energy level of the daughter
                currentLevel = (*alpha)->getLevelRecord();

                // For alphas we simulate a disintegration but the
                // transport will not be performed
                return 0;
            }
        }

        // Metastable internal transition gammas
        for (vector<GammaRecord *>::iterator gamma = myMetastableGammas.begin();
                gamma != myMetastableGammas.end(); gamma++) {
            if (u < (*gamma)->getTransitionIntensity()) {

                // Increment the shower number
                // Yes, these internal transition gammas are counted as
                // disintegrations! This is due to the metastable state.
                ishower++;

                // Increment the counter of gammas and get the charge
                (*gamma)->incrNumSampled();
                currentQ = (*gamma)->getCharge();

                // Notice that we do NOT set the energy level of the daughter
                // Instead, all of these are treated as uncorrelated

                // Get the gamma internal transition energy
                E = (*gamma)->getDecayEnergy();

                return E;
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

        // Shouldn't get here if intensities are normalized correctly
        return 0;
    };

    /*! \brief Returns the maximum energy that may be emitted.
     */
    EGS_Float maxEnergy() const {
        return Emax;
    };

    /*! \brief Not implemented - returns 0.
     */
    EGS_Float expectedAverage() const {
        return 0;
    };

private:

    EGS_Ensdf                   *decays;
    vector<BetaRecordLeaf *>    myBetas;
    vector<AlphaRecord *>       myAlphas;
    vector<GammaRecord *>       myGammas;
    vector<GammaRecord *>       myMetastableGammas;
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
                                spectrumWeight,
                                totalGammaEnergy;
    EGS_I64                     ishower;

    EGS_RadionuclideBetaSpectrum *betaSpectra;
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

        EGS_Float relativeActivity;
        err = inp->getInput("relative activity",relativeActivity);
        if (err) {
            relativeActivity = 1;
        }

        // Determine whether to sample X-Rays and Auger electrons
        // using the ensdf data (options: yes or no)
        string tmp_useFl, useFluorescence;
        err = inp->getInput("use fluorescence and auger", tmp_useFl);
        if (!err) {
            useFluorescence = tmp_useFl;
        }
        else {
            useFluorescence = "yes";
        }
        if (useFluorescence == "yes") {
            egsInformation("EGS_BaseSpectrum::createSpectrum: Fluorescence and auger from the ensdf file will be used.\n");
        }
        else {
            egsInformation("EGS_BaseSpectrum::createSpectrum: Fluorescence and auger from the ensdf file will be ignored.\n");
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

        // Check that the ensdf file exists
        ifstream ensdf_fh;
        ensdf_fh.open(ensdf_file.c_str(),ios::in);
        if (!ensdf_fh.is_open()) {
            egsWarning("EGS_BaseSpectrum::createSpectrum: failed to open ensdf file %s"
                       " for reading\n",ensdf_file.c_str());
            return 0;
        }
        ensdf_fh.close();

        // Create the spectrum
        spec = new EGS_RadionuclideSpectrum(isotope, ensdf_file, relativeActivity, useFluorescence);
    }
    else {
        egsWarning("%s unknown spectrum type %s\n",spec_msg1,stype.c_str());
    }
    if (delete_it) {
        delete inp;
    }
    return spec;
}
