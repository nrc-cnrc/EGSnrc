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
#  Contributors:
#
###############################################################################
#
#  C++ user code for estimating the quantity kerma in a volume.
#
#  Additionally, the fluence in the volume can be also calculated if
#  requested in the scoring options input block. Two calculation options
#  are available:
#
#  - If a cavity geometry provided, a forced detection scoring technique
#    can be used to score kerma and fluence for photons reaching the geometry
#    that haven't been in any of the exclusion regions. Photons interacting
#    inside the cavity are NOT included.
#
#  - If no geometry provided, an analog TL scoring 'a la FLURZnrc' is used
#    for kerma and fluence.
#
#  Required: E*muen or E*mutr file for scoring either collision or total kerma
#  --------  for the cavity medium
#
#  Calculations for multiple geometries.
#
#  Kerma ratios can be calculated using a correlated scoring technique.
#
#  Exclusion of user specified regions.
#
#  Scoring volume regions must be provided (consider using labels).
#
#  Fluence scoring must be specifically requested.
#
#  Dose calculation in the cavity can be done using a dose scoring ausgab
#  object.This is useful to check the validity of the kerma-approximation.
#
#  NOTE 1 : Dose calculation with very high ECUT produces an estimate of
#           total kerma, not collision kerma.
#
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

EGS_Float Eave;

class APP_EXPORT EGS_KermaApplication : public EGS_AdvancedApplication {

public:

    /*! Constructor */
    EGS_KermaApplication(int argc, char **argv) :
        EGS_AdvancedApplication(argc,argv), ngeom(0),
        kerma(0), scg(0), cgeom(0),
        ncg(0), flug(0),flugT(0) {
        Eave=0.0;
    };

