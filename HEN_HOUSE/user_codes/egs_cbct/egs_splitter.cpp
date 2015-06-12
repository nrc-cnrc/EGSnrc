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
#  Implementation of the EGS_Splitter class.
#
#  Estimates importances C_i based on the relative contribution to the signal
#  from user defined importance regions.
#
###############################################################################
*/


#include <iostream>
#include <fstream>
#include <cmath>

#include "egs_splitter.h"
#include "egs_input.h"
#include "egs_functions.h"

using namespace std;

EGS_Splitter::EGS_Splitter(EGS_Input* inp, const int& n_p,
                           //const int& p_x,
                           const int& v_x,
                           const int& v_y,
                           const int& v_z):
warming(true),
//px(p_x),Px(256),
vx(v_x),vy(v_y),vz(v_z),
Vx(16), Vy(16), Vz(16), latch(1),
Nmin(10), Nmax(1000), Np(n_p), Nv_score(0),Nscore(0), Nt(0)
{

    /* get splitter resolution */
    int err0 = 0;
    //****************
    // Not in use!!!!*
    //****************
    //vector<int> screen;
    //err0 = inp->getInput("splitter resolution",screen);
    //if( err0 || screen.size() != 1 ) egsFatal(
    //   "\n\n***  Wrong/missing 'splitter resolution' input\n"
    //   "    This is a fatal error\n\n");
    //Px = screen[0];

    /* get type of signal to compute importances */
    vector<string> allowed_types;
    allowed_types.push_back("attenuated");
    allowed_types.push_back("scattered");
    allowed_types.push_back("scattered average");
    allowed_types.push_back("scattered total");
    int itype = inp->getInput("signal type",allowed_types,1);
    if( itype == 0 ) latch=0;// set to 1 by default

    /* get splitter geometry */
    err0 = 0; vector<int> phantom;
    err0 = inp->getInput("splitter geometry",phantom);
    if( err0 || (phantom.size() != 1 && phantom.size() != 3) )
      egsFatal("\n\n***  Wrong/missing 'splitter geometry' input\n"
               "    This is a fatal error\n\n");
    if( phantom.size() == 1){Vx=phantom[0];Vy=Vx;Vz=Vx;}
    if( phantom.size() == 3){Vx=phantom[0];Vy=phantom[1];Vz=phantom[2];}
    if (vx<Vx){
       egsFatal("\n********\n"
                "Splitter geometry (Nx=%d) can't have more regions"
                " than actual geometry (nx=%d)!!! Aborting ....\n",
                Vx,vx);
    }
    if (vy<Vy){
       egsFatal("\n********\n"
                "Splitter geometry (Ny=%d) can't have more regions"
                " than actual geometry (ny=%d)!!! Aborting ....\n",
                Vy,vy);
    }
    if (vz<Vz){
       egsFatal("\n********\n"
                "Splitter geometry (Nz=%d) can't have more regions"
                " than actual geometry (nz=%d)!!! Aborting ....\n",
                Vz,vz);
    }
    if (vx%Vx){
       egsFatal("\n********\n"
                "# of regions in splitter geometry (Nx=%d) must be "
                "a factor of # of regions in actual geometry (nx=%d)!!!"
                "\n Aborting ....\n********\n",
                Vx,vx);
    }
    if (vy%Vy){
       egsFatal("\n********\n"
                "# of regions in splitter geometry (Ny=%d) must be "
                "a factor of # of regions in actual geometry (ny=%d)!!!"
                "\n Aborting ....\n********\n",
                Vy,vy);
    }
    if (vz%Vz){
       egsFatal("\n********\n"
                "# of regions in splitter geometry (Nz=%d) must be "
                "a factor of # of regions in actual geometry (nz=%d)!!!"
                "\n Aborting ....\n********\n",
                Vz,vz);
    }
    Nv = Vx*Vy*Vz;
    K = new EGS_Float[Nv]; C = new EGS_Float[Nv];
    Nscore = new EGS_Float[Nv]; voxel = new int[Nv];
    for (int i=0; i<Nv;i++){K[i] = 0.0;C[i]=1;Nscore[i]=0;voxel[i]=-1;}
    //for (int i=0; i<Nv;i++){K[i] = 0.0;C[i]=Np;Nscore[i]=0;voxel[i]=-1;}
    vxy = vx*vy; Vxy = Vx*Vy; //mpx = px/Px;// Not in use!!!!
    mvx = vx/Vx, mvy = vy/Vy, mvz = vz/Vz;

    /* get maximum and minimum splitting */
    err0 = 0; int nspl = 0;
    err0 = inp->getInput("minimum splitting",nspl);
    if( !err0 && nspl>0) Nmin = nspl;
    err0 = 0; nspl = 0;
    err0 = inp->getInput("maximum splitting",nspl);
    if( !err0 && nspl>0) Nmax = nspl;
    if (Nmax <= Nmin)
      egsFatal("****** Splitter object ERORR: Maximum splitting smaller than the minimum splitting number!!!\n");
    /* get maximum and minimum imortances */
    Cmin = EGS_Float(Nmin)/EGS_Float(Np);
    Cmax = EGS_Float(Nmax)/EGS_Float(Np);

    /* Make a list assigning a splitting voxel to
       each geometrical region */
    int Ngeom = vx*vy*vz; index = new int[Ngeom];
    for (int j=0; j<Ngeom; j++){index[j] = findVoxel(j);}

}

