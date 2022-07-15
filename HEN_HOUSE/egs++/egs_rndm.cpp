/*
###############################################################################
#
#  EGSnrc egs++ random number generators
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
#  Contributors:    Hubert Ho
#                   Reid Townson
#
###############################################################################
*/


/*! \file egs_rndm.cpp
 *  \brief EGS_RandomGenerator implementation
 *  \IK
 *
 *  Also provides implementation of RANMAR and should offer
 *  implementation of RANLUX in a future release
 */

#include "egs_rndm.h"
#include "egs_input.h"
#include "egs_functions.h"

#include <iostream>
#include <string>
using namespace std;

void EGS_RandomGenerator::allocate(int n) {
    if (n < 1) {
        egsFatal("Attempt to construct a RNG with n < 1\n");
    }
    rarray = new EGS_Float[n];
    np = n;
    ip = np;
}

void EGS_RandomGenerator::copyBaseState(const EGS_RandomGenerator &r) {
    if (np > 0 && np != r.np) {
        delete [] rarray;
        np = 0;
    }
    if (np <= 0) {
        allocate(r.np);
    }
    ip = r.ip;
    have_x = r.have_x;
    the_x = r.the_x;
    count = r.count;
    for (int j=ip; j<np; j++) {
        rarray[j] = r.rarray[j];
    }
}

EGS_RandomGenerator::EGS_RandomGenerator(int n) : count(0), np(0) {
    allocate(n);
    have_x = false;
    for (int j=0; j<np; j++) {
        rarray[j] = 0;
    }
}

bool EGS_RandomGenerator::storeState(ostream &data) {
    if (!egsStoreI64(data,count)) {
        return false;
    }
    data << " " << np << "  " << ip << endl;
    for (int j=0; j<np; j++) {
        data << rarray[j] << " ";
    }
    data << endl;
    if (!data.good()) {
        return false;
    }
    return storePrivateState(data);
}

bool EGS_RandomGenerator::setState(istream &data) {
    if (!egsGetI64(data,count)) {
        return false;
    }
    int np1;
    data >> np1 >> ip;
    if (!data.good() || data.fail() || data.eof()) {
        return false;
    }
    if (np1 < 1) {
        return false;
    }
    if (np1 != np && np > 0) {
        delete [] rarray;
        rarray = new EGS_Float [np1];
    }
    np = np1;
    for (int j=0; j<np; j++) {
        data >> rarray[j];
    }
    if (!data.good()) {
        return false;
    }
    return setPrivateState(data);
}

bool EGS_RandomGenerator::addState(istream &data) {
    EGS_I64 count_save = count;
    if (!setState(data)) {
        return false;
    }
    count += count_save;
    return true;
}

/*! \brief A ranmar RNG class.
 *
 * This RNG class implements the Zaman \& Marsaglia ranmar generator.
 * ranmar has been the default EGS4 and EGSnrc RNG for many years.
 */
class EGS_LOCAL EGS_Ranmar : public EGS_RandomGenerator {

public:

    /*! \brief Construct a ranmar RNG using \a ixx and \a jxx as the
     * initial seeds.
     */
    EGS_Ranmar(int ixx=1802, int jxx=9373, int n=128) :
        EGS_RandomGenerator(n), high_res(false), copy(0) {
        setState(ixx,jxx);
    };

    EGS_Ranmar(const EGS_Ranmar &r) : EGS_RandomGenerator(r),
        ix(r.ix), jx(r.jx), c(r.c), iseed1(r.iseed1), iseed2(r.iseed2),
        high_res(r.high_res) {
        for (int j=0; j<97; j++) {
            u[j] = r.u[j];
        }
    };

    ~EGS_Ranmar() {
        if (copy) {
            delete copy;
        }
    };

    /*! \brief Fill the array pointed to by \a array with random numbers
     */
    void fillArray(int n, EGS_Float *array);

    /*! \brief Output information about this RNG using egsInformation() */
    void describeRNG() const;

    EGS_RandomGenerator *getCopy();

    void setState(EGS_RandomGenerator *r);