    /*! Destructor.  */
    ~EGS_KermaApplication() {
        if (kerma) {
            delete kerma;
        }
        if (ngeom > 0) {
            if (flug) {
                for (int j=0; j<ngeom; j++) if (flug[j]) {
                        delete flug[j];
                    }
                if (flug) {
                    delete [] flug;
                }
                if (flugT) {
                    delete flugT;
                }
            }
            delete [] geoms;
            delete [] mass;
            int j;
            for (j=0; j<ngeom; j++) if (transforms[j]) {
                    delete transforms[j];
                }
            if (transforms) {
                delete [] transforms;
            }
            for (j=0; j<ngeom; j++) {
                delete [] is_cavity[j];
            }
            delete [] is_cavity;
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
        egsInformation("This is EGS_KermaApplication %s based on\n"
                       "      EGS_AdvancedApplication %s\n\n",
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
            int latch = the_stack->latch[np];
            /* Analog Kerma scoring */
            if (is_cavity[ig][ir]) {
                if (!cgeom && latch >= 0) {
                    EGS_Float E = the_stack->E[np], gle = log(E),
                              emuen_rho   = E_Muen_Rho->interpolateFast(gle),
                              wtstep      = the_stack->wt[np]*the_epcont->tvstep;

                    kerma->score(ig,wtstep*emuen_rho*rho_cav);
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
                            flugT->score(ig,wtstep);
                        }
                    }
                }
                /* Mark photon inside cavity */
                latch = 1;
            }
            /* Mark photons in exclusion regions */
            else if (is_excluded[ig][ir]) {
                latch = -1;
            }
            /* photon outside cavity and has not been in any exclusion zone */
            else if (latch >= 0) {
                latch = 0;
            }

            the_stack->latch[np] = latch;

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
        Eave += p.q ? p.E - the_useful->rm: p.E;

        int err = startNewShower();
        if (err) {
            return err;
        }
        EGS_BaseGeometry *save_geometry = geometry;
        for (ig=0; ig<ngeom; ig++) {
            geometry = geoms[ig];
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

        Photons touching excludedd region are immediately discarded.

        TODO: Use region labels to define cavity regions

     */
    int scoreInCavity() {

        int np = the_stack->np-1;
        EGS_Float E = the_stack->E[np];

        if (E < the_bounds->pcut) {
            return 0;
        }

        EGS_Vector x(the_stack->x[np],the_stack->y[np],the_stack->z[np]);
        EGS_Vector u(the_stack->u[np],the_stack->v[np],the_stack->w[np]);

        int ireg   = the_stack->ir[np]-2, newmed = geometry->medium(ireg);

        EGS_Float tstep;
        int inew;
        int imed = -1;
        EGS_Float gmfp, sigma = 0, cohfac = 1;
        EGS_Float cohfac_int, gle = log(E), //rho_cav, -> global var now
                              exp_Lambda = 0, Lambda_to_CV = 0;
        double Lambda = 0, ttot = 0, t_cav = 0;
        bool enters_cavity  = false;
        /* Ray-trace from current position to CV and keep track of path to
         * CV and path in the CV
         */
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
            tstep = 1e35;
            inew = geometry->howfar(ireg,x,u,tstep,&newmed);

            Lambda += tstep*sigma;// keep track of path

            if (is_cavity[ig][ireg]) {   //in cavity, get path through it
                t_cav   += tstep;
                //rho_cav = the_media->rho[imed];
                if (!enters_cavity) {
                    Lambda_to_CV = Lambda - tstep*sigma;
                    enters_cavity = true;
                }
            }

            if (inew < 0) {
                break;    // outside geometry, stop and score
            }
            // track-length estimation of kerma

            if (enters_cavity && !is_cavity[ig][inew]) {
                break;    // leaves  cavity
            }

            ireg = inew;
            x += u*tstep;
            ttot += tstep;
        }
        if (enters_cavity) {
            EGS_Float wt = the_stack->wt[np];
            exp_Lambda = exp(-Lambda_to_CV);
            //exp_Lambda = exp(-Lambda);
            //--------------------------------------------
            // score Kerma
            //--------------------------------------------
            EGS_Float emuen_rho  = E_Muen_Rho->interpolateFast(gle);
            kerma->score(ig,wt*exp_Lambda*emuen_rho*rho_cav*t_cav);
            //--------------------------------------------
            // score photon fluence
            //--------------------------------------------
            if (flug) {
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
                    aux->score(je,wt*exp_Lambda*t_cav);
                    flugT->score(ig,wt*exp_Lambda*t_cav);
                }
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
                    return 108+2*j;
                }
            }
            if (!flugT->storeState(*data_out)) {
                return 109+2*ngeom;
            }
        }

