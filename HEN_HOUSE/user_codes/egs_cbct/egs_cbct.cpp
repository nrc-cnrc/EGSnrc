/*
###############################################################################
#
#  EGSnrc egs++ egs_cbct application
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
#
#  EGSnrc egs++ application for simulating a CBCT scanner.
#
#  Users can define arbitrary sources and geometries and a 2D ideal detector
#  which scores the total, primary and scatter air-kerma components at each
#  detector pixel. The detector is defined by a scoring plane (position and
#  direction) and the number of pixels Nx (horizontal) and Ny (vertical) on
#  the scoring plane.
#
#  Variance reduction techniques:
#
#  - forced detection
#  - path length biasing
#  - splitting, Russian roulette and delta transport (fictitious interaction).
#  - smoothing
#
#  A study of efficiency is required to tune the parameters to any specific
#  setup.
#
###############################################################################
*/


#include "egs_cbct.h"
// Every C++ EGSnrc application needs this header file
#include "egs_interface2.h"
// To get the maximum source energy
#include "egs_base_source.h"
// Transformations
#include "egs_transformations.h"
// Interpolators
#include "egs_interpolator.h"

#include <egs_run_control.h>

#include "egs_smoothing.h"

#include "egs_timer.h"

const int __debug_case = -1;

class EGS_RectangularBeam;

/*
   Constructs a CBCT setup: No rotation nor transformation done.
*/
EGS_CBCTSetup::EGS_CBCTSetup():
angle(0), step(1), orbit(360), irot(2),
amin(0), amax(0), nproj(360), iproj(-1),
defined(false), swap(false)
{
    defined = true; iproj=0;
    EGS_RotationMatrix m = EGS_RotationMatrix();//unit matrix
    R = m; if( R.isI() ) has_R = false; else has_R = true;
}

/*
   Constructs a CBCT setup: It reads in the rotation orbit, step
   and axis and sets up the proper transformation.
*/
EGS_CBCTSetup::EGS_CBCTSetup(EGS_Input *i):
angle(0), step(1), orbit(360),
amin(0), amax(0), nproj(360), iproj(-1),
defined(false), swap(false)
{

    EGS_RotationMatrix rm = EGS_RotationMatrix();
    EGS_Vector        tra = EGS_Vector(0,0,0);

    EGS_Input *setup = i->takeInputItem("cbct setup");
    if (setup){
        defined = true;
        EGS_Float convert = EGS_Float(Pi)/EGS_Float(180.), ang, orb, stp, ipr;

        int error = 0;error = setup->getInput("minimum angle",ang);
        if (!error){amin=ang;}
            error = 0;error = setup->getInput("maximum angle",ang);
        if (!error){amax=ang;}
            error = 0;error = setup->getInput("orbit",orb);
        if (!error){orbit=orb;}
        error = 0;error = setup->getInput("step",stp);
        if (!error){step=stp;}
        //************************************************
        // Input of amax,amin takes precedence over orbit
        //************************************************
        if (amax > amin) orbit = amax - amin;
        else             amax = amin + orbit;
        nproj = orbit/step;

        //************************************************
        // Determine if CBCT setup translation required
        //************************************************
        vector<EGS_Float> trans;
        error = 0;error = setup->getInput("translation",trans);
        if (error){
           egsWarning("\n*** No translation requested!!!\n");
           tra = EGS_Vector(0,0,0);
        }
        else{
          switch (trans.size()){
          case 1:
            tra = EGS_Vector(trans[0],0,0);
            break;
          case 2:
            tra = EGS_Vector(trans[0],trans[1],0);
            break;
          case 3:
            tra = EGS_Vector(trans[0],trans[1],trans[2]);
            break;
          default:
           egsFatal(
           "\n\n***  Wrong 'translation' input!\n"
           "         Allowed sizes: 1,2, or 3\n"
           "\n       This is a fatal error\n\n");
          }
        }

        //**********************************************************
        // Determine rotation axis and angle. Default is z-rotation.
        // If projection index provided, determine angle. If no
        // projection entry or error in angle, iproj will be negative.
        //**********************************************************
        error = 0;error = setup->getInput("projection",ipr);
        if (!error && ipr >= 0){angle = amin + iproj*step;}
        else{
          error = 0;error = setup->getInput("angle",ang);
          if (!error){angle=ang;iproj=(angle-amin)/step;}
        }
        vector<string> allowed_rotation;
        allowed_rotation.push_back("x");
        allowed_rotation.push_back("y");
        allowed_rotation.push_back("z");
        irot = setup->getInput("rotation axis",allowed_rotation,2);
        if(      irot == 0 ) rm = EGS_RotationMatrix(angle*convert,0,0);
        else if( irot == 1 ) rm = EGS_RotationMatrix(0,angle*convert,0);
        else if( irot == 2 ) rm = EGS_RotationMatrix(0,0,angle*convert);
        else                 rm = EGS_RotationMatrix(0,0, angle*convert);// defaults to z-rotation

        //**********************************************
        // input error above, check whether using old input format
        //**********************************************
        if (iproj < 0){
           error = 0;error = setup->getInput("x-rotation",ang);
           if (!error){angle=ang;rm = EGS_RotationMatrix(angle*convert,0,0);irot=0;}
           else{
              error = 0;error = setup->getInput("y-rotation",ang);
              if (!error){angle=ang;rm = EGS_RotationMatrix(0,angle*convert,0);irot=1;}
              else{
                error = 0;error = setup->getInput("z-rotation",ang);
                if (!error){angle=ang;rm = EGS_RotationMatrix(0,0,angle*convert);irot=2;}
                else{
                  error = 0;error = setup->getInput("any-rotation",ang);
                  if (!error){angle=ang;rm = EGS_RotationMatrix(0,0,angle*convert);irot=2;}
                  else{// unit matrix, no rotation
                    angle=amin;
                    egsWarning("\n*** No rotation requested!\n"
                               "Computing first projection at %g degrees\n",amin/convert);
                  }
                }
              }
           }
           iproj=(angle-amin)/step;
        }

        angle=angle*convert;amin=amin*convert; amax=amax*convert; orbit=orbit*convert; step=step*convert;


        //
        // ***** scan output mode
        //
        vector<string> allowed_types;
        allowed_types.push_back("yes"); allowed_types.push_back("no");
        int scan_swap = setup->getInput("swap",allowed_types,1);
        if( scan_swap == 0 ) swap = true;
    }
    else{
     egsWarning("\n\n*** No CBCT setup input block found!!!!\n");
    }

    //egsInformation("\n------------\n-----> iproj = %d \n------------\n",iproj);

    t = tra; R = rm;
    if( t.length2() > 0  ) has_t = true; else has_t = false;
    if( R.isI() ) has_R = false; else has_R = true;
}

/*
   CBCT setup: Takes in the rotation orbit, step
   and matrix. Left in to be able to use old input files.
*/
void EGS_CBCTSetup::setup(const EGS_Float & a,
                          const EGS_Float & s,
                          const EGS_Float & o,
                          const EGS_RotationMatrix &m)
{
   angle = a; step = s; orbit = o; R = m;
   amin=0; amax = amin + orbit;
   iproj=(angle-amin)/step; nproj=orbit/step;
   has_t = false;
   if( R.isI() ) has_R = false; else has_R = true;
   defined = true; swap=false;
}

void EGS_CBCTSetup::describeMe(){
    if (orbit != 0 && step != 0 ){
     EGS_Float convert = Pi/180.;
     egsInformation(
     " \n\n***\nCone Beam CT simulation: projection # %d out of %d\n",iproj,steps());
     egsInformation("rotation angle = %g degrees\n",angle/convert);
     egsInformation("orbit = %g degrees from %g degrees to %g degrees\n",
                    orbit/convert,amin/convert,amax/convert);
     egsInformation("step = %g degrees\n",step/convert);
     if(hasTranslation()) egsInformation("translation = (%g, %g, %g)\n",t.x,t.y,t.z);
     if (swap) egsInformation("Writting X coordinates as Y coordinates.\n");
     if(hasRotation()){
       if(      irot == 0 ) egsInformation("CBCT rotation around x-axis\n");
       else if( irot == 1 ) egsInformation("CBCT rotation around y-axis\n");
       else if( irot == 2 ) egsInformation("CBCT rotation around z-axis\n");
       egsInformation("***\n\n");
     }
     else                egsInformation("No rotation will be done.\n***\n\n");
    }
}

EGS_Hist::EGS_Hist():
min(0), max(100), hsize(1024), nscores(0), istatus(0)
{
   bw = (max-min)/hsize;
   for (int i=0; i<hsize;i++) hist.push_back(0.0);
}

EGS_Hist::EGS_Hist(const EGS_Float &_max):
min(0), max(100), hsize(1024), nscores(0), istatus(0)
{
   max = _max; bw = (max-min)/hsize;
   for (int i=0; i<hsize;i++) hist.push_back(0.0);
}

EGS_Hist::EGS_Hist(const EGS_Float &_max, const int &_hs):
min(0), max(100), hsize(1024), nscores(0), istatus(0)
{
   max = _max; hsize = _hs; bw = (max-min)/hsize;
   for (int i=0; i<hsize;i++) hist.push_back(0.0);
}

/*! Constructor.  */
EGS_CBCT::EGS_CBCT(int argc, char **argv) :
        EGS_AdvancedApplication(argc,argv), type(planar), ngeom(0),
        kermaT(0),kermaA(0),kermaS(0), cker(0), cut_off(0.5), epsilon(1.0),
        nsplit_p(1), nsplit_s(1), mfptr_do(false), mfptr_lamo(1),
        isize(512), dt_medium(-1), profile(0),
        patt_have(false), patt_med(-1), patt_using(false),
        patt_score_sum(0), patt_score_count(0),
        f_split(1), max_latch(-1), egsdat(true), egsmap(false), verbose(false),
        cbctS(0), scan_type(none), split_type(no_split), rhormax(1),d_split(-1),
        error_estimation(0), split_geom(0), pnorm(1), m_real(0), m_blank(0),
        nmax(10), nmax2d(6), chi2max(2), dmin(0.02), do_smoothing(false),
        splitter(0), c_att(0), C_imp(1), C_imp_save(1), ray_tracing(false)
{
        gsections = new EGS_GeometryIntersections[isize];
        //hist = new EGS_Hist(100.,1024);

//create_nsplit_hist(500., 500 );

}

/*! Destructor.  */
EGS_CBCT::~EGS_CBCT() {
   if( kermaT ) delete kermaT;
   if( kermaA ) delete kermaA;
   if( kermaS ) delete kermaS;
   if( cker ) delete cker;
   if( isize > 0 ) delete [] gsections;
   if( cbctS ) delete cbctS;
   if( splitter ) delete splitter;
   if( c_att ) delete c_att;
   //if(hist) delete hist;
}

string EGS_CBCT::revision = "$Revision: 1.39 $";

inline int EGS_CBCT::computeIntersections(int ireg, const EGS_Vector &x,
        const EGS_Vector &u) {
//        const EGS_Vector &u, EGS_GeometryIntersections *isections) {
    int nsec = 0;
    while(1) {
        nsec = geometry->computeIntersections(ireg,isize,x,u,gsections);
        if( nsec >= 0 ) break;
        int nsize = 2*isize;
        if( nsize > 16384 ) {
            egsInformation("\nThere are more than %d intersections for"
                    " this geometry\n",isize);
            egsInformation("  are you sure this is OK?\n");
            egsInformation("  position: (%g,%g,%g)\n",x.x,x.y,x.z);
            egsInformation("  direction: (%g,%g,%g)\n",u.x,u.y,u.z);
            egsInformation("  region: %d\n",ireg);
            geometry->computeIntersections(ireg,isize,x,u,gsections);
            egsInformation("Here are the intersections:\n");
            for(int j=0; j<isize; j++)
                egsInformation("%12.6f  %6d  %4d  %12.6f\n",
                        gsections[j].t,gsections[j].ireg,
                        gsections[j].imed,gsections[j].rhof);
            egsFatal("Quiting now\n\n");
        }
        EGS_GeometryIntersections *tmp =
            new EGS_GeometryIntersections [nsize];
        for(int j=0; j<isize; j++) tmp[j] = gsections[j];
        delete [] gsections; gsections = tmp; isize = nsize;
    }
    return nsec;
}

/*
Index i assigned to particles at x[i] <= x < x[i+1].
Therefore, if x hits the edge exactly at xmax it can get through
the checks below due to round-off errors, taking index Nx (or Ny)
which is outside the screen. This was generating illegal memory
access.

Solved by using the same arithmetic expressions in the comparison
and the computation of the scoring region index. That way, round-off
errors affect the logic in a consistent manner.

*/
int EGS_CBCT::hitsScreen(const EGS_Vector &x, const EGS_Vector &u,
        EGS_Float *dist) {
    EGS_Float xp = x*a, up = u*a;
    if( (up > 0 && distance > xp) || (up < 0 && distance < xp) ) {
        EGS_Float t = (distance - xp)/up; // distance to plane along u
        EGS_Vector x1(x + u*t - midpoint);// vector on scoring plane
        EGS_Float xcomp = x1*ux;// x-direction component
        EGS_Float ycomp = x1*uy;// y-direction component
                  xcomp = 2*xcomp + ax;
                  ycomp = 2*ycomp + ay;
        if( xcomp > 0 && xcomp < 2*ax &&
            ycomp > 0 && ycomp < 2*ay ){
            int i = int(xcomp/(2*vx)),
                j = int(ycomp/(2*vy)),
                k = i + j*Nx;
            if( error_estimation == 1 ) {
                if( i < ee_xmin || i > ee_xmax ||
                    j < ee_ymin || j > ee_ymax ) k = -1;
            }
            if( dist ) *dist = t;
            return k;
        }
    }
    return -1;
}

/*
  **********************************************
  *** RDIS:                                  ***
  *** Region dependent importance sampling   ***
  **********************************************
    C_imp      => set before interaction through setSplitting
    C_imp_save => set before selecting mfp, at current location

     After an interaction, iphat is set to 1 to avoid splitting
     higher order scattered photons. But here, higher order scattered
     photons are split too!

*/
inline int EGS_CBCT::setSplitting(const int &ireg,
                                  const int& iphat,
                                  EGS_Float &wt)
{
    if (!splitter || splitter->isWarming()) return iphat; // Do Fixed Splitting
    //if (!splitter || iphat==1 || splitter->isWarming() ) return iphat;// Do not split higher order scatter
    //if (iphat==1)  return iphat; // Do not split higher order scatter

    /**********************************************************
       Get importance and splitting number for particles
       aimed at detector
    ***********************************************************/
    C_imp = splitter->getImportance(ireg);//C_imp_save=1./C_imp;
    EGS_Float asplit = wt*C_imp*nsplit_p;
    //EGS_Float asplit = EGS_Float(iphat)*C_imp;
    //EGS_Float asplit = EGS_Float(iphat)*C_imp/C_imp_save;
    int nspl = 1;
    if( asplit >1 ){
        nspl = (int) asplit; wt /= asplit;
        if( rndm->getUniform() < asplit - nspl ) ++nspl;
        wt *= nspl;
    }
    return nspl;
}

/*
  **********************************************
  *** PDIS:                                  ***
  *** Position dependent importance sampling ***
  *** with attenuation plane.                ***
  **********************************************
  Computes spliting number based on the ratio Ka/<Katt>, where
  <Katt> is the average primary signal on the screen and Ka is
  the signal from the current particle position, attenuated
  along the attenuation plane's direction. Path obtained
  by simple substraction.
*/
inline int EGS_CBCT::setSplitting(const EGS_Vector &x, EGS_Float &wt, EGS_Float gle)
{
    patt_using = false; int nsplit = -1;
    if( patt_have ) {
        patt_datt = patt_d - patt_a*x;
        EGS_Float w_att = 1, gmfp = 0;
        if( patt_datt > 0 ) {
            if (patt_med >=0){// attenuation medium
               gmfp = i_gmfp[patt_med].interpolateFast(gle);
               if( the_xoptions->iraylr )
                   gmfp *= i_cohe[patt_med].interpolateFast(gle);
               w_att = exp(-patt_datt/gmfp);
            }
            else // average medium
             w_att = exp(-patt_datt*muatt->interpolate(gle));
        }
        EGS_Float mu_en = muen->interpolateFast(gle);
        EGS_Float asplit = f_split*wt*w_att*mu_en/patt_score;
        nsplit = (int) asplit; wt /= asplit;
        asplit -= nsplit;
        if( rndm->getUniform() < asplit ) ++nsplit;
        wt *= nsplit; patt_using = true;
    }
    return nsplit;
}

/*
  **********************************************
  *** PDIS:                                  ***
  *** Position dependent importance sampling ***
  *** with gsections                         ***
  **********************************************
  Computes spliting number based on the ratio Ka/<Katt>, where
  <Katt> is the average primary signal on the screen and Ka is
  the signal from the current particle position, attenuated
  along the particle's path to the detector. Path obtained
  computing the intersections.
  *******
  Beware:
  *******
  Currently used for particles not aimed at detector. For this
  reason no corrections used here and not included in the estimation
  of the corrections. If one desires to use this method for particles
  aimed at detector using corrections remove commments in pertinent
  lines.
  **********************************************
*/
//int Natt_min = 10000000;
//int Natt_max = 0;
//int Natt_ave = 0;
//int Natt_n = 0;
//int Nmin_a = 1;
inline int EGS_CBCT::setSplitting(const int &ireg,
                                  const EGS_Vector &x,
                                  const EGS_Vector &u,
                                  EGS_Float &wt, EGS_Float gle)
{
    int nsplit = -1;
    //EGS_Float Xi = c_att ? c_att->getCorrection(ireg) : 1;
    int nsec = computeIntersections(ireg,x,u);
    patt_datt = gsections[nsec-1].t;
    EGS_Float w_att = 1, gmfp = 0;
    if( patt_datt > 0 )
        w_att = exp(-patt_datt*muatt->interpolate(gle));
    EGS_Float Katt = wt*w_att*muen->interpolateFast(gle);
    EGS_Float asplit = f_split*Katt/patt_score;
    //EGS_Float asplit = f_split*Katt*Xi/patt_score;
    nsplit = 1;
    if( asplit > 1 ){
      nsplit = (int) asplit; wt /= asplit;
      asplit -= nsplit;
      if( rndm->getUniform() < asplit ) ++nsplit;
      wt *= nsplit;
    }
    the_extra_stack->katt[the_stack->np-1] = -1;
    //the_extra_stack->katt[the_stack->np-1] = Katt/nsplit;
//if (nsplit < Natt_min) Natt_min = nsplit;
//if (nsplit > Natt_max) Natt_max = nsplit;
//if (nsplit>0){Natt_ave += nsplit;Natt_n++;}
//scoreNsplit(nsplit);
    return nsplit;
}

/*
  **********************************************
  *** PDIS:                                  ***
  *** Position dependent importance sampling ***
  *** with pre-calculated att_lambda         ***
  **********************************************
  Computes spliting number based on the ratio Ka/<Katt>, where
  <Katt> is the average primary signal on the screen and Ka is
  the signal from the current particle position, attenuated
  along the particle's path to the detector. Path previously
  obtained in selectPhotonMFP.

  Only works for particles headed to detector, i.e., must
  compute gsections for away headed particles with the above
  method! Might be reason for no significant efficiency increase!!!

*/
inline int EGS_CBCT::setSplitting( EGS_Float &att_lambda,
                                   const int &ireg,
                                   const EGS_Float &_aup,
                                   EGS_Float &wt,
                                   EGS_Float gle)
{
    int nsplit = -1;
    EGS_Float Xi = c_att ? c_att->getCorrection(ireg) : 1;
    patt_datt = att_lambda;
    EGS_Float w_att = 1, gmfp = 0;
    if( patt_datt > 0 )
         //w_att = exp(-patt_datt);
         w_att = exp(-patt_datt*muatt->interpolate(gle));
/*    if( patt_datt > 0 )
         w_att = patt_datt;*/
    EGS_Float aup = _aup > 0 ? _aup:1;
    EGS_Float Katt = wt/aup*w_att*muen->interpolateFast(gle);
    EGS_Float asplit = f_split*Katt*Xi/patt_score;
    nsplit = 1;
/*    if( asplit < Nmin_a ) nsplit = Nmin_a;
    else */
    if( asplit > 1 ){
      nsplit = (int) asplit; wt /= asplit;
      asplit -= nsplit;
      if( rndm->getUniform() < asplit ) ++nsplit;
      wt *= nsplit;
    }
    the_extra_stack->katt[the_stack->np-1] = Katt/nsplit;
//if (nsplit < Natt_min) Natt_min = nsplit;
//if (nsplit > Natt_max) Natt_max = nsplit;
//if (nsplit>0){Natt_ave += nsplit;Natt_n++;}
//scoreNsplit(nsplit);
    return nsplit;
}

void EGS_CBCT::scoreNsplit(const EGS_Float &aux ) {
   nsplit_hist->score(aux);
}

void EGS_CBCT::create_nsplit_hist(const int &_nmax, const int nsize ) {
   nsplit_hist = new EGS_Hist(EGS_Float(_nmax),nsize);
}

