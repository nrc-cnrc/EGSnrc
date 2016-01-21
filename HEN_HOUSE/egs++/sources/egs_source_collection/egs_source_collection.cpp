/*
###############################################################################
#
#  EGSnrc egs++ source collection
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


/*! \file egs_source_collection.cpp
 *  \brief A source collection
 *  \IK
 */

#include "egs_source_collection.h"
#include "egs_input.h"

EGS_SourceCollection::EGS_SourceCollection(EGS_Input *input,
        EGS_ObjectFactory *f) : EGS_BaseSource(input,f), nsource(0), count(0) {
    vector<EGS_BaseSource *> s;
    egsInformation("EGS_BaseSource::EGS_BaseSource: input is:\n");
    input->print(0,cout);
    EGS_Input *isource;
    while ((isource = input->takeInputItem("source",false))) {
        egsInformation("EGS_SourceCollection: got input\n");
        EGS_BaseSource *this_source = EGS_BaseSource::createSource(isource);
        if (!this_source) {
            egsWarning("EGS_SourceCollection: got null source\n");
        }
        else {
            s.push_back(this_source);
        }
        delete isource;
    }
    vector<string> snames;
    int err = input->getInput("source names",snames);
    if (!err) {
        for (unsigned int j=0; j<snames.size(); j++) {
            EGS_BaseSource *this_source = EGS_BaseSource::getSource(snames[j]);
            if (!this_source) {
                egsWarning("EGS_SourceCollection: got null source\n");
            }
            else {
                s.push_back(this_source);
            }
        }
    }
    if (s.size() < 1) {
        egsWarning("EGS_SourceCollection: no sources\n");
        return;
    }
    vector<EGS_Float> prob;
    err = input->getInput("weights",prob);
    if (err) {
        egsWarning("EGS_SourceCollection: missing 'weights' input\n");
        return;
    }
    if (prob.size() != s.size()) {
        egsWarning("EGS_SourceCollection: the number of sources (%d) is not"
                   " the same as the number of input probabilities (%d)\n",
                   s.size(),prob.size());
        return;
    }
    setUp(s,prob);
}

void EGS_SourceCollection::setUp(const vector<EGS_BaseSource *> &S,
                                 const vector<EGS_Float> &prob) {
    otype = "EGS_SourceCollection";
    nsource = S.size();
    if (prob.size() < nsource) {
        nsource = prob.size();
    }
    description = "Invalid source collection";
    if (isValid()) {
        p = new EGS_Float [nsource];
        EGS_Float *dum = new EGS_Float [nsource];
        sources = new EGS_BaseSource* [nsource];
        Emax = 0;
        for (int j=0; j<nsource; j++) {
            p[j] = prob[j];
            sources[j] = S[j];
            if (p[j] < 0 || !sources[j]) {
                if (p[j] < 0) egsWarning("EGS_SourceCollection: input "
                                             "probability p[%d]=%g is less than zero.\n",j,p[j]);
                else {
                    egsWarning("EGS_SourceCollection: source %d is null\n",j);
                }
                delete [] p;
                delete [] dum;
                for (int i=0; i<j; j++) {
                    EGS_Object::deleteObject(sources[i]);
                }
                delete [] sources;
                nsource = 0;
                return;
            }
            dum[j] = 1;
            sources[j]->ref();
            EGS_Float e = sources[j]->getEmax();
            if (e > Emax) {
                Emax = e;
            }
        }
        table = new EGS_AliasTable(nsource,dum,p,0);
        delete [] dum;
        description = "Source collection";
        last_cases = new EGS_I64 [ nsource ];
        for (int i=0; i<nsource; i++) {
            last_cases[i] = 0;
        }
    }
}

extern "C" {

    EGS_SOURCE_COLLECTION_EXPORT EGS_BaseSource *createSource(EGS_Input *input,
            EGS_ObjectFactory *f) {
        return
            createSourceTemplate<EGS_SourceCollection>(input,f,"source collection");
    }

}
