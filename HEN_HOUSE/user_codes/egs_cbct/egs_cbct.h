/*
###############################################################################
#
#  EGSnrc egs++ egs_cbct application headers
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
#  Author:          Ernesto Mainegra-Hing, 2007
#
#  Contributors:    Iwan Kawrakow
#
###############################################################################
*/


#ifndef EGS_CBCT_
#define EGS_CBCT_

#include "egs_advanced_application.h"
#include "egs_transformations.h"
#include "egs_utils.h"
#include "egs_mortran.h"
#include "egs_splitter.h"
#include "egs_corrector.h"
#include <string>
#include <vector>

/******************************/
/* Forward class declarations */
/******************************/

/* egspp classes */
class EGS_ScoringArray;
class EGS_Interpolator;
class EGS_RotationMatrix;
class EGS_AffineTransform;

/***********************************/
/* EGS_CBCTSetup class declaration */
/***********************************/
/*! \brief Class to rotate CBCT (source-detector) setup

  Use this class to define a projection in a CBCT scan. Note that
  any transformation defined here will affect both, the source and
  the scoring plane. Users must make sure source and scoring plane
  are positioned and directed properly.

*/
class EGS_CBCTSetup : public EGS_AffineTransform {
public:
      EGS_CBCTSetup();
      EGS_CBCTSetup(EGS_Input *i);
     ~EGS_CBCTSetup(){};
      void describeMe();
      bool isDefined(){return defined;};
      bool swapX2Y(){return swap;};
      EGS_Float getStep(){return step;};
      int       steps(){return nproj>0? nproj:-nproj;};
      EGS_Float atStep(){return iproj > 0? iproj:-iproj;};
      void setup(const EGS_Float & a,
                 const EGS_Float & s,
                 const EGS_Float & o,
                 const EGS_RotationMatrix &m);

private:

EGS_Float angle, // projection angle, user-input or calculated from iproj, defaults to 0 degree
          orbit, // total scan orbit, defaults to 360 degrees
          step;  // angular step
EGS_Float amin;  // minimum angle, defaults to 0 degree
EGS_Float amax;  // maximum angle, defaults to 360 degrees, excluded
int       iproj, // projection index, if not provided, defaults to -1
          nproj, // number of projections, defaults to 360
           irot; // 0 -> x, 1 -> y, 2 -> z axis rotation
bool defined,
     swap;// swap X scan coordinates to Y scan coordinates

};

struct EGS_Hist{
public:
    EGS_Hist();
    EGS_Hist(const EGS_Float &_max);
    EGS_Hist(const EGS_Float &_max, const int &_hs);
    ~EGS_Hist(){};
    EGS_Float get_max(){return max;};
    EGS_Float get_min(){return min;};
    int size(){return hsize;};
    EGS_Float nscore(){return nscores;};
    void score(const EGS_Float &re){
      int index = int((re-min)/bw);
      if (re >= min && re <= max) hist[index]+=1;
      else if(re > max)           hist[hsize-1]+=1;
      else                        hist[0] +=1;
      nscores += 1.0;
    };
    EGS_Float currentHistVariance(EGS_Float &mean){
      EGS_Float sum = 0, sum2 = 0, x, aux;
      for (int ih=0; ih<hist.size(); ih++){
          x = min + bw*(ih + 0.5);aux = hist[ih]*x;
          sum += aux;sum2 += aux*x;
      }
      sum /=nscores; sum2 /=nscores;
      mean = sum;
      return sum2 - sum*sum;
    };
    void outputHist(const string &fname){
      ofstream fout(fname.c_str());
      for (int ih=0; ih<hist.size(); ih++){
         fout << min + bw*(ih + 0.5) <<" "
              << float(hist[ih])/nscores << endl;
      }
      istatus++; //clear();
    };
    void outputHistwithErrors(const string &fname){
      ofstream fout(fname.c_str());
      fout << "@type xydy\n";
      for (int ih=0; ih<hist.size(); ih++){
         fout << min + bw*(ih + 0.5)     << " "
              << float(hist[ih])/nscores << " "
              << sqrt(float(hist[ih]))/nscores << endl;
      }
      fout << "&\n";
      istatus++; //clear();
    };
    void clear(){
      for (int ih=0; ih<hist.size(); ih++) hist[ih]=0.0;
      nscores=0;
    };
    int getStatus(){return istatus;};
private:
    vector<EGS_Float> hist;
    EGS_Float min, max, bw, nscores;
    int hsize, istatus;
};


