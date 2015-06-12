/*
###############################################################################
#
#  EGSnrc egs++ egs_cbct application smoothing class
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
#
#  Implementation of the EGS_Smoothing class.
#
#  A locally adaptive Savitzky-Goley smoothing algorithm as described by
#  Kawrakow in Med. Phys. 27, 485-498 (2000). The EGS_Smoothing class performs
#  a smoothing of a 2D or 3D distribution using local polynomial fits and
#  rejecting points where the smoothing hypothesis is unlikely to be true.
#
###############################################################################
*/


#include <iostream>
#include <fstream>
#include <cmath>

#include "egs_smoothing.h"
#include "egs_input.h"
#include "egs_functions.h"

using namespace std;

#define NP_2D 6
#define NP_3D 10

#define MIN(a,b) (a) < (b) ? (a) : (b)

EGS_Distribution2D::EGS_Distribution2D(const char *fname) {
    ifstream in(fname,ios::binary);
    if( !in ) egsFatal("Failed to open file %s\n",fname);
    in.read((char *) &nx,sizeof(int));
    in.read((char *) &ny,sizeof(int));
    if( in.fail() || !in.good() ) egsFatal("Failed reading file %s\n",fname);
    if( nx < 1 || ny < 1 || nx > 5000 || ny > 5000 ) {
        egsFatal("Got nx=%d ny=%d while reading file. This is unreasonable\n",
                nx,ny);
    }
    nreg = nx*ny;
    d_array = new float [nreg]; error_array = new float [nreg];
    in.read((char *) d_array,nreg*sizeof(float));
    in.read((char *) error_array,nreg*sizeof(float));
    if( in.fail() || !in.good() ) egsFatal("Failed reading file %s\n",fname);
    for(int j=0; j<nreg; j++) error_array[j] *= error_array[j];
}

EGS_Distribution2D::EGS_Distribution2D(const char *fname, int Nx, int Ny) {
    ifstream in(fname,ios::binary);
    if( !in ) egsFatal("Failed to open file %s\n",fname);
    nx = Nx; ny = Ny; nreg = nx*ny;
    d_array = new float [nreg]; error_array = new float [nreg];
    in.read((char *) d_array,nreg*sizeof(float));
    in.read((char *) error_array,nreg*sizeof(float));
    if( in.fail() || !in.good() ) egsFatal("Failed reading file %s\n",fname);
    for(int j=0; j<nreg; j++) error_array[j] *= error_array[j];
}

EGS_Distribution2DArray::EGS_Distribution2DArray(const char *fname,
                        int Nx, int Ny, int Nz){
    ifstream in(fname,ios::binary);
    if( !in ) egsFatal("Failed to open file %s\n",fname);
    nx=Nx; ny=Ny; nz=Nz; int nreg = Nx*Ny;
    float* d = new float [nreg]; float* err = new float [nreg];
    float* dummy = new float[2*nreg*Nz];
    in.read((char *) dummy,2*nreg*Nz*sizeof(float));
    scan = new EGS_Distribution2D*[Nz];
    for (int i=0; i<2*Nz; i=i+2){
      for (int j=0; j<nreg; j++){
        d[j]   = dummy[j+    i*nreg];
        err[j] = dummy[j+(i+1)*nreg];
      }
      scan[i/2] = new EGS_Distribution2D(Nx,Ny,d, err);
    }
    if( in.fail() || !in.good() ) egsFatal("Failed reading file %s\n",fname);
    delete [] dummy;
}

void EGS_Smoothing::setNmax(int Nmax) {
    if( nmax == Nmax ) return;
    if( nmax > 0 ) {
        delete [] cc1; delete [] cc2; delete [] cc3;
        delete [] cc4; delete [] cc5; delete [] cc6; delete [] ccc;
    }
    nmax = Nmax;
    cc1 = new double [nmax+1]; cc2 = new double [nmax+1];
    cc3 = new double [nmax+1]; cc4 = new double [nmax+1];
    cc5 = new double [nmax+1]; cc6 = new double [nmax+1];
    ccc = new double [nmax+1];
    for(int n=2; n<=nmax; n++) {
        double d0 = 2*n+1; double d2 = d0/3*n*(n+1);
        double d4 = 0.2*d2*(3*n*n+3*n-1);
        double d6 = d2/7*(3*n*n*n*n+6*n*n*n-3*n+1);
        double del1 = d0*d4-d2*d2; double del2 = d2*d6-d4*d4;
        cc1[n]=d4/del1; cc2[n]=d2/del1; cc3[n]=d0/del1;
        cc4[n]=d6/del2; cc5[n]=d4/del2; cc6[n]=d2/del2;
        ccc[n]=d2;
    }
}

