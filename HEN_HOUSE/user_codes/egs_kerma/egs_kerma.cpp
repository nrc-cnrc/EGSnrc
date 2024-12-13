/*
###############################################################################
#
#  EGSnrc egs++ egs_kerma application
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
#  Author:        Ernesto Mainegra-Hing, 2016
#
#  Contributors:  Reid Townson
#
###############################################################################
#
#   C++ user code for estimating the quantity kerma in a scoring volume. Kerma in
#   each scoring volume region, as well as total and differential photon fluence
#   can be requested in the scoring options input block.
#
#   Two calculation options are available:
#
#  - If a forced-detection (FD) geometry provided, a ray-tracing towards and across
#    the FD geometry combined with an exponential track-length (eTL) scoring technique
#    is used to score kerma and fluence for photons reaching the scoring volume
#    if they haven't been in any of the exclusion regions. Photons interacting inside
#    the scoring regions are included.
#
#  - If no geometry provided, a linear track-length (TL) scoring 'a la FLURZnrc' is
#    used.
#
#  Required: E*muen or E*mutr file for scoring either collision or total kerma
#  --------  for the scoring medium (unique).
#
#  Calculations for multiple geometries
#
#  Kerma ratios can be calculated using a correlated scoring technique
#
#  Exclusion of user specified regions
#
#  Scoring volume regions must be provided (perhaps use labels?)
#
#  Dose calculation in the scoring volume can be done using a dose scoring AO.
#  This could be useful to check the validity of the kerma-approximation.
#
#  NOTE 1 : Dose calculation with very high ECUT produces an estimate of
            total kerma, not collision kerma.
#  NOTE 2 : dose scoring ONLY makes sense for ONE calculation geometry.
#           If more than one geometry defined, dose scoring should NOT
#           be used as it would score dose for all geometries in one
#           scoring array.
#
###############################################################################
*/

#include <cstdlib>
// We derive from EGS_AdvancedApplication => need the header file.
#include "egs_advanced_application.h"
// We use scoring objects provided by egspp => need the header file.
#include "egs_scoring.h"
// Every C++ EGSnrc application needs this header file
#include "egs_interface2.h"
// We use egsInformation() => need the egs_functions.h header file.
#include "egs_functions.h"
// We use the EGS_Input class
#include "egs_input.h"
// To get the maximum source energy
#include "egs_base_source.h"
// The random number generator
#include "egs_rndm.h"
// Transformations
#include "egs_transformations.h"
// Interpolators
#include "egs_interpolator.h"

#include <fstream>
#include <iostream>

#define calculatePhotonMFP F77_OBJ_(calculate_photon_mfp,CALCULATE_PHOTON_MFP)
extern __extc__ void calculatePhotonMFP(EGS_Float *,EGS_Float *);
#define doRayleigh F77_OBJ_(do_rayleigh,DO_RAYLEIGH)
extern __extc__ void doRayleigh();
#define calculatePhotonBranching F77_OBJ_(calculate_photon_branching,CALCULATE_PHOTON_BRANCHING)
extern __extc__ void calculatePhotonBranching(EGS_Float *gbr1,EGS_Float *gbr2);
extern __extc__ void F77_OBJ(pair,PAIR)();
extern __extc__ void F77_OBJ(compt,COMPT)();
extern __extc__ void F77_OBJ(photo,PHOTO)();

#define INT_MAX 2147483647
#define TSTEP_MAX 1e35
//const EGS_Float kermaEpsilon = 1.0/(1ULL<<50);
const EGS_Float kermaEpsilon = 4.0*2.225E-308;// About 4 times the Min. normal positive double

class APP_EXPORT EGS_KermaApplication : public EGS_AdvancedApplication {

public:

    /*! Constructor */
    EGS_KermaApplication(int argc, char **argv) :
        EGS_AdvancedApplication(argc,argv), ngeom(0),
        kerma(0), kerma_r(0), scg(0), fd_geom(0),
        ncg(0), flug(0),flugT(0) {
        Eph_ave = 0.0;
        Nph = 0.0;
        Eph_sc  = 0.0;
        Nsc = 0.0;
        Eel_ave = 0.0, Nel = 0.0;
    };

    /*! Destructor.  */
    ~EGS_KermaApplication() {
        if (ngeom > 0) {
            if (kerma_r) {
                for (int j=0; j<ngeom; j++) if (kerma_r[j]) {
                        delete kerma_r[j];
                    }
                if (kerma_r) {
                    delete [] kerma_r;
                }
                if (kerma) {
                    delete kerma;
                }
            }
            if (flug) {
                for (int j=0; j<ngeom; j++) if (flug[j]) {
                        delete flug[j];
                    }
                if (flug) {
                    delete [] flug;
                }
                if (flugT) {
                    delete [] flugT;
                }
            }
            delete [] geoms;
            delete [] fd_geoms;
            delete [] mass;
            delete [] active_med;
            int j;
            for (j=0; j<ngeom; j++) if (transforms[j]) {
                    delete transforms[j];
                }
            if (transforms) {
                delete [] transforms;
            }
            for (j=0; j<ngeom; j++) {
                delete [] is_sensitive[j];
            }
            delete [] is_sensitive;
            for (j=0; j<ngeom; j++) {
                delete [] is_excluded[j];
            }
            delete [] is_excluded;
        }
        if (ncg > 0) {
            delete [] gind1;
            delete [] gind2;
            delete [] scg;
        }
    };

    /*! Describe the application.  */
    void describeUserCode() const {
        egsInformation(
            "\n               *************************************************"
            "\n               *                                               *"
            "\n               *                  egs_kerma                    *"
            "\n               *                                               *"
            "\n               *************************************************"
            "\n\n");
        egsInformation("This is EGS_KermaApplication %s based on"
                       " EGS_AdvancedApplication %s\n\n",
                       egsSimplifyCVSKey(revision).c_str(),
                       egsSimplifyCVSKey(base_revision).c_str());

    };

    /*! Describe the simulation */
    void describeSimulation();

    /*! Initialize scoring.  */
    int initScoring();

    /*! Accumulate quantities of interest at run time */
    int ausgab(int iarg) {
        int np = the_stack->np-1, ir = the_stack->ir[np]-2, iq = the_stack->iq[np];
        /* Photon about to be transported in geometry */
        if (iarg == BeforeTransport &&  !iq && ir >= 0) {
            /* Track-Length Kerma scoring (classic) */
            if (is_sensitive[ig][ir]) {
                if (!fd_geom) {
                    EGS_Float E = the_stack->E[np], gle = log(E),
                              emuen   = E_Muen_Rho->interpolateFast(gle)*rho_cv[ig],
                              wtstep  = the_stack->wt[np]*the_epcont->tvstep;
                    kerma->score(ig,wtstep*emuen);
                    if (kerma_r[ig]) {
                        kerma_r[ig]->score(ir,wtstep*emuen);
                    }
                    // Track energy of scoring photons
                    Eph_sc += the_stack->wt[np]*E;
                    Nsc    += the_stack->wt[np];
                    //--------------------------------------------
                    // score photon fluence
                    //--------------------------------------------
                    if (flug) {
                        EGS_Float e = E;
                        if (flu_s) {
                            e = log(e);
                        }
                        EGS_Float ae;
                        int je;
                        if (e > flu_xmin && e <= flu_xmax) {
                            ae = flu_a*e + flu_b;
                            je = min((int)ae,flu_nbin-1);//je = (int) ae;
                            EGS_ScoringArray *aux = flug[ig];
                            aux->score(je,wtstep);
                            flugT[ig]->score(ir,wtstep);
                        }
                    }
                    else {
                        if (flugT) {
                            flugT[ig]->score(ir,wtstep);
                        }

                    }
                }
            }
            /* Mark photons in exclusion regions */
            else if (is_excluded[ig][ir]) {
                the_epcont->idisc = -1;// i.e. discard after the step.
                the_stack->wt[np] = 0; // do not contribute energy deposition
            }
        }
        return 0;
    }

