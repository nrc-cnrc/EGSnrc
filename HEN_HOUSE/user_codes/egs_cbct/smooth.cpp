/*
###############################################################################
#
#  EGSnrc egs++ egs_cbct application smoothing
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
#  Author:          Iwan Kawrakow, 2007
#
#  Contributors:    Ernesto Mainegra-Hing
#
###############################################################################
*/


#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <cmath>

using namespace std;

#include "egs_smoothing.h"
#include "egs_functions.h"

//const int Nx = 64, Ny = 64;

int main(int argc, char**argv) {

    int nmax = 4, nmax2d = 3;
    double chi2max = 1, dmin = 0.02;
    int Nx = 64, Ny = 64, Nz = 72;
    char *ifile=0, *ofile=0, *bench=0;
    for(int j=1; j<argc-1; j++) {
        string tmp(argv[j]);
        if( tmp == "-i" ) ifile = argv[++j];
        else if( tmp == "-o" ) ofile = argv[++j];
        else if( tmp == "-b" ) bench = argv[++j];
        else if( tmp == "-nmax" ) nmax = atoi(argv[++j]);
        else if( tmp == "-nmax2d" ) nmax2d = atoi(argv[++j]);
        else if( tmp == "-chi2max" ) chi2max = atof(argv[++j]);
        else if( tmp == "-min" ) dmin = atof(argv[++j]);
        else if( tmp == "-nx" ) Nx = atoi(argv[++j]);
        else if( tmp == "-ny" ) Ny = atoi(argv[++j]);
        else if( tmp == "-nz" ) Nz = atoi(argv[++j]);
        else if( tmp == "-nz" ) Nz = atoi(argv[++j]);
        else cerr << "Unknown option " << argv[j] << endl;
    }
    if( !ifile ) {
        cerr << "Usage: " << argv[0] << " -i input [-o output] [-nmax n] "
             << "[-nmax2d n2d] [-chi2max chi2]\n";
        return 1;
    }


    /**************************************************/
    /* projection set input */
    /**************************************************/
    string fname(ifile); fname += ".scatonly.scan";
    egsInformation("Reading data ... ");

    EGS_Distribution2DArray proj(fname.c_str(),Nx,Ny,Nz);

    /**************************************************/

    EGS_Distribution2DArray *be = 0;
    if( bench ) {
        fname = bench; fname += ".scatonly.scan";
        be = new EGS_Distribution2DArray(fname.c_str(),Nx,Ny,Nz);
    }

    if( ofile ) fname = ofile;
    else {
        fname = ifile; fname += "_smoothed.scatonly.scan";
    }
    ofstream out(fname.c_str(),ios::binary|ios::app);

    egsInformation("OK\n");

    EGS_Smoothing smoo;
    smoo.setNmax2d(nmax2d); smoo.setNmax(nmax);
    smoo.setChi2Max(chi2max); smoo.setDmin(dmin);
    smoo.setDimensions(Nx,Ny);
    egsInformation("Smoothing data ... ");
    smoo.describeIt();
    /* no smoothing done if any of these is zero */
    if (nmax2d*nmax*chi2max==0){
        out.close();
        egsFatal("...No smoothing done since one of the smoothing\n"
                 "   parameters is null! Smoothed scan identical\n"
                 "   to original scan!\n");
    }
    for(int iproj=0; iproj<Nz; iproj++){

        EGS_Distribution2D* scan = proj.get_proj(iproj);
        EGS_Distribution2D* b    = 0;
        if( be ) {b = be->get_proj(iproj);}

        double sumo = 0, maxdo = 0;
        if( be ) {
           for(int j=0; j<Nx*Ny; j++) {
              double aux = fabs(scan->d_array[j] - b->d_array[j]);
              sumo += aux*aux;
              if( aux > maxdo ) maxdo = aux;
           }
        }
        EGS_Distribution2D *smoothed = smoo.smooth1(scan);
        if( !smoothed ) {
          cerr << "Error while smoothing" << iproj
               << " projection\n"; return 1;
        }
        if( be ) {
          double sum = 0, maxd = 0;
          for(int j=0; j<Nx*Ny; j++) {
             double aux = fabs(smoothed->d_array[j] - b->d_array[j]);
             sum += aux*aux;
             if( aux > maxd ) maxd = aux;
          }
          sum /= (Nx*Ny); sumo /= (Nx*Ny);
          egsInformation("\nMSD  smoothed=%lg original=%lg IR=%lg",sum,sumo,
                         sumo/sum);
          egsInformation("\nMax. difference: smoothed=%lg original=%lg"
                         "IR=%lg\n",
                         maxd,maxdo,maxdo/maxd);
        }

        out.write((char *) smoothed->d_array,smoothed->nreg*sizeof(float));
    }
    egsInformation(" done !\n");

    out.close();
    delete be;
    return 0;

}
