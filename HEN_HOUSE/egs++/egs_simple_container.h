/*
###############################################################################
#
#  EGSnrc egs++ simple container headers
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
#  Author:          Iwan Kawrakow, 2008
#
#  Contributors:
#
###############################################################################
*/


#ifndef EGS_SIMPLE_CONTAINER_
#define EGS_SIMPLE_CONTAINER_

#include "egs_functions.h"

/*! \file   egs_simple_container.h
    \brief  A template for a simple container class.
    \IK
*/


/*! A very simple, lightweight container. */
template <class T> class EGS_EXPORT EGS_SimpleContainer {

public:

    EGS_SimpleContainer() : n_have(0), n_tot(0), n_start(4), n_max(1000000) {};

    EGS_SimpleContainer(int size) : n_have(0), n_tot(size), n_start(4),
        n_max(1000000) {
        if (n_tot > 0) {
            array = new T [n_tot];
        }
        else {
            n_tot = 0;
        }
    };

    ~EGS_SimpleContainer() {
        if (n_tot > 0) {
            delete [] array;
        }
    };

    void add(const T &t) {
        if (n_have >= n_tot) {
            grow();
        }
        array[n_have++] = t;
    };

    void clear() {
        n_have = 0;
    };

    T &operator[](int j) {
        return array[j];
    };

    const T &operator[](int j) const {
        return array[j];
    };

    void  setNmax(int Nmax) {
        if (Nmax > n_tot) {
            n_max = Nmax;
        }
    };

    T &pop() {
        return array[--n_have];
    }

    unsigned int size() const {
        return n_have;
    };

    unsigned int maxSize() const {
        return n_tot;
    };

    unsigned int maxAllowedSize() const {
        return n_max;
    };


protected:

    void   grow() {
        if (n_tot > 0) {
            int nnew = 2*n_tot;
            if (nnew > n_max) {
                nnew = n_max;
                if (nnew <= n_tot) egsFatal("EGS_SimpleContainer::grow(): "
                                                "reached maximum allowed size of %d\n",n_max);
            }
            T *tmp = new T [nnew];
            for (int j=0; j<n_have; j++) {
                tmp[j] = array[j];
            }
            delete [] array;
            array = tmp;
            n_tot = nnew;
        }
        else {
            array = new T [n_start];
            n_tot = n_start;
        }
    };

    int    n_have;
    int    n_tot;
    int    n_start;
    int    n_max;
    T     *array;

};

#endif
