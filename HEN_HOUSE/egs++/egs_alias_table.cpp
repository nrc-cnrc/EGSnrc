/*
###############################################################################
#
#  EGSnrc egs++ alias table
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
#                   Frederic Tessier
#                   Blake Walters
#                   Reid Townson
#
###############################################################################
*/


/*! \file egs_alias_table.cpp
 *  \brief EGS_AliasTable implementation
 *  \IK
 */

#include "egs_alias_table.h"
#include "egs_functions.h"

void EGS_AliasTable::clear() {
    if (n > 0) {
        delete [] fi;
        delete [] xi;
        delete [] wi;
        delete [] bin;
        n = 0;
    }
}

void EGS_AliasTable::allocate(int N, int Type) {
    clear();
    n = N;
    type = Type;
    xi = new EGS_Float [n];
    if (type == 0) {
        np = n;
        fi = new EGS_Float [n];
        wi = new EGS_Float [n];
        bin = new int [n];
    }
    else {
        np = n-1;
        wi = new EGS_Float [n-1];
        bin = new int [n-1];
        if (type == 1) {
            fi = new EGS_Float [n-1];
        }
        else {
            fi = new EGS_Float [n];
        }
    }
}


void EGS_AliasTable::copy(const EGS_AliasTable &t) {
    clear();
    if (t.n > 0) {
        allocate(t.n,t.type);
        for (int j=0; j<np; j++) {
            xi[j] = t.xi[j];
            fi[j] = t.fi[j];
            wi[j] = t.wi[j];
            bin[j] = t.bin[j];
        }
        if (type) {
            xi[np] = t.xi[np];
            if (type == 2 || type == 3) {
                fi[np] = t.fi[np];
            }
        }
    }
}

/**************************************************
      Initializes alias-table:
      N = number of abscissa points
**************************************************/
void EGS_AliasTable::initialize(int N, const EGS_Float *x,
                                const EGS_Float *f, int Type) {
    allocate(N,Type);
    for (int i=0; i<np; i++) {
        xi[i] = x[i];
        fi[i] = f[i];
    }
    if (Type) {
        xi[np] = x[np];
    }
    if (Type == 2 || Type == 3) {
        fi[np] = f[np];
    }
    make();
}

void EGS_AliasTable::make() {
    EGS_Float *fcum = new EGS_Float[np];
    bool *not_done = new bool[np];
    EGS_Float sum = 0, sum1 = 0;
    int i;
    for (i=0; i<np; i++) {
        if (type == 0) {
            fcum[i] = fi[i];
        }
        else if (type == 1) {
            fcum[i] = fi[i]*(xi[i+1]-xi[i]);
        }
        else {
            fcum[i] = 0.5*(fi[i]+fi[i+1])*(xi[i+1]-xi[i]);
        }
        sum += fcum[i];
        wi[i] = 1;
        bin[i] = 0;
        not_done[i] = true;
        if (type == 0) {
            sum1 += fcum[i]*xi[i];
        }
        else if (type == 1) {
            sum1 += 0.5*fcum[i]*(xi[i+1]+xi[i]);
        }
        else sum1 += fcum[i]*(fi[i]*(2*xi[i]+xi[i+1])+
                                  fi[i+1]*(xi[i]+2*xi[i+1]))/(3*(fi[i]+fi[i+1]));
    }
    average = sum1/sum;

    for (i=0; i<np; i++) {
        fi[i] /= sum;
    }
    if (type == 2 || type == 3) {
        fi[np] /= sum;
    }
    sum /= np;

    int jh, jl;
    for (i=0; i<np-1; i++) {

        // find the next "high" bin (above average)
        int high_bin = -1;
        for (jh=0; jh<np; jh++) {
            if (not_done[jh] && fcum[jh] > sum) {
                high_bin = jh;
                break;
            }
        }
        if (high_bin < 0) {
            break;
        }

        // find the next "low" bin (below average)
        int low_bin = -1;
        for (jl=0; jl<np; jl++) {
            if (not_done[jl] && fcum[jl] < sum) {
                low_bin = jl;
                break;
            }
        }
        if (low_bin < 0) {
            egsWarning("EGS_AliasTable::make(): found a high bin, but no low bin; this is abnormal.");
            break;
        }

        // alias the high bin from the low bin
        EGS_Float aux = sum - fcum[low_bin];
        fcum[high_bin] -= aux;
        not_done[jl] = false;
        wi[low_bin] = fcum[low_bin]/sum;
        bin[low_bin] = high_bin;
    }
    delete [] fcum;
    delete [] not_done;
}

#define AT_NCHECK 3