/******************************/
/* EGS_CBCT class declaration */
/******************************/
class APP_EXPORT EGS_CBCT : public EGS_AdvancedApplication {

public:

    /*! Calculation type */
    enum Type { planar=0, volumetric=1, both=2 };

    /*! Scan type */
    enum ScanType { blank=0, real=1, ideal=2, scatter=3,
                    real_ideal=4, all=5, none=6 };

    /*! Display type */
    enum DisplayType { total=0, attenuated=1, scattered=2 };

    /*! Splitting type */
    enum SplittingType { no_split=0, FS=1, PDIS=2, RDIS=3 };

    /*! Constructor */
    EGS_CBCT(int argc, char **argv);

    /*! Destructor.  */
    ~EGS_CBCT();

    void initOutput();
    void initVRT();

    void startNewParticle();

    /*! Describe the application.  */
    void describeUserCode() const;

    /*! Describe the simulation */
    void describeSimulation();

    /*! Initialize scoring.  */
    int initScoring();

    /*! Accumulate quantities of interest at run time */
    int ausgab(int iarg) ;

    /*! Simulate a single shower.
        We need to do special things and therefore reimplement this method.
     */
    int simulateSingleShower();

    /*! Score Air-Kerma from path length from back of the geometry to scoring plane    */
    inline void scoreKerma( const EGS_Float & gle,
                            const EGS_Float & tstep,
                            const EGS_Float & up, int k);
    /*! Score Air-Kerma for particles missing geometry but hitting detector    */
    inline void scoreDirectKerma( const EGS_Float & gle,
                                  const EGS_Float & weight,
                                  const EGS_Float & tstep,
                                  const EGS_Float & up, int k   );
    /*! Ray-trace primaries and score air-kerma */
    inline void RayTrace( const int & ir0);
    /*! Read results from a .egsdat file. */
    int readData();

    /*! Reset the variables used for accumulating results */
    void resetCounter();

    /*! Output intermediate results to the .egsdat file. */
    int outputData();

    /*! Add simulation results */
    int addState(istream &data);

    /*! Output the results of a simulation. */
    void outputResults();

    /*! Get the current simulation result.  */
    void getCurrentResult(double &sum,  double &sum2,
                          double &norm, double &count);

    /*! simulate a shower */
    int shower();

    /* Select photon mean-free-path */
    void selectPhotonMFP(EGS_Float &dpmfp);

    /* re-implement here to handle parallel jobs */
    int finishSimulation();

protected:

    /*! Start a new shower.  */
    int startNewShower();

    /* get average value in scoring array */
    EGS_Float aveKerma( EGS_ScoringArray &kerma );
    /* get minimum value in scoring array */
    EGS_Float minKerma( EGS_ScoringArray &kerma );
    /* get maximum value in scoring array */
    EGS_Float maxKerma( EGS_ScoringArray &kerma );
    /*! Get maximum value of ratio primary/total signal */
    EGS_Float maxAatt();

    /* get region index of maximum value in scoring array */
    int maxKermaReg(EGS_ScoringArray &kerma );

    /* Gets mean square error for contaminant scatter signal */
    EGS_Float getAveScatErrorSqr();

/* get average uncertainty in regions with values larger than
   a given fraction of the max. value. Given in percentage. */
    EGS_Float aveError(const char * label, EGS_ScoringArray &kerma,
                       const EGS_Float &cut_off );
    EGS_Float aveError(EGS_ScoringArray &kerma );
    void errorDistribution(const char * label, EGS_ScoringArray &kerma );

/* get average uncertainty in regions with values larger than
   a given fraction of the max. value. Given ion percentage. */
    EGS_Float aveErrorAatt(EGS_Float cut_off);

/*  get ratio att/total */
    float* getAttTotalRatio();

/*  get the total kerma  */
    float* getArray(EGS_ScoringArray *array);
/*  prints scans to file */
    void printScans();
/*  prints scan results or a requestd profile to file */
    void printProfiles();

    void doMyRayleigh();
    void doMyCompton();
    void doMyPhoto();
    void fastCompton();