    /*! Simulate a single shower.
        We need to do special things and therefore reimplement this method.
     */
    int simulateSingleShower() {
        last_case = current_case;
        EGS_Vector x,u;

        current_case = source->getNextParticle(rndm,p.q,p.latch,p.E,p.wt,x,u);

        if (p.q == 0) {
            Eph_ave += p.wt*p.E;
            Nph += p.wt;
        }
        else {
            Eel_ave += p.wt*p.E;
            Nel     += p.wt;
        }

        int err = startNewShower();
        if (err) {
            return err;
        }

        EGS_BaseGeometry *save_geometry = geometry;
        for (ig=0; ig<ngeom; ig++) {
            geometry = geoms[ig];
            if (fd_geoms[ig]) {
                fd_geom = fd_geoms[ig];
            }
            else {
                fd_geom = 0;
            }
            p.x = x;
            p.u = u;
            if (transforms[ig]) {
                transforms[ig]->transform(p.x);
                transforms[ig]->rotate(p.u);
            }
            int ireg = geometry->isWhere(p.x);
            if (ireg < 0) {
                EGS_Float t = 1e30;
                ireg = geometry->howfar(ireg,p.x,p.u,t);
                if (ireg >= 0) {
                    p.x += p.u*t;
                }
            }
            if (ireg >= 0) {
                p.ir = ireg;
                err = shower();
                if (err) {
                    return err;
                }
            }
        }
        err = finishShower();
        geometry = save_geometry;
        return err;
    };

    /*! Score Air-Kerma in Collecting Volume (CV)

        Ray-trace photon across geometry keeping track of path to scoring
        volume (CV). Once inside CV, determine its path across the CV and
        once it exits the CV, proceed to score kerma and fluence.

        If the CV is such that multiple entries are possible, e.g., photons
        entering or backscattering into hollow geometries, for instance a
        spherical or cylindrical shell, this method continues the
        ray-tracing to the next re-entry.

        Photons touching excluded region are discarded immediately.

        TODO: Use region labels to define sensitive regions

     */
    int scoreInCV() {

        int np = the_stack->np-1;

        if (the_stack->E[np] < the_bounds->pcut) {
            return 0;
        }

        // Track energy of scoring photons
        Eph_sc += the_stack->E[np]*the_stack->wt[np];
        Nsc    += the_stack->wt[np];

        EGS_Vector x(the_stack->x[np],the_stack->y[np],the_stack->z[np]);
        EGS_Vector u(the_stack->u[np],the_stack->v[np],the_stack->w[np]);

        int ireg = the_stack->ir[np]-2, newmed = geometry->medium(ireg);

        EGS_Float tstep;
        int inew;
        int imed = -1;
        EGS_Float gmfp, sigma = 0, cohfac = 1, mu_cv = 0;
        EGS_Float gle = the_epcont->gle, wt_att = 1, Lambda_to_CV = 0;
        double Lambda = 0, t_sc_tot = 0;
        double t_sc[2*n_scoring_r[ig]];
        int   ir_sc[2*n_scoring_r[ig]];
        int n_ir_sc = 0;
        bool inside_cv  = false, re_enters_cv = false, navigating = true;
        /* Ray-trace from current position to CV and keep track of path to
         * CV and path in the CV
         */
        while (navigating) {
            while (1) {
                if (is_excluded[ig][ireg]) {
                    break;
                }
                if (imed != newmed) {
                    imed = newmed;
                    if (imed >= 0) {
                        gmfp = i_gmfp[imed].interpolateFast(gle);
                        if (the_xoptions->iraylr) {
                            cohfac = i_cohe[imed].interpolateFast(gle);
                            gmfp *= cohfac;
                        }
                        sigma = 1/gmfp;
                    }
                    else {
                        sigma = 0;
                        cohfac = 1;
                    }
                }
                tstep = TSTEP_MAX;
                inew = geometry->howfar(ireg,x,u,tstep,&newmed);

                Lambda += tstep*sigma;// keep track of path outside scoring volume

                if (is_sensitive[ig][ireg]) {   //in cavity, get path through it
                    ir_sc[n_ir_sc]  = ireg;
                    t_sc[n_ir_sc]   = tstep;
                    t_sc_tot       += tstep;
                    n_ir_sc++;
                    if (!inside_cv) {
                        Lambda_to_CV = Lambda - tstep*sigma;
                        inside_cv = true;
                        mu_cv = sigma;
                    }
                    Lambda = Lambda - tstep*sigma;
                }

                if (inew < 0) {
                    break;    // outside geometry, stop and score
                }

                ireg = inew;
                x += u*tstep;

                // Leaves CV?
                if (inside_cv && !is_sensitive[ig][ireg]) {
                    // Does it re-enter CV?
                    tstep = TSTEP_MAX;
                    if (fd_geom->isInside(x) ||
                            fd_geom->howfar(-1,x,u,tstep,&newmed)>= 0) {
                        re_enters_cv = true;
                    }
                    break;
                }
            }
            if (inside_cv) {
                EGS_Float wt = the_stack->wt[np]*wt_att;
                EGS_Float emuen_rho  = E_Muen_Rho->interpolateFast(gle);
                EGS_Float exp_Lambda_to_CV = exp(-Lambda_to_CV),
                          exp_Lambda = exp_Lambda_to_CV,
                          exp_CV     = 1.0,
                          exp_Att, edepCV;
                for (int i = 0; i < n_ir_sc; i++) {
                    edepCV     = emuen_rho*rho_cv[ig];// Data base contains E_muen/rho values
                    exp_CV     = exp(-mu_cv*t_sc[i]);
                    exp_Att    = sigma ? exp_Lambda*(1-exp_CV)/mu_cv : 1.0 ;//Attenuation in scoring region
                    edepCV    *= exp_Att;
                    //--------------------------------------------
                    // score kerma in scoring region
                    //--------------------------------------------
                    if (kerma_r[ig]) {
                        kerma_r[ig]->score(ir_sc[i],wt*edepCV);
                    }
                    //--------------------------------------------
                    // score photon fluence
                    //--------------------------------------------
                    if (flug) {
                        flugT[ig]->score(ir_sc[i],wt*exp_Att);
                        EGS_Float e = the_stack->E[np];
                        if (flu_s) {
                            e = log(e);
                        }
                        EGS_Float ae;
                        int je;
                        if (e > flu_xmin && e <= flu_xmax) {
                            ae = flu_a*e + flu_b;
                            je = min((int)ae,flu_nbin-1);
                            EGS_ScoringArray *aux = flug[ig];
                            aux->score(je,wt*exp_Att);
                        }
                    }
                    else {
                        if (flugT) {
                            flugT[ig]->score(ir_sc[i],wt*exp_Att);
                        }

                    }

                    // Include as attenuation to next scoring region
                    exp_Lambda *= exp_CV;
                }
                //--------------------------------------------
                // score total kerma and fluence in CV
                //--------------------------------------------
                edepCV     = emuen_rho*rho_cv[ig];// Data base contains E_muen/rho values
                exp_CV     = exp(-mu_cv*t_sc_tot);
                exp_Att    = sigma ? exp_Lambda_to_CV*(1-exp_CV)/mu_cv : 1.0;
                edepCV    *= exp_Att;
                kerma->score(ig,wt*edepCV);
                // Ray-tracing continues
                if (re_enters_cv) {
                    wt_att *= exp_Lambda;
                    Lambda_to_CV = 0;
                    Lambda = 0;
                    t_sc_tot = 0;
                    n_ir_sc = 0;
                    inside_cv = false;
                }
                else {
                    navigating = false;
                }
            }
            else {
                navigating = false;
            }
        }
        //==============================================================
        return 0;
    }

