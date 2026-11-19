/*
###############################################################################
#
#  EGSnrc egs++ egs_al_eq_ct_source locate function
#  Copyright (C) 2026 Marie-Luise Kuhlmann
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
#  Author:          Marie-Luise Kuhlmann, 2021
#
#
###############################################################################
*/

#include <iostream>
#include <stdlib.h>
#include "egs_functions.h"

int locate(EGS_Float *xx, int n, const EGS_Float x0) {

    int ju, jm, jl, jsav;
    bool ascnd=(xx[n-1] >= xx[0]);
    jl=0;
    ju=n-1;
    while (ju-jl > 1) {
        jm = (ju+jl) >> 1;
        if (x0 >= xx[jm] == ascnd) {
            jl=jm;
        }
        else {
            ju=jm;
        }
    }
    jsav = jl;
    return jsav;
}
