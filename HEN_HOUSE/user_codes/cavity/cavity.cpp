/*
###############################################################################
#
#  EGSnrc egs++ cavity application
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
#  Contributors:    Ernesto Mainegra-Hing
#                   Reid Townson
#                   Blake Walters
#
###############################################################################
#
#  cavity is an advanced EGSnrc application using the C++ interface. It
#  implements most of the functionality of the cavrznrc user code written in
#  mortran except that now, due to the use of the general geometry and source
#  packages included in egspp, any geometry or any source can be used in the
#  simulation.
#
#
#  TERMINOLOGY
#  -----------
#
#  Simulations are split into 'chunks'. For simple simulations (no parallel
#  runs, etc.) there is a single simulation chunk with the number of
#  histories specified in the input file. For parallel runs the number of
#  chunks and number of histories per chunk are determined by a 'run control
#  object' (see below).
#
#  Each simulation chunk is split into 'batches'. The batches are not required
#  for statistical analysis (by using the provided scoring classes it is easy
#  to have a history-by-history uncertainty estimation). Instead, simulation
#  chunks are split into batches so that the progress of the simulation can be
#  reported after the completion of a batch and the current results can be
#  stored into a data file. By default there are 10 batches per simulation chunk
#  but this can be changed in the input file.
#
#  The simulation is controlled via a 'run control object' (RCO) The purpose
#  of the run control object is to give to the shower loop the number of
#  histories per simulation chunk, number of batches per chunk and to possibly
#  terminate the simulation prematurely if certain conditions are met (e.g.
#  maximum cpu time allowed is exceeded or the required uncertainty has been
#  reached).
#
#  egs++ provides 2 run control objects:
#
#  1)  simple:  the simple RCO always uses a single simulation chunk.
#
#  2)  JCF:     a JCF object is used by default for parallel runs
#               JCF stands for Job Control File as this type of object
#               uses a file placed in the user code directory to record
#               the number of histories remaining, the number of jobs
#               running, etc., in parallel runs. This is explained in
#               more details in PIRS-877. A JCF object uses by default
#               10 simulation chunks but this can be changed in the
#               input file.
#
#  It is possible to use a simple control object for parallel runs by giving
#  the -s or --simple command line option. In this case, each parallel job
#  will run the number of histories specified in the input file but
#  automatically adjust the initial random number seed(s) with the job index.
#  This additional possibility has been implemented because several users have
#  reported problems with file locking needed for a JCF run control object.
#
#  It is also possible to have other RCO's compiled into shared libraries and
#  automatically loaded at run time (e.g., one could implement a RCO that
#  communicates via TCP/IP with a remote server to obtain the number of
#  histories in the next simulation chunk).
#
#
#  USAGE
#  -----
#
#  - Geometry and particle source are specified in an input file as explained
#    in PIRS-899 and PIRS-898.
#
#  - Run control is specified in a section of the input file delimited by
#    :start run control: and :stop run control: labels.
#
#  - A simple RCO is used for single job runs.
#
#  - A JCF RCO is used by default for parallel runs, unless -s or --simple
#    is specified on the command line.
#
#  - A simple RCO understands the following keys:
#    ncase                       = number of histories to run
#    nbatch                      = number of batches to use
#    statistical accuracy sought = required uncertainty, in %
#    max cpu hours allowed       = max. processor time allowed
#    calculation                 = first | restart | combine | analyze
#
#    All inputs except for ncase are optional (a missing ncase key will result
#    in a simulation with 0 particles).
#
#  - A JCF object understands all the above keys plus
#    nchunk = number of simulation chunks
#
#  - Input related to scoring is between :start scoring options: and
#    :stop scoring options: labels.
#
#  - The following calculation types are recognized:
#    calculation type = Dose | Awall | Fano | HVL | FAC
#
#    When type = Dose, cavity calculates the dose to the cavity
#
#    When type = Awall, cavity also calculates the Awall correction factor
#
#    When type = Fano, scattered photons are thrown away and the attenuation
#    of primary photons is unweighted so that, apart for possible differences
#    in photon scattering properties between the materials, there is a uniform
#    source of electrons throughout the geometry.
#
#    When type = HVL, kerma at an user-defined circular field is calculated by
#    using an user-provided data file with E*muen/rho values.
#
#    When type = FAC, Free Air Chamber correction factors are calculated.
#    These include Awall, Aatt, Ascat, Aap, Aeloss and Ag. They correct for
#    the following processes affecting the cavity dose:
#
#       Aap    => removes contribution from FAC aperture to cavity dose
#       Ascat  => removes scatter contribution to cavity dose
#       Aatt   => removes photon attenuation
#       Aeloss => electron loss correction
#       Ag     => geometry correction factor
#
#  - calculation geometries are specified in the scoring options input block
#    in the form:
#
#    :start calculation geometry:
#        geometry name  = name_of_a_previously_defined_geometry
#        cavity regions = list_of_cavity_region_indeces
#        cavity mass    = total cavity mass, in grams
#    :stop calculation geometry:
#
#    It is possible to have more than one geometry in the simulation (this is
#    handy, e.g., for the calculation of in-air profiles or dose to the
#    chamber for different locations in the phantom).
#
#  - The simulation is run using
#
#    cavity -i input_file -p pegs_file [-o output_file] [-s] [-P n -j i]
#
#    where command line arguments between [] are optional. The -P n option
#    specifies the number of parallel jobs n and -j i the index of this job.
#    On Linux/Unix systems it is more convenient to use the 'exb' script for
#    parallel job submission (see PIRS-877)
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

#ifdef USTEP_DEBUG
#define MXSTEP 10000
struct EGS_GeometryDebug {
    long long icase;
    float x_debug[MXSTEP],y_debug[MXSTEP],z_debug[MXSTEP];
    float u_debug[MXSTEP],v_debug[MXSTEP],w_debug[MXSTEP];
    float ustep_debug[MXSTEP],dnear_debug[MXSTEP];
    int   ir_debug[MXSTEP],irnew_debug[MXSTEP],np_debug[MXSTEP];
    int   nstep;
};

extern __extc__ struct EGS_GeometryDebug
                 F77_OBJ_(geometry_debug,GEOMETRY_DEBUG);
struct EGS_GeometryDebug* the_geometry_debug =
                &F77_OBJ_(geometry_debug,GEOMETRY_DEBUG);
#endif

#define calculatePhotonMFP F77_OBJ_(calculate_photon_mfp,CALCULATE_PHOTON_MFP)
extern __extc__ void calculatePhotonMFP(EGS_Float *,EGS_Float *);
#define doRayleigh F77_OBJ_(do_rayleigh,DO_RAYLEIGH)
extern __extc__ void doRayleigh();
#define calculatePhotonBranching F77_OBJ_(calculate_photon_branching,CALCULATE_PHOTON_BRANCHING)
extern __extc__ void calculatePhotonBranching(EGS_Float *gbr1,EGS_Float *gbr2);
extern __extc__ void F77_OBJ(pair,PAIR)();
extern __extc__ void F77_OBJ(compt,COMPT)();
extern __extc__ void F77_OBJ(photo,PHOTO)();

#ifdef GDEBUG
#define MAX_STEP 100000
extern EGS_Vector steps_x[MAX_STEP], steps_u[MAX_STEP];
extern int steps_ireg[MAX_STEP], steps_inew[MAX_STEP];
extern EGS_Float steps_ustepi[MAX_STEP], steps_ustepf[MAX_STEP];
extern int steps_n;
#endif

/**********************************************************************
EGS_HVL:
This class is inteded for calculating the HVL on the fly. Currently
it assumes that the kerma ratios absorber-to-air follow a linear
dependence, which the user should make sure applies. For low energies,
even picking absorber thicknesses which are very close to each other,
this assumption can be very wrong and a more complex expression should
be used for the fit.
TODO: obtain HVL from spectrum.
***********************************************************************/
class APP_EXPORT EGS_HVL {

public:
     /* Constructor */
     EGS_HVL();
     /* Constructor: default, HVL calculated from air-kerma ratios */
     EGS_HVL( vector<EGS_Float> thickness,
              vector<double> kr,
              vector<double> dkr);
     /* Constructor: HVL calculated from spectrum*/
     EGS_HVL( EGS_ScoringArray *flu,
              EGS_Float flu_a,
              EGS_Float flu_b,
              EGS_Interpolator *emuen,
              EGS_Interpolator *mu_air,
              EGS_Interpolator *mu_abs);
     /* Constructor: HVL can be calculated from ratios and spectrum*/
     EGS_HVL( vector<EGS_Float> thickness,
              vector<double> kr,
              vector<double> dkr,
              EGS_ScoringArray *flu,
              EGS_Float flu_a,
              EGS_Float flu_b,
              EGS_Interpolator *emuen,
              EGS_Interpolator *mu_air,
              EGS_Interpolator *mu_abs);
     /* Destructor */
    ~EGS_HVL();
     /* Least squares fit to a straight line */
     void leastSquaresFit();
     /* Recursive iteration to obtain HVL */
     void recursiveIteration();
     /* Get HVL and its error from a straight line fit*/
     void getHVL(double m, double dm, double b, double db);
     /* Print HVL, its error and the fit parameters */
     void printFittedHVL();
     void printCovarianceMatrix();

private:

    /* Input data */
    vector<EGS_Float> x;    //absorber thicknesses
    vector<double>    y, dy;//kerma ratios and their errors
    EGS_ScoringArray *flu;  //photon spectrum

    /* Data */
    EGS_Interpolator *emuen;       //E*muen for air
    EGS_Interpolator *mu_air;      //mass attenuation for air
    EGS_Interpolator *mu_abs;      //mass attenuation for absorber (Cu, Al)

    /* Fit results */
    double hvl_fit, dhvl_fit;//linear fit HVL and its error
    double hvl_rec, dhvl_rec;//recursive HVL and its error
    double **C;              //least squares covariance matrix
    double b, db, m, dm;     //straight line y-shift and slope and their errors
    int npoints;             //number of kerma ratios

};



class APP_EXPORT Cavity_Application : public EGS_AdvancedApplication {

public:

    /*! Calculation type */
    enum Type { Dose=0, Awall=1, Fano=2, HVL=3, FAC=4 };
    enum eFluType { flurz=0, stpwr=1, stpwrO5=2 };

    /*! Constructor */
    Cavity_Application(int argc, char **argv) :
        EGS_AdvancedApplication(argc,argv), type(Dose), ngeom(0), dose(0),
        dose1(0),dose2(0),dose3(0),dose4(0), dose5(0),
        aperture_scores(0),aperture_hits(0),reject(0),
        ideal_dose(0), fsplit(1), fspliti(1), rr_flag(0), Esave(0), rho_rr(1),
        cgeom(0), nsmall_step(0), ncg(0), score_q(0), cavity_medium(0),
        flug(0),flugT(0),flum(0), flumT(0), flup(0), flupT(0),flu_s(0),
        flu_stpwr(stpwr), Rho(0) {};

    /*! Destructor.  */
    ~Cavity_Application() {
        if( dose )  delete dose; if( dose1 ) delete dose1;
        if( dose2 ) delete dose2;if( dose3 ) delete dose3;
        if( dose4 ) delete dose4;if( dose5 ) delete dose5;
        if( ideal_dose ) delete ideal_dose;
        if( ngeom > 0 ) {
            if( flup ) {
                for(int j=0; j<ngeom; j++) {
                   delete flup[j]; delete flum[j];
                }
                delete [] flup; delete [] flum;
                if( flumT ) delete flumT;
                if( flupT ) delete flupT;
            }
            if ( Rho ) delete [] Rho;
            if( flug ) {
                for(int j=0; j<ngeom; j++) {delete flug[j];}
                delete [] flug;
                if( flugT ) delete flugT;
            }
            delete [] geoms; delete [] mass; int j;
            for(j=0; j<ngeom; j++) if( transforms[j] ) delete transforms[j];
            delete [] transforms;
            if( type == Awall || type == FAC ) delete [] corr;
            for(j=0; j<ngeom; j++) delete [] is_cavity[j];
            delete [] is_cavity;
            for(j=0; j<ngeom; j++) delete [] is_aperture[j];
            delete [] is_aperture;

            if( type == FAC ){
              delete [] scd1; delete [] scd2;
              delete [] scd3; delete [] scd4;
              delete [] scd5; delete [] scd0; delete [] scd13;
              if( ncg > 0 ) delete [] scg5;
            }
        }
        if( ncg > 0 ) {
            delete [] gind1; delete [] gind2; delete [] scg;
        }
    };

    void startNewParticle() {
        EGS_AdvancedApplication::startNewParticle();
        nsmall_step = 0;
    };

