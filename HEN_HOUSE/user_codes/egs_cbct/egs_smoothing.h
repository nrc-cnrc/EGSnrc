/*
###############################################################################
#
#  EGSnrc egs++ egs_cbct application smoothing class headers
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
#  Definition of the EGS_Smoothing class.
#
#  A locally adaptive Savitzky-Goley smoothing algorithm as described by
#  Kawrakow in Med. Phys. 27, 485-498 (2000). The EGS_Smoothing class performs
#  a smoothing of a 2D or 3D distribution using local polynomial fits and
#  rejecting points where the smoothing hypothesis is unlikely to be true.
#
###############################################################################
*/


#ifndef EGS_SMOOTHING__
#define EGS_SMOOTHING__

#include <string>
#include <iostream>

using namespace std;
using std::string;

struct  EGS_Distribution2D {
    int    nreg, ncase;
    int    nx, ny;
    float  *d_array, *error_array;
    EGS_Distribution2D() { nreg = 0; nx = ny = 0; };
    EGS_Distribution2D(int N, float d=0, float err=1e30) {
        nreg = N; d_array = new float [nreg];
        error_array = new float [nreg];
        for(int j=0; j<nreg; j++) { d_array[j]=d; error_array[j]=err; }
    };
    EGS_Distribution2D(int Nx, int Ny, float d=0, float err=1e30) {
        nx = Nx; ny = Ny; nreg = nx*ny;
        d_array = new float [nreg]; error_array = new float [nreg];
        for(int j=0; j<nreg; j++) { d_array[j]=d; error_array[j]=err; }
    };
    EGS_Distribution2D(int Nx, int Ny, const float *d, const float *err) {
        nx = Nx; ny = Ny; nreg = nx*ny;
        d_array = new float [nreg]; error_array = new float [nreg];
        for(int j=0; j<nreg; j++) {
            d_array[j]=d[j];
            error_array[j]=err[j]*err[j];
        }
    };
    EGS_Distribution2D(const EGS_Distribution2D &dose) {
        nreg = dose.nreg;
        nx = dose.nx; ny = dose.ny;
        d_array = new float [nreg];
        error_array = new float [nreg];
        ncase = dose.ncase;
        for(int j=0; j<nreg; j++) {
            d_array[j] = dose.d_array[j];
            error_array[j] = dose.error_array[j];
        }
    };
    EGS_Distribution2D(const char *fname);
    EGS_Distribution2D(const char *fname, int Nx, int Ny);
    ~EGS_Distribution2D() {
        if( nreg > 0 ) { delete [] d_array; delete [] error_array; }
    };
    void add(const EGS_Distribution2D *dose) {
        if( nreg != dose->nreg ) return;
        for(int j=0; j<nreg; j++) {
            double aux1 = d_array[j]/error_array[j] +
                dose->d_array[j]/dose->error_array[j];
            double aux2 = 1./error_array[j] + 1./dose->error_array[j];
            d_array[j] = aux1/aux2; error_array[j] = 1./aux2;
            // the above assumes that the errors are already squared!
        }
    };
};

struct EGS_Distribution2DArray {

EGS_Distribution2DArray(const char *fname, int Nx, int Ny, int Nz);

~EGS_Distribution2DArray() {
        if( nx*ny*nz > 0 ) { delete [] scan; }
};

EGS_Distribution2D* get_proj(int i){return scan[i];}

private:

int nx, ny, nz;
EGS_Distribution2D** scan;

};


class EGS_Input;

class  EGS_Smoothing {

protected:

    int     nmax, nmax3d;
    double  chi2_max, dmin, dmax;
    int     nx, ny, nreg;
    int     ni, nj;
    int     ii, jj, reg;
    double  N, ai, bi, aj, bj;
    double  p0, pi, pj, pk, pij, pik, pjk, pii, pjj, pkk;
    double  s0, si, sj, sk, sij, sik, sjk, sii, sjj, skk;
    double  u0, ui, uj, uk, uij, uik, ujk, uii, ujj, ukk;

    float   *d_array, *error_array;
    float   *fsmoo, *dfsmoo;
    float   *fsmoo1, *dfsmoo1;
    char    *ni_array, *nj_array;

    double  *cc1, *cc2, *cc3, *cc4, *cc5, *cc6, *ccc;

    void    calculateCoefficients(int Ni, int Nj, bool do_norm = true);
    void    setCoefficients();
    int     calculateChi2(int,int,double &,double &);

    bool    smooth1D(int iii, int Nx, int Ni, int &nn, double &se2, double &ww);
    void    setDefaults() {
        nmax = 0; setNmax(3); chi2_max = 1; setDimensions(0,0);
    };

public:

    EGS_Smoothing() { setDefaults(); };
    EGS_Smoothing(EGS_Input *);

    ~EGS_Smoothing() {
        if( nmax > 0 ) {
            delete [] cc1; delete [] cc2; delete [] cc3;
            delete [] cc4; delete [] cc5; delete [] cc6; delete [] ccc;
        }
    };

    void  setNmax2d(int Nmax) { nmax3d = Nmax; };
    void  setNmax(int Nmax);
    void  setChi2Max(double chi2) { chi2_max = chi2; };
    void  setDmin(double Dmin) { dmin = Dmin; };
    void  setDimensions(int Nx, int Ny) {
        nx = Nx; ny = Ny;;
        nreg = nx*ny;
    };
    void describeIt();
    EGS_Distribution2D *smooth1(EGS_Distribution2D *the_dose);

};

