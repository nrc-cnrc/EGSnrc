/*
###############################################################################
#
#  EGSnrc egs++ timer headers
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
#  Contributors:    Reid Townson
#
###############################################################################
*/


/*! \file egs_timer.h
 *  \brief EGS_Timer class header file
 *  \IK
 */

#ifndef EGS_TIMER_
#define EGS_TIMER_

#include "egs_libconfig.h"

class EGS_PrivateTimer;

/*! \brief A simple class for measuring CPU time.
 *
 * \ingroup egspp_main
 *
 * The reson for the existence of this simple class is the fact that
 * on Linux the CLOCKS_PER_SEC macro is equal to 1e6/second so that the
 * clock() function, which returns the CPU time in units of CLOCKS_PER_SEC
 * runs out only after about 40 minutes on 32 bit systems where an unsigned
 * 32 bit integer variable is used. This is not very long for a Monte Carlo
 * simulation.
 *
 * \todo Should add wall time measurement
 */
class EGS_EXPORT EGS_Timer {

public:

    /*! \brief Construct an EGS_Timer object. */
    EGS_Timer();
    /*! \brief Destructor */
    ~EGS_Timer();

    /*! \brief Starts the time measurement. */
    void start();

    /*! \brief Returns the CPU time in seconds since start() was called.
     */
    EGS_Float time();

private:

    EGS_PrivateTimer *p;             //!< Used to hide implementation details.
    EGS_Timer(const EGS_Timer &) {}; //!< Prevent copy construction.

};

#endif