int EGS_AliasTable::initialize(EGS_Float xmin, EGS_Float xmax, EGS_Float accu,
                               int nmax, EGS_AtFunction f, void *data) {
    allocate(2,2);
    xi[0] = xmin;
    xi[1] = xmax;
    fi[0] = f(xi[0],data);
    fi[1] = f(xi[1],data);
    EGS_Float *xtemp, *ftemp;
    int error = 0;
    for (EGS_I64 loopCount=0; loopCount<=loopMax; ++loopCount) {
        if (loopCount == loopMax) {
            egsFatal("EGS_AliasTable::initialize: Too many iterations were required! Input may be invalid, or consider increasing loopMax.");
            return 1;
        }
        int nnn = (n-1)*AT_NCHECK + n;
        xtemp = new EGS_Float[nnn];
        ftemp = new EGS_Float[nnn];
        if (!xtemp || !ftemp) egsFatal("EGS_AliasTable::initialize: "
                                           "not enough memory!\n");
        bool is_ok = true;
        int i,j=0;
        for (i=0; i<n-1; i++) {
            xtemp[j] = xi[i];
            ftemp[j] = fi[i];
            EGS_Float dx = (xi[i+1]-xi[i])/(AT_NCHECK+1);
            for (int l=0; l<AT_NCHECK; l++) {
                EGS_Float x = xi[i]+dx*(l+1);
                EGS_Float fe = f(x,data);
                EGS_Float fa = fi[i]+(fi[i+1]-fi[i])/(xi[i+1]-xi[i])*(x-xi[i]);
                EGS_Float test = fabs(fa/fe-1);
                if (test > accu) {
                    is_ok = false;
                    xtemp[++j] = x;
                    ftemp[j] = fe;
                }
            }
            j++;
        }
        if (is_ok) {
            break;
        }
        xtemp[j] = xi[n-1];
        ftemp[j++] = fi[n-1];
        allocate(j,2);
        for (i=0; i<j; i++) {
            xi[i] = xtemp[i];
            fi[i] = ftemp[i];
        }
        if (n >= nmax) {
            error = 1;
            break;
        }
        delete [] xtemp;
        delete [] ftemp;
    }
    delete [] xtemp;
    delete [] ftemp;
    make();
    return error;
}

int EGS_AliasTable::sampleBin(EGS_RandomGenerator *rndm) const {
    EGS_Float r1 = rndm->getUniform();
    EGS_Float aj = r1*np;
    int j = (int) aj;
    aj -= j;
    if (aj > wi[j]) {
        j = bin[j];
    }
    return j;
}

EGS_Float EGS_AliasTable::sample(EGS_RandomGenerator *rndm) const {
    EGS_Float r1 = rndm->getUniform();
    EGS_Float aj = r1*np;
    int j = (int) aj;
    aj -= j;
    if (aj > wi[j]) {
        j = bin[j];
    }
    if (!type) {
        return xi[j];
    }
    EGS_Float x = xi[j];
    EGS_Float dx = xi[j+1] - x;
    EGS_Float r2 = rndm->getUniform();
    if (type == 1) {
        return x + dx*r2;
    }
    EGS_Float res;
    if (fi[j] > 0) {
        EGS_Float a = fi[j+1]/fi[j]-1;
        if (fabs(a) < 0.2) {
            EGS_Float rnno1 = 0.5*(1-r2)*a;
            res = x + r2*dx*(1+rnno1*(1-r2*a));
        }
        else {
            res = x - dx/a*(1-sqrt(1+r2*a*(2+a)));
        }
    }
    else {
        res = x + dx*sqrt(r2);
    }
    return res;
}


EGS_SimpleAliasTable::EGS_SimpleAliasTable(int N, const EGS_Float *f) : n(0) {

    if (N < 1) {
        return;
    }

    // initialize data members
    n = N;
    wi = new EGS_Float [n];
    bins = new int [n];

    // local variables
    int i;
    double sum = 0;
    double *p = new EGS_Float [n];

    // initialize distribution and bin aliases, and compute histogram sum
    for (i=0; i<n; i++) {
        bins[i] = i;
        p[i] = f[i];
        sum += p[i];
    }

    // normalize distribution
    if (sum <= 0) {
        egsFatal("Error: %s, line %d: degenerate distribution, histogram sum <= 0", __FILE__, __LINE__);
    }
    else {
        for (i=0; i<n; i++) {
            p[i] /= sum;
        }
    }

    // sort bins into "big" and "small" lists
    vector<int> big_list;               // bins above average
    vector<int> small_list;             // bins below average
    for (i=0; i<n; i++) {
        wi[i] = p[i]*n;
        if (wi[i] <= 1.0) {
            small_list.push_back(i);
        }
        else {
            big_list.push_back(i);
        }
    }

    // alias
    int loopCount=0;
    while (big_list.size() > 0 && small_list.size() > 0 && loopCount++ <= loopMax) {

        // get a pair of big and small bins
        int big = big_list.back();
        int small = small_list.back();

        // alias: fill small bin
        bins[small] = big;              // alias small to big
        wi[big] -= (1.0 - wi[small]);   // remove aliased portion from big bin
        small_list.pop_back();          // small bin is now filled

        // check if big bin is now small
        if (wi[big] < 1.0 + epsilon) {
            big_list.pop_back();
            small_list.push_back(big);
        }
    }
    if (big_list.size() > 0) {
        egsWarning("Warning: %s, line %d: table aliasing may be incomplete", __FILE__, __LINE__);
    }

    // delete local arrays
    delete [] p;
}


EGS_SimpleAliasTable::~EGS_SimpleAliasTable() {
    if (n > 0) {
        delete [] wi;
        delete [] bins;
    }
}