    /*! Output intermediate results to the .egsdat file. */
    int outputData() {
        int err = EGS_AdvancedApplication::outputData();
        if (err) {
            return err;
        }
        if (!kerma->storeState(*data_out)) {
            return 101;
        }
        for (int j=0; j<ngeom; j++) {
            if (kerma_r[j] && !kerma_r[j]->storeState(*data_out)) {
                return 108+2*j;
            }
        }
        if (ncg > 0) {
            for (int j=0; j<ncg; j++) {
                double aux = kerma->thisHistoryScore(gind1[j])*
                             kerma->thisHistoryScore(gind2[j]);
                (*data_out) << scg[j]+aux << "  ";
            }
            (*data_out) << endl;
            if (!data_out->good()) {
                return 104;
            }
        }
        if (flug) {
            for (int j=0; j<ngeom; j++) {
                if (!flug[j]->storeState(*data_out)) {
                    return 108+2*(ngeom+j);
                }
                if (!flugT[j]->storeState(*data_out)) {
                    return 109+4*(ngeom+j);
                }
            }
        }

        (*data_out) << Eph_ave << " " << Nph << " "
                    << Eel_ave << " " << Nel << " "
                    << Eph_sc  << " " << Nsc << endl;
        if (!data_out->good()) {
            return 1031;
        }

        data_out->flush();
        delete data_out;
        data_out = 0;
        return 0;
    };

    /*! Read results from a .egsdat file. */
    int readData() {
        int err = EGS_AdvancedApplication::readData();
        if (err) {
            return err;
        }
        if (!kerma->setState(*data_in)) {
            return 101;
        }
        for (int j=0; j<ngeom; j++) {
            if (kerma_r[j] && !kerma_r[j]->setState(*data_in)) {
                return 108+2*j;
            }
        }
        if (ncg > 0) {
            for (int j=0; j<ncg; j++) {
                (*data_in) >> scg[j];
            }
            if (!data_in->good()) {
                return 104;
            }
        }
        if (flug) {
            for (int j=0; j<ngeom; j++) {
                if (!flug[j]->setState(*data_in)) {
                    return 108+2*(ngeom+j);
                }
                if (!flugT[j]->setState(*data_in)) {
                    return 109+4*(ngeom+j);
                }
            }
        }

        (*data_in) >> Eph_ave >> Nph >> Eel_ave >> Nel >> Eph_sc >> Nsc;
        if (!data_in->good()) {
            return 1031;
        }

        return 0;
    };

    /*! Reset the variables used for accumulating results */
    void resetCounter() {
        EGS_AdvancedApplication::resetCounter();
        kerma->reset();
        for (int j=0; j<ngeom; j++) {
            if (kerma_r[j]) {
                kerma_r[j]->reset();
            }
        }
        if (ncg > 0) {
            for (int j=0; j<ncg; j++) {
                scg[j] = 0;
            }
        }
        if (flug) {
            for (int j=0; j<ngeom; j++) {
                flug[j]->reset();
                flugT[j]->reset();
            }
        }

        Eph_sc  = 0;
        Nsc = 0;
        Eph_ave = 0;
        Nph = 0;
        Eel_ave = 0;
        Nel = 0;

    };

    /*! Add simulation results */
    int addState(istream &data) {
        int err = EGS_AdvancedApplication::addState(data);
        if (err) {
            return err;
        }
        EGS_ScoringArray ktmp(ngeom);
        if (!ktmp.setState(data)) {
            return 101;
        }
        (*kerma) += ktmp;
        for (int j=0; j<ngeom; j++) {
            if (kerma_r[j]) {
                EGS_ScoringArray ktmp_r(kerma_r[j]->regions());
                if (!ktmp_r.setState(data)) {
                    return 108+2*j;
                }
                (*kerma_r[j]) += ktmp_r;
            }
        }
        if (ncg > 0) {
            for (int j=0; j<ncg; j++) {
                double tmp;
                data >> tmp;
                if (!data.good()) {
                    return 104;
                }
                scg[j] += tmp;
            }
        }
        if (flug) {
            EGS_ScoringArray tg(flu_nbin);
            EGS_ScoringArray tgT(ngeom);
            for (int j=0; j<ngeom; j++) {
                if (!tg.setState(data)) {
                    return 108+2*(ngeom+j);
                }
                (*flug[j]) += tg;
                if (!tgT.setState(data)) {
                    return 109+4*(ngeom+j);
                }
                (*flugT[j]) += tgT;
            }
        }

        EGS_Float aux_Eph_ave, aux_Nph, aux_Eel_ave, aux_Nel, aux_Eph_sc, aux_Nsc;
        data >> aux_Eph_ave >> aux_Nph >> aux_Eel_ave >> aux_Nel >> aux_Eph_sc >> aux_Nsc;
        if (!data.good()) {
            return 1036;
        }

        Eph_sc  += aux_Eph_sc;
        Nsc     += aux_Nsc;
        Eph_ave += aux_Eph_ave;
        Nph     += aux_Nph;
        Eel_ave += aux_Eel_ave;
        Nel     += aux_Nel;

        return 0;
    };

    /*! Output the results of a simulation. */
    void outputResults() {

        egsInformation("\n\n last case = %lld fluence = %g\n\n",
                       current_case,source->getFluence());

        EGS_Float F = current_case/getFluence();

        if (Eph_sc > kermaEpsilon && Nsc > kermaEpsilon) {
            Eph_sc /= Nsc;
            egsInformation(" Mean scoring photon energy <Epsc> = %g MeV\n\n",Eph_sc);
        }

        if (Eph_ave > kermaEpsilon && Nph > kermaEpsilon) {
            Eph_ave /= Nph;
            egsInformation(" Mean source photon energy <Ep> = %g MeV\n\n",Eph_ave);
        }

        if (Eel_ave > kermaEpsilon && Nel > kermaEpsilon) {
            Eel_ave /= Nel;
            egsInformation(" Mean source electron energy <Ee> = %g MeV\n\n",Eel_ave);
        }

        outputKermaResults(F);

        if (flug || flugT) {
            outputFluenceResults(F);
        }

    };