void EGS_CBCT::printNsplit() {
/*   if (nsplit_hist->nscore()){
     EGS_Float mean=0.0, var = nsplit_hist->currentHistVariance(mean);
     nsplit_hist->outputHist(constructIOFileName(".egsnsplit",true));
     egsInformation("\n---> mean splitting = %g +/- %g\n", mean, sqrt(var));
     egsInformation("\n     <Nsplit> = %g\n", Natt_ave/Natt_n);
   }
   else{
     egsInformation("\n---> mean splitting = 0 +/- 0\n");
   }*/
}

/*
  **********************************************
  *** Position dependent importance sampling ***
  *** with pre-calculated Ka                 ***
  **********************************************
  Computes spliting number based on the ratio Ka/<Katt>, where
  <Katt> is the average primary signal on the screen and Ka is
  the signal from the current particle position, attenuated
  along the particle's path to the detector previously
  obtained in selectPhotonMFP.

  Only works for particles headed to detector, i.e., must
  compute gsections for away headed particles with the above
  method! Might be reason for no efficiency increase!!!

  This seems to be less efficient than using an average or
  predominant attenuation medium.

*/

// method missing: Should be identical to above method only with
// precomputed Ka!!!
/***************************************************/


/*
  **********************************************
  *** RDIS:                                  ***
  *** Region dependent importance sampling ***
  *** with pre-calculated Ka and attenuation ***
  *** based importances                      ***
  **********************************************/
inline int EGS_CBCT::setSplitting(const EGS_Float &Katt,
                                        EGS_Float &wt,
                                        EGS_Float gle)
{
    int nsplit = -1;
    EGS_Float C = Katt/patt_score;
    if (C < splitter->minImportance()) C = splitter->minImportance();
    if (C > splitter->maxImportance()) C = splitter->maxImportance();
    EGS_Float asplit = wt*C*nsplit_p;
    nsplit = (int) asplit; wt /= asplit;
    asplit -= nsplit;
    if( rndm->getUniform() < asplit ) ++nsplit;
    wt *= nsplit;
    return nsplit;
}

/****************************************************************
   NOT IN USE: not as efficient as the original implementation!!!!

   Calculates number of mfp along the geometry intersections
   accounting for the real media. This is an alternative to the
   approximation of using the most abundant medium in the geometry
   called "attenuation medium".
******************************************************************/
inline EGS_Float EGS_CBCT::getLambda(const int &nsec,
                                     EGS_GeometryIntersections *isections,
                                     EGS_Float gle)
{
   int imed=-1; EGS_Float tlast = 0, Lambda = 0;
   EGS_Float gmfp=1e30, sigma = 0, cohfac = 1;
   for(int j=0; j<nsec; j++) {
       EGS_Float tnew  = isections[j].t,
                 tstep = tnew - tlast;
       if( imed != isections[j].imed ) {
           imed = isections[j].imed;
           if( imed >= 0 ) {
               gmfp = i_gmfp[imed].interpolateFast(gle);
               if( the_xoptions->iraylr ) {
                   cohfac = i_cohe[imed].interpolateFast(gle);
                   gmfp *= cohfac;
               }
           }
           else { gmfp=1e15; cohfac = 1; }
       }
       tlast   = isections[j].t;
       Lambda += tstep*isections[j].rhof/gmfp;
   }
   return Lambda;
}


/**********************************************************************
   Checks whether photon headed to detector after an interaction.
   If it is, no further splitting done (photons headed to detector
   are split only once). Returns a value of 1.
   If particle not headed towards detector, RR game played. If photon
   wins RR game, it returns nsplit_s. If it loses the RR game, it returns
   0 and the particle is killed.
   The return value "result" is used to increase its weight (wt*result)
   and set the splitting number (iphat=result) if > 0 or to kill the
   particle if = 0.
***********************************************************************/
int EGS_CBCT::checkParticle(const EGS_Vector &x, const EGS_Vector &u,
                                  EGS_Float E, EGS_Float &wt, EGS_I32 &idetr,
                                  EGS_Float *ddet) {
    int result = 0;
    idetr = hitsScreen(x,u,ddet);
    if( idetr >= 0 ) {//headed towards detector
        result = 1;
//         if( patt_using ) {// ORIGINAL IMPLEMENTATION, SEEMS LESS EFFICIENT
//             EGS_Float up = patt_a*u;
//             EGS_Float gle = log(E); EGS_Float w_att = 1;
//             if( patt_datt > 0 && up > 0 ) {
//                 //EGS_Float gmfp = i_gmfp[patt_med].interpolateFast(gle);
//                 //if( the_xoptions->iraylr )
//                 //    gmfp *= i_cohe[patt_med].interpolateFast(gle);
//                 //w_att = exp(patt_datt/(gmfp*up));// patt_att is dist. to att plane
//                 w_att = exp(patt_datt*muatt->interpolate(gle));
//                 //w_att = exp(patt_datt/gmfp);
//             }
//             EGS_Float mu_en = muen->interpolateFast(gle);
//             EGS_Float c = patt_score*w_att/(wt*mu_en);
//             if( c > 1 ) {// RR
//                 if( rndm->getUniform()*c < 1 ) wt *= c;
//                 else result = 0;
//             }
//        }
/*        else{
            EGS_Float c = C_imp_save;
            if( c > 1 ) {// RR
                if( rndm->getUniform()*c < 1 ) wt *= c;
                else result = 0;
            }
        }*/
    }
    else {// away from detector, play Russian Roulette
        if( rndm->getUniform()*nsplit_s < 1 ) result = nsplit_s;
        //if( rndm->getUniform()*nsplit_s*C_imp < 1 ) result = nsplit_s*C_imp;
    }
    return result;
}

void EGS_CBCT::doMyRayleigh() {
    int np = the_stack->np-1;
    int nsplit = the_extra_stack->iphat[np];
    if( nsplit < 1 || nsplit_s < 1 )
      egsFatal("nsplit=%d nsplit_s=%d?\n", nsplit, nsplit_s);
    double E = the_stack->E[np];
    EGS_Float ui=the_stack->u[np], vi=the_stack->v[np], wi=the_stack->w[np];
    EGS_Vector x(the_stack->x[np],the_stack->y[np],the_stack->z[np]);
    the_stack->wt[np] /= nsplit;
    EGS_Float wthin = the_stack->wt[np], katt = the_extra_stack->katt[np];
    int latch_o = ++the_stack->latch[np];
    the_extra_stack->iphat[np] = 1;
    for(int j=0; j<nsplit; j++) {
        the_stack->np = np+1;
        doRayleigh();// changes particle direction
        EGS_Vector u(the_stack->u[np],the_stack->v[np],the_stack->w[np]);
        int iwt=checkParticle(x,u,E,the_stack->wt[np],
                the_extra_stack->irdet[np],&the_extra_stack->ddet[np]);
        if( iwt > 0 ) {//accept particle
#ifdef DEBUG_WEIGHTS
            the_extra_stack->inter[np] = interactions.nnow;
#endif
            the_stack->wt[np] *= iwt;
            the_extra_stack->iphat[np++] = iwt;
            if( j<nsplit-1 ) {
                if(np>=MXSTACK){
                    egsFatal("\n\n******************************************\n"
                            "ERROR: In EGS_CBCT::doMyRayleigh() :\n"
                            "max. stack depth MXSTACK=%d < np=%d\n"
                            "Stack overflow due to splitting!\n"
                            "******************************************\n"
                            ,MXSTACK,np);
                }
                the_stack->x[np]=x.x; the_stack->y[np]=x.y; the_stack->z[np]=x.z;
                the_stack->u[np] = ui; the_stack->v[np] = vi; the_stack->w[np] = wi;
                the_stack->iq[np]=0;
                the_stack->dnear[np]=0; the_stack->latch[np]=latch_o;
                the_stack->ir[np]=the_stack->ir[np-1];
                the_extra_stack->iphat[np]=1;the_extra_stack->katt[np]=katt;
                the_stack->E[np] = E; the_stack->wt[np]=wthin;
            }
        }
        else{// particle rejected, reuse it but reset to initial direction
            the_stack->u[np] = ui; the_stack->v[np] = vi; the_stack->w[np] = wi;
        }
    }
    the_stack->np = np;

}

/****************************************************************************
 Compton interaction split nsplit times. If no bound Compton requested,
 a fast method based on the KN cross sections is used. Electrons are discarded.
 Photons not aimed at detector are Russian Rouletted in checkParticle.
********************************************************************************/
void EGS_CBCT::doMyCompton() {
    if( !the_xoptions->ibcmp ) { fastCompton(); return; }
    int np = the_stack->np-1;
    int nsplit = the_extra_stack->iphat[np];
    double E = the_stack->E[np];int nrej = 0, ir = the_stack->ir[np];
    EGS_Float u = the_stack->u[np], v = the_stack->v[np], w = the_stack->w[np];
    EGS_Vector x(the_stack->x[np],the_stack->y[np],the_stack->z[np]);
    int latch_o = ++the_stack->latch[np];
    EGS_Float wthin = the_stack->wt[np]/nsplit;
    EGS_Float ddet = the_extra_stack->ddet[np], katt = the_extra_stack->katt[np];
    int      irdet = the_extra_stack->irdet[np];
    the_extra_stack->iphat[np] = 1;
    for(int i=0; i<nsplit; i++) {
        the_stack->np = np+1;
        if(np+1>MXSTACK){
           egsFatal("\n\n******************************************\n"
                     "ERROR: In EGS_CBCT::doMyCompton() :\n"
                     "max. stack depth MXSTACK=%d < np=%d\n"
                     "Stack overflow due to splitting!\n"
                     "******************************************\n"
                     ,MXSTACK,np);
        }
        the_stack->E[np] = E;
        the_stack->u[np] = u; the_stack->v[np] = v; the_stack->w[np] = w;
        the_stack->x[np] = x.x; the_stack->y[np] = x.y; the_stack->z[np] = x.z;
        the_stack->iq[np] = 0; the_stack->wt[np] = wthin;
        the_stack->ir[np] = ir; the_stack->dnear[np] = 0;
        the_stack->latch[np] = latch_o; the_extra_stack->iphat[np] = 1;
        the_extra_stack->irdet[np] = irdet;
        the_extra_stack->ddet[np]  = ddet;
        the_extra_stack->katt[np]  = katt;

        F77_OBJ(compt,COMPT)();
        if( the_stack->np > the_stack->npold ) {
            int j = the_stack->npold-1; np = the_stack->np-1;
            do {
                the_stack->latch[j] = latch_o;
                int iwt = -1;
                if( !the_stack->iq[j] ) {
                    EGS_Vector uj(the_stack->u[j],the_stack->v[j],
                            the_stack->w[j]);
                    iwt = checkParticle(x,uj,the_stack->E[j],
                            the_stack->wt[j],
                            the_extra_stack->irdet[j],
                           &the_extra_stack->ddet[j]);
                    if( iwt > 0 ) {
                        the_stack->wt[j] *= iwt;
                        the_extra_stack->iphat[j] = iwt;
                    }
                }

                if( iwt > 0 ) ++j;// keep jth particle, get next one on stack
                else {
                    if( j < np ) {// kill jth particle by replacing with np-th particle
                        the_stack->E[j] = the_stack->E[np];
                        the_stack->iq[j] = the_stack->iq[np];
                        the_stack->u[j] = the_stack->u[np];
                        the_stack->v[j] = the_stack->v[np];
                        the_stack->w[j] = the_stack->w[np];
                        the_extra_stack->irdet[j] = the_extra_stack->irdet[np];
                        the_extra_stack->ddet[j]  = the_extra_stack->ddet[np];
                        the_extra_stack->iphat[j] = the_extra_stack->iphat[np];
                    }
                    --np;
                }
            } while ( j <= np );
            ++np;
        }
        else{ ++nrej;--np;
              //if (nrej<100)
              //   egsWarning("Warning: %d interactions rejected!!! iq = %d, E = %g\n",nrej,
             //              the_stack->iq[the_stack->npold-1],the_stack->E[the_stack->npold-1]);
        }
    }
    the_stack->np = np;
}

/* The following assumes no relaxation particles will be created.
   This should apply to all simulations in the patient.
 */
void EGS_CBCT::doMyPhoto() {
    --the_stack->np;
}

void EGS_CBCT::fastCompton() {

    int np = the_stack->np-1, n_o = np;
    int nsplit = the_extra_stack->iphat[np];
    double E = the_stack->E[np];
    EGS_Float wthin = the_stack->wt[np]/nsplit;
    //EGS_Float wphat = wthin*nsplit_s;
    EGS_Float katt = the_extra_stack->katt[np];
    int latch = the_stack->latch[np]+1;
    int ir = the_stack->ir[np];
    EGS_Vector ui(the_stack->u[np],the_stack->v[np],the_stack->w[np]);
    EGS_Vector x(the_stack->x[np],the_stack->y[np],the_stack->z[np]);
    EGS_Float sinpsi = ui.x*ui.x+ui.y*ui.y; EGS_Float sindel,cosdel;
    if( sinpsi > 1e-20 ) {
        sinpsi = sqrt(sinpsi); sindel = ui.y/sinpsi; cosdel = ui.x/sinpsi;
    }
    EGS_Float Ko = E/the_useful->prm;
    EGS_Float broi = 1+2*Ko;
    EGS_Float rejf, Br, Sinthe, Temp, cost, sint, cphi, sphi;
    EGS_Float bro = 1./broi; EGS_Float bro1 = 1 - bro;
    EGS_Float rejmax = broi + bro;
    EGS_Float eta = rndm->getUniform();
    for(int j=0; j<nsplit; j++) {
        /*
        do {
            Br = bro + bro1*rndm->getUniform();
            Temp = (1-Br)/(Ko*Br); Sinthe = Temp*(2-Temp);
            rejf = Br*Br + 1 - Br*Sinthe;
        } while ( rndm->getUniform()*Br*rejmax > rejf );
        */
        EGS_Float r1 = (eta + j)/nsplit;
        while(1) {
            Br = bro + bro1*r1;
            Temp = (1-Br)/(Ko*Br); Sinthe = Temp*(2-Temp);
            rejf = Br*Br + 1 - Br*Sinthe;
            if( rndm->getUniform()*Br*rejmax < rejf ) break;
            r1 = rndm->getUniform();
        }
        EGS_Float Ej = Br*E;
        if( Ej > the_bounds->pcut ) {
            if( Temp < 2 ) { cost = 1 - Temp; sint = sqrt(Sinthe); }
            else { cost = -1; sint = 0; }
            rndm->getAzimuth(cphi,sphi); EGS_Vector u;
            if( sinpsi > 1e-10 ) {
                EGS_Float us = sint*cphi; EGS_Float vs = sint*sphi;
                u.x = ui.z*cosdel*us - sindel*vs + ui.x*cost;
                u.y = ui.z*sindel*us + cosdel*vs + ui.y*cost;
                u.z = ui.z*cost - sinpsi*us;
            }
            else {
                u.x = sint*cphi; u.y = sint*sphi; u.z = cost;
            }
            EGS_Float wt = wthin, ddet; int iphat,irdet;
            int iwt = checkParticle(x,u,Ej,wt,irdet,&ddet);
            if( iwt > 0 ) {
                if(np+1>MXSTACK){
                   egsFatal("\n\n******************************************\n"
                     "ERROR: In EGS_CBCT::fastCompton() :\n"
                     "max. stack depth MXSTACK=%d < np=%d\n"
                     "Stack overflow due to splitting!\n"
                     "******************************************\n"
                     ,MXSTACK,np);
                }
                the_stack->E[np] = Ej; the_stack->wt[np] = wt*iwt;
                the_stack->latch[np] = latch; the_stack->iq[np] = 0;
                the_stack->ir[np] = ir; the_stack->dnear[np] = 0;
                the_stack->x[np]=x.x;the_stack->y[np]=x.y;the_stack->z[np]=x.z;
                the_stack->u[np]=u.x;the_stack->v[np]=u.y;the_stack->w[np]=u.z;
#ifdef DEBUG_WEIGHTS
                the_extra_stack->inter[np] = interactions.nnow;
#endif
                the_extra_stack->irdet[np] = irdet;
                the_extra_stack->ddet[np] = ddet;
                the_extra_stack->katt[np] = katt;
                the_extra_stack->iphat[np++] = iwt;
            }
        }
    }

    the_stack->np = np;
}