        (*data_out) << Eave << endl;
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
                    return 108+2*j;
                }
            }
            if (!flugT->setState(*data_in)) {
                return 109+2*ngeom;
            }
        }

        (*data_in) >> Eave;
        if (!data_in->good()) {
            return 1031;
        }

        return 0;
    };

    /*! Reset the variables used for accumulating results */
    void resetCounter() {
        EGS_AdvancedApplication::resetCounter();
        kerma->reset();
        if (ncg > 0) {
            for (int j=0; j<ncg; j++) {
                scg[j] = 0;
            }
        }
        if (flug) {
            for (int j=0; j<ngeom; j++) {
                flug[j]->reset();
            }
            flugT->reset();
        }
        Eave = 0;
    };

    /*! Add simulation results */
    int addState(istream &data) {
        int err = EGS_AdvancedApplication::addState(data);
        if (err) {
            return err;
        }
        EGS_ScoringArray tmp(ngeom);
        if (!tmp.setState(data)) {
            return 101;
        }
        (*kerma) += tmp;
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
            for (int j=0; j<ngeom; j++) {
                if (!tg.setState(data)) {
                    return 108+2*j;
                }
                (*flug[j]) += tg;
            }
            EGS_ScoringArray tgT(ngeom);
            if (!tgT.setState(data)) {
                return 109+2*ngeom;
            }
            (*flugT) += tgT;
        }

        EGS_Float aux_Eave;
        data >> aux_Eave;
        if (!data.good()) {
            return 1036;
        }
        Eave += aux_Eave;

        return 0;
    };

    /*! Output the results of a simulation. */
    void outputResults() {
        egsInformation("\n\n last case = %lld fluence = %g\n\n",
                       current_case,source->getFluence());
        egsInformation(" Average sampled energy Eave = %g \n\n",Eave/EGS_Float(current_case));

        egsInformation("%-25s       Cavity kerma[Gy]  ","Geometry");
        egsInformation("\n-----------------------------------------------\n");
        char c = '%';
        for (int j=0; j<ngeom; j++) {
            double r,dr;
            kerma->currentResult(j,r,dr);
            if (r > 0) {
                dr = 100*dr/r;
            }
            else {
                dr = 100;
            }
            EGS_Float norm = 1.602e-10*current_case/source->getFluence();
            norm /= mass[j];
            egsInformation("%-25s %14.8le +/- %-10.6lf%c \n",
                           geoms[j]->getName().c_str(),
                           r*norm,dr,c);
        }
        egsInformation("\n\n");
        if (ncg > 0) {
            egsInformation("%-20s %-20s    KERMA ratio\n","Geometry 1", "Geometry 2");

            vector<double> ratio, dratio;

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
                    double r = r1*mass[gind2[j]]/(r2*mass[gind1[j]]);
                    egsInformation("%-20s %-20s     %-11.8lg +/- %-10.8lg [%-10.6lf%c]\n",
                                   geoms[gind1[j]]->getName().c_str(),
                                   geoms[gind2[j]]->getName().c_str(),r,r*dr,100.*dr,c);
                    ratio.push_back(r);
                    dratio.push_back(r*dr);
                }
                else {
                    egsInformation("zero dose\n");
                }
            }

        }
        if (flug) {
            string spe_name = constructIOFileName(".agr",true);
            //string spe_name = output_file + ".agr";
            ofstream spe_output(spe_name.c_str());
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

            egsInformation("\n\nPhoton fluence\n"
                           "=============================\n");
            for (int j=0; j<ngeom; j++) {
                double norm = current_case/source->getFluence();//per particle
                norm /= (mass[j]/rho_cav);               //per unit volume
                norm *= flu_a;                           //per unit bin width

                egsInformation("\nGeometry %s :",geoms[j]->getName().c_str());
                spe_output<<"@    s"<<j<<" errorbar linestyle 0\n";
                spe_output<<"@    s"<<j<<" legend \""<<
                          geoms[j]->getName().c_str()<<"\"\n";
                spe_output<<"@target G0.S"<<j<<"\n";
                spe_output<<"@type xydy\n";
                double fe,dfe,fp,dfp;
                flugT->currentResult(j,fe,dfe);
                if (fe > 0) {
                    dfe = 100*dfe/fe;
                }
                else {
                    dfe = 100;
                }
                egsInformation(" total fluence [cm-2] = %10.4le +/- %-7.3lf\%\n\n",
                               fe*norm/flu_a,dfe);
                egsInformation("   Emid/MeV    Flu/(MeV*cm2)   DFlu/(MeV*cm2)\n"
                               "---------------------------------------------\n");
                for (int i=0; i<flu_nbin; i++) {
                    flug[j]->currentResult(i,fe,dfe);
                    EGS_Float e = (i+0.5-flu_b)/flu_a;
                    if (flu_s) {
                        e = exp(e);
                    }
                    spe_output<<e<<" "<<fe *norm<<" "<<dfe *norm<< "\n";
                    egsInformation("%11.6f  %14.6e  %14.6e\n",
                                   e,fe*norm,dfe*norm);
                }
                spe_output << "&\n";
            }
        }

    };

    /*! Get the current simulation result.  */
    void getCurrentResult(double &sum, double &sum2, double &norm,
                          double &count) {
        count = current_case;
        double flu = source->getFluence();
        norm = flu > 0 ? 1.602e-10*count/(flu*mass[0]) : 0;
        kerma->currentScore(0,sum,sum2);
    };

    /* Select photon mean-free-path */
    void selectPhotonMFP(EGS_Float &dpmfp) {
        int np = the_stack->np-1;
        EGS_Vector x(the_stack->x[np],the_stack->y[np],the_stack->z[np]);
        EGS_Vector u(the_stack->u[np],the_stack->v[np],the_stack->w[np]);
        int ireg   = the_stack->ir[np]-2, newmed = geometry->medium(ireg);
        EGS_Float tstep = 1e35;
        //******************************************************************
        // FD Track-length kerma estimation for photons entering or aimed
        // at the cavity. It requires a cavity geometry and that the photon
        // has not touched an exclusion region and is not inside the geometry
        // (latch = 0)
        //******************************************************************
        if (cgeom && !the_stack->latch[np] &&
                (is_cavity[ig][ireg] || cgeom->howfar(-1,x,u,tstep,&newmed)>= 0)) {
            /* Photon at or aimed at cavity */
            int errK = scoreInCavity();
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
            if (flug) {
                for (int j=0; j<ngeom; j++) {
                    flug[j]->setHistory(current_case);
                }
                flugT->setHistory(current_case);
            }
            last_case = current_case;
        }
        return 0;
    };