void EGS_Smoothing::calculateCoefficients(int Ni, int Nj, bool do_norm) {
    s0 = si = sj = sij = sii = sjj = 0;
    for(register int j=-nj; j<=nj; j++) {
        register int j2 = j*j;
        for(register int i=-ni; i<=ni; i++) {
            register double ff = fsmoo1[reg + i*Ni + j*Nj];
            s0 += ff; sj += ff*j; sjj += ff*j2;
            if( i ) {
                ff *= i; si += ff; sij += ff*j; sii += ff*i;
            }
        }
    }
    if( do_norm ) {
        N = (2*ni+1)*(2*nj+1);
        ai = ni*(ni+1); ai /= 3; bi = 0.8*ai - 0.2;
        aj = nj*(nj+1); aj /= 3; bj = 0.8*aj - 0.2;
    }
    setCoefficients();
}

void EGS_Smoothing::setCoefficients() {
    pi = si/N/ai; pj = sj/N/aj; pij = sij/(ai*aj*N); p0 = s0/N;
    pii = (sii/ai/N-p0)/bi; pjj = (sjj/aj/N-p0)/bj;
    p0 -= (ai*pii+aj*pjj);
}

bool EGS_Smoothing::smooth1D(int iii, int Nx, int Ni, int &Nmax,
        double &se2, double &ww) {
    Nmax = MIN(iii,nmax); if(Nmax < 2) return false;
    Nmax = MIN(Nx-1-iii,Nmax); if(Nmax < 2) return false;

    double sum0=d_array[reg], sum1=0, sum2=0, sum3=0;
    for(register int i=1; i<=Nmax; i++) {
        register double fp = d_array[reg+i*Ni], fm = d_array[reg-i*Ni];
        register double fpp = fp+fm, fmm = (fp-fm)*i;
        register int i2 = i*i;
        sum0 += fpp; sum1 += fmm; sum2 += fpp*i2; sum3 += fmm*i2;
    }
    while(1) {
        N = 2*Nmax+1; ai = Nmax*(Nmax+1); ai /= 3;
        bi = 0.8*ai-0.2;
        p0 = sum0/N; N *= ai; pi = sum1/N; pii = (sum2/N-p0)/bi;
        p0 -= ai*pii; pj = sum1*cc4[Nmax]-sum3*cc5[Nmax];
        pjj = sum3*cc6[Nmax]-sum1*cc5[Nmax];
        double chi2 = 0, chi2a = 0; int nc = 0; se2 = 0;
        double se2a = 1e30;
        for(register int i=-Nmax; i<=Nmax; i++) {
            register int ireg = reg + i*Ni;
            register double df = error_array[ireg];
            if( df > 0 ) {
                ++nc; register double fs = p0 + pi*i + pii*i*i - d_array[ireg];
                //egsWarning("%d %lg %g %lg\n",i,fs,d_array[ireg],df);
                chi2 += fs*fs/df;
                register double aux1 = 1 + (ai-i*i)/bi;
                se2 += aux1*aux1*df;
            }
        }
        //egsWarning("nc=%d chi2=%lg chi2max=%lg\n",nc,chi2,chi2_max);
        if( nc < 4 ) { Nmax = 1; return false; }// minimum possible window np+1
        if( chi2 < chi2_max*(nc-3) ) {
            N /= ai; se2 /= (N*N); ww = (1+ai/bi)/N;
            //egsWarning("accepting: p0=%lg\n",p0);
            return true;
        }
        if( Nmax == 2 ) {
            Nmax = 1; return false;
        }
        register double fp = d_array[reg+Nmax*Ni], fm = d_array[reg-Nmax*Ni];
        register double tmp = Nmax*(fm-fp);
        sum1 += tmp; fp += fm; sum3 += tmp*Nmax*Nmax;
        sum0 -= fp; sum2 -= fp*Nmax*Nmax; --Nmax;
    }
}

int EGS_Smoothing::calculateChi2(int Ni, int Nj, double &chi2, double &se2) {
    chi2 = se2 = 0; int nc = 0;
    for(register int j=-nj; j<=nj; j++) {
        register double tmp1 = p0 + pj*j + pjj*j*j;
        register double tmp2 = pi + pij*j;
        register double aux1 = 1 + (aj-j*j)/bj;
        for(register int i=-ni; i<=ni; i++) {
            register int ireg = reg + i*Ni + j*Nj;
            register double df = error_array[ireg];
            if( df > 0 ) {
                ++nc;
                register double fs = tmp1 + (tmp2+pii*i)*i - d_array[ireg];
                chi2 += fs*fs/df; register double aux2 = aux1 + (ai-i*i)/bi;
                se2 += aux2*aux2*df;
            }
        }
    }
    se2 /= (N*N); return nc;
}

