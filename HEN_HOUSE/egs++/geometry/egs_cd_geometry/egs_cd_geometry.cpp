/*
###############################################################################
#
#  EGSnrc egs++ cd geometry
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
#  Contributors:    Frederic Tessier
#                   Ernesto Mainegra-Hing
#                   Marc Chamberland
#
###############################################################################
*/


/*! \file egs_cd_geometry.cpp
 *  \brief A CD-Geometry: implementation
 *  \IK
 */

#include "egs_cd_geometry.h"
#include "egs_input.h"

#ifdef NO_SSTREAM
    #include <strstream>
    #define S_STREAM std::istrstream
#else
    #include <sstream>
    #define S_STREAM std::istringstream
#endif

string EGS_CDGeometry::type = "EGS_CDGeometry";

void EGS_CDGeometry::setMedia(EGS_Input *,int,const int *) {
    egsWarning("EGS_CDGeometry::setMedia: don't use this method. Use the\n"
               " setMedia() methods of the geometry objects that make up this geometry\n");
}

void EGS_CDGeometry::setRelativeRho(int start, int end, EGS_Float rho) {
    egsWarning("EGS_CDGeometry::setRelativeRho(): don't use this method\n"
               "  Use the setRelativeRho() methods of the geometry objects that make"
               " up this geometry\n");
}

void EGS_CDGeometry::setRelativeRho(EGS_Input *) {
    egsWarning("EGS_CDGeometry::setRelativeRho(): don't use this method\n"
               "  Use the setRelativeRho() methods of the geometry objects that make"
               " up this geometry\n");
}

void EGS_CDGeometry::setBScaling(int start, int end, EGS_Float bf) {
    egsWarning("EGS_CDGeometry::setBScaling(): don't use this method\n"
               "  Use the setBScaling() methods of the geometry objects that make"
               " up this geometry\n");
}

void EGS_CDGeometry::setBScaling(EGS_Input *) {
    egsWarning("EGS_CDGeometry::setBScaling(): don't use this method\n"
               "  Use the setBScaling() methods of the geometry objects that make"
               " up this geometry\n");
}

void EGS_CDGeometry::setUpIndexing() {
    int nr = 0;
    int j;
    for (j=0; j<nbase; j++) {
        if (g[j]) {
            nr += g[j]->regions();
        }
        else {
            ++nr;
        }
    }
    reg_to_base = new int [nr];
    local_start = new int [nbase];
    int ir=0;
    for (j=0; j<nbase; j++) {
        local_start[j] = ir;
        if (g[j]) {
            int n = g[j]->regions();
            for (int i=0; i<n; i++) {
                reg_to_base[ir++] = j;
            }
        }
        else {
            reg_to_base[ir++] = j;
        }
    }
    new_indexing = true;
    nreg = nr;
}