    void transportPhoton(EGS_CBCT_Photon &p);
    void transportSinglePhoton(EGS_CBCT_Photon &p);
    bool computePhotonScore(EGS_CBCT_Photon &p, int nspl,
            double &sc);

    /*! Fast transport using only Klein-Nishina and photo-absorption
        with no characteristic X-rays.
     */
    void fastTransport(EGS_CBCT_Photon &p);

    /*! Sample Klein-Nishina interactions  */
    void doKleinNishina(EGS_CBCT_Photon &p);

    vector<string> getAtt();

    void setupAverageMu();


    /*! Returns scoring voxel index, if the particle hits the screen, -1
        otherwise. If \a dist is not null, it is set to the distance to the
        screen
     */
   //inline int setSplitting(const int &isc,const int &ireg,
   inline int setSplitting(const int &ireg,
                           const EGS_Vector &x,
                           const EGS_Vector &u,
                           EGS_Float &wt, EGS_Float gle);
   inline int setSplitting(const EGS_Float &Katt,
                                 EGS_Float &wt,
                                 EGS_Float gle);
    inline int setSplitting( EGS_Float &att_lambda,
                             const int &ireg,
                             const EGS_Float &_aup,
                             EGS_Float &wt,
                             EGS_Float gle);
    inline int hitsScreen(const EGS_Vector &x,
                          const EGS_Vector &u,
                          EGS_Float *dist = 0);
    inline int setSplitting(const int &ireg,
                            const int& iphat,
                            EGS_Float &wt);
    inline int setSplitting(const EGS_Vector &x,
                            EGS_Float &wt,
                            EGS_Float gle);
    inline void resetSplitting() { patt_using = false; };
    inline int  checkParticle(const EGS_Vector &x,
                              const EGS_Vector &u,
                              EGS_Float E, EGS_Float &wt,
                              EGS_I32 &idetr, EGS_Float *ddet);

/*    inline int computeIntersections(int ireg, const EGS_Vector &x,
                                    const EGS_Vector &u,
                             EGS_GeometryIntersections *isections);*/
    inline int computeIntersections(int ireg, const EGS_Vector &x,const EGS_Vector &u);
    inline EGS_Float getLambda(const int &nsec,
                               EGS_GeometryIntersections *isections,
                               EGS_Float gle);

    inline bool checkVector(const EGS_Vector &v, const string & msg);

    inline void scoreNsplit(const EGS_Float &aux );

    void create_nsplit_hist(const int &_nmax, const int nsize );
    void printNsplit();
    EGS_Hist *nsplit_hist;


private:

    Type             type;      // calculation type:
                                //   = 0 => kerma at a plane
                                //   = 1 => kerma in a volume
                                //   = 2 => both

    ScanType         scan_type; // defaults to no scan
                                // real       => real scan simulation
                                // ideal      => scan without scattering, i.e.,
                                //               attenuated signal
                                // scatter    => scatter corrections given by ratio
                                //               Katt/Ktotal
                                // real_ideal => real and ideal scans
                                // all        => all three scans
                                // none       => no scan

    DisplayType       dtype;    // monitoring display type:
                                //   = 0 => total kerma
                                //   = 1 => primary kerma
                                //   = 2 => secondary (scatter) kerma

    SplittingType   split_type; // Splitting scheme
                                //  none => no splitting
                                //  FS   => fixed splitting
                                //  PDIS => Position Dependent Importance Sampling
                                //  RDIS => Region Dependent Importance Sampling

    bool             verbose;   // much more output files: egshist, egscorr, egsimp
    bool             egsdat;    // switch on/off storing data
                                // after each batch. On by default.
    bool             ray_tracing;// true if user requests ray-tracing mode

    bool             egsmap;    // switch on/off storing signal data
                                // after each batch. Off by default.

    int              forced_detection;// scoring type:
                                  // = 0 => kerma scoring: track length estimation at plane
                                  // = else => kerma scoring: forced detection
    int              ngeom;     // number of geometries to calculate
                                // quantities of interest
    int              ig;        // current geometry index

    int              iair;      // surrounding medium, almost always air

    EGS_ScoringArray *kermaT;  // scoring array for kerma scoring
    EGS_ScoringArray *kermaA;  // scoring array for kerma scoring with
                                // scatter removed.
    EGS_ScoringArray *kermaS;  // scoring array for scattered kerma scoring
    EGS_CorrelatedScoring *cker;// for scoring correlations
    //double            *ckerma;  // sum(kermaT*KermaA): for scatter correction

