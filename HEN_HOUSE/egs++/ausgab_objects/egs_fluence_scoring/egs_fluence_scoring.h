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
# 
# Ausgab objects (AOs) for fluence scoring in arbitrary geometrical regions or at 
# circular  or rectangular scoring fields located anywhere in space for a specific 
# particle type. Fluence can be scored for multiple particle types by definining 
# different AOs. An option exists for scoring the fluence of primary particles. 
# Classification into primary and secondary particles follows the definition used 
# in FLURZnrc for IPRIMARY = 2 (primaries) and IPRIMARY = 4 (secondaries).
#
############################################################################### 
*/

/*! \file egs_fluence_scoring.h
 *  \brief A fluence scoring object : header 
 *  \EM
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
enum ParticleType { electron = -1, photon = 0, positron = 1, unknown = -99 };

/*! Charged particle fluence calculation type */
enum eFluType { flurz=0, stpwr=1, stpwrO5=2 };

/*! \brief Base class for fluence scoring. 

  \ingroup AusgabObjects

  Contains the basic ingredients for fluence scoring such as energy grid,
  particle type, scoring regions, and whether to score primary fluence. 
  Provides the method for defining primary and secondary particles.

  \todo Total kerma scoring
  \todo Account for multiple app geometries
  \todo Fluence for any particle type?
*/
class EGS_FLUENCE_SCORING_EXPORT EGS_FluenceScoring : public EGS_AusgabObject {

public:
   /*! Constructors */
   EGS_FluenceScoring(const string &Name="", EGS_ObjectFactory *f = 0);
   /*! Destructor.  */
  ~EGS_FluenceScoring();

   void initScoring(EGS_Input *inp);

   void getSensitiveRegions(EGS_Input *inp);

   void getNumberRegions( const string &str, vector<int> &regs );

   void getLabelRegions( const string &str, vector<int> &regs );

   void setUpRegionFlags();

   void describeMe();

   int getDigits(int i) {
        int imax = 10;
        while (i>=imax) {
            imax*=10;
        }
        return (int)log10((float)imax);
   };

   void flagSecondaries( const int &iarg, const int &q ){
       int npold = app->getNpOld(), 
              np = app->getNp();
       if ( scoring_charge ){
          /*************************************************************** 
           DEFAULT: 
           FLURZnrc IPRIMARY = 2 (primaries) IPRIMARY = 4 (secondaries)
               Secondary charged particles defined as those resulting 
               from charged particle interactions, atomic relaxations 
               following EII, brems and annihilation photons. 

           OPTIONAL:
           FLURZnrc IPRIMARY = 1 (e- from brems as primaries)
               Set `source particle` to 'photon' in the input file to not flag
               brems photons so that when scoring charged particle fluence in 
               photon beams, first generation e- are primaries. This is
               implicit for photon interactions when scoring charged particle 
               fluence. But one must explicitly account for that during brems events.
          ****************************************************************/
          if ( iarg == EGS_Application::AfterBrems       || 
               iarg == EGS_Application::AfterMoller      ||
               iarg == EGS_Application::AfterAnnihFlight || 
               iarg == EGS_Application::AfterAnnihRest   ){
               /************************************************************************ 
                Skip block below for a photon beam. First generation e- are primaries.
                This will apply to ALL brems events in a photon beam simulation. One
                could fine tune it to only brems events in certain regions, for instance
                a bremsstrahlung target, by using the is_source flag for those regions.
               *************************************************************************/
               if (!(iarg == EGS_Application::AfterBrems && source_charge == photon)){
                 for(int ip = npold+1; ip <= np; ip++) {
                    app->setLatch(ip,1);
                 }
               }
          }
          else if ( iarg == EGS_Application::AfterBhabha ){
               if (q == -1) app->setLatch(np,1);
               else         app->setLatch(np-1,1);
          }
       }
       else{
          /*************************************************************** 
           FLURZnrc IPRIMARY = 3
              Flag scattered photons, secondaries, and relaxation 
              particles as secondaries
          ****************************************************************/
          if ( iarg == EGS_Application::AfterPair     || 
               iarg == EGS_Application::AfterCompton  || 
               iarg == EGS_Application::AfterPhoto    || 
               iarg == EGS_Application::AfterRayleigh ){
               for(int ip = npold; ip <= np; ip++) {
                  app->setLatch(ip,1);
               }
          }
       }
     
   };
   
protected:   