extern "C" {

    EGS_CDGEOMETRY_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {
        if (!input) {
            egsWarning("createGeometry(CD_Geometry): null input?\n");
            return 0;
        }
        EGS_Input *ij;
        while ((ij = input->takeInputItem("geometry",false)) != 0) {
            EGS_BaseGeometry::createSingleGeometry(ij);
            delete ij;
        }
        string bg_name;
        int err = input->getInput("base geometry",bg_name);
        if (err) {
            egsWarning("createGeometry(CD_Geometry): no 'base geometry' input\n");
            return 0;
        }
        EGS_BaseGeometry *g = EGS_BaseGeometry::getGeometry(bg_name);
        if (!g) {
            egsWarning("createGeometry(CD_Geometry): no geometry named %s is"
                       " defined\n",bg_name.c_str());
            return 0;
        }
        int nreg = g->regions();
        if (nreg < 1) {
            egsWarning("createGeometry(CD_Geometry): the base geometry has %d"
                       " regions?\n",nreg);
            return 0;
        }
        EGS_BaseGeometry **G = new EGS_BaseGeometry* [nreg];
        int j;
        for (j=0; j<nreg; j++) {
            G[j] = 0;
        }
        int ng = 0;
        string aux;
        err = 0;
        while ((ij = input->takeInputItem("set geometry")) != 0) {
            vector<string> aux;
            ij->getInput("set geometry",aux);
            int istart, iend;
            string name;
            bool is_ok = true;
            if (aux.size() == 2) {
                string auxx = aux[0];
                auxx += ' ';
                auxx += aux[1];
                auxx += ' ';
                S_STREAM in(auxx.c_str());
                in >> istart >> name;
                if (in.fail() || !in.good()) {
                    egsWarning("createGeometry(CD_Geometry): parse error in\n"
                               "  set geometry = %s\n",auxx.c_str());
                    err++;
                    is_ok = false;
                }
                else {
                    if (istart < 0 || istart > nreg-1) {
                        istart = -1;
                        iend = -2;
                    }
                    else {
                        iend = istart+1;
                    }
                }
            }
            else if (aux.size() == 3) {
                string auxx = aux[0];
                auxx += ' ';
                auxx += aux[1];
                auxx += ' ';
                auxx += aux[2];
                auxx += ' ';
                S_STREAM in(auxx.c_str());
                in >> istart >> iend  >> name;
                if (in.fail() || !in.good()) {
                    egsWarning("createGeometry(CD_Geometry): parse error in\n"
                               "  set geometry = %s\n",auxx.c_str());
                    err++;
                    is_ok = false;
                }
                else {
                    if (istart < 0) {
                        istart = 0;
                    }
                    if (iend > nreg) {
                        iend = nreg;
                    }
                }
            }
            else {
                err++;
                is_ok = false;
            }
            if (is_ok) {
                EGS_BaseGeometry *gj = EGS_BaseGeometry::getGeometry(name);
                if (!gj) {
                    egsWarning("createGeometry(CD_Geometry): no geometry named %s"
                               " is defined\n",name.c_str());
                    err++;
                }
                else {
                    for (int j=istart; j<iend; j++) {
                        if (G[j]  && G[j] != gj) {
                            G[j]->deref();
                        }
                        /*
                        if( !G[j] ) gj->ref();
                        else {
                            if( G[j] != gj ) G[j]->deref();
                        }
                        */
                        G[j] = gj;
                        gj->ref();
                        ng++;
                    }
                }
            }
            delete ij;
        }
        if (err) {
            egsWarning("createGeometry(CD_Geometry): %d errors\n",err);
            for (j=0; j<nreg; j++) {
                if (G[j]) {
                    if (!G[j]->deref()) {
                        delete G[j];
                    }
                }
            }
            delete [] G[j];
            return 0;
        }
        if (!ng)
            egsWarning("createGeometry(CD_Geometry): no geometries in addition to"
                       " the base geometry defined?\n"
                       "  Hope you know what you are doing\n");
        g->ref();
        int indexing = 0;
        input->getInput("new indexing style",indexing);
        EGS_BaseGeometry *result = new EGS_CDGeometry(g,G,"",indexing);
        delete [] G;
        result->setName(input);
        result->setBoundaryTolerance(input);
        result->setLabels(input);
        return result;

    }


    void EGS_CDGeometry::getLabelRegions(const string &str, vector<int> &regs) {

        // label defined in the base geometry
        vector<int> bgregs;
        bg->getLabelRegions(str, bgregs);

        // expand base regions to global region lists
        int rstart = 0;
        int rend   = 0;
        for (int i=0; i<bgregs.size(); i++) {
            if (new_indexing) {
                rstart = local_start[bgregs[i]];
                if (bgregs[i] < nbase-1) {
                    rend = local_start[bgregs[i]+1];
                }
                else {
                    rend = nreg;
                }
            }
            else {
                rstart = bgregs[i]*nmax;
                rend   = (bgregs[i]+1)*nmax;
            }
            for (int j=rstart; j<rend; j++) {
                regs.push_back(j);
            }
        }

        // label defined in the set geometries
        vector<int> gregs;
        int shift=0;
        for (int i=0; i<nbase; i++) {

            // add regions from set geometries
            gregs.clear();
            if (g[i]) {
                g[i]->getLabelRegions(str, gregs);
            }

            // shift region numbers according to indexing style
            if (new_indexing) {
                shift = local_start[i];
            }
            else {
                shift = i*nmax;
            }
            for (int j=0; j<gregs.size(); j++) {
                gregs[j] += shift;
            }

            // add regions to the list
            regs.insert(regs.end(), gregs.begin(), gregs.end());

        }

        // label defined in self (cd input block)
        EGS_BaseGeometry::getLabelRegions(str, regs);

    }

}