int EGS_CBCT::initScoring() {

    /* get angular rotation information in degrees and convert to radians */
    cbctS = new EGS_CBCTSetup(input);

    EGS_Input *options = input->takeInputItem("scoring options");
    if( options ) {

        //
        // *********** calculation type
        //
        vector<string> allowed_types;
        allowed_types.push_back("planar");
        allowed_types.push_back("volumetric");
        allowed_types.push_back("both");
        allowed_types.push_back("ray-tracing");
        int itype = options->getInput("calculation type",allowed_types,0);
        if(      itype == 1 ) type = volumetric;
        else if( itype == 2 ) type = both;
        else if( itype == 3 ){type = planar; ray_tracing=true;}
        else                  type = planar;

        /*get angular rotation information in degrees and convert to radians*/
        EGS_Float convert = Pi/180.;
        int error = 0; EGS_Float ang, orb, stp; bool ang_found = false;
        error = options->getInput("angle",ang);
        if (!error){ang=ang*convert; ang_found = true;}
        error = 0;
        error = options->getInput("orbit",orb);
        if (!error){orb=orb*convert;}
        error = 0;
        error = options->getInput("step",stp);
        if (!error){stp=stp*convert;}
        // rotation around x-axis (angle > 0) or z-axis (angle < 0)
        EGS_RotationMatrix rm = EGS_RotationMatrix();
        if (ang>=0){   rm = EGS_RotationMatrix(0,ang);}// x-rotation
        else {ang*=-1;rm = EGS_RotationMatrix(ang,0);}// z-rotation

        //if (rm.isRotation()&&!rm.isI()){ // asking for rotation here???
        if (ang_found){ // asking for rotation here???
            if (cbctS->isDefined()) // rotation already defined, issue warning
            {
                egsWarning("\n\n*** WARNING: Using deprecated input for rotation,\n"
                "    but rotation already defined in block 'cbct setup'!!!\n"
                "    Ignoring old format input!!!");
            }
            else{// using old input style with fixed rotation around x-axis
                egsWarning("\n\n*** WARNING: Using deprecated input for rotation,\n"
                "    maybe this is an old input file???\n");
                cbctS->setup(ang,stp,orb,rm);
            }
        }
        EGS_Input *aux;
        if( type == planar ) {
            aux = options->takeInputItem("planar scoring");
            if( !aux )
                egsFatal("\n\n*** Calculation type set to planar but no\n"
                             "    scoring plane provided!\n"
                             "    This is a fatal error\n\n");

            /* surrounding medium, defaults to vacuum */
            // needed to have acces to this medium's cross sections
            string env_med; iair = -1;
            int err0 = aux->getInput("surrounding medium",env_med);
            if( !err0 ) {
                EGS_BaseGeometry::setActiveGeometryList(app_index);
                int nmed = geometry->nMedia();
                iair = geometry->addMedium(env_med);
                if( iair >= nmed ) {
                   egsWarning(
                   "\n\n*********** no medium"
                   " with name %s initialized \n=> "
                   "using VACUUM as surrounding medium!\n",
                   env_med.c_str());
                   iair = -1;
                }
            }
            else{
              egsWarning(
              "\n\n*********** no surrounding medium found in input file.\n"
              "            Using first medium %s\n",
              geometry->getMediumName(0));
            }

            /* get target uncertainty and kerma cut_off */
            // defaults set to 1% and 0.5 respectively
            err0 = 0; EGS_Float eps, cut;
            err0 = aux->getInput("target uncertainty",eps);
            if (!err0 && eps > 0.0 && eps <= 100.0){epsilon=eps;}
            err0 = 0;
            err0 = aux->getInput("maximum Aatt fraction",cut);
            if( !err0 ) {
                if( (cut >= 0 && cut <= 1) || cut < 0 ) cut_off=cut;
            }
            else{
              err0 = 0;err0 = aux->getInput("minimum Kscat fraction",cut);
              if( !err0 ) {
                if(cut >= 0 && cut < 1) cut_off=cut;
                else cut_off = 0;
              }
            }

            /* get screen resolution */
            err0 = 0;
            vector<int> screen;
            err0 = aux->getInput("screen resolution",screen);
            if( err0 || screen.size() != 2 ) egsFatal(
              "\n\n***  Wrong/missing 'screen resolution' input "
              "for a planar calculation\n    This is a fatal error\n\n");

            Nx = screen[0];   Ny = screen[1];
            ee_xmin = 0; ee_xmax = Nx-1; ee_ymin = 0; ee_ymax = Ny-1;

            vector<int> eest;
            err0 = aux->getInput("uncertainty estimation",eest);
            if( !err0 && (eest.size() == 1 || eest.size() == 5) ) {
                error_estimation = eest[0];
                if( error_estimation == 1 && eest.size() == 5 ) {
                    ee_xmin = eest[1]; if( ee_xmin < 0 ) ee_xmin = 0;
                    ee_xmax = eest[2]; if( ee_xmax > Nx-1 ) ee_xmax = Nx-1;
                    ee_ymin = eest[3]; if( ee_ymin < 0 ) ee_ymin = 0;
                    ee_ymax = eest[4]; if( ee_ymax > Ny-1 ) ee_ymax = Ny-1;
                }
            }

            /* get voxel size */
            err0 = 0;
            vector<EGS_Float> voxel;
            err0 = aux->getInput("voxel size",voxel);
            if( err0 ) {
             /* get field size instead */
             err0 = 0;
             err0 = aux->getInput("scoring field size",voxel);
             if( err0 ) egsFatal(
              "\n\n***  Missing 'field size' or 'voxel size' input "
              "for a planar calculation\n    This is a fatal error\n\n");
             switch (voxel.size()){
             case 1:
               ax = voxel[0]; ay = voxel[0];
               break;
             case 2:
               ax = voxel[0]; ay = voxel[1];
               break;
             default:
              egsFatal(
              "\n\n***  Wrong 'voxel size' input "
              "for a planar calculation\n    This is a fatal error\n\n");
             }
             vx = ax/Nx; vy = ay/Ny;
            }
            else {// get field size from voxel and resolution
             switch (voxel.size()){
             case 1:
               vx = voxel[0]; vy = voxel[0];
               break;
             case 2:
               vx = voxel[0]; vy = voxel[1];
               break;
             default:
              egsFatal(
              "\n\n***  Wrong 'voxel size' input "
              "for a planar calculation\n    This is a fatal error\n\n");
             }
             ax = Nx*vx; ay = Ny*vy;
            }
            /**********************************************************/
            // Scoring plane definition: Defaults to plane at origin
            // with normal along positive z-axis to mimic default used
            // by sources and shapes. User must define a proper
            // transformation to make sure source particles don't
            // miss the scoring plane.
            //
            // BEWARE: a, ux, and uy set to match coordinate system
            //         in FDK reconstruction algorithm!
            /********************************************************/

            /* scoring plane location in space */
            midpoint = EGS_Vector(0,0,0); // plane at origin by default
            /* scoring plane normal */
            a = EGS_Vector(0,0,1); // default normal along positive z-axis
            //a = EGS_Vector(1,0,0); // default normal along positive x-axis
            /* define unit vectors on scoring plane */
            uy = EGS_Vector(-1,0,0); ux = EGS_Vector(0,1,0); // if positive z-axis as default
            //uy = EGS_Vector(0,0,1); ux = EGS_Vector(0,1,0);// if positive x-axis as default
            /* Request a scoring plane transformation for initial position and orientation */
            EGS_AffineTransform *t = EGS_AffineTransform::getTransformation(aux);
            if (t){
                t->rotate(a) ; t->transform(midpoint);
                t->rotate(ux); t->rotate(uy);
                delete t;
            }

            /* transformation requested: usually CBCT rotation, but also origin translation */
            cbctS->rotate(a); cbctS->transform(midpoint);
            cbctS->rotate(ux);cbctS->rotate(uy);

            /* distance from origin to plane midpoint */
            distance = a*midpoint;

            /* Point selector on the screen */
            pselector = new EGS_PlanePointSelector(midpoint,ux,uy,ax,ay,Nx,Ny);

            /* get E*muen/rho values */
            string muen_file;
            int err3 = aux->getInput("muen file",muen_file);
            if( err3 ) egsFatal(
              "\n\n***  Wrong/missing 'muen file' input for a "
              "kerma calculation\n    This is a fatal error\n\n");
            ifstream muen_data(muen_file.c_str());
            if( !muen_data.is_open() ){
                egsFatal(
              "\n\n***  Failed to open muen file %s\n"
                  "     This is a fatal error\n",muen_file.c_str());
            }
            else{
              egsInformation("\nUsing E*muen file %s for air-kerma calculation\n",
                             muen_file.c_str());
            }
            int ndat; muen_data >> ndat;
            if( ndat < 2 || muen_data.fail() ) egsFatal(
                    "\n\n*** Failed to read muen data file\n");
            EGS_Float *xmuen = new EGS_Float [ndat];
            EGS_Float *fmuen = new EGS_Float [ndat];
            for(int j=0; j<ndat; j++) muen_data >> xmuen[j] >> fmuen[j];
            if( muen_data.fail() ) egsFatal(
                 "\n\n*** Failed to read muen data file\n");
            muen = new EGS_Interpolator(ndat,log(xmuen[0]),
                    log(xmuen[ndat-1]),fmuen);
            delete [] xmuen; delete [] fmuen;
            //vector<string> scatter;
            //scatter.push_back("no"); scatter.push_back("yes");
        }

        kermaT = new EGS_ScoringArray(Nx*Ny);
        kermaA = new EGS_ScoringArray(Nx*Ny);
        kermaS = new EGS_ScoringArray(Nx*Ny);
        cker = new EGS_CorrelatedScoring(kermaT,kermaA);
        //ckerma = new double [Nx*Ny];
        //for(int j=0; j<Nx*Ny-1; j++) ckerma[j] = 0;


        delete options;
    }
    else {
        egsWarning("\n\n*********** no 'scoring options' input *********\n\n");
        return 2;
    }

    //**************************************************
    //            **** smoothing options ****
    //**************************************************
    EGS_Input *smooth_i = input->takeInputItem("smoothing options");
    if( smooth_i ) {
        do_smoothing = true;
        int error = 0; int nmaxi, nmaxi2d; double chi2maxi;
        if (!smooth_i->getInput("nmax",nmaxi)) nmax=nmaxi;
        if (!smooth_i->getInput("nmax2d",nmaxi2d)) nmax2d=nmaxi2d;
        if (!smooth_i->getInput("chi2max",chi2maxi)) chi2max=chi2maxi;

/*      Option for only smoothing distro: not needed really

        int nproji = 1, Nz = 1; bool only_smooth = false;
        vector<string> only_smooth_options;
        only_smooth_options.push_back("yes");
        only_smooth_options.push_back("no");
        int ismooth = smooth_i->getInput("smooth only",only_smooth_options,1);
        if ( !ismooth ){
           if (!smooth_i->getInput("number of projections",nproji)) Nz=nproji;
           EGS_Information("\n=>Smoothing existing scatter distribution only ....\n");
           // read in scan values of scatter Kerma
           string scat_f_str = real_scan; float *scat_i = new float[2*Nx*Ny*Nz];
           scat_f_str.replace(scat_file.rfind(".scan",scat_f_str.length()-1),
                                  5,".scatonly.scan");
           EGS_BinaryFile *scat_f = new EGS_BinaryFile(scat_f_str.c_str());
           if (scat_f->fileSize()==2*Nx*Ny*sizeof(float)){
               scat_i = scat_f->readValues();
           }
           else { // blank scan is unity
            egsWarning("\n\n***** Wrong size or empty scat scan file!!!\n");
            egsWarning("scat file is: %s \n",scat_f->Name());
            egsWarning(" blank file size = %d required size = %d\n\n",
            scat_f->fileSize(), 2*Nx*Ny*sizeof(float)
            );
           }
           float *scat_v = new float[Nx*Ny*Nz]; float *scat_e = new float[Nx*Ny*Nz];
           int l = 0, j_v = 0, j_e = 0;
           for (int k = 0; k < 2*Nz; iscan++){
            for (int i = 0; i < 2*Nx*Ny; i++){
              l = i + k*Nx*Ny;
              if ( k%2==0 ) scat_v[j_v++] = scat_i[l]
              else          scat_e[j_e++] = scat_i[l]
            }
           }
           // delete blank scan file since no longer needed
           delete scat_f; delete [] scat_i;
           exit(1);
        }
*/
    }
    //**************************************************
    //            **** output options ****
    //**************************************************
    initOutput();
    //**************************************************
    //           **** variance reduction ****
    //**************************************************
    forced_detection = 1;
    initVRT();
    the_extra_stack->iphatI = nsplit_p;
    //**************************************************
    //
    // **** ausgab calls only after photon interactions
    //      to mark scattered photons.
    int call;
    for(call=BeforeTransport; call<UnknownCall; ++call)
        setAusgabCall((AusgabCall)call,false);
    for(call=BeforeTransport; call<=ExtraEnergy; ++call)
        setAusgabCall((AusgabCall)call,true);
    setAusgabCall(AfterCompton, true);
    setAusgabCall(AfterPhoto,   true);
    setAusgabCall(AfterRayleigh,true);
    if( nsplit_s > 1 || nsplit_p > 1 ) {
        setAusgabCall(BeforeCompton, true);
        setAusgabCall(BeforePhoto,   true);
        setAusgabCall(BeforeRayleigh,true);
    }

    /***********************************************************************
     *                maximum relative mass density
     **********************************************************************/
    if( geometry->hasRhoScaling() ) {
        EGS_Float rmax = 0;
        for(int j=0; j<geometry->regions(); j++) {
            EGS_Float rhor = geometry->getRelativeRho(j);
            if( rhor > rmax ) rmax = rhor;
        }
        if( rmax > 0 ) rhormax = rmax;
    }
    egsInformation("\n=> Maximum relative mass density in the geometry = "
            "%g g/cm**3\n\n",rhormax);

    EGS_Float E= source->getEmax(), logE = log(E);
    egsInformation("\n===================================================================="
                   "\n Linear mass attenuation coefficients for maximum source Energy = "
                   "%g MeV"
                   "\n====================================================================\n\n",E);
    for (int l =0;l<geometry->nMedia();l++){
      EGS_Float gmfp1 = i_gmfp[l].interpolateFast(logE), gmfp = gmfp1;
      if( the_xoptions->iraylr ) {
        EGS_Float cohfac = i_cohe[l].interpolateFast(logE);
        gmfp *= cohfac;
      }
      egsInformation("medium %-24s mu = %g cm-1 mu(no Rayleigh) = %g cm-1\n",
      geometry->getMediumName(l), 1./gmfp,1./gmfp1);
    }

    return 0;
}

void EGS_CBCT::setupAverageMu(){

    egsInformation("\n===> Setting up average mu interpolation arrays\n");

// get Emax and Emin
    EGS_Float e_max, emax=-1.0e+30, e_min, emin=1.0e+30;
    int nbin = 0, n_bin;
    for (int l =0;l<geometry->nMedia();l++){
      e_max = i_gmfp[l].getXmax();e_min = i_gmfp[l].getXmin();
      emax = e_max > emax ? e_max:emax; emin = e_min < emin ? e_min:emin;
      n_bin = i_gmfp[l].getIndex(emax)+2;
      nbin = n_bin > nbin ? n_bin: nbin;
    }
    egsInformation("     Emax = %g Emin = %g nbin = %d\n",exp(emax),exp(emin),nbin);
    egsInformation("     mu_max = %g mu_min = %g\n",i_gmfp[0].interpolate(emax),
    i_gmfp[0].interpolate(emin));

// get media weights
    egsInformation("     setting up media weights for %d regions\n",geometry->regions());
    EGS_Float *mWeight = new EGS_Float[geometry->nMedia()]; EGS_Float vacWeight = 0;

    for (int l =0;l<geometry->nMedia();l++){mWeight[l] = 0.0;}
    /*************************************************************/
    /* Weigh media based on the number of regions of each medium */
    /*************************************************************/
    for (int ir =0; ir<geometry->regions(); ir++){
        //egsInformation("region %d",ir);
        if (geometry->medium(ir)>=0 &&
            geometry->isRealRegion(ir))//Check if virtual region
            mWeight[geometry->medium(ir)]+=1.0;
        else vacWeight += 1.0;// either vacuum or virtual
    }
    /**********************************************************/
    //egsInformation("# of vacuum/virtual regions = %d\n",vacWeight);
    for (int j =0;j<geometry->nMedia();j++){
        mWeight[j] /= (EGS_Float(geometry->regions()) - vacWeight);
        egsInformation("medium %-24s weight = %g\n",
        geometry->getMediumName(j), mWeight[j]);
    }

    EGS_Float *xmu = new EGS_Float [nbin];EGS_Float *fmu = new EGS_Float [nbin];
    EGS_Float bw = (emax - emin)/EGS_Float(nbin);
    for (int i =0;i<nbin;i++){
        xmu[i] = emin + i*bw; fmu[i]=0;
        for (int j =0;j<geometry->nMedia();j++){
            EGS_Float gmfp = i_gmfp[j].interpolateFast(xmu[i]);
            if( the_xoptions->iraylr ) {
               EGS_Float cohfac = i_cohe[j].interpolateFast(xmu[i]);
               gmfp *= cohfac;
            }
            fmu[i] += mWeight[j]/gmfp;
        }
    }
    muatt = new EGS_Interpolator(nbin,xmu[0],xmu[nbin-1],fmu);

//  // testing interpolator
//     bw = nbin/10.0;
//     for (int i =0;i<10;i++){
//         int j = i*bw;
//         egsInformation(" x = %g mu = %g mu_int = %g\n",exp(xmu[j]),fmu[j], muatt->interpolate(xmu[j]));
//     }

    egsInformation("     done!\n");

    delete [] xmu; delete [] fmu; delete [] mWeight;
}

void EGS_CBCT::initOutput() {
    EGS_Input *out = input->takeInputItem("output options");
    if( out ) {

        vector<string> output_data_options;
        output_data_options.push_back("yes");
        output_data_options.push_back("no");
        int idat = out->getInput("store data arrays",output_data_options,0);
        if (idat!=0) egsdat = false; // true by default
        int imap = out->getInput("store signal map",output_data_options,1);
        if (imap==0) egsmap = true; // false by default
        int iverb = out->getInput("verbose",output_data_options,1);
        if (iverb==0) verbose = true; // false by default

        vector<string> allowed_display_modes;
        allowed_display_modes.push_back("total");
        allowed_display_modes.push_back("attenuated");
        allowed_display_modes.push_back("scattered");
        dtype = total;
        int d_type = out->getInput("display type",allowed_display_modes,0);
        if(      d_type == 0 ) dtype = total;
        else if( d_type == 1 ) dtype = attenuated;
        else                   dtype = scattered;

      /* check whether user requests to output a profile */
      EGS_Input *xy = out->takeInputItem("xy-profile");
      if (xy){ profile = new EGS_XYProfile(xy,ax,ay,Nx,Ny);delete xy;}

      /* check whether user requests scan output */
      EGS_Input *scano = out->takeInputItem("scan output");
      if(scano){
        string b_scan;
        int out_err = scano->getInput("blank scan",b_scan);
        if( out_err ) {
            egsWarning(
              "\n\n***  Missing 'blank scan' file \n"
              "       Producing a blank scan !\n\n");
            scan_type = blank;
        }
        else{
          blank_scan = b_scan;
        }
        /*
          If no scan file name entry, a warning is issued.
          If scan file does not exist, create it and zero it.
        */
        string the_scan;
        out_err = 0; out_err = scano->getInput("scan file",the_scan);
        if( out_err ) {
            egsWarning(
              "\n\n***  Missing 'scan' file name!\n"
              "       This is not a proper CBCT calculation.\n\n");
        }
        else{
          real_scan = the_scan;
        }
        /* check what scans are requested by the user
           real => real scan simulation
           ideal=> scan without scattering, i.e., only attenuated signal
           scat => scatter corrections given by ratio Katt/Ktotal
           both => real and ideal scans
           all  => all three scans
        */
        if (scan_type!=blank){
            scan_type = none;
            vector<string> allowed_scans;
            allowed_scans.push_back("blank");
            allowed_scans.push_back("real");
            allowed_scans.push_back("ideal");
            allowed_scans.push_back("scatter");
            allowed_scans.push_back("real_ideal");
            allowed_scans.push_back("all");
            allowed_scans.push_back("none");
            int stype = scano->getInput("scan type",allowed_scans,6);
            switch(stype){
            case 0:
              scan_type = blank;
              break;
            case 1:
              scan_type = real;
              break;
            case 2:
              scan_type = ideal;
              break;
            case 3:
              scan_type = scatter;
              break;
            case 4:
              scan_type = real_ideal;
              break;
            case 5:
              scan_type = all;
              break;
            default:
              scan_type = none;
            }
            if (ray_tracing && scan_type!=ideal) scan_type = ideal;
        }
        if(blank_scan.length()&&(scan_type!=none && scan_type!=scatter)){
            EGS_BaseFile *bs = new EGS_BaseFile(blank_scan.c_str());
            if (bs->fileExist()){
              if(bs->fileSize()!=Nx*Ny*sizeof(float)){
                int fileSize = bs->fileSize();
                delete bs;
                egsFatal(
                "\n\n***  Wrong blank scan file size = %d bytes\n"
                "     It should be %d bytes"
                "     This is a fatal error.\n\n",
                fileSize,Nx*Ny*sizeof(float));
              }
              else{
                egsInformation(
                "\n\nBlank scan file size = %d bytes\n",bs->fileSize());
              }
            }
            else{
                egsFatal("\n\n*** blank scan %s does not exist ...\n"
                         "    This is a fatal error!\n",
                         blank_scan.c_str());
            }
            delete bs;
        }
        delete scano;
      }
      delete out;
    }
}
void EGS_CBCT::initVRT() {
    EGS_Input *vr = input->takeInputItem("variance reduction");
    if( vr ) {

      egsInformation("\n\n==================================="
                     "\n== Variance Reduction Parameters =="
                     "\n===================================\n\n");

      //************************************************
      //     Air-kerma scoring method
      //************************************************
      vector<string> score;
      score.push_back("track_length");score.push_back("forced_detection");
      forced_detection = vr->getInput("scoring type",score,1);
      //************************************************
      //     Splitting scheme input
      //************************************************
      vector<int> split; vector<EGS_Float> ifsplit;
      if( !vr->getInput("splitting",split) && split.size() == 2 ) {
          nsplit_p = split[0]; nsplit_s = split[1];
          if( nsplit_p < 1 ) nsplit_p = 1;
          if( nsplit_s < 1 ) nsplit_s = 1;
      }
      else if (!vr->getInput("FS splitting",split) && split.size() == 2){
          nsplit_p = split[0]; nsplit_s = split[1];
          if( nsplit_p < 1 ) nsplit_p = 1;
          if( nsplit_s < 1 ) nsplit_s = 1;
          split_type=FS;
      }
      else if (!vr->getInput("RDIS splitting",split) && split.size() == 2){
          nsplit_p = split[0]; nsplit_s = split[1];
          if( nsplit_p < 1 ) nsplit_p = 1;
          if( nsplit_s < 1 ) nsplit_s = 1;
          split_type=RDIS;
      }
      else if (!vr->getInput("PDIS splitting",ifsplit) && ifsplit.size() == 2){
          f_split = ifsplit[0]; nsplit_s = int(ifsplit[1]);
          if( f_split < 0 )  f_split = 1;
          if( nsplit_s < 1 ) nsplit_s = 1;
          split_type=PDIS;
      }
      //************************************************
      // Get RDIS splitter object for computing importances,
      // account for possible OLD input format and
      // set split_type properly
      //************************************************
      EGS_Input *splitter_inp = vr->takeInputItem("RDIS splitter setup");
      if (!splitter_inp) // try OLD input format
       splitter_inp = vr->takeInputItem("splitter setup");
      if( splitter_inp && split_type !=PDIS) {
        splitter = new EGS_Splitter(splitter_inp, nsplit_p, //Nx,
                                    geometry->getNRegDir(0),
                                    geometry->getNRegDir(1),
                                    geometry->getNRegDir(2));
        delete splitter_inp;
        if (split_type==no_split) split_type=RDIS;
      }
      //************************************************
      // If this entry available and no splitting scheme
      // defined in input, probably using OLD input
      // format for PDIS f_split
      //************************************************
      EGS_Float t_fsplit=-1;
      if( !vr->getInput("splitting factor",t_fsplit) &&
          t_fsplit > 0 && split_type==no_split){
          f_split = t_fsplit;
          split_type=PDIS;
      }
      if (split_type==PDIS){
          //************************************************
          // Get PDIS corrector object to account for the fact,
          // that splitting number computed based on potential
          // contribution to scoring plane BEFORE interaction.
          //************************************************
          EGS_Input *corrector_inp = vr->takeInputItem("PDIS corrector setup");
          if( !corrector_inp ) // try OLD input format
            corrector_inp = vr->takeInputItem("corrector setup");
          if( corrector_inp ) {
            c_att = new EGS_Corrector(corrector_inp,
                                      geometry->getNRegDir(0),
                                      geometry->getNRegDir(1),
                                      geometry->getNRegDir(2),
                                      f_split);
            delete corrector_inp;
            if (split_type==no_split) split_type=PDIS;
          }

          //*****************************************************************
          // Define PDIS attenuation plane to compute potential contribution
          // to air-kerma BEFORE interacting based on distance to this plane
          // along the plane's direction. If not defined, estimate made from
          // path along particle's direction determined by ray-tracing.
          //
          // DEFAULT: plane at origin perpendicular to Z-axis
          //*****************************************************************
          EGS_Input *attp = vr->takeInputItem("attenuation plane");
          if( attp ) {
              /* user-requested transformation */
              EGS_AffineTransform *t = EGS_AffineTransform::getTransformation(attp);
              patt_point = EGS_Vector(0,0,0); patt_a = EGS_Vector(0,0,1);
              t->transform(patt_point); t->rotate(patt_a);
              cbctS->transform(patt_point);cbctS->transform(patt_a);
              patt_d = patt_a*patt_point;
              egsInformation("\nUsing attenuation plane at distance d=%g,\n"
                 "    located at P0=(%g,%g,%g)\n\n",patt_d,
                 patt_point.x,patt_point.y,patt_point.z);
              patt_have = true;
              vector<string> att_medium;
              if( !attp->getInput("attenuation medium",att_medium) ) {
                  int nmed = the_media->nmed;
                  patt_med =
                      egsAddMedium(att_medium[0].c_str(),att_medium[0].size())-1;
                  if( patt_med >= nmed ) {
                      patt_med = -1; --the_media->nmed;
                      egsInformation(
                      "  -> Using average medium attenuation\n"
                      "     since requested attenuation medium %d (%s)\n"
                      "     has not been initialized!\n",
                      patt_med, att_medium[0].c_str());
                  }
              }
              delete attp;
          }
          //*****************************************************************
          if (patt_med ==-1){
             setupAverageMu();
          }
      }
      vector<EGS_Float> mfptr;
      /*if( !vr->getInput("mfp transform",mfptr) && mfptr.size() == 2 ) {
          if( mfptr[0] > 0.5 && mfptr[1] > 0 ) {
              mfptr_do = true;
              mfptr_lamo = mfptr[1];
              egsInformation("\n=> Using mfp trafo: %g %g\n\n",mfptr[0],mfptr[1]);
          }
      }*/
      if( !vr->getInput("mfp transform",mfptr)) {
          if( mfptr.size() == 2 && mfptr[1] > 0 ) {
              mfptr_do = true;
              mfptr_lamo = mfptr[1];
              egsInformation(
              "\nPath length biasing 2*eta0**2/(eta + eta0)**3 eta0 = %g\n\n",mfptr[1]);
          }
          if( mfptr.size() == 1 && mfptr[0] > 0 ) {
              mfptr_do = true;
              mfptr_lamo = mfptr[0];
              egsInformation(
              "\nPath length biasing 2*eta0**2/(eta + eta0)**3 eta0 = %g\n\n",mfptr[0]);
          }
      }
      vector<string> dtm;
      if( !vr->getInput("delta transport medium",dtm) && dtm.size() == 1 ) {
          int nmed = the_media->nmed;
          dt_medium = egsAddMedium(dtm[0].c_str(),dtm[0].size())-1;
          if( dt_medium >= nmed ) {
              dt_medium = -1; --the_media->nmed;
              egsFatal("\n*** ERROR: Requested delta transport with"
                " medium %d (%s) "
                "\n    which has not been initialized.\n\n",dt_medium,
                dtm[0].c_str());
          }
          if( dt_medium >= 0 ) egsInformation("\nDelta transport with"
                " medium %d (%s) providing max. cross section\n\n",dt_medium,
                dtm[0].c_str());
      }
      else{
        if ( split_type != no_split ){
           egsFatal("\n\n******Delta transport MUST be used with splitting!!!"
                    " Aborting...\n");
        }
      }

      vr->getInput("splitting distance",d_split);
      int t_max_latch;
      if( !vr->getInput("maximum latch",t_max_latch) && t_max_latch >= 0 )
          max_latch = t_max_latch;
      string mscan, mblank;
      if( !vr->getInput("measured scan",mscan) &&
          !vr->getInput("blank scan",mblank) ) {
          ifstream msc(mscan.c_str(),ios::binary);
          ifstream mbl(mblank.c_str(),ios::binary);
          if( msc && mbl ) {
              m_real = new float [Nx*Ny]; m_blank = new float [Nx*Ny];
              mbl.read((char *)m_blank,Nx*Ny*sizeof(float));
              EGS_I64 pos = ((EGS_I64)(cbctS->atStep()+0.5))*Nx*Ny*sizeof(float);
              msc.seekg(pos);
              msc.read((char *)m_real,Nx*Ny*sizeof(float));
              bool ok = true;
              if( msc.fail() || !msc.good() ) {
                  ok = false;
                  egsWarning("Failed reading measured scan %s "
                    "at position %lld\n",mscan.c_str(),pos);
              }
              if( mbl.fail() || !mbl.good() ) {
                  ok = false;
                  egsWarning("Failed reading blank scan %s\n",mblank.c_str());
              }
              if( ok ) {
                  int j,n=0; double sumb=0;
                  for(j=0; j<Nx*Ny; j++) {
                      if( m_blank[j] > 0 ) { ++n; sumb += m_blank[j]; }
                  }
                  sumb /= n; double sump = 0;
                  EGS_Float *aux = new EGS_Float [Nx*Ny];
                  EGS_Float maxp = 0;
                  for(int j=0; j<Nx*Ny; j++) {
                      if( m_blank[j] <= 0 ) m_blank[j] = sumb;
                      aux[j] = exp(m_real[j])/m_blank[j];
                      sump += aux[j];
                      if( aux[j] > maxp ) maxp = aux[j];
                  }
                  pnorm = sump/(Nx*Ny);
                  egsInformation(" *** sump = %g max = %g\n",pnorm,maxp/pnorm);
                  pselector->setProbabilities(aux);
                  delete [] aux;
              }
              }
              else {
              if( !msc ) egsWarning("Failed to open measured scan file %s\n",
                      mscan.c_str());
              if( !mbl ) egsWarning("Failed to open blank scan file %s\n",
                      mblank.c_str());
              }
          }

      string sgeom;
      if( !vr->getInput("splitting geometry",sgeom) ) {
          EGS_BaseGeometry::setActiveGeometryList(app_index);
          split_geom = EGS_BaseGeometry::getGeometry(sgeom);
          if( !split_geom ) egsWarning("\n\n********** no geometry named"
            "%s exists, will not collect/use splitting info\n",sgeom.c_str());
          else {
              split_ne_bins = 1;
              vr->getInput("splitting energy bins",split_ne_bins);
              if( split_ne_bins < 1 ) split_ne_bins = 1;
              vector<int> sarea;
              if( !vr->getInput("splitting area",sarea) && sarea.size() == 4 ) {
                  split_xmin = sarea[0]; if( split_xmin < 0 ) split_xmin = 0;
                  split_xmax = sarea[1]; if( split_xmax <= split_xmin )
                  split_xmax = Nx-1;
                  split_ymin = sarea[2]; if( split_ymin < 0 ) split_ymin = 0;
                  split_ymax = sarea[3]; if( split_ymax <= split_ymin )
                  split_ymax = Ny-1;
              }
              else {
                  split_xmin = 0; split_xmax = Nx-1;
                  split_ymin = 0; split_ymax = Ny-1;
              }
              egsInformation(
              "\n\n Splitting geometry: %s with %d regions.\n"
                   " Using %d energy bins.\n"
                   " Collecting info in %d...%d,%d...%d.\n",
                   split_geom->getName().c_str(),
                   split_geom->regions(),split_ne_bins,
                   split_xmin,split_xmax,split_ymin,split_ymax);
              int nr = split_geom->regions();
              split_collect = new double [nr];
              split_current = new EGS_Float [nr];
              split_count = new EGS_I64 [nr];
              for(int j=0; j<nr; j++) {
                  split_collect[j] = 0; split_current[j] = 0;
                  split_count[j] = 0;
              }
              if( split_ne_bins > 1 ) {
                  split_e_collect = new double [split_ne_bins];
                  split_e_current = new EGS_Float [split_ne_bins];
                  split_e_count = new EGS_I64 [split_ne_bins];
                  for(int j=0; j<split_ne_bins; j++) {
                      split_e_collect[j] = 0; split_e_current[j] = 0;
                      split_e_count[j] = 0;
                  }
              }
              split_collect_tot = 0;
              split_count_tot = 0;
              split_collect_tot_save = 0;
              split_count_tot_save = 0;
              EGS_Float Emax = 1.05*source->getEmax();
              EGS_Float Emin = the_bounds->pcut;
              if( Emin < 1e-3 ) Emin = 1e-3;
              split_e_a = ((EGS_Float) split_ne_bins-1)/Emax;
              split_e_b = 0;
          }
      }

      delete vr;
    }
}


