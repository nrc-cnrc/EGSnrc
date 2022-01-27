/*
###############################################################################
#
#  EGSnrc egs++ fluence scoring object header
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
#  Author:          Ernesto Mainegra-Hing, 2022
#
#  Contributors:    
#
###############################################################################

/*! \file egs_fluence_scoring.h
    \brief A fluence scoring object : header
  
\ingroup AusgabObjects


NEEDS UPDATING ... !

This ausgab object can be used to score particle fluence during 
run time and to output this information into a file
This ausgab object is specified via
\verbatim
:start ausgab object:
    library = egs_fluence_scoring
    name    = some_name
    score photons   = yes or no # optional, yes assumed if missing
    score electrons = yes or no # optional, yes assumed if missing
    score positrons = yes or no # optional, yes assumed if missing
    start scoring   = event_number # optional, 0 assumed if missing
    stop  scoring   = event_number # optional, 1024 assumed if missing
    buffer size     = size         # optional, 1024 assumed if missing
    file name addition = some_string # optional, empty string assumed if missing
:stop ausgab object:
\endverbatim
The output file name is normally constructed from the output file name, 
the string specified by <code>file name addition</code> (if present and not empty), 
and <code>_wJob</code> in case of parallel runs. The extension given is <code>ptracks</code>. 
Using <code>start scoring</code> and <code>stop scoring</code> one can select a 
range of histories for which to score the particle track info. One can also 
select specific particle type(s). 
 */

#ifndef EGS_FLUENCE_SCORING_
#define EGS_FLUENCE_SCORING_

#include "egs_ausgab_object.h"
#include "egs_transformations.h"
#include "egs_interpolator.h"
#include <egs_scoring.h>

#include <fstream>
using namespace std;


#ifdef WIN32

#ifdef BUILD_FLUENCE_SCORING_DLL
#define EGS_FLUENCE_SCORING_EXPORT __declspec(dllexport)
#else
#define EGS_FLUENCE_SCORING_EXPORT __declspec(dllimport)
#endif
#define EGS_FLUENCE_SCORING_LOCAL 

#else

#ifdef HAVE_VISIBILITY
#define EGS_FLUENCE_SCORING_EXPORT __attribute__ ((visibility ("default")))
#define EGS_FLUENCE_SCORING_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define EGS_FLUENCE_SCORING_EXPORT
#define EGS_FLUENCE_SCORING_LOCAL
#endif

#endif

/*! Field type */
enum FieldType { circle=0, rectangle=1 };

/*! Particle type */
enum ParticleType { electron = -1, photon = 0, positron = 1 };

/*! Charged particle fluence calculation type */
enum eFluType { flurz=0, stpwr=1, stpwrO5=2 };

class EGS_AdvancedApplication;

class EGS_FLUENCE_SCORING_EXPORT EGS_PlanarFluence : public EGS_AusgabObject {

public:
   /*! Constructors */
   EGS_PlanarFluence(const string &Name="", EGS_ObjectFactory *f = 0);
   /*! Destructor.  */
  ~EGS_PlanarFluence();
   EGS_Float area(){return Area;};
   //bool needsCall(EGS_Application::AusgabCall iarg) const { return true; };
   bool needsCall(EGS_Application::AusgabCall iarg) const {
       if (iarg == EGS_Application::BeforeTransport ||
           iarg == EGS_Application::AfterTransport ){
           return true;
       }
       else {
           return false;
       }
   };

