/*
###############################################################################
#
#  EGSnrc egs++ egs_cbct application corrector class headers
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
#  Author:          Ernesto Mainegra-Hing, 2010
#
#  Contributors:
#
###############################################################################
#
#  Definition of the EGS_Corrector class.
#
#  Estimates corrections in a given region based on the ratio of the actual
#  contribution to the signal to the signal obtained from ray-tracing before
#  the interaction.
#
###############################################################################
*/


#ifndef EGS_CORRECTOR__
#define EGS_CORRECTOR__

#include "egs_functions.h"
#include "egs_base_geometry.h"

class EGS_Input;

class  EGS_Corrector {

    EGS_Float C, Cx,//Normalization ensuring same average splitting
              KaTot,//Total estimated signal
              KtTot,//Total true signal
              KTotRel,//Total ratio SUM[Kt[i]/Ka[i]]
              NTot, //Total number of scoring particles
              fsplit,// splitting factor
              KaPrim;// Primary Kerma attenuated through phantom
    EGS_Float  *Kt; // true signal on detector from a given voxel
    EGS_Float  *Ka; // estimated signal on detector from a given voxel
    EGS_Float  *X;  // corrections
    EGS_Float Xmin, Xmax; // minimum and maximum corrections

    int Vx, Vy, Vz; // corrector phantom resolution
    int vx, vy, vz; // transport phantom resolution
    int vxy, Vxy;
    int mvx,mvy,mvz;// voxel multiplicities
    int *mv,        // adaptive grid multiplicities
        *mv_used, mv_used_size,
         mv_current,// current grid multiplicity
         mv_size,   // adaptive grid multiplicity array size
         step,      // update grid every step times
         updates;   // update number

    int  Nv,    // number of voxels
         Ngeom;
    EGS_I64 *Nscore;// array of number of scores from an imp. region
    int     *index; // list linking geom. regions with imp. regions
    bool warming,
         use_adaptive_grid;
public:

    EGS_Corrector(EGS_Input* inp,const int& p_x,
                                 const int& v_y,
                                 const int& v_z,
                           const EGS_Float& fsp);
    ~EGS_Corrector();

    bool isWarming(){return warming;};
    void stopWarming(){warming=false;};

    void describeIt();
    void printStatus();
    void printCorrections(const string& fname);
    int  getGridNx(){return Vx;};
    int  getGridNy(){return Vy;};
    int  getGridNz(){return Vz;};
    void getGrids(){};
    EGS_Float currentC(){return KtTot!=0 ? KaTot/KtTot:1;};

    EGS_Float getCorrection(const int& ireg){
       return ireg >= 0 ? X[index[ireg]] : 1;
    };

    float fetchCorrection(const int& corr_reg){
       return corr_reg >= 0 ? X[corr_reg] : 1;
    };