void EGS_CBCT::describeSimulation() {
    EGS_AdvancedApplication::describeSimulation();
    egsInformation("\n=============================================\n");
    egsInformation("\n===========    user inputs summary    =======\n");
    egsInformation("\n=============================================\n");
    egsInformation("\n\nxsec_out = %d\n\n",the_egsio->xsec_out);
    egsInformation("\n\nCalculation type = ");
    if( type == planar && !ray_tracing)     egsInformation("planar");
    else if( type == planar && ray_tracing) egsInformation("planar (ray-tracing)");
    else if( type == volumetric )           egsInformation("volumetric");
    else                                    egsInformation("planar/volumetric");
    egsInformation("\n");

    switch(scan_type){
        case blank:
          egsInformation("\n ... Blank scan output requested\n");
          break;
        case real:
          egsInformation("\n ... Real scan output requested\n");
          break;
        case ideal:
          if (ray_tracing)
             egsInformation("\n ... Only ideal scan output when ray-tracing\n");
          else
             egsInformation("\n ... Attenuated scan output requested\n");
          break;
        case scatter:
          egsInformation("\n ... Scatter correction scan output requested\n");
          break;
        case real_ideal:
          egsInformation("\n ... Real and attenuated scan output requested\n");
          break;
        case all:
          egsInformation("\n ... All type of scans requested\n");
          break;
        default:
          egsInformation("\n ... No scans requested\n");
   }

   cbctS->describeMe();

   egsInformation("scoring screen resolution: %d X %d \n",Nx,Ny);
   egsInformation("voxel size: %g X %g cm^2 \n",vx,vy);
   egsInformation("scoring screen size:  %g X %g cm^2 \n",ax,ay);
   egsInformation("\nscoring plane at: (%g,%g,%g)\n",
   midpoint.x,midpoint.y,midpoint.z);
   egsInformation("distance origin to detector along plane normal d = %g \n",distance);
   egsInformation("scoring plane normal: (%g,%g,%g)\n",a.x,a.y,a.z);
   // Compute source-detector distance
   //EGS_Vector x, u;
   //current_case = source->getNextParticle(rndm,p.q,p.latch,p.E,p.wt,x,u);
   //cbctS->rotate(u);cbctS->transform(x);
   //float dist_p = x*a,
   //      dist_s = midpoint*a, dist_tot = -dist_p+dist_s;
   //egsInformation("source-detector distance = %g\n",dist_tot);
   //source->resetCounter();

   egsInformation("\nscoring plane unit vectors \n"
                   " ux(%g,%g,%g) \n"
                   " uy(%g,%g,%g) \n",ux.x,ux.y,ux.z,uy.x,uy.y,uy.z);
   if( iair >= 0 )
     egsInformation("\n Surrounding medium name is : %s\n",
                  geometry->getMediumName(iair));
   else
     egsInformation("\n Surrounding medium name is : VACUUM (iair=%d)\n",
                 iair);

   if (!forced_detection)
    egsInformation("\n Scoring kerma when CROSSING plane [%d]",
                  forced_detection);
   else
    egsInformation("\n Scoring kerma when DIRECTED towards plane [%d]",
                  forced_detection);
    char c = '%';
//     egsInformation("\n\n Target uncertainty for Aatt values <");
//     if( cut_off > 0 )
//         egsInformation(" %g*Amax : %g%c\n",cut_off,epsilon,c);
//     else
//         egsInformation(" %g : %g%c\n",-cut_off,epsilon,c);
    egsInformation("\n\n Calculate efficiency based on uncertainty in regions\n"
                   " with values larger than %g%c of the max. scatter.\n",
                   cut_off*100.,c);

    //*******************
    // VRT's description
    //*******************
    if (split_type==FS){
      egsInformation("\n=> Fixed splitting scheme: Np=%d Ns=%d\n", nsplit_p, nsplit_s);
    }
    else if (split_type==PDIS){
      egsInformation("\n=> Position Dependent splitting scheme\n");
      egsInformation("  -> splitting factor fsplit=%g Ns=%d\n", f_split, nsplit_s);
      if (patt_have)
         egsInformation("  -> attenuation plane normal=(%g,%g,%g)\n",
                         patt_a.x,patt_a.y,patt_a.z);
      else
         egsInformation("  -> attenuation along particle's direction in the geometry\n");
      if (patt_med ==-1){
        egsInformation("  -> Using average medium attenuation\n");
      }
      else egsInformation("  -> attenuation medium: %d (%s)\n",
           patt_med, geometry->getMediumName(patt_med));
      if (c_att) c_att->describeIt();
    }
    else if (split_type==RDIS){
      egsInformation("\n=> Region Dependent splitting scheme: Np=%d Ns=%d\n", nsplit_p, nsplit_s);
      if (splitter) splitter->describeIt();
      else{
         egsInformation("\n *** WARNING: No splitter object defined. Switching to FS!\n");
         split_type=FS;
      }
    }
    else egsInformation("\n No splitting scheme defined: Np=%d Ns=%d\n", nsplit_p, nsplit_s);

    switch(dtype){
    case total:
       egsInformation("\n batch monitoring total Kerma \n");
       break;
    case attenuated:
       egsInformation("\n batch monitoring primary Kerma \n");
       break;
    case scattered:
       egsInformation("\n batch monitoring scatter Kerma \n");
       break;
    }

    if( profile ) profile->describeIt();

    if (egsmap) egsInformation("\n Creating an egsmap file\n");

    if (do_smoothing){
       egsInformation("\n Smoothing scatter distribution:\n");
       egsInformation("maximum search window nmax   = %d\n",nmax);
       egsInformation("maximum smooth window nmax2d = %d\n",nmax2d);
       egsInformation("maximum chi2         chi2max = %g\n",chi2max);
    }

   egsInformation("\n=============================================\n");
}

void EGS_CBCT::startNewParticle() {
     EGS_AdvancedApplication::startNewParticle();
}

/*! Describe the application.  */
void EGS_CBCT::describeUserCode() const {
        egsInformation(
          "\n               *************************************************"
          "\n               *                                               *"
          "\n               *                  egs_cbct                     *"
          "\n               *                                               *"
          "\n               *************************************************"
          "\n\n");
        egsInformation("This is EGS_CBCT %s based on\n"
          "      EGS_AdvancedApplication %s\n\n",
          egsSimplifyCVSKey(revision).c_str(),
          egsSimplifyCVSKey(base_revision).c_str());

}

/*! Score Air-Kerma from path length to scoring plane    */
inline void EGS_CBCT::scoreKerma( const EGS_Float & gle,
                 const EGS_Float & tstep,
                 const EGS_Float & up, int k   ){

      int np = the_stack->np-1;
      double    Lambda;
      EGS_Float cohfac, sigma, gmfp;
      if( iair >= 0 ) {
             gmfp = i_gmfp[iair].interpolateFast(gle);
             if( the_xoptions->iraylr ) {
                 cohfac = i_cohe[iair].interpolateFast(gle);
                 gmfp *= cohfac;
             }
             sigma = 1/gmfp;
      } else { sigma = 0; cohfac = 1; }
      Lambda = tstep*sigma;// mfps to plane in surrounding medium

      EGS_Float exp_Lambda = Lambda < 80 ? exp(-Lambda) : 0;
      EGS_Float _muen = muen->interpolateFast(gle);
      EGS_Float aup = fabs(up); if( aup < 0.08 ) aup = 0.08;
      //EGS_Float aup = 1;
      double sc = the_stack->wt[np]/aup*exp_Lambda*_muen;
      /*--------------------------------------------------------*/
      /*                 score total kerma                      */
      /*--------------------------------------------------------*/
      cker->score(k,0,sc);
      //kermaT->score(k,the_stack->wt[np]/aup*exp_Lambda*_muen);
      /*--------------------------------------------------------*/
      /*           score primary kerma, only attenuation        */
      /*--------------------------------------------------------*/
      if(the_stack->latch[np]== 0) kermaA->score(k,sc);
      else                         kermaS->score(k,sc);
      /*--------------------------------------------------------*/
}

/*! -------------------------------------------------------------
    Ray-trace primaries and score air-kerma.
-----------------------------------------------------------------*/
inline void EGS_CBCT::RayTrace( const int & ir0 )
{

       // check if particle headed towards plane
       EGS_Float t;
       int k = hitsScreen(p.x, p.u, &t);

       if ( k < 0 ) return; // Does not hit detector

       //
       // *** Particle headed for the detection screen
       // *** Get particle's trajectory intersections through geometry
       //
       int ireg = ir0,
           nsec = computeIntersections(ireg,p.x,p.u);
       //
       // *** Ray trace path along particle's direction to the screen
       //
       EGS_Float gle = log(p.E), tlast = 0, gmfp=1e30,
                 sigma = 0, cohfac = 1, rhor = 1;
       int imed = -1; bool reached_screen = false; double Lambda = 0;
// egsInformation("ireg = %d nsec = %d pixel=%d\n",ir0,nsec,k);
       if (!nsec){ scoreDirectKerma( gle, p.wt, t, a*p.u, k ); return;}
/*if (nsec)
egsInformation("-> Initial: ireg = %d x=%g y=%g z=%g u=%g v=%g w=%g nsec = %d pixel=%d\n",
                  ireg,p.x.x,p.x.y,p.x.z,p.u.x,p.u.y,p.u.z,nsec,k);*/
       for(int j=0; j<nsec; j++) {
           EGS_Float tnew = gsections[j].t, // distance from particle's position to jth intersection
                     tstep = tnew - tlast;  // step through jth region
           /**************************************************/
           /* Get mu value for current region and MFP numbers*/
           /**************************************************/
           if( imed != gsections[j].imed ) {
               imed = gsections[j].imed;
               if( imed >= 0 ) {
                   gmfp = i_gmfp[imed].interpolateFast(gle);
                   if( the_xoptions->iraylr ) {
                       cohfac = i_cohe[imed].interpolateFast(gle);
                       gmfp *= cohfac;
                   }
               }
               else { continue;}//gmfp=1e15; cohfac = 1; }
           }
           sigma = gsections[j].rhof/gmfp;
           EGS_Float this_lambda = tstep*sigma;// MFP numbers tstep*mu = tstep/lambda
           /**************************************************/

           if( !reached_screen && tnew >= t ) {
               reached_screen = true;
               Lambda += (t - tlast)*sigma;// t - tlast is distance from
                                           // last intersection to detector
           }

           if( reached_screen ) break;

           tlast = gsections[j].t; Lambda += this_lambda;

/*  egsInformation("med[%d] = %d t[%d] = %g cm, step= %g cm, lamda=%g \n",
                  j,gsections[j].imed,j,gsections[j].t,tstep,this_lambda);*/
       }

       if( !reached_screen ) {
           //
           // *** Particle has not yet intersected the scoring plane
           //     => compute MFPs in medium surrounding the geometry
           //
           if( iair >= 0 ) {
               gmfp = i_gmfp[iair].interpolateFast(gle);
               if( the_xoptions->iraylr ) {
                   cohfac = i_cohe[iair].interpolateFast(gle);
                   gmfp *= cohfac;
               }
               Lambda += (t - gsections[nsec-1].t)/gmfp;
           }
       }

       //
       // *** Score particle contribution to detected signal
       //
       if( Lambda < 80 ) {
           EGS_Float _muen = muen->interpolateFast(gle);
           EGS_Float exp_Lambda = exp(-Lambda);
           EGS_Float up = a*p.u;
           EGS_Float aup = fabs(up); if( aup < 0.08 ) aup = 0.08;
           double sc = p.wt/aup*exp_Lambda*_muen;
           /*--------------------------------------------------------*/
           /*                 score total kerma                      */
           /*--------------------------------------------------------*/
           if (scan_type == blank) cker->score(k,0,sc);
           /*---------------------------------------*/
           /* score primary kerma, only attenuation */
           /*---------------------------------------*/
           if (scan_type == ideal) kermaA->score(k,sc);
           /*---------------------------------------*/
       }
//        else
//        {
//          egsInformation("-> Not scoring through geometry Lambda = %g\n",Lambda);
//        }

}
/*! -------------------------------------------------------------
    Score primary air-kerma from path length to scoring plane
    when particles are not directed towards the geometry but
    are hitting the scoring plane. IN this case all particles are
    primary particles.
-----------------------------------------------------------------*/
inline void EGS_CBCT::scoreDirectKerma( const EGS_Float & gle,
                                  const EGS_Float & weight,
                                  const EGS_Float & tstep,
                                  const EGS_Float & up, int k   )
{
      double    Lambda = 0.0;
      EGS_Float cohfac, sigma, gmfp;
      if( iair >= 0 ) {
             gmfp = i_gmfp[iair].interpolateFast(gle);
             if( the_xoptions->iraylr ) {
                 cohfac = i_cohe[iair].interpolateFast(gle);
                 gmfp *= cohfac;
             }
             sigma = 1/gmfp;
      } else { sigma = 0; cohfac = 1; }
      Lambda = tstep*sigma;// mfps to plane in surrounding medium
      EGS_Float exp_Lambda = Lambda < 80 ? exp(-Lambda) : 0;
      EGS_Float _muen = muen->interpolateFast(gle);
      EGS_Float aup = fabs(up); if( aup < 0.08 ) aup = 0.08;
      //EGS_Float aup = 1;
      double sc = weight/aup*exp_Lambda*_muen;
      if (ray_tracing){
         if (scan_type == blank) cker->score(k,0,sc);
         if (scan_type == ideal) kermaA->score(k,sc);
      }
      else{// Default scoring mode
        /*--------------------------------------------------------*/
        /*                 score total kerma                      */
        /*--------------------------------------------------------*/
         cker->score(k,0,sc);
        /*--------------------------------------------------------*/
        /*           score primary kerma, only attenuation        */
        /*--------------------------------------------------------*/
         kermaA->score(k,sc);
        /*--------------------------------------------------------*/
      }
}


