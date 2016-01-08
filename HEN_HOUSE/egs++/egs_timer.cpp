/*
###############################################################################
#
#  EGSnrc egs++ timer
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
#  Contributors:
#
###############################################################################
*/


/*! \file egs_timer.cpp
 *  \brief EGS_Timer implementation
 *  \IK
 */

#include "egs_timer.h"
#include "egs_functions.h"

#ifdef WIN32
#include <ctime>

class EGS_PrivateTimer {
public:
    EGS_PrivateTimer() : mark(clock()) {};
    unsigned long mark;
    void start() {
        mark = clock();
    };
    EGS_Float time() {
        EGS_Float cpu = clock();
        return (cpu - mark)/CLOCKS_PER_SEC;
    };
};

#else

#include <sys/times.h>
#include <unistd.h>

clock_t clps = 0;

#ifndef SKIP_DOXYGEN

/*!  \brief Implementation of the EGS_Timer interface.

  \internwarning

*/
class EGS_PrivateTimer {
public:
    tms tstart, tend;
    EGS_PrivateTimer() {
        if (!clps) {
            clps = sysconf(_SC_CLK_TCK);
        }
        times(&tstart);
    };
    void start() {
        if (times(&tstart) < 0) {
            egsWarning(" times returned < 0???\n");
        }
    };
    EGS_Float time() {
        times(&tend);
        EGS_Float cpu = tend.tms_utime;
        return (cpu - tstart.tms_utime)/clps;
    };
};
#endif

#endif

EGS_Timer::EGS_Timer() {
    p = new EGS_PrivateTimer;
}

EGS_Timer::~EGS_Timer() {
    delete p;
}

void EGS_Timer::start() {
    p->start();
}

EGS_Float EGS_Timer::time() {
    return p->time();
}