    bool updatedGrid()
    {
     if (mv_current >= mv_size-1 ||
        !use_adaptive_grid       ||
        updates%step)
        return false;
egsInformation("\nUpdating adaptive grid from %dX%dX%d ",Vx,Vx,Vx);
     /* find next multiplicity which must be a
        multiple of the previous one          */
/*     bool found_mv = false; int mv_tmp = mv_current;
     do{
       mv_current++; int mnext=mv[mv_current], mnow=mv[mv_tmp];
       if (!(mnow%mnext)){found_mv=true;break;}
     } while(mv_current<mv_size-1);
     if (!found_mv) return false;*/
     mv_current++;

     mv_used[mv_used_size++] = mv[mv_current];
     //int igrid = mv_current >= mv_size ? mv_size-1:mv_current;
     /* Store quantities to temporary arrays */
     EGS_Float Kt_t[Nv], Ka_t[Nv]; EGS_I64 Nscore_t[Nv]; int Vx_t=Vx;
     for (int i=0; i<Nv;i++){
        Kt_t[i] = Kt[i]; Ka_t[i] = Ka[i]; Nscore_t[i]=Nscore[i];
     }
     /* Reset and resize all scoring arrays */
     delete [] Kt; delete [] Ka; delete [] X; delete [] Nscore;
     mvx = mv[mv_current]; mvy = mvx; mvz= mvy;
     Vx = vx/mvx; Vy = vy/mvy; Vz = vz/mvz; Nv = Vx*Vy*Vz;
     Kt = new EGS_Float[Nv]; Ka = new EGS_Float[Nv];
     X  = new EGS_Float[Nv]; Nscore = new EGS_I64[Nv];
     /* Reset link between geometric and adaptive grid */
     for (int j=0; j<Ngeom; j++){index[j] = findVoxel(j);}
     /* Assign values to the new grid */
     int V1=Vx, V2=Vx_t;// V1 finer grid, V2 coarser grid
     C = KtTot!=0 ? KaTot/KtTot:1;
     for (int i=0; i<Nv;i++){
         int k = matchVoxel(i,V1,V2);
         Kt[i] = Kt_t[k]; Ka[i] = Ka_t[k];
         Nscore[i]=Nscore_t[k];
         X[i] = Nscore[i]>0 ? Kt[i]/Ka[i]*C:1;
         if (X[i]<Xmin) X[i]=Xmin;
         if (X[i]>Xmax) X[i]=Xmax;
     }
egsInformation("to %dX%dX%d\n",Vx,Vx,Vx);
     return true;
    }

    /* Calculates corrections as the ratio
                  Kt/Ka*C
       where
                  C = KaTot/KtTot
    */
    void updateCorrections()
    {
      updates++;
      if (updatedGrid()) return;
      /* Compute corrections */
      C = KtTot!=0 ? KaTot/KtTot:1;
      for( int i=0; i<Nv; i++){
         X[i] = 1;// resetting this
         if (!Nscore[i]) continue;
         X[i] = Kt[i]/Ka[i]*C;
         if (X[i]<Xmin) X[i]=Xmin;
         if (X[i]>Xmax) X[i]=Xmax;
      }
    };
    /* Calculates corrections as the ratio
                  Kt/Ka*C
       where
                  C = KaTot/KtTot
    */
    void updateCorrections(const EGS_Float& KaP)
    {
      updates++;
      if (updatedGrid()) return;
      /* Compute corrections */
      C = KtTot!=0 ? KaTot/KtTot:1;
      KaPrim = KaP; Cx = KtTot!=0 ? NTot*KaP/KtTot: 1;
      for( int i=0; i<Nv; i++){
         X[i] = 1;// resetting this
         if (!Nscore[i]) continue;
         X[i] = Kt[i]/Ka[i]*C;
         //X[i] = Kt[i]/Ka[i]*Cx;
         if (X[i]<Xmin) X[i]=Xmin;
         if (X[i]>Xmax) X[i]=Xmax;
      }
    };

    void score(const int& ireg,
               const EGS_Float& sc,
               const EGS_Float& sa,
               const EGS_Float& _wt)
    {
       if (ireg>=0){
          int i = index[ireg]; NTot += _wt;
          Kt[i] +=sc; Ka[i] +=sa; Nscore[i]++;
          KtTot +=sc; KaTot +=sa; KTotRel += sc/sa;
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

    /*
        Matches region ireg in grid 1 with one in grid 2
        Equidistant grid in all directions assumed.
        Multiplicity M means grid2 contains M voxels
        from grid1.
        Rounding done to allow for non-integer multiplicities.
    */
    int matchVoxel(const int& ireg, const int& V1, const int& V2)
    {
      int V1xy = V1*V1, V2xy = V2*V2; double M = double(V1)/double(V2);
      /* voxel indices in geometry 1 */
      int iz = ireg/V1xy, imod = ireg - iz*V1xy,
          iy = imod/V1,
          ix = imod - iy*V1;
      /* voxel indices in geometry 2*/
      int i_x = round(double(ix)/M),
          i_y = round(double(iy)/M),
          i_z = round(double(iz)/M);
      return i_x+i_y*V2+i_z*V2xy;
    }

    int round(double x)
    {
       return int(x > 0.0 ? x + 0.5 : x - 0.5);
    }

};
#endif
