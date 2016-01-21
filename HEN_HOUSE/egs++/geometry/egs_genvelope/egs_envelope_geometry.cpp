/*
###############################################################################
#
#  EGSnrc egs++ envelope geometry
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
#
###############################################################################
*/


/*! \file egs_envelope_geometry.cpp
 *  \brief An envelope geometry: implementation
 *  \IK
 */

#include "egs_envelope_geometry.h"
#include "egs_input.h"
#include "egs_functions.h"

#include <cstdlib>

using namespace std;

string EGS_ENVELOPEG_LOCAL EGS_EnvelopeGeometry::type = "EGS_EnvelopeGeometry";
string EGS_ENVELOPEG_LOCAL EGS_FastEnvelope::type = "EGS_FastEnvelope";

void EGS_EnvelopeGeometry::setMedia(EGS_Input *,int,const int *) {
    egsWarning("EGS_EnvelopeGeometry::setMedia: don't use this method. Use the\n"
               " setMedia() methods of the geometry objects that make up this geometry\n");
}

void EGS_EnvelopeGeometry::setRelativeRho(int start, int end, EGS_Float rho) {
    setRelativeRho(0);
}

void EGS_EnvelopeGeometry::setRelativeRho(EGS_Input *) {
    egsWarning("EGS_EnvelopeGeometry::setRelativeRho(): don't use this method."
               " Use the\n setRelativeRho methods of the geometry objects that make up"
               " this geometry\n");
}

void EGS_FastEnvelope::setMedia(EGS_Input *,int,const int *) {
    egsWarning("EGS_FastEnvelope::setMedia: don't use this method. Use the\n"
               " setMedia() methods of the geometry objects that make up this geometry\n");
}

void EGS_FastEnvelope::setRelativeRho(int start, int end, EGS_Float rho) {
    setRelativeRho(0);
}

void EGS_FastEnvelope::setRelativeRho(EGS_Input *) {
    egsWarning("EGS_FastEnvelope::setRelativeRho(): don't use this method."
               " Use the\n setRelativeRho methods of the geometry objects that make up"
               " this geometry\n");
}

struct EGS_ENVELOPEG_LOCAL EnvelopeAux {
    EGS_BaseGeometry *g;
    int              nreg;
    int              *regs;
    EnvelopeAux() : nreg(0) {};
    ~EnvelopeAux() {
        if (nreg>0) {
            delete [] regs;
        }
    };
};

EGS_EnvelopeGeometry::EGS_EnvelopeGeometry(EGS_BaseGeometry *G,
        const vector<EGS_BaseGeometry *> &geoms, const string &Name,
        bool newindexing) :
    EGS_BaseGeometry(Name), reg_to_inscr(0), local_start(0) {
    if (!G) {
        egsFatal("EGS_EnvelopeGeometry: base geometry must not be null\n");
    }
    g = G;
    g->ref();
    new_indexing = false;
    nbase = g->regions();
    n_in = geoms.size();
    nmax = 1;
    int nreg_inscribed = 0;
    if (n_in > 0) {
        geometries = new EGS_BaseGeometry * [n_in];
        for (int j=0; j<n_in; j++) {
            geometries[j] = geoms[j];
            geometries[j]->ref();
            int nj = geometries[j]->regions();
            if (nj > nmax) {
                nmax = nj;
            }
            nreg_inscribed += nj;
        }
    }
    nreg = nbase + n_in*nmax;
    is_convex = g->isConvex();
    has_rho_scaling = g->hasRhoScaling();
    if (!has_rho_scaling) {
        for (int j=0; j<n_in; j++) {
            if (geometries[j]->hasRhoScaling()) {
                has_rho_scaling = true;
                break;
            }
        }
    }
    if (!n_in) {
        return;
    }
    if (newindexing) {
        new_indexing = true;
        local_start = new int [n_in];
        reg_to_inscr = new int [nreg_inscribed];
        int ir=0;
        for (int j=0; j<n_in; j++) {
            int n = geometries[j]->regions();
            local_start[j] = nbase + ir;
            for (int i=0; i<n; i++) {
                reg_to_inscr[ir++] = j;
            }
        }
        nreg = nbase + ir;
    }
}