EGS_Distribution2D *
EGS_Smoothing::smooth1(EGS_Distribution2D *the_dose) {
    if( the_dose->nreg != nreg ) {
        egsWarning("EGS_Smoothing::smooth1: distribution to be smoothed "
                "has %d regions\n",the_dose->nreg);
        egsWarning("  whereas I'm expecting %d regions\n",nreg);
        egsWarning("  You sure you know what you are doing?\n");
        return 0;
    }
    EGS_Distribution2D *result = new EGS_Distribution2D(*the_dose);
    fsmoo = result->d_array; dfsmoo = result->error_array;
    d_array = the_dose->d_array; error_array = the_dose->error_array;
    dmax = 0;
    double sume2 = 0;
    bool *smoothed = new bool [nreg];
    fsmoo1 = the_dose->d_array; dfsmoo1 = the_dose->error_array;
    for(ii=0; ii<nreg; ii++) {
        if( d_array[ii] > dmax ) dmax = d_array[ii];
        sume2 += error_array[ii]; smoothed[ii] = false;
        //egsWarning("%d %g %g\n",ii,d_array[ii],error_array[ii]);
    }

    //------------------------------------------------------
    // Searching for maximum acceptable 1D smoothing window
    //------------------------------------------------------
    ni_array = new char [nreg]; nj_array = new char [nreg];
    int ns = 0, ns1d = 0, ns2d = 0;
    double se2a = 0, se2b = 0, se2c = 0, se2d = 0;
    for(ii=0; ii<nx; ii++) {
        //egsWarning(".");
        for(jj=0; jj<ny; jj++) {

            reg = jj*nx + ii;
            if( d_array[reg] < dmin*dmax ) continue;// dmin is fraction below which no smoothing
            double old_df = dfsmoo[reg];
            se2a += old_df; se2b += old_df; ++ns;

            double d2i = old_df, d2j = old_df;
            int nd=0;
            double sump0 = 0, sumw = 0, wi = 0, wj = 0;
            if( smooth1D(ii,nx,1,ni,d2i,wi) ) {
                ++nd; sump0 += p0/d2i; sumw += 1./d2i;
            }
            if( smooth1D(jj,ny,nx,nj,d2j,wj) ) {
                ++nd; sump0 += p0/d2j; sumw += 1./d2j;
            }
            if( nd ) {
                fsmoo[reg] = sump0/sumw; dfsmoo[reg] = 1./sumw;
                if( nd > 1 ) dfsmoo[reg] *= (1+2*old_df*wi*wj/(sumw*d2i*d2j));
                ++ns1d; se2b += dfsmoo[reg] - old_df;
                smoothed[reg] = true;
            }
            ni_array[reg] = ni; nj_array[reg] = nj;
        }
    }
    //egsWarning("\n");
    //------------------------------------------------------
    for(reg=0; reg<nreg; reg++) {
        fsmoo1[reg] = fsmoo[reg]; dfsmoo1[reg] = dfsmoo[reg];
    }
    for(ii=0; ii<nx; ii++) {
        //egsWarning(".");
        for(jj=0; jj<ny; jj++) {

            reg = jj*nx + ii;
            if( d_array[reg] < dmin*dmax ) continue;
            double old_df = dfsmoo[reg];

            ni = MIN(nmax3d,ni_array[reg]);
            nj = MIN(nmax3d,nj_array[reg]);
            if( ni*nj < NP_2D ) continue;// edge or vertice: don't smooth!
            register int i,j;
            double chi2, se2; int nc;
            double fak = 0.8*dfsmoo[reg]/error_array[reg];
            while (1) {
                N = (2*ni+1)*(2*nj+1);
                ai = ni*(ni+1); ai /= 3; bi = 0.8*ai - 0.2;
                aj = nj*(nj+1); aj /= 3; bj = 0.8*aj - 0.2;
                double fakn = (56*ai*aj-9*ai-9*aj+1)/(N*(4*ai-1)*(4*aj-1));
                if( fakn > fak ) break;
                calculateCoefficients(1,nx);
                nc = calculateChi2(1,nx,chi2,se2);
                if( nc > NP_2D && chi2/(nc-NP_2D) < chi2_max ) {
                    if( se2 < dfsmoo[reg] ) {
                        smoothed[reg] = true; ++ns2d;
                        se2b += se2 - dfsmoo[reg];
                        dfsmoo[reg] = se2; fsmoo[reg] = p0;
                    }
                    break;
                }
                if( ni > nj ) --ni; else --nj;
                if( !(ni*nj) ) break;
            }
        }
    }
    //egsWarning("\n");
    int  nsmoothed = 0;char c = '%';
    for(reg=0; reg<nreg; reg++) {
       if (smoothed[reg]) nsmoothed++;
    }
    egsInformation("\n==================\n"
                    " smoothing status \n"
                    "==================\n");
    egsInformation("smoothed %d voxels out of %d\n",nsmoothed,nreg);
    egsInformation("smoothing rate: %g %c\n",100.0*EGS_Float(nsmoothed)/EGS_Float(nreg),c);
    egsInformation("==================\n");

    delete [] smoothed;
    delete [] ni_array; delete [] nj_array;
    return result;
}