    /*! Output the dosimetry results of a simulation. */
    void outputKermaResults(const EGS_Float &F) {
        /************************************************************/
        /* Print out kerma in scoring volume regions and the total. */
        /************************************************************/

        /* Normalize to actual source fluence */
        EGS_Float MeVtoJgtokg = 1.6021773e-10, // energy and mass conversion
                  normD = MeVtoJgtokg*F;
        int irmax_digits = getDigits(max_sc_reg),
            max_medl     = getMaxMedLength();
        int count = 0;
        string line;
        string med_name;
        double r,dr, fe, dfe;
        EGS_Float rho = -1.0, m = -1.0;
        int imed = -1, nreg = 0;
        /* Compute deposited energy and dose */
        for (int j=0; j<ngeom; j++) {
            if (F == 1) {
                egsInformation("\n\n==> Calculation summary (per particle) in geometry: %s\n\n",
                               geoms[j]->getName().c_str());
                if (flug) {
                    egsInformation(
                        "  %*s        m/g          Edep/[MeV]                   K/[Gy]            "
                        "      Flu/[cm-2]           (muen/rho)=K/Flu/Eave[cm^2/g]      %n\n",
                        irmax_digits,"ir",&count);
                }
                else {
                    egsInformation(
                        "  %*s        m/g          Edep/[MeV]                   K/[Gy]         %n\n",
                        irmax_digits,"ir",&count);
                }
            }
            else {
                egsInformation("\n==> Calculation summary (per fluence) in geometry: %s\n\n",
                               geoms[j]->getName().c_str());
                if (flug) {
                    egsInformation(
                        "  %*s        m/g        Edep/[MeV*cm2]                 K/[Gy*cm2]       "
                        "        Flu/[cm-2]           (muen/rho)=K/Flu/Eave[cm^2/g]      %n\n",
                        irmax_digits,"ir",&count);
                }
                else {
                    egsInformation(
                        "  %*s        m/g        Edep/[MeV*cm2]                 K/[Gy*cm2]         %n\n",
                        irmax_digits,"ir",&count);

                }
            }
            line.append(count,'-');
            egsInformation("  %s\n",line.c_str());

            if (kerma_r[j]) {
                nreg = geoms[j]->regions();
                for (int ir = 0; ir < nreg; ir++) {
                    if (is_sensitive[j][ir]) {
                        imed     = getMedium(ir);
                        rho      = getMediumRho(imed);
                        med_name = getMediumName(imed);
                        m = mass[j][ir];
                        kerma_r[j]->currentResult(ir,r,dr);
                        if (r > 0) {
                            dr = 100.*dr/r;
                            if (dr < 100.*kermaEpsilon) {
                                dr = 100.0;
                            }
                        }
                        else {
                            dr=100.0;
                        }
                        if (flug) {
                            flugT[j]->currentResult(ir,fe,dfe);
                            if (fe > 0) {
                                dfe = 100*dfe/fe;
                            }
                            else {
                                dfe = 100;
                            }
                            egsInformation("  %*d  %12.6e %12.6e +/- %-8.4f%% %12.6e +/- %-8.4f%% %12.6e +/- %-8.4f%% %12.6e +/- %-8.4f%%\n",
                                           irmax_digits, ir, m, r*F, dr, r*normD/m, dr,
                                           fe*F*rho/m, dfe, r/(fe*rho)/Eph_sc, sqrt(dr*dr+dfe*dfe));
                        }
                        else {
                            egsInformation("  %*d  %12.6e %12.6e +/- %-8.4f%% %12.6e +/- %-8.4f%%\n",
                                           irmax_digits, ir, m, r*F, dr, r*normD/m, dr);

                        }
                    }
                }
                egsInformation("  %s\n",line.c_str());
            }
            kerma->currentResult(j,r,dr);
            if (r > 0) {
                dr = dr/r;
                if (dr < kermaEpsilon) {
                    dr = 1.0;
                }
            }
            else {
                dr=1.0;
            }
            egsInformation("  Total: %12.6e %12.6e +/- %-8.4f%% %12.6e +/- %-8.4f%%\n",
                           mass_cv[j],r*F,dr*100.,r*normD/mass_cv[j],dr*100.);
            egsInformation("  %s\n",line.c_str());
            count = 0;
            line.clear();// reset line
        }

        egsInformation("\n\n");
        /******************************************************************************/
        /* Print out Kerma ratio in the whole scoring volume of correlated geometries */
        /******************************************************************************/
        if (ncg > 0) {
            egsInformation("%-20s %-20s    KERMA ratio\n","Geometry 1", "Geometry 2");

            for (int j=0; j<ncg; j++) {
                double r1,dr1,r2,dr2;
                kerma->currentResult(gind1[j],r1,dr1);
                kerma->currentResult(gind2[j],r2,dr2);
                if (r1 > 0 && r2 > 0) {
                    double rc=(scg[j]/(r1*r2*current_case)-1)/(current_case-1);
                    dr1 /= r1;
                    dr2 /= r2;
                    double dr = dr1*dr1 + dr2*dr2 - 2*rc;
                    if (dr > 0) {
                        dr = sqrt(dr);
                    }
                    double r = r1*mass_cv[gind2[j]]/(r2*mass_cv[gind1[j]]);
                    egsInformation("%-20s %-20s     %-11.8lg +/- %-10.8lg [%-10.6lf%%]\n",
                                   geoms[gind1[j]]->getName().c_str(),
                                   geoms[gind2[j]]->getName().c_str(),r,r*dr,100.*dr);
                }
                else {
                    egsInformation("zero dose\n");
                }
            }

        }
    }

    /*! Output the fluence results of a simulation. */
    void outputFluenceResults(const EGS_Float &F) {

        string spe_name = constructIOFileName(".agr",true);
        ofstream spe_output(spe_name.c_str());
        if (flug) {
            spe_output << "# Photon fluence \n";
            spe_output << "# \n";
            spe_output << "@    legend 0.2, 0.8\n";
            spe_output << "@    legend box linestyle 0\n";
            spe_output << "@    legend font 4\n";
            spe_output << "@    xaxis  label \"energy / MeV\"\n";
            spe_output << "@    xaxis  label char size 1.560000\n";
            spe_output << "@    xaxis  label font 4\n";
            spe_output << "@    xaxis  ticklabel font 4\n";
            spe_output << "@    yaxis  label \"fluence / MeV\\S-1\\Ncm\\S-2\"\n";
            spe_output << "@    yaxis  label char size 1.560000\n";
            spe_output << "@    yaxis  label font 4\n";
            spe_output << "@    yaxis  ticklabel font 4\n";
            spe_output << "@    title \""<< output_file <<"\"\n";
            spe_output << "@    title font 4\n";
            spe_output << "@    title size 1.500000\n";
            spe_output << "@    subtitle \"pegs4 data: "<< pegs_file <<"\"\n";
            spe_output << "@    subtitle font 4\n";
            spe_output << "@    subtitle size 1.000000\n";
        }
        egsInformation("\n\nPhoton fluence summary\n"
                       "======================\n");
        double fe,dfe,fp,dfp;
        for (int j=0; j<ngeom; j++) {
            egsInformation("\nGeometry: %s \n\n",geoms[j]->getName().c_str());
            if (flugT) {
                int count = 0;
                int nreg = geoms[j]->regions();
                int irmax_digits = getDigits(max_sc_reg);
                egsInformation(
                    "  %*s      m/g            Flu/[cm-2]         %n\n",
                    irmax_digits,"ir",&count);
                string line;
                line.append(count,'-');
                egsInformation("  %s\n",line.c_str());

                for (int ir = 0; ir < nreg; ir++) {
                    if (is_sensitive[j][ir]) {
                        int imed = getMedium(ir);
                        /* Per volume */
                        EGS_Float m = mass[j][ir];
                        EGS_Float normT = F*getMediumRho(imed)/m;
                        flugT[j]->currentResult(ir,fe,dfe);
                        if (fe > 0) {
                            dfe = 100*dfe/fe;
                        }
                        else {
                            dfe = 100;
                        }
                        egsInformation("  %*d  %12.6e %12.6e +/- %-8.4f%%\n",
                                       irmax_digits, ir, m,fe*normT, dfe);
                    }
                }
            }

            if (flug) {
                /* Diff. fluence currently in whole scoring volume */
                EGS_Float normV = F/(mass_cv[j]/rho_cv[j]);//per unit volume
                EGS_Float norm, expbw, DE;
                if (flu_s) {
                    expbw = exp(1.0/flu_a);// exp{log(Emax/Emin)/Nbin}
                }
                else {
                    norm = normV*flu_a; //per unit bin width (linear scale)
                }
                spe_output<<"@    s"<<j<<" errorbar linestyle 0\n";
                spe_output<<"@    s"<<j<<" legend \""<<
                          geoms[j]->getName().c_str()<<"\"\n";
                spe_output<<"@target G0.S"<<j<<"\n";
                spe_output<<"@type xydy\n";
                egsInformation("\n\n"
                               "   Emid/MeV    dFlu/dE/[MeV-1/cm2]   DFlu/[MeV-1/cm2]\n"
                               "   --------------------------------------------------\n");
                for (int i=0; i<flu_nbin; i++) {
                    flug[j]->currentResult(i,fe,dfe);
                    EGS_Float e = (i+0.5-flu_b)/flu_a;
                    if (flu_s) {
                        e = exp(e);
                        DE    = exp(flu_xmin)*pow(expbw,i)*(expbw-1);
                        norm  = normV/DE; //per unit bin width (log scale)
                    }
                    spe_output<<e<<" "<<fe *norm<<" "<<dfe *norm<< "\n";
                    egsInformation("%11.6f  %14.6e      %14.6e\n",
                                   e,fe*norm,dfe*norm);
                }
                spe_output << "&\n";
            }
        }
    }

    // determine maximum medium name length
    int getMaxMedLength() {
        char buf[32];
        int count = 0;
        int imed=0, max_medl = -1;
        for (imed=0; imed < getnMedia(); imed++) {
            sprintf(buf,"%s%n",getMediumName(imed),&count);
            if (count > max_medl) {
                max_medl = count;
            }
        }
        return max_medl;
    }

    /*! Get number of digits of the_int.  */
    int getDigits(int the_int) {
        int imax = 10;
        while (the_int>=imax) {
            imax*=10;
        }
        return (int)log10((float)imax);
    };
    /*! Get the current simulation result for first geometry.  */
    void getCurrentResult(double &sum, double &sum2, double &norm,
                          double &count) {
        count = current_case;
        double flu = source->getFluence(),
               mCV = kerma_r[0] ? mass[0][active_reg]:mass_cv[0];
        norm = flu > 0 ? 1.602e-10*count/(flu*mCV) : 0;
        if (kerma_r[0]) {
            kerma_r[0]->currentScore(active_reg,sum,sum2);
        }
        else {
            kerma->currentScore(0,sum,sum2);
        }
    };

