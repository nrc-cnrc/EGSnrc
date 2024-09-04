/*
###############################################################################
#
#  EGSnrc egs++ smart envelope geometry
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
#  Contributors:    Frederic Tessier
#                   Ernesto Mainegra-Hing
#                   Marc Chamberland
#
###############################################################################
#
#  A "smart" envelope geometry. The smartness is due to the fact that there
#  can be only zero or one inscribed geometry per base geometry region, which
#  makes many of the checks faster.
#
#  In addition, unlike the regular envelope geometry where all inscribed
#  geometries always must completely fit inside the base geometry, here
#  geometries can be inscribed using logic 0 or 1.
#
#  Logic 0 is as before, i.e., geometry must completely fit into the region
#  where it is being inscribed.
#
#  Logic 1 means that the inscribed geometry extends beyond the region and
#  therefore the smart envelope also checks the base geometry in this case.
#  This is very similar to a CD geometry except that now the space outside
#  the inscribed geometry is still part of the geometry.
#
#  Warning: not completely tested, so don't use for production runs yet.
#
###############################################################################
*/


/*! \file egs_smart_envelope.cpp
 *  \brief A smart envelope: implementation
 *  \IK
 */

#include "egs_smart_envelope.h"
#include "egs_input.h"
#include "egs_functions.h"

#include <cstdlib>

using namespace std;

string EGS_SMART_ENVELOPE_LOCAL EGS_SmartEnvelope::type = "EGS_SmartEnvelope";

void EGS_SmartEnvelope::setMedia(EGS_Input *,int,const int *) {
    egsWarning("EGS_SmartEnvelope::setMedia: don't use this method. Use the\n"
               " setMedia() methods of the geometry objects that make up this geometry\n");
}

void EGS_SmartEnvelope::setRelativeRho(int start, int end, EGS_Float rho) {
    setRelativeRho(0);
}

void EGS_SmartEnvelope::setRelativeRho(EGS_Input *) {
    egsWarning("EGS_SmartEnvelope::setRelativeRho(): don't use this method."
               " Use the\n setRelativeRho methods of the geometry objects that make up"
               " this geometry\n");
}

void EGS_SmartEnvelope::setBScaling(int start, int end, EGS_Float bf) {
    setBScaling(0);
}

void EGS_SmartEnvelope::setBScaling(EGS_Input *) {
    egsWarning("EGS_SmartEnvelope::setsetBScaling(): don't use this method."
               " Use the\n setsetBScaling methods of the geometry objects that make up"
               " this geometry\n");
}

struct EGS_SMART_ENVELOPE_LOCAL SmartEnvelopeAux {
    EGS_BaseGeometry *g;
    int              ireg;
    int              type;
    int              nreg;
    int              *regs;
    SmartEnvelopeAux() : g(0), ireg(-1), type(0), nreg(0) {};
    ~SmartEnvelopeAux() {
        if (nreg>0) {
            delete [] regs;
        }
    };
};

EGS_SmartEnvelope::EGS_SmartEnvelope(EGS_BaseGeometry *G,
                                     const vector<SmartEnvelopeAux *> &fgeoms, const string &Name) :
    EGS_BaseGeometry(Name), geometries(0), gindex(0),
    reg_to_inscr(0), reg_to_base(0), local_start(0), itype(0) {
    if (!G) {
        egsFatal("EGS_SmartEnvelope: base geometry must not be null\n");
    }
    g = G;
    g->ref();
    n_in = fgeoms.size();
    if (!n_in) {
        egsFatal("EGS_SmartEnvelope: no inscribed geometries!\n");
    }
    geometries = new EGS_BaseGeometry * [n_in];
    nbase = g->regions();
    gindex = new int [nbase];
    itype = new char [fgeoms.size()];
    int j;
    for (j=0; j<nbase; j++) {
        gindex[j] = -1;
    }
    for (j=0; j<fgeoms.size(); j++) {
        itype[j] = 0;
    }
    int nreg_inscribed = 0;
    bool ok = true;
    for (j=0; j<fgeoms.size(); j++) {
        geometries[j] = fgeoms[j]->g;
        geometries[j]->ref();
        int i = fgeoms[j]->ireg;
        if (gindex[i] >= 0) {
            egsWarning("EGS_SmartEnvelope:"
                       " There can only be a single geometry inscribed in a region\n");
            egsWarning(" You are trying to inscribe %s into region %d but\n",
                       geometries[j]->getName().c_str(),i);
            egsWarning(" geometry %s is already inscribed in this region\n",
                       geometries[gindex[i]]->getName().c_str());
            ok = false;
        }
        else {
            gindex[i] = j;
            itype[j] = fgeoms[j]->type;
            //egsInformation("inscribing %d into %d with type %d\n",j,i,itype[j]);
            nreg_inscribed += geometries[j]->regions();
        }
    }
    if (!ok) {
        egsFatal("EGS_SmartEnvelope: errors during definition\n");
    }
    nreg = nbase + nreg_inscribed;
    local_start = new int [fgeoms.size()];
    reg_to_inscr = new int [nreg_inscribed];
    reg_to_base = new int [nreg_inscribed];
    int nr = 0;
    for (j=0; j<fgeoms.size(); j++) {
        local_start[j] = nbase + nr;
        int nj = geometries[j]->regions();
        for (int i=nr; i<nr+nj; ++i) {
            reg_to_inscr[i] = j;
            reg_to_base[i] = fgeoms[j]->ireg;
        }
        nr += nj;
    }
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
    has_B_scaling = g->hasBScaling();
    if (!has_B_scaling) {
        for (int j=0; j<n_in; j++) {
            if (geometries[j]->hasBScaling()) {
                has_B_scaling = true;
                break;
            }
        }
    }
}