   inline int hitsField(const EGS_Particle& p, EGS_Float* dist);
   inline void    score(const EGS_Particle& p, const int& ivoxel);
   void describeMe();//!< Sets fluence scoring object \c description
   void initScoring(EGS_Input *inp);
   void setApplication(EGS_Application *App);
   void ouputResults();
   void reportResults();
   int processEvent(EGS_Application::AusgabCall iarg) {

       int q = app->top_p.q;

       if (q != scoring_charge ) return 0;

       /* Quantify photon contribution to scoring field */
       if( iarg == EGS_Application::BeforeTransport ){
           ixy = hitsField(app->top_p,&distance);
           if (ixy >= 0){
              x0 = app->top_p.x; hits_field = true; 
           }
           else{hits_field = false; }
       }

       if( iarg == EGS_Application::AfterTransport && hits_field ){
           EGS_Vector xstep = app->top_p.x - x0;
           if (xstep.length() >= distance){// crossed scoring field
              //if (!app->top_p.latch ) m_primary += app->top_p.wt;
              m_tot  += app->top_p.wt;
              score( app->top_p, ixy );
           }
           hits_field = false;
       }

       return 0;

   };
   void setCurrentCase(EGS_I64 ncase) {
        if( ncase != current_ncase ) {
            current_ncase = ncase;
            if (flu){
              for (int j = 0; j < Nx*Ny; j++)
                   flu[j]->setHistory(ncase);
              fluT->setHistory(ncase);
            }
        }
   };
   void resetCounter(){
      current_ncase = 0; 
      if (flu){
        for (int j = 0; j < Nx*Ny; j++)
             flu[j]->reset();
        fluT->reset();
      }
   }
   bool storeState(ostream &data) const;
   bool setState(istream &data);
   bool addState(istream &data);

private:   

  FieldType field_type;

  ParticleType scoring_charge;

  string particle_name;

  EGS_ScoringArray** flu;  
  EGS_ScoringArray*  fluT;  
  /* Circular scoring field parameters */  
  EGS_Vector      m_normal,
                  m_midpoint,
                  ux, uy;
  EGS_Vector      x0;
  EGS_Float     m_R, m_R2, // scoring field
                norm_u;         // User normalization
  /* Rectangular field parameters */  
  EGS_Float     ax, ay, vx, vy;
  int           Nx, Ny, n_sensitive_regs, ixy;
  /* Energy grid inputs */
  EGS_Float Area, flu_a,
                  flu_b,
                  flu_xmin,
                  flu_xmax;
  int             flu_s,
                  flu_nbin;
  
  EGS_Float     m_d;       // distance from origin to center of scoring field
  EGS_Float     distance; // distance to scoring field along particle's direction
  EGS_Float     m_primary, 
                m_tot;
//  int fluence_scoring, energy_scoring;
  bool     hits_field;
  EGS_I64  current_ncase;                  

  bool verbose;

};

class EGS_FLUENCE_SCORING_EXPORT EGS_VolumetricFluence : public EGS_AusgabObject {

public:
   /*! Constructors */
   EGS_VolumetricFluence(const string &Name="", EGS_ObjectFactory *f = 0);

   /*! Destructor.  */
  ~EGS_VolumetricFluence();

   bool needsCall(EGS_Application::AusgabCall iarg) const {
       if (iarg == EGS_Application::BeforeTransport ||
           iarg == EGS_Application::UserDiscard ){
           return true;
       }
       else {
           return false;
       }
   };

   void describeMe();//!< Sets fluence scoring object \c description

   void initScoring(EGS_Input *inp);

   void setApplication(EGS_Application *App);

   void ouputResults();

   void reportResults();