private:

    int              ngeom;             // number of geometries to calculate quantities of interest
    int              ig;                // current geometry index

    int              ncg;               // number of correlated geometry pairs.
    int              *gind1,
                     *gind2;            // indices of correlated geometries

    double           *scg;              // sum(kerma(gind1[j])*kerma(gind2[j]);

    EGS_BaseGeometry **geoms;           // geometries for which to calculate the quantites of interest.
    EGS_AffineTransform **transforms;   // transformations to apply before transporting for each geometry
    bool             **is_cavity;       // array of flags for each region in each geometry, which is true if the region belongs to the cavity and false otherwise

    bool             **is_excluded;     // array of flags for each region in each geometry, which is true if the region is excludedd, false otherwise

    EGS_ScoringArray *kerma;            // kerma scoring array

    EGS_Interpolator *E_Muen_Rho;


    /****************************************************************/

    EGS_ScoringArray **flug;            // photon fluence
    EGS_ScoringArray *flugT;            // total photon fluence
    EGS_Float       flu_a,
                    flu_b,
                    flu_xmin,
                    flu_xmax,
                    rho_cav;            // cavity mass density
    int             flu_s,
                    flu_nbin;
    EGS_Float       *mass;              // mass of the material in the cavity.

    /*! Cavity bounding geometry.
      If no cavity bounding geometry is defined, range-rejection of RR
      is used only on a region-by-region basis. If a cavity bounding geometry
      is defined, then tperp to that geometry is also checked and if greater
      than the electron range, range-rejection or RR is done.
     */
    EGS_BaseGeometry *cgeom;

    static string revision;
};