    void saveState();
    void resetState();

    int  rngSize() const {
        return baseSize() + 102*sizeof(int);
    };

    void setHighResolution(bool hr) {
        high_res = hr;
    };

protected:

    /*! Stores the pointers ix and jx, the carry c, the initial seeds that
     * were used to initialize the generator and the 97 element array u.
     */
    bool storePrivateState(ostream &data);
    /*! Reads the same data stored by storePrivateState() */
    bool setPrivateState(istream &data);

    void set(const EGS_Ranmar &r) {
        copyBaseState(r);
        ix = r.ix;
        jx = r.jx;
        c = r.c;
        iseed1 = r.iseed1;
        iseed2 = r.iseed2;
        for (int j=0; j<97; j++) {
            u[j] = r.u[j];
        }
    };

private:

    void setState(int ixx, int jxx);

    int        ix, jx, c;
    int        u[97];
    static     int cd, cm;
    static EGS_Float  twom24;
    int        iseed1, iseed2;

    bool       high_res;

    EGS_Ranmar *copy;

};

void EGS_Ranmar::saveState() {
    if (copy) {
        copy->set(*this);
    }
    else {
        copy = new EGS_Ranmar(*this);
    }
    copy->copy = 0;
}

void EGS_Ranmar::resetState() {
    if (copy) {
        EGS_Ranmar *tmp = copy;
        set(*copy);
        copy = tmp;
    }
}

EGS_RandomGenerator *EGS_Ranmar::getCopy() {
    EGS_Ranmar *c = new EGS_Ranmar(*this);
    c->np = 0;
    c->copy = 0;
    c->copyBaseState(*this);
    return c;
}

void EGS_Ranmar::setState(EGS_RandomGenerator *r) {
    copyBaseState(*r);
    EGS_Ranmar *r1 = dynamic_cast<EGS_Ranmar *>(r);
    if (!r1) {
        egsFatal("EGS_Ranmar::setState: attampt to set my state by a non EGS_Ranmar RNG!\n");
    }
    ix = r1->ix;
    jx = r1->jx;
    c = r1->c;
    iseed1 = r1->iseed1;
    iseed2 = r1->iseed2;
    for (int j=0; j<97; j++) {
        u[j] = r1->u[j];
    }
}

bool EGS_Ranmar::storePrivateState(ostream &data) {
    data << ix << " " << jx << " " << c << " "
         << iseed1 << " " << iseed2 << " " << high_res << endl;
    for (int j=0; j<97; j++) {
        data << u[j] << " ";
    }
    data <<  endl;
    return data.good();
}

bool EGS_Ranmar::setPrivateState(istream &data) {
    data >> ix >> jx >> c >> iseed1 >> iseed2 >> high_res;
    for (int j=0; j<97; j++) {
        data >> u[j];
    }
    return data.good();
}


EGS_Float EGS_Ranmar::twom24 = 1./16777216.;
int EGS_Ranmar::cd = 7654321;
int EGS_Ranmar::cm = 16777213;

void EGS_Ranmar::describeRNG() const {
    egsInformation("Random number generator:\n"
                   "============================================\n");
    egsInformation("  type                = ranmar\n");
    egsInformation("  high resolution     = ");
    if (high_res) {
        egsInformation("yes\n");
    }
    else {
        egsInformation("no\n");
    }
    egsInformation("  initial seeds       = %d %d\n",iseed1,iseed2);
    egsInformation("  numbers used so far = %lld\n",count);
}

void EGS_Ranmar::setState(int ixx, int jxx) {
    if (ixx <= 0 || ixx >= 31328) {
        ixx = 1802;
    }
    if (jxx <= 0 || jxx >= 30081) {
        jxx = 9373;
    }
    iseed1 = ixx;
    iseed2 = jxx;
    int i = (ixx/177)%177 + 2;
    int j = ixx%177 + 2;
    int k = (jxx/169)%178 + 1;
    int l = jxx%169;

    for (int ii=0; ii<97; ii++) {
        int s = 0;
        int t = 8388608;
        for (int jj=0; jj<24; jj++) {
            int m = (((i*j)%179)*k)%179;
            i = j;
            j = k;
            k = m;
            l = (53*l+1)%169;
            if ((l*m)%64 >= 32) {
                s += t;
            }
            t /= 2;
        }
        u[ii] = s;
    }
    c = 362436;
    ix = 96;
    jx = 32;
}