EGS_FastEnvelope::EGS_FastEnvelope(EGS_BaseGeometry *G,
                                   const vector<EnvelopeAux *> &fgeoms, const string &Name,
                                   int newindexing) :
    EGS_BaseGeometry(Name), reg_to_inscr(0), local_start(0) {
    if (!G) {
        egsFatal("EGS_FastEnvelope: base geometry must not be null\n");
    }
    g = G;
    g->ref();
    n_in = fgeoms.size();
    nmax = 1;
    new_indexing = false;
    if (!n_in) {
        egsFatal("EGS_FastEnvelope: no inscribed geometries!\n");
    }
    geometries = new EGS_BaseGeometry * [n_in];
    nbase = g->regions();
    int *iaux = new int [nbase];
    int j;
    for (j=0; j<nbase; j++) {
        iaux[j] = 0;
    }
    int nlist = 0;
    int nreg_inscribed = 0;
    for (j=0; j<fgeoms.size(); j++) {
        geometries[j] = fgeoms[j]->g;
        geometries[j]->ref();
        int nj = geometries[j]->regions();
        nreg_inscribed += nj;
        if (nj > nmax) {
            nmax = nj;
        }
        for (int i=0; i<fgeoms[j]->nreg; i++) {
            int k = fgeoms[j]->regs[i];
            if (k >= 0 && k < nbase) {
                ++iaux[k];
                ++nlist;
            }
        }
    }
    n_start = new int [nbase+1];
    glist = new int [nlist];
    int ilist=0;
    for (j=0; j<nbase; j++) {
        n_start[j] = ilist;
        if (iaux[j] > 0) {
            for (int l=0; l<fgeoms.size(); l++) {
                for (int i=0; i<fgeoms[l]->nreg; i++) {
                    int k = fgeoms[l]->regs[i];
                    if (k == j) {
                        glist[ilist++] = l;
                    }
                }
            }
        }
    }
    n_start[nbase] = ilist;
    nreg = nbase + n_in*nmax;
    is_convex = g->isConvex();
    has_rho_scaling = g->hasRhoScaling();
    if (!has_rho_scaling) {
        for (int j=0; j<n_in; j++) {
            if (geometries[j]->hasRhoScaling()) {
                has_rho_scaling = true;
                break;
            }
        }
    }
    delete [] iaux;
    if (newindexing) {
        new_indexing = true;
        local_start = new int [n_in];
        reg_to_inscr = new int [nreg_inscribed];
        int ir=0;
        for (int j=0; j<n_in; j++) {
            int n = geometries[j]->regions();
            local_start[j] = nbase + ir;
            for (int i=0; i<n; i++) {
                reg_to_inscr[ir++] = j;
            }
        }
        nreg = nbase + ir;
    }
}

EGS_EnvelopeGeometry::~EGS_EnvelopeGeometry() {
    if (!g->deref()) {
        delete g;
    }
    for (int j=0; j<n_in; j++) {
        if (!geometries[j]->deref()) {
            delete geometries[j];
        }
    }
    delete [] geometries;
    if (new_indexing) {
        if (local_start) {
            delete [] local_start;
        }
        if (reg_to_inscr) {
            delete [] reg_to_inscr;
        }
    }
}

EGS_FastEnvelope::~EGS_FastEnvelope() {
    if (!g->deref()) {
        delete g;
    }
    for (int j=0; j<n_in; j++) {
        if (!geometries[j]->deref()) {
            delete geometries[j];
        }
    }
    delete [] geometries;
    delete [] n_start;
    delete [] glist;
    if (new_indexing) {
        if (local_start) {
            delete [] local_start;
        }
        if (reg_to_inscr) {
            delete [] reg_to_inscr;
        }
    }
}

void EGS_EnvelopeGeometry::printInfo() const {
    EGS_BaseGeometry::printInfo();
    egsInformation(" base geometry = %s (type %s)\n",g->getName().c_str(),
                   g->getType().c_str());
    egsInformation(" inscribed geometries:\n");
    for (int j=0; j<n_in; j++) egsInformation("   %s (type %s)\n",
                geometries[j]->getName().c_str(),geometries[j]->getType().c_str());
    egsInformation(
        "=======================================================\n");
}