/*! Accumulate quantities of interest at run time */
int EGS_CBCT::ausgab(int iarg) {

    int np = the_stack->np-1;

    /* only photons since E>0.511 for electrons */
    if( the_stack->E[np] < the_bounds->pcut) {
       --the_stack->np; return 0;
    }

    //=======================================================
    // leaving geometry: score if photons crossing plane
    //=======================================================
    if(iarg==UserDiscard && !forced_detection && !the_stack->iq[np] ){
    //if(iarg==UserDiscard && !forced_detection){

        EGS_Vector x(the_stack->x[np],
                     the_stack->y[np],
                     the_stack->z[np]);
        EGS_Vector u(the_stack->u[np],
                     the_stack->v[np],
                     the_stack->w[np]);
        EGS_Float xp = x*a, up = u*a;
        /* check if particle headed towards plane */
        int k = hitsScreen(x, u, 0);
        if ( k >= 0 ) {
           EGS_Float tstep = (distance - xp)/up,
                     gle   = the_epcont->gle;
           scoreKerma(gle, tstep, up, k);
        }
        return 0;
    }

    if(iarg==BeforeRayleigh||iarg==BeforeCompton||iarg==BeforePhoto){
        EGS_Vector x(the_stack->x[np],the_stack->y[np],the_stack->z[np]);
        EGS_Vector u(the_stack->u[np],the_stack->v[np],the_stack->w[np]);
        int k = hitsScreen(x, u, 0); EGS_Vector u_s = k>=0? u:patt_a;
        //int nspl = setSplitting(k,the_stack->ir[np]-2,x,u,
        int nspl = setSplitting(the_stack->ir[np]-2,x,u_s,
                the_stack->wt[np],the_epcont->gle);
        if( !nspl ) { --the_stack->np; return -1; }
        if( nspl > 0 ) the_extra_stack->iphat[np] = nspl;
        else nspl = the_extra_stack->iphat[np];
#ifdef DEBUG_WEIGHTS
        Interaction inter(x,EGS_Vector(the_stack->u[np],
             the_stack->v[np],the_stack->w[np]),the_stack->E[np],
             the_stack->wt[np],the_stack->ir[np]-2,the_stack->latch[np],
             iarg,nspl,2);
#endif
        if( iarg==BeforeRayleigh ) doMyRayleigh();
        else if( iarg==BeforeCompton ) doMyCompton();
        else doMyPhoto();
        resetSplitting();
#ifdef DEBUG_WEIGHTS
        interactions.addInteraction(inter);
#endif
        return -1;
    }

    //=======================================================
    //  **** mark scattered photons. Rejected Compton photons
    //       are not marked. Rayleigh scattered photons are.
    //=======================================================
    /*
        Forced-detection (forced_detection == true):
        Interactions of photons not aimed at scoring plane
        are handled outside selectPhotonMFP if no delta
        transport in effect. But the scoring is still done
        inside it. One must update relevant quantities
        such as latch, ddet and irdet.
    */
    if(iarg==AfterRayleigh||iarg==AfterCompton||iarg==AfterPhoto){
        int npold = the_stack->npold-1;
        //if( iarg==AfterCompton && np == npold ) return 0;
        for(int ip=npold; ip<=np; ip++) {
            if( !the_stack->iq[ip] ) {// photon
               if( iarg!=AfterCompton || np > npold )
                   ++the_stack->latch[ip];
               if (forced_detection){// update ddet and irdet
                   EGS_Vector x(the_stack->x[ip],
                                the_stack->y[ip],
                                the_stack->z[ip]);
                   EGS_Vector u(the_stack->u[ip],
                                the_stack->v[ip],
                                the_stack->w[ip]);
                   the_extra_stack->irdet[ip] = hitsScreen(x,u,
                  &the_extra_stack->ddet[ip]);
                   the_extra_stack->iphat[ip]=1;
               }
            }
        }
    }
    return 0;
}

/*! Simulate a single shower.
        We need to do special things and therefore reimplement this method.
*/
int EGS_CBCT::simulateSingleShower() {
        last_case = current_case;
        EGS_Vector x,u;
        current_case = source->getNextParticle(rndm,p.q,p.latch,p.E,p.wt,x,u);

        /* if desired, rotate particles around x-axis (CBCT calculations)*/
/*        if (cbctR){
            u = (*cbctR)*u;
            x = (*cbctR)*x;
        }*/
        cbctS->rotate(u);
        cbctS->transform(x);

        int err = startNewShower(); if( err ) return err;
        p.x = x; p.u = u;

        int ireg = geometry->isWhere(p.x);
        /* Ray-tracing mode to produce ideal or blank scan */
        if (ray_tracing){
           RayTrace(ireg);
           err = finishShower();return err;
        }

        if( ireg < 0 ) {
            EGS_Float t = 1e30; ireg = geometry->howfar(ireg,p.x,p.u,t);
            if( ireg >= 0 ) p.x += p.u*t;
        }
        if( ireg >= 0 ) {//**** shower call *****
            p.ir = ireg;
            err = shower(); if( err ) return err;
        }
        else{//particle won't enter geometry
           EGS_Float up = u*a;
          // check if particle headed towards plane
          EGS_Float tstep;
          int k = hitsScreen(x, u, &tstep);
          if ( k >= 0 ) {
             EGS_Float gle   = log(p.E);
             scoreDirectKerma(gle, p.wt, tstep, up, k);
          }
        }
        err = finishShower();
        return err;
}

/*! Output intermediate results to the .egsdat file. */
int EGS_CBCT::outputData() {
        if (!egsdat) return 0;// don't store data after each batch
        int err = EGS_AdvancedApplication::outputData();
        if( err ) return err;
        if( !kermaT->storeState(*data_out) ) return 101;
        if( !kermaA->storeState(*data_out) ) return 102;
        if( !kermaS->storeState(*data_out) ) return 103;
        if( !cker->storeState(*data_out) ) return 104;
        if( !data_out->good() ) return 105;
        data_out->flush();
        delete data_out; data_out = 0;
        return 0;
}

/*! Read results from a .egsdat file. */
int EGS_CBCT::readData() {
        int err = EGS_AdvancedApplication::readData();
        if( err ) return err;
        if( !kermaT->setState(*data_in) ) return 101;
        if( !kermaA->setState(*data_in) ) return 102;
        if( !kermaS->setState(*data_in) ) return 103;
        if( !cker->setState(*data_in) ) return 104;
        if( !data_in->good() ) return 105;
        return 0;
}

/*! Reset the variables used for accumulating results */
void EGS_CBCT::resetCounter() {
        EGS_AdvancedApplication::resetCounter();
        kermaT->reset();
        kermaA->reset();
        kermaS->reset();
        cker->reset();
}

/*! Add simulation results */
int EGS_CBCT::addState(istream &data) {
        int err = EGS_AdvancedApplication::addState(data);
        if( err ) return err;
        EGS_ScoringArray tmp(Nx*Ny);
        if( !tmp.setState(data) ) return 101;
        (*kermaT) += tmp;
        if( !tmp.setState(data) ) return 102;
        (*kermaA) += tmp;
        if( !tmp.setState(data) ) return 103;
        (*kermaS) += tmp;
        EGS_CorrelatedScoring ctmp(kermaT,kermaA);
        if( !ctmp.setState(data) ) return 104;
        (*cker) += ctmp;
        return 0;
}

/* These should be implemented in EGS_ScoringArray in the future */

/* get maximum value in scoring array */
EGS_Float EGS_CBCT::maxKerma(EGS_ScoringArray &kerma ) {
   double Kmax = 0, ktmp, dktmp;
   for(int j=0; j<kerma.regions(); j++) {
       kerma.currentResult(j, ktmp, dktmp);
       if (ktmp > Kmax) Kmax = ktmp;
   }
   return Kmax;
}

/* get maximum value in scoring array */
int EGS_CBCT::maxKermaReg(EGS_ScoringArray &kerma ) {
   double Kmax = 0, ktmp, dktmp;int jmax = 0;
   for(int j=0; j<kerma.regions(); j++) {
       kerma.currentResult(j, ktmp, dktmp);
       if (ktmp > Kmax) {Kmax = ktmp;jmax = j;}
   }
   return jmax;
}

/* get minimum value in scoring array */
EGS_Float EGS_CBCT::minKerma(EGS_ScoringArray &kerma ) {
   double Kmin = 1.e30, ktmp, dktmp;
   for(int j=0; j<kerma.regions(); j++) {
       kerma.currentResult(j, ktmp, dktmp);
       if (ktmp < Kmin) Kmin = ktmp;
   }
   return Kmin;
}
/* get average value in scoring array */
EGS_Float EGS_CBCT::aveKerma(EGS_ScoringArray &kerma ) {
   double Kave = 1.e30, ktmp, dktmp; int Nave = 0;
   for(int j=0; j<kerma.regions(); j++) {
       kerma.currentResult(j, ktmp, dktmp);
       if (ktmp > 0){Kave = ktmp; Nave++;}
   }
   return Nave>0 ? Kave/Nave : 0;
}

/* Maximum fraction Aatt = A/T = A/(S+A) */
EGS_Float EGS_CBCT::maxAatt() {
    double Amax = 0; double r,dr,r1,dr1;
    for(int j=0; j<kermaA->regions(); j++) {
        kermaT->currentResult(j,r,dr);
        if( r > 0 ) {
            kermaA->currentResult(j,r1,dr1);
            r1 /= r; if( r1 > Amax ) Amax = r1;
        }
    }
    return Amax;
}

/* Root mean squared error of Aatt = A/T error */
EGS_Float EGS_CBCT::aveErrorAatt(EGS_Float cut_off) {
    if( error_estimation == 1 ) {
        double avsc = 0;
        double sum = 0, r,dr; int n=0;
        for(int ix=ee_xmin; ix<=ee_xmax; ix++) {
            for(int iy=ee_ymin; iy<=ee_ymax; iy++) {
                int j = ix + iy*Nx;
                kermaS->currentResult(j,r,dr);
                avsc += r; sum += dr*dr; ++n;
            }
        }
        avsc /= n; sum = sqrt(sum/n);
        return 100*sum/avsc;
    }

    double Amax;
    /* Compute RMSE for regions with A/T > cut_off*Amax */
    if( cut_off > 0 ) {
        Amax = maxAatt();
        egsInformation("\nMaximum Aatt found: %lg\n",Amax);
        Amax *= cut_off;
    }
    /* Compute RMSE for regions with A/T > cut_off */
    else Amax = -cut_off;
    double sum = 0, r,dr, r1,dr1; int n = 0;
    for(int j=0; j<kermaA->regions(); j++) {
        kermaT->currentResult(j,r,dr);
        if( r > 0 ) {
            kermaA->currentResult(j,r1,dr1);
            if( r1 > 0 ) {
                if( r1/r > Amax ) {
                //if( r1/r < Amax ) {
                    dr1 /= r1; dr /= r;
                    double corr = cker->getCorrelation(j);
                    double cov = (corr/(r*r1*current_case) - 1)/
                                 (current_case-1);
                    double dAatt2 = dr*dr + dr1*dr1 - 2*cov;
                    sum += dAatt2; ++n;
                }
            }
        }
    }
    if( n > 0 ) sum = sqrt(sum/n);
    else sum = 1;
    return 100*sum;
}

/* get average uncertainty in regions with values larger than
   a given fraction of the max. value. Given in percentage.
*/
EGS_Float EGS_CBCT::aveError(const char * label, EGS_ScoringArray &kerma,
                             const EGS_Float &cut_off ) {

   EGS_Hist *err_hist;
   if (verbose) err_hist = new EGS_Hist(100.,512);
   EGS_Float Kmax = maxKerma(kerma);
   int n = 0;
   double ktmp, dktmp, aveError = 0;
   for(int j=0; j<kerma.regions(); j++) {
       kerma.currentResult(j, ktmp, dktmp);
       if (ktmp >= cut_off*Kmax && ktmp > 0) {
           double aux = dktmp/ktmp;
           if (verbose) err_hist->score(aux*100.);
           aveError += aux*aux; n++;
       }
   }

   if (n > 0) aveError = sqrt(aveError/n);
   else aveError = 0;

   if (verbose){
      string suffix = label; suffix += ".egserr";
      EGS_Float mean=0.0, var = err_hist->currentHistVariance(mean);
      err_hist->outputHist(constructIOFileName(suffix.c_str(),true));
      egsInformation("\n---> mean error = %g var = %g\n", mean, var);
      if (err_hist) delete err_hist;
   }

   return 100.0*aveError;
}

EGS_Float EGS_CBCT::aveError(EGS_ScoringArray &kerma )
{
    double sum = 0, r,dr; int n=0;
    for(int j=0; j<kerma.regions(); j++) {
        kerma.currentResult(j,r,dr);
        if (r){sum += dr*dr/r/r; ++n;}
    }
    if (n) sum = sqrt(sum/n);
    return 100*sum;
}

/*
   Gets mean square error for contaminant scatter signal,
   i.e., for detector pixels with an actual attenuated signal.
   Pixels with no actual attenuated signal will have a negative
   total signal which can be used to clean them.
*/
EGS_Float EGS_CBCT::getAveScatErrorSqr(){
     float *the_blank = new float[Nx*Ny];
     for (EGS_I32 i = 0; i<Nx*Ny; i++){the_blank[i]=1.0;}
     /* read in blank scan values of Kerma */
     EGS_BinaryFile *blank_file = new EGS_BinaryFile(blank_scan.c_str());
     if (blank_file && blank_file->fileSize()==Nx*Ny*sizeof(float)){
             the_blank = blank_file->readValues();
     }
     else return 0;
     if (blank_file) delete blank_file;
     EGS_Float norm = current_case/source->getFluence();
     EGS_Float sum = 0, scan, dscan, scat, dscat; int n=0;
     for (EGS_I32 i = 0; i<Nx*Ny; i++){
        kermaT->currentResult(i,scan,dscan);
        kermaS->currentResult(i,scat,dscat);
        scan = (the_blank[i]>0.0 && scan  > 0.0) ?
               log(the_blank[i]/scan/norm) : 0.0;
        if ( scan > 0 ){
         if (scat){sum += dscat*dscat/scat/scat; ++n;}
        }
    }
    if (the_blank) delete [] the_blank;
    if (n) sum = sum/n;
    return sum;
}

/* Prints out a histogram of the errors */
void EGS_CBCT::errorDistribution(const char * label, EGS_ScoringArray &kerma )
{
    string suffix = label;
    int hsize = 100;
    vector<int> hist(hsize); double min = 0.0, max=-1e30;
    for (int ih=0; ih < hist.size(); ih++){hist[ih]=0;};
    double r,dr; int n=0;

    for(int j=0; j<kerma.regions(); j++) {
        kerma.currentResult(j,r,dr);
        if (r && dr/r > max){max = dr/r;}
    }
    max = 100.*max;
    double bw = (max-min)/hsize, re;

    for(int j=0; j<kerma.regions(); j++) {
        kerma.currentResult(j,r,dr);
        if (r){
            re = 100.*dr/r; ++n;
            int index = int( (re-min)/bw - fmod(re,bw) );
            if (re >= min && re <= max) hist[index]+=1;
            else if(re > max)           hist[hsize-1]+=1;
            else                        hist[0] +=1;
        }
    }
    suffix += ".egserr";
    string fname = constructIOFileName(suffix.c_str(),true);
    ofstream fout(fname.c_str());
    for (int ih=0; ih<hist.size(); ih++){
         fout << min + bw*(ih + 0.5) <<" "<< float(hist[ih])/float(n) << endl;
    }
}


float* EGS_CBCT::getArray(EGS_ScoringArray *a) {
    float *res = new float [Nx*Ny];
    double r,dr;
    for(int ix=0; ix<Nx; ix++) {
        for(int iy=0; iy<Ny; iy++) {
            a->currentResult(ix+iy*Nx,r,dr);
            if ( cbctS->swapX2Y() ){
             res[iy+ix*Ny] = float(r);// output X as Y
            }
            else{
             res[ix+iy*Nx] = float(r);
            }
        }
    }
    return res;
}

float * EGS_CBCT::getAttTotalRatio(){
    float * tot = new float[Nx*Ny];
    double r,dr, r1, dr1;
    for(int i=0; i<Nx; i++) {
        for(int j=0; j<Ny; j++) {
            int reg = i + j*Nx;
            kermaT->currentResult(reg,r,dr);
            kermaA->currentResult(reg,r1,dr1);
            if ( cbctS->swapX2Y() ){ tot[j+i*Ny] = r > 0 ? float(r1/r) : 0;}
            else                   { tot[i+j*Nx] = r > 0 ? float(r1/r) : 0;}
        }
    }
    return tot;
}

vector<string> EGS_CBCT::getAtt(){
    vector<string> rAtt;
    string str; char buf[8192];
    char c = '%';
    for(int j=0; j<Nx*Ny; j++) {
      double r,dr; kermaT->currentResult(j,r,dr);
      if( r > 0 ) dr = 100*dr/r; else dr = 100;
      EGS_Float norm = 1.602e-10*current_case/source->getFluence();
      norm /= vx*vy; // voxel area
      sprintf(buf," %6d  %10.4le +/- %-7.3lf%c ", j, r*norm,dr,c);
      str = buf;
      double r1,dr1;
      kermaA->currentResult(j,r1,dr1);
      if( r1 > 0 ) dr1 = 100*dr1/r1; else dr1 = 100;
      sprintf(buf,"  %10.4le +/- %-7.3lf%c ",r1*norm,dr1,c);
      str += buf;
      double Aatt,dAatt;
      if( r > 0 ) {
        double corr = cker->getCorrelation(j);
        if( r1 > 0 ) {
            double cov = (corr/(r*r1*current_case) - 1)/
                (current_case-1);
            dAatt = sqrt(dr*dr + dr1*dr1 - 2e4*cov);
            Aatt = r1/r;
        }
        else { Aatt = 0; dAatt = 0; }
      }
      else { Aatt = 0; dAatt = 100; }
      sprintf(buf,"  %7.5lf +/- %-7.3lf%c\n",Aatt,dAatt,c);
      str += buf;
      rAtt.push_back(str);
    }
    return rAtt;
}

/*! Output the results of a simulation. */
void EGS_CBCT::outputResults() {

     char c = '%';
     egsInformation("\n\n last case = %lld fluence = %g\n\n",
                    current_case,source->getFluence());
     egsInformation("\n***** This is a CBCT calculation.\n\n");

     //EGS_Float aError = aveError(*kermaT, cut_off);
     EGS_Float attError = aveErrorAatt(cut_off);
     EGS_Float aError = aveError(*kermaA);
     EGS_Float sError = aveError(".scatter",*kermaS, cut_off);
     EGS_Float tError = aveError(*kermaT);
     EGS_Float cpuTime = run->getCPUTime();

     if (verbose) errorDistribution(".total",*kermaT);


     egsInformation("==================================================\n\n");
     if( error_estimation == 1 )
         egsInformation(" Average Error of scatter signal = %g%c\n",attError,c);
     else {
/*       if( cut_off > 0 )
         egsInformation(" => Average Error for Aatt < %3.1f*Amax = %g%c\n",
                    cut_off,attError,c);
       else
         egsInformation(" => Average Error for Aatt > %6.4f = %g%c\n",
                 -cut_off,attError,c);
         egsInformation("    Aatt calculation efficiency    = %g 1/second\n\n",
                    1e4/(cpuTime*attError*attError));*/
         egsInformation(" => Average Error of primary signal = %g%c\n",aError,c);
         if (aError)
           egsInformation("    Katt calculation efficiency    = %g 1/second\n\n",
                    1e4/(cpuTime*aError*aError));
         egsInformation(" => Average Error of scatter signal = %g%c\n",sError,c);
         if (sError)
           egsInformation("    Kscat calculation efficiency    = %g 1/second\n\n",
                    1e4/(cpuTime*sError*sError));
         egsInformation(" => Average Error of total signal = %g%c\n",tError,c);
         if (tError)
           egsInformation("    Ktot calculation efficiency    = %g 1/second\n\n",
                    1e4/(cpuTime*tError*tError));

         EGS_Float scError = getAveScatErrorSqr();// mean square error
         egsInformation(" => mean ave. scatter error = %g%c\n",100*sqrt(scError),c);
         if (scError)
         egsInformation("    contaminant scat. calc. eff. = %g 1/second\n\n",
                    1./(cpuTime*scError));

       }
/*     if (aError > epsilon){
        egsInformation(" histories to get %g%c uncertainty    = %g \n",
                    epsilon,c,aError*aError/epsilon/epsilon*current_case);
        egsInformation("( this will take about %g seconds on this machine )\n\n",
                    aError*aError/epsilon/epsilon*cpuTime);
     }*/
     egsInformation("==================================================\n\n");

// if (patt_have){
// cout << "\nNatt_min = " << Natt_min << endl;
// cout <<   "Natt_max = " << Natt_max << endl;
// printNsplit();
// }

EGS_Float K_max = maxKerma(*kermaS), K_min = minKerma(*kermaS);
egsInformation("Scatter signal: Kmax = %g Kmin = %g ",K_max, K_min);
if(K_min)
 egsInformation("ratio Kmax/Kmin = %g\n", K_max/K_min);

     if (splitter && verbose){
        splitter->printStatus();
        splitter->printImportances(constructIOFileName(".egsimp",true));
        int NgX=splitter->getGridNx(),
            NgY=splitter->getGridNy(),
            NgZ=splitter->getGridNz(),
            NgXY = NgX*NgY, Ng = NgXY*NgZ, j = 0;
        float* cimp = new float[Ng];
        for(int iz=0; iz<NgZ; iz++){
          for(int iy=0; iy<NgY; iy++){
            for(int ix=0; ix<NgX; ix++){
              int ir = ix + iy*NgX + iz*NgXY;
              cimp[j++]=float(splitter->fetchImportance(ir));
            }
          }
        }
        string the_imp_file = constructIOFileName(".bin.egsimp",true);
        EGS_I64 block_size = Ng*sizeof(float);
        EGS_BinaryFile *bfile = new EGS_BinaryFile(
                                the_imp_file.c_str(),
                                block_size,
                                (char *)cimp);
        bfile->writeBinary(0);
        delete bfile; //delete [] cimp;
     }

     if (c_att && verbose){
         c_att->printStatus();
         c_att->printCorrections(constructIOFileName(".egscorr",true));
        int NgX=c_att->getGridNx(),
            NgY=c_att->getGridNy(),
            NgZ=c_att->getGridNz(),
            NgXY = NgX*NgY, Ng = NgXY*NgZ, j = 0;
        float* corr = new float[Ng];
        for(int iz=0; iz<NgZ; iz++){
          for(int iy=0; iy<NgY; iy++){
            for(int ix=0; ix<NgX; ix++){
              int ir = ix + iy*NgX + iz*NgXY;
              corr[j++]=float(c_att->fetchCorrection(ir));
            }
          }
        }
        string the_corr_file = constructIOFileName(".bin.egscorr",true);
        EGS_I64 block_size = Ng*sizeof(float);
        EGS_BinaryFile *bfile = new EGS_BinaryFile(
                                the_corr_file.c_str(),
                                block_size,
                                (char *)corr);
        bfile->writeBinary(0);
        delete bfile;
     }
}