void EGS_Ranmar::fillArray(int n, EGS_Float *array) {
    for (int ii=0; ii<n; ii++) {
        register int r = u[ix] - u[jx--];
#ifdef MSVC
        if (r > 16777219) {
            egsWarning("r = %d\n",r);
        }
#endif
        if (r < 0) {
            r += 16777216;
        }
        u[ix--] = r;
        if (ix < 0) {
            ix = 96;
        }
        if (jx < 0) {
            jx = 96;
        }
        c -= cd;
        if (c < 0) {
            c+=cm;
        }
        r -= c;
        if (r < 0) {
            r+=16777216;
        }
#ifdef MSVC
        if (r > 16777219) {
            egsWarning("r = %d\n",r);
        }
#endif
        array[ii] = twom24*r;
    }
    if (high_res) {
        for (int ii=0; ii<n; ii++) {
            register int r = u[ix] - u[jx--];
#ifdef MSVC
            if (r > 16777219) {
                egsWarning("r = %d\n",r);
            }
#endif
            if (r < 0) {
                r += 16777216;
            }
            u[ix--] = r;
            if (ix < 0) {
                ix = 96;
            }
            if (jx < 0) {
                jx = 96;
            }
            c -= cd;
            if (c < 0) {
                c+=cm;
            }
            r -= c;
            if (r < 0) {
                r+=16777216;
            }
#ifdef MSVC
            if (r > 16777219) {
                egsWarning("r = %d\n",r);
            }
#endif
            array[ii] += twom24*twom24*r;
        }
    }
    count += n;
}


EGS_RandomGenerator *EGS_RandomGenerator::createRNG(EGS_Input *input,
        int sequence) {
    if (!input) {
        egsWarning("EGS_RandomGenerator::createRNG: null input?\n");
        return 0;
    }
    EGS_Input *i;
    bool delete_it = false;
    if (input->isA("rng definition")) {
        i = input;
    }
    else {
        i = input->takeInputItem("rng definition");
        if (!i) {
            egsWarning("EGS_RandomGenerator::createRNG: no 'RNG definition'"
                       " input\n");
            return 0;
        }
        delete_it = true;
    }
    string type;
    int err = i->getInput("type",type);
    if (err) {
        egsWarning("EGS_RandomGenerator::createRNG: no RNG type specified\n"
                   "  Assuming ranmar.\n");
        type = "ranmar";
    }
    EGS_RandomGenerator *result;
    if (i->compare(type,"ranmar")) {
        vector<int> seeds;
        err = i->getInput("initial seeds",seeds);
        EGS_Ranmar *res;
        if (!err && seeds.size() == 2) {
            res = new EGS_Ranmar(seeds[0],seeds[1] + sequence);
        }
        else {
            res = new EGS_Ranmar(1802,9373+sequence);
        }
        vector<string> hr_options;
        hr_options.push_back("no");
        hr_options.push_back("yes");
        bool hr = i->getInput("high resolution",hr_options,0);
        res->setHighResolution(hr);
        result = res;
    }
    else {
        egsWarning("EGS_RandomGenerator::createRNG: unknown RNG type %s\n",
                   type.c_str());
        result = 0;
    }
    if (delete_it) {
        delete i;
    }
    return result;
}

EGS_RandomGenerator *EGS_RandomGenerator::defaultRNG(int sequence) {
    sequence += 97;
    int ixx = 33;
    int iaux = sequence/30081;
    int jxx = sequence - iaux*30081;
    if (iaux > 0) {
        ixx += iaux;
        if (iaux > 31328) egsFatal("EGS_RandomGenerator::defaultRNG: "
                                       "sequence %d is outside of allowed range\n",sequence);
    }
    return new EGS_Ranmar(ixx,jxx);
}