void EGS_Smoothing::describeIt(){
         cout << "\nnmax = "    << nmax
              << " nmax2d = " << nmax3d
              << " chi2max = "<< chi2_max
              << " dmin = "<< dmin << endl;
}


EGS_Smoothing::EGS_Smoothing(EGS_Input *I) {
    setDefaults();
    if( !I ) return;
    bool delete_it; EGS_Input *input;
    if( I->isA("smoothing options") ) {
        input = I; delete_it = false;
    }
    else {
        input = I->takeInputItem("smoothing options",false);
        if( !input ) return;
        delete_it = true;
    }

    int nm, err; EGS_Float tmp;
    err = input->getInput("maximum 2D window",nm); if( !err ) nmax3d = nm;
    err = input->getInput("maximum 1D window",nm);
    if( !err ) setNmax(nm);
    err = input->getInput("maximum chi2",tmp); if( !err ) chi2_max = tmp;
    err = input->getInput("minimum dose",tmp); if( !err ) dmin = tmp;

    if( delete_it ) delete input;
}

/****************************************************
              3D Smoothing
*****************************************************/

EGS_Distribution3D::EGS_Distribution3D(const char *fname, int Nx, int Ny, int Nz)
{
    ifstream in(fname,ios::binary);
    if( !in ) egsFatal("Failed to open file %s\n",fname);
    nx=Nx; ny=Ny; nz=Nz; nreg = Nx*Ny*Nz; int nxy = Nx*Ny;
    d_array = new float [nreg]; error_array = new float [nreg];
    float* dummy = new float[2*nreg];
    in.read((char *) dummy,2*nreg*sizeof(float));
    for (int i=0; i<2*Nz; i=i+2){
      int ii = i/2;
      for (int j=0; j<nxy; j++){
            d_array[j+ii*nxy] = dummy[j+    i*nxy];
        error_array[j+ii*nxy] = dummy[j+(i+1)*nxy];
      }
    }
    if( in.fail() || !in.good() ) egsFatal("Failed reading file %s\n",fname);
    delete [] dummy;
}

void EGS_Smoothing3D::set_nmax(int Nmax) {
    if( nmax == Nmax ) return;
    if( nmax > 0 ) {
      delete [] cc1; delete [] cc2; delete [] cc3;
      delete [] cc4; delete [] cc5; delete [] cc6; delete [] ccc;
    }
    nmax = Nmax;
    cc1 = new double [nmax+1]; cc2 = new double [nmax+1];
    cc3 = new double [nmax+1]; cc4 = new double [nmax+1];
    cc5 = new double [nmax+1]; cc6 = new double [nmax+1];
    ccc = new double [nmax+1];
    for(int n=2; n<=nmax; n++) {
      double d0 = 2*n+1; double d2 = d0/3*n*(n+1);
      double d4 = 0.2*d2*(3*n*n+3*n-1);
      double d6 = d2/7*(3*n*n*n*n+6*n*n*n-3*n+1);
      double del1 = d0*d4-d2*d2; double del2 = d2*d6-d4*d4;
      cc1[n]=d4/del1; cc2[n]=d2/del1; cc3[n]=d0/del1;
      cc4[n]=d6/del2; cc5[n]=d4/del2; cc6[n]=d2/del2;
      ccc[n]=d2;
    }
}

void EGS_Smoothing3D::calc_sums() {
  s0 = si = sj = sk = sij = sik = sjk = sii = sjj = skk = 0;
  for(register int k=-nk; k<=nk; k++) {
    register int k2 = k*k; register int irk = (k+kk)*nxy;
    for(register int j=-nj; j<=nj; j++) {
      register int j2 = j*j; register int kj = k*j;
      register int ir = irk + (jj+j)*nx;
      for(register int i=-ni; i<=ni; i++) {
        register double ff = fsmoo1[ir+ii+i];
        s0 += ff; si += ff*i; sj += ff*j; sk += ff*k;
        sij += ff*i*j; sik += ff*i*k; sjk += ff*kj;
        sii += ff*i*i; sjj += ff*j2; skk += ff*k2;
      }
    }
  }
}

void EGS_Smoothing3D::calc_coeffs_ij(int Ni, int Nj, bool do_norm) {
  s0 = si = sj = sij = sii = sjj = 0;
  for(register int j=-nj; j<=nj; j++) {
    register int j2 = j*j;
    for(register int i=-ni; i<=ni; i++) {
      register double ff = fsmoo1[reg + i*Ni + j*Nj];
      s0 += ff; sj += ff*j; sjj += ff*j2;
      if( i ) {
        ff *= i; si += ff; sij += ff*j; sii += ff*i;
      }
    }
  }
  if( do_norm ) {
    N = (2*ni+1)*(2*nj+1);
    ai = ni*(ni+1); ai /= 3; bi = 0.8*ai - 0.2;
    aj = nj*(nj+1); aj /= 3; bj = 0.8*aj - 0.2;
  }
  set_coeffs_ij();
}