  EGS_I64 current_ncase;                  

  ParticleType scoring_charge;// charge of scored particles
  ParticleType source_charge; // charge of source particles

  /* Fluence Scoring Arrays */
  EGS_ScoringArray **flu;   // differential fluence: primaries + secondaries
  EGS_ScoringArray **flu_p; // differential fluence: primaries only 
  EGS_ScoringArray  *fluT;  // Total fluence: primaries + secondaries
  EGS_ScoringArray  *fluT_p;// Total fluence: primaries only

  /* Regions flags */
  vector<bool> is_sensitive;     // flag scoring regions
  vector<bool> is_source;        // Flag regions such as brems target or radiactive source
                                 // Interacting particles not subjected to classification
  vector <int> f_start, f_stop;  // Markers for group regions input
  vector <int> f_region;         // Input list of scoring regions
  vector <int> s_region;         // Input list of source regions
  string       f_regionsString;  // Input string of scoring regions or labels
  string       s_regionsString;  // Input string of source regions or labels
  int          n_scoring_regions;// number of scoring regions
  int          n_source_regions; // number of source regions
  int          nreg;             // regions in geometry
  int          max_reg;          // maximum scoring region number
  int          active_region;    // Region showing calculation progress
  bool score_in_all_regions;
  bool source_in_all_regions;

  EGS_Float norm_u;              // User normalization

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

  string  particle_name;
  /* Auxiliary input variables*/
  bool    verbose, 
          score_spe,
          score_primaries;
};

