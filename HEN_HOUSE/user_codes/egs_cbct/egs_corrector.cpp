/*
###############################################################################
#
#  EGSnrc egs++ egs_cbct application corrector class
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
#  Implementation of the EGS_Corrector class.
#
#  Estimates corrections in a given region based on the ratio of the actual
#  contribution to the signal to the signal obtained from ray-tracing before
#  the interaction.
#
###############################################################################
*/


#include <iostream>
#include <fstream>
#include <cmath>

#include "egs_corrector.h"
#include "egs_input.h"
#include "egs_functions.h"

using namespace std;

EGS_Corrector::EGS_Corrector(EGS_Input* inp,
                             const int& v_x,
                             const int& v_y,
                             const int& v_z,
                             const EGS_Float& fsp):
warming(true), use_adaptive_grid(false),
vx(v_x),vy(v_y),vz(v_z), step(1), updates(0),
Vx(16), Vy(16), Vz(16),mv_size(0), mv_used_size(0), mv_current(0),
Nscore(0), Ngeom(0), Xmin(0.0), Xmax(100), C(1),KaTot(0),KtTot(0),KTotRel(0),
fsplit(fsp), KaPrim(1), Cx(1), NTot(0)
{
    /* get corrector geometry */
    int err0 = 0; vector<int> phantom;
    err0 = inp->getInput("corrector geometry",phantom);
    if( err0 || (phantom.size() != 1 && phantom.size() != 3) )
      egsFatal("\n\n***  Wrong/missing 'corrector geometry' input\n"
               "    This is a fatal error\n\n");
    if( phantom.size() == 1){Vx=phantom[0];Vy=Vx;Vz=Vx;}
    if( phantom.size() == 3){Vx=phantom[0];Vy=phantom[1];Vz=phantom[2];}
    if (vx<Vx){
       egsFatal("\n********\n"
                "Corrector geometry (Nx=%d) can't have more regions"
                " than actual geometry (nx=%d)!!! Aborting ....\n",
                Vx,vx);
    }
    if (vy<Vy){
       egsFatal("\n********\n"
                "Corrector geometry (Ny=%d) can't have more regions"
                " than actual geometry (ny=%d)!!! Aborting ....\n",
                Vy,vy);
    }
    if (vz<Vz){
       egsFatal("\n********\n"
                "Corrector geometry (Nz=%d) can't have more regions"
                " than actual geometry (nz=%d)!!! Aborting ....\n",
                Vz,vz);
    }
    if (vx%Vx){
       egsFatal("\n********\n"
                "# of regions in corrector geometry (Nx=%d) must be "
                "a factor of # of regions in actual geometry (nx=%d)!!!"
                "\n Aborting ....\n********\n",
                Vx,vx);
    }
    if (vy%Vy){
       egsFatal("\n********\n"
                "# of regions in corrector geometry (Ny=%d) must be "
                "a factor of # of regions in actual geometry (ny=%d)!!!"
                "\n Aborting ....\n********\n",
                Vy,vy);
    }
    if (vz%Vz){
       egsFatal("\n********\n"
                "# of regions in corrector geometry (Nz=%d) must be "
                "a factor of # of regions in actual geometry (nz=%d)!!!"
                "\n Aborting ....\n********\n",
                Vz,vz);
    }
    err0 = 0; EGS_Float x_min = Xmin, x_max = Xmax; int _step = 1;
    err0 = inp->getInput("minimum correction",x_min);
    if( !err0 ) Xmin = x_min;
    err0 = 0;err0 = inp->getInput("maximum correction",x_max);
    if( !err0 ) Xmax = x_max;
    err0 = 0;err0 = inp->getInput("update step",_step);
    if( !err0 ) step = _step; int Vmax = vx, _Vmax;
    err0 = 0;err0 = inp->getInput("maximum resolution",_Vmax);
    if( !err0 ) Vmax = _Vmax;

    vector<string> adaptive_grid_options;
    adaptive_grid_options.push_back("no");
    adaptive_grid_options.push_back("yes");
    int iadap = inp->getInput("adaptive grid",adaptive_grid_options,0);
    if (iadap!=0) use_adaptive_grid = true; // false by default

    Nv = Vx*Vy*Vz;
    Kt = new EGS_Float[Nv]; Ka = new EGS_Float[Nv]; X = new EGS_Float[Nv];
    Nscore = new EGS_I64[Nv];
    for (int i=0; i<Nv;i++){
         Kt[i] = 0.0; Ka[i] = 0.0;
         X[i] = 1; Nscore[i]=0;
    }
    vxy = vx*vy; Vxy = Vx*Vy;
    mvx = vx/Vx, mvy = vy/Vy, mvz = vz/Vz;

    /* Make a list assigning a correction voxel to
       each geometrical region */
    Ngeom = vx*vy*vz; index = new int[Ngeom];
    for (int j=0; j<Ngeom; j++){index[j] = findVoxel(j);}

   /* Make a list of possible grid resolutions */
   int Vtmp[Vmax-Vx+1];// trying all possible resolutions
   for (int k=Vx;k<=Vmax;k++){
     if (!(vx%k)) Vtmp[mv_size++] = vx/k;
   }
   mv = new int[mv_size]; mv_used = new int[mv_size];
   for (int l=0;l<mv_size;l++) mv[l]=Vtmp[l];
   mv_used[mv_used_size++] = mvx;
}