/*! Output egsmap or profile if requested. */
void EGS_CBCT::printProfiles() {

     if( !profile || !profile->isDefined() ){

        char c = '%';
        string fname = constructIOFileName(".egsmap",true);
        ofstream fout(fname.c_str());

        fout << "  reg #   Ktot                       Katt" <<
                "                     Kscatt   \n"         ;
        fout << "----------------------------------" <<
               "--------------------------------------------\n";
        char buf[8192]; string str;
        for(int j=0; j<Nx*Ny; j++) {
            double r,dr; kermaT->currentResult(j,r,dr);
            if( r > 0 ) dr = 100*dr/r; else dr = 100;
            EGS_Float norm = 1.602e-10*current_case/source->getFluence();
            norm /= vx*vy; // voxel area
            sprintf(buf," %6d  %10.4le +/- %-7.3lf%c ", j, r*norm,dr,c);
            str = buf;
            double r1,dr1;
            kermaA->currentResult(j,r1,dr1);
            if( r1 > 0 ) dr1 = 100*dr1/r1; else dr1 = 100;
            sprintf(buf," %10.4le +/- %-7.3lf%c ", r1*norm,dr1,c);
            str += buf;

            double r2,dr2;
            kermaS->currentResult(j,r2,dr2);
            if( r2 > 0 ) dr2 = 100*dr2/r2; else dr2 = 100;
            sprintf(buf," %10.4le +/- %-7.3lf%c \n", r2*norm,dr2,c);
            str += buf;

/*
            double Aatt,dAatt;
            if( r > 0 ) {
              double corr = cker->getCorrelation(j);
              if( r1 > 0 ) {
                  double cov = (corr/(r*r1*current_case) - 1)/
                      (current_case-1);
                  dAatt = dr*dr + dr1*dr1 - 2e4*cov;
                  if( dAatt > 0 ) dAatt = sqrt(dAatt); else dAatt = 0;
                  //dAatt = sqrt(dr*dr + dr1*dr1 - 2e4*cov);
                  Aatt = r1/r;
              }
              else { Aatt = 0; dAatt = 0; }
            }
            else { Aatt = 0; dAatt = 100; }
            sprintf(buf,"  %7.5lf +/- %-7.3lf%c\n",Aatt,dAatt,c);
            str += buf;
*/
            fout << str.c_str();
        }
        fout << "\n\n";

     }
     else{
        profile->saveProfile(constructIOFileName(".egsmap",true),getAtt());
     }
}


/*! Prints scans. */
void EGS_CBCT::printScans() {

  /* Creates either a profile or a whole printout of the detector scan */
  if (egsmap) printProfiles();

  if (scan_type != none){

     EGS_Float norm = current_case/source->getFluence();
     EGS_I64 block_size = Nx*Ny*sizeof(float);
     EGS_I64 block_pos  = ((EGS_I64)(cbctS->atStep()+0.5))*block_size;
                             //Nx*Ny*sizeof(float);

     egsInformation("===> This is projection # %d out of %d \n\n",
                    ((EGS_I64)(cbctS->atStep()+0.5)),cbctS->steps());
cout << "->projection # " << cbctS->atStep() << " out of " << cbctS->steps() << endl;
     //cbctS->describeMe();

     // it is a blank scan calculation
     if (!blank_scan.length() && real_scan.length() && scan_type == blank){
           float *scan = getArray(kermaT);
           for (EGS_I32 i = 0; i<Nx*Ny; i++){scan[i]=norm*scan[i];}
           EGS_BinaryFile *bfile = new EGS_BinaryFile(real_scan.c_str(),
                                                      block_size,
                                                     (char *)scan);
           bfile->writeBinary(block_pos);
           delete bfile; //delete [] scan;
           return;
     }
     if (blank_scan.length() && scan_type != scatter){
        // get blank scan: values set initially to 1
        float *blank = new float[Nx*Ny];
        for (EGS_I32 i = 0; i<Nx*Ny; i++){blank[i]=1.0;}

        // write total kerma to the proper position on the
        // projection file

        /* read in blank scan values of Kerma */
        EGS_BinaryFile *blank_file = new EGS_BinaryFile(blank_scan.c_str());
        if (blank_file->fileSize()==Nx*Ny*sizeof(float)){
             blank = blank_file->readValues();
        }
        else { // blank scan is unity
            egsWarning("\n\n***** Wrong size or empty blank scan file!!!\n"
                       "Output will be just the scan signal,\n\n");
            egsWarning("blank file is: %s \n",blank_file->Name());
            egsWarning(" blank file size = %d required size = %d\n\n",
            blank_file->fileSize(), Nx*Ny*sizeof(float)
            );
        }
        // delete blank scan file since no longer needed
        delete blank_file;
        /* create a real scan */
        if ( real_scan.length() && scan_type != ideal ){// it's real, both or all
           float *scan = getArray(kermaT);
           for (EGS_I32 i = 0; i<Nx*Ny; i++){
              // check that blank and scan are not zero
               scan[i]  = (blank[i]>0.0 && scan[i]  > 0.0) ?
                           log(blank[i]/scan[i]/norm) : 0.0;
           }
           EGS_BinaryFile *bfile = new EGS_BinaryFile(real_scan.c_str(),
                                                      block_size,
                                                     (char *)scan);
           bfile->writeBinary(block_pos);
           delete bfile; //delete [] scan;
        }
        /* create an attenuated (ideal) scan */
        if ( real_scan.length() && scan_type != real ){// it's ideal, both or all
           float *scan_a = getArray(kermaA);//, *scan_atmp = new float[Nx*Ny];
           for (EGS_I32 i = 0; i<Nx*Ny; i++){
              // check that blank and scan are not zero
              scan_a[i]= (blank[i]>0.0 && scan_a[i]> 0.0) ?
                          log(blank[i]/scan_a[i]/norm) : 0.0;
           }

           string real_att = real_scan;
           //if(scan_type!=ideal)
           real_att.replace(real_att.rfind(".scan",real_att.length()-1),
                                      5,".att.scan");
           EGS_BinaryFile *batt = new EGS_BinaryFile(real_att.c_str(),
                                      block_size, (char *)scan_a);
           batt->writeBinary(block_pos);
           delete batt;
        }
        if (blank) delete [] blank;
     }
     /* get scatter corrections */
     if (real_scan.length() && (scan_type == scatter || scan_type == all)){
         string scat_file = real_scan;
         //if(scan_type!=scat)
         scat_file.replace(scat_file.rfind(".scan",scat_file.length()-1),
                                  5,".scatonly.scan");
         float *scan = new float [Nx*Ny], *err = new float [Nx*Ny];
         for(int i=0; i<Nx; i++) {
             for(int j=0; j<Ny; j++) {
                 double r,dr; int reg = i + j*Nx;
                 kermaS->currentResult(reg,r,dr);
                 //scan[j+i*Ny] = r*pnorm; err[j+i*Ny] = dr*pnorm;
                 if ( cbctS->swapX2Y() ){
                  scan[j+i*Ny] = r*pnorm*norm; err[j+i*Ny] = dr*pnorm*norm;
                 }
                 else                  {
                  scan[i+j*Nx] = r*pnorm*norm; err[i+j*Nx] = dr*pnorm*norm;
                 }
                 if( m_real && m_blank ) {
                     double aux = exp(-m_real[i+j*Nx])*m_blank[i+j*Nx];
                     //scan[j+i*Ny] *= aux; err[j+i*Ny] *= aux;
                     if ( cbctS->swapX2Y() ){
                       scan[j+i*Ny] *= aux; err[j+i*Ny] *= aux;
                     }
                     else                  {
                       scan[i+j*Nx] *= aux; err[i+j*Nx] *= aux;
                     }
                 }
             }
         }
         EGS_BinaryFile *bscatonly = new EGS_BinaryFile(scat_file.c_str());
         bscatonly->writeBinary((char *)scan,block_size,2*block_pos);
         bscatonly->writeBinary((char *)err ,block_size,2*block_pos+block_size);
         delete bscatonly;

         /**********************************/
         /* smoothing scatter distribution */
         /**********************************/
         if (do_smoothing){

             string smooth_file = real_scan;
             smooth_file.replace(
             smooth_file.rfind(".scan",smooth_file.length()-1),
             5,".smoothed.scatonly.scan");
             //ofstream smooth_stream(smooth_file.c_str(),ios::binary | ios::in | ios::out);

             EGS_Distribution2D the_scat(Nx, Ny, scan, err);
             EGS_Smoothing smoo;
             smoo.setNmax2d(nmax2d); smoo.setNmax(nmax);
             smoo.setChi2Max(chi2max); smoo.setDmin(0.02);
             smoo.setDimensions(the_scat.nx,the_scat.ny);
             egsInformation("===> smoothing scatter distribution\n");
             EGS_Distribution2D *smoothed = smoo.smooth1(&the_scat);
             if( !smoothed ) {
               egsWarning("!!!!!\nError while smoothing!!!!\n!!!!!\n");
             }
             else{

               EGS_Float rmsd = 0; int nc = 0;
               for (int i=0; i<Nx*Ny; i++){
                   EGS_Float s2 = smoothed->d_array[i]*smoothed->d_array[i],
                            ds2 = smoothed->error_array[i];
                   if (s2 == 0) continue;
                   rmsd += ds2/s2; nc++;
               }
               rmsd /= nc; rmsd = sqrt(rmsd);
               EGS_Float cpuTime = run->getCPUTime();char c = '%';
               egsInformation(" => Average Error of smoothed scatter signal = %g%c\n",
                              100.*rmsd,c);
               if (rmsd)
                egsInformation("    smoothed Kscat calculation efficiency    = %g 1/second\n\n",
                               1./(cpuTime*rmsd*rmsd));

               EGS_BinaryFile *smooth_scatonly = new EGS_BinaryFile(smooth_file.c_str());
               smooth_scatonly->writeBinary((char*)smoothed->d_array,
                                                    smoothed->nreg*sizeof(float),
                                                    block_pos);
               delete smooth_scatonly;
             }
         }
         /**********************************/
         delete [] scan; delete [] err;
     }
  }

}

/*! Output the results of a simulation. */
int EGS_CBCT::finishSimulation() {
    int err = EGS_Application::finishSimulation();
    egsInformation("finishSimulation(%s) %d\n",app_name.c_str(),err);

    if( err <= 0 ) {// interactive run or not last parallel job
       if (getNparallel()==0) {// interactive run
          egsInformation("\n Running an interactive job!!!\n\n");
          printScans();
       }
       return err;
    }

    // if err is > 0, this is the last job in a parallel run
    // => we have to combine the results
    // EGS_Application::finishSimulation() has already called
    // our finishRun() which has called egs_finish => all data has been
    // moved to the user code directory and the working directory has been
    // reset to be the user code directory. We must now reset the
    // output_file name and re-open units.
    output_file = final_output_file; the_egsio->i_parallel = 0;
    int flag = 0; egsOpenUnits(&flag);
    // The following is necessary because finishRun() was called from
    // EGS_Application::finishSimulation() and this resets I/O
    // to stdout and stderr. But if we are here, we are combining
    // the results of a parallel run and we want the output to go
    // into the combined run log file.
    io_flag = 1;
    final_job = true;
    describeUserCode(); describeSimulation();
    err = combineResults(); if (egsdat) outputData();
    if( err ) return err;
    run->finishSimulation();
    outputResults(); printScans();
    egsInformation("\n Running %d parallel jobs!!!\n\n",getNparallel());
    finishRun();
    return 0;
}

/*! Get the current simulation result.  */
void EGS_CBCT::getCurrentResult(double &sum, double &sum2, double &norm,
                               double &count) {
        count = current_case; double flu = source->getFluence();
        norm = flu > 0 ? 1.602e-10*count/(flu*vx*vy) : 0;
        EGS_I32 i = Nx/2 * (Ny+1);
        if (dtype == total)
           kermaT->currentScore(i,sum,sum2);
        else if(dtype == attenuated)
           kermaA->currentScore(i,sum,sum2);
        else
           kermaS->currentScore(i,sum,sum2);
        //kermaT->currentScore(maxKermaReg(*kermaT),sum,sum2);

        //if (splitter){splitter->updateSplitting();}
        //if (splitter && splitter->isWarming()){splitter->stopWarming();}
         if (splitter){
             if (splitter->isWarming()){
                splitter->stopWarming();
                //kermaS->reset();
             }
             if (splitter->scoringLatch())
                splitter->updateSplitting();
             else
                splitter->updateSplitting(patt_score);
         }

         if (c_att){
             if (c_att->isWarming())
                c_att->stopWarming();
             c_att->updateCorrections(patt_score);
            // //c_att->updateCorrections();
            //egsInformation("\n---> current C = %g Katt = %g\n",
            //               c_att->currentC(), patt_score);
         }

//          if (hist&&patt_have){
//             EGS_Float mean=0.0, var = hist->currentHistVariance(mean);
//             char buf[128]; sprintf(buf,"_batch_%d.egshist",hist->getStatus()) ;
//             string prefix = buf;
//             hist->outputHistwithErrors(constructIOFileName(prefix.c_str(),true));
//             //hist->outputHist(constructIOFileName(prefix.c_str(),true));
//             egsInformation("\n---> mean = %g var = %g\n", mean, var);
//          }

    if( split_geom && split_count_tot > 0 ) {
        int nr = split_geom->regions();
        double aux = split_collect_tot/split_count_tot;
        //egsInformation("Computing importances: stot=%lg ctot=%lld nr=%d %lg"
        //        " lg\n",
        //        split_collect_tot,split_count_tot,nr,aux,aux/nsplit_p);
        double sum=0; EGS_I64 nc=0;
        for(int j=0; j<nr; j++)
            if( split_count[j] > 0 ) {
                //++nc; sum += split_collect[j]/split_count[j];
                sum += split_collect[j]; nc += split_count[j];
            }
        //egsInformation("\n     %lg  ",
        //        split_collect_tot/split_count_tot);
        //sum /= nc; double smin = 2*sum/nsplit_p;
        sum /= nc; double smin = sum/aux;
        sum = split_collect_tot/(split_count_tot*(sum+smin)*nsplit_p);
        /*
        double sum1 = 0, sum2 = 0;
        for(int j=0; j<nr; j++) {
            if( split_count[j] > 0 ) {
                double aux = (smin+split_collect[j]/split_count[j])*sum;
                aux *= nsplit_p; if( aux < 1 ) aux = 1;
                sum1 += aux*split_count[j]; sum2 += split_count[j];
            }
        }
        sum1 /= sum2;
        egsInformation("\n %lg %lg %lg\n",
                sum1,split_collect_tot/split_count_tot,
                split_collect_tot/(sum1*split_count_tot));
        sum *= split_collect_tot/(sum1*split_count_tot);
        */
        if( split_count_tot_save > 0 ) {
            double old_split = split_collect_tot_save/split_count_tot_save;
            double new_split = (split_collect_tot-split_collect_tot_save)/
                (split_count_tot-split_count_tot_save);
            //egsInformation("%lg   ",old_split/new_split);
            sum *= old_split/new_split;
            sum *= old_split/new_split;
        }
        else {
            split_collect_tot_save = split_collect_tot;
            split_count_tot_save = split_count_tot;
        }
        //egsInformation("Regions:\n");
        for(int j=0; j<nr; j++) {
            if( split_count[j] > 0 ) {
                //int iz = j/100; int iy = (j-100*iz)/10; int ix = j-100*iz-10*iy;
                //egsInformation("%6d %4d %4d %4d %8lld %14.6e %14.7e %14.7e\n",
                //        j,ix,iy,iz,split_count[j],
                //        split_collect[j]/split_count[j],
                //        split_collect[j]/split_count[j]*norm,
                //        split_collect[j]/split_count[j]*sum);
                split_current[j] = (smin+split_collect[j]/split_count[j])*sum;
            }
        }
        /*
        sum=0; nc=0; double sum1=0; EGS_I64 nc1=0;
        for(int j=0; j<split_ne_bins; j++) {
            if( split_e_count[j] > 0 ) {
                sum1 += split_e_collect[j]; nc1 += split_e_count[j];
                sum += split_e_collect[j]/split_e_count[j]; ++nc;
                //split_e_current[j] = split_e_collect[j]/split_e_count[j];
            }
            //else split_e_current[j] = 1;
        }
        sum /= nc; sum1 /= nc1;
        egsInformation("\nEnergy bins:\n");
        for(int j=0; j<split_ne_bins; j++) {
            double aux;
            if( split_e_count[j] > 0 ) {
                aux = split_e_collect[j]/split_e_count[j];
                split_e_current[j] = split_e_collect[j]/split_e_count[j]/sum;
            }
            else { split_e_current[j] = 1; aux = 0; }
            egsInformation("%8lld  %14.7le  %14.7le  %12.8f\n",split_e_count[j],
               split_e_collect[j],aux/sum1,split_e_current[j]);
        }
        */
    }
}

/*! simulate a shower */
int EGS_CBCT::shower() {
#ifdef USTEP_DEBUG
    the_geometry_debug->nstep = 0;
    the_geometry_debug->icase = current_case;
#endif
#ifdef DEBUG_WEIGHTS
    interactions.nnow = 0;
#endif
    if (splitter) C_imp_save = 1;
    if( p.E < the_bounds->pcut ) return 0;
    if( p.q && p.wt ) {egsWarning("This application is not meant to transport "
            "electrons! wt = %g q=%d E=%g\n",p.wt,p.q,p.E); return 0;}
    if( forced_detection ) {
        EGS_Float d;
        int ireg = hitsScreen(p.x,p.u,&d);
        int nspl = nsplit_p;
        /**************************************************************
         If particle not headed towards detector, play Russian Roulette
         with probability nsplit_p/nsplit_s and if it survives then set
         splitting to nsplit_s and weight to wt*nsplit_s/nsplit_p
         NOTE: Using nsplit_s <= nsplit_p effectively turns-off RR
               since particles will always survive.
        ***************************************************************/
        if( ireg < 0 ) {
          if (!patt_have){// RDIS or FS
            if (nsplit_s > nsplit_p){
               if( rndm->getUniform()*nsplit_s > nsplit_p ) return 0; //kill particle
               nspl = nsplit_s; p.wt *= nsplit_s; p.wt /= nsplit_p;   //keep particle
            }
          }
          else{// PDIS does not use n_p !!!!
            if (nsplit_s > f_split){
               if( rndm->getUniform()*nsplit_s > f_split ) return 0; //kill particle
               nspl = nsplit_s; p.wt *= nsplit_s; p.wt /= f_split;   //keep particle
            }
          }
        }
        /***************************************************************/

        the_extra_stack->katt[0] = 0;
        the_extra_stack->ddet[0] = d;
        the_extra_stack->iphat[0] = nspl;
        the_extra_stack->irdet[0] = ireg;
    }
    return EGS_AdvancedApplication::shower();
}