/*! \brief Ausgab object for scoring fluence at circular or rectangular fields

  \ingroup AusgabObjects

  A linear track-length estimator in the zero-thickness limit is used to 
  compute fluence for either a circular field or a rectangular screen of 
  arbitrary resolution (pixels). Rectangular fields are by default at Z = 0 
  directed along the positive z-axis. An affine transofrmation can be included
  to place a rectangular field anywhere in space. User can request to score primary 
  fluence as well as differential fluence. If fluence for more than one particle type 
  is desired, multiple AOs are required. 
  
  For charged particle fluence scoring this method can be inefficient as 
  it checks at every single step whether a charged particle is aimed at scoring field.
  To improve the efficieny for charged particles, one has the option to define contributing regions 
  from where to score. However, users must be careful to select all regions from where
  charged particles can cross the scoring field. This option has the added benefit of 
  allowing to estimate the contribution to the fluence from specific regions in the geometry.

  To define an EGS_PlanarFluence AO use the syntax below:
  \verbatim
:start ausgab object:
    name    = id-string            # Arbitrary identifying string
    library = egs_fluence_scoring  # Library name
    type    = planar               # Score on circular or square field
    scoring particle = photon, or electron, or positron
    source particle  = photon, or electron, or positron 
                                   # Optional. Only required to score primary fluence.
                                   # Defaults to source particles if all the same.
                                   # In the case of multiple particles, 
                                   # defaults to scoring particle. Useful for
                                   # bremsstrahlung targets and radioactive sources.
    score primaries = yes or no    # Defaults to `no`.
    score spectrum  = yes or no    # Defaults to `no`.
    verbose         = yes or no    # Defaults to `no`.
    normalization   = norm         # User-requested normalization. Defaults to 1.
    #########
    # If scoring spectrum, define energy grid
    # Default: 128 linear energy bins between 1 keV and 1 MeV
    #########
    :start energy grid:
      number of bins = nbins
      minimum kinetic energy = Emin
      maximum kinetic energy = Emax
      scale = linear or logarithmic # Defaults to `linear`.
    :stop energy grid:
    ########
    # Define scoring based on type
    ########
    :start planar scoring:
       # Define contributing regions
       contributing regions = ir1 ir2 ... irn
       ### Alternatively:
       # start contributing region = iri_1, iri_2, ..., iri_n
       # stop contributing region =  irf_1, irf_2, ..., irf_n
       ###
       ################################
       # If a circular field desired:
       ################################
       scoring circle = x y z R
       scoring plane normal = ux uy uz
       ########################################################
       # If a rectangular field desired:
       #
       #scoring rectangle = xmin xmax ymin ymax
       #####
       # See documentation for EGS_AffineTransform
       #####
       #:start transformation:
       #   rotation = 2, 3 or 9 floating point numbers
       #   translation = tx, ty, tz
       #:stop transformation:
       ##########################################################
    :stop planar scoring:
:stop ausgab object:
  \endverbatim

  \todo Store results in a 2D binary file for visualization
*/
class EGS_FLUENCE_SCORING_EXPORT EGS_PlanarFluence : public EGS_FluenceScoring {

public:
   /*! Constructors */
   EGS_PlanarFluence(const string &Name="", EGS_ObjectFactory *f = 0);
   /*! Destructor.  */
  ~EGS_PlanarFluence();
   EGS_Float area(){return Area;};
   bool needsCall(EGS_Application::AusgabCall iarg) const {
       if (iarg == EGS_Application::BeforeTransport ||
           iarg == EGS_Application::AfterTransport ){
           return true;
       }
       else if ( score_primaries && 
              (iarg == EGS_Application::AfterPair        || 
               iarg == EGS_Application::AfterCompton     || 
               iarg == EGS_Application::AfterPhoto       || 
               iarg == EGS_Application::AfterRayleigh    ||
               iarg == EGS_Application::AfterBrems       || 
               iarg == EGS_Application::AfterMoller      ||
               iarg == EGS_Application::AfterBhabha      || 
               iarg == EGS_Application::AfterAnnihFlight || 
               iarg == EGS_Application::AfterAnnihRest   ))
       {
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
   void ouputPlanarFluence( EGS_ScoringArray *fT, const double &norma );
   void ouputResults();
   void reportResults();
   int processEvent(EGS_Application::AusgabCall iarg) {

       int q = app->top_p.q,
          ir = app->top_p.ir;

       if ( q == scoring_charge && ir >= 0 && is_sensitive[ir] )
       {

          /* Quantify contribution to scoring field */
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
       }

       /******************************************************************** 
        * Flag secondaries after interactions. Definition of secondaries
        * matches FLURZnrc. One could fine tune it by using the is_source 
        * flag to skip this block in certains regions such as brems targets,
        * radioactive sources, etc.
        * 
        * BEWARE: Latch set to 1 (bit 0) to flag secondaries.
        *         Other applications might use latch for other purposes!
        *********************************************************************/
       if ( score_primaries && ir >= 0 && !is_source[ir]){
          flagSecondaries( iarg, q );
       }

       return 0;

   };
   
   void setCurrentCase(EGS_I64 ncase) {
        if( ncase != current_ncase ) {
            current_ncase = ncase;

            fluT->setHistory(ncase);

            if (score_spe){
              for (int j = 0; j < Nx*Ny; j++)
                   flu[j]->setHistory(ncase);
            }

            if ( score_primaries ){
              fluT_p->setHistory(ncase);
              if (score_spe){
                 for (int j = 0; j < Nx*Ny; j++)
                     flu_p[j]->setHistory(ncase);
              }
            }
        }
   };

   void resetCounter(){
      current_ncase = 0; 
      fluT->reset();
      if (flu){
        for (int j = 0; j < Nx*Ny; j++)
             flu[j]->reset();
      }
      if ( score_primaries ){
         fluT_p->reset();
         if ( score_spe ){
            for (int j = 0; j < Nx*Ny; j++)
                flu_p[j]->reset();
         }
      }
   }
   bool storeState(ostream &data) const;
   bool setState(istream &data);
   bool addState(istream &data);

private:   

  FieldType field_type;

  EGS_Float Area;

  /* Circular scoring field parameters */  
  EGS_Vector      m_normal,
                  m_midpoint,
                  ux, uy;
  EGS_Vector      x0;
  EGS_Float       m_R, m_R2; // scoring field
  /* Rectangular field parameters */  
  EGS_Float     ax, ay, vx, vy;
  int           Nx, Ny, n_sensitive_regs, ixy;
  
  EGS_Float m_d;       // distance from origin to center of scoring field
  EGS_Float distance; // distance to scoring field along particle's direction
  bool      hits_field;

};

/*! \brief Ausgab object for scoring fluence in arbitrary geometry regions

  \ingroup AusgabObjects

  A linear track-length estimator is used to compute fluence in specific 
  geometrical regions. User can request to score primary fluence as well 
  as differential fluence. If fluence for more than one particle type is 
  desired, multiple AOs are required.

  Differential fluence for charged particle is calculated accounting for 
  continuos energy losses along the path using two methods. One method follows 
  the FLURZnrc implementation, whereby stopping power is assumed constant along 
  the step and estimated as the ratio EDEP/TVSTEP. The contributions to each 
  energy bin of width \f$\Delta E_i\f$ is obtained as the energy fraction 
  TVSTEP*\f$\Delta E_i\f$/EDEP.

  The second method follows the approach currently used in the EGSnrc 
  application \ref cavity "cavity", which accounts for stopping power 
  changes within a scoring energy bin. It uses a series expansion of the 
  integral of the inverse of the stopping power with respect to energy. 
  Stopping power is represented as a linear interpolation over a log energy grid. 
  A technical note is in preparation providing more details about this implementation.

  To define an EGS_VolumetricFluence AO use the syntax below:
  \verbatim
:start ausgab object:
    name    = id-string            # Arbitrary identifying string
    library = egs_fluence_scoring  # Library name
    type    = volumetric           # Score in a volume
    scoring particle = photon, or electron, or positron
    source particle  = photon, or electron, or positron 
                                   # Optional. Only required to score primary fluence.
                                   # Defaults to source particles if all the same.
                                   # In the case of multiple particles, 
                                   # defaults to scoring particle. Useful for
                                   # bremsstrahlung targets and radioactive sources.
    score primaries = yes or no    # Defaults to `no`.
    score spectrum  = yes or no    # Defaults to `no`.
    verbose         = yes or no    # Defaults to `no`.
    normalization   = norm         # User-requested normalization. Defaults to 1.
    # If scoring spectrum, define energy grid
    # Default: 128 linear energy bins between 1 keV and 1 MeV
    :start energy grid:
      number of bins = nbins
      minimum kinetic energy = Emin
      maximum kinetic energy = Emax
      scale = linear or logarithmic # Defaults to `linear`.
    :stop energy grid:
    :start volumetric scoring:
        scoring regions = ir1 ir2 ... irn
        ### Alternatively:
        #start region = iri_1, iri_2, ..., iri_n
        #stop region  = irf_1, irf_2, ..., irf_n
        ###
        volumes = V1, V2, ..., VN # Enter as many as scoring regions. If same number
                                  # of entries as group of regions, assumes groups of 
                                  # equal volume regions. If only one entry, assumes 
                                  # equal volumes in all regions. Defaults to 1.
        method  = flurz or stpwr or stpwrO5 # For charged particle scoring.
                  # 
                  # flurz   => FLURZnrc algorithm
                  # 
                  # Path length at each energy interval from energy
                  # deposited EDEP and total particle step TVSTEP. 
                  # Assumes stopping power constancy along the particle's
                  # step. It might introduce artifacts if ESTEPE or the
                  # scoring bin width are too large.
                  # 
                  # stpwr   => Accounts for stopping power variation 
                  #          along the particle's step. More accurate 
                  #          than method used in FLURZnrc albeit about
                  #          about 10% slower in electron beam cases.
                  #
                  # Uses an O(3) series expansion of the integral of the
                  # inverse of the stopping power with respect to energy. 
                  # Stopping power is represented as a linear interpolation 
                  # over a log energy grid.
                  # 
                  # stpwrO5 => Uses an O(5) series expansion. Slightly slower.
                  # 
                  # Defaults to `stpwr`.
    :stop volumetric scoring:
:stop ausgab object:
  \endverbatim
*/
class EGS_FLUENCE_SCORING_EXPORT EGS_VolumetricFluence : public EGS_FluenceScoring {

public:
   /*! Constructors */
   EGS_VolumetricFluence(const string &Name="", EGS_ObjectFactory *f = 0);

   /*! Destructor.  */
  ~EGS_VolumetricFluence();

   /*************************************************************** 
     NOTE: Primary particles defined as particles that suffer any 
           type of interaction, except for the slowing down of 
           charged particles in a medium.  
   ****************************************************************/
   bool needsCall(EGS_Application::AusgabCall iarg) const {
       if (iarg == EGS_Application::BeforeTransport ||
           iarg == EGS_Application::UserDiscard ){
           return true;
       }
       else if ( score_primaries && 
              (iarg == EGS_Application::AfterPair        || 
               iarg == EGS_Application::AfterCompton     || 
               iarg == EGS_Application::AfterPhoto       || 
               iarg == EGS_Application::AfterRayleigh    ||
               iarg == EGS_Application::AfterBrems       || 
               iarg == EGS_Application::AfterMoller      ||
               iarg == EGS_Application::AfterBhabha      || 
               iarg == EGS_Application::AfterAnnihFlight || 
               iarg == EGS_Application::AfterAnnihRest   ))
       {
             return true;
       }
       else {
           return false;
       }
   };

   void describeMe();//!< Sets fluence scoring object \c description

   void initScoring(EGS_Input *inp);

   void setApplication(EGS_Application *App);

   void ouputVolumetricFluence( EGS_ScoringArray *fT, const double &norma );
   
   void ouputResults();

   void reportResults();

   int processEvent(EGS_Application::AusgabCall iarg) {

    int    q = app->top_p.q,
          ir = app->top_p.ir, 
       latch = app->top_p.latch;

    if ( q == scoring_charge && ir >= 0 && is_sensitive[ir] ) {

       if ( !q ){// It's a photon
          /* Score photon fluence */
          if (iarg == EGS_Application::BeforeTransport ) {
             /* Linear track-Length scoring */
             EGS_Float wtstep  = app->top_p.wt*app->getTVSTEP();
             /* Score total fluence */
             fluT->score(ir,wtstep);
             if (score_primaries && !latch){
                fluT_p->score(ir,wtstep);
             }
             /* Score differential fluence */
             if ( score_spe ) {
                EGS_Float e = app->top_p.E;
                if (flu_s) {
                     e = log(e);
                }
                EGS_Float ae; int je;
                /* Score differential fluence */
                if ( e > flu_xmin && e <= flu_xmax ) {
                    ae = flu_a*e + flu_b;
                    je = min((int)ae,flu_nbin-1);
                    EGS_ScoringArray *aux = flu[ir];
                    aux->score(je,wtstep);
                    if (score_primaries && !latch){
                       flu_p[ir]->score(je,wtstep);
                    }
                }
             }
          }
       }
       else {// It's a charged particle

          EGS_Float edep = app->getEdep();
   
          /* Score charged particle fluence */
          if ( edep &&
             ( iarg == EGS_Application::BeforeTransport || 
               iarg == EGS_Application::UserDiscard )   ){

             /**************************/
             /***** Initialization *****/
             /**************************/
             EGS_Float weight = app->top_p.wt;
             bool score_p = score_primaries && !latch;
             /* Integral fluence scoring arrays */
             EGS_ScoringArray *auxT = fluT, *auxT_p;
             if ( score_p ){
                 auxT_p = fluT_p;
             }

#ifdef USETVSTEP
             EGS_Float a_step = weight*app->getTVSTEP();
             auxT->score(ir,a_step);
             if (score_p) 
                auxT_p->score(ir,a_step);
             if ( score_spe ){
#endif             

                 EGS_ScoringArray *aux, *aux_p;
#ifndef USETVSTEP
                 if ( score_spe ){
#endif             
                    aux = flu[ir];
                    if ( score_p ){
                       aux_p = flu_p[ir];
                    }
#ifndef USETVSTEP
                 }
#endif             

                 EGS_Float Eb = app->top_p.E - app->getRM(),
                           Ee = Eb - edep;
    
                 EGS_Float xb, xe;
                 if( flu_s ) {
                     xb = log(Eb);
                     if( Ee > 0 )
                         xe = log(Ee);
                     else xe = -15;
                 }
                 else{
                   xb = Eb; xe = Ee;
                 }
                 /**********************************************************/
                 /* If not out of bounds, proceed with rest of calculation */
                 /**********************************************************/
                 if( xb > flu_xmin && xe < flu_xmax ) {
                     EGS_Float ab, ae; int jb, je;
                     /* Fraction of the initial bin covered */
                     if( xb < flu_xmax ) {
                         ab = flu_a*xb + flu_b; jb = (int) ab;
                         /* Variable bin-width for log scale*/
                         if (flu_s){
                            ab = (Eb*a_const[jb]-1)*r_const;
                         }
                         else{ ab -= jb;}
                     }
                     else { // particle's energy above Emax
                         xb = flu_xmax; ab = 1; jb = flu_nbin - 1; 
                     }
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
                     if (jb == je) 
                       one_bin++;
                     else{          
                       multi_bin++;
                     }
    
                     binDist->score(jb-je,weight);
    
                     EGS_Float totStep = 0, the_step = app->getTVSTEP();
#endif                            
     
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
                     if ( flu_stpwr ){
                        int imed = app->getMedium(ir);
                        // Initial and final energies in same bin
                        EGS_Float step;
                        if( jb == je ){
                            step = weight*(ab-ae)*getStepPerEnergyLoss(imed,xb,xe);
#ifdef DEBUG
                                totStep += step;
#endif                            
                            /* Differential fluence */
                            if ( score_spe ){
                               aux->score(jb,step);
                               if (score_p)
                                  aux_p->score(jb,step);
                            }
#ifndef USETVSTEP
                            /* Integral fluence */
                            if (flu_s){
                               auxT->score(ir,step*DE[jb]);
                               if (score_p)
                                  auxT_p->score(ir,step*DE[jb]);
                            }
                            else{
                               auxT->score(ir,step);
                               if (score_p) 
                                  auxT_p->score(ir,step);
                            }
#endif                        
                        }
                        else {
    
                            // First bin
    
                            Ee = flu_xmin + jb*flu_a_i; Eb=xb;
                            step = weight*ab*getStepPerEnergyLoss(imed,Eb,Ee);
#ifdef DEBUG
                                totStep += step;
#endif                            
                            /* Differential fluence */
                            if ( score_spe ){
                               aux->score(jb,step);
                               if (score_p) 
                                  aux_p->score(jb,step);
                            }
#ifndef USETVSTEP
                            /* Integral fluence */
                            if (flu_s){
                               auxT->score(ir,step*DE[jb]);
                               if (score_p)
                                  auxT_p->score(ir,step*DE[jb]);
                            }
                            else{
                               auxT->score(ir,step);
                               if (score_p)
                                  auxT_p->score(ir,step);
                            }
#endif                            
    
                            // Last bin
    
                            Ee = xe; Eb = flu_xmin+(je+1)*flu_a_i;
                            step = weight*(1-ae)*getStepPerEnergyLoss(imed,Eb,Ee);
#ifdef DEBUG
                                totStep += step;
#endif                            
                            /* Differential fluence */
                            if ( score_spe ){
                               aux->score(je,step);
                               if (score_p)
                                  aux_p->score(je,step);
                            }
#ifndef USETVSTEP
                            /* Integral fluence */
                            if (flu_s){
                               auxT->score(ir,step*DE[je]);
                               if (score_p) 
                                  auxT_p->score(ir,step*DE[je]);
                            }
                            else{
                               auxT->score(ir,step);
                               if (score_p)
                                  auxT_p->score(ir,step);
                            }
#endif                            
    
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
                                 step = weight*getStepPerEnergyLoss(imed,Eb,Ee);
                               }
                               else{// use pre-computed values of 1/Lmid
                                step = weight*Lmid_i[j + imed*flu_nbin];
                               }
#ifdef DEBUG
                                totStep += step;
#endif                            
                               /* Differential fluence */
                               if ( score_spe ){
                                  aux->score(j,step);
                                  if (score_p) 
                                     aux_p->score(j,step);
                               }
#ifndef USETVSTEP
                               /* Integral fluence */
                               if (flu_s){
                                 auxT->score(ir,step*DE[j]);
                                 if (score_p)
                                    auxT_p->score(ir,step*DE[j]);
                               }
                               else{
                                 auxT->score(ir,step);
                                 if (score_p) 
                                    auxT_p->score(ir,step);
                               }
#endif                            
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
#ifdef DEBUG
                                totStep += step;
#endif                
                          /* Differential fluence */
                          if ( score_spe ){
                             aux->score(jb,step);
                             if (score_p) 
                                aux_p->score(jb,step);
                          }
#ifndef USETVSTEP
                          /* Integral fluence */
                          if (flu_s){
                            auxT->score(ir,step*DE[jb]);
                            if (score_p)
                               auxT_p->score(ir,step*DE[jb]);
                          }
                          else{
                            auxT->score(ir,step);
                            if (score_p) 
                               auxT_p->score(ir,step);
                          }
#endif                
                        }
                        else {
    
                            // First bin
    
                            step = wtstep*ab;
#ifdef DEBUG
                                totStep += step;
#endif                            
                            /* Differential fluence */
                            if ( score_spe ){
                               aux->score(jb,step);
                               if (score_p)
                                  aux_p->score(jb,step);
                            }
#ifndef USETVSTEP
                            /* Integral fluence */
                            if (flu_s){
                              auxT->score(ir,step*DE[jb]);
                              if (score_p)
                                 auxT_p->score(ir,step*DE[jb]);
                            }
                            else{
                              auxT->score(ir,step);
                              if (score_p)
                                 auxT_p->score(ir,step);
                            }
#endif                            
    
                            // Last bin
    
                            step = wtstep*(1-ae);
#ifdef DEBUG
                                totStep += step;
#endif                            
                            /* Differential fluence */
                            if ( score_spe ){
                               aux->score(je,step);
                               if (score_p)
                                  aux_p->score(je,step);
                            }
#ifndef USETVSTEP
                            /* Integral fluence */
                            if (flu_s){
                              auxT->score(ir,step*DE[je]);
                              if (score_p)
                                 auxT_p->score(ir,step*DE[je]);
                            }
                            else{
                              auxT->score(ir,step);
                              if (score_p) 
                                 auxT_p->score(ir,step);
                            }
#endif                            
                            // intermediate bins
                            for(int j=je+1; j<jb; j++){
#ifdef DEBUG
                                totStep += wtstep;
#endif                            
                                /* Differential fluence */
                                if ( score_spe ){
                                   aux->score(j,wtstep);
                                   if (score_p)
                                      aux_p->score(j,wtstep);
                                }
#ifndef USETVSTEP
                                /* Integral fluence */
                                if (flu_s){
                                  auxT->score(ir,wtstep*DE[j]);
                                  if (score_p)
                                     auxT_p->score(ir,wtstep*DE[j]);
                                }
                                else{
                                  auxT->score(ir,wtstep);
                                  if (score_p)
                                     auxT_p->score(ir,wtstep);
                                }
#endif                            
                            }
                        }
                     }
    
#ifdef DEBUG
                     EGS_Float edep_step = totStep*flu_a_i, diff = edep_step/the_step;
                     EGS_Float astep = step_a*the_step + step_b; int jstep = (int) astep;
                     if (jstep < 0 || jstep > n_step_bins)
                        egsFatal("\n**** EGS_VolumetricFluence::processEvent-> jstep = %d\n is out of bound!\n");
                     stepDist->score(jstep,app->top_p.wt);
                     relStepDiff->score(jstep,diff); eCases++;
#endif                 
                 }
#ifdef USETVSTEP
             }
#endif                 
          }
       }
    }

    /******************************************************************** 
     * Flag secondaries after interactions. Definition of secondaries
     * matches FLURZnrc. One could fine tune it by using the is_source 
     * flag to skip this block in certains regions such as brems targets,
     * radioactive sources, etc.
     * 
     * BEWARE: Latch set to 1 (bit 0) to flag secondaries.
     *         Other applications might use latch for other purposes!
     *********************************************************************/
    if ( score_primaries && ir >= 0 && !is_source[ir]){
       flagSecondaries( iarg, q );
    }

    return 0;

   };

    /*! Computes path per energy loss traveled by a charged particle when
     *  slowing down from Eb to Ee.
     *
     * Computes the path-length traveled while slowing down from energy Eb to energy
     * Ee, both energies falling in the same energy bin assuming full coverage.
     * The returned value should be multiplied by the fraction of the
     * energy bin covered by Eb-Ee to compute fluence per bin width.
     * If using a logarithmic energy interpolation, Eb and Ee are actually the
     * logarithms of the initial and final energies. The expression is based on
     * linear interpolation in a logarithmic energy grid as used in EGSnrc
     * (i.e. dedx = a + b*log(E) ) and a power series expansion of the ExpIntegralEi
     * function that is the result of the integration of the inverse of the stopping
     * power with respect to energy.
     */
    EGS_Float getStepPerEnergyLoss( const int & imed,
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
            fluT->setHistory(ncase);
            if ( score_primaries )
               fluT_p->setHistory(ncase);
            if (score_spe){
              for (int j = 0; j < nreg; j++){
                  if ( is_sensitive[j] ) 
                     flu[j]->setHistory(ncase);
              }
              if ( score_primaries ){
                 for (int j = 0; j < nreg; j++){
                     if ( is_sensitive[j] ) 
                        flu_p[j]->setHistory(ncase);
                 }
              }
            }
        }
#ifdef DEBUG
        binDist->setHistory(ncase);
        if ( scoring_charge ){
           stepDist->setHistory(ncase);
           relStepDiff->setHistory(ncase);
        }
#endif        

   };
   void resetCounter(){
      current_ncase = 0; 
      fluT->reset();
      if ( score_primaries ) 
         fluT_p->reset();
      if ( score_spe ){
        for (int j = 0; j < nreg; j++){
            if ( is_sensitive[j] ) 
               flu[j]->reset();
        }
        
        if ( score_primaries ){
           for (int j = 0; j < nreg; j++){
               if ( is_sensitive[j] ) 
                  flu_p[j]->reset();
           }
        }
      }
#ifdef DEBUG
      binDist->reset();
      if ( scoring_charge ){
         stepDist->reset();
         relStepDiff->reset();
      }
#endif
   }

   bool storeState(ostream &data) const;
   
   bool setState(istream &data);
   
   bool addState(istream &data);

private:   

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

  EGS_I64 eCases;

  vector<EGS_Float> volume;    // volume of each scoring region
  /* Energy grid inputs */
  //EGS_Float flu_a_i;
  /* Auxiliary input variables*/
  vector <EGS_Float> vol_list;       // Input list of region volumes

#ifdef DEBUG
  /* Debugging information */
  int one_bin, multi_bin;
  EGS_ScoringArray *binDist;  
  EGS_ScoringArray *stepDist;  
  EGS_ScoringArray *relStepDiff; // Relative difference between tvstep and edep-derived step
  EGS_Float max_step;
  EGS_Float step_a, step_b;
  EGS_I32 n_step_bins;
#endif

};

#endif