EGS_Corrector::~EGS_Corrector(){
  delete [] Kt; delete [] Ka; delete [] X; delete [] Nscore;
  if (mv_size){ delete [] mv; delete [] mv_used;}
}

void EGS_Corrector::describeIt(){

   egsInformation("\n================\n"
                  "Corrector Info:\n"
                  "================\n");
   egsInformation("%d X %d X %d corrector phantom with %d regions\n",
                  Vx,Vy,Vz,Nv);
   egsInformation("%d X %d X %d real phantom\n",
                  vx,vy,vz);
   egsInformation("phantom multiplicities: mvx = %d mvy = %d mvz = %d\n",
                  mvx,mvy,mvz);
   egsInformation("minimum correction allowed Xmin = %g\n",Xmin);
   egsInformation("maximum correction allowed Xmax = %g\n",Xmax);
   if (mv_size&&use_adaptive_grid){
    egsInformation("Number of adaptive grids: %d\n",mv_size);
    egsInformation("Adaptive grid multiplicities: ");
    for(int i=0;i<mv_size;i++) egsInformation("%d, ",mv[i]);
    egsInformation("\nAdaptive grid resolutions: ");
    for(int i=0;i<mv_size;i++) egsInformation("%d, ",vx/mv[i]);
    egsInformation("\n-> Changing grid every %d updates\n",step);
    egsInformation("=================================\n");
   }
}

void EGS_Corrector::printStatus(){
 egsInformation("\n=====================\n"
                  "Adaptive grid status:"
                "\n=====================\n");
 if (mv_used_size){
    for (int j=0;j<mv_used_size;j++)
        egsInformation("grid # %d : %d X %d X %d\n",
                       j,vx/mv_used[j],vy/mv_used[j],vz/mv_used[j]);
 }
 EGS_I64 Nscores = 0;
 for (int i=0; i<Nv;i++){Nscores +=Nscore[i];}
 egsInformation("-> fsplit = %g C = %g Cx = %g KaPrim = %g\n",fsplit, C, Cx, KaPrim);
 egsInformation("-> KaTot= %g KtTot = %g <KaTot>= %g <KtTot> = %g Nscores = %d \n",KaTot,KtTot,KaTot/Nscores,KtTot/Nscores,Nscores);
 egsInformation("-> <Nsplit>apriori    : %g \n",KaTot/Nscores/KaPrim*fsplit);
 egsInformation("-> <Nsplit>aposteriori: %g \n",KtTot/Nscores/KaPrim*fsplit);
 egsInformation("-> <sc>*fsplit/<Katt>    : %g \n",KTotRel/Nscores);

 egsInformation("\n\n-> <KaTot>= %g <KtTot> = %g NTot = %g \n",KaTot/NTot,KtTot/NTot,NTot);
 egsInformation("-> <Nsplit>apriori    : %g \n",KaTot/NTot/KaPrim*fsplit);
 egsInformation("-> <Nsplit>aposteriori: %g \n",KtTot/NTot/KaPrim*fsplit);
 egsInformation("-> <sc>*fsplit/<Katt>    : %g \n",KTotRel/NTot);


 // egsInformation("-> mean splitting # without correction: %g \n",KaTot/NTot/KaPrim*fsplit);
// egsInformation("-> mean splitting # with C  correction: %g \n",KtTot/NTot/KaPrim*C*fsplit);
// egsInformation("-> mean splitting # with Cx correction: %g \n",KtTot/NTot/KaPrim*Cx*fsplit);

}

void EGS_Corrector::printCorrections(const string& fname){

   ofstream fout(fname.c_str());
   fout << "Geometrical corrections\n"
        << "=======================\n\n"
        << " ix     iy    iz     Xi     Kt        Ka \n"
        << "-----------------------------------------\n";
   char buf[8192]; string str="";

   for (int ix=0; ix<Vx; ix++){
     for(int iy=0; iy<Vy; iy++){
       for(int iz=0; iz<Vz; iz++){
         int ir = ix + iy*Vx + iz*Vxy;
         sprintf(buf," %-6d %-6d  %-6d %-13.5g %-13.5g %-13.5g\n",
                ix,iy,iz,X[ir],Kt[ir],Ka[ir]);
         str += buf;
       }
     }
   }
   fout << str.c_str();
}