bool EGS_CBCT::computePhotonScore(EGS_CBCT_Photon &p, int nspl,
        double &sc) {

    //egsInformation("\n*** computePhotonScore():\n"
    //      "E=%g wt=%g latch=%d nspl=%d\n",p.E,p.wt,p.latch,nspl);
    //int nsec = computeIntersections(p.ir,p.x,p.u,gsections);
    int nsec = computeIntersections(p.ir,p.x,p.u);
    double lambda, lambda_old; bool not_interacted = false;
    EGS_Float eta, d_eta = 1, wt = p.wt; int ispl = 0;
    if( nspl > 0 ) {
        eta = rndm->getUniform();
        if( nspl > 1 ) { d_eta = 1./nspl; eta /= nspl; p.wt *= d_eta; }
        eta = 1 - eta; lambda = -log(eta);
        not_interacted = true; lambda_old = lambda;
        //egsInformation("lambda=%g\n",lambda);
    }
    double Lambda = 0, tlast = 0, ynew, tstep, gmfp = 1e30;
    int imed = -1;
    for(int j=0; j<nsec; j++) {

        double tnew = gsections[j].t; tstep = tnew - tlast;
        if( imed != gsections[j].imed ) {
            imed = gsections[j].imed;
            if( imed >= 0 )
                gmfp = i_gmfp[imed].interpolateFast(p.gle);
            else gmfp=1e30;
        }
        double sigma = gsections[j].rhof/gmfp;
redo_step:
        double this_lambda = tstep*sigma;
        if( not_interacted ) {
            //egsInformation("step: tnew=%g tstep=%g this_lambda=%g lambda=%g\n",
            //        tnew,tstep,this_lambda,lambda);
            if( this_lambda < lambda ) lambda -= this_lambda;
            else if(imed >= 0){
                EGS_Float tt = lambda/sigma;
                if( ++ispl < nspl ) {
                    Lambda += lambda;
                    eta -= d_eta; lambda = -log(eta) - lambda_old;
                    lambda_old += lambda;
                    //egsInformation("ispl=%d next lambda: %g lambda_old=%g\n",
                    //        ispl,lambda,lambda_old);
                    EGS_CBCT_Photon p1(p); p1.x += p1.u*(tlast + tt);
                    p1.imed = imed; p1.ir = gsections[j].ireg;
                    p1.latch = -p1.latch-1; pc.addParticle(p1);
                    tstep -= tt; tlast += tt;
                    goto redo_step;
                }
                else {
                    //egsInformation("final interaction\n");
                    p.x += p.u*(tlast + tt);
                    p.imed = imed; p.ir = gsections[j].ireg;
                    not_interacted = false;
                }
            }
        }

        tlast = tnew; Lambda += this_lambda;
        if( Lambda > 80 && !not_interacted ) break;
    }

    if( Lambda < 80 ) {
        double _muen = muen->interpolateFast(p.gle);
        double exp_Lambda = exp(-Lambda);
        double up = a*p.u;
        double aup = fabs(up); if( aup < 0.08 ) aup = 0.08;
        sc = wt/aup*exp_Lambda*_muen;
    } else sc = 0;

    return nspl > 0 && !not_interacted ? true : false;

}

static int count_print = 0;

void EGS_CBCT::transportPhoton(EGS_CBCT_Photon &p) {
    while(1) {
        transportSinglePhoton(p);
        if( pc.nnow <= 0 ) break;
        p = pc.takeParticle();
    }
}

void EGS_CBCT::transportSinglePhoton(EGS_CBCT_Photon &p) {

    if( p.latch < 0 ) {
        p.latch = -p.latch-1; goto do_interaction;
    }

new_step:
    /*
       egsInformation("*** transportSinglePhoton(): E=%g wt=%g latch=%d\n"
       " x=(%g,%g,%g) u=(%g,%g,%g) reg=%d med=%d\n",
       p.E,p.wt,p.latch,p.x.x,p.x.y,p.x.z,p.u.x,p.u.y,p.u.z,p.ir,p.imed);
     */
    bool do_interaction;
    if( !p.latch ) {
        double sc;
        do_interaction = computePhotonScore(p,nsplit_s,sc);
        if( sc > 0 ) {

            // only collect average score of particles that interact in
            // the geometry to avoid large contributions from photons
            // not entering the geometry (or just going through air/vacuum)
            if( do_interaction ) {
                patt_score_sum += sc; ++patt_score_count; //<== original
            }

            //kermaT->score(k,sc);
            cker->score(p.isc,0,sc);
            if( p.latch == 0 ) kermaA->score(p.isc,sc);
            else               kermaS->score(p.isc,sc);
        }
        if( !do_interaction ) return;
    }
    else {

        // The maximum cross section
        EGS_Float gmfp_max = i_gmfp[dt_medium].interpolateFast(p.gle)/rhormax;
        do_interaction = false; EGS_Float gmfp = -1;
        EGS_Float lambda = -log(1-rndm->getUniform());
        while(1) {

            if( p.imed < 0 || p.imed == iair ) {
                //
                // *** Don't do delta transport if the particle is in air or in
                //     vacuum. Instead transport until the particle ends up in
                //     another medium, exits the geometry, or interacts
                //
                while(1) {
                    EGS_Float t;
                    if( iair >= 0 && p.imed == iair ) {
                        if( gmfp < 0 ) {
                            gmfp = i_gmfp[iair].interpolateFast(p.gle);
                        }
                        t = lambda*gmfp;
                    }
                    else t = 1e30;
                    int newmed;
                    int inew = geometry->howfar(p.ir,p.x,p.u,t,&newmed);
                    if( inew < 0 ) {
                        //
                        // *** Particle exits the geometry.
                        //
                        return;
                    }
                    p.x += p.u*t;
                    if( inew == p.ir ) {
                        //
                        // *** Particle interacts
                        //
                        do_interaction = true; break;
                    }
                    p.ir = inew;
                    if( iair >= 0 && p.imed == iair ) lambda -= t/gmfp;
                    if( newmed != p.imed ) {
                        p.imed = newmed;
                        if( newmed >= 0 && newmed != iair ) break;
                        //
                        // *** i.e., we are not in air or vacuum => jump out of
                        //     loop to do delta transport
                        //
                    }
                }
                if( do_interaction ) break;
            }
            EGS_Float t = lambda * gmfp_max; p.x += p.u*t;
            p.ir = geometry->isWhere(p.x);
            if( p.ir < 0 ) return;
            p.imed = geometry->medium(p.ir);
            if( p.imed >= 0 ) {
                gmfp = i_gmfp[p.imed].interpolateFast(p.gle);
                EGS_Float rhor = 1;
                if( geometry->hasRhoScaling() )
                    rhor = geometry->getRelativeRho(p.ir);
                gmfp = gmfp/rhor;
                if( rndm->getUniform()*gmfp < gmfp_max ) {
                    do_interaction = true; break;
                }
            }
            lambda = -log(1-rndm->getUniform());
        }
    }

do_interaction:

    //
    // *** Time to interact. We use Klein-Nishina and photo-absorption
    //
    //egsInformation("interaction at x=(%g,%g,%g) reg=%d med=%d\n",
    //        p.x.x,p.x.y,p.x.z,p.ir,p.imed);
    EGS_Float gbr2 = i_gbr2[p.imed].interpolateFast(p.gle);
    if( rndm->getUniform() > gbr2 ) return;
    ++p.latch;

    int sreg = -1, ebin = -1;
    if( split_geom ) {
        sreg = split_geom->isWhere(p.x);
        if( split_ne_bins > 1 ) {
            //int ebin = (int) (split_e_a*p.gle + split_e_b);
            ebin = (int) (split_e_a*p.E + split_e_b);
            //int ebin = (int) (split_e_a*p.E*p.E + split_e_b);
        }
    }
    //int nspl = p.phat;
    //EGS_Float aspl_tot = nspl;
    EGS_Float aspl_tot; int nspl;
    if( sreg >= 0 && split_current[sreg] > 0 ) {
        /*
           if( ++count_print < 1000 )
           egsInformation("Interacting at (%g,%g,%g) sreg=%d split_current=%g\n",
           p.x.x,p.x.y,p.x.z,sreg,split_current[sreg]);
         */
        aspl_tot = split_current[sreg]*p.phat;
        if( aspl_tot <= 1 ) {
            aspl_tot = 1; nspl = 1;
        }
        else {
            nspl = (int) aspl_tot;
            if( rndm->getUniform() < aspl_tot - nspl ) ++nspl;
        }
    }
    else if( d_split > 0 ) {
        EGS_Float d = distance - p.x*a;
        if( d > 1 ) {
            EGS_Float s = d_split/d; s *= s; s *= s;
            if( patt_med >= 0 ) {
                EGS_Float patt_datt = patt_d - patt_a*p.x;
                if( patt_datt > 0 ) {
                    EGS_Float mui = f_split*i_gmfp[patt_med].interpolateFast(p.gle);
                    s *= exp(-patt_datt/mui);
                }
            }
            aspl_tot = s*p.phat;
            if( aspl_tot <= 1 ) {
                aspl_tot = 1; nspl = 1;
            }
            else {
                nspl = (int) aspl_tot;
                if( rndm->getUniform() < aspl_tot - nspl ) ++nspl;
            }
            //egsInformation("d=%g s=%g aspl_tot=%g nspl=%d\n",d,s,aspl_tot,nspl);
        }
        else {
            nspl = p.phat; aspl_tot = nspl;
        }
    }
    else {
        nspl = p.phat; aspl_tot = nspl;
    }
    EGS_Float br, temp, sint, rejf;
    EGS_Float wt = p.wt; EGS_Float wnew = wt/aspl_tot;

    bool have_scattered_photon = false; bool is_tracked = false;
    int keep = (int) (aspl_tot*rndm->getUniform());
    EGS_CBCT_Photon ps; double sctot = 0; int nc = 0;

    EGS_Float Ko = p.E/the_useful->rm;
    EGS_Float broi = 1 + 2*Ko; EGS_Float bro = 1/broi;
    EGS_Float bro1 = 1 - bro; EGS_Float rejmax = broi + bro;

    double ko = Ko;
    double sig_tot = (2*ko*(2+ko*(1+ko)*(8+ko))*bro*bro-
             (2+(2-ko)*ko)*log(broi))/(ko*ko*ko);
    EGS_Float zz = distance - p.x*a;
    EGS_Float w_screen = ax*ay/(M_PI*zz*zz*sig_tot);
    //egsInformation("\nInteraction at (%g,%g,%g) E=%g wt=%g\n",p.x.x,p.x.y,
    //        p.x.z,p.E,p.wt);
    //egsInformation("ko=%g sigtot=%lg zz=%g w_screen=%g\n",Ko,sig_tot,
    //        zz,w_screen);
    if( w_screen < 1 ) {
        EGS_Float asample = w_screen*aspl_tot;
        int nsample = (int) asample;
        if( rndm->getUniform() < asample - nsample ) ++nsample;
        //egsInformation("atot=%g nspl=%d asamp=%g nsamp=%d keep=%d\n",
        //        aspl_tot,nspl,asample,nsample,keep);
        zz = 0.5*zz*zz*zz; bool do_other = true;
        for(int j=0; j<nsample; j++) {
            /*
            EGS_Float eta_x = rndm->getUniform(),
                      eta_y = rndm->getUniform();
            int ix = (int) (eta_x*Nx), iy = (int) (eta_y*Ny);
            int isc = ix + iy*Nx;
            eta_x = (eta_x - 0.5)*ax; eta_y = (eta_y - 0.5)*ay;
            EGS_Vector xscreen(midpoint + ux*eta_x + uy*eta_y);
            */
            EGS_Vector xscreen; int isc = pselector->getPoint(rndm,xscreen);
            EGS_Vector u(xscreen - p.x);
            EGS_Float di = 1/u.length(); u *= di;
            EGS_Float cost = u*p.u;
            EGS_Float aux = 1./(1 + Ko*(1-cost));
            //di *= aux;
            EGS_Float prob = (1+aux*aux-aux*(1-cost*cost))*zz*di*di*di*aux;
            //egsInformation("%d xs=(%g,%g,%g) u=(%g,%g,%g) cost=%g d=%g prob=%g\n",
            //        j,xscreen.x,xscreen.y,xscreen.z,u.x,u.y,u.z,cost,1/di,prob);
            bool use_it = rndm->getUniform() < prob ? true : false;
            //if( rndm->getUniform() < prob ) {
                //egsInformation("keeping it, E=%g\n",p.E*aux);
                if ( keep == j && use_it ) do_other = false;
                if( p.E*aux > the_bounds->pcut ) {
                    EGS_CBCT_Photon p1(p); p1.E = p.E*aux; p1.wt = wnew;
                    p1.gle = log(p1.E); p1.u = u;
                    //int ns = keep != j ? 0 : 1;
                    int ns = keep == j && use_it ? 1 : 0;
                    double sc;
                    bool result = computePhotonScore(p1,ns,sc);
                    if( sc > 0 ) {
                        sc *= prob;
                        cker->score(isc,0,sc);
                        kermaS->score(isc,sc);
                        int ix = isc%Nx, iy = isc/Nx;
                        if( ix >= split_xmin && ix <= split_xmax &&
                                iy >= split_ymin && iy <= split_ymax ) {
                            sctot += sc; ++nc;
                        }
                    }
                    if( keep == j && result ) {
                        p1.wt = wt; p1.latch = -p1.latch-1;
                        transportSinglePhoton(p1);
                        //have_scattered_photon = true; ps = p1;
                        //is_tracked = true;
                    }
                }
            //}
        }
        if( do_other ) {
            ps = p;
            while(1) {
                do {
                    br = bro + bro1*rndm->getUniform();
                    temp = (1-br)/(Ko*br); sint = temp*(2-temp);
                    rejf = br*br + 1 - br*sint;
                } while ( rndm->getUniform()*rejmax*br > rejf );
                EGS_Float cphi, sphi; rndm->getAzimuth(cphi,sphi);
                EGS_Float cost;
                if( temp < 2 ) { cost = 1 - temp; sint = sqrt(sint); }
                else { cost = -1; sint = 0; }
                ps.u.rotate(cost,sint,cphi,sphi);
                if( hitsScreen(ps.x,ps.u,0) < 0 ) break;
                ps.u = p.u;
            }
            ps.E *= br;
            if( ps.E > the_bounds->pcut ) {
                have_scattered_photon = true;
                ps.gle = log(ps.E);
            }
        }
    }
    else {
        for(int j=0; j<nspl; j++) {
            do {
                br = bro + bro1*rndm->getUniform();
                temp = (1-br)/(Ko*br); sint = temp*(2-temp);
                rejf = br*br + 1 - br*sint;
            } while ( rndm->getUniform()*rejmax*br > rejf );
            if( p.E*br > the_bounds->pcut ) {
                EGS_CBCT_Photon p1(p); p1.E = p.E*br; p1.wt = wnew;
                p1.gle = log(p1.E);
                EGS_Float cphi, sphi; rndm->getAzimuth(cphi,sphi);
                EGS_Float cost = 1 - temp; sint = sqrt(sint);
                p1.u.rotate(cost,sint,cphi,sphi);
                int isc = hitsScreen(p1.x,p1.u,0); bool result;
                if( isc >= 0 ) {
                    int ns = keep != j ? 0 : 1;
                    double sc;
                    result = computePhotonScore(p1,ns,sc);
                    if( sc > 0 ) {
                        cker->score(isc,0,sc);
                        kermaS->score(isc,sc);
                        int ix = isc%Nx, iy = isc/Nx;
                        if( ix >= split_xmin && ix <= split_xmax &&
                                iy >= split_ymin && iy <= split_ymax ) {
                            sctot += sc; ++nc;
                        }
                        //++nc; if( sc > sctot ) sctot = sc;
                    }
                }
                if( keep == j ) {
                    if( (isc >= 0 && result) || isc < 0 ) {
                        have_scattered_photon = true; ps = p1;
                        if( isc >= 0 ) is_tracked = true;
                    }
                }
            }
        }
    }

    if( sreg >= 0 ) {
        split_collect_tot += aspl_tot; ++split_count_tot;
        if( nc > 0 ) {
            double av = aspl_tot*sctot/nc;
            split_collect[sreg] += av; ++split_count[sreg];
        }
    }

    if( have_scattered_photon ) {
        p = ps; p.wt = wt;
        if( is_tracked ) goto do_interaction;
        goto new_step;
    }
}

bool EGS_CBCT::checkVector(const EGS_Vector &v, const string & msg){
if (isnan(v.x) || isnan(v.y)||isnan(v.z)){
 egsWarning("%s \n "
          "=> NaN value: case = %d stack position = %d\n"
          "x = %g y = %g z = %g\n", msg.c_str(),current_case, the_stack->np-1,
           v.x, v.y, v.z);
 return false;
}
if (isinf(v.x)||isinf(v.y)||isinf(v.z)){
 egsWarning("%s \n"
          "=> Infinite value in :\n case = %d stack position = %d\n"
          "x = %g y = %g z = %g\n", msg.c_str(), current_case, the_stack->np-1,
           v.x, v.y, v.z);
 return false;
}
 return true;
}

