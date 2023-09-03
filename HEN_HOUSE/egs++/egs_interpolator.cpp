/*
###############################################################################
#
#  EGSnrc egs++ interpolator
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
#  Author:          Iwan Kawrakow, 2005
#
#  Contributors:    Ernesto Mainegra-Hing
#                   Reid Townson
#
###############################################################################
*/


/*! \file egs_interpolator.cpp
 *  \brief EGS_Interpolator implementation
 *  \IK
 */

#include <cmath>

#include "egs_interpolator.h"
#include "egs_functions.h"

EGS_Interpolator::EGS_Interpolator() : n(0), own_data(true) {};

EGS_Interpolator::EGS_Interpolator(int nbin, EGS_Float Xmin, EGS_Float Xmax,
                                   const EGS_Float *values) : n(0), own_data(true) {
    initialize(nbin,Xmin,Xmax,values);
}

EGS_Interpolator::EGS_Interpolator(int nbin, EGS_Float Xmin, EGS_Float Xmax,
                                   EGS_InterpolatorFuncion func, void *data) : n(0), own_data(true) {
    initialize(nbin,Xmax,Xmax,func,data);
}

EGS_Interpolator::EGS_Interpolator(int nbin, EGS_Float Xmin, EGS_Float Xmax,
                                   EGS_Float *a, EGS_Float *b) : n(0), own_data(true) {
    initialize(nbin,Xmax,Xmax,a,b);
}

EGS_Interpolator::EGS_Interpolator(EGS_Float Xmin, EGS_Float Xmax,
                                   EGS_InterpolatorFuncion func, void *data,
                                   int nmax, EGS_Float accu) : n(0), own_data(true) {
    initialize(Xmin,Xmax,func,data,nmax,accu);
}

void EGS_Interpolator::clear() {
    if (n > 0 && own_data) {
        delete [] a;
        delete [] b;
        n = 0;
    }
}

EGS_Interpolator::~EGS_Interpolator() {
    clear();
}

void EGS_Interpolator::check(int nbin, EGS_Float Xmin, EGS_Float Xmax) {
    if (nbin < 2) egsFatal("EGS_Interpolator::initialize: \n"
                               "    attempt to initialize the interpolator with %d bins\n",nbin);
    if (Xmin >= Xmax) egsFatal("EGS_Interpolator::initialize: \n"
                                   "    Xmin must be > Xmax, I got Xmin=%g Xmax=%g\n",Xmin,Xmax);
}

void EGS_Interpolator::initialize(int nbin, EGS_Float Xmin, EGS_Float Xmax,
                                  const EGS_Float *values) {
    check(nbin,Xmin,Xmax);
    clear();
    own_data = true;
    n = nbin - 1;
    a = new EGS_Float [nbin], b = new EGS_Float [nbin];
    xmin = Xmin;
    xmax = Xmax;
    fmin = values[0];
    fmax = values[n];
    EGS_Float dx = (xmax-xmin)/n;
    bx = 1/dx;
    ax = -xmin*bx;
    for (int j=0; j<n; j++) {
        b[j] = (values[j+1]-values[j])*bx;
        a[j] = values[j] - b[j]*(xmin + dx*j);
    }
    /****************************************************
       Extra subinterval at top of interval, taking care
       of round-off errors via extrapolation.
       Mimics PEGS4 PWLF QFIT approach.
    *****************************************************/
    a[n] = a[n-1];
    b[n] = b[n-1];
}

void EGS_Interpolator::initialize(int nbin, EGS_Float Xmin, EGS_Float Xmax,
                                  EGS_InterpolatorFuncion func,void *data) {
    EGS_Float *aux = new EGS_Float [nbin];
    EGS_Float dx = (Xmax - Xmin)/(nbin-1);
    for (int j=0; j<nbin; j++) {
        aux[j] = func(Xmin + dx*j, data);
    }
    initialize(nbin,Xmin,Xmax,aux);
    delete [] aux;
}

void EGS_Interpolator::initialize(int nbin, EGS_Float Xmin, EGS_Float Xmax,
                                  EGS_Float *A, EGS_Float *B) {
    check(nbin,Xmin,Xmax);
    clear();
    own_data = false;
    n = nbin - 1;
    xmin = Xmin;
    xmax = Xmax;
    EGS_Float dx = (xmax-xmin)/n;
    bx = 1/dx;
    ax = -xmin*bx;
    a = A;
    b = B;
    fmin = a[0] + b[0]*xmin;
    fmax = a[n-1]+b[n-1]*xmax;
}

void EGS_Interpolator::initialize(EGS_Float Xmin, EGS_Float Xmax,
                                  EGS_InterpolatorFuncion func, void *data,
                                  int nmax, EGS_Float accu) {
    check(nmax,Xmin,Xmax);
    clear();
    if (nmax <= 9) {
        initialize(nmax,Xmin,Xmax,func,data);
        return;
    }
    int nnow = 9;
    EGS_Float *tmp = new EGS_Float [nnow];
    int j;
    EGS_Float ddx = (Xmax-Xmin)/(nnow-1);
    for (j=0; j<nnow; j++) {
        tmp[j] = func(Xmin + ddx*j, data);
    }
    for (EGS_I64 loopCount=0; loopCount<=loopMax; ++loopCount) {
        if (loopCount == loopMax) {
            egsFatal("EGS_Interpolator::initialize: Too many iterations were required! Input may be invalid, or consider increasing loopMax.");
            return;
        }
        EGS_Float *tmp1 = new EGS_Float [nnow-1];
        for (j=0; j<nnow-1; j++) {
            tmp1[j] = func(Xmin + ddx*(0.5+j),data);
        }
        bool ok = true;
        for (j=0; j<nnow-1; j++) {
            EGS_Float fint = 0.5*(tmp[j] + tmp[j+1]);
            EGS_Float err = fabs(fint/tmp1[j]-1);
            if (err > accu) {
                ok = false;
                break;
            }
        }
        if (ok) {
            initialize(nnow,Xmin,Xmax,tmp);
            delete [] tmp;
            delete [] tmp1;
            return;
        }
        if (2*nnow-1 > nmax) {
            delete [] tmp;
            delete [] tmp1;
            break;
        }
        EGS_Float *aux = new EGS_Float [2*nnow-1];
        aux[0] = tmp[0];
        int i = 1;
        for (j=1; j<nnow; j++) {
            aux[i++] = tmp1[j-1];
            aux[i++] = tmp[j];
        }
        delete [] tmp;
        delete [] tmp1;
        tmp = aux;
        nnow = 2*nnow-1;
        ddx = (Xmax-Xmin)/(nnow-1);
    }
    initialize(nmax,Xmin,Xmax,func,data);
}