    EGS_XYProfile         *profile;// 2D profile scanning object

    /*! kerma scoring plane definition*/
    EGS_Vector        a;       // normal a = u x v
    EGS_Vector        ux, uy;  // unit vectors
    EGS_Vector        midpoint;// location
    EGS_Float         distance;// distance from source
    EGS_Float         ax, ay;  // rectangular scoring field
    EGS_Float         vx, vy;  // rectangular scoring voxels
    int               Nx, Ny;  // # of scoring regions in x and y dimension
    EGS_Float         first_rng;

    EGS_Interpolator *muen;
    EGS_Interpolator *muatt;

    int error_estimation; // method to use for average uncertainty calculation:
                          // = 0  -> use uncertainty of Katt/Ktot
                          // = 1  -> use uncertainty of Kscat/<Kscat>
    int ee_xmin, ee_xmax, // for error_estimation=1, only use voxels in the
        ee_ymin, ee_ymax; // area ee_xmin...ee_xmax,ee_ymin...ee_ymax

    EGS_Float cut_off; // average stat. errors for kerma values
                       // larger than cut_off*Kmax, defaults to 0.5
    EGS_Float epsilon; // target statistical uncertainty in percent
                       // defaults to 1%

    /*** cone beam rotation defined as follows:
         - equidistant angles ranging from 0o to orbit (360o or 180o)
         - rotation around x-axis
         - defined in block scoring options
         - projection file will have Nx*Ny*[(orbit/step)+1]*sizeof(float) bytes
           position in projection file will be given by angle/step (0...orbit)
    EGS_Float angle;// rotate source and scoring plane arond x axis, in radians
    EGS_Float orbit;// 360o or 180o, but in radians
    EGS_Float step; // angular step, to determine position in projection file
    EGS_RotationMatrix *cbctR; // to rotate source particles and scoring plane
    */
    EGS_CBCTSetup *cbctS; // to rotate source particles and scoring plane
    string blank_scan;
    string real_scan;

    int               nsplit_p, nsplit_s;
    bool              mfptr_do;
    EGS_Float         mfptr_lamo;

    EGS_Float        f_split;
    EGS_Float        d_split;

    int              max_latch;

    int              isize;
    EGS_GeometryIntersections *gsections;

    int              dt_medium; // medium to use for delta transport

    EGS_CBCT_ParticleContainer pc;

    /*! Phantom bounding geometry.
       Used to compute dnear, the minimum distance to
       boundary along the particle's direction. This
       replaces the use of a fixed plane.
     */
    EGS_BaseGeometry *patt_geom;
    EGS_Vector        patt_point;
    EGS_Vector        patt_a;
    EGS_Float         patt_d;
    EGS_Float         patt_score;
    double            patt_score_sum;
    EGS_I64           patt_score_count;
    EGS_Float         patt_datt;
    EGS_Float         patt_rhor;
    int               patt_med;
    bool              patt_have;
    bool              patt_using;

    EGS_PlanePointSelector *pselector;

    EGS_BaseGeometry *split_geom;
    double           *split_collect;
    EGS_I64          *split_count;
    EGS_Float        *split_current;
    double           *split_e_collect;
    EGS_I64          *split_e_count;
    EGS_Float        *split_e_current;
    double            split_collect_tot;
    EGS_I64           split_count_tot;
    double            split_collect_tot_save;
    EGS_I64           split_count_tot_save;
    EGS_Float         split_e_a;
    EGS_Float         split_e_b;
    int               split_ne_bins;
    int               split_xmin, split_xmax,
                      split_ymin, split_ymax;

    EGS_Float         pnorm;
    float             *m_real, *m_blank;

    /* smoothing parameters */
    int               nmax ,
                      nmax2d;
    double            chi2max,
                      dmin;
    bool              do_smoothing;

    /* corrector object */
    EGS_Corrector* c_att;
    EGS_Hist* hist;
    /* splitter object */
    EGS_Splitter* splitter;
    EGS_Float C_imp,
              C_imp_save;// importance in previous region, reset to 1 in shower()

    EGS_Float         rhormax;

#ifdef DEBUG_WEIGHTS
    EGS_CBCT_Interactions interactions;
#endif

    static string revision;

};

#endif
