/*
###############################################################################
#
#  EGSnrc egs++ mesh geometry library implementation.
#
#  Copyright (C) 2019 Mevex Corporation
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
#  Authors:          Dave Macrillo,
#                    Matt Ronan,
#                    Nigel Vezeau,
#                    Lou Thompson,
#                    Max Orok
#
###############################################################################
*/

/*! \file egs_Mevex_tet_collection.cpp
 *  \brief A mevex tet collection geometry: implementation
 *  \MJR
 */

#include "egs_mesh.h"
#include "egs_input.h"
#include <sstream>
#include <fstream>
#include <iostream>

void EGS_Mevex_tet_collection::printInfo() const {
    EGS_BaseGeometry::printInfo();
    egsInformation(" nTets = %i\n",nreg);
    egsInformation("=======================================================\n");
    // for (int i = 0; i < nreg; ++i){
      auto i = 0;
      egsInformation("Tet %i: %g %g %g %g %g %g %g %g %g %g %g %g %i %i %i %i %i %i\n", (i+1), aN1[i].x, aN1[i].y, aN1[i].z, aN2[i].x, aN2[i].y, aN2[i].z, aN3[i].x, aN3[i].y, aN3[i].z, aN4[i].x, aN4[i].y, aN4[i].z, n1[i], n2[i], n3[i], n4[i], BoundaryTet[i], mediaIndices[i]);
    // }
    // egsInformation("Tet2: %g %g %g %g %g %g %g %g %g %g %g %g %i %i %i %i %i %i\n",aN1[1].x, aN1[1].y, aN1[1].z, aN2[1].x, aN2[1].y, aN2[1].z, aN3[1].x, aN3[1].y, aN3[1].z, aN4[1].x, aN4[1].y, aN4[1].z, n1[1], n2[1], n3[1], n4[1], BoundaryTet[1], mediaIndices[1]);
}

// string EGS_Mevex_tet_collection::type("EGS_Mevex_tet_collection");

static char EGS_MEVEX_TET_COLLECTION_LOCAL ebox_message1[] =
"createGeometry(Mevex_tet_collection):\n";
static char EGS_MEVEX_TET_COLLECTION_LOCAL ebox_message2[] = "null input?";
static char EGS_MEVEX_TET_COLLECTION_LOCAL ebox_message3[] =
"wrong/missing input?";
static char EGS_MEVEX_TET_COLLECTION_LOCAL ebox_message4[] =
    "expecting 1 int input for 'nTets'";
static char EGS_MEVEX_TET_COLLECTION_LOCAL ebox_message5[] =
    "Could not open tet data file %s Check path to file. ";
static char EGS_MEVEX_TET_COLLECTION_LOCAL ebox_key1[] = "nTets";
static char EGS_MEVEX_TET_COLLECTION_LOCAL ebox_key2[] = "tet data file";

extern "C" {

    // EGS_MEVEX_TET_COLLECTION_EXPORT
EGS_MEVEX_TET_COLLECTION_EXPORT
EGS_BaseGeometry *createGeometry(EGS_Input *input) {
        if (!input) {
            egsWarning(ebox_message1,ebox_message2);
            return 0;
        }
        int nT;
        vector<int> NT;
        int err = input->getInput(ebox_key1,NT);
        if (err) {
            egsWarning(ebox_message1,ebox_message3);
            return 0;
        }
        EGS_Float *X1;
        EGS_Float *Y1;
        EGS_Float *Z1;
        EGS_Float *X2;
        EGS_Float *Y2;
        EGS_Float *Z2;
        EGS_Float *X3;
        EGS_Float *Y3;
        EGS_Float *Z3;
        EGS_Float *X4;
        EGS_Float *Y4;
        EGS_Float *Z4;
        int *N1;
        int *N2;
        int *N3;
        int *N4;
        bool *bT;
        int *mediaIndexes;

        if (NT.size() == 1) {
            nT = NT[0];

            X1 = new EGS_Float [nT];
            Y1 = new EGS_Float [nT];
            Z1 = new EGS_Float [nT];
            X2 = new EGS_Float [nT];
            Y2 = new EGS_Float [nT];
            Z2 = new EGS_Float [nT];
            X3 = new EGS_Float [nT];
            Y3 = new EGS_Float [nT];
            Z3 = new EGS_Float [nT];
            X4 = new EGS_Float [nT];
            Y4 = new EGS_Float [nT];
            Z4 = new EGS_Float [nT];
            N1 = new int [nT];
            N2 = new int [nT];
            N3 = new int [nT];
            N4 = new int [nT];
            bT = new bool [nT];
            mediaIndexes = new int [nT];

        }
        else {
            egsWarning(ebox_message1,ebox_message4);
            return 0;
        }

        string fname;
        err = input->getInput(ebox_key2,fname);
        if (err) {
            egsWarning(ebox_message1, ebox_message3);
            return 0;
        }

        const char * fnameC = fname.c_str();
        ifstream fs;
        fs.open(fnameC, ifstream::in);
        if(!fs.is_open()){
            egsWarning(ebox_message5,fnameC);
            return 0;
        }
        else{
            char str[1000];
            istringstream iss;
            for (int i=0; i<nT;i++) {
                fs.getline(str,1000);
                iss.seekg(0);
                iss.str(str);
                iss >> X1[i];
                iss >> Y1[i];
                iss >> Z1[i];
                iss >> X2[i];
                iss >> Y2[i];
                iss >> Z2[i];
                iss >> X3[i];
                iss >> Y3[i];
                iss >> Z3[i];
                iss >> X4[i];
                iss >> Y4[i];
                iss >> Z4[i];
                iss >> N1[i];
                iss >> N2[i];
                iss >> N3[i];
                iss >> N4[i];
                iss >> bT[i];
                iss >> mediaIndexes[i];
            }
            fs.close();
        }

        EGS_Mevex_tet_collection *result;
        result = new EGS_Mevex_tet_collection(nT, X1, Y1, Z1, X2,
          Y2, Z2, X3, Y3, Z3, X4, Y4, Z4, N1, N2, N3, N4, bT, mediaIndexes);
        result->setName(input);
        result->setBoundaryTolerance(input);
        result->setTetMedia(input, mediaIndexes);
        result->setLabels(input);
        return result;
    }
}