void EGS_Smoothing3D::set_coeffs_ij() {
  pi = si/N/ai; pj = sj/N/aj; pij = sij/(ai*aj*N); p0 = s0/N;
  pii = (sii/ai/N-p0)/bi; pjj = (sjj/aj/N-p0)/bj;
  p0 -= (ai*pii+aj*pjj);
}

bool EGS_Smoothing3D::smooth_1d(float *darray, int iii, int Nx, int Ni,
                                int &Nmax, double &se2, double &ww) {
  Nmax = MIN(iii,nmax); if(Nmax < 2) return false;
  Nmax = MIN(Nx-1-iii,Nmax); if(Nmax < 2) return false;

  double sum0=darray[reg], sum1=0, sum2=0;
  for(register int i=1; i<=Nmax; i++) {
    register double fp = darray[reg+i*Ni], fm = darray[reg-i*Ni];
    register double fpp = fp+fm, fmm = (fp-fm)*i;
    register int i2 = i*i;
    sum0 += fpp; sum1 += fmm; sum2 += fpp*i2;
  }
  if( Nmax < 2 ) return false;
  while(1) {
    N = 2*Nmax+1; ai = Nmax*(Nmax+1); ai /= 3;
    bi = 1/(0.8*ai-0.2); // bi = (4*ai-1)/5
    double aii = 1/ai, Nii = 1/N;
    p0 = sum0*Nii; pi = sum1*Nii*aii; pii = (sum2*aii*Nii-p0)*bi;
    double al = p0; p0 -= ai*pii; double bl = 6*sum1/(N*N*Nmax);
    double chi2 = 0, chi2l = 0, se2l = 0; int nc = 0; se2 = 0;
    for(register int i=-Nmax; i<=Nmax; i++) {
      register int ireg = reg + i*Ni;
      //register double df = error_array[ireg];
      register double df = dfsmoo1[ireg];
      if( df > 0 ) {
        ++nc; register double fs = p0 + pi*i + pii*i*i - darray[ireg];
        chi2 += fs*fs/df;
        register double aux1 = 1 + (ai-i*i)*bi;
        se2 += aux1*aux1*df;
        fs = al + bl*i - darray[ireg];
        chi2l += fs*fs/df; se2l += df;
      }
    }
    if( nc < 4 ) { Nmax = 1; return false; }
    if( chi2 < chi2_max*(nc-3) ) {
      se2 *= Nii*Nii; ww = (1+ai*bi)*Nii;
      return true;
    }
    if( Nmax == 2 ) {
      Nmax = 1; return false;
    }
    register double fp = darray[reg+Nmax*Ni], fm = darray[reg-Nmax*Ni];
    register double tmp = Nmax*(fm-fp);
    sum1 += tmp; fp += fm;
    sum0 -= fp; sum2 -= fp*Nmax*Nmax; --Nmax;
  }

}

int EGS_Smoothing3D::calc_chi2_ij(int Ni, int Nj, double &chi2, double &se2) {
  chi2 = se2 = 0; int nc = 0;
  for(register int j=-nj; j<=nj; j++) {
    register double tmp1 = p0 + pj*j + pjj*j*j;
    register double tmp2 = pi + pij*j;
    register double aux1 = 1 + (aj-j*j)/bj;
    for(register int i=-ni; i<=ni; i++) {
      register int ireg = reg + i*Ni + j*Nj;
      register double df = error_array[ireg];
      if( df > 0 ) {
        ++nc;
        register double fs = tmp1 + (tmp2+pii*i)*i - d_array[ireg];
        chi2 += fs*fs/df; register double aux2 = aux1 + (ai-i*i)/bi;
        se2 += aux2*aux2*df;
      }
    }
  }
  se2 /= (N*N); return nc;
}

void EGS_Smoothing3D::set_coeffs() {
  double Ni = 1/N, aii = 1/ai, aji = 1/aj, aki = 1/ak,
         bii = 1/bi, bji = 1/bj, bki = 1/bk;
  pi = si*Ni*aii; pj = sj*Ni*aji; pk = sk*Ni*aki;
  pij = sij*aii*aji*Ni; pik = sik*Ni*aii*aki; pjk = sjk*aji*aki*Ni;
  p0 = s0*Ni;
  pii = (sii*aii*Ni-p0)*bii; pjj = (sjj*aji*Ni-p0)*bji;
  pkk = (skk*aki*Ni-p0)*bki;
  p0 -= (ai*pii+aj*pjj+ak*pkk);
}