void EGS_FastEnvelope::printInfo() const {
    EGS_BaseGeometry::printInfo();
    egsInformation(" base geometry = %s (type %s)\n",g->getName().c_str(),
                   g->getType().c_str());
    egsInformation(" inscribed geometries:\n");
    for (int j=0; j<n_in; j++) egsInformation("   %s (type %s)\n",
                geometries[j]->getName().c_str(),geometries[j]->getType().c_str());
    egsInformation(
        "=======================================================\n");
}


static char EGS_ENVELOPEG_LOCAL eeg_message1[] =
    "createGeometry(envelope geometry): %s\n";
static char EGS_ENVELOPEG_LOCAL eeg_message2[] =
    "null input?";
static char EGS_ENVELOPEG_LOCAL eeg_message3[] =
    "no 'base geometry' input?";
static char EGS_ENVELOPEG_LOCAL eeg_message4[] =
    "incorrect base geometry definition";
static char EGS_ENVELOPEG_LOCAL eeg_message5[] =
    "missing/incorrect 'base geometry' input";
static char EGS_ENVELOPEG_LOCAL eeg_message6[] =
    "createGeometry(envelope geometry): no geometry with name %s defined\n";
static char EGS_ENVELOPEG_LOCAL eeg_message7[] =
    "no inscirebed geometries defined?. I hope you know what you are doing";
static char EGS_ENVELOPEG_LOCAL eeg_message8[] =
    "an error occured while constructing inscibed geometries";

static char EGS_ENVELOPEG_LOCAL eeg_keyword1[] = "base geometry";
static char EGS_ENVELOPEG_LOCAL eeg_keyword2[] = "geometry";
static char EGS_ENVELOPEG_LOCAL eeg_keyword3[] = "inscribed geometries";

