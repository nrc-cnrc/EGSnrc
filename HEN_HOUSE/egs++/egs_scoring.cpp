/*
###############################################################################
#
#  EGSnrc egs++ scoring
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
#  Contributors:    Berit Behnke
#                   Reid Townson
#
###############################################################################
*/


/*! \file egs_scoring.cpp
 *  \brief EGS_ScoringSingle and EGS_ScoringArray implementation
 *  \IK
 */

#include "egs_scoring.h"
#include "egs_functions.h"

#include <string>
using std::string;

EGS_ScoringArray::EGS_ScoringArray(int N) :
    current_ncase(0), current_ncase_65536(0), current_ncase_short(0) {
    if (N <= 0) egsFatal("EGS_ScoringArray::EGS_ScoringArray:\n"
                             "   attempt to construct a scoring array with non-positive size\n");
    result = new EGS_ScoringSingle [N];
    nreg = N;
}

EGS_ScoringArray::~EGS_ScoringArray() {
    delete [] result;
}

void EGS_ScoringArray::setHistory(EGS_I64 ncase) {
    if (ncase != current_ncase) {
        current_ncase = ncase;
        EGS_I64 aux = ncase >> 16;
        if (aux != current_ncase_65536) {
            current_ncase_65536 = aux;
            for (int j=0; j<nreg; j++) {
                result[j].finishCase(0,0);
            }
        }
        aux = ncase - (aux << 16);
        unsigned short aux1 = (unsigned short) aux;
        current_ncase_short = aux1;
    }
}

void EGS_ScoringArray::reportResults(double norm, const char *title,
                                     bool relative_error, const char *format) {
    if (title) egsInformation("\n\n%s for %lli particles:\n\n",title,
                                  current_ncase);
    if (current_ncase < 2) {
        egsWarning("EGS_ScoringArray::reportResults: you must run more than 2 "
                   "histories\n\n");
        return;
    }
    char c = (relative_error) ? '%' : ' ';
    string myformat = "  %d    %g  +/-  %g %c\n";
    const char *oformat = format ? format : myformat.c_str();
    for (int j=0; j<nreg; j++) {
        double r,dr;
        result[j].currentResult(current_ncase,r,dr);
        if (relative_error) {
            dr = (r > 0) ? 100*dr/r : 100;
        }
        else {
            dr *= norm;
        }
        egsInformation(oformat,j,r*norm,dr,c);
    }
}