int EGS_Smoothing3D::calc_chi2(double &chi2, double &se2) {
  chi2 = se2 = 0; int nc = 0;
  double bii = 1/bi, bji = 1/bj, bki = 1/bk;
  for(register int k=-nk; k<=nk; k++) {
    register double tmp = p0 + pk*k + pkk*k*k;
    register double aux = 1 + (ak-k*k)*bki;
    register int irk = (kk+k)*nxy;
    for(register int j=-nj; j<=nj; j++) {
      register double tmp1 = tmp + pj*j + pjj*j*j + pjk*j*k;
      register double tmp2 = pi + pik*k + pij*j;
      register double aux1 = aux + (aj-j*j)*bji;
      register int ir = irk + (jj+j)*nx;
      for(register int i=-ni; i<=ni; i++) {
        register int ireg = ir+ii+i;
        register double df = error_array[ireg];
        if( df > 0 ) {
          ++nc;
          register double fs = tmp1 + (tmp2+pii*i)*i - d_array[ireg];
          //register double fs = tmp1 + (tmp2+pii*i)*i - fsmoo1[ireg];
          //junk << " 3dchi2: " << fs << "  " << fs/sqrt(df) << endl;
          chi2 += fs*fs/df; register double aux2 = aux1 + (ai-i*i)*bii;
          se2 += aux2*aux2*df;
        }
      }
    }
  }
  se2 /= (N*N); return nc;
}