    /* Select photon mean-free-path */
    void selectPhotonMFP(EGS_Float &dpmfp) {
        int np = the_stack->np-1;
        EGS_Vector x(the_stack->x[np],the_stack->y[np],the_stack->z[np]);
        EGS_Vector u(the_stack->u[np],the_stack->v[np],the_stack->w[np]);
        int ireg   = the_stack->ir[np]-2, newmed = geometry->medium(ireg);
        EGS_Float tstep = TSTEP_MAX;
        //******************************************************************
        // FD Track-length kerma estimation for photons entering or aimed
        // at the collecting volume. It requires an FD geometry to direct
        // the ray-tracing and that the photon has not touched an exclusion
        // region (latch >= 0). Photons inside this geometry or any scoring
        // region are also ray-traced.
        //******************************************************************
        if (fd_geom &&  // TAKES ALL PHOTONS !!!
                (is_sensitive[ig][ireg] ||
                 fd_geom->howfar(-1,x,u,tstep,&newmed)>= 0 ||
                 fd_geom->isInside(x))) {
            /* Photon at or aimed at cavity */
            int errK = scoreInCV();
        }
        dpmfp = -log(1 - rndm->getUniform());
        return;

    };

protected:

    /*! Start a new shower.  */
    int startNewShower() {
        int res = EGS_Application::startNewShower();
        if (res) {
            return res;
        }
        if (current_case != last_case) {
            if (ncg > 0) {
                for (int j=0; j<ncg; j++)
                    scg[j] += kerma->thisHistoryScore(gind1[j])*
                              kerma->thisHistoryScore(gind2[j]);
            }
            kerma->setHistory(current_case);
            for (int j=0; j<ngeom; j++) {
                if (kerma_r[j]) {
                    kerma_r[j]->setHistory(current_case);
                }
            }
            if (flug) {
                for (int j=0; j<ngeom; j++) {
                    flug[j]->setHistory(current_case);
                    flugT[j]->setHistory(current_case);
                }
            }
            last_case = current_case;
        }
        return 0;
    };

private:

    void outputSensitiveRegions(const int &igeom, const bool *isScoring);

    int              ngeom;     // number of geometries to calculate
    // quantities of interest
    int              ig;        // current geometry index

    int              ncg;       // number of correlated geometry pairs.
    int              *gind1,
                     *gind2;    // indices of correlated geometries

    double           *scg;      // sum(kerma(gind1[j])*kerma(gind2[j]);

    EGS_BaseGeometry **geoms;   // geometries for which to calculate the
    // quantites of interest.
    EGS_AffineTransform **transforms;
    // transformations to apply before transporting
    // for each geometry
    bool          **is_sensitive; // array of flags for each region in each
    // geometry, which is true if the region
    // belongs to the collecting volume and false otherwise

    bool          **is_excluded;// array of flags for each region in each
    // geometry, which is true if the region
    // is excludedd, false otherwise

    EGS_ScoringArray *kerma;    // total kerma per geometry
    EGS_ScoringArray **kerma_r; // individual kerma per region

    EGS_Interpolator *E_Muen_Rho;

    /****************************************************************/

    EGS_ScoringArray **flug;    // Differential fluence in ALL scoring regions
    EGS_ScoringArray **flugT;   // Integral fluence in EACH scoring region
    EGS_Float       flu_a,
                    flu_b,
                    flu_xmin,
                    flu_xmax;
    int             flu_s,
                    flu_nbin;
    EGS_Float      *rho_cv;       // mass density of scoring volume
    EGS_Float      *mass_cv;      // mass of scoring volume material
    EGS_Float       **mass;       // masses of the CV regions.
    int            *n_scoring_r; // Number of scoring regions in geometry.
    int             max_sc_reg;  // Largest scoring region in all geometries
    int             active_reg;  // Scoring region in first geometry shown in progress
    int            *active_med;  // Scoring medium in each geometry

    /*! Force-Detection geometry.
      If no FD geometry defined, kerma scoring only done when photons
      enter scoring regions. If an FD geometry is defined photons AIMED
      AT or INSIDE that geometry score a contribution weighted by
      the probability of reaching the scoring volume via a ray-tracing algorithm.
      Photons already in a scoring region are also ray-traced if an FD geometry
      is defined.
      NOTE: Although one could use an input flag to set FD on or off independently of
            the definition of an FD geometry, thus ray-tracing everywhere, it is useful to
            require the definition of an FD geometry (which can also be the whole geometry).
            Ideally, the FD geometry sets the direction in which ray-tracing is done, and
            hence should be judciously chosen to avoid wasting time ray-tracing particles
            that never hit a scoring region.
     */
    EGS_BaseGeometry *fd_geom;  // Global FD geometry
    EGS_BaseGeometry **fd_geoms;// Individual FD geometries

    EGS_Float Eph_ave; // Mean source photon energy
    EGS_Float Nph;     // Number of source photons
    EGS_Float Eel_ave; // Mean source electron energy
    EGS_Float Nel;     // Number of source electrons
    EGS_Float Eph_sc;  // Mean scoring photon energy
    EGS_Float Nsc;     // Number of scoring photons


    static string revision;
};

string EGS_KermaApplication::revision = "$Revision: 1.1 $";

extern __extc__  void
F77_OBJ_(select_photon_mfp,SELECT_PHOTON_MFP)(EGS_Float *dpmfp) {
    EGS_Application *a = EGS_Application::activeApplication();
    EGS_KermaApplication *app = dynamic_cast<EGS_KermaApplication *>(a);
    if (!app) egsFatal("select_photon_mfp called with active application "
                           " not being of type EGS_KermaApplication!\n");
    app->selectPhotonMFP(*dpmfp);
}