EGS_Splitter::~EGS_Splitter(){
  delete [] K; delete [] C; delete [] Nscore;
}

void EGS_Splitter::describeIt(){

   egsInformation("================\n"
                  "Splitter Info:\n"
                  "================\n");
   egsInformation("%d X %d X %d splitter phantom with %d regions\n",
                  Vx,Vy,Vz,Nv);
   egsInformation("%d X %d X %d real phantom\n",
                  vx,vy,vz);
   egsInformation("phantom multiplicities: mvx = %d mvy = %d mvz = %d\n",
                  mvx,mvy,mvz);
   //egsInformation("=> screen multiplicities: mpx = %d\n", mpx);
   egsInformation("Nmin = %d Nmax = %d Np = %d\n", Nmin, Nmax, Np);
   string signal = "Using ";
   if (latch==0) signal += "attenuated";
   else signal += "scattered";
   egsInformation("%s signal to compute importances\n",signal.c_str());
}

// EGS_Float EGS_Splitter::averageSplitting(){
//   EGS_Float Nav=0;
//   for (int i = 0; i < Nv_score; i++){ Nav += C[voxel[i]];
// /*  if(!((i+1)%2))
// egsInformation("{v[%d] = %d, N=%g}\n",i,voxel[i],C[voxel[i]]);
//   else
// egsInformation("{v[%d] = %d, N=%g}",i,voxel[i],C[voxel[i]]);*/
//   }
//   return Np*Nav/EGS_Float(Nv_score);
// }

/**************************************************
   Calculates actual average splitting number Nave
   for a given importance map Cj:

   Nave = SUM[Cj*Nj]/SUM[Nj]*Np = <C>*Np

   where
   Cj => importance in region j
   Nj => # of particles scoring from region j
   Np => user-requested average splitting
*************************************************/
EGS_Float EGS_Splitter::averageSplitting(){
  EGS_Float Nav=0;
  for (int i = 0; i < Nv_score; i++){
      int j = voxel[i];
      Nav += C[j]*Nscore[j];
  }
  return EGS_Float(Np)*Nav/Nt;
}

void EGS_Splitter::printStatus(){
   egsInformation("================\n"
                  "Splitter Status:\n");
   egsInformation("<Nsplit> = %g ",averageSplitting());
   if (latch)
    egsInformation("Kt = %g ",Kt);
   else
    egsInformation("<Katt> = %g ",Kt);
   egsInformation("voxels scoring: %d",
                   Nv_score);
   egsInformation("\n================\n");
}

void EGS_Splitter::printImportances(const string& fname){

   ofstream fout(fname.c_str());
   fout << "Geometrical importances\n"
        << "=======================\n\n"
        << " ix     iy    iz     importance     Ni        Ki        <Ki> \n"
        << "------------------------------------------------------------\n";
   char buf[8192]; string str="";

   for (int ix=0; ix<Vx; ix++){
     for(int iy=0; iy<Vy; iy++){
       for(int iz=0; iz<Vz; iz++){
         int ir = ix + iy*Vx + iz*Vxy;
         if(Nscore[ir])
           sprintf(buf," %-6d %-6d  %-6d %-10.2f %-10d %-13.3g %-13.3g\n",
                ix,iy,iz,C[ir],Nscore[ir],K[ir],K[ir]/Nscore[ir]);
         else
           sprintf(buf," %-6d %-6d  %-6d %-10.2f %-10d %-13.3g %-13.3g\n",
                ix,iy,iz,C[ir],Nscore[ir],K[ir],Nscore[ir]);
         str += buf;
       }
     }
   }
/*   for (int ir=0; ir<Nv; ir++){
        int iz = ir/Vxy, imod = ir - iz*Vxy,
            iy = imod/Vx,
            ix = imod - iy*Vx;
        sprintf(buf," %-10d %-6d %-6d %-6d %-10.2f\n",ir,iz,iy,ix,C[ir]);
        str += buf;
   }*/
   fout << str.c_str();
}

// EGS_Float* EGS_Splitter::getImportances(int& nreg){
//    nreg = Nv;
//    EGS_Float* c = new EGS_Float[Nv];
//    int j = 0;
//    for(int iz=0; iz<Vz; iz++){
//      for(int iy=0; iy<Vy; iy++){
//        for (int ix=0; ix<Vx; ix++){
//          int ir = ix + iy*Vx + iz*Vxy;
//          c[j++]=C[ir];
//        }
//      }
//    }
//    return c;
// }