EGS_Distribution3D *EGS_Smoothing3D::smooth1(const EGS_Distribution3D *the_dose){
  if( the_dose->nreg != nreg ) return 0;
  float dbmax = 0; double diff_b = 0;

  EGS_Distribution3D *result = new EGS_Distribution3D(the_dose);
  fsmoo = result->d_array; dfsmoo = result->error_array;
  d_array = the_dose->d_array; error_array = the_dose->error_array;
  dmax = 0;
  double sume2 = 0;
  bool *smoothed = new bool [nreg];
  fsmoo1 = new float [nreg]; dfsmoo1 = new float [nreg];
  for(ii=0; ii<nreg; ii++) {
    if( d_array[ii] > dmax ) dmax = d_array[ii];
    sume2 += error_array[ii]; smoothed[ii] = false;
    fsmoo1[ii] = d_array[ii]; dfsmoo1[ii] = error_array[ii];
  }

  ni_array = new char [nreg]; nj_array = new char [nreg];
  nk_array = new char [nreg];
  for(int j=0; j<nreg; j++) {
      ni_array[j] = 1; nj_array[j] = 1; nk_array[j] = 1;
  }
  int ns = 0, ns3d = 0, ns1d = 0, ns2d = 0; int mist;
  double se2a = 0, se2b = 0, se2c = 0, se2d = 0;
  float chi2_max_save = chi2_max;
  double scale = sqrt((double) 4*nmax-1); //0.3;
  float *darray; darray = fsmoo1;
  //float *darray = d_array;
  double rough=0; int ncount=0;
  for(ii=0; ii<nx; ii++) {
    for(jj=0; jj<ny; jj++) {
      for(kk=0; kk<nz; kk++) {

        reg = kk*nxy + jj*nx + ii;
        if( d_array[reg] < dmin*dmax ) continue;
        double old_df = dfsmoo[reg];
        //if( iiter==1 ) { se2a += old_df; se2b += old_df; ++ns; }
        se2a += old_df; se2b += old_df; ++ns;

        //double d2i = old_df, d2j = old_df, d2k = old_df;
        double d2i = 1/old_df; double d2j = d2i, d2k = d2i;
        //double d2i = old_df, d2j = old_df, d2k = old_df;
        double p0i, p0j, p0k; int nd=0;
        double sump0 = 0, sumw = 0, wi = 0, wj = 0, wk = 0;
        if( smooth_1d(darray,ii,nx,1,ni,d2i,wi) ) {
          d2i = 1/d2i;
          ++nd; sump0 += p0*d2i; sumw += d2i; //wi = (1+ai/bi)/N;
        }
        if( smooth_1d(darray,jj,ny,nx,nj,d2j,wj) ) {
          d2j = 1/d2j;
          ++nd; sump0 += p0*d2j; sumw += d2j; //wj = (1+ai/bi)/N;
        }
        if( smooth_1d(darray,kk,nz,nxy,nk,d2k,wk) ) {
          d2k = 1/d2k;
          ++nd; sump0 += p0*d2k; sumw += d2k; //wk = (1+ai/bi)/N;
        }
        if( nd ) {
          sumw = 1/sumw; fsmoo[reg] = sump0*sumw;
          if( nd > 1 ) sumw *=
            (1 + 2*old_df*sumw*(wi*wj*d2i*d2j+wi*wk*d2i*d2k+wj*wk*d2j*d2k));
          if( sumw < dfsmoo[reg] ) dfsmoo[reg] = sumw;
          ++ns1d; se2b += dfsmoo[reg] - old_df;
          smoothed[reg] = true;
        }
        ni_array[reg] = ni; nj_array[reg] = nj; nk_array[reg] = nk;
      }
    }
  }

  for(int j=0; j<nreg; j++) dfsmoo1[j] = dfsmoo[j];

  if( !do_3d ) return result;

  for(ii=0; ii<nx; ii++) {
    for(jj=0; jj<ny; jj++) {
      for(kk=0; kk<nz; kk++) {

        reg = kk*nxy + jj*nx + ii;
        if( d_array[reg] < dmin*dmax ) continue;
        double old_df = dfsmoo[reg];

        ni = MIN(nmax3d,ni_array[reg]);
        nj = MIN(nmax3d,nj_array[reg]);
        nk = MIN(nmax3d,nk_array[reg]);
        register int i,j,k;
        double chi2, se2; int nc;
        if( ni*nj*nk > 0 ) {
          se2c += dfsmoo[reg]; se2d += dfsmoo[reg];
          double fak = 0.8*dfsmoo[reg]/error_array[reg];
          while (1) {
            N = (2*ni+1)*(2*nj+1)*(2*nk+1);
            ai = ni*(ni+1); ai /= 3; bi = 0.8*ai-0.2;
            aj = nj*(nj+1); aj /= 3; bj = 0.8*aj-0.2;
            ak = nk*(nk+1); ak /= 3; bk = 0.8*ak-0.2;
            double fakn = (304*ai*aj*ak-56*(ai*aj+ai*ak+aj*ak)+9*(ai+aj+ak)-1)/
                          (N*(4*ai-1)*(4*aj-1)*(4*ak-1));
            if( fakn > fak ) break;
            calc_sums(); set_coeffs();
            nc = calc_chi2(chi2,se2);
            if( nc > NP_3D && chi2/(nc-NP_3D) < chi2_max ) {
            //if( chi2 < chi2_max*(nc-NP_3D) ) {
              if( se2 < dfsmoo[reg] ) {
                se2d += se2 - dfsmoo[reg];
                se2b += se2 - dfsmoo[reg]; ++ns3d;
                fsmoo[reg] = p0;
                //if( !smoothed[reg] ) dfsmoo[reg] = se2;
                dfsmoo[reg] = se2;
              }
              smoothed[reg] = true; break;
            }
            if( ni > nj && ni > nk ) --ni;
            else if( nj > nk ) --nj;
            else --nk;
            if( !(ni*nj*nk) ) break;
          }
        }
        else {  // this must be a surface voxel, try 2d
          double fak = 0.8*dfsmoo[reg]/error_array[reg];
          if( ni*nj > 0 ) {
            while (1) {
              N = (2*ni+1)*(2*nj+1);
              ai = ni*(ni+1); ai /= 3; bi = 0.8*ai - 0.2;
              aj = nj*(nj+1); aj /= 3; bj = 0.8*aj - 0.2;
              double fakn = (56*ai*aj-9*ai-9*aj+1)/(N*(4*ai-1)*(4*aj-1));
              if( fakn > fak ) break;
              calc_coeffs_ij(1,nx);
              nc = calc_chi2_ij(1,nx,chi2,se2);
              if( nc > NP_2D && chi2/(nc-NP_2D) < chi2_max ) {
                if( se2 < dfsmoo[reg] ) {
                  smoothed[reg] = true; ++ns2d;
                  se2b += se2 - dfsmoo[reg];
                  dfsmoo[reg] = se2; fsmoo[reg] = p0;
                }
                break;
              }
              if( ni > nj ) --ni; else --nj;
              if( !(ni*nj) ) break;
            }
          }
          else if( ni*nk > 0 ) {
            nj = nk;
            while (1) {
              N = (2*ni+1)*(2*nj+1);
              ai = ni*(ni+1); ai /= 3; bi = 0.8*ai - 0.2;
              aj = nj*(nj+1); aj /= 3; bj = 0.8*aj - 0.2;
              double fakn = (56*ai*aj-9*ai-9*aj+1)/(N*(4*ai-1)*(4*aj-1));
              if( fakn > fak ) break;
              calc_coeffs_ij(1,nxy);
              nc = calc_chi2_ij(1,nxy,chi2,se2);
              if( nc > NP_2D && chi2/(nc-NP_2D) < chi2_max ) {
                if( se2 < dfsmoo[reg] ) {
                  smoothed[reg] = true; ++ns2d;
                  se2b += se2 - dfsmoo[reg];
                  dfsmoo[reg] = se2; fsmoo[reg] = p0;
                }
                break;
              }
              if( ni > nj ) --ni; else --nj;
              if( !(ni*nj) ) break;
            }
          }
          else if( nj*nk > 0 ) {
            ni = nk;
            while (1) {
              N = (2*ni+1)*(2*nj+1);
              ai = ni*(ni+1); ai /= 3; bi = 0.8*ai - 0.2;
              aj = nj*(nj+1); aj /= 3; bj = 0.8*aj - 0.2;
              double fakn = (56*ai*aj-9*ai-9*aj+1)/(N*(4*ai-1)*(4*aj-1));
              if( fakn > fak ) break;
              calc_coeffs_ij(nxy,nx);
              nc = calc_chi2_ij(nxy,nx,chi2,se2);
              if( nc > NP_2D && chi2/(nc-NP_2D) < chi2_max ) {
                if( se2 < dfsmoo[reg] ) {
                  smoothed[reg] = true; ++ns2d;
                  se2b += se2 - dfsmoo[reg];
                  dfsmoo[reg] = se2; fsmoo[reg] = p0;
                }
                break;
              }
              if( ni > nj ) --ni; else --nj;
              if( !(ni*nj) ) break;
            }
          }
        }
      }
    }
  }
//   chi2_max = chi2_max_save;
//   ns1d=0;
//   {for(int j=0; j<nx*ny*nz; j++) {
//       if( d_array[j] >= dmin*dmax && smoothed[j] ) ns1d++;
//       if( smoothed[j] ) {
//           result->error_array[j] *= 1.075;
//           if( result->error_array[j] > the_dose->error_array[j] )
//               result->error_array[j] = the_dose->error_array[j];
//       }
//   }}

  se2a = se2b = 0;
  delete [] smoothed; delete [] fsmoo1; delete [] dfsmoo1;
  delete [] ni_array; delete [] nj_array; delete [] nk_array;
  return result;
}