int EGS_KermaApplication::initScoring() {

    EGS_Input *options = input->takeInputItem("scoring options");
    if (options) {

        //
        // *********** calculation geometries
        //
        vector<EGS_BaseGeometry *> geometries;
        vector<string>      fd_global_gs;
        vector<int *>       cavity_regions;
        vector<int>         n_cavity_regions;
        vector<int>         n_region_groups;
        vector<EGS_Float *> cavity_masses;
        vector<int>         n_cavity_masses;
        vector<int *>       excluded_regions;
        vector<int>         n_excluded_regions;
        vector<EGS_AffineTransform *> transformations;
        EGS_Input *aux;
        EGS_BaseGeometry::setActiveGeometryList(app_index);
        while ((aux = options->takeInputItem("calculation geometry"))) {

            /* Handle deprecated inputs */

            string dummy_name;
            vector<int> dummy_regs;
            EGS_Float dummy_mass;
            int dummy_err = aux->getInput("cavity geometry",dummy_name);
            if (!dummy_err) egsFatal("initScoring: Using deprecated input key 'cavity geometry' \n"
                                         "use 'FD geometry' instead!!!\n\n");
            dummy_err = aux->getInput("cavity regions",dummy_regs);
            if (!dummy_err) egsFatal("initScoring: Using deprecated input key 'cavity regions' \n"
                                         "use 'scoring regions' instead!!!\n\n");
            dummy_err = aux->getInput("cavity mass",dummy_mass);
            if (!dummy_err) egsFatal("initScoring: Using deprecated input key 'cavity mass' \n"
                                         "use 'scoring region masses' or 'scoring volume mass' instead!!!\n\n");

            /* Process inputs */

            string gname, cgname, apertString;
            int err  = aux->getInput("geometry name",gname);
            int errc = aux->getInput("FD geometry",cgname);
            vector<int> apert; // Excluded regions
            int err4 = aux->getInput("excluded regions",apertString);
            vector<EGS_Float> cmass;
            int err2 = aux->getInput("scoring region masses",cmass);

            if (err) egsWarning("initScoring: missing/wrong 'geometry name' "
                                    "input\n");

            if (errc) {
                cgname = "";//Set to empty, i.e., no FD geometry for this
            }

            /* Get scoring regions: expect different input approaches */

            enum Kind {individual, ranges, groups, incremental};
            Kind k = individual;
            int err1 = 0;

            vector<string> reg_inp_key = {"scoring regions",
                                          "scoring region ranges",
                                          "scoring start region",
                                          "incremental scoring regions"
                                         };

            string cavString;
            for (int ir_choice = 0; ir_choice < 4; ir_choice++) {
                if (!aux->getInput(reg_inp_key[ir_choice], cavString)) {
                    k = Kind(ir_choice);
                    break;
                }
            }

            vector<int> cav;

            bool mass_per_group = false;

            // Load the geometry
            EGS_BaseGeometry::setActiveGeometryList(app_index);
            EGS_BaseGeometry *g = EGS_BaseGeometry::getGeometry(gname);
            if (!g) {
                egsWarning("initScoring: no geometry named %s -->"
                                    " input ignored\n",gname.c_str());
            } else {
                g->getNumberRegions(apertString, apert);
                g->getLabelRegions(apertString, apert);

                switch (k) {
                case individual: {
                    // Read entries for individual regions
                    g->getNumberRegions(cavString, cav);
                    g->getLabelRegions(cavString, cav);
                    break;
                }
                case ranges: {
                    // Read pairs of contiguous range of regions
                    vector<int> pairs;
                    g->getNumberRegions(cavString, pairs);
                    g->getLabelRegions(cavString, pairs);

                    if (pairs.size() % 2 == 0) {
                        vector<EGS_Float> le_mass;
                        //User provided one mass value for each group
                        if (pairs.size()/2 == cmass.size()) {
                            le_mass = cmass;
                            cmass.clear();
                            mass_per_group = true;
                        }
                        unsigned int j = 0, valid_pair = 0;
                        while (j < pairs.size()) {
                            int ireg = pairs[j], ereg = pairs[++j];
                            if (ereg > ireg) {
                                for (unsigned i = ireg; i <= ereg; i++) {
                                    cav.push_back(i);
                                    if (mass_per_group) {
                                        cmass.push_back(le_mass[valid_pair]);
                                    }
                                }
                                j++;
                                valid_pair++;
                            }
                            else {
                                egsFatal("initScoring: wrong scoring region range'\n"
                                        " on %d-th pair: %d %d\n",
                                        valid_pair+1,ireg,ereg);
                            }
                        }
                        n_region_groups.push_back(pairs.size());
                    }
                    else {
                        egsFatal("initScoring: Error in 'scoring region ranges' input\n");
                    }
                    break;
                }
                case groups: {
                    vector <int> d_start, d_stop;

                    g->getNumberRegions(cavString, d_start);
                    g->getLabelRegions(cavString, d_start);

                    int err2g = aux->getInput("scoring stop region",cavString);

                    if (!err2g) {
                        g->getNumberRegions(cavString, d_stop);
                        g->getLabelRegions(cavString, d_stop);

                        err1 = 0;
                        if (d_start.size() == d_stop.size()) { // groups of regions
                            vector<EGS_Float> le_mass;
                            //User provided one mass value for each group
                            if (d_start.size() == cmass.size()) {
                                le_mass = cmass;
                                cmass.clear();
                                mass_per_group = true;
                            }
                            int valid_pair = 0;
                            for (int i=0; i<d_start.size(); i++) {
                                int ir = d_start[i], fr = d_stop[i];
                                if (fr > ir) {
                                    for (int ireg=ir; ireg<=fr; ireg++) {
                                        cav.push_back(ireg);
                                        if (mass_per_group) {
                                            cmass.push_back(le_mass[valid_pair]);
                                        }
                                    }
                                    valid_pair++;
                                }
                                else {
                                    egsFatal("initScoring: wrong 'start/stop scoring regions'\n"
                                            " on %d-th triplet: %d %d\n",
                                            valid_pair+1,ir,fr);
                                }
                            }
                            n_region_groups.push_back(d_start.size());
                        }
                        else {
                            egsFatal("initScoring: Mismatch in start and stop"
                                    " scoring region groups !!!\n");
                        }
                    }
                    break;
                }
                case incremental: {
                    // Check if groups of equally spaced regions desired
                    vector<int> triplets;

                    g->getNumberRegions(cavString, triplets);
                    // Turn off sorting of the region list since it's not just regions
                    g->getLabelRegions(cavString, triplets, false);

                    if (triplets.size() % 3 == 0) {
                        vector<EGS_Float> le_mass;
                        //User provided one mass value for each group
                        if (triplets.size()/3 == cmass.size()) {
                            le_mass = cmass;
                            cmass.clear();
                            mass_per_group = true;
                        }
                        unsigned int j = 0, valid_triplet = 0;
                        while (j < triplets.size()) {
                            int ireg = triplets[j],
                                ereg = triplets[++j],
                                dreg = triplets[++j];
                            if (ereg > ireg) {
                                for (unsigned i = ireg; i <= ereg; i = i + dreg) {
                                    cav.push_back(i);
                                    if (mass_per_group) {
                                        cmass.push_back(le_mass[valid_triplet]);
                                    }
                                }
                                j++;
                                valid_triplet++;
                            }
                            else {
                                egsFatal("initScoring: wrong 'incremental scoring regions'\n"
                                        " on %d-th triplet: %d %d %d\n",
                                        valid_triplet+1,ireg,ereg,dreg);
                            }
                        }
                        n_region_groups.push_back(triplets.size());
                        //egsInformation("---> Scoring from region %d to %d in %d regions increments\n",
                        //         triplets[0], triplets[1], triplets[2]);
                    }
                    else {
                        egsFatal("initScoring: missing/wrong "
                                "'incremental scoring regions' input\n"
                                "Expected triplets: ir_min ir_max ir_delta ...\n");
                        err1 = 1;
                    }
                    break;
                }
                default:
                    err1 = 1;
                }
            }

            if (err2) {// Error reading scoring region masses
                err2 = 0;
                err2 = aux->getInput("scoring volume mass",cmass);
                if (err2 || cmass.size() != 1) {
                    egsFatal("initScoring: missing/wrong 'scoring region masses'\n"
                             "             or 'scoring volume mass' input\n");
                }
            }
            else { // Warn user. Not needed above.
                if (cav.size() != cmass.size()) {
                    if (cmass.size() == 1) {
                        if (cmass[0] < 0) {
                            egsWarning("\ninitScoring: Only one mass defined. Assuming it is the total mass!\n\n");
                            cmass[0] *= -1;
                        }
                        else {
                            egsWarning("\ninitScoring: Only one mass defined. Assuming same mass for all regions!\n\n");
                            for (int j=1; j<cav.size(); j++) {
                                cmass.push_back(cmass[0]);
                            }
                        }
                    }
                    else
                        egsFatal(
                            "\n**************************************************************\n"
                            "initScoring: Aborting due to error in mass input\n"
                            "             Number of mass entries using 'scoring region masses' must match: \n"
                            "             - Number of scoring regions using \n"
                            "             - Number of scoring region groups \n"
                            "             - Unique mass value for all scoring regions\n"
                            "             - Total volume mass (unique negative value)\n\n"
                            " Alternatively, kerma scoring in whole scoring volume triggered"
                            " by entering total volume mass using 'scoring volume mass'\n"
                            "**************************************************************\n\n");
                }
            }

            if (err || err1) {
                egsWarning("  --> input ignored\n");
            }
            else if(g) {

                int nreg = g->regions();
                int *regs = new int [cav.size()];
                EGS_Float *m_g  = new EGS_Float [cmass.size()];
                int ncav = 0;
                for (int j=0; j<cav.size(); j++) {
                    if (cav[j] < 0 || cav[j] >= nreg)
                        egsWarning("initScoring: region %d is not within"
                                    " the allowed range of 0...%d -> input"
                                    " ignored\n",cav[j],nreg-1);
                    else {
                        regs[ncav++] = cav[j];
                    }
                }
                //Transfer Vector<EGS_Float> to EGS_Float*
                for (int j=0; j<cmass.size(); j++) {
                    m_g[j] = cmass[j];
                }
                if (!ncav) {
                    egsWarning("initScoring: no sensitive regions "
                                "specified for geometry %s --> input ignored\n",
                                gname.c_str());
                    delete [] regs;
                }
                else {
                    geometries.push_back(g);
                    /*Add FD geometry name (can be empty)*/
                    fd_global_gs.push_back(cgname);
                    n_cavity_regions.push_back(ncav);
                    cavity_regions.push_back(regs);
                    cavity_masses.push_back(m_g);
                    n_cavity_masses.push_back(cmass.size());
                    transformations.push_back(
                                        EGS_AffineTransform::getTransformation(aux));
                    /* excluded regions */
                    if (!err4 && apert.size() > 0) {
                        int *ap = new int [apert.size()];
                        int nap=0;
                        for (int j=0; j<apert.size(); j++) {
                            if (apert[j] >= 0 && apert[j] < nreg) {
                                ap[nap++] = apert[j];
                            }
                            else {
                                egsFatal("\n\n*** Excluded region %d is\n"
                                            " outside the allowed range of  \n"
                                            " 0...%d  \n"
                                            " This is a fatal error\n\n",
                                            apert[j],nreg-1);
                            }
                        }
                        n_excluded_regions.push_back(nap);
                        excluded_regions.push_back(ap);
                    }
                    else {
                        excluded_regions.push_back(0);
                        n_excluded_regions.push_back(0);
                    }
                }
            }
            delete aux;
        }
        ngeom = geometries.size();
        if (!ngeom) {
            egsWarning("initScoring: no calculation geometries defined\n");
            return 1;
        }
        geoms        = new EGS_BaseGeometry* [ngeom];
        fd_geoms     = new EGS_BaseGeometry* [ngeom];
        is_sensitive = new bool* [ngeom];
        is_excluded  = new bool* [ngeom];
        rho_cv       = new EGS_Float[ngeom];
        mass_cv      = new EGS_Float[ngeom];
        n_scoring_r  = new int[ngeom];
        active_med   = new int[ngeom];
        mass       = new EGS_Float* [ngeom];
        kerma      = new EGS_ScoringArray(ngeom);
        kerma_r    = new EGS_ScoringArray* [ngeom];
        transforms = new EGS_AffineTransform* [ngeom];

        max_sc_reg = -1, active_reg = INT_MAX;

        for (int j=0; j<ngeom; j++) {
            rho_cv[j] = -1;
            geoms[j] = geometries[j]; //geoms[j]->ref();
            if (!fd_global_gs[j].empty()) {
                EGS_BaseGeometry::setActiveGeometryList(app_index);
                fd_geoms[j] =  EGS_BaseGeometry::getGeometry(fd_global_gs[j]);
            }
            else {
                fd_geoms[j] = 0;
            }
            transforms[j] = transformations[j];
            int nreg = geoms[j]->regions();
            is_sensitive[j] = new bool [nreg];
            is_excluded[j]  = new bool [nreg];
            int i;
            /* Initialize masses */
            if (n_cavity_masses[j] > 0) {
                mass[j]    = new EGS_Float [nreg];
                kerma_r[j] = new EGS_ScoringArray(nreg);
                for (i=0; i<nreg; i++) {
                    mass[j][i] = -1.0;
                }
                mass_cv[j] = 0;
            }
            else {
                mass_cv[j] = cavity_masses[j][0];
                mass[j]    = 0;
                kerma_r[j] = 0;
            }
            /* Initialize sensitive and exclude regions */
            for (i=0; i<nreg; i++) {
                is_sensitive[j][i] = false;
                is_excluded[j][i]  = false;
            }
            int imed = -999;
            for (i=0; i<n_cavity_regions[j]; i++) {
                int ireg = cavity_regions[j][i];
                is_sensitive[j][ireg] = true;
                if (imed == -999) {
                    imed = geoms[j]->medium(ireg);
                }
                else {
                    int imed1 = geoms[j]->medium(ireg);
                    if (imed1 != imed)
                        egsWarning(
                            "initScoring: different "
                            "medium %d in region %d compared to medium %d in "
                            "region %d. Hope you know what you are doing\n",
                            imed1,ireg,imed,cavity_regions[j][0]);
                }
                //if (n_cavity_masses[j] > 1){
                if (mass[j]) {
                    mass[j][ireg] = cavity_masses[j][i];
                    mass_cv[j]   += cavity_masses[j][i];
                }
                //Find largest scoring region in all geometries
                if (ireg > max_sc_reg) {
                    max_sc_reg = ireg;
                }
                if (ireg < active_reg) {
                    active_reg = ireg;
                }
            }
            active_med[j] = imed;
            n_scoring_r[j] = n_cavity_regions[j];
            delete [] cavity_regions[j];
            delete [] cavity_masses[j];
            /* Get cavity mass density */
            if (rho_cv[j] < 0) {
                rho_cv[j] = the_media->rho[imed];
            }
            else {
                EGS_Float rho_cv_new = the_media->rho[imed];
                if (rho_cv[j] != rho_cv_new)
                    egsWarning(
                        "initScoring:\n"
                        "density of cavity medium in geometry %s is %g g/cm3\n"
                        "which differs from initial cavity medium density %g g/cm3",
                        geoms[j]->getName().c_str(), rho_cv_new, rho_cv[j]);
            }

            //if (n_excluded_regions.size()>0){
            if (!n_excluded_regions.empty()) {
                vector<int> ir_sensitive_excluded;
                for (i=0; i<n_excluded_regions[j]; i++) {
                    int areg = excluded_regions[j][i];
                    /* is_sensitive takes priority */
                    if (is_sensitive[j][areg]) {
                        is_excluded[j][areg] = false;
                        ir_sensitive_excluded.push_back(areg);
                    }
                    else {
                        is_excluded[j][areg] = true;
                    }
                }

                if (ir_sensitive_excluded.size()) {
                    egsWarning(
                        "\n*******************\n"
                        "Error(initScoring):\n"
                        "*******************\n"
                        "Regions defined as sensitive and excluded in geometry %s:\n\n",
                        geoms[j]->getName().c_str());
                    // Loop through the vector and print 25 elements per line
                    for (size_t i = 0; i < ir_sensitive_excluded.size(); ++i) {
                        cout << ir_sensitive_excluded[i] << " ";
                        if ((i + 1) % 25 == 0) { // New line after every 25th element
                            cout << endl;
                        }
                    }
                    // Ensure the output ends with a newline if the last line has fewer than 25 elements
                    if (ir_sensitive_excluded.size() % 25 != 0) {
                        cout << endl;
                    }
                    egsFatal(
                        "\nalthough in the code preference is given to sensitive regions,"
                        "\nwe have opted for aborting to avoid unintended consequences\n\n");
                }

                delete [] excluded_regions[j];
            }
        }

        vector<int> cor1, cor2;
        while ((aux = options->takeInputItem("correlated geometries"))) {
            vector<string> gnames;
            int err = aux->getInput("correlated geometries",gnames);
            if (!err && gnames.size() == 2) {
                int j1, j2;
                for (j1=0; j1<ngeom; j1++)
                    if (gnames[0] == geoms[j1]->getName()) {
                        break;
                    }
                for (j2=0; j2<ngeom; j2++)
                    if (gnames[1] == geoms[j2]->getName()) {
                        break;
                    }
                if (j1 < ngeom && j2 < ngeom) {
                    cor1.push_back(j1);
                    cor2.push_back(j2);
                }
            }
        }
        if (cor1.size() > 0) {
            ncg = cor1.size();
            gind1 = new int [ncg];
            gind2 = new int [ncg];
            scg = new double [ncg];
            for (int j=0; j<ncg; j++) {
                scg[j] = 0;
                gind1[j] = cor1[j];
                gind2[j] = cor2[j];
            }
        }

        aux = options->takeInputItem("fluence scoring");
        if (aux) {
            EGS_Float flu_Emin, flu_Emax;
            int er1 = aux->getInput("minimum energy",flu_Emin);
            int er2 = aux->getInput("maximum energy",flu_Emax);
            int er3 = aux->getInput("number of bins",flu_nbin);
            vector<string> scale;
            scale.push_back("linear");
            scale.push_back("logarithmic");
            flu_s = aux->getInput("scale",scale,0);
            /* Checks to ensure no scoring outside array bounds */
            /*            EGS_Float Emax = source ? source->getEmax() : 0;
                        //egsInformation("\n=> Emax = %g MeV\n\n",Emax);
                        if ( flu_Emax <= Emax ) {
                           flu_Emax = Emax + 2*(flu_Emax - flu_Emin)/flu_nbin;
                           flu_nbin += 2;
                        }*/
            if (!er1 && !er2 && !er3) {
                flug  = new EGS_ScoringArray * [ngeom];
                for (int j=0; j<ngeom; j++) {
                    flug[j] = new EGS_ScoringArray(flu_nbin);
                }
                if (flu_s == 0) {
                    flu_xmin = flu_Emin;
                    flu_xmax = flu_Emax;
                }
                else {
                    flu_xmin = log(flu_Emin);
                    flu_xmax = log(flu_Emax);
                }
                flu_a = flu_nbin;
                flu_a /= (flu_xmax - flu_xmin);
                flu_b = -flu_xmin*flu_a;
            }
            else {
                egsInformation("\n\n******* Missing differential fluence inputs"
                               " errors: %d %d %d\n",er1,er2,er3);
                egsInformation("            => Integral fluence scoring ONLY\n\n");
            }
            // Integral fluence array
            flugT = new EGS_ScoringArray* [ngeom];
            for (int j=0; j<ngeom; j++) {
                flugT[j] = new EGS_ScoringArray(geoms[j]->regions());
            }

            delete aux;
        }

        string emuen_file;
        int errKmuen = options->getInput("emuen file", emuen_file);
        if (errKmuen) {
            // Handle legacy option
            errKmuen = options->getInput("muen file", emuen_file);
            if (errKmuen)
                egsFatal(
                    "\n\n***  Wrong/missing 'emuen file' input for a "
                    "Kerma calculation\n    This is a fatal error\n\n");
        }

        emuen_file = egsExpandPath(emuen_file);

        ifstream muen_data(emuen_file.c_str());
        if (!muen_data) {
            egsFatal(
                "\n\n***  Failed to open emuen file %s\n"
                "     This is a fatal error\n", emuen_file.c_str());
        }
        else {
            egsInformation(
                "\n\n=============== Kerma Scoring ===============\n"
                "E*muen/rho file: %s\n"
                "=============================================\n\n",
                emuen_file.c_str());
        }
        int ndat;
        muen_data >> ndat;
        if (ndat < 2 || muen_data.fail()) egsFatal(
                "\n\n*** Failed to read emuen data file\n");
        EGS_Float *xmuen = new EGS_Float [ndat];
        EGS_Float *fmuen = new EGS_Float [ndat];
        for (int j=0; j<ndat; j++) {
            muen_data >> xmuen[j] >> fmuen[j];
        }
        if (muen_data.fail()) egsFatal(
                "\n\n*** Failed to read emuen data file\n");
        E_Muen_Rho = new EGS_Interpolator(ndat,log(xmuen[0]),
                                          log(xmuen[ndat-1]),fmuen);
        delete [] xmuen;
        delete [] fmuen;
        // Global FD definition
        string fd_global_g;
        int errGFDgeom = options->getInput("Default FD geometry",fd_global_g);
        if (!errGFDgeom) {
            EGS_BaseGeometry::setActiveGeometryList(app_index);
            fd_geom = EGS_BaseGeometry::getGeometry(fd_global_g);
            if (fd_geom) {
                // Use global FD geometry for calc geom without one
                for (int i = 0; i < ngeom; i++) {
                    if (!fd_geoms[i]) {
                        fd_geoms[i] = fd_geom;
                    }
                }
            }
        }
        else {
            errGFDgeom = options->getInput("cavity geometry",fd_global_g);
            if (!errGFDgeom) {
                egsFatal("Using deprecated input key 'cavity geometry' use 'Default FD geometry' instead!!!");
            }
        }

        delete options;
    }
    else {
        egsWarning("\n\n*********** no 'scoring options' input *********\n\n");
        return 2;
    }

    return 0;
}