    /*! Describe the application.  */
    void describeUserCode() const {
        egsInformation(
          "\n               *************************************************"
          "\n               *                                               *"
          "\n               *                  cavity                       *"
          "\n               *                                               *"
          "\n               *************************************************"
          "\n\n");
        egsInformation("This is Cavity_Application %s based on\n"
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
        int np = the_stack->np-1;
        //
        //  **** energy deposition
        //
        if( iarg <= ExtraEnergy ) {
            int ir = the_stack->ir[np]-2;
#ifdef GDEBUG
            if( steps_n < MAX_STEP ) {
                steps_ustepi[steps_n] = the_epcont->vstep;
                steps_x[steps_n] = EGS_Vector(the_stack->u[np],
                        the_stack->v[np],the_stack->w[np]);
                steps_u[steps_n] = EGS_Vector(the_stack->x[np],
                        the_stack->y[np],the_stack->z[np]);
                steps_ireg[steps_n] = ir;
                steps_inew[steps_n] = the_epcont->irnew-2;
                steps_ustepf[steps_n++] = the_epcont->tstep;
            }
#endif
            if( the_epcont->ustep < 1e-5 ) {
                if( ++nsmall_step > 10000 ) {
                    egsWarning("Too many small steps: ir=%d x=(%g,%g,%g)\n",
                        ir,the_stack->x[np],the_stack->y[np],
                        the_stack->z[np]);
                    the_stack->wt[np] = 0;
                    nsmall_step = 0;
                }
            }
            else nsmall_step = 0;
            if( ir >= 0 && is_cavity[ig][ir] ) {
                EGS_Float aux = the_epcont->edep*the_stack->wt[np];
                if( aux > 0 ) {
                    if( type != Fano ) dose->score(ig,aux);
                    else dose->score(ig,aux*expmfp[np]);

                    if( (type == Awall || type == FAC) &&
                          the_stack->latch[np] == 0 ) {//unattenuated primary dose
                        ideal_dose->score(ig,aux* expmfp[np]);//everywhere
                        if (type == FAC)
                         dose3->score(     ig,aux*kexpmfp[np]);//beyond POM
                    }

                    if( type == FAC ) {
                     if( the_stack->latch[np] >= 0 ){ // hasn't been in aperture
                        dose1->score(ig,aux);
                     }
                     else{ aperture_scores++; }

                     if(the_stack->latch[np] == 0 ||   // primary particle
                       (the_stack->latch[np]>0 && latchr[np]==1)
                     ){                                // primary interaction
                                                       // rejected
                        dose2->score(ig,aux);         // unscattered
                     }
                    }
                }
                //**************************************************
                // ***     Charged particle fluence scoring     ****
                //**************************************************
                /* Based on energy loss during a CH step where a particle
                 * slows down from initial energy Eb to final energy Ee,
                 * one can determine the corresponding path length segment
                 * at each energy bin between Eb and Ee. First and last
                 * energy bins might not be fully covered, hence the
                 * fraction of energy lost corresponding to these bins
                 * must be determined.
                 * Two different approaches are used: Approach A accounts
                 * for stopping power variation along the particle's step
                 * while approach B assumes no change in the stopping
                 * power (used in FLURZnrc) and is about 8% faster than
                 * approach A for a 5 MeV e- source and ESTEPE = 0.25.
                 * However approach B overestimates the fluence at low
                 * energies where the stopping powers increase significantly
                 * with decreasing energy. This artifacts can be eliminated
                 * by reducing ESTEPE to 0.01 which increases CPU time by
                 * almost a factor of 10.
                 *
                 * - Differential fluence computed per energy bin-width.
                 * - Total fluence also scored
                 *
                 *****************************************************/
                 if( flum &&
                    ( iarg == BeforeTransport || iarg == UserDiscard ) &&
                     the_epcont->edep && the_stack->iq[np] ){
                    /***** Initialization *****/
                    EGS_Float Eb = the_stack->E[np] - the_useful->prm,
                              Ee = Eb-the_epcont->edep,
                              weight = the_stack->wt[np];
                    EGS_Float xb, xe;
                    /*********************************/
                    if( flu_s ) {
                        xb = log(Eb);
                        if( Ee > 0 )
                            xe = log(Ee);
                        else xe = -15;
                    }
                    else{
                      xb = Eb; xe = Ee;
                    }
                    EGS_Float ab, ae; int jb, je;
                    if( xb > flu_xmin && xe < flu_xmax ) {
                        /* Fraction of the initial bin covered */
                        if( xb < flu_xmax ) {
                            ab = flu_a*xb + flu_b; jb = (int) ab;
                            /* Variable bin-width for log scale*/
                            if (flu_s){
                               ab = (Eb*a_const[jb]-1)*r_const;
                            }
                            else{ ab -= jb;}// particle's energy above Emax
                        }
                        else { ab = 1; jb = flu_nbin - 1; }
                        /* Fraction of the final bin covered */
                        if( xe > flu_xmin ) {
                            ae = flu_a*xe + flu_b; je = (int) ae;
                            /* Variable bin-width for log scale*/
                            if (flu_s){
                               ae = (Ee*a_const[je]-1)*r_const;
                            }
                            else{ ae -= je; }
                        }
                        else { ae = 0; je = 0; }// extends below Emin
                        EGS_ScoringArray *aux = the_stack->iq[np] == -1 ?
                                                flum[ig] : flup[ig];
                        EGS_ScoringArray *auxT = the_stack->iq[np] == -1 ?
                                                flumT : flupT;
                        /************************************************
                         * Approach A:
                         * -----------
                         * Uses either an O(3) or O(5) series expansion of the
                         * integral of the inverse of the stopping power with
                         * respect to energy. The stopping power is represented
                         * as a linear interpolation over a log energy grid. It
                         * accounts for stopping power variation along the particle's
                         * step within the resolution of the scoring array. This
                         * is more accurate than the method used in FLURZnrc albeit
                         * about 10% slower in electron beam cases.
                         *
                         * BEWARE: For this approach to work, no range rejection
                         * ------  nor Russian Roulette should be used.
                         *
                         ************************************************/
                        if (flu_stpwr){
                           int imed = geometry->medium(ir);
                           // Initial and final energies in same bin
                           EGS_Float step;
                           if( jb == je ){
                               step = weight*(ab-ae)*getStepPerFraction(imed,xb,xe);
                               aux->score(jb,step);
                               if (flu_s)
                                  auxT->score(ig,step*DE[jb]);
                               else
                                  auxT->score(ig,step);
                           }
                           else {
                               EGS_Float flu_a_i = 1/flu_a;
                               // First bin
                               Ee = flu_xmin+jb*flu_a_i; Eb=xb;
                               step = weight*ab*getStepPerFraction(imed,Eb,Ee);
                               aux->score(jb,step);
                               if (flu_s)
                                  auxT->score(ig,step*DE[jb]);
                               else
                                  auxT->score(ig,step);
                               // Last bin
                               Ee = xe; Eb = flu_xmin+(je+1)*flu_a_i;
                               step = weight*(1-ae)*getStepPerFraction(imed,Eb,Ee);
                               aux->score(je,step);
                               if (flu_s)
                                  auxT->score(ig,step*DE[je]);
                               else
                                  auxT->score(ig,step);
                               // intermediate bins
                               for(int j=je+1; j<jb; j++){
                                  if (flu_stpwr == stpwrO5){
                                    Ee = Eb; Eb = flu_xmin+(j+1)*flu_a_i;
                                   /* O(eps^5) would require more pre-computed values
                                    * than just 1/Lmid. One requires lnEmid[i] to get
                                    * the b parameter and eps[i]=1-E[i]/E[i+1]. Not
                                    * impossible, but seems unnecessary considering
                                    * the excellent agreement with O(eps^3), which
                                    * should be always used.
                                    */
                                    step = weight*getStepPerFraction(imed,Eb,Ee);
                                  }
                                  else{// use pre-computed values of 1/Lmid
                                   step = weight*Lmid_i[j+ig*flu_nbin];
                                  }
                                  aux->score(j,step);
                                  if (flu_s)
                                    auxT->score(ig,step*DE[j]);
                                  else
                                    auxT->score(ig,step);
                               }
                           }
                        }
                        /***************************************************
                         * -----------------------
                         * Approach B (FLURZnrc):
                         * ----------------------
                         * Path length at each energy interval from energy
                         * deposited edep and total particle step tvstep. It
                         * assumes stopping power constancy along the particle's
                         * step. It might introduce artifacts if ESTEPE or the
                         * scoring bin width are too large.
                         *
                         * BEWARE: For this approach to work, no range rejection
                         * ------  nor Russian Roulette should be used.
                         **************************************************/
                        else{
                           EGS_Float step, wtstep = weight*the_epcont->tvstep/the_epcont->edep;
                           // Initial and final energies in same bin
                           if( jb == je ){
                             step = wtstep*(ab-ae);
                             aux->score(jb,step);
                             if (flu_s)
                               auxT->score(ig,step*DE[jb]);
                             else
                               auxT->score(ig,step);
                           }
                           else {
                               // First bin
                               step = wtstep*ab;
                               aux->score(jb,step);
                               if (flu_s)
                                 auxT->score(ig,step*DE[jb]);
                               else
                                 auxT->score(ig,step);
                               // Last bin
                               step = wtstep*(1-ae);
                               aux->score(je,step);
                               if (flu_s)
                                 auxT->score(ig,step*DE[je]);
                               else
                                 auxT->score(ig,step);
                               // intermediate bins
                               for(int j=je+1; j<jb; j++){
                                   aux->score(j,wtstep);
                                   if (flu_s)
                                     auxT->score(ig,wtstep*DE[j]);
                                   else
                                     auxT->score(ig,wtstep);
                               }
                           }
                        }
                    }
                }
            }
            //************************************************************************
            // *** check whether entering an aperture zone and flag the particles by
            //     making latch < 0 if not already done. Scattered particles going
            //     through the aperture are considered part of the aperture contribution
            //     not the contribution from scattered particles.
            //     Only for FAC calculations
            //
            //     latch = -rr_flag-1 => descendants of RR survivors go through aperture
            //     latch = -rr_flag   => electron surviving RR goes through aperture
            //     latch = -1         => particle goes through aperture
            //     latch =  0         => primary particle
            //     latch =  1         => scattered particle
            //     latch = rr_flag    => electron survived Russian Roulette
            //     latch = rr_flag+1  => descendants of particle surviving RR
            //
            //************************************************************************
            if( iarg == BeforeTransport && ir >= 0  ) {
             int latch = the_stack->latch[np];
             if( is_aperture[ig][ir]    &&
                 type  == FAC           &&
                 latch >=0              ){
                the_stack->latch[np] = latch > 0 ? -latch : -1;
                aperture_hits++;
             }
            }
            //************************************************************************
            return 0;
        }

        //
        // *** charge scoring
        //
        if( iarg == AfterTransport ) {
            if( do_charge[ig] && the_stack->iq[np] &&
                the_epcont->irnew != the_epcont->irold ) {
                int iold = the_epcont->irold-2, inew = the_epcont->irnew-2;
                if( iold >= 0 && is_charge[ig][iold] ) {
                    /*
                    egsInformation("scoring charge exit: ir=%d iq=%d "
                       "x=(%g,%g,%g)\n",the_epcont->irold-2,-the_stack->iq[np],
                       the_stack->x[np],the_stack->y[np],the_stack->z[np]);
                    */
                    score_q->score(ig,-the_stack->wt[np]*the_stack->iq[np]);
                }
                if( inew >= 0 && is_charge[ig][inew] ) {
                    /*
                    egsInformation("scoring charge enter: ir=%d iq=%d "
                       "x=(%g,%g,%g)\n",the_epcont->irnew-2,the_stack->iq[np],
                       the_stack->x[np],the_stack->y[np],the_stack->z[np]);
                    */
                    score_q->score(ig, the_stack->wt[np]*the_stack->iq[np]);
                }
            }
            return 0;
        }

        //
        //  **** mark or throw away scattered photons
        //
        if( iarg == AfterBrems     || iarg == AfterMoller      ||
            iarg == AfterBhabha    || iarg == AfterAnnihFlight ||
            iarg == AfterAnnihRest || iarg == AfterRayleigh    ||
            iarg == AfterCompton   || iarg == AfterPhoto       ){
            for(int ip=the_stack->npold-1; ip<=np; ip++) {
                if( !the_stack->iq[ip] ) {
                    if( type == Fano ) the_stack->wt[ip] = 0;
                    else {
                        if( fsplit > 1 && abs(the_stack->latch[ip]) < 2 ) {
                            if( rndm->getUniform()*fsplit < 1 )
                                the_stack->wt[ip] *= fsplit;
                            else
                                the_stack->wt[ip] = 0;
                        }
                        //=====
                        //NOTE:
                        //=====
                        //latch = -1: primary/scattered was in aperture
                        //is not  being modified here, should we?
                        //answer: NO, they will be accounted for in the
                        //        aperture correction.
                        if( the_stack->latch[ip] == 0 ||
                           (rr_flag > 1 && abs(the_stack->latch[ip]) == rr_flag) )
                            the_stack->latch[ip] +=
                                (the_stack->latch[ip] < 0 ?  -1 : 1) ;
                        // after a real photon interaction, latchr must be 0
                        latchr[ip] = 0;
                    }
                }
            }
            return 0;
        }
        return 0;
    };
    /*! Computes path per energy bin-width traveled by a charged particle when
     *  slowing down from Eb to Ee.
     *
     * Computes the path-length traveled while slowing down from energy Eb to energy
     * Ee, both energies falling in the same energy bin assuming full coverage.
     * The returned value should be multiplied by the actual fraction of the
     * energy bin covered by Eb-Ee.
     * If using a logarithmic energy interpolation, Eb and Ee are actually the
     * logarithms of the initial and final energies. The expression is based on
     * linear interpolation in a logarithmic energy grid as used in EGSnrc
     * (i.e. dedx = a + b*log(E) ) and a power series expansion of the ExpIntegralEi
     * function that is the result of the integration of the inverse of the stopping
     * power with respect to energy.
     */
    inline EGS_Float getStepPerFraction( const int & imed,
                                      const EGS_Float & Eb,
                                      const EGS_Float & Ee){
        EGS_Float stpFrac, eps, lnEmid;
        if (flu_s){//Using log(E)
          eps     = 1 - exp(Ee-Eb);
          /* 4th order series expansion of log([Eb+Ee]/2) */
          lnEmid  = 0.5*(Eb+Ee+0.25*eps*eps*(1+eps*(1+0.875*eps)));
        }
        else{//Using E
          if (flu_stpwr == stpwrO5)
             eps  = 1 - Ee/Eb;
          lnEmid  = log(0.5*(Eb+Ee));
        }
        EGS_Float dedxmid_i = 1/i_ededx[imed].interpolate(lnEmid);
        /* O(eps^3) approach */
        if (flu_stpwr == stpwr) return dedxmid_i;
        /* O(eps^5) approach */
        EGS_Float b = i_ededx[imed].get_b(i_ededx[imed].getIndexFast(lnEmid));
        EGS_Float aux = b*dedxmid_i;
        aux = aux*(1+2*aux)*pow(eps/(2-eps),2)/6;
        stpFrac = dedxmid_i*(1+aux);
        return stpFrac;
    }

    /*! Simulate a single shower.
        We need to do special things and therefore reimplement this method.
     */
    int simulateSingleShower() {
        last_case = current_case;
        EGS_Vector x,u;
        current_case = source->getNextParticle(rndm,p.q,p.latch,p.E,p.wt,x,u);
        //egsInformation("particle: E=%g q=%d x=(%g,%g,%g)\n",p.E,p.q,x.x,x.y,x.z);
        int err = startNewShower(); if( err ) return err;
        EGS_BaseGeometry *save_geometry = geometry;
        EGS_Float arng;
        if( type == HVL ) arng = 1 - rndm->getUniform();
        for(ig=0; ig<ngeom; ig++) {
            geometry = geoms[ig]; p.x = x; p.u = u;
            if( transforms[ig] ) {
                transforms[ig]->transform(p.x); transforms[ig]->rotate(p.u);
            }
            int ireg = geometry->isWhere(p.x);
            if( ireg < 0 ) {
                EGS_Float t = 1e30; ireg = geometry->howfar(ireg,p.x,p.u,t);
                if( ireg >= 0 ) p.x += p.u*t;
            }
            if( ireg >= 0 ) {
                //egsInformation("entering: x=(%g,%g,%g) u=(%g,%g,%g) E=%g q=%d"
                //   " wt=%g latch=%d ir=%d\n",
                //p.x.x,p.x.y,p.x.z,p.u.x,p.u.y,p.u.z,
                //    p.E,p.q,p.wt,p.latch,ireg);
                p.ir = ireg;
                nsmall_step = 0;
                if( score_q ) {
                    if( do_charge[ig] ) {
                        if( p.q && is_charge[ig][ireg] )
                            score_q->score(ig,p.wt*p.q);
                        setAusgabCall(AfterTransport,true);
                    }
                }
                if( type == HVL ) hvl_first_rng = arng;
                if( type == FAC ) {
                  err = scorePrimaryKerma();
                  if ( err ) return err;
                }
                err = shower(); if( err ) return err;
                //setAusgabCall(AfterTransport,false);
            }
        }
        err = finishShower();
        geometry = save_geometry;
        return err;
    };

    /*! Score Air-Kerma at Point Of Measurement (POM) and Collecting Volume
        (CV)of primary photons through aperture opening.
        Photons touching aperture are immediately discarded.
     */
    int scorePrimaryKerma(){

     if( p.E < the_bounds->pcut ) {
         return 0;
     }

     EGS_Vector x( p.x.x, p.x.y, p.x.z );
     EGS_Vector u( p.u.x, p.u.y, p.u.z );
     EGS_Float xp = x*hvl_normal, up = u*hvl_normal;

     EGS_Float t = (hvl_d - xp)/up;
     EGS_Vector x1(x + u*t - hvl_midpoint);
     if( x1.length2() < hvl_R*hvl_R ) {
        EGS_Float tstep; int inew; int ireg = p.ir;
        int newmed = geometry->medium(ireg);
        int imed = -1; EGS_Float gmfp, sigma = 0, cohfac = 1;
        EGS_Float cohfac_int;
        EGS_Float gle = log(p.E);
        double Lambda = 0, ttot = 0;
        double    t_cav = 0;
        EGS_Float rho_cav;

       //==============================================================
       // Trace path to POM (point of measurement) along photon's
       // direction to score Air-Kerma at hvl_d (dose5) and in the
       // CV (collecting volume, dose4). Primary beam is attenuated
       // by the distance to the POM. Scoring done only for photons
       // going through the aperture's opening without touching
       // the aperture. As soon as a region is identified as aperture
       // region, scoring is interrupted. Therefore, when a photon
       // reaches the POM, one must score the Air-Kerma dose5 and then
       // continue on tracing the photon through the geometry to obtain
       // the path along the CV.
       //==============================================================
        EGS_Float exp_Lambda = 0;
        EGS_Float emuen_rho  = hvl_muen->interpolateFast(gle);
        EGS_Float aup = fabs(up); if( aup < 0.08 ) aup = 0.08;
        bool crossed_plane = false;
        while(1) {
            if (is_aperture[ig][ireg]) return 0;
            if( imed != newmed ) {
                imed = newmed;
                if( imed >= 0 ) {
                    gmfp = i_gmfp[imed].interpolateFast(gle);
                    if( the_xoptions->iraylr ) {
                        cohfac = i_cohe[imed].interpolateFast(gle);
                        gmfp *= cohfac;
                    }
                    sigma = 1/gmfp;
                } else { sigma = 0; cohfac = 1; }
            }
            tstep = 1e35;
            inew = geometry->howfar(ireg,x,u,tstep,&newmed);

            if( !crossed_plane ) {
                if( ttot + tstep >= t ) {//about to cross POM
                    crossed_plane = true; Lambda += (t - ttot)*sigma;
                    exp_Lambda = Lambda < 80 ? exp(-Lambda) : 0;
                    dose5->score(ig,p.wt/aup*exp_Lambda*emuen_rho);
                    //--------------------------------------------
                    // score photon fluence at POM if requested
                    //--------------------------------------------
                    if( flug ) {
                      EGS_Float e = p.E;
                      if( flu_s ) {e = log(e);}
                      EGS_Float ae; int je;
                      if( e > flu_xmin && e <= flu_xmax) {
                        ae = flu_a*e + flu_b; je = (int) ae;
                        EGS_ScoringArray *aux = flug[ig];
                        aux->score(je,p.wt/aup*exp_Lambda);
                        flugT->score(ig,p.wt/aup*exp_Lambda);
                      }
                    }
                }
                else Lambda += tstep*sigma;// keep track of path
            }

            if ( is_cavity[ig][ireg] ){//in cavity, get path through it
                t_cav   += tstep;
                rho_cav = the_media->rho[imed];
            }
            if( inew < 0 ) break; // outside geometry, stop and score
                                  // track-length estimation of kerma

            ireg = inew; x += u*tstep; ttot += tstep;
        }
        // score air-kerma at CV
        dose4->score(ig,p.wt*exp_Lambda*emuen_rho*rho_cav*t_cav);
       //==============================================================
     }
     return 0;
    }

    /*! Output intermediate results to the .egsdat file. */
    int outputData() {
        int err = EGS_AdvancedApplication::outputData();
        if( err ) return err;
        if( !dose->storeState(*data_out) ) return 101;
        if( type == Awall || type == FAC ) {
            if( !ideal_dose->storeState(*data_out) ) return 102;
            for(int j=0; j<ngeom; j++) (*data_out) << corr[j] << "  ";
            (*data_out) << endl;
            if( !data_out->good() ) return 103;
        }
        if( type == FAC ) {
            if( !dose1->storeState(*data_out) ) return 1021;
            if( !dose2->storeState(*data_out) ) return 1022;
            if( !dose3->storeState(*data_out) ) return 1023;
            if( !dose4->storeState(*data_out) ) return 1024;
            if( !dose5->storeState(*data_out) ) return 1025;
            for(int j=0; j<ngeom; j++) (*data_out) << scd0[j] << "  ";
            for(int j=0; j<ngeom; j++) (*data_out) << scd1[j] << "  ";
            for(int j=0; j<ngeom; j++) (*data_out) << scd2[j] << "  ";
            for(int j=0; j<ngeom; j++) (*data_out) << scd3[j] << "  ";
            for(int j=0; j<ngeom; j++) (*data_out) << scd4[j] << "  ";
            for(int j=0; j<ngeom; j++) (*data_out) << scd5[j] << "  ";
            for(int j=0; j<ngeom; j++) (*data_out) << scd13[j]<< "  ";
            (*data_out) << endl;
            (*data_out) << aperture_hits   << " "
                        << aperture_scores << " "
                        << reject << endl;
            if( !data_out->good() ) return 1031;
            if( ncg > 0 ) {
              for(int j=0; j<ncg; j++) {
                double aux = dose5->thisHistoryScore(gind1[j])*
                             dose5->thisHistoryScore(gind2[j]);
                (*data_out) << scg5[j]+aux << "  ";
              }
              (*data_out) << endl;
              if( !data_out->good() ) return 1045;
            }
        }
        if( ncg > 0 ) {
            for(int j=0; j<ncg; j++) {
                double aux = dose->thisHistoryScore(gind1[j])*
                             dose->thisHistoryScore(gind2[j]);
                (*data_out) << scg[j]+aux << "  ";
            }
            (*data_out) << endl;
            if( !data_out->good() ) return 104;
        }
        if( score_q ) {
            if( !score_q->storeState(*data_out) ) return 105;
        }
        if( flup ) {
            for(int j=0; j<ngeom; j++) {
                if( !flum[j]->storeState(*data_out) ) return 106+2*j;
                if( !flup[j]->storeState(*data_out) ) return 107+2*j;
            }
            if( !flumT->storeState(*data_out) )   return 110+2*ngeom;
            if( !flupT->storeState(*data_out) )   return 111+2*ngeom;
        }
        if( flug ) {
            for(int j=0; j<ngeom; j++) {
                if( !flug[j]->storeState(*data_out) ) return 108+2*j;
            }
            if( !flugT->storeState(*data_out) ) return 109+2*ngeom;
        }
        data_out->flush();
        delete data_out; data_out = 0;
        return 0;
    };

    /*! Read results from a .egsdat file. */
    int readData() {
        int err = EGS_AdvancedApplication::readData();
        if( err ) return err;
        if( !dose->setState(*data_in) ) return 101;
        if( type == Awall || type == FAC ) {
            if( !ideal_dose->setState(*data_in) ) return 102;
            for(int j=0; j<ngeom; j++) (*data_in) >> corr[j];
            if( !data_in->good() ) return 103;
        }
        if( type == FAC ) {
            if( !dose1->setState(*data_in) ) return 1021;
            if( !dose2->setState(*data_in) ) return 1022;
            if( !dose3->setState(*data_in) ) return 1023;
            if( !dose4->setState(*data_in) ) return 1024;
            if( !dose5->setState(*data_in) ) return 1025;
            for(int j=0; j<ngeom; j++) (*data_in) >> scd0[j];
            for(int j=0; j<ngeom; j++) (*data_in) >> scd1[j];
            for(int j=0; j<ngeom; j++) (*data_in) >> scd2[j];
            for(int j=0; j<ngeom; j++) (*data_in) >> scd3[j];
            for(int j=0; j<ngeom; j++) (*data_in) >> scd4[j];
            for(int j=0; j<ngeom; j++) (*data_in) >> scd5[j];
            for(int j=0; j<ngeom; j++) (*data_in) >> scd13[j];
            (*data_in) >> aperture_hits >> aperture_scores >> reject;
            if( !data_in->good() ) return 1031;
            if( ncg > 0 ) {
              for(int j=0; j<ncg; j++) (*data_in) >> scg5[j];
              if( !data_in->good() ) return 1045;
            }
        }
        if( ncg > 0 ) {
            for(int j=0; j<ncg; j++) (*data_in) >> scg[j];
            if( !data_in->good() ) return 104;
        }
        if( score_q ) {
            if( !score_q->setState(*data_in) ) return 105;
        }
        if( flup ) {
            for(int j=0; j<ngeom; j++) {
                if( !flum[j]->setState(*data_in) ) return 106+2*j;
                if( !flup[j]->setState(*data_in) ) return 107+2*j;
            }
            if( !flumT->setState(*data_in) ) return 110+2*ngeom;
            if( !flupT->setState(*data_in) ) return 111+2*ngeom;
        }
        if( flug ) {
            for(int j=0; j<ngeom; j++) {
                if( !flug[j]->setState(*data_in) ) return 108+2*j;
            }
            if( !flugT->setState(*data_in) ) return 109+2*ngeom;
        }
        return 0;
    };

    /*! Reset the variables used for accumulating results */
    void resetCounter() {
        EGS_AdvancedApplication::resetCounter();
        dose->reset();
        if( type == Awall || type == FAC ) {
            ideal_dose->reset();
            for(int j=0; j<ngeom; j++) corr[j] = 0;
        }
        if( type == FAC ) {
            dose1->reset();dose2->reset();
            dose3->reset();dose4->reset();
            dose5->reset();
            for(int j=0; j<ngeom; j++){
                scd1[j] = 0; scd2[j] = 0;
                scd3[j] = 0; scd4[j] = 0;
                scd5[j] = 0; scd0[j] = 0; scd13[j] = 0;
            }
            aperture_hits = 0; aperture_scores = 0; reject = 0;
            if( ncg > 0 ) {
              for(int j=0; j<ncg; j++) scg5[j] = 0;
            }
        }
        if( ncg > 0 ) {
            for(int j=0; j<ncg; j++) scg[j] = 0;
        }
        if( score_q ) score_q->reset();
        if( flup ) {
            for(int j=0; j<ngeom; j++) {
                flum[j]->reset(); flup[j]->reset();
            }
            flumT->reset();
            flupT->reset();
        }
        if( flug ) {
            for(int j=0; j<ngeom; j++) {
                flug[j]->reset();
            }
            flugT->reset();
        }
    };

    /*! Add simulation results */
    int addState(istream &data) {
        int err = EGS_AdvancedApplication::addState(data);
        if( err ) return err;
        EGS_ScoringArray tmp(ngeom);
        if( !tmp.setState(data) ) return 101;
        (*dose) += tmp;
        if( type == Awall || type == FAC ) {
            if( !tmp.setState(data) ) return 102;
            (*ideal_dose) += tmp;
            for(int j=0; j<ngeom; j++) {
                double aux; data >> aux;
                if( !data.good() ) return 103;
                corr[j] += aux;
            }
        }
        if( type == FAC ) {
            if( !tmp.setState(data) ) return 1021;
            (*dose1) += tmp;
            if( !tmp.setState(data) ) return 1022;
            (*dose2) += tmp;
            if( !tmp.setState(data) ) return 1023;
            (*dose3) += tmp;
            if( !tmp.setState(data) ) return 1024;
            (*dose4) += tmp;
            if( !tmp.setState(data) ) return 1025;
            (*dose5) += tmp;
            for(int j=0; j<ngeom; j++) {
                double aux; data >> aux;
                if( !data.good() ) return 1031;
                scd0[j] += aux;
            }
            for(int j=0; j<ngeom; j++) {
                double aux; data >> aux;
                if( !data.good() ) return 1031;
                scd1[j] += aux;
            }
            for(int j=0; j<ngeom; j++) {
                double aux; data >> aux;
                if( !data.good() ) return 1032;
                scd2[j] += aux;
            }
            for(int j=0; j<ngeom; j++) {
                double aux; data >> aux;
                if( !data.good() ) return 1033;
                scd3[j] += aux;
            }
            for(int j=0; j<ngeom; j++) {
                double aux; data >> aux;
                if( !data.good() ) return 1034;
                scd4[j] += aux;
            }
            for(int j=0; j<ngeom; j++) {
                double aux; data >> aux;
                if( !data.good() ) return 1035;
                scd5[j] += aux;
            }
            for(int j=0; j<ngeom; j++) {
                double aux; data >> aux;
                if( !data.good() ) return 1035;
                scd13[j] += aux;
            }
            int aux_hits, aux_scores, aux_reject;
            (data) >> aux_hits >> aux_scores >> aux_reject;
            if( !data.good() ) return 1036;
            aperture_hits   += aux_hits;
            aperture_scores += aux_scores;
            reject += aux_reject;
            if( ncg > 0 ) {
              for(int j=0; j<ncg; j++) {
                double tmp; data >> tmp;
                if( !data.good() ) return 1045;
                scg5[j] += tmp;
              }
            }
        }
        if( ncg > 0 ) {
            for(int j=0; j<ncg; j++) {
                double tmp; data >> tmp;
                if( !data.good() ) return 104;
                scg[j] += tmp;
            }
        }
        if( score_q ) {
            if( !tmp.setState(data) ) return 105;
            (*score_q) += tmp;
        }
        if( flup ) {
            EGS_ScoringArray t(flu_nbin);
            for(int j=0; j<ngeom; j++) {
                if( !t.setState(data) ) return 106+2*j;
                (*flum[j]) += t;
                if( !t.setState(data) ) return 107+2*j;
                (*flup[j]) += t;
            }
            EGS_ScoringArray tmT(ngeom);
            if( !tmT.setState(data) ) return 110+2*ngeom;
            (*flumT) += tmT;
            EGS_ScoringArray tpT(ngeom);
            if( !tpT.setState(data) ) return 111+2*ngeom;
            (*flupT) += tpT;
        }
        if( flug ) {
            EGS_ScoringArray tg(flu_nbin);
            for(int j=0; j<ngeom; j++) {
                if( !tg.setState(data) ) return 108+2*j;
                (*flug[j]) += tg;
            }
            EGS_ScoringArray tgT(ngeom);
            if( !tgT.setState(data) ) return 109+2*ngeom;
            (*flugT) += tgT;
        }
        return 0;
    };

    /*! Output the results of a simulation. */
    void outputResults() {
        egsInformation("\n\n last case = %lld fluence = %g\n\n",
                current_case,source->getFluence());
        if( type == Fano ) egsInformation(
          "****** This is a Fano calculation, i.e. scattered/secondary photons were\n"
          "****** thrown away and primary photons were regenerated after each\n"
          "****** interaction\n\n");
        if( type == HVL ) {
            egsInformation("\n***** This is a HVL calculation.\n");
            if( !hvl_scatter ) egsInformation("      scatter is NOT included!\n");
            egsInformation(
            "      KERMA is scored in a circle with radius %g\n"
            "      with midpoint (%g,%g,%g) in a plane with normal (%g,%g,%g)\n\n",
            hvl_R,hvl_midpoint.x,hvl_midpoint.y,hvl_midpoint.z,
            hvl_normal.x,hvl_normal.y,hvl_normal.z);
            egsInformation("%-25s          KERMA         ","Geometry");
        }
        else
            egsInformation("%-25s       Cavity dose      ","Geometry");
        if( type == Awall ) egsInformation("      Awall\n"
        "-------------------------------------------------------------------\n");
        else if( type == FAC ) egsInformation("   Awall=Dtot/Dideal\n"
        "-------------------------------------------------------------------\n");
        else egsInformation("\n"
        "-----------------------------------------------\n");
        char c = '%';
        for(int j=0; j<ngeom; j++) {
            double r,dr; dose->currentResult(j,r,dr);
            if( r > 0 ) dr = 100*dr/r; else dr = 100;
            // Line below commented out to get energy rather than dose deposited
            // Needed for calculations to check consistency of the EADL relaxation
            // implementation
            EGS_Float norm = 1.602e-10*current_case/source->getFluence();
            //EGS_Float norm = current_case/source->getFluence();
            //egsInformation("current_case=%lld fluence=%lg norm=%g\n",
            //        current_case,source->getFluence(),norm);
            if( type == HVL ) norm /= (M_PI*hvl_R*hvl_R);
            else norm /= mass[j];
            egsInformation("%-25s %12.6le +/- %-9.5lf%c ",
                    calc_names[j].c_str(),
                    r*norm,dr,c);
            if( type == Awall || type == FAC ) {
                double r1,dr1;
                ideal_dose->currentResult(j,r1,dr1);
                if( r1 > 0 ) dr1 = 100*dr1/r1; else dr1 = 100;
                double Aw,dAw;
                if( r > 0 && r1 > 0 ) {
                    double cov = (corr[j]/(r*r1*current_case) - 1)/
                                 (current_case-1);
                    dAw = sqrt(dr*dr + dr1*dr1 - 2e4*cov);
                    Aw = r/r1;
                }
                else { Aw = 1; dAw = 100; }
                egsInformation("%7.5lf +/- %-7.3lf%c\n",Aw,dAw,c);
                egsInformation("Dideal = %10.4le +/- %-7.3lf%c\n",
                    r1*norm,dr1,c);

               if( type == FAC ) {
                egsInformation("\n\n Free Air Chamber corrections\n"
                                   "-----------------------------\n");
                double E1,dE1;
                dose1->currentResult(j,E1,dE1);
                if( E1 > 0 ) dE1 = 100*dE1/E1; else dE1 = 100;
                double Afac,dAfac;
                if( r > 0 && E1 > 0 ) {
                  double covf = (scd1[j]/(r*E1*current_case) - 1)/
                               (current_case-1);
                  dAfac = sqrt(dr*dr + dE1*dE1 - 2e4*covf);
                  Afac  = r/E1;
                }
                else { Afac = 1; dAfac = 100; }
                egsInformation(" Aap    = %7.5lf +/- %-8.4lf%c ",Afac,dAfac,c);
                egsInformation(" D1 = %10.4le +/- %-7.3lf%c\n",E1*norm,dE1,c);

                double E2,dE2;
                dose2->currentResult(j,E2,dE2);
                if( E2 > 0 ) dE2 = 100*dE2/E2; else dE2 = 100;
                if( E1 > 0 && E2 > 0 ) {
                  double covf = (scd2[j]/(E1*E2*current_case) - 1)/
                               (current_case-1);
                  dAfac = sqrt(dE1*dE1 + dE2*dE2 - 2e4*covf);
                  Afac  = E1/E2;
                }
                else { Afac = 1; dAfac = 100; }
                egsInformation(" Ascat  = %7.5lf +/- %-8.4lf%c ",Afac,dAfac,c);
                egsInformation(" D2 = %10.4le +/- %-7.3lf%c\n",E2*norm,dE2,c);

                double E3,dE3;
                dose3->currentResult(j,E3,dE3);
                if( E3 > 0 ) dE3 = 100*dE3/E3; else dE3 = 100;
                if( E2 > 0 && E3 > 0 ) {
                  double covf = (scd3[j]/(E2*E3*current_case) - 1)/
                               (current_case-1);
                  dAfac = sqrt(dE2*dE2 + dE3*dE3 - 2e4*covf);
                  Afac  = E2/E3;
                }
                else { Afac = 1; dAfac = 100; }
                egsInformation(" Aatt   = %7.5lf +/- %-8.4lf%c ",Afac,dAfac,c);
                egsInformation(" D3 = %10.4le +/- %-7.3lf%c\n",E3*norm,dE3,c);

                if( E1 > 0 && E3 > 0 ) {
                  double covf = (scd13[j]/(E1*E3*current_case) - 1)/
                               (current_case-1);
                  dAfac = sqrt(dE1*dE1 + dE3*dE3 - 2e4*covf);
                  Afac  = E1/E3;
                }
                else { Afac = 1; dAfac = 100; }
                egsInformation(" Awall  = %7.5lf +/- %-8.4lf%c ",Afac,dAfac,c);
                egsInformation(" ==> Awall = D1/D3 \n");

                double E4,dE4;
                dose4->currentResult(j,E4,dE4);
                if( E4 > 0 ) dE4 = 100*dE4/E4; else dE4 = 100;
                if( E3 > 0 && E4 > 0 ) {
                  double covf = (scd4[j]/(E3*E4*current_case) - 1)/
                               (current_case-1);
                  dAfac = sqrt(dE3*dE3 + dE4*dE4 - 2e4*covf);
                  Afac  = E3/E4;
                }
                else { Afac = 1; dAfac = 100; }
                egsInformation(" Aeloss = %7.5lf +/- %-8.4lf%c ",Afac,dAfac,c);
                egsInformation(" D4 = %10.4le +/- %-7.3lf%c\n",E4*norm,dE4,c);

                EGS_Float norm5 = norm*mass[j]/(M_PI*hvl_R*hvl_R);
                double E5,dE5;
                dose5->currentResult(j,E5,dE5);
                if( E5 > 0 ) dE5 = 100*dE5/E5; else dE5 = 100;
                if( E4 > 0 && E5 > 0 ) {
                  double covf = (scd5[j]/(E4*E5*current_case) - 1)/
                               (current_case-1);
                  dAfac = sqrt(dE4*dE4 + dE5*dE5 - 2e4*covf);
                  Afac  = E4*norm/E5/norm5;
                }
                else { Afac = 1; dAfac = 100; }
                egsInformation(" Ag     = %7.5lf +/- %-8.4lf%c ",Afac,dAfac,c);
                egsInformation(" D5 = %10.4le +/- %-7.3lf%c\n",E5*norm5,dE5,c);

                if( r > 0 && E5 > 0 ) {
                  double covf = (scd0[j]/(r*E5*current_case) - 1)/
                               (current_case-1);
                  dAfac = sqrt(dr*dr + dE5*dE5 - 2e4*covf);
                  Afac  = r*norm/E5/norm5;
                }
                else { Afac = 1; dAfac = 100; }
                egsInformation("-----------------------------\n");
                egsInformation(" Atotal = %7.5lf +/- %-8.4lf%c\n",Afac,dAfac,c);

                egsInformation("\n\n times aperture particles scored in cavity: %d \n",
                        aperture_scores);
                egsInformation("\n\n times particles hit/penetrated aperture: %d \n",
                        aperture_hits);
                egsInformation("\n\n times primary Compton interaction rejected: %d \n",
                        reject);
               }


            } else egsInformation("\n");
        }
        egsInformation("\n\n");
        if( ncg > 0 ) {
            if( type == HVL )
                egsInformation("%-20s %-20s    KERMA ratio\n","Geometry 1",
                    "Geometry 2");
            else
                egsInformation("%-20s %-20s    Dose ratio\n","Geometry 1",
                    "Geometry 2");


            vector<double> ratio, dratio;

            for(int j=0; j<ncg; j++) {
                double r1,dr1,r2,dr2;
                dose->currentResult(gind1[j],r1,dr1);
                dose->currentResult(gind2[j],r2,dr2);
                if( r1 > 0 && r2 > 0 ) {
                    double rc=(scg[j]/(r1*r2*current_case)-1)/(current_case-1);
                    dr1 /= r1; dr2 /= r2;
                    double dr = dr1*dr1 + dr2*dr2 - 2*rc;
                    if( dr > 0 ) dr = sqrt(dr);
                    double r = type == HVL ? r1/r2 :
                        r1*mass[gind2[j]]/(r2*mass[gind1[j]]);
                    egsInformation("%-20s %-20s     %-8.5lf +/- %-7.5lf\n",
                                   calc_names[gind1[j]].c_str(),
                                   calc_names[gind2[j]].c_str(),r,r*dr);
                    ratio.push_back(r); dratio.push_back(r*dr);
                }
                else egsInformation("zero dose\n");
            }

            if (thicknesses.size()>0){
                EGS_HVL *hvl_f = new EGS_HVL(thicknesses, ratio, dratio);
                hvl_f->printFittedHVL(); hvl_f->printCovarianceMatrix();
            }

            if( type == FAC){
              ratio.clear(); dratio.clear();
              egsInformation("%-20s %-20s    KERMA ratio\n","Geometry 1",
                    "Geometry 2");
              for(int j=0; j<ncg; j++) {
                  double rK1,drK1,rK2,drK2;
                  dose5->currentResult(gind1[j],rK1,drK1);
                  dose5->currentResult(gind2[j],rK2,drK2);
                  if( rK1 > 0 && rK2 > 0 ) {
                    double rc=(scg[j]/(rK1*rK2*current_case)-1)/(current_case-1);
                    drK1 /= rK1; drK2 /= rK2;
                    double dr = drK1*drK1 + drK2*drK2 - 2*rc;
                    if( dr > 0 ) dr = sqrt(dr);
                    double r = rK1/rK2;
                    egsInformation("%-20s %-20s     %-8.5lf +/- %-7.5lf\n",
                                   calc_names[gind1[j]].c_str(),
                                   calc_names[gind2[j]].c_str(),r,r*dr);
                    ratio.push_back(r); dratio.push_back(r*dr);
                  }
                  else egsInformation("zero dose\n");
              }
              if (thicknesses.size()>0){
                EGS_HVL *hvl_f = new EGS_HVL(thicknesses, ratio, dratio);
                hvl_f->printFittedHVL(); hvl_f->printCovarianceMatrix();
              }

            }
        }
        if( score_q ) {
            egsInformation("\n\nCharge balance\n"
                           "===============================\n\n");
            double norm = current_case/source->getFluence();
            double norm1 = norm/33.97e-6;
            for(int j=0; j<ngeom; j++) {
                if( do_charge[j] ) {
                    egsInformation("Geometry %s\n",geoms[j]->getName().c_str());
                    double q,dq;
                    score_q->currentResult(j,q,dq);
                    q *= norm; dq *= norm;
                    egsInformation("    charge per fluence: %g +/- %g\n",q,dq);
                    double r,dr;
                    dose->currentResult(j,r,dr); r *= norm1; dr *= norm1;
                    egsInformation("    cavity ion pairs per fluence: %g +/- "
                            "%g\n",r,dr);
                    egsInformation("    ratio: %g\n\n",q/r);
                }
            }
        }
        if( flup ) {
            egsInformation("\n\nElectron and positron fluence\n"
                               "=============================\n");
            for(int j=0; j<ngeom; j++) {
                /********************************************************
                 * To get volume-averaged path length one needs to know
                 * the volume of the cavity since (dE/dx) rather than
                 * (dE/rho/dx) is used. Rho taken from density of the first
                 * cavity region.
                 ********************************************************/
                double norm = current_case/(source->getFluence()*mass[j])*Rho[j];
                egsInformation("\nGeometry: %s\n\n",geoms[j]->getName().c_str());

                double fe,dfe,fp,dfp,totFe=0,totFeErr=0,totFp=0,totFpErr=0;
                EGS_Float flu_a_i = 1/flu_a,
                          the_bw  = flu_s? 1.0 : flu_a_i;
                /**********************************************************
                 * Total e-/+ fluence, uncertainty includes correlations
                 ************************************************************/
                egsInformation("\n Total e-/+ fluence scored (includes correlations)\n");
                egsInformation(" -------------------------------------------------\n");
                /* electrons */
                flumT->currentResult(j,fe,dfe);
                if( fe > 0 ) dfe = 100*dfe/fe; else dfe = 100;
                egsInformation(" F_e- = %10.4le +/- %-7.3lf\% ",
                               fe*norm*the_bw,dfe);
                /* positrons */
                flupT->currentResult(j,fp,dfp);
                if( fp > 0 ) dfp = 100*dfp/fp; else dfp = 100;
                egsInformation("F_e+ = %10.4le +/- %-7.3lf\%\n\n",
                               fp*norm*the_bw,dfp);
                /***********************************************
                 *     Differential e-/+ fluence
                 **********************************************/
                for(int i=0; i<flu_nbin; i++) {
                    flum[j]->currentResult(i,fe,dfe);
                    flup[j]->currentResult(i,fp,dfp);
                    EGS_Float e = ( i + 0.5 - flu_b )*flu_a_i;
                    if( flu_s ) e = exp(e);
                    egsInformation("%11.6f  %14.6e  %14.6e  %14.6e  %14.6e\n",
                            e,fe*norm,dfe*norm,fp*norm,dfp*norm);
                    the_bw = flu_s ? DE[i] : flu_a_i;
                    totFe += fe*the_bw; totFeErr += dfe*dfe*the_bw*the_bw;
                    totFp += fp*the_bw; totFpErr += dfp*dfp*the_bw*the_bw;
                }
                /*****************************************************************
                 * Integrated fluence with uncertainty ignoring correlations
                 *
                 * Since one particle can contribute to several bins, this approach
                 * ignores this correlation by assuming the values in each bin to
                 * be independent.
                 ******************************************************************/
                if ( totFe > 0 ) totFeErr = 100*sqrt(totFeErr)/totFe; else totFeErr = 100;
                if ( totFp > 0 ) totFpErr = 100*sqrt(totFpErr)/totFp; else totFpErr = 100;
                egsInformation("\n Integrated fluence (correlations neglected)\n");
                egsInformation(" -------------------------------------------\n");
                egsInformation(
                  "\n F_e- = %10.4le +/- %-7.3lf\% ", totFe*norm,totFeErr);
                egsInformation(
                     "F_e+ = %10.4le +/- %-7.3lf\%\n\n",totFp*norm,totFpErr);
            }
        }
        if( flug ) {
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

            egsInformation("\n\nPhoton fluence [cm-2*MeV-1]\n"
                               "=============================\n");
            for(int j=0; j<ngeom; j++) {
                double norm = current_case/source->getFluence();//per particle
                       norm /= (M_PI*hvl_R*hvl_R);              //per unit area
                       norm *= flu_a;                           //per unit bin width

                egsInformation("\nGeometry %s :",geoms[j]->getName().c_str());
                spe_output<<"@    s"<<j<<" errorbar linestyle 0\n";
                spe_output<<"@    s"<<j<<" legend \""<<
                          geoms[j]->getName().c_str()<<"\"\n";
                spe_output<<"@target G0.S"<<j<<"\n";
                spe_output<<"@type xydy\n";
                double fe,dfe,fp,dfp;
                flugT->currentResult(j,fe,dfe);
                if( fe > 0 ) dfe = 100*dfe/fe; else dfe = 100;
                egsInformation(" total fluence = %10.4le +/- %-7.3lf\%\n\n",
                        fe*norm,dfe);
                for(int i=0; i<flu_nbin; i++) {
                    flug[j]->currentResult(i,fe,dfe);
                    EGS_Float e = (i+0.5-flu_b)/flu_a;
                    if( flu_s ) e = exp(e);
                    spe_output<<e<<" "<<fe*norm<<" "<<dfe*norm<< "\n";
                    egsInformation("%11.6f  %14.6e  %14.6e\n",
                            e,fe*norm,dfe*norm);
                }
                spe_output << "&\n";
egsInformation("=> norm = %g \n",current_case/source->getFluence());
            }
        }

    };

    /*! Get the current simulation result.  */
    void getCurrentResult(double &sum, double &sum2, double &norm,
            double &count) {
        count = current_case; double flu = source->getFluence();
        norm = flu > 0 ? 1.602e-10*count/(flu*mass[0]) : 0;
        dose->currentScore(0,sum,sum2);
    };

    /*! simulate a shower */
    int shower() {
        expmfp[0] = 1;
        latchr[0]  = 0;
        kexpmfp[0] = 1;
#ifdef USTEP_DEBUG
        the_geometry_debug->nstep = 0;
        the_geometry_debug->icase = current_case;
#endif
        return EGS_AdvancedApplication::shower();
    };

    /* Select photon mean-free-path */
    void selectPhotonMFP(EGS_Float &dpmfp) {
        int np = the_stack->np-1;
        if( type == HVL ) {
            if( the_stack->E[np] < the_bounds->pcut ) {
                --the_stack->np; dpmfp=-1; return;
            }
            EGS_Vector x(the_stack->x[np],the_stack->y[np],the_stack->z[np]);
            EGS_Vector u(the_stack->u[np],the_stack->v[np],the_stack->w[np]);
            EGS_Float xp = x*hvl_normal, up = u*hvl_normal;
            if( (up > 0 && hvl_d > xp) || (up < 0 && hvl_d < xp) ) {
                EGS_Float t = (hvl_d - xp)/up;
                EGS_Vector x1(x + u*t - hvl_midpoint);
                if( x1.length2() < hvl_R*hvl_R ) {
                   //egsWarning("selectPhotonMFP(HVL): x=(%g,%g,%g) u=(%g,%g,%g) xp=%g"
                   // " up=%g\n",x.x,x.y,x.z,u.x,u.y,u.z,xp,up);
                    EGS_Float tstep; int inew;
                    int ireg = the_stack->ir[np]-2;
                    int newmed = geometry->medium(ireg);
                    int imed = -1; EGS_Float gmfp, sigma = 0, cohfac = 1;
                    EGS_Float cohfac_int;
                    EGS_Float gle = the_epcont->gle;
                    EGS_Float eta;
                    if( hvl_first_rng > 0 ) {
                        eta = hvl_first_rng; hvl_first_rng = -1;
                    }
                    else eta = 1 - rndm->getUniform();
                    double lambda = -log(eta);
                    //egsWarning("will hit circle: %g %g %g %g\n",x1.x,x1.y,
                    //        x1.length2(),lambda);
                    double Lambda = 0, ttot = 0, Lambda_new = 0;
                    EGS_Vector xint; bool do_interaction = false; int iint;
                    while(1) {
                        if( imed != newmed ) {
                            imed = newmed;
                            if( imed >= 0 ) {
                                gmfp = i_gmfp[imed].interpolateFast(gle);
                                if( the_xoptions->iraylr ) {
                                    cohfac = i_cohe[imed].interpolateFast(gle);
                                    gmfp *= cohfac;
                                }
                                sigma = 1/gmfp;
                            } else { sigma = 0; cohfac = 1; }
                            //egsWarning("imed=%d sigma=%g cohfac=%g\n",
                            //        imed,sigma,cohfac);
                        }
                        tstep = 1e35;
                        inew = geometry->howfar(ireg,x,u,tstep,&newmed);
                        Lambda_new += tstep*sigma;
                        //egsWarning("after step: %d %g %g\n",inew,tstep,
                        //        Lambda_new);
                        if( Lambda_new > lambda && !do_interaction ) {
                            do_interaction = true;
                            double dt = (lambda - Lambda_new)/sigma + tstep;
                            xint = x + u*dt; iint = ireg;
                            cohfac_int = cohfac;
                        }
                        if( ttot + tstep <= t ) Lambda = Lambda_new;
                        else if( ttot <= t ) Lambda += (t - ttot)*sigma;
                        else if( do_interaction ) break;
                        if( inew < 0 ) break;
                        ireg = inew; x += u*tstep; ttot += tstep;
                    }
                    EGS_Float exp_Lambda = Lambda < 80 ? exp(-Lambda) : 0;
                    EGS_Float muen = hvl_muen->interpolateFast(gle);
                    EGS_Float aup = fabs(up); if( aup < 0.08 ) aup = 0.08;
                    //egsWarning("wt=%g exp_pambda=%g muen=%g tot=%g\n",
                    //        the_stack->wt[np],exp_Lambda,muen,
                    //        the_stack->wt[np]*exp_Lambda*muen);
                    dose->score(ig,the_stack->wt[np]/aup*exp_Lambda*muen);
                    //--------------------------------------------
                    // score photon fluence
                    //--------------------------------------------
                    if( flug ) {
                      EGS_Float e = the_stack->E[np];
                      if( flu_s ) {e = log(e);}
                      EGS_Float ae; int je;
                      if( e > flu_xmin && e <= flu_xmax) {
                        ae = flu_a*e + flu_b; je = (int) ae;
                        EGS_ScoringArray *aux = flug[ig];
                        aux->score(je,the_stack->wt[np]/aup*exp_Lambda);
                        flugT->score(ig,the_stack->wt[np]/aup*exp_Lambda);
                      }
                    }
                    dpmfp = -1;
                    if( do_interaction ) {
                        /*
                        int itest = geometry->isWhere(xint);
                        if( itest != iint ) {
                            egsWarning("Interacting at x=(%g,%g,%g)\n",
                                    xint.x,xint.y,xint.z);
                            egsWarning("We think we are in region %d but "
                                 "isWhere() returns %d. Geometry is %d\n",
                                 iint,itest,ig);
                        }
                        */
                        the_stack->x[np] = xint.x;
                        the_stack->y[np] = xint.y;
                        the_stack->z[np] = xint.z;
                        the_stack->ir[np] = iint + 2;
                        the_stack->dnear[np] = 0;
                        imed = geometry->medium(iint);
                        the_useful->medium = imed+1;
                        //egsWarning("Interacting at (%g,%g,%g) %d %d\n",
                        //      xint.x,xint.y,xint.z,iint,the_useful->medium);
                        if( the_xoptions->iraylr ) {
                            if( rndm->getUniform() < 1 - cohfac_int ) {
                                if( type == HVL && !hvl_scatter ) --the_stack->np;
                                else doRayleigh();
                                return;
                            }
                        }
                        eta = rndm->getUniform();
                        EGS_Float gbr1 = i_gbr1[imed].interpolateFast(gle);
                        EGS_Float gbr2 = i_gbr2[imed].interpolateFast(gle);
                        if( the_stack->E[np] > the_thresh->rmt2 && eta < gbr1 ) {
                            if( type == HVL && !hvl_scatter ) --the_stack->np;
                            else F77_OBJ(pair,PAIR)();
                        }
                        else if( eta < gbr2 ) {
                            F77_OBJ(compt,COMPT)();
                            if( type == HVL && !hvl_scatter &&
                                the_stack->np > the_stack->npold )
                                the_stack->np = the_stack->npold-1;
                        }
                        else {
                            if( type == HVL && !hvl_scatter )
                                --the_stack->np;
                            else F77_OBJ(photo,PHOTO)();
                        }
                    }
                    else --the_stack->np;
                    return;
                }
            }
            dpmfp = -log(1 - rndm->getUniform()); return;
        }
        if( fsplit <= 1 ) {
            dpmfp = -log(1 - rndm->getUniform()); return;
        }
        if( the_stack->iq[np] ) egsFatal("selectPhotonMFP called with a"
            " particle of charge %d\n",the_stack->iq[np]);
        EGS_Float wt_o = the_stack->wt[np];
        EGS_Float expmfp_o  = expmfp[np];
        EGS_Float kexpmfp_o;
        if (type == FAC) kexpmfp_o = kexpmfp[np];
        EGS_Float E = the_stack->E[np];
        int ireg   = the_stack->ir[np]-2, latch = the_stack->latch[np];
        int latch1 = latch,
            alatch = abs(latch),
            signo = latch < 0 ? -1 : 1,
            latchr1 = latchr[np];
        EGS_Float f_split, f_spliti;
        if( alatch < 2 ) { f_split = fsplit; f_spliti = fspliti; }
        else {
            f_split = rr_flag; f_spliti = 1/f_split;
            latch1 = latch - signo*rr_flag; the_stack->latch[np] = latch1;
        }
        the_stack->wt[np] = wt_o*f_spliti;
        int imed = geometry->medium(ireg);
        EGS_Float gmfpr=1e15, cohfac=1;
        EGS_Float gle = the_epcont->gle;
        //if( imed >= 0 ) calculatePhotonMFP(&gmfp,&cohfac);
        if( imed >= 0 ) {
            gmfpr = i_gmfp[imed].interpolateFast(gle);
            if( the_xoptions->iraylr ) {
                cohfac = i_cohe[imed].interpolateFast(gle);
                gmfpr *= cohfac;
            }
        }
        EGS_Vector x(the_stack->x[np],the_stack->y[np],the_stack->z[np]);
        EGS_Vector u(the_stack->u[np],the_stack->v[np],the_stack->w[np]);
        EGS_Float rhor = the_useful->rhor;
        EGS_Float gmfp = gmfpr/rhor;

        int i_survive = (int) ( f_split*rndm->getUniform() );
#ifdef CAVITY_DEBUG
        egsInformation("\nselectPhotonMFP(): np=%d E=%lg wt=%g ireg=%d "
            "imed=%d latch=%d fsplit=%g\n",np,E,wt_o,ireg,imed,latch,f_split);
        egsInformation("                   x=(%g,%g,%g) u=(%g,%g,%g)\n",
                x.x,x.y,x.z,u.x,u.y,u.z);
        egsInformation("                   expmfp=%g i_survive=%d gmfp=%g\n",
                expmfp_o,i_survive,gmfp);
        egsInformation("                   case=%lld\n",current_case);
#endif
        EGS_Float mfp_old = 0,
                  eta_prime = 1 + rndm->getUniform()*f_spliti;
        dpmfp = -1; int isplit = 0;
        //egsInformation("slectPhotonMFP: x=(%g,%g,%g) ir=%d gmfp=%g rhor=%g\n",
        //        x.x,x.y,x.z,ireg,gmfp,rhor);
        while(1) {
            eta_prime -= f_spliti;
            if( eta_prime <= 0 ) { --the_stack->np; return; }
            EGS_Float mfp = -log(eta_prime) - mfp_old;
            //egsInformation("next interaction at %g mfp\n",mfp);
#ifdef CAVITY_DEBUG
            egsInformation("  new mfp: %g\n",mfp);
#endif
            EGS_Float exp_mfp = exp(mfp);
            if( type != Dose ) expmfp_o *= exp_mfp;
            EGS_Float xp , up, t;
            /*************************************************
             * No need to check whether t negative below since
             * only primaries are corrected for attenuation,i.e.,
             * this photons will be moving along initial direction.
             *************************************************/
            if( type == FAC ) {//kexpmfp_o *= exp_mfp;
             xp = x*hvl_normal;up = u*hvl_normal;t = (hvl_d - xp)/up;
            }
            double ttot = 0;
            mfp_old = mfp_old + mfp;
            while(1) {
                EGS_Float tstep = mfp*gmfp; int newmed;
                //egsInformation("new step: %g %g %d %g %g\n",mfp,tstep,ireg,gmfp,rhor);
#ifdef CAVITY_DEBUG
                egsInformation("  step: %g ",tstep);
#endif
#ifdef GDEBUG
                if( steps_n < MAX_STEP ) steps_ustepi[steps_n] = tstep;
#endif
                int inew = geometry->howfar(ireg,x,u,tstep,&newmed);
#ifdef GDEBUG
                if( steps_n < MAX_STEP ) {
                    steps_x[steps_n] = x; steps_u[steps_n] = u;
                    steps_ireg[steps_n] = ireg; steps_inew[steps_n] = inew;
                    steps_ustepf[steps_n++] = tstep;
                }
                if( tstep < -1e-4 ) {
                    egsWarning("Negative step: %g\n",tstep);
                    for(int j=0; j<steps_n; j++)
                        egsWarning("%d x=(%g,%g,%g) u=(%g,%g,%g) ireg=%d "
                          "inew=%d ustepi=%g ustepf=%g\n",j,steps_x[j].x,
                          steps_x[j].y,steps_x[j].z,
                                steps_u[j].x,steps_u[j].y,steps_u[j].z,
                                steps_ireg[j],steps_inew[j],steps_ustepi[j],
                                steps_ustepf[j]);
                    exit(1);
                }
#endif
#ifdef CAVITY_DEBUG
                egsInformation("new: reg=%d med=%d step=%g\n",
                        inew,newmed,tstep);
#endif
                if( inew < 0 ) { --the_stack->np; return; }
                //------------------------------------------------
                //ausgab(BeforeTransport);
                //------------------------------------------------
                if ( type  == FAC ){
                    if( ireg >= 0             &&
                        is_aperture[ig][ireg] &&
                        latch >=0             ){
                        signo = -1;
                        latch  = latch  > 0 ? -latch  : -1;
                        latch1 = latch1 > 0 ? -latch1 : -1;
                        the_stack->latch[np] = latch1;
                        aperture_hits++;
                    }

                    if     ( ttot >= t )       kexpmfp_o *= exp(tstep/gmfp);
                    else if( ttot + tstep > t) kexpmfp_o *= exp((ttot+tstep-t)/gmfp);
                    ttot += tstep;
                }
                //------------------------------------------------
                x += u*tstep;
                //------------------------------------------------
                if( inew == ireg ) break;
                mfp -= tstep/gmfp;
                if( geometry->hasRhoScaling() )
                    rhor = geometry->getRelativeRho(inew);
                else rhor = 1;
                ireg = inew;
                if( newmed != imed ) {
                    imed = newmed; the_useful->medium = imed+1;
#ifdef CAVITY_DEBUG
                    if( imed < 0 || imed >= geometry->nMedia() )
                        egsWarning(" new medium is %d ?\n",imed);
#endif
                    //if( imed >= 0 ) calculatePhotonMFP(&gmfp,&cohfac);
                    if( imed >= 0 ) {
                        gmfpr = i_gmfp[imed].interpolateFast(gle);
                        if( the_xoptions->iraylr ) {
                            cohfac = i_cohe[imed].interpolateFast(gle);
                            gmfpr *= cohfac;
                        }
                    }
                    else { gmfpr=1e15, cohfac=1; }
                }
                gmfp = gmfpr/rhor;
            }
            //
            // *********** time to interact
            //
#ifdef CAVITY_DEBUG
            egsInformation("  interaction at (%g,%g,%g)\n",x.x,x.y,x.z);
#endif
            the_stack->x[np]=x.x; the_stack->y[np]=x.y; the_stack->z[np]=x.z;
            the_stack->ir[np] = ireg+2; expmfp[np] = expmfp_o;
            the_stack->dnear[np] = 0;
            if (type == FAC) kexpmfp[np] = kexpmfp_o;
            bool is_rayleigh = false;
            if( the_xoptions->iraylr ) {
                if( rndm->getUniform() < 1 - cohfac ) { // ******** rayleigh
#ifdef CAVITY_DEBUG
                    egsInformation("  Rayleigh\n");
#endif
                    is_rayleigh = true;
                    if( isplit != i_survive ) { --np; --the_stack->np; }
                    else {
                        the_stack->wt[np] = wt_o;
                        doRayleigh();
                        the_stack->latch[np] = alatch < 2 ?
                            signo : signo*(rr_flag+1);
                        latchr[np] = 0;
                    }
                }
            }

            if( !is_rayleigh ) {
                bool is_compton = false;
                EGS_Float gbr1, gbr2;
                //calculatePhotonBranching(&gbr1,&gbr2);
                gbr1 = i_gbr1[imed].interpolateFast(gle);
                gbr2 = i_gbr2[imed].interpolateFast(gle);
                EGS_Float eta = rndm->getUniform();
#ifdef CAVITY_DEBUG
                egsInformation("  branching ratios: %g %g rnno: %g\n",
                        gbr1,gbr2,eta);
#endif
                if( E > the_thresh->rmt2 && eta < gbr1 ) { // ********* pair
                    F77_OBJ(pair,PAIR)();
                }
                else if( eta < gbr2 ) {                    // ********* compton
                    F77_OBJ(compt,COMPT)();
                    /*********************************************************
                      Remember and don't get confused again!
                      ======================================
                      We remove attenuation by unweighting with exp(mfp).
                      But the Compton cross section comes from Klein-Nishina
                      and is therefore larger that the real cross section with
                      binding => we unattenuate too much. To get the proper
                      Awall, we have to count primary photons that have a
                      rejected Compton interaction as scattered!
                      This approach will not give the correct Ascall and Aatt,
                      but Awall = Ascat*Aatt will be correct.
                    if( the_stack->np == the_stack->npold )
                        mark_photons = false;
                    // i.e. interaction was rejected => don't mark the
                    // original photon as scattered.
                    **********************************************************/
                    is_compton = true;
                }
                else {                                     // ********* photo
                    F77_OBJ(photo,PHOTO)();
                }
                np = the_stack->np-1; int ip = the_stack->npold-1;
                int ipo = ip, npo = np;
                bool do_rr=(rr_flag>0 && !is_cavity[ig][ireg]);
                EGS_Float cperp=1e30;
                if( do_rr && cgeom ) {
                    if( !cgeom->isInside(x) ) cperp = cgeom->hownear(-1,x);
                    else do_rr = false;
                } else do_rr = false;
#ifdef CAVITY_DEBUG
                egsInformation("  cperp = %g %d\n",cperp,do_rr);
                egsInformation("  Produced particles:\n");
#endif

                do {
#ifdef CAVITY_DEBUG
                    egsInformation("   %d %d q=%d E=%g wt=%g\n",
                            ip,np,the_stack->iq[ip],
                            the_stack->E[ip],the_stack->wt[ip]);
#endif
                   /*******************************************************
                    * As soon as a primary Compton interaction is rejected,
                    * the flag latchr is set to 1. If the interaction is
                    * accepted at any time, latchr is set back to 0. Once
                    * latch is set to  1 after a true interaction or rejection,
                    * latchr should be left unchanged in subsequent rejections.
                    * This accounts for successive rejections or rejection
                    * after a true interaction.
                    ********************************************************/
                    if (is_compton && type == FAC){
                        if (ipo == npo ){                // Compton interaction rejected
                            if (the_stack->latch[np] == 0){// primary particle
                                latchr[np] = 1;            // mark as rejected; latchr = 1
                                reject++;//count Compton rejected primary particles
                            }
                        }
                        //else: rejection of either a scatter or already
                        //      rejected interaction. Leave latchr unchanged.
                        else {             // Compton interaction accepted
                            latchr[ip] = 0;// mark as accepted; latchr = 0
                        }
                    }
                    //********************************************
                    //***** flag particles after interacting *****
                    //********************************************
                    //latch = -rr_flag-1=> descendants of RR survivors go through aperture
                    //latch = -rr_flag  => electron surviving RR goes through aperture
                    //latch = -1        => particle goes through aperture
                    //latch =  0        => primary particle
                    //latch =  1        => scattered particle
                    //latch = rr_flag   => electron survived Russian Roulette
                    //latch = rr_flag+1 => descendants of particle surviving RR
                    /*************************************************************/
                    if( !the_stack->iq[ip] ) {
                        if( isplit == i_survive && type != Fano ) {
                            the_stack->wt[ip] = wt_o;
                            // reset latchr so that photon no longer counts as a primary
                            // with a rejected Compton interaction
                            if( type == FAC && !is_compton ) latchr[ip] = 0;
                            the_stack->latch[ip++] = alatch < 2 ?
                                signo : signo*(rr_flag+1);
                        }
                        else {
                            if( ip < np ) {
                                the_stack->E[ip] = the_stack->E[np];
                                the_stack->iq[ip] = the_stack->iq[np];
                                the_stack->latch[ip] = the_stack->latch[np];
                                          latchr[ip] = latchr[np];
                                the_stack->u[ip] = the_stack->u[np];
                                the_stack->v[ip] = the_stack->v[np];
                                the_stack->w[ip] = the_stack->w[np];
                            }
                            --np; --the_stack->np;
                        }
                    }
                    else {
                        bool keep = true;
                        if( do_rr ) {
                            if( the_media->rho[imed] > 0.95*rho_rr ) {
                                EGS_Float crange = 0;
                                EGS_Float e = the_stack->E[ip]-the_useful->rm;
                                if( e > 0 ) {
                                    EGS_Float elke=log(e);
                                    crange = the_stack->iq[ip] == -1 ?
                                        rr_erange.interpolate(elke) :
                                            rr_prange.interpolate(elke);
                                }
#ifdef CAVITY_DEBUG
                                egsInformation(" crange = %g\n",crange);
#endif
                                if( crange < cperp ) {
                                    if( rr_flag == 1 ) keep = false;
                                    else {
                                        if( rndm->getUniform()*rr_flag < 1 ) {
                                            the_stack->wt[ip] *= rr_flag;
                                            the_stack->latch[ip] += signo*rr_flag;
                                        } else keep = false;
                                    }
                                }
                            }
                        }
                        if( keep ) ++ip;
                        else {
                            if( ip < np ) {
                                the_stack->E[ip] = the_stack->E[np];
                                the_stack->iq[ip] = the_stack->iq[np];
                                the_stack->latch[ip] = the_stack->latch[np];
                                          latchr[ip] = latchr[np];
                                the_stack->u[ip] = the_stack->u[np];
                                the_stack->v[ip] = the_stack->v[np];
                                the_stack->w[ip] = the_stack->w[np];
                            }
                            --np; --the_stack->np;
                        }
                    }
                } while (ip <= np);
            }
            ++isplit;
            ++np; ++the_stack->np;
            the_stack->E[np] = E; the_stack->wt[np] = wt_o*f_spliti;
            the_stack->iq[np] = 0;
            the_stack->latch[np] = latch1;latchr[np]=latchr1;
            the_stack->ir[np] = ireg+2; expmfp[np] = expmfp_o;
            if (type == FAC)           kexpmfp[np] = kexpmfp_o;
            the_stack->u[np]=u.x; the_stack->v[np]=u.y; the_stack->w[np]=u.z;
            the_stack->x[np]=x.x; the_stack->y[np]=x.y; the_stack->z[np]=x.z;
            the_stack->dnear[np] = 0;
        }
    };

    int rangeDiscard(EGS_Float tperp, EGS_Float range) const {
        // we can be sure that when this function is called
        // range rejection/RR is on.
        //
        // If rr_flag = 1 & E<Esave, we immediately discard the particle if it
        // can not reach the cavity or escape the current region
        // If rr_flag > 1, we play RR with the particle with survival
        // probability of 1/rr_flag, if it can not reach the cavity or
        // discard it if it is in the cavity and can not escape and E<Esave
        // However, we only play RR if that was not done before.
        // This is indicated by the value of latch:
        //   latch=0,1 indicates a primary/secondary electron that has
        //             not been previosly subjected to RR.
        //   latch=x,x+1 (with x>1) indicates a primary/secondary electron
        //             that has already been range-RR'ed.
        //
        int np = the_stack->np-1;
        if( abs(the_stack->latch[np]) > 1 ) return 0;
        int signo = the_stack->latch[np]<0 ? -1 : 1;
        bool is_cav = is_cavity[ig][the_stack->ir[np]-2];
        if( (rr_flag == 1 || is_cav) && the_stack->E[np] > Esave ) return 0;
          // i.e., if rr_flag is 1 or rr_flag > 1 but we are in the cavity and
          // the energy is greater than Esave, don't discard the particle
        int retval = the_stack->iq[np] == -1 ? 1 : 99;
          // if here: rr_flag = 1 && E < Esave
          //  or      rr_flag > 1 && (in cavity but E<Esave) || not in cavity
        bool do_RR = false;
        if( range < tperp ) { // can not escape current region
            if( rr_flag == 1 || is_cav ) return retval;
            do_RR = true;
        }
        else { // can escape current region
            if( is_cav || !cgeom ) return 0;
            // don't do it in low density media
            EGS_Float rho = the_media->rho[the_useful->medium-1];
            if( rho < 0.95*rho_rr ) return 0;
            EGS_Vector x(the_stack->x[np],the_stack->y[np],the_stack->z[np]);
            int ireg = cgeom->isWhere(x);
            if( ireg < 0 ) {
                EGS_Float cperp = cgeom->hownear(ireg,x);
                EGS_Float crange = the_stack->iq[np] == -1 ?
                    rr_erange.interpolateFast(the_epcont->elke) :
                    rr_prange.interpolateFast(the_epcont->elke);
                //egsInformation("E=%g elke=%g crange=%g x=(%g,%g,%g) cperp=%g\n",
                //        the_stack->E[np],the_epcont->elke,crange,
                //        x.x,x.y,x.z,cperp);
                if( crange < cperp ) {
                    if( rr_flag == 1 ) return retval;
                    do_RR = true;
                }
            }
        }
        if( !do_RR ) return 0;
        if( rndm->getUniform()*rr_flag < 1 ) {
            // particle survives.
            the_stack->wt[np] *= rr_flag; the_stack->latch[np] += signo*rr_flag;
            return 0;
        }
        return -1; // i.e. particle is killed and must be discarded immediately.
    };

protected:

    /*! Start a new shower.  */
    int startNewShower() {
        int res = EGS_Application::startNewShower();
        if( res ) return res;
        if( current_case != last_case ) {
            if( type == Awall || type == FAC ) {
                for(int j=0; j<ngeom; j++)
                    corr[j] += dose->thisHistoryScore(j)*
                         ideal_dose->thisHistoryScore(j);
                ideal_dose->setHistory(current_case);
            }
            if( type == FAC ) {
                for(int j=0; j<ngeom; j++){
                   scd0[j] += dose->thisHistoryScore(j)*
                             dose5->thisHistoryScore(j);
                   scd1[j] += dose->thisHistoryScore(j)*
                             dose1->thisHistoryScore(j);
                   scd2[j] += dose1->thisHistoryScore(j)*
                              dose2->thisHistoryScore(j);
                   scd3[j] += dose2->thisHistoryScore(j)*
                              dose3->thisHistoryScore(j);
                   scd4[j] += dose3->thisHistoryScore(j)*
                              dose4->thisHistoryScore(j);
                   scd5[j] += dose4->thisHistoryScore(j)*
                              dose5->thisHistoryScore(j);
                   scd13[j] += dose1->thisHistoryScore(j)*
                              dose3->thisHistoryScore(j);
                }
                if( ncg > 0 ) {
                   for(int j=0; j<ncg; j++){
                    scg5[j] += dose5->thisHistoryScore(gind1[j])*
                               dose5->thisHistoryScore(gind2[j]);
                   }
                }
                dose1->setHistory(current_case);
                dose2->setHistory(current_case);
                dose3->setHistory(current_case);
                dose4->setHistory(current_case);
                dose5->setHistory(current_case);
            }
            if( ncg > 0 ) {
                for(int j=0; j<ncg; j++)
                    scg[j] += dose->thisHistoryScore(gind1[j])*
                              dose->thisHistoryScore(gind2[j]);
            }
            dose->setHistory(current_case);
            if( score_q ) score_q->setHistory(current_case);
            if( flup ) {
                for(int j=0; j<ngeom; j++) {
                    flup[j]->setHistory(current_case);
                    flum[j]->setHistory(current_case);
                }
                flumT->setHistory(current_case);
                flupT->setHistory(current_case);
            }
            if( flug ) {
                for(int j=0; j<ngeom; j++) {
                    flug[j]->setHistory(current_case);
                }
                flugT->setHistory(current_case);
            }
            last_case = current_case;
        }
        return 0;
    };

private:

    Type             type;      // calculation type:
                                //   = 0 => just calculate dose
                                //   = 1 => calculate dose and Awall
                                //   = 2 => fano calculation.
    int              ngeom;     // number of geometries to calculate
                                // quantities of interest
    int              ig;        // current geometry index

    int              ncg;       // number of correlated geometry pairs.
    int              *gind1,
                     *gind2;    // indeces of correlated geometries
    double           *scg;      // sum(dose(gind1[j])*dose(gind2[j]);
    double           *scg5;     // sum(dose5(gind1[j])*dose5(gind2[j]);

    EGS_BaseGeometry **geoms;   // geometries for which to calculate the
                                // quantites of interest.
    string           *calc_names; // calculation names
    EGS_AffineTransform **transforms;
                                // transformations to apply before transporting
                                // for each geometry
    bool             **is_cavity; // array of flags for each region in each
                                // geometry, which is true if the region
                                // belongs to the cavity and false otherwise
    bool             **is_aperture;// array of flags for each region in each
                                   // geometry, which is true if the region
                                   // belongs to the aperture, false otherwise
    EGS_ScoringArray *dose;     // scoring array for dose scoring in each of
                                // the calculation geometries.
    EGS_ScoringArray *ideal_dose; // scoring array for dose scoring in each of
                                // the calculation geometries with attenuation
                                // and scatter removed.

    EGS_ScoringArray *dose1;    // array for dose scoring if aperture
                                // were totally absorbing.
    EGS_ScoringArray *dose2;    // array for primary dose scoring in
                                // each of the calculation geometries
    EGS_ScoringArray *dose3;    // array for unattenuated primary
                                // dose scoring
    EGS_ScoringArray *dose4;    // array for unattenuated primary
                                // dose scoring when electrons deposit their
                                // energy on the spot (kerma)
    EGS_ScoringArray *dose5;    // array for kerma scoring at POM.

    double           *scd0;      // sum(dose *dose5): Atotal
    double           *scd1;      // sum(dose *dose1); Aap
    double           *scd2;      // sum(dose1*dose2); Ascat
    double           *scd3;      // sum(dose2*dose3); Aatt
    double           *scd13;     // sum(dose1*dose3); Awall = D1/D3
    double           *scd4;      // sum(dose3*dose4); Aeloss
    double           *scd5;      // sum(dose4*dose5); Ag

    EGS_ScoringArray **flug;    // photon fluence
    EGS_ScoringArray * flugT;   // total photon fluence

    EGS_ScoringArray **flum;    // electron fluence
    EGS_ScoringArray **flup;    // positron fluence
    EGS_ScoringArray * flumT;   // total electron fluence
    EGS_ScoringArray * flupT;   // total positron fluence
    /********************************************************
     * To get volume-averaged path length one needs to know
     * the volume of the cavity since (dE/dx) rather than
     * (dE/rho/dx) is used. Rho is the density of the first
     * cavity region.
     ********************************************************/
    int       *cavity_medium; // cavity medium, usually air
    EGS_Float      *Rho;      // material density in the cavity.
    EGS_Float      *Lmid_i;   // pre-computed inverse of bin midpoint stpwr
    EGS_Float       flu_a,    // interpolation parameter : 1/bw
                    flu_b,    // interpolation parameter : -Emin/bw
                    flu_xmin, // minimum energy Emin or log of Emin
                    flu_xmax; // maximum energy Emax or log of Emax
    int             flu_s,    // flag to turn on log scale scoring
                    flu_nbin; // number of energy bins
    eFluType        flu_stpwr;// flurz   => ave. stpwr = edep/tvstep,
                              // stpwr   => 3rd order in edep/Eb,
                              // stpwrO5 => 5th order in edep/Eb
   /**************************************************************
    * Parameters required for fluence calculations on a log-scale
    * The main issue here is that the bin width is not constant
    *************************************************************/
    EGS_Float       r_const;    // inverse of (Emax/Emin)**1/flu_nbin - 1 = exp(binwidth)-1
    EGS_Float      *a_const;    // constant needed to determine bin fractions on log scale
    EGS_Float      *DE;         // bin width of logarithmic scale
    /*****************************************************************/
    double           *corr;     // correlation between the above two
    EGS_Float        *mass;     // mass of the material in the cavity.
    EGS_Float        *expmfp;   // attenuation unweighting
    EGS_Float       *kexpmfp;   // attenuation unweighting beyond kerma scoring
                                // plane

    EGS_Float        fsplit;    // photon splitting number
    EGS_Float        fspliti;   // inverse photon splitting number

    bool            *do_charge;
    bool           **is_charge;
    EGS_ScoringArray *score_q;

    /*! Rejected Compton interaction flag
      If set to 0, no Compton interaction rejected
      If set to 1,    Compton interaction rejected
     */
    int*             latchr; // Compton rejection flag
    /*! Range rejection flag
      If set to 0, no range rejection is used
      If set to 1, charged particles that can not enter the cavity are
      immediately discarded.
      If > 1, Russian Roulette (RR) with survival probability 1/rr_flag
      is played with charged particles that can not enter the cavity.
     */
    int              rr_flag;   // range rejection flag:
    /*! Save energy for range rejection
      For rr_flag = 1, electrons are range-discarded if E<Esave
      For rr_flag > 1, electrons in the cavity are range-discarded if
      E<Esave, electrons outside of the cavity are always rouletted,
      no matter what their energy.
     */
    EGS_Float        Esave;
    /*! Mass density of the range rejection medium */
    EGS_Float        rho_rr;
    /*! Cavity bounding geometry.
      If no cavity bounding geometry is defined, range-rejection of RR
      is used only on a region-by-region basis. If a cavity bounding geometry
      is defined, then tperp to that geometry is also checked and if greater
      than the electron range, range-rejection or RR is done.
     */
    EGS_BaseGeometry *cgeom;
    /*! Range interpolators */
    EGS_Interpolator rr_erange;
    EGS_Interpolator rr_prange;

    /*! HVL scoring */
    EGS_Vector        hvl_normal;
    EGS_Vector        hvl_midpoint;
    EGS_Float         hvl_d;
    EGS_Float         hvl_R;
    EGS_Float         hvl_first_rng;
    int               hvl_scatter;
    EGS_Interpolator *hvl_muen;
    vector<EGS_Float> thicknesses;
    int nsmall_step;

    int aperture_scores;
    int aperture_hits;
    int reject;

    static string revision;

};

string Cavity_Application::revision = " ";

struct EGS_ExtraStack {
    EGS_Float expmfp[MXSTACK];
    EGS_Float kexpmfp[MXSTACK];
    int       latchr[MXSTACK];
    EGS_Float expmfpI, kexpmfpI;
    int       latchrI;
};

extern __extc__ struct EGS_ExtraStack F77_OBJ_(extra_stack,EXTRA_STACK);
static struct EGS_ExtraStack *the_extra_stack =
                 &F77_OBJ_(extra_stack,EXTRA_STACK);

extern __extc__  void
F77_OBJ_(select_photon_mfp,SELECT_PHOTON_MFP)(EGS_Float *dpmfp) {
    EGS_Application *a = EGS_Application::activeApplication();
    Cavity_Application *app = dynamic_cast<Cavity_Application *>(a);
    if( !app ) egsFatal("select_photon_mfp called with active application "
            " not being of type Cavity_Application!\n");
    app->selectPhotonMFP(*dpmfp);
}

extern __extc__ void F77_OBJ_(range_discard,RANGE_DISCARD)(
        const EGS_Float *tperp, const EGS_Float *range) {
    Cavity_Application *app = dynamic_cast<Cavity_Application *>(
            EGS_Application::activeApplication());
    the_epcont->idisc = app->rangeDiscard(*tperp,*range);
}
extern __extc__ void F77_OBJ_(egs_scale_xcc,EGS_SCALE_XCC)(const int *,
                                  const EGS_Float *);
#define egsScaleXsection F77_OBJ_(egs_scale_photon_xsection,EGS_SCALE_PHOTON_XSECTION)
extern __extc__ void egsScaleXsection(const int *imed, const EGS_Float *fac,
                                      const int *which);

int Cavity_Application::initScoring() {
    EGS_Input *options = input->takeInputItem("scoring options");
    if( options ) {
        //
        // *********** photon cross section scaling
        //
        EGS_Input *scaling;
        EGS_BaseGeometry::setActiveGeometryList(app_index);
        while( (scaling = options->takeInputItem("scale photon x-sections")) ) {
            EGS_Float factor; string medname; int what;
            int err1 = scaling->getInput("factor",factor);
            int err2 = scaling->getInput("medium",medname);
            vector<string> allowed;
            allowed.push_back("all"); allowed.push_back("Rayleigh");
            allowed.push_back("Compton"); allowed.push_back("Pair");
            allowed.push_back("Photo");
            what = scaling->getInput("cross section",allowed,0);
            if( !err1 && !err2 ) {
                int imed;
                if( medname == "ALL" || medname == "all" || medname == "All" )
                    imed = 0;
                else {
                    EGS_BaseGeometry::setActiveGeometryList(app_index);
                    imed = EGS_BaseGeometry::addMedium(medname); ++imed;
                    if( imed > the_media->nmed ) {
                        egsInformation("Scaling requested for medium %s,"
                              " but such medium does not exist\n",medname.c_str());
                        imed = -1;
                    }
                }
                if( imed >= 0 )
                    egsScaleXsection(&imed,&factor,&what);
            }
            delete scaling; scaling = 0;

        }

        //
        // *********** calculation type
        //
        vector<string> allowed_types;
        allowed_types.push_back("dose"); allowed_types.push_back("Awall");
        allowed_types.push_back("Fano"); allowed_types.push_back("HVL");
        allowed_types.push_back("FAC");
        //type = (Type) options->getInput("calculation type",allowed_types);
        int itype = options->getInput("calculation type",allowed_types,0);
        if( itype == 1 )      type = Awall;
        else if( itype == 2 ) type = Fano;
        else if( itype == 3 ) type = HVL;
        else if( itype == 4 ) type = FAC;
        else                  type = Dose;

        //
        // ********* scale elastic scattering
        //
        EGS_Input *scale;
        while( (scale = options->takeInputItem("scale xcc")) ) {
            vector<EGS_Float> tmp;
            int err = scale->getInput("scale xcc",tmp);
            if( !err ) {
                int im = (int) tmp[0]; ++im;
                egsInformation("Scaling xcc of medium %d with %g\n",im,tmp[1]);
                F77_OBJ_(egs_scale_xcc,EGS_SCALE_XCC)(&im,&tmp[1]);
            }
            delete scale;
        }

        //
        // *********** calculation geometries
        //
        vector<EGS_BaseGeometry *> geometries;
        vector<string> cnames;
        vector<int *>  cavity_regions;
        vector<int>  n_cavity_regions;
        vector<int *>  aperture_regions;
        vector<int>  n_aperture_regions;
        vector<EGS_Float> cavity_masses;
        vector<EGS_AffineTransform *> transformations;
        vector<int> charge_nr;
        vector<int *> charge_regions;
        EGS_Input *aux;
        EGS_BaseGeometry::setActiveGeometryList(app_index);
        while( (aux = options->takeInputItem("calculation geometry")) ) {
            string gname,cname;
            int err = aux->getInput("geometry name",gname);
            int errx = aux->getInput("calculation name",cname);
            if( errx ) cname = gname;
            string cavString;
            vector<int> cav;
            int err1 = aux->getInput("cavity regions",cavString);
            string apertString;
            vector<int> apert;
            int err4 = aux->getInput("aperture regions",apertString);
            EGS_Float cmass;
            int err2 = aux->getInput("cavity mass",cmass);
            string chargeString;
            vector<int> charge;
            int err3 = aux->getInput("charge regions",chargeString);
            if( err ) egsWarning("initScoring: missing/wrong 'geometry name' "
                    "input\n");
            if( err1 ) egsWarning("initScoring: missing/wrong 'cavity regions' "
                    "input\n");
            if( err2 ) {
                egsWarning("initScoring: missing/wrong 'cavity mass' "
                    "input\n"); cmass = -1;
            }
            if ( type == FAC && err4 ){
             egsWarning("\n\n*** Calculation type set to FAC but an \n"
                      "     error was found while reading aperture\n"
                      "     regions\n"
                      "    No region will be set as aperture region\n\n");
            }
            if( err || err1 ) egsWarning("  --> input ignored\n");
            else {
                EGS_BaseGeometry::setActiveGeometryList(app_index);
                EGS_BaseGeometry *g = EGS_BaseGeometry::getGeometry(gname);
                if( !g ) egsWarning("initScoring: no geometry named %s -->"
                        " input ignored\n",gname.c_str());
                else {

                    g->getNumberRegions(cavString, cav);
                    g->getLabelRegions(cavString, cav);
                    g->getNumberRegions(apertString, apert);
                    g->getLabelRegions(apertString, apert);
                    g->getNumberRegions(chargeString, charge);
                    g->getLabelRegions(chargeString, charge);

                    int nreg = g->regions();
                    int *regs = new int [cav.size()];
                    int ncav = 0;
                    for(int j=0; j<cav.size(); j++) {
                        if( cav[j] < 0 || cav[j] >= nreg )
                            egsWarning("initScoring: region %d is not within"
                               " the allowed range of 0...%d -> input"
                               " ignored\n",cav[j],nreg-1);
                        else regs[ncav++] = cav[j];
                    }
                    if( !ncav ) {
                        egsWarning("initScoring: no cavity regions "
                        "specified for geometry %s --> input ignored\n",
                        gname.c_str());
                        delete [] regs;
                    }
                    else {
                        geometries.push_back(g);
                        cnames.push_back(cname);
                        n_cavity_regions.push_back(ncav);
                        cavity_regions.push_back(regs);
                        cavity_masses.push_back(cmass);
                        transformations.push_back(
                                EGS_AffineTransform::getTransformation(aux));
                        if( !err3 && charge.size() > 0 ) {
                            int *r = new int [charge.size()]; int nr=0;
                            for(int j=0; j<charge.size(); j++) {
                                if( charge[j] >= 0 && charge[j] < nreg ) {
                                    r[nr++] = charge[j];
                                }
                            }
                            if( nr ) {
                                charge_regions.push_back(r);
                                charge_nr.push_back(nr);
                            }
                            else {
                                delete [] r;
                                charge_regions.push_back(0);
                                charge_nr.push_back(0);
                            }
                        }
                        else {
                            charge_regions.push_back(0);
                            charge_nr.push_back(0);
                        }
                        if( type == FAC ) {
                          if( apert.size() > 0 ) {
                            int *ap = new int [apert.size()]; int nap=0;
                            for(int j=0; j<apert.size(); j++) {
                                if( apert[j] >= 0 && apert[j] < nreg ) {
                                    ap[nap++] = apert[j];
                                }
                                else{
                                 egsFatal("\n\n*** Calculation type is FAC\n"
                                     " but aperture region %d is\n"
                                     " outside the allowed range of  \n"
                                     " 0...%d  \n"
                                     " This is a fatal error\n\n",
                                     apert[j],nreg-1);
                                }
                            }
                            n_aperture_regions.push_back(nap);
                            aperture_regions.push_back(ap);
                          }
                          else{
                            egsWarning("\n\n*** Calculation type set to FAC\n"
                                     " but no aperture regions specified\n"
                                     " for geometry %s \n"
                                     " No region set as aperture!!!\n\n",
                                     gname.c_str());
                          }
                        }
                        else{
                            aperture_regions.push_back(0);
                            n_aperture_regions.push_back(0);
                        }
                    }
                }
            }
            delete aux;
        }
        ngeom = geometries.size();
        if( !ngeom ) {
            egsWarning("initScoring: no calculation geometries defined\n");
            return 1;
        }
        geoms = new EGS_BaseGeometry* [ngeom];
        calc_names = new string [ngeom];
        is_cavity   = new bool* [ngeom];
        is_aperture = new bool* [ngeom];
        mass = new EGS_Float [ngeom];
        dose = new EGS_ScoringArray(ngeom);
        transforms = new EGS_AffineTransform* [ngeom];
        is_charge = new bool* [ngeom];
        do_charge = new bool [ngeom];
        if( type == Awall || type == FAC ) {
            ideal_dose = new EGS_ScoringArray(ngeom);
            corr = new double [ngeom];
            for(int j=0; j<ngeom; j++) corr[j] = 0;
        }
        if( type == FAC ) {
            dose1 = new EGS_ScoringArray(ngeom);
            dose2 = new EGS_ScoringArray(ngeom);
            dose3 = new EGS_ScoringArray(ngeom);
            dose4 = new EGS_ScoringArray(ngeom);
            dose5 = new EGS_ScoringArray(ngeom);
            scd0 = new double [ngeom];
            scd1 = new double [ngeom];
            scd2 = new double [ngeom];
            scd3 = new double [ngeom];
            scd4 = new double [ngeom];
            scd5 = new double [ngeom];
            scd13= new double [ngeom];
            for(int j=0; j<ngeom; j++){
               scd1[j] = 0;scd2[j] = 0;
               scd3[j] = 0;scd4[j] = 0;
               scd5[j] = 0;scd0[j] = 0;scd13[j]=0;
            }
            aperture_scores = 0;
            aperture_hits   = 0;
            reject          = 0;
        }
        for(int j=0; j<ngeom; j++) {
            do_charge[j] = false;
            geoms[j] = geometries[j]; //geoms[j]->ref();
            calc_names[j] = cnames[j];
            mass[j] = cavity_masses[j];
            transforms[j] = transformations[j];
            int nreg = geoms[j]->regions();
            is_cavity[j]   = new bool [nreg];
            is_aperture[j] = new bool [nreg];
            int i; for(i=0; i<nreg; i++){
                is_cavity[j][i]   = false;
                is_aperture[j][i] = false;
            }
            int imed = -999;
            for(i=0; i<n_cavity_regions[j]; i++) {
                int ireg = cavity_regions[j][i];
                is_cavity[j][ireg] = true;
                if( imed == -999 ) imed = geoms[j]->medium(ireg);
                else {
                    int imed1 = geoms[j]->medium(ireg);
                    if( imed1 != imed ) egsWarning("initScoring: different "
                        "medium %d in region %d compared to medium %d in "
                        "region %d. Hope you know what you are doing\n",
                        imed1,ireg,imed,cavity_regions[j][0]);
                }
            }
            delete [] cavity_regions[j];
            if( charge_nr[j] > 0 ) {
                do_charge[j] = true;
                if( !score_q ) score_q = new EGS_ScoringArray(ngeom);
                is_charge[j] = new bool [nreg];
                int i; for(i=0; i<nreg; i++) is_charge[j][i] = false;
                for(i=0; i<charge_nr[j]; i++)
                    is_charge[j][charge_regions[j][i]] = true;
                delete [] charge_regions[j];
            }
            if (n_aperture_regions.size()>0){
            for(i=0; i<n_aperture_regions[j]; i++) {
                int areg = aperture_regions[j][i];
                is_aperture[j][areg] = true;
            }
            delete [] aperture_regions[j];
            }
        }

        if( type == Dose || type == HVL || type == FAC ) {
            EGS_Input *aux;
            vector<int> cor1, cor2;
            while( (aux = options->takeInputItem("correlated geometries")) ) {
                vector<string> gnames;
                int err = aux->getInput("correlated geometries",gnames);
                if( !err && gnames.size() == 2 ) {
                    int j1, j2;
                    for(j1=0; j1<ngeom; j1++)
                        if( gnames[0] == geoms[j1]->getName() ) break;
                    for(j2=0; j2<ngeom; j2++)
                        if( gnames[1] == geoms[j2]->getName() ) break;
                    if( j1 < ngeom && j2 < ngeom ) {
                        cor1.push_back(j1); cor2.push_back(j2);
                    }
                }
            }
            if( cor1.size() > 0 ) {
                ncg = cor1.size();
                gind1 = new int [ncg]; gind2 = new int [ncg];
                scg = new double [ncg];
                for(int j=0; j<ncg; j++) {
                    scg[j] = 0; gind1[j] = cor1[j]; gind2[j] = cor2[j];
                }
                if(type == FAC){
                   scg5 = new double [ncg];
                   for(int j=0; j<ncg; j++) {
                       scg5[j] = 0;
                   }
                }
            }

            aux = options->takeInputItem("fluence scoring");
            if( aux ) {
                EGS_Float flu_Emin, flu_Emax;
                int er1 = aux->getInput("minimum energy",flu_Emin);
                int er2 = aux->getInput("maximum energy",flu_Emax);
                int er3 = aux->getInput("number of bins",flu_nbin);
                vector<string> scale;
                scale.push_back("linear"); scale.push_back("logarithmic");
                flu_s = aux->getInput("scale",scale,0);
                if( !er1 && !er2 && !er3 ) {
                    /* charged particle fluence */
                    if (type == Dose){
                      flum = new EGS_ScoringArray * [ngeom];
                      flup = new EGS_ScoringArray * [ngeom];
                      flumT = new EGS_ScoringArray(ngeom);
                      flupT = new EGS_ScoringArray(ngeom);
                      Rho  = new EGS_Float [ngeom];
                      cavity_medium = new int [ngeom];
                      for(int j=0; j<ngeom; j++) {
                         flum[j] = new EGS_ScoringArray(flu_nbin);
                         flup[j] = new EGS_ScoringArray(flu_nbin);
                         /* Get cavity medium's density */
                         cavity_medium[j] = -1;
                         for(int i=0; i<geoms[j]->regions(); i++) {
                             if( is_cavity[j][i] ) {
                                 if ( Rho && cavity_medium[j] < 0 ){
                                    cavity_medium[j] = geoms[j]->medium(i);
                                    Rho[j] = the_media->rho[cavity_medium[j]];
                                 }
                                 else break;
                             }
                         }
                      }
                      vector<string> method;
                      method.push_back("flurz"); method.push_back("stpwr");   // 3rd order
                                                 method.push_back("stpwrO5"); // 5th order
                      flu_stpwr = eFluType(aux->getInput("method",method,1));
                      EGS_Float bw = flu_s ?
                                    (log(flu_Emax / flu_Emin))/flu_nbin :
                                        (flu_Emax - flu_Emin) /flu_nbin;
                      EGS_Float expbw;
                      /* Pre-calculated values for faster evaluation on log scale */
                      if (flu_s){
                         expbw   = exp(bw); // => (Emax/Emin)^(1/nbin)
                         r_const = 1/(expbw-1);
                         DE      = new EGS_Float [flu_nbin];
                         a_const = new EGS_Float [flu_nbin];
                         for ( int i = 0; i < flu_nbin; i++ ){
                           DE[i]      = flu_Emin*pow(expbw,i)*(expbw-1);
                           a_const[i] = 1/flu_Emin*pow(1/expbw,i);
                         }
                      }
                      /* Do not score below ECUT - PRM */
                      if (flu_Emin < the_bounds->ecut-the_useful->prm){
                         flu_Emin = the_bounds->ecut - the_useful->prm;
                         /* Decrease number of bins, preserve bin width */
                         flu_nbin = flu_s ?
                                    ceil((log(flu_Emax / flu_Emin))/bw) :
                                    ceil(    (flu_Emax - flu_Emin) /bw);
                      }
                      flu_a = 1.0/bw;
                      /* Pre-calculated values for faster 1/stpwr evaluation */
                      if (flu_stpwr){
                         EGS_Float lnEmin  = flu_s ? log(0.5*flu_Emin*(expbw+1)):0,
                                   lnEmid;
                         Lmid_i  = new EGS_Float [flu_nbin*ngeom];
                         for ( int i = 0; i < flu_nbin; i++ ){
                           lnEmid     = flu_s ? lnEmin + i*bw : log(flu_Emin+bw*(i+0.5));
                           for(int j=0; j<ngeom; j++) {
                              Lmid_i[i+j*flu_nbin] = 1/i_ededx[cavity_medium[j]].interpolate(lnEmid);
                           }
                         }
                      }
                    }
                    else{/* photon fluence */
                      flug  = new EGS_ScoringArray * [ngeom];
                      flugT = new EGS_ScoringArray(ngeom);
                      for(int j=0; j<ngeom; j++) {
                         flug[j] = new EGS_ScoringArray(flu_nbin);
                      }
                    }
                    if( flu_s == 0 ) {
                        flu_xmin = flu_Emin; flu_xmax = flu_Emax;
                    }
                    else {
                        flu_xmin = log(flu_Emin); flu_xmax = log(flu_Emax);
                    }
                    if (flug){
                       flu_a = flu_nbin; flu_a /= (flu_xmax - flu_xmin);
                    }
                    flu_b = -flu_xmin*flu_a;
                }
                else {
                    egsInformation("\n\n******* Fluence scoring input"
                            " errors: %d %d %d\n",er1,er2,er3);
                    egsInformation("            => no fluence scoring\n\n");
                }
                delete aux;
            }
            if( type == HVL ) {
                aux = options->takeInputItem("HVL scoring");
                if( !aux )
                    egsFatal("\n\n*** Calculation type set to HVL but no HVL\n"
                                 "    scoring options provided\n"
                                 "    This is a fatal error\n\n");
                vector<EGS_Float> tmp_normal;
                int err1 = aux->getInput("scoring plane normal",tmp_normal);
                if( err1 || tmp_normal.size() != 3 ) egsFatal(
                  "\n\n***  Wrong/missing 'scoring plane normal' input for a "
                  "HVL calculation\n    This is a fatal error\n\n");
                vector<EGS_Float> tmp_circle;
                int err2 = aux->getInput("scoring circle",tmp_circle);
                if( err2 || tmp_circle.size() != 4 ) egsFatal(
                  "\n\n***  Wrong/missing 'scoring circle' input for a "
                  "HVL calculation\n    This is a fatal error\n\n");
                hvl_normal = EGS_Vector(tmp_normal[0],tmp_normal[1],
                                        tmp_normal[2]);
                hvl_normal.normalize();
                hvl_midpoint = EGS_Vector(tmp_circle[0],tmp_circle[1],
                                          tmp_circle[2]);
                hvl_d = hvl_normal*hvl_midpoint;
                hvl_R = tmp_circle[3];
                string muen_file;
                int err3 = aux->getInput("muen file",muen_file);
                if( err3 ) egsFatal(
                  "\n\n***  Wrong/missing 'muen file' input for a "
                  "HVL calculation\n    This is a fatal error\n\n");
                muen_file = egsExpandPath(muen_file);
                ifstream muen_data(muen_file.c_str());
                if( !muen_data ){
                    egsFatal(
                  "\n\n***  Failed to open muen file %s\n"
                      "     This is a fatal error\n",muen_file.c_str());
                }
                int ndat; muen_data >> ndat;
                if( ndat < 2 || muen_data.fail() ) egsFatal(
                        "\n\n*** Failed to read muen dfata file\n");
                EGS_Float *xmuen = new EGS_Float [ndat];
                EGS_Float *fmuen = new EGS_Float [ndat];
                for(int j=0; j<ndat; j++) muen_data >> xmuen[j] >> fmuen[j];
                if( muen_data.fail() ) egsFatal(
                     "\n\n*** Failed to read muen data file\n");
                hvl_muen = new EGS_Interpolator(ndat,log(xmuen[0]),
                        log(xmuen[ndat-1]),fmuen);
                delete [] xmuen; delete [] fmuen;
                vector<string> scatter;
                scatter.push_back("no"); scatter.push_back("yes");
                hvl_scatter = aux->getInput("scatter",scatter,1);
                /* as many as correlated geometries*/
                int err4 = aux->getInput("absorber thicknesses",thicknesses);
                if (err4 || thicknesses.size() == 0 || thicknesses.size() > ncg){
                    egsWarning("-> Error reading absorber thicknesses ...\n"
                               "   will just output kerma ratios without \n"
                               "   calculating the HVL!");
                    thicknesses.clear();//making sure its empty of garbage
                }
            }
        }

        if( type == FAC ) {
           aux = options->takeInputItem("Kerma scoring");
           if( !aux )
               egsFatal("\n\n*** Calculation type set to FAC but no \n"
                            "    kerma scoring options provided\n"
                            "    This is a fatal error\n\n");
           vector<EGS_Float> tmp_normal;
           int err1 = aux->getInput("scoring plane normal",tmp_normal);
           if( err1 || tmp_normal.size() != 3 ) egsFatal(
             "\n\n***  Wrong/missing 'scoring plane normal' input for a "
             "FAC calculation\n    This is a fatal error\n\n");
           vector<EGS_Float> tmp_circle;
           int err2 = aux->getInput("scoring circle",tmp_circle);
           if( err2 || tmp_circle.size() != 4 ) egsFatal(
             "\n\n***  Wrong/missing 'scoring circle' input for a "
             "FAC calculation\n    This is a fatal error\n\n");
           hvl_normal = EGS_Vector(tmp_normal[0],tmp_normal[1],
                                   tmp_normal[2]);
           hvl_normal.normalize();
           hvl_midpoint = EGS_Vector(tmp_circle[0],tmp_circle[1],
                                     tmp_circle[2]);
           hvl_d = hvl_normal*hvl_midpoint;
           hvl_R = tmp_circle[3];
           string muen_file;
           int err3 = aux->getInput("muen file",muen_file);
           if( err3 ) egsFatal(
             "\n\n***  Wrong/missing 'muen file' input for a "
             "HVL calculation\n    This is a fatal error\n\n");
           ifstream muen_data(muen_file.c_str());
           if( !muen_data ){
               egsFatal(
             "\n\n***  Failed to open muen file %s\n"
                 "     This is a fatal error\n",muen_file.c_str());
           }
           int ndat; muen_data >> ndat;
           if( ndat < 2 || muen_data.fail() ) egsFatal(
                   "\n\n*** Failed to read muen dfata file\n");
           EGS_Float *xmuen = new EGS_Float [ndat];
           EGS_Float *fmuen = new EGS_Float [ndat];
           for(int j=0; j<ndat; j++) muen_data >> xmuen[j] >> fmuen[j];
           if( muen_data.fail() ) egsFatal(
                "\n\n*** Failed to read muen data file\n");
           hvl_muen = new EGS_Interpolator(ndat,log(xmuen[0]),
                   log(xmuen[ndat-1]),fmuen);
           delete [] xmuen; delete [] fmuen;
           /* as many as correlated geometries*/
           int err4 = aux->getInput("absorber thicknesses",thicknesses);
           if (err4 || thicknesses.size() == 0 || thicknesses.size() > ncg){
               egsWarning("-> Error reading absorber thicknesses ...\n"
                       "   will just output kerma ratios without \n"
                       "   calculating the HVL!");
               thicknesses.clear();//making sure its empty of garbage
           }

        }

        delete options;
    }
    else {
        egsWarning("\n\n*********** no 'scoring options' input *********\n\n");
        return 2;
    }

    //
    // **** variance reduction
    //
    EGS_Input *vr = input->takeInputItem("variance reduction");
    if( vr ) {
        //
        // ******** photon splitting
        //
        EGS_Float tmp; int err = vr->getInput("photon splitting",tmp);
        if( !err && tmp > 1 ) {
            fsplit = tmp; fspliti = 1/tmp;
        }
        //
        // ******* range rejection
        //
        EGS_Input *rr = vr->takeInputItem("range rejection");
        if( rr ) {
            int iaux; err = rr->getInput("rejection",iaux);
            if( !err && iaux >= 0 ) rr_flag = iaux;
            if( rr_flag ) {
                EGS_Float aux; err = rr->getInput("Esave",aux);
                if( !err && aux >= 0 ) Esave = aux;
                string cavity_geometry;
                err = rr->getInput("cavity geometry",cavity_geometry);
                if( !err ) {
                    EGS_BaseGeometry::setActiveGeometryList(app_index);
                    cgeom = EGS_BaseGeometry::getGeometry(cavity_geometry);
                    if( !cgeom ) egsWarning("\n\n********** no geometry named"
                       " %s exists => using region-by-region rejection only\n");
                }
                if( !Esave && rr_flag == 1 ) {
                    egsWarning("\n\n********* rr_flag = 1 but Esave = 0 =>"
                         " not using range rejection\n\n");
                    rr_flag = 0;
                }
                if( rr_flag && cgeom ) {
                    string rej_medium; int irej_medium = -1;
                    err = rr->getInput("rejection range medium",rej_medium);
                    if( !err ) {
                        EGS_BaseGeometry::setActiveGeometryList(app_index);
                        int nmed = cgeom->nMedia();
                        int imed = cgeom->addMedium(rej_medium);
                        if( imed >= nmed ) egsWarning(
                           "\n\n*********** no medium"
                           " with name %s initialized => "
                           "using region-by-region rejection only\n",
                           rej_medium.c_str());
                        else irej_medium = imed;
                    }
                    if( irej_medium < 0 ) { cgeom = 0; rr_flag = 1; }
                    else {
                        //
                        // *** prepare an interpolator for the electron range
                        //     in the range rejection medium
                        //
                        int i = irej_medium; // save some typing
                        rho_rr = the_media->rho[i];
                        EGS_Float log_emin = i_ededx[i].getXmin();
                        EGS_Float log_emax = i_ededx[i].getXmax();
                        int nbin = 512;
                        EGS_Float dloge = (log_emax - log_emin)/nbin;
                        EGS_Float *erange = new EGS_Float [nbin];
                        EGS_Float *prange = new EGS_Float [nbin];
                        erange[0] = 0; prange[0] = 0;
                        EGS_Float ededx_old = i_ededx[i].interpolate(log_emin);
                        EGS_Float pdedx_old = i_pdedx[i].interpolate(log_emin);
                        EGS_Float Eold = exp(log_emin);
                        EGS_Float efak = exp(dloge);
                        for(int j=1; j<nbin; j++) {
                            EGS_Float elke = log_emin + dloge*j;
                            EGS_Float E = Eold*efak;
                            EGS_Float ededx = i_ededx[i].interpolate(elke);
                            EGS_Float pdedx = i_pdedx[i].interpolate(elke);
                            if( ededx < ededx_old )
                                erange[j] = erange[j-1]+1.02*(E-Eold)/ededx;
                            else
                                erange[j] = erange[j-1]+1.02*(E-Eold)/ededx_old;
                            if( pdedx < pdedx_old )
                                prange[j] = prange[j-1]+1.02*(E-Eold)/pdedx;
                            else
                                prange[j] = prange[j-1]+1.02*(E-Eold)/pdedx_old;
                            Eold = E; ededx_old = ededx; pdedx_old = pdedx;
                        }
                        rr_erange.initialize(nbin,log_emin,log_emax,erange);
                        rr_prange.initialize(nbin,log_emin,log_emax,prange);
                    }
                }
            }
            delete rr;
        }
        delete vr;
    }
    the_egsvr->i_do_rr = rr_flag;

    //
    // **** set up the pointer to the expmfp array in extra_stack
    //
     expmfp = the_extra_stack->expmfp;
    kexpmfp = the_extra_stack->kexpmfp;
     latchr = the_extra_stack->latchr;
    //
    // **** set up ausgab calls
    //
    int call;
    if( type == HVL ) {
        for(call=BeforeTransport; call<=UnknownCall; ++call)
            setAusgabCall((AusgabCall)call,false);
        return 0;
    }
    for(call=BeforeTransport; call<=ExtraEnergy; ++call)
        setAusgabCall((AusgabCall)call,true);
    for(call=AfterTransport; call<UnknownCall; ++call)
        setAusgabCall((AusgabCall)call,false);
    if( type != Dose || fsplit > 1 ) {
        //
        // For Fano and Awall calculations we wish to get informed about events
        // producing secondary photons so that we can discard them (Fano) or
        // mark them as secondary (Awall). If fsplit > 1 we also want to
        // Russian Roulette such photons. We don't need to call ausgab after
        // photon interactions as the entire photon transport is done in
        // selectPhotonMFP() and we discard/mark scattered photons there.
        //
        setAusgabCall(AfterBrems,true);
        if( the_xoptions->eii_flag ) {
            // with EII on, we may get fluorescent events after Moller/Bhabha
            setAusgabCall(AfterMoller,true);
            setAusgabCall(AfterBhabha,true);
        }
        setAusgabCall(AfterAnnihFlight,true);
        setAusgabCall(AfterAnnihRest,true);
    }

    if( fsplit <= 1 ) {
       setAusgabCall(AfterCompton,true);
       setAusgabCall(AfterRayleigh,true);
       setAusgabCall(AfterPhoto,true);
    }

    return 0;
}

void Cavity_Application::describeSimulation() {
    EGS_AdvancedApplication::describeSimulation();
    egsInformation("Variance reduction\n"
            "====================================================\n");
    egsInformation("Photon splitting = ");
    if( fsplit > 1 ) egsInformation("%g\n",fsplit);
    else egsInformation("off\n");
    egsInformation("Range rejection = ");
    if( rr_flag == 0 ) egsInformation("off\n");
    else if( rr_flag == 1 ) egsInformation("on for E < %g\n",Esave);
    else {
        egsInformation("Russian Roullette (RR)\n");
        egsInformation("    rejection in cavity for E < %g\n",Esave);
        egsInformation("    else RR with survival probability %g\n",
                1./rr_flag);
        if (cgeom)
            egsInformation("    rejection geometry is %s\n",
                cgeom->getName().c_str());
        else
            egsInformation("    Rejection geometry does not exist or input missing.\n"
                           "    Russian Roulette used on a region by region basis!\n");
        //egsInformation("    rejection medium is \n");
    }
    egsInformation("\n=============================================\n");
    egsInformation("         Calculation details\n");
    egsInformation("=============================================\n");
    egsInformation("Type = ");
    if( type == Fano ) egsInformation("Fano");
    else if( type == Awall ) egsInformation("Awall");
    else if( type == HVL ) egsInformation("HVL");
    else if( type == FAC ) egsInformation("FAC (Awall included)");
    else {
      egsInformation("Dose");
      if (flum){
        egsInformation("\n-> Charged particle fluence requested\n");
        if (flu_s){
           egsInformation("   between %g MeV <= E <= %g MeV with %d bins \n",
                        exp(flu_xmin),exp(flu_xmax),flu_nbin);
           egsInformation("   linearly interpolated on a log-scale of %g bin-width.\n",
                          1/flu_a);
        }
        else{
           egsInformation("   between %g MeV <= E <= %g MeV with %d bins of %g MeV width \n",
                        flu_xmin,flu_xmax,flu_nbin,1/flu_a);
        }
        if (flu_stpwr){
          if (flu_stpwr == stpwr)
             egsInformation("   O(eps^3) approach: accounts for change in stpwr\n"
                            "   along the step with eps=edep/Eb\n");
          else if (flu_stpwr == stpwrO5)
             egsInformation("   O(eps^5) approach: accounts for change in stpwr\n"
                            "   along the step with eps=edep/Eb\n");
        }
        else
          egsInformation("   Fluence calculated a-la-FLURZ using Lave=EDEP/TVSTEP.\n");
        if ( rr_flag > 0 ){
           if (flu_stpwr){
              egsWarning("\n***** Warning ****** \n"
                       " Using range rejection. Charged particle fluence\n"
                       " will be affected by the selection of ESAVE!\n"
                       " Make sure it reproduces a calculation without\n"
                       " range rejection within desired accuracy!\n"
                       "************************************************\n");
           }
           else{
              egsFatal("***** ERROR ****** \n"
                       "The selected method for charged particle fluence\n"
                       "calculation relies on the particle's step! Hence,\n"
                       "range rejection will produce wrong results\n"
                       "This is a fatal error. Aborted.\n"
                       "*************************************************\n");
           }
        }
      }
    }
    egsInformation("\n");
    if( type != HVL ) {
        for(int j=0; j<ngeom; j++) {
            egsInformation("Calculation geometry: %s\n",
                    geoms[j]->getName().c_str());
            geoms[j]->printInfo();
            for(int i=0; i<geoms[j]->regions(); i++) {
                if( is_cavity[j][i] ) {
                    egsInformation("  cavity region %d, medium = %d\n",
                            i,geoms[j]->medium(i));
                }
            }
            if(Rho) {
                egsInformation("  density of cavity medium = %g g/cm3\n",Rho[j]);
            }
        }
    }
    egsInformation("=============================================\n");
    if (type == FAC){
      for (int i=0;i<ngeom;i++){
        egsInformation("aperture regions for geometry %s :",
                       geoms[i]->getName().c_str());
        for(int j=0;j<geoms[i]->regions();j++){
          if (is_aperture[i][j]) egsInformation(" %d",j);
        }
        egsInformation("\n");
      }
    }

}


EGS_HVL::EGS_HVL( vector<EGS_Float> thickness,
                  vector<double> kr,
                  vector<double> dkr){

    /*Input values*/
    x  = thickness;
    y  = kr;
    dy = dkr;

    npoints = dy.size();

    /*Initialize covariance matrix array*/
    C    = new double*[2];
    C[0] = new double[2];
    C[1] = new double[2];
    for (unsigned short i = 0; i < 2; i++){
      for (unsigned short j = 0; j < 2; j++){C[i][j] = 0.0;}
    }

    leastSquaresFit();

}

void EGS_HVL::leastSquaresFit(){

    double xmean = 0.0, ymean = 0.0;// arithmetic means
    double xsigma2 = 0.0;           //x mean square deviation
    double sumx2 = 0.0, sumxy = 0.0, covxy = 0.0;

    for (unsigned short j = 0; j < npoints; j++){
        xmean += x[j]; ymean += y[j];
        sumx2 += x[j]*x[j]; sumxy += x[j]*y[j];
    }
    xmean  /= npoints; ymean /= npoints;
    xsigma2 = sumx2/npoints - xmean*xmean;
    covxy   = sumxy/npoints - xmean*ymean;
    /*--------------------------------------*/
    /* straight line parameters */
    /*--------------------------------------*/
    m = covxy/xsigma2;    // slope
    b = ymean - m * xmean;// y-shift

    /*--------------------------------------*/
    /*Error estimation for least squares fit*/
    /*--------------------------------------*/

    /* compute the denominator */
    double den;
    double sidy2   = 0.0;// sum of 1/dy^2
    double sxidy2  = 0.0;// sum of x/dy^2
    double sx2idy2 = 0.0;// sum of x^2/dy^2
    for (unsigned short i = 0; i < npoints; i++){
        sidy2   +=       1.0/dy[i]/dy[i];
        sxidy2  +=      x[i]/dy[i]/dy[i];
        sx2idy2 += x[i]*x[i]/dy[i]/dy[i];
    }
    den = sx2idy2 - sxidy2*sxidy2/sidy2;
    /*if denominator not 0, get covariance matrix*/
    if ( den != 0.0 ){
       C[0][0] = sx2idy2/sidy2/den;
       C[1][1] = 1.0/den;
       C[0][1] = -sxidy2/sidy2/den;
       C[1][0] = C[0][1];
    }

    /*--------------------------------------*/
    /* uncertainty in straight line parameters */
    /*--------------------------------------*/
    if (C[0][0]>=0.0) db = sqrt(C[0][0]); // error in y-shift
    if (C[1][1]>=0.0) dm = sqrt(C[1][1]); // error in slope

    getHVL(m,dm,b,db);
}

void EGS_HVL::getHVL(double m, double dm, double b, double db){
     /*half value layer estimate*/
     hvl_fit = (0.5-b)/m;
     /*half value layer error estimate*/
     dhvl_fit  = C[0][0]/m/m + pow((0.5-b),2)*C[1][1]/pow(m,4);
     dhvl_fit += 2.0*hvl_fit*C[1][0]/pow(m,2);
     if (dhvl_fit >= 0.0 ) dhvl_fit = sqrt(dhvl_fit);
     else                  dhvl_fit = 100.0;
}

void EGS_HVL::printFittedHVL(){

     char c = '%';

     egsInformation("\n\n====================================\n"
                    "HVL from least squares fit to a line\n"
                    "          y = m*x + b               \n"
                    "====================================\n\n");
     egsInformation("HVL = %g +/- %g [%g %c]\n",
                   hvl_fit, dhvl_fit,100.0*dhvl_fit/hvl_fit,c);
     egsInformation("m   = %g +/- %g \n", m, dm);
     egsInformation("b   = %g +/- %g \n", b, db);
     /*compute Chi-square of the fit for n-2 dof*/
     egsInformation("\n---> Chi-square for %u degrees of freedom\n",
                    npoints-2);
     double Chi21 = 0.0; double Chi22 = 0.0; double Chi23 = 0.0;
     for (unsigned short i = 0; i < npoints; i++){
         Chi21 += pow((y[i] - m*x[i] - b),2)/dy[i]/dy[i];
         Chi22 += pow((y[i] - m*x[i] - b),2)/(pow(dm*x[i],2) + db*db);
         Chi23 += pow((y[i] - m*x[i] - b),2);
     }
     egsInformation("Chi2(%u) = %10.5le",npoints-2,Chi21/(npoints-2));
     egsInformation(" <= with measurement errors\n");
     egsInformation("Chi2(%u) = %10.5le",npoints-2,Chi22/(npoints-2));
     egsInformation(" <= with fit errors\n");
     egsInformation("Chi2(%u) = %10.5le",npoints,Chi23);
     egsInformation(" <= as in xmgrace (total square deviation)\n");
/*
     //compute correlation coefficient between measurements and fit
     double covfy = 0.0; double meany = 0.0; double meanf = 0.0;
                         double meany2= 0.0; double meanf2= 0.0;
     for (unsigned short i = 0; i < npoints; i++){
         meany  += y[i];    // sum
         meanf  += m*x[i]+b;//sum
         meany2 += y[i]*y[i];      // sum of squares
         meanf2 += pow(m*x[i]+b,2);// sum of squares
         covfy  += y[i]*(m*x[i]+b);// sum of product y*f(x)
     }
     // unbiased estimators of second moments var and cov
     covfy = (npoints*covfy - meanf*meany)/npoints/(npoints-1);
     double vary = 0.0; double varf = 0.0;
     vary = (npoints*meany2 - meany)/npoints/(npoints-1);
     varf = (npoints*meanf2 - meanf)/npoints/(npoints-1);
     double corr = covfy/sqrt(vary*varf);
     egsInformation("\ncorrelation coefficient = %10.5le\n",corr);
*/
     egsInformation("\ncorrelation coefficient corr[m,b] = %10.5le\n",
                    C[0][1]/sqrt(C[0][0]*C[1][1]));
}

void EGS_HVL::printCovarianceMatrix(){
     egsInformation("\n\n----------------------------\n"
                    "Covariance matrix of m and b\n"
                    "----------------------------\n\n");
     egsInformation("|                            |\n");
     egsInformation("| %+-10.5le  %+-10.5le |\n",C[0][0], C[0][1]);
     egsInformation("| %+-10.5le  %+-10.5le |\n",C[1][0], C[1][1]);
     egsInformation("|                            |\n");
     egsInformation("\n----------------------------\n");
}


void EGS_HVL::recursiveIteration(){

     /* to be added ...*/

}

#ifdef BUILD_APP_LIB
APP_LIB(Cavity_Application);
#else
APP_MAIN(Cavity_Application);
#endif