/* Select photon mean-free-path: calculates Kerma at a given position */
void EGS_CBCT::selectPhotonMFP(EGS_Float &dpmfp) {
   int np = the_stack->np-1;
   EGS_Float LambdaInt = 0, aup_int = 1.0;
   //EGS_Float exp_Lambda = 1;

   if( the_stack->E[np] < the_bounds->pcut ) {
       --the_stack->np; dpmfp=-1; return;
   }

   /* check which scoring scheme to use */
   if (!forced_detection) {
       dpmfp = -log(1-rndm->getUniform()); return;
   }
/*
   if( the_xoptions->iraylr == 0 && the_xoptions->ibcmp == 0 &&
           dt_medium >= 0 ) {
       //egsInformation("in selectPhotonMFP: np=%d latch=%d\n",
       //        np,the_stack->latch[np]);
       EGS_CBCT_Photon p;
       p.x = EGS_Vector(the_stack->x[np],the_stack->y[np],the_stack->z[np]);
       p.u = EGS_Vector(the_stack->u[np],the_stack->v[np],the_stack->w[np]);
       p.E = the_stack->E[np]; p.gle = the_epcont->gle;
       p.wt = the_stack->wt[np];
       p.latch = the_stack->latch[np]; p.ir = the_stack->ir[np]-2;
       p.isc = the_extra_stack->irdet[np]; p.imed = the_useful->medium-1;
       p.phat = the_extra_stack->iphat[np];
       transportPhoton(p);
       --the_stack->np; dpmfp=-1; return;
   }
*/
   /* select number of mfp, possibly using a transformation */
   EGS_Float wtr = 1, eta; double lambda;
   EGS_Float asplit;
   int isc = the_extra_stack->irdet[np];
   //if( mfptr_do && !the_stack->latch[np] ) {
   if( mfptr_do && isc >= 0 ) {
       eta = 1/(1-rndm->getUniform()); EGS_Float aux = sqrt(eta);
       lambda = mfptr_lamo*(aux-1);
       asplit = mfptr_lamo*0.5*eta*aux;
       wtr = asplit*exp(-lambda);
   }
   else {
       eta = 1 - rndm->getUniform();
       lambda = -log(eta);
   }

   EGS_Vector x(the_stack->x[np],the_stack->y[np],the_stack->z[np]);
   EGS_Vector u(the_stack->u[np],the_stack->v[np],the_stack->w[np]);

   if( isc >= 0 ) {
       //
       // *** Particle headed for the detection screen
       //
       EGS_Float t = the_extra_stack->ddet[np];

       //
       // *** Get the particle trajectory intersections with the geometry
       //
       int ireg = the_stack->ir[np]-2;
       //int nsec = computeIntersections(ireg,x,u,gsections);
       int nsec = computeIntersections(ireg,x,u);
       //
       // *** Compute number of MFP's and possibly interaction site
       //
       EGS_Float tlast = 0; int imed = -1;
       EGS_Float gle = the_epcont->gle;
       EGS_Float gmfp=1e30, sigma = 0, cohfac = 1, cohfac_int = 1;
       EGS_Float rhor = 1;
       bool do_interaction = false;
       bool reached_screen = false;
       int jint = -1, imed_int = -1; double Lambda = 0;
       EGS_Vector xint; EGS_Float patt_int;
       //egsInformation("*** x=(%g,%g,%g) u=(%g,%g,%g)\n",x.x,x.y,x.z,u.x,u.y,u.z);
       for(int j=0; j<nsec; j++) {
           EGS_Float tnew = gsections[j].t;
           EGS_Float tstep = tnew - tlast;
           if( imed != gsections[j].imed ) {
               imed = gsections[j].imed;
               if( imed >= 0 ) {
                   gmfp = i_gmfp[imed].interpolateFast(gle);
                   if( the_xoptions->iraylr ) {
                       cohfac = i_cohe[imed].interpolateFast(gle);
                       gmfp *= cohfac;
                   }
               }
               else { gmfp=1e15; cohfac = 1; }
           }
           sigma = gsections[j].rhof/gmfp;
           EGS_Float this_lambda = tstep*sigma;
           if( !do_interaction ) {
               if( this_lambda < lambda ) lambda -= this_lambda;
               else if(imed >= 0 && sigma > 0){
                   do_interaction = true;
                   EGS_Float tt = lambda/sigma;
                   //LambdaInt += (tstep-tt)*sigma;
                   LambdaInt += (tstep-tt);
                   xint       = x + u*(tlast + tt);
                   cohfac_int = cohfac;
                   jint       = j; imed_int = gsections[jint].imed;
                   //
                   // *** The following is only needed if bound Compton is on.
                   //     In this case, a bound Compton interaction may get
                   //     rejected and the particle gets re-transported
                   //     scoring voxel and distance to screen must be set
                   //     properly
                   the_extra_stack->ddet[np] = t-tlast-tt;
                   if( reached_screen ) the_extra_stack->irdet[np] = -1;
               }
           }
           //else{LambdaInt += this_lambda;}
           else{LambdaInt += tstep;}

           if( !reached_screen && tnew >= t ) {
               reached_screen = true;
               Lambda += (t - tlast)*sigma;
               //LambdaInt += (t - tlast)*sigma;
               LambdaInt += (t - tlast);
           }

           if( reached_screen && do_interaction ) break;

           tlast = gsections[j].t; Lambda += this_lambda;
// egsInformation("med[%d] = %d t[%d] = %g cm, step= %g cm, lamda=%g \n",
//                   j,gsections[j].imed,j,gsections[j].t,tstep,this_lambda);

       }

       if( !reached_screen ) {
           //
           // *** Particle has not yet intersected the scoring plane
           //     => compute MFPs in medium surrounding the geometry
           //
           if( iair >= 0 ) {
               gmfp = i_gmfp[iair].interpolateFast(gle);
               if( the_xoptions->iraylr ) {
                   cohfac = i_cohe[iair].interpolateFast(gle);
                   gmfp *= cohfac;
               }
               Lambda += (t - gsections[nsec-1].t)/gmfp;
               //LambdaInt += (t - gsections[nsec-1].t)/gmfp;
               //LambdaInt += (t - gsections[nsec-1].t);
               //Surrounding MUST be always air or vacuum!!!
               //What if surrounding is tissue-like???
           }
       }
       //egsInformation("idet=%d lambda = %g\n",isc,Lambda);

       //
       // *** Score particle contribution to detected signal
       //
       if( Lambda < 80 ) {
           EGS_Float _muen = muen->interpolateFast(gle);
           EGS_Float exp_Lambda = exp(-Lambda);
           //exp_Lambda = exp(-Lambda);
           EGS_Float up = a*u;
           EGS_Float aup = fabs(up); if( aup < 0.08 ) aup = 0.08;aup_int=aup;
           double sc = the_stack->wt[np]/aup*exp_Lambda*_muen;
           if( !the_stack->latch[np] ) {
                patt_score_sum += sc*aup; ++patt_score_count; //<== original: <Ka>
                patt_score = patt_score_sum/patt_score_count;
           }


/*           if( sc*f_split/patt_score > 5 && the_stack->latch[np] )
               egsWarning("\nIn case %lld particle %d with E=%g wt=%g "
                       "latch=%d iphat=%d deposits %g K/Ka=%g\n",
                       current_case,np, the_stack->E[np],the_stack->wt[np],
                       the_stack->latch[np],the_extra_stack->iphat[np],
                       sc,sc*f_split/patt_score);*/
#ifdef DEBUG_WEIGHTS
           if( sc*aup > 5*patt_score && fabs(xcomp)<15 && fabs(ycomp)<15 ) {
               egsInformation("\nIn case %lld particle %d with E=%g wt=%g "
                       "latch=%d deposits %g\n",current_case,np,
                       the_stack->E[np],the_stack->wt[np],
                       the_stack->latch[np],sc);
               egsInformation("Particle originated in interaction %d\n",
                       the_extra_stack->inter[np]);
               EGS_Float gmfp = i_gmfp[patt_med].interpolateFast(gle);
               if( the_xoptions->iraylr )
                   gmfp *= i_cohe[patt_med].interpolateFast(gle);
               egsInformation("muen=%g muatt=%g aup=%g Lambda=%g\n",
                       _muen,1/gmfp,aup,Lambda);
               interactions.printInteractions();
           }
#endif
             if (the_stack->latch[np]){// scattered photon
                /* PDIS */
                //if (split_type ==PDIS){
//                if (patt_have){
//
//                    if( sc*f_split/patt_score > 5 ) {// discard too large contributions
//                      --the_stack->np; return;       // should we just not score instead?
//                    }

                   /* Histogram relative deviation from average contribution */
                   //if (hist)
                   //    hist->score(sc*f_split/patt_score);
//                }

                /* PDIS with corrector */
                if (c_att && the_extra_stack->katt[np] >= 0)
                   c_att->score(the_stack->ir[np]-2,sc,the_extra_stack->katt[np],the_stack->wt[np]);

                /* RDIS */
                if (splitter)
                   //splitter->score(the_stack->ir[np]-2,sc);
                   splitter->score(the_stack->ir[np]-2,sc,the_stack->wt[np]);
             }

           /*--------------------------------------------------------*/
           /*                 score total kerma                      */
           /*--------------------------------------------------------*/
           cker->score(isc,0,sc);
           /*--------------------------------------------------------*/
           /*           score primary kerma, only attenuation        */
           /*--------------------------------------------------------*/
           if( the_stack->latch[np] == 0 ) kermaA->score(isc,sc);
           else                            kermaS->score(isc,sc);
           /*--------------------------------------------------------*/
       }

       dpmfp = -1;

       if( do_interaction ) {


           if( max_latch >= 0 && the_stack->latch[np] >= max_latch ) {
               --the_stack->np; return;
           }

           //
           // *** The particle has interacted at xint. Simulate the interaction
           //

           //
           // *** First update the stack with position, region, etc.
           //

           the_stack->wt[np] *= wtr;
           the_stack->x[np] = xint.x;
           the_stack->y[np] = xint.y;
           the_stack->z[np] = xint.z;
           the_stack->dnear[np] = 0;
           the_stack->ir[np] = gsections[jint].ireg + 2;
           the_useful->medium = imed_int + 1;

           //********************************************************
           // *** Now reduce weight by the fraction that will
           //     be photo-absorbed and only do Compton or Rayleigh
           //********************************************************
           /* ---try
           EGS_Float gbr2 = i_gbr2[gsections[jint].imed].interpolateFast(gle);
           the_stack->wt[np] *= (1 - (1-gbr2)*cohfac);
           ---try */

           //****************************************************
           // *** Set splitting number for this interaction
           //****************************************************
           //
           // Splitting number proportional to Np or fsplit:
           //
           // FS   => splits by Np
           //
           // PDIS => splits by wt * Ka(j)/Katt * fsplit
           //
           //         Ka(j) is estimated contribution to air-kerma from
           //         current location BEFORE interaction attenuated by
           //         path through geometry along particle's direction
           //         using an average medium or a user-supplied medium
           //         if using an attenuation plane.
           //
           //         Katt is average contribution from all primaries to air-kerma.
           //
           // RDIS => splits by wt * C(j) * Np
           //
           //         C(j) is importance of current region.
           //
           // j corresponds to position/importance region of current interaction
           //
           //**********************************************************************
           int nspl = -1;
           //if (!patt_have)
           if (split_type !=PDIS) // FS or RDIS splitting scheme
              nspl = setSplitting(the_stack->ir[np]-2,
                                  the_extra_stack->iphat[np],
                                  the_stack->wt[np]);
           else{// PDIS splitting scheme
            //nspl = setSplitting(isc,the_stack->ir[np]-2,xint,u,the_stack->wt[np],gle);
            //nspl = setSplitting(the_stack->ir[np]-2,xint,u,the_stack->wt[np],gle);
              if (patt_have)
                nspl = setSplitting(xint,the_stack->wt[np],gle);
              else
              // nspl = setSplitting(LambdaInt,the_stack->ir[np]-2,aup_int,the_stack->wt[np],gle);
              {
                EGS_Float LambdaRest = gsections[nsec-1].t - LambdaInt;
                //EGS_Float LambdaRest = Lambda - LambdaInt;
                nspl = setSplitting(LambdaRest,the_stack->ir[np]-2,aup_int,the_stack->wt[np],gle);
                //nspl = setSplitting(Lambda,the_stack->ir[np]-2,aup_int,the_stack->wt[np],gle);
              }
                //nspl = setSplitting(exp_Lambda,the_stack->ir[np]-2,aup_int,the_stack->wt[np],gle);
                //nspl = setSplitting(gsections[nsec-1].t,the_stack->ir[np]-2,aup_int,the_stack->wt[np],gle);
           }

           if( !nspl ) { --the_stack->np; return;}
           if( nspl > 0 ) the_extra_stack->iphat[np] = nspl;
           else nspl = the_extra_stack->iphat[np];
           //****************************************************

           EGS_Float gbr2 = i_gbr2[imed_int].interpolateFast(gle);
           /*
              If photo-effect, just kill the photon
              (1-(1-gbr2)*cohfac) = (Ray+Comp+Pair)/Tot
           */
           EGS_Float r_adjust = 1-(1-gbr2)*cohfac_int;
           //*******************************
//           the_stack->wt[np] *= r_adjust;// <= VERY INEFFICIENT??? Not always!!!
           //*******************************
           if(rndm->getUniform()>r_adjust){--the_stack->np; return;}

#ifdef DEBUG_WEIGHTS
           Interaction inter(xint,u,the_stack->E[np],the_stack->wt[np],
                   the_stack->ir[np]-2,the_stack->latch[np],0,nspl,0);
#endif

           bool did_rayleigh = false; int itype;
           if( the_xoptions->iraylr ) {
               if( rndm->getUniform()*r_adjust < 1 - cohfac_int ) {
                   doMyRayleigh();
                   did_rayleigh = true;
                   itype = BeforeRayleigh;
               }
           }
           if( !did_rayleigh ) {
               itype = BeforeCompton;
               doMyCompton();
           }
#ifdef DEBUG_WEIGHTS
           inter.type = itype;
           interactions.addInteraction(inter);
#endif
           resetSplitting();
       }
       else --the_stack->np;

       //
       // *** Return to the photon transport routine. Because
       //     dpmfp is negative this will cause immediate return
       //     to shower().
       //
       return;
   }

   if( max_latch >= 0 && the_stack->latch[np] >= max_latch ) {
       --the_stack->np; return;
   }

   //
   // *** The particle is not moving towards the detection screen
   //     If the user has not requested delta transport
   //     (a.k.a. Woodcock transport or fictitious cross section method ),
   //     which is indicated by dt_medium < 0, just return.
   //     Otherwise transport the particle utilizing delta transport
   //

   the_stack->wt[np] *= wtr;
   dpmfp = lambda;


   /*  W A R N I N G !!!!!  */
   if( dt_medium < 0 ) return;// if delta transport off, returns to regular EGSnrc
                              // photon routine, which is not right if splitting is
                              // on. Should implement transport to interaction point
                              // right here.

   int ireg = the_stack->ir[np]-2, imed = geometry->medium(ireg);
   EGS_Float gle = the_epcont->gle;
   // The maximum cross section
   EGS_Float gmfp_max = i_gmfp[dt_medium].interpolateFast(gle);
   if( the_xoptions->iraylr )
       gmfp_max *= i_cohe[dt_medium].interpolateFast(gle);
   gmfp_max = gmfp_max/rhormax;

   bool do_interaction = false; EGS_Float cohfac; EGS_Float gmfp = -1;
   if( current_case == __debug_case ) egsWarning("Transporting non-scoring particle from (%g,%g,%g) %d\n",x.x,x.y,x.z,ireg);
   while(1) {
       if( imed < 0 || imed == iair ) {
           //
           // *** Don't do delta transport if the particle is in air or in
           //     vacuum. Instead transport until the particle ends up in
           //     another medium, exits the geometry, or interacts
           //
           if( current_case == __debug_case ) egsWarning("Air/vacuum %d\n",imed);
           while(1) {
               EGS_Float t;
               if( iair >= 0 && imed == iair ) {
                   if( gmfp < 0 ) {
                       gmfp = i_gmfp[iair].interpolateFast(gle);
                       if( the_xoptions->iraylr )
                           cohfac = i_cohe[iair].interpolateFast(gle);
                       else cohfac = 1;
                       gmfp *= i_cohe[iair].interpolateFast(gle);
                   }
                   t = lambda*gmfp;
               }
               else t = 1e30;
               int newmed; int inew = geometry->howfar(ireg,x,u,t,&newmed);
               if( current_case == __debug_case ) egsWarning("t=%g inew=%d\n",t,inew);
               if( inew < 0 ) {
                   //
                   // *** Particle exits the geometry.
                   //
                   dpmfp = -1; --the_stack->np; return;
               }
               x += u*t;
               if( inew == ireg ) {
                   //
                   // *** Particle interacts
                   //
                   do_interaction = true; break;
               }
               ireg = inew;
               if( iair >= 0 && imed == iair ) lambda -= t/gmfp;
               if( newmed != imed ) {
                   imed = newmed;
                   if( newmed >= 0 && newmed != iair ) break;
                   //
                   // *** i.e., we are not in air or vacuum => jump out of
                   //     loop to do delta transport
                   //
               }
           }
           if( do_interaction ) break;
       }
       EGS_Float t = lambda * gmfp_max; x += u*t;
       // still inside ?
       ireg = geometry->isWhere(x);
       if(current_case == __debug_case)
          egsWarning("delta transport: t=%g ireg=%d\n",t,ireg);
       if( ireg < 0 ) { dpmfp = -1; --the_stack->np; return; }
       // Huh? The following was missing (IK, May 21 '07)
       imed = geometry->medium(ireg);
       // actual cross section in current region
       if( imed >= 0 ) {
           gmfp = i_gmfp[imed].interpolateFast(gle);
           if( the_xoptions->iraylr ) {
               cohfac = i_cohe[imed].interpolateFast(gle);
               gmfp *= cohfac;
           } else cohfac = 1;
           EGS_Float rhor = 1;
           if( geometry->hasRhoScaling() )
              rhor = geometry->getRelativeRho(ireg);
           gmfp = rhor > 0 ? gmfp/rhor : 1e30;
           if( rndm->getUniform()*gmfp < gmfp_max ) {
               // real interaction
               do_interaction = true; break;
           }
       }
       else{gmfp = 1e30; cohfac = 1;}
       // resample MFP and repeat
       lambda = -log(1-rndm->getUniform());
   }
   // update particle position
   the_stack->x[np] = x.x;
   the_stack->y[np] = x.y;
   the_stack->z[np] = x.z;
   the_stack->dnear[np] = 0;
   the_stack->ir[np] = ireg+2;
   the_useful->medium = imed+1;

   if( do_interaction ) {
       if(current_case==__debug_case)
          egsWarning("Interaction: x=(%g,%g,%g) ireg=%d\n",x.x,x.y,x.z,u.x,u.y,u.z,ireg);
       dpmfp = -1;
       EGS_Float gbr2 = i_gbr2[imed].interpolateFast(gle);
       EGS_Float r_adjust = 1-(1-gbr2)*cohfac;
//       the_stack->wt[np] *= r_adjust;// <- VERY INEFFICIENT ??? Not always!!!
       if(rndm->getUniform()>r_adjust){--the_stack->np;return;}
       //=========================
       // Select splitting number:
       //=========================
       //
       // RR surviving scattered photons will have weight wt*Ns,
       // hence splitting number proportional to Ns. Use similar splitting
       // as for those aimed at detector.
       //
       // FS   => splits by Ns
       //
       // PDIS => splits by Ns * Ka(j)/Ka(i)
       //         Ka(j) is estimated contribution to air-kerma from
       //         current location attenuated by path through geometry
       //         along scoring or attenuation plane direction.
       //         Ka(i) is estimated contribution to air-kerma from
       //         previous location.
       //
       // RDIS => splits by Ns * Kave(j)/Kave(i)
       //         Kave(j) is average contribution to air-kerma from
       //         current importance region.
       //         Kave(i) is average contribution to air-kerma from
       //         previous importance region.
       //
       // i corresponds to position/importance region of previous interaction
       // j corresponds to position/importance region of current interaction
       //
       //**********************************************************************
       int nspl;
       if (split_type !=PDIS) // FS or RDIS
           nspl = setSplitting(the_stack->ir[np]-2,
                               the_extra_stack->iphat[np],
                               the_stack->wt[np]);
       else{
           if (patt_have)
            nspl = setSplitting(x,the_stack->wt[np],gle);
          else
            nspl = setSplitting(the_stack->ir[np]-2,x,a,the_stack->wt[np],gle);
          //nspl = setSplitting(the_stack->ir[np]-2,x,patt_a,the_stack->wt[np],gle);
       }

       if( !nspl ) { --the_stack->np; return; }
       if( nspl > 0 ) the_extra_stack->iphat[np] = nspl;
       else nspl = the_extra_stack->iphat[np];

#ifdef DEBUG_WEIGHTS
       Interaction inter(x,u,the_stack->E[np],the_stack->wt[np],
                     the_stack->ir[np]-2,the_stack->latch[np],0,nspl,1);
#endif
       bool did_rayleigh = false; int itype;
       if( the_xoptions->iraylr ) {
           if( rndm->getUniform()*r_adjust < 1 - cohfac ) {
               doMyRayleigh(); itype = BeforeRayleigh;
               did_rayleigh = true;
           }
       }
       if( !did_rayleigh ) {
           doMyCompton(); itype = BeforeCompton;
       }
#ifdef DEBUG_WEIGHTS
       inter.type = itype;
       interactions.addInteraction(inter);
#endif
       resetSplitting();
       return;
   }
   // if here, we are returning for normal transport
   // (should never happen)
   egsInformation("Huh? Ended up at end of selectPhotonMFP()\n");
   dpmfp = lambda;
}

/*! Start a new shower.  */
int EGS_CBCT::startNewShower() {
    int res = EGS_Application::startNewShower();
    if( res ) return res;
    if( current_case != last_case ) {
        cker->startNewCase(current_case);
        kermaS->setHistory(current_case);
        pselector->resetList();
    }
    return 0;
}

void EGS_CBCT::fastTransport(EGS_CBCT_Photon &p) {

    //
    // *** number of MFP
    //
    EGS_Float lambda, wtr = 1; bool check_interaction;
    if( !p.latch && p.isc >= 0 ) {
        check_interaction = true;
        if( mfptr_do ) {
            EGS_Float eta = 1/(1-rndm->getUniform()); EGS_Float aux = sqrt(eta);
            lambda = mfptr_lamo*(aux-1);
            wtr = mfptr_lamo*0.5*eta*aux*exp(-lambda);
        }
        else lambda = -log(1 - rndm->getUniform());
    }
    else { lambda = 1e30; check_interaction = false; }

    //
    // *** geometry intersections
    //
    int nsec = 0;
    while(1) {
        nsec = geometry->computeIntersections(p.ir,isize,p.x,p.u,gsections);
        if( nsec >= 0 ) break;
        // if here, need to increase size of gsections
        int nsize = 2*isize;
        if( nsize > 16384 ) {
            egsInformation("\nThere are more than %d intersections for"
                    " this geometry\n",isize);
            egsInformation("  are you sure this is OK?\n");
            egsInformation("  position: (%g,%g,%g)\n",p.x.x,p.x.y,p.x.z);
            egsInformation("  direction: (%g,%g,%g)\n",p.u.x,p.u.y,p.u.z);
            egsInformation("  region: %d\n",p.ir);
            egsFatal("Quiting now\n\n");
        }
        EGS_GeometryIntersections *tmp =
            new EGS_GeometryIntersections [nsize];
        for(int j=0; j<isize; j++) tmp[j] = gsections[j];
        delete [] gsections; gsections = tmp; isize = nsize;
    }

    //
    // *** Compute number of MFP to end of geometry and possibly interaction
    //     site
    //
    EGS_Float tlast = 0; int imed = -1; EGS_Float gle = p.gle;
    EGS_Float Lambda = 0, gmfp = 1e15; int jint;
    EGS_Vector xint; bool do_interaction = false;
    for(int j=0; j<nsec; j++) {
        EGS_Float tnew = gsections[j].t; EGS_Float tstep = tnew - tlast;
        if( imed != gsections[j].imed ) {
            imed = gsections[j].imed;
            if( imed >= 0 ) gmfp = i_gmfp[imed].interpolateFast(gle);
        }
        if( imed >= 0 ) {
            EGS_Float sigma = gsections[j].rhof/gmfp;
            EGS_Float this_lambda = tstep*sigma;
            if( check_interaction ) {
                if( Lambda + this_lambda >= lambda ) {
                    check_interaction = false; do_interaction = true;
                    EGS_Float tt = lambda/sigma;
                    xint = p.x + p.u*(tlast + tt); jint = j;
                }
            }
            Lambda += this_lambda;
        }
        tlast = tnew;
    }

    if( Lambda < 80 ) {
        EGS_Float mu_en = muen->interpolateFast(gle);
        EGS_Float up = a*p.u;
        EGS_Float aup = fabs(up); if( aup < 0.08 ) aup = 0.08;
        double sc = p.wt*exp(-Lambda)/aup*mu_en;
        if( sc > 1e-30 ) {
            if( !p.latch ) {
                patt_score_sum += sc*aup; ++patt_score_count;
                patt_score = patt_score_sum/patt_score_count;
            }
            cker->score(p.isc,0,sc);
            if( !p.latch ) kermaA->score(p.isc,sc);
        }
    }

    if( do_interaction ) {
        imed = gsections[jint].imed;
        if( rndm->getUniform() < i_gbr2[imed].interpolateFast(gle) ) {
            p.x = xint; p.wt *= wtr; p.ir = gsections[jint].ireg;
            doKleinNishina(p);
        }
    }

}

void EGS_CBCT::doKleinNishina(EGS_CBCT_Photon &p) {

    EGS_Float Ko = p.E/the_useful->rm;
    EGS_Float broi = 1+2*Ko, Ko2 = Ko*Ko;
    EGS_Float alpha1_t = log(broi);
    EGS_Float eps1_t = 1/broi, eps2_t = 1;
    EGS_Float w2 = alpha1_t*(Ko2-2*Ko-2)+
          (eps2_t-eps1_t)*(1./eps1_t/eps2_t + broi + Ko2*(eps1_t+eps2_t)/2);
    EGS_Float sigt = w2/(Ko*Ko2);
}

#ifdef BUILD_APP_LIB
APP_LIB(EGS_CBCT);
#else
APP_MAIN(EGS_CBCT);
#endif