EGS_Smoothing3D::EGS_Smoothing3D(EGS_Input *I) {
    setDefaults();
    if( !I ) return;
    bool delete_it; EGS_Input *input;
    if( I->isA("smoothing options") ) {
        input = I; delete_it = false;
    }
    else {
        input = I->takeInputItem("smoothing options",false);
        if( !input ) return;
        delete_it = true;
    }

    int nm, err; EGS_Float tmp;
    err = input->getInput("maximum 3D window",nm); if( !err ) nmax3d = nm;
    err = input->getInput("maximum 1D window",nm); if( !err ) set_nmax(nm);
    err = input->getInput("maximum chi2",tmp); if( !err ) chi2_max = tmp;
    err = input->getInput("minimum fraction",tmp); if( !err ) dmin = tmp;

    if( delete_it ) delete input;
}

void EGS_Smoothing3D::show() { //(std::ostream &stream) {
  egsInformation(" smoothing options:\n max. window in 1d: %d\n"
   " max. window in 3d: %d\n max. chi2        : %g\n"
   " min. dose        : %g\n",nmax,nmax3d,chi2_max,dmin);
}

void EGS_Distribution3D::analyze_dose(float norm, float cpu) {
  register int j; float dmax = 0; int imax=-1;
  for(j=0; j<nreg; j++) {
    if( d_array[j] > dmax ) { dmax = d_array[j]; imax = j; }
  }
  egsInformation(
  "============ EGS_Distribution3D::analyze: ===================\n");
  egsInformation(" max dose is %g in region %d\n",dmax,imax);
  double ddmax = dmax;
  if(norm > 0) ddmax = norm;
  double e20=0, e50=0, e90=0, t20=0, t50=0, t90=0;
  for(j=0; j<nreg; j++) {
    double d = d_array[j];
    if( d > 0.2*ddmax ) {
      double aux = error_array[j];
      t20 += 1; e20 += aux;
      if( d > 0.5*ddmax ) {
        t50 += 1; e50 += aux;
        if( d > 0.9*ddmax ) {
          t90 += 1; e90 += aux;
        }
      }
    }
  }
  e20 /= (t20+1e-4); e50 /= (t50+1e-4); e90 /= (t90+1e-4);
  float eff; char shit='%';
  egsInformation(" cpu time: %g\n",cpu);
  egsInformation(" average stat. errors and efficiencies:\n");
  egsInformation("   D > 20%c: %g ",shit,100*sqrt(e20)/ddmax);
  eff = ddmax*ddmax/e20/cpu;
  egsInformation(" eff = %g (%d voxels)\n",eff,(int)t20);
  egsInformation("   D > 50%c: %g ",shit,100*sqrt(e50)/ddmax);
  eff = ddmax*ddmax/e50/cpu;
  egsInformation(" eff = %g (%d voxels)\n",eff,(int)t50);
  egsInformation("   D > 90%c: %g ",shit,100*sqrt(e90)/ddmax);
  eff = ddmax*ddmax/e90/cpu;
  egsInformation(" eff = %g (%d voxels)\n",eff,(int)t90);
  egsInformation("===================================================\n");

  e20=0; e50=0; e90=0; t20=0; t50=0; t90=0;
  for(j=0; j<nreg; j++) {
    double d = d_array[j];
    if( d > 0.5*ddmax ) {
      double aux = error_array[j]/d/d;
      t50 += 1; e50 += aux;
    }
  }
  e50 /= (t50+1e-4);
  egsInformation("\n\n Efficiency derived from average relative uncertainty\n"
    "  for  D > 50%c: %g 1/s\n\n",shit,1./e50/cpu);
  egsInformation("===================================================\n");
}