#define REGIONS_ENTRIES 100
#define REGIONS_PER_LINE 25
#define GROUPS_PER_LINE 10
void EGS_KermaApplication::outputSensitiveRegions(const int &igeom, const bool *isScoring) {

    int nsensitive = 0, nreg = geoms[igeom]->regions(), max_reg = 0;
    for (int i = 0; i < nreg; i++) {
        if (isScoring[i]) {
            nsensitive++;
            max_reg = i;
        }
    }
    //egsInformation("---> There are %d scoring regions in geometry %s !\n",
    //                nsensitive, geoms[igeom]->getName().c_str());

    char buf[128];
    string scoring_str;

    // Report scoring regions in the geometry.
    if (nsensitive == nreg) {
        scoring_str = "ALL\n";
    }
    else {
        scoring_str = "\n     ";
        int start = 0, stop = 0, k = 0, entries = 0, groups = 0;
        bool line_ended;
        /* List up to 100 scoring groups or regions */
        while (k < nreg && entries < REGIONS_ENTRIES) {
            line_ended = false;
            if (isScoring[k]) {
                start = k;
                entries++;
                while (isScoring[k] && k < nreg) {
                    k++;
                }

                stop = k-1;

                if (start < stop) {
                    groups++;
                    if (groups % GROUPS_PER_LINE) {
                        sprintf(buf,"%d-%d ",start,stop);
                    }
                    else {
                        sprintf(buf,"%d-%d\n     ",start,stop);
                    }
                }
                else if (k % REGIONS_PER_LINE) {
                    sprintf(buf," %d",start);
                }
                else {
                    sprintf(buf," %d\n     ",start);
                    line_ended = true;
                }

                if (entries == REGIONS_ENTRIES) {
                    sprintf(buf,"... %d\n",max_reg);
                    line_ended = true;
                }
                scoring_str += buf;
            }
            k++;
        }
        if (!line_ended) {
            scoring_str += "\n";
        }
    }

    egsInformation("%s",scoring_str.c_str());

}