string EGS_KermaApplication::revision = "$Revision: 1.0 $";

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
        vector<int *>  cavity_regions;
        vector<int>  n_cavity_regions;
        vector<int *>  excluded_regions;
        vector<int>  n_excluded_regions;
        vector<EGS_Float> cavity_masses;
        vector<EGS_AffineTransform *> transformations;
        EGS_Input *aux;
        EGS_BaseGeometry::setActiveGeometryList(app_index);
        while ((aux = options->takeInputItem("calculation geometry"))) {
            string gname;
            int err = aux->getInput("geometry name",gname);
            vector<int> cav;
            int err1 = aux->getInput("cavity regions",cav);
            vector<int> apert;
            int err4 = aux->getInput("excluded regions",apert);
            EGS_Float cmass;
            int err2 = aux->getInput("cavity mass",cmass);
            if (err) egsWarning("initScoring: missing/wrong 'geometry name' "
                                    "input\n");
            if (err1) egsWarning("initScoring: missing/wrong 'cavity regions' "
                                     "input\n");
            if (err2) {
                egsWarning("initScoring: missing/wrong 'cavity mass' "
                           "input\n");
                cmass = -1;
            }

            if (err4) {
                egsWarning("\n\n*** Geometry %s : Error reading excluded regions or not found.\n"
                           "                 No region excluded from scoring\n\n",gname.c_str());
            }
            if (err || err1) {
                egsWarning("  --> input ignored\n");
            }
            else {
                EGS_BaseGeometry::setActiveGeometryList(app_index);
                EGS_BaseGeometry *g = EGS_BaseGeometry::getGeometry(gname);
                if (!g) egsWarning("initScoring: no geometry named %s -->"
                                       " input ignored\n",gname.c_str());
                else {
                    int nreg = g->regions();
                    int *regs = new int [cav.size()];
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
                    if (!ncav) {
                        egsWarning("initScoring: no cavity regions "
                                   "specified for geometry %s --> input ignored\n",
                                   gname.c_str());
                        delete [] regs;
                    }
                    else {
                        geometries.push_back(g);
                        n_cavity_regions.push_back(ncav);
                        cavity_regions.push_back(regs);
                        cavity_masses.push_back(cmass);
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
            }
            delete aux;
        }
        ngeom = geometries.size();
        if (!ngeom) {
            egsWarning("initScoring: no calculation geometries defined\n");
            return 1;
        }
        geoms = new EGS_BaseGeometry* [ngeom];
        is_cavity   = new bool* [ngeom];
        is_excluded = new bool* [ngeom];
        mass = new EGS_Float [ngeom];
        kerma = new EGS_ScoringArray(ngeom);
        transforms = new EGS_AffineTransform* [ngeom];

        rho_cav = -1;
        for (int j=0; j<ngeom; j++) {
            geoms[j] = geometries[j]; //geoms[j]->ref();
            mass[j] = cavity_masses[j];
            transforms[j] = transformations[j];
            int nreg = geoms[j]->regions();
            is_cavity[j]   = new bool [nreg];
            is_excluded[j] = new bool [nreg];
            int i;
            for (i=0; i<nreg; i++) {
                is_cavity[j][i]   = false;
                is_excluded[j][i] = false;
            }
            int imed = -999;
            for (i=0; i<n_cavity_regions[j]; i++) {
                int ireg = cavity_regions[j][i];
                is_cavity[j][ireg] = true;
                if (imed == -999) {
                    imed = geoms[j]->medium(ireg);
                }
                else {
                    int imed1 = geoms[j]->medium(ireg);
                    if (imed1 != imed) egsWarning("initScoring: different "
                                                      "medium %d in region %d compared to medium %d in "
                                                      "region %d. Hope you know what you are doing\n",
                                                      imed1,ireg,imed,cavity_regions[j][0]);
                }
            }
            delete [] cavity_regions[j];

            /* Get cavity mass density */
            if (rho_cav < 0) {
                rho_cav = the_media->rho[imed];
            }
            else {
                EGS_Float rho_cav_new = the_media->rho[imed];
                if (rho_cav != rho_cav_new) egsWarning("initScoring:\n"
                                                           "density of cavity medium in geometry %s is %g g/cm3\n"
                                                           "which differs from initial cavity medium density %g g/cm3",
                                                           geoms[j]->getName().c_str(), rho_cav_new, rho_cav);

            }

            //if (n_excluded_regions.size()>0){
            if (!n_excluded_regions.empty()) {
                for (i=0; i<n_excluded_regions[j]; i++) {
                    int areg = excluded_regions[j][i];
                    is_excluded[j][areg] = true;
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
                flugT = new EGS_ScoringArray(ngeom);
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
                egsInformation("\n\n******* Fluence scoring input"
                               " errors: %d %d %d\n",er1,er2,er3);
                egsInformation("            => no fluence scoring\n\n");
            }
            delete aux;
        }

        string muen_file;
        int errKmuen = options->getInput("emuen file",muen_file);
        if (errKmuen) {
            errKmuen = options->getInput("muen file",muen_file);
            if (errKmuen)
                egsFatal(
                    "\n\n***  Wrong/missing 'emuen file' input for a "
                    "Kerma calculation\n    This is a fatal error\n\n");
        }
        ifstream muen_data(muen_file.c_str());
        if (!muen_data) {
            egsFatal(
                "\n\n***  Failed to open emuen file %s\n"
                "     This is a fatal error\n",muen_file.c_str());
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
        string cavity_geometry;
        int errCavGeom = options->getInput("cavity geometry",cavity_geometry);
        if (!errCavGeom) {
            EGS_BaseGeometry::setActiveGeometryList(app_index);
            cgeom = EGS_BaseGeometry::getGeometry(cavity_geometry);
            if (!cgeom) {
                egsWarning("\n\n********** No geometry named"
                           " %s exists! \nThis is required for forced detection track-length Kerma estimation!\n",
                           cavity_geometry.c_str());
            }
        }
        else {
            egsWarning("\n\n********** No cavity geometry entry found!\n"
                       " This is required for track-length Kerma estimation using forced detection!\n");
        }
        /* No ausgab call if using forced detection */
//         for(int call=BeforeTransport; call<=UnknownCall; ++call)
//             setAusgabCall((AusgabCall)call,false);
//         if (!cgeom)
        /* Ausgab call before trasnporting particle for analog kerma scoring */
//             setAusgabCall(BeforeTransport,true);
        /* ******************************************************
         * Enable all ausgab calls for energy deposition scoring
         *
         * Only possible for one calculation geometry. Implement
         * dose scoring in the ausgab routine if needed for all
         * geometries.
         *
         *********************************************************/
//         if (ngeom == 1){
//            vector<string> allow_dose;
//            allow_dose.push_back("no"); allow_dose.push_back("yes");
//            int score_dose = options->getInput("allow dose scoring",allow_dose,0);
//            if( score_dose )
//              for(int call=BeforeTransport; call<=ExtraEnergy; ++call)
//                  setAusgabCall((AusgabCall)call,true);
//         }

        delete options;
    }
    else {
        egsWarning("\n\n*********** no 'scoring options' input *********\n\n");
        return 2;
    }

    return 0;
}

void EGS_KermaApplication::describeSimulation() {
    EGS_AdvancedApplication::describeSimulation();
    egsInformation("**********************************************\n"
                   "   Volumetric Track-length Kerma estimation \n"
                   "**********************************************\n\n");
    if (cgeom)
        egsInformation("---> Scoring using forced detection (FD)\n"
                       "     for photons aimed at geometry %s\n\n",
                       cgeom->getName().c_str());
    else
        egsInformation("---> Scoring only when photon enters volume\n"
                       "     including those scattered inside the geometry.\n"
                       "     Fluence is equivalent to FLURZ total fluence!\n\n");
    egsInformation("\n\n");
    for (int j=0; j<ngeom; j++) {
        egsInformation("Calculation geometry: %s\n",
                       geoms[j]->getName().c_str());
        geoms[j]->printInfo();
        for (int i=0; i<geoms[j]->regions(); i++) {
            if (is_cavity[j][i]) {
                egsInformation("  cavity region %d, medium = %d\n\n",
                               i,geoms[j]->medium(i));
            }
        }
    }
    for (int i=0; i<ngeom; i++) {
        egsInformation("excluded regions for geometry %s :",
                       geoms[i]->getName().c_str());
        int nexcl = 0;
        for (int j=0; j<geoms[i]->regions(); j++) {
            if (is_excluded[i][j]) {
                egsInformation(" %d",j);
                nexcl++;
            }
        }
        if (!nexcl) {
            egsInformation(" NONE");
        }
        egsInformation("\n");
    }
}

#ifdef BUILD_APP_LIB
APP_LIB(EGS_KermaApplication);
#else
APP_MAIN(EGS_KermaApplication);
#endif

