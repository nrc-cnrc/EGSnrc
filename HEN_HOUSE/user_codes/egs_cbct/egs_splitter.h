/*
###############################################################################
#
#  EGSnrc egs++ egs_cbct application splitter class
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
#  Author:          Ernesto Mainegra-Hing, 2008
#
#  Contributors:
#
###############################################################################
#
#  Definition of the EGS_Splitter class.
#
#  Estimates importances C_i based on the relative contribution to the signal
#  from user defined importance regions.
#
###############################################################################
*/


#ifndef EGS_SPLITTER__
#define EGS_SPLITTER__

#include "egs_functions.h"
#include "egs_base_geometry.h"

class EGS_Input;

class  EGS_Splitter {

protected:

    EGS_Float  *K; // signal on detector from a given voxel
    EGS_Float   Kt; // total signal on detector
    EGS_Float  *C;   // splitting numbers
    EGS_Float Cmin, Cmax;// min. and max. importances

    //int Px; // splitter detector resolution
    //int px; // actual detector resolution

    int Vx, Vy, Vz; // splitter phantom resolution
    int vx, vy, vz; // transport phantom resolution
    int vxy, Vxy;
    int mvx,mvy,mvz; // voxel multiplicities
    //    mpx;     // pixel multiplicities


    int  Nv,    // number of voxels
         Np,  // average splitting (set by user)
         Nmin,  // minimum splitting
         Nmax;  // maximum splitting
    EGS_Float *Nscore;// array of number of particles scoring from an imp. voxel
    EGS_Float Nt;
    int  Nv_score,// number of scoring imp. voxels
        *voxel,   //array containing indices of scoring imp. voxels
        *index;   // list linking each geom. region with an imp. region
    int  latch;   // if 0, attenuated signal, else scatter
    bool warming;
public:

    EGS_Splitter(EGS_Input* inp, const int& n_p,
                           //const int& p_x,
                           const int& v_x,
                           const int& v_y,
                           const int& v_z);
    ~EGS_Splitter();

    bool isWarming(){return warming;};
    void stopWarming(){warming=false;};

    void describeIt();
    EGS_Float averageSplitting();
    void printStatus();
    void printImportances(const string& fname);
    int scoringLatch(){return latch;};
    void setLatch(const int& _latch){latch=_latch;};
    int getGridNx(){return Vx;};
    int getGridNy(){return Vy;};
    int getGridNz(){return Vz;};

    EGS_Float getImportance(const int& ireg){
       return ireg >= 0 ? C[index[ireg]] : Cmin;
    };

    float fetchImportance(const int& imp_reg){
       return imp_reg >= 0 ? C[imp_reg] : Cmin;
    };

    /* Calculates splitting number based on geometrical
       importance. The expression used is:

       C_i = (K_i/N_i) / (K_t/N_v) * Np

       where K_t = Sum(K_i/N_i)

    */
//     void updateSplitting()
//     {
//
//       Kt = 0;//reset this
//
//       /* Compute sum of averages */
//       for( int i=0; i<Nv; i++){
//          C[i] = 0;// resetting this
//          if (!Nscore[i]) continue;
//          C[i] = K[i]/Nscore[i];
//          Kt += C[i];
//       }
//       /* compute constant quantity first*/
//       //EGS_Float aux = Kt > 0 ? EGS_Float(Nv_score*Np)/Kt:0;
//       EGS_Float aux = Kt > 0 ? EGS_Float(Nv_score)/Kt:0;
//       for( int i=0; i<Nv; i++){
//          C[i] *= aux;
//          if (C[i]<Cmin) C[i]=Cmin;
//          if (C[i]>Cmax) C[i]=Cmax;
//       }
//     };

    /**********************************************
       Calculates geometrical importances C_i. The
       expression used is:

       C_i  = <K_i>/<Kt>

       where

    <K_i> = K_i/N_score_i         => ave. contr. from region i

    <Kt>  = Sum(K_i/)/Sum(N_score)=> ave. contr. from all regions

    ***********************************************/
    void updateSplitting()
    {

      Kt = 0; Nt = 0;//reset these

      /* Compute sum of averages */
      for( int i=0; i<Nv; i++){
         C[i] = 0;// resetting this
         if (!Nscore[i]) continue;
         /***************************/
         /* C_i proportional to <K_i> */
         /***************************/
         C[i] = K[i]/Nscore[i];
         Kt  += K[i]; Nt += Nscore[i];
         /***************************/
         /* C_i proportional to K_i */
         /***************************/
         //C[i] = K[i];
         //Kt  += K[i]*Nscore[i]; Nt += Nscore[i];
      }
      /* compute constant quantity first*/
      EGS_Float aux = Kt > 0 ? Nt/Kt:0;
      for( int i=0; i<Nv; i++){
         C[i] *= aux;
         if (C[i]<Cmin) C[i]=Cmin;
         if (C[i]>Cmax) C[i]=Cmax;
      }
    };

    void updateSplitting(const EGS_Float& KattAve)
    {

      Kt = KattAve;
      /* Compute sum of averages */
      for( int i=0; i<Nv; i++){
         C[i] = 0;// resetting this
         if (!Nscore[i]) continue;
         C[i] = K[i]/Nscore[i];
      }
      /* compute constant quantity first*/
      EGS_Float aux = KattAve > 0 ? 1.0/KattAve:0;
      for( int i=0; i<Nv; i++){
         C[i] *= aux;
         if (C[i]<Cmin) C[i]=Cmin;
         if (C[i]>Cmax) C[i]=Cmax;
      }
    };

    EGS_Float minImportance(){return Cmin;};
    EGS_Float maxImportance(){return Cmax;};
    int minSplitting(){return Nmin;};
    int maxSplitting(){return Nmax;};

    void score(const int& ireg, const EGS_Float& sc)
    {
       if (ireg>=0){
          int i = index[ireg];
          if (!Nscore[i])
             voxel[Nv_score++]=i; // i hasn't scored before
          K[i] +=sc;Nscore[i]++;
       }
    };

    void score(const int& ireg, const EGS_Float& sc, const EGS_Float& wt)
    {
       if (ireg>=0){
          int i = index[ireg];
          if (!Nscore[i])
             voxel[Nv_score++]=i; // i hasn't scored before
          K[i] +=sc; Nscore[i]+=wt;
       }
    };

    int findVoxel(const int& ireg)
    {
      /* voxel indices in transport geometry */
      int iz = ireg/vxy, imod = ireg - iz*vxy,
          iy = imod/vx,
          ix = imod - iy*vx;
      /* voxel indices in splitter geometry */
      int i_x = ix/mvx,
          i_y = iy/mvy,
          i_z = iz/mvz;
      return i_x+i_y*Vx+i_z*Vxy;
    }

    //int findPixel(const int& idet)
    //{
    //      /* pixel indices in actual screen */
    //  int iy = idet/px,
    //      ix = idet - iy*px;
    //  /* pixel indices in splitter screen */
    //  int i_x = ix/mpx,
    //      i_y = iy/mpx;
    //  return i_x+i_y*Px;
    //}
};
#endif