void EGS_KermaApplication::describeSimulation() {

    EGS_AdvancedApplication::describeSimulation();

    egsInformation("**********************************************\n"
                   "   Volumetric Track-length Kerma estimation \n"
                   "**********************************************\n\n");

    for (int j=0; j<ngeom; j++) {
        egsInformation("Calculation geometry: %s\n",
                       geoms[j]->getName().c_str());
        geoms[j]->printInfo();
        if (fd_geoms[j])
            egsInformation("---> Scoring using forced detection (FD)\n"
                           "     for photons aimed at or inside geometry %s\n",
                           fd_geoms[j]->getName().c_str());
        else {
            egsInformation("\n---> Scoring only when photon enters volume\n");
        }

        //"     including those scattered inside the geometry.\n");
        //"     Fluence is equivalent to FLURZ total fluence!\n");
        egsInformation("     Sensitive regions in %s :",
                       getMediumName(active_med[j]));
        outputSensitiveRegions(j,is_sensitive[j]);

        if (kerma_r[j]) {
            egsInformation("     Scoring in %d individual regions\n",n_scoring_r[j]);
        }
        else {
            egsInformation("     Scoring in whole scoring volume\n");
        }

        egsInformation("     Exclude contributions from photons entering regions : ");
        int nexcl = 0;
        for (int i = 0; i < geoms[j]->regions(); i++) {
            if (is_excluded[j][i]) {
                egsInformation(" %d",i);
                nexcl++;
            }
        }
        if (!nexcl) {
            egsInformation(" NONE");
        }
        egsInformation("\n\n");
    }
}

#ifdef BUILD_APP_LIB
APP_LIB(EGS_KermaApplication);
#else
APP_MAIN(EGS_KermaApplication);
#endif
// int main(int argc, char **argv) {
//
//     EGS_KermaApplication app(argc,argv);
//     int err = app.initSimulation();
//     if( err ) return err;
//     err = app.runSimulation();
//     if( err < 0 ) return err;
//     return app.finishSimulation();
//
// }