EGS_SmartEnvelope::~EGS_SmartEnvelope() {
    if (!g->deref()) {
        delete g;
    }
    for (int j=0; j<n_in; j++) {
        if (!geometries[j]->deref()) {
            delete geometries[j];
        }
    }
    if (geometries) {
        delete [] geometries;
    }
    if (gindex) {
        delete [] gindex;
    }
    if (reg_to_inscr) {
        delete [] reg_to_inscr;
    }
    if (reg_to_base) {
        delete [] reg_to_base ;
    }
    if (local_start) {
        delete [] local_start;
    }
    if (itype) {
        delete itype;
    }
}

void EGS_SmartEnvelope::printInfo() const {
    EGS_BaseGeometry::printInfo();
    egsInformation(" base geometry = %s (type %s)\n",g->getName().c_str(),
                   g->getType().c_str());
    egsInformation(" inscribed geometries:\n");
    for (int j=0; j<n_in; j++) egsInformation("   %s (type %s) in region=%d, "
                " itype=%d\n",
                geometries[j]->getName().c_str(),geometries[j]->getType().c_str(),
                reg_to_base[j],(int)itype[j]);
    egsInformation(
        "=======================================================\n");
}


static char EGS_SMART_ENVELOPE_LOCAL eeg_message1[] =
    "createGeometry(smart envelope): %s\n";
static char EGS_SMART_ENVELOPE_LOCAL eeg_message2[] =
    "null input?";
static char EGS_SMART_ENVELOPE_LOCAL eeg_message3[] =
    "no 'base geometry' input?";
static char EGS_SMART_ENVELOPE_LOCAL eeg_message4[] =
    "incorrect base geometry definition";
static char EGS_SMART_ENVELOPE_LOCAL eeg_message5[] =
    "missing/incorrect 'base geometry' input";
static char EGS_SMART_ENVELOPE_LOCAL eeg_message6[] =
    "createGeometry(smart envelope): no geometry with name %s defined\n";
//static char EGS_SMART_ENVELOPE_LOCAL eeg_message7[] =
//"no inscirebed geometries defined?. I hope you know what you are doing";
//static char EGS_SMART_ENVELOPE_LOCAL eeg_message8[] =
//"an error occured while constructing inscibed geometries";

static char EGS_SMART_ENVELOPE_LOCAL eeg_keyword1[] = "base geometry";
static char EGS_SMART_ENVELOPE_LOCAL eeg_keyword2[] = "geometry";
//static char EGS_SMART_ENVELOPE_LOCAL eeg_keyword3[] = "inscribed geometries";

extern "C" {

    EGS_SMART_ENVELOPE_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {
        if (!input) {
            egsWarning(eeg_message1,eeg_message2);
            return 0;
        }
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
        vector<SmartEnvelopeAux *> fgeoms;
        int nbase = g->regions();
        EGS_Input *ix;
        while ((ix = input->takeInputItem("inscribe geometry")) != 0) {
            vector<string> values;
            ix->getInput("inscribe geometry",values);
            if (values.size() < 2) egsWarning("createGeometry(smart envelope):"
                                                  " %d inputs for 'inscribe geometry'? 2 or more are needed\n",values.size());
            else {
                EGS_BaseGeometry *gj = EGS_BaseGeometry::getGeometry(values[0]);
                if (!gj) {
                    egsWarning(eeg_message6,values[0].c_str());
                }
                else {
                    SmartEnvelopeAux *aux = new SmartEnvelopeAux;
                    aux->g = gj;
                    aux->ireg = atoi(values[1].c_str());
                    aux->type = values.size() == 3 ? atoi(values[2].c_str()) : 0;
                    //egsInformation("set geometr: %s %d %d\n",values[0].c_str(),aux->ireg,aux->type);
                    if (aux->ireg < 0 || aux->ireg >= nbase) {
                        egsWarning("createGeometry(smart envelope): wrong "
                                   "region index %d for inscribed geometry %s\n",
                                   aux->ireg,gj->getName().c_str());
                        delete aux;
                    }
                    else {
                        fgeoms.push_back(aux);
                    }
                }
            }
            delete ix;
        }
        EGS_BaseGeometry *result = new EGS_SmartEnvelope(g,fgeoms,"");
        result->setName(input);
        result->setBoundaryTolerance(input);
        result->setLabels(input);
        for (int j=0; j<fgeoms.size(); j++) {
            delete fgeoms[j];
        }
        return result;

    }

    void EGS_SmartEnvelope::getLabelRegions(const string &str, vector<int> &regs) {

        // label defined in the envelope geometry
        g->getLabelRegions(str, regs);

        // label defined in the inscribed geometries
        vector<int> gregs;
        for (int i=0; i<n_in; i++) {

            // add regions from set geometries
            gregs.clear();
            if (geometries[i]) {
                geometries[i]->getLabelRegions(str, gregs);
            }

            // shift region numbers according to indexing style
            for (int j=0; j<gregs.size(); j++) {
                gregs[j] += local_start[i];
            }

            // add regions to the list
            regs.insert(regs.end(), gregs.begin(), gregs.end());

        }

        // label defined in self (envelope geometry input block)
        EGS_BaseGeometry::getLabelRegions(str, regs);

    }

}