   int processEvent(EGS_Application::AusgabCall iarg) {

       int q = app->top_p.q;
       if ( q != scoring_charge ) return 0;

       int ir = app->top_p.ir;
       if ( ir < 0 || !is_sensitive[ir] ) return 0;

       /* Scoring photon about to be transported in geometry */
       if ( !q && iarg == EGS_Application::BeforeTransport ) {
          /* Track-Length scoring (classic) */
          EGS_Float e = app->top_p.E,
          wtstep  = app->top_p.wt*app->getTVSTEP();
          if (flu_s) {
               e = log(e);
          }
          EGS_Float ae;
          int je;
          if (e > flu_xmin && e <= flu_xmax) {
              ae = flu_a*e + flu_b;
              je = min((int)ae,flu_nbin-1);
              EGS_ScoringArray *aux = flu[ir];
              aux->score(je,wtstep);
              fluT->score(ir,wtstep);
          }
          return 0;
       }

       EGS_Float edep = app->getEdep();

       /* Scoring charged particle about to be transported in geometry */
       if ( q && edep &&
          ( iarg == EGS_Application::BeforeTransport || 
            iarg == EGS_Application::UserDiscard )   ){
          /***** Initialization *****/
          EGS_Float Eb = app->top_p.E - app->getRM(),
                    Ee = Eb - edep,
                    weight = app->top_p.wt;
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
              else { xb = flu_xmax; ab = 1; jb = flu_nbin - 1; }
              /* Fraction of the final bin covered */
              if( xe > flu_xmin ) {
                  ae = flu_a*xe + flu_b; je = (int) ae;
                  /* Variable bin-width for log scale*/
                  if (flu_s){
                     ae = (Ee*a_const[je]-1)*r_const;
                  }
                  else{ ae -= je; }
              }
              else { xe = flu_xmin; ae = 0; je = 0; }// extends below Emin

#ifdef DEBUG 
              //egsInformation("iarg = %d jb = %d je = %d edep = %g MeV weight = %g\n",iarg,jb,je,edep,weight);                    
              if (jb == je) 
                one_bin++;
              else{          
                multi_bin++;
                //egsInformation("\niarg = %d jb = %d je = %d edep = %g MeV ab = %g ae = %g",iarg,jb,je,edep,ab,ae);                    
              }
              binDist->score(jb-je,weight);
#endif
              EGS_ScoringArray *aux = flu[ir];
              EGS_ScoringArray *auxT = fluT;

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
                 int imed = app->getMedium(ir);
                 // Initial and final energies in same bin
                 EGS_Float step;
                 if( jb == je ){
                     step = weight*(ab-ae)*getStepPerFraction(imed,xb,xe);
                     aux->score(jb,step);
                     if (flu_s)
                        auxT->score(ir,step*DE[jb]);
                     else
                        auxT->score(ir,step);
                 }
                 else {
                     //EGS_Float flu_a_i = 1/flu_a;
                     // First bin
                     Ee = flu_xmin + jb*flu_a_i; Eb=xb;
                     step = weight*ab*getStepPerFraction(imed,Eb,Ee);
                     aux->score(jb,step);
                     if (flu_s)
                        auxT->score(ir,step*DE[jb]);
                     else
                        auxT->score(ir,step);
                     // Last bin
                     Ee = xe; Eb = flu_xmin+(je+1)*flu_a_i;
                     step = weight*(1-ae)*getStepPerFraction(imed,Eb,Ee);
                     aux->score(je,step);
                     if (flu_s)
                        auxT->score(ir,step*DE[je]);
                     else
                        auxT->score(ir,step);
                     // intermediate bins
                     for(int j=je+1; j<jb; j++){
                        if (flu_stpwr == stpwrO5){
                          Ee = Eb; Eb = flu_xmin + (j+1)*flu_a_i;
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
                         step = weight*Lmid_i[j + imed*flu_nbin];
                        }
                        aux->score(j,step);
                        if (flu_s)
                          auxT->score(ir,step*DE[j]);
                        else
                          auxT->score(ir,step);
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
                 EGS_Float step, wtstep = weight*app->getTVSTEP()/edep;
                 // Initial and final energies in same bin
                 if( jb == je ){
                   step = wtstep*(ab-ae);
                   aux->score(jb,step);
                   if (flu_s)
                     auxT->score(ir,step*DE[jb]);
                   else
                     auxT->score(ir,step);
                 }
                 else {
                     // First bin
                     step = wtstep*ab;
                     aux->score(jb,step);
                     if (flu_s)
                       auxT->score(ir,step*DE[jb]);
                     else
                       auxT->score(ir,step);
                     // Last bin
                     step = wtstep*(1-ae);
                     aux->score(je,step);
                     if (flu_s)
                       auxT->score(ir,step*DE[je]);
                     else
                       auxT->score(ir,step);
                     // intermediate bins
                     for(int j=je+1; j<jb; j++){
                         aux->score(j,wtstep);
                         if (flu_s)
                           auxT->score(ir,wtstep*DE[j]);
                         else
                           auxT->score(ir,wtstep);
                     }
                 }
              }
          }
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
    EGS_Float getStepPerFraction( const int & imed,
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

        EGS_Float dedxmid_i = dedx_i[imed].interpolate(lnEmid);
        // Used in cavity:
        // EGS_Float dedxmid_i = 1./i_dedx[imed].interpolate(lnEmid);
#ifdef DEBUG
if (!isfinite(dedxmid_i)){
  if (isnan(dedxmid_i))
    egsInformation("\n Is NaN? dedxmid_i = %g",dedxmid_i);
  else    
    egsInformation("\n Is infinite? dedxmid_i = %g",dedxmid_i);
}
else{
  if (dedxmid_i<0){
    egsInformation("\n Is negative? dedxmid_i = %g Emid = %g lnEmid = %g index = %d",
    dedxmid_i,exp(lnEmid),lnEmid, dedx_i[imed].getIndex(lnEmid) );
  }
  if (dedxmid_i > 1.E10)
    egsInformation("\n Is very large? dedxmid_i = %g Emid = %g lnEmid = %g",dedxmid_i,exp(lnEmid),lnEmid);
      
}
#endif        
        /* O(eps^3) approach */
        if (flu_stpwr == stpwr) return dedxmid_i;
        
        /* O(eps^5) approach */
        EGS_Float b = i_dedx[imed].get_b(i_dedx[imed].getIndexFast(lnEmid));
        EGS_Float aux = b*dedxmid_i;
        aux = aux*(1+2*aux)*pow(eps/(2-eps),2)/6;
        //aux = aux*(1+2*aux)*eps*eps/((2-eps)*(2-eps))*0.16666666667;
        stpFrac = dedxmid_i*(1+aux);
        return stpFrac;
    }


   void setCurrentCase(EGS_I64 ncase) {
        if( ncase != current_ncase ) {
            current_ncase = ncase;
            if (flu){
              for (int j = 0; j < nreg; j++){
                  if ( is_sensitive[j] ) 
                     flu[j]->setHistory(ncase);
              }
              fluT->setHistory(ncase);
            }
        }
#ifdef DEBUG
        binDist->setHistory(ncase);
#endif        

   };
   void resetCounter(){
      current_ncase = 0; 
      if (flu){
        for (int j = 0; j < nreg; j++)
            if ( is_sensitive[j] ) 
               flu[j]->reset();
        fluT->reset();
      }
#ifdef DEBUG
      binDist->reset();
#endif
   }
   void getNumberRegions(const string &str, vector<int> &regs);

   void getLabelRegions(const string &str, vector<int> &regs);
   
   bool storeState(ostream &data) const;
   
   bool setState(istream &data);
   
   bool addState(istream &data);

private:   

  ParticleType scoring_charge;

  string particle_name;

  EGS_I64  current_ncase;                  

  /* Fluence Scoring Arrays */
  EGS_ScoringArray** flu;  
  EGS_ScoringArray*  fluT;  

  /*******************************************/
  /* Charged particle fluence: Required data */
  /*******************************************/
  EGS_Interpolator* i_dedx; // stopping power for each medium
  EGS_Interpolator* dedx_i; // inverse stopping power for each medium
  EGS_Float*        Lmid_i; // pre-computed inverse of bin midpoint stpwr
  eFluType       flu_stpwr; // flurz   => ave. stpwr = edep/tvstep,
                            // stpwr   => 3rd order in edep/Eb,
                            // stpwrO5 => 5th order in edep/Eb
  /**************************************************************
   * Parameters required for calculations on a log-scale
   * The main issue here is that the bin width is not constant
   *************************************************************/
   EGS_Float       r_const;    // inverse of (Emax/Emin)**1/flu_nbin - 1 = exp(binwidth)-1
   EGS_Float      *a_const;    // constant needed to determine bin fractions on log scale
   EGS_Float      *DE;         // bin width of logarithmic scale
  /*****************************************************************/

  vector<EGS_Float> volume;// volume of each scoring region
  int       active_region;    // Region showing calculation progress
  int       n_scoring_regions; // number of scoring regions
  int       nreg;              // regions in geometry
  int max_reg ;             // maximum scoring region number
  vector<bool> is_sensitive;// flag scoring regions
  EGS_Float norm_u;         // User normalization
  /* Energy grid inputs */
  EGS_Float flu_a, flu_a_i,
            flu_b,
            flu_xmin,
            flu_xmax;
  int       flu_s,
            flu_nbin;
  /* Classification variables */
  EGS_Float m_primary, 
            m_tot;
  /* Auxiliary input variables*/
  vector <EGS_Float> vol_list;      // Input list of region volumes
  vector <int>       f_region;      // Input list of scoring regions
  string             f_regionsString;// Input string of scoring regions or labels

#ifdef DEBUG
  /* Debugging information */
  int one_bin, multi_bin;
  EGS_ScoringArray *binDist;  
#endif

  bool verbose;

};

#endif
