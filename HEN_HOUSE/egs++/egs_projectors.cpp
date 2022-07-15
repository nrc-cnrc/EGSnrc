/*
###############################################################################
#
#  EGSnrc egs++ projectors
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


/*! \file egs_projectors.cpp
 *  \brief EGS_Projector implementation
 *  \IK
 */

#include "egs_projectors.h"
#include "egs_functions.h"

EGS_Projector::EGS_Projector(const EGS_Vector &A, const string &Type) :
    a(A), xo(EGS_Vector()), d(0), type(Type) {
    norm = a.length();
    a.normalize();
    if (a.x*a.x + a.y*a.y > 0) {
        v1 = EGS_Vector(-a.y,a.x,0);
        v1.normalize();
    }
    else {
        v1 = EGS_Vector(1,0,0);
    }
    v2 = a%v1;
}

EGS_Projector::EGS_Projector(const EGS_Vector &x1, const EGS_Vector &x2,
                             const EGS_Vector &x3, const string &T) : type(T) {
    v1 = x2-x1;
    v2 = x3-x1;
    a = v1%v2;
    xo = x1;
    if (a.length2() < epsilon) egsFatal("EGS_Projector::EGS_Projector: "
                                            " the vectors are co-linear\n");
    a.normalize();
    norm = 1;
    v1.normalize();
    v2 = a%v1;
    d = a*x1;
}

void EGS_Projector::printInfo() const {
    egsInformation(" normal = (%g,%g,%g)\n",a.x,a.y,a.z);
};