extern "C" {

    EGS_ENVELOPEG_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {
        if (!input) {
            egsWarning(eeg_message1,eeg_message2);
            return 0;
        }
        //
        // *** new indexing style
        //
        int indexing = 0;
        input->getInput("new indexing style",indexing);
        //
        // *** Base geometry
        //
        EGS_Input *i = input->takeInputItem(eeg_keyword1);
        if (!i) {
            egsWarning(eeg_message1,eeg_message3);
            return 0;
        }
        EGS_Input *ig = i->takeInputItem(eeg_keyword2);
        EGS_BaseGeometry *g;
        if (ig) {  // defined inline
            g = EGS_BaseGeometry::createSingleGeometry(ig);
            delete ig;
            if (!g) {
                egsWarning(eeg_message1,eeg_message4);
                delete i;
                return 0;
            }
        }
        else {  // defined via a name of a previously defined geometry
            string bgname;
            int err = i->getInput(eeg_keyword1,bgname);
            delete i;
            if (err) {
                egsWarning(eeg_message1,eeg_message5);
                return 0;
            }
            g = EGS_BaseGeometry::getGeometry(bgname);
            if (!g) {
                egsWarning(eeg_message6,bgname.c_str());
                return 0;
            }
        }
        vector<EnvelopeAux *> fgeoms;
        EGS_Input *ix;
        while ((ix = input->takeInputItem("inscribe in regions")) != 0) {
            vector<string> values;
            ix->getInput("inscribe in regions",values);
            if (values.size() < 2) egsWarning("createGeometry(envelope geometry):"
                                                  " %d inputs for 'inscribe in regions'? 2 or more are needed\n",values.size());
            else {
                EGS_BaseGeometry *gj = EGS_BaseGeometry::getGeometry(values[0]);
                if (!gj) {
                    egsWarning(eeg_message6,values[0].c_str());
                }
                else {
                    EnvelopeAux *aux = new EnvelopeAux;
                    aux->g = gj;
                    aux->nreg = values.size()-1;
                    aux->regs = new int [aux->nreg];
                    for (int j=0; j<aux->nreg; j++) {
                        aux->regs[j] = atoi(values[j+1].c_str());
                    }
                    fgeoms.push_back(aux);
                }
            }
            delete ix;
        }
        if (fgeoms.size() > 0) {
            EGS_BaseGeometry *result = new EGS_FastEnvelope(g,fgeoms,"",indexing);
            result->setName(input);
            for (int j=0; j<fgeoms.size(); j++) {
                delete fgeoms[j];
            }
            return result;
        }

        //
        // *** Inscribed geometries
        //
        i = input->takeInputItem(eeg_keyword3);
        vector<EGS_BaseGeometry *> geoms;
        if (!i) {
            if (fgeoms.size()) {
                egsWarning(eeg_message1,eeg_message7);
            }
        }
        else {
            // first try for inscribed geometries defined inline
            while ((ig = i->takeInputItem(eeg_keyword2)) != 0) {
                EGS_BaseGeometry *gj = EGS_BaseGeometry::createSingleGeometry(ig);
                delete ig;
                if (!gj) {
                    egsWarning(eeg_message1,eeg_message8);
                }
                else {
                    geoms.push_back(gj);
                }
            }
            // if geoms.size() is 0, check if inscribed geometries are defined
            // via names of previously defined geometries.
            if (!geoms.size()) {
                vector<string> igeoms;
                int err = i->getInput(eeg_keyword3,igeoms);
                if (err || !igeoms.size()) {
                    egsWarning(eeg_message1,eeg_message7);
                }
                else {
                    for (unsigned int j=0; j<igeoms.size(); j++) {
                        EGS_BaseGeometry *gj =
                            EGS_BaseGeometry::getGeometry(igeoms[j]);
                        if (!gj) {
                            egsWarning(eeg_message6,igeoms[j].c_str());
                        }
                        else {
                            geoms.push_back(gj);
                        }
                    }
                }
            }
            delete i;
        }
        /*
        EGS_BaseGeometry *result = fgeoms.size() ?
            new EGS_EnvelopeGeometry(g,fgeoms,geoms) :
            new EGS_EnvelopeGeometry(g,geoms);
            */
        EGS_BaseGeometry *result = new EGS_EnvelopeGeometry(g,geoms,"",indexing);
        result->setName(input);
        result->setLabels(input);
        return result;

    }


    void EGS_EnvelopeGeometry::getLabelRegions(const string &str, vector<int> &regs) {

        // label defined in the envelope geometry
        g->getLabelRegions(str, regs);

        // label defined in the inscribed geometries
        vector<int> gregs;
        int shift=0;
        for (int i=0; i<n_in; i++) {

            // add regions from set geometries
            gregs.clear();
            if (geometries[i]) {
                geometries[i]->getLabelRegions(str, gregs);
            }

            // shift region numbers according to indexing style
            if (new_indexing) {
                shift = local_start[i];
            }
            else {
                shift = nbase+i*nmax;
            }
            for (int j=0; j<gregs.size(); j++) {
                gregs[j] += shift;
            }

            // add regions to the list
            regs.insert(regs.end(), gregs.begin(), gregs.end());

        }

        // label defined in self (envelope geometry input block)
        EGS_BaseGeometry::getLabelRegions(str, regs);

    }

    void EGS_FastEnvelope::getLabelRegions(const string &str, vector<int> &regs) {

        // label defined in the envelope geometry
        g->getLabelRegions(str, regs);

        // label defined in the inscribed geometries
        vector<int> gregs;
        int shift=0;
        for (int i=0; i<n_in; i++) {

            // add regions from set geometries
            gregs.clear();
            if (geometries[i]) {
                geometries[i]->getLabelRegions(str, gregs);
            }

            // shift region numbers according to indexing style
            if (new_indexing) {
                shift = local_start[i];
            }
            else {
                shift = nmax;
            }
            for (int j=0; j<gregs.size(); j++) {
                gregs[j] += shift;
            }

            // add regions to the list
            regs.insert(regs.end(), gregs.begin(), gregs.end());

        }

        // label defined in self (envelope geometry input block)
        EGS_BaseGeometry::getLabelRegions(str, regs);

    }

}