/****************************************************
              3D Smoothing Classes
*****************************************************/
struct EGS_Distribution3D {
  int    nreg, ncase, nbatch;
  int    nx, ny, nz;
  double fluence;
  float  *d_array, *error_array;
  EGS_Distribution3D() { nreg = 0; nx = ny = nz = 0; };
  EGS_Distribution3D(int N, float d=0, float err=1e30) {
    nreg = N; d_array = new float [nreg];
    error_array = new float [nreg];
    for(register int j=0; j<nreg; j++) { d_array[j]=d; error_array[j]=err; }
  };
  EGS_Distribution3D(int Nx, int Ny, int Nz, float d=0, float err=1e30) {
    nx = Nx; ny = Ny; nz = Nz; nreg = nx*ny*nz;
    d_array = new float [nreg]; error_array = new float [nreg];
    for(register int j=0; j<nreg; j++) { d_array[j]=d; error_array[j]=err; }
  };
  EGS_Distribution3D(const char *fname, int Nx, int Ny, int Nz);
  EGS_Distribution3D(const EGS_Distribution3D *dose) {
    nreg = dose->nreg;
    nx = dose->nx; ny = dose->ny; nz = dose->nz;
    d_array = new float [nreg];
    error_array = new float [nreg];
    ncase = dose->ncase; nbatch = dose->nbatch;
    for(register int j=0; j<nreg; j++) {
      d_array[j] = dose->d_array[j];
      error_array[j] = dose->error_array[j];
    }
  }
  ~EGS_Distribution3D() {
    if( nreg > 0 ) {
        delete [] d_array; delete [] error_array;
    }
  };
  void add_dose1(const EGS_Distribution3D *dose) {
    if( nreg != dose->nreg ) return;
    ncase += dose->ncase; register double w2 = dose->ncase; w2 /= ncase;
    register double w1 = 1 - w2; register double w12 = w1*w1;
    register double w22 = w2*w2;
    cout << "combining: ncase = " << ncase << "  " << dose->ncase << endl;
    for(register int j=0; j<nreg; j++) {
      d_array[j] = w1*d_array[j] + w2*dose->d_array[j];
      error_array[j] = w12*error_array[j] + w22*dose->error_array[j];
    }
  };
  void add_dose(const EGS_Distribution3D *dose) {
    if( nreg != dose->nreg ) return;
    for(register int j=0; j<nreg; j++) {
      register double aux1 = d_array[j]/error_array[j] +
	    dose->d_array[j]/dose->error_array[j];
      register double aux2 = 1./error_array[j] + 1./dose->error_array[j];
      d_array[j] = aux1/aux2; error_array[j] = 1./aux2;
        // the above assumes that the errors are already squared!
    }
  }
  void analyze_dose(float norm, float cpu);
  void combine(const EGS_Distribution3D *dose) {
    if( nreg != dose->nreg ) return;
    for(register int j=0; j<nreg; j++) {
      if( dose->error_array[j] < error_array[j] ) {
        d_array[j] = dose->d_array[j];
	 error_array[j] = dose->error_array[j];
      }
    }
  }
};

class EGS_Smoothing3D {

protected:

  int     nmax, nmax3d;
  bool    do_3d;
  double  chi2_max, dmin, dmax;
  int     nx, ny, nz, nxy, nreg;
  int     ni, nj, nk;
  int     ii, jj, kk, reg;
  double  N, ai, bi, aj, bj, ak, bk;
  double  p0, pi, pj, pk, pij, pik, pjk, pii, pjj, pkk;
  double  s0, si, sj, sk, sij, sik, sjk, sii, sjj, skk;
  double  u0, ui, uj, uk, uij, uik, ujk, uii, ujj, ukk;

  float   *d_array, *error_array;
  const float *rhof;
  float   *fsmoo, *dfsmoo;
  float   *fsmoo1, *dfsmoo1;
  char    *ni_array, *nj_array, *nk_array;

  double  *cc1, *cc2, *cc3, *cc4, *cc5, *cc6, *ccc;
  void    calc_sums();
  void    copy_sums();
  void    reset_sums();
  void    set_coeffs();
  int     calc_chi2(double &, double &);

  void    calc_coeffs_ij(int Ni, int Nj, bool do_norm = true);
  void    set_coeffs_ij();
  int     calc_chi2_ij(int,int,double &,double &);

  bool    smooth_1d(float *darray, int iii, int Nx, int Ni,
                    int &nn, double &se2, double &ww);
  void    setDefaults() {
    nmax = 0; set_nmax(3); chi2_max = 1; set_cube_dimensions(0,0,0);
    do_3d = true; dmin = 0.1;
  };

public:

  EGS_Smoothing3D() {
    nmax = 0; set_nmax(3); chi2_max = 1; set_cube_dimensions(0,0,0);
    do_3d = true; dmin = 0.1;
  };
  EGS_Smoothing3D(EGS_Input *I);
  ~EGS_Smoothing3D() {
    if( nmax > 0 ) {
      delete [] cc1; delete [] cc2; delete [] cc3;
      delete [] cc4; delete [] cc5; delete [] cc6; delete [] ccc;
    }
  };

  void  set_nmax3d(int Nmax) { nmax3d = Nmax; };
  void  set_nmax(int Nmax);
  void  set_chi2_max(double chi2) { chi2_max = chi2; };
  void  set_dmin(double Dmin) { dmin = Dmin; };
  void  set_cube_dimensions(int Nx, int Ny, int Nz) {
                             nx = Nx; ny = Ny; nz = Nz;
                             nxy = nx*ny; nreg = nxy*nz; };
  void set_do3d(bool doit) { do_3d = doit; };

  EGS_Distribution3D *smooth1(const EGS_Distribution3D *the_dose);
  void   show(); //(std::ostream &stream);

};

#endif
