/*
###############################################################################
#
#  EGSnrc egs++ source collection headers
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
#                   Marc Chamberland
#                   Blake Walters
#
###############################################################################
*/


/*! \file egs_source_collection.h
 *  \brief A source collection
 *  \IK
 */

#ifndef EGS_SOURCE_COLLECTION_
#define EGS_SOURCE_COLLECTION_

#include "egs_vector.h"
#include "egs_base_source.h"
#include "egs_rndm.h"
#include "egs_alias_table.h"

#include <algorithm>

#ifdef WIN32

    #ifdef BUILD_SOURCE_COLLECTION_DLL
        #define EGS_SOURCE_COLLECTION_EXPORT __declspec(dllexport)
    #else
        #define EGS_SOURCE_COLLECTION_EXPORT __declspec(dllimport)
    #endif
    #define EGS_SOURCE_COLLECTION_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_SOURCE_COLLECTION_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_SOURCE_COLLECTION_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_SOURCE_COLLECTION_EXPORT
        #define EGS_SOURCE_COLLECTION_LOCAL
    #endif

#endif

/*! \brief A source collection

  \ingroup Sources

A source collection consists of an arbitrary number \f$N\f$ of any
other sources \f$s_1, s_2, ..., s_N\f$ with weights of \f$w_1, w_2, ..., w_N\f$
and delivers particles from \f$s_j\f$ with probability \f$w_j\f$.
A source collection is defined using
\verbatim
:start source:
    library = egs_source_collection
    name = some_name
    source names = list of names of previously defined sources
    weights = list of weights for the sources
:stop source:
\endverbatim

A simple example:
\verbatim
:start source definition:
    :start source:
        library     = egs_point_source
        name        = p1
        position    = -2 0 0
        :start spectrum:
            type    = monoenergetic
            energy  = 1
        :stop spectrum:
        charge      = 0
    :stop source:
    :start source:
        library     = egs_point_source
        name        = p2
        position    = 2 0 0
        :start spectrum:
            type    = monoenergetic
            energy  = 1
        :stop spectrum:
        charge      = 0
    :stop source:
    :start source:
        library = egs_source_collection
        name = my_source
        source names = p1 p2
        weights = 0.1 0.9
    :stop source:

    simulation source = my_source

:stop source definition:
\endverbatim
\image html egs_source_collection.png "A simple example"
*/
class EGS_SOURCE_COLLECTION_EXPORT EGS_SourceCollection :
    public EGS_BaseSource {

public:

    /*! \brief Constructor

      Construct a source collection from the sources \a S using the
      probabilities \a prob.
    */
    EGS_SourceCollection(const vector<EGS_BaseSource *> &S,
                         const vector<EGS_Float> &prob,
                         const string &Name="", EGS_ObjectFactory *f=0) :
        EGS_BaseSource(Name,f), nsource(0), count(0) {
        setUp(S,prob);
    };

    /*! \brief Constructor

    Construct a source collection from the input pointed to by \a inp.
    */
    EGS_SourceCollection(EGS_Input *, EGS_ObjectFactory *f=0);
    ~EGS_SourceCollection() {
        if (nsource > 0) {
            for (int j=0; j<nsource; j++) {
                EGS_Object::deleteObject(sources[j]);
            }
            delete [] sources;
            delete table;
            delete [] p;
            delete [] last_cases;
            delete [] last_flu;
            delete [] p_group;
        }
    };

    EGS_I64 getNextParticle(EGS_RandomGenerator *rndm,
                            int &q, int &latch, EGS_Float &E, EGS_Float &wt,
                            EGS_Vector &x, EGS_Vector &u) {
        int j = table->sample(rndm);
        EGS_I64 this_case = sources[j]->getNextParticle(rndm,q,latch,E,wt,x,u);
        count += this_case - last_cases[j];
        last_cases[j] = this_case;
        for (int i=0; i<nsource; i++) {
            //prevent "false" fluence counts in case of a collection consisting of multiple
            //transformations of a common base source using a vector that, for each source,
            //stores all other sources sharing the same base source
            //use fluence increments in the unselected sources to detect this
            if (i != j && sources[i]->getFluence() > last_flu[i]) {
                if (std::find(p_group[i].begin(), p_group[i].end(), j) == p_group[i].end()) {
                    p_group[i].push_back(j);
                    p_group[j].push_back(i);
                }
                //set last_case for this source equal to that for source j
                last_cases[i]=last_cases[j];
            }
            last_flu[i] = sources[i]->getFluence();
        }
        return count;
    };
    EGS_Float getEmax() const {
        return Emax;
    };
    EGS_Float getFluence() const {
        EGS_Float flu = 0;
        for (int j=0; j<nsource; j++) {
            EGS_Float p_tot=p[j];
            for (int i=0; i<p_group[j].size(); i++) {
                p_tot += p[p_group[j][i]];
            }
            int norm = 1;
            //if we've combined results where a group of sources using a single base source
            //is used, then at this point the summed fluence for EACH source is actually multiplied
            //by the number of sources in the group
            if (i_add) {
                norm = p_group[j].size()+1;
            }
            flu += p[j]/p_tot*sources[j]->getFluence()/norm;
        }
        return flu;
    };
    bool storeState(ostream &data) const {
        bool res = EGS_BaseSource::storeState(data);
        if (!res) {
            return res;
        }
        res = egsStoreI64(data,count);
        if (!res) {
            return res;
        }
        data << " ";
        for (int j=0; j<nsource; j++) {
            res = egsStoreI64(data,last_cases[j]);
            if (!res) {
                return res;
            }
            data << " ";
            for (int i=0; i<p_group[j].size(); i++) {
                data << p_group[j][i] << " ";
            }
            // use -1 to denote end of group sharing a common base source
            data << -1 << " ";
            if (!sources[j]->storeState(data)) {
                return false;
            }
        }
        return true;
    };
    bool setState(istream &data) {
        bool res = EGS_BaseSource::setState(data);
        if (!res) {
            return res;
        }
        res = egsGetI64(data,count);
        if (!res) {
            return res;
        }
        for (int j=0; j<nsource; j++) {
            res = egsGetI64(data,last_cases[j]);
            if (!res) {
                return res;
            }
            EGS_I64 tmp_int;
            data >> tmp_int;
            while (tmp_int != -1) {
                if (std::find(p_group[j].begin(), p_group[j].end(), tmp_int) == p_group[j].end()) {
                    p_group[j].push_back(tmp_int);
                }
                data >> tmp_int;
            }
            if (!sources[j]->setState(data)) {
                return false;
            }
            last_flu[j]=sources[j]->getFluence();
        }
        return true;
    }

    void resetCounter() {
        EGS_BaseSource::resetCounter();
        count = 0;
        for (int j=0; j<nsource; ++j) {
            last_cases[j] = 0;
            last_flu[j] = 0;
            sources[j]->resetCounter();
        }
    };

    virtual bool addState(istream &data_in) {
        EGS_I64 tmp;
        bool res = EGS_BaseSource::addState(data_in);
        if (!res) {
            return res;
        }
        res = egsGetI64(data_in,tmp);
        if (!res) {
            return res;
        }
        count += tmp;
        for (int j=0; j<nsource; j++) {
            res = egsGetI64(data_in,tmp);
            if (!res) {
                return res;
            }
            last_cases[j] += tmp;
            EGS_I64 tmp_int;
            data_in >> tmp_int;
            while (tmp_int != -1) {
                if (std::find(p_group[j].begin(), p_group[j].end(), tmp_int) == p_group[j].end()) {
                    p_group[j].push_back(tmp_int);
                }
                data_in >> tmp_int;
            }
            if (!sources[j]->addState(data_in)) {
                return false;
            }
            last_flu[j]=sources[j]->getFluence();
        }
        i_add = true;
        return true;
    };

    bool isValid() const {
        return (nsource > 0);
    };

    void setSimulationChunk(EGS_I64 nstart, EGS_I64 nrun, int npar, int nchunk) {
        for (int j=0; j<nsource; j++) {
            sources[j]->setSimulationChunk(nstart, nrun, npar, nchunk);
        }
    };

    void containsDynamic(bool &hasdynamic);

    void printSampledEmissions() {
        for (int j=0; j<nsource; j++) {
            sources[j]->printSampledEmissions();
        }
    }

    vector<EGS_Ensdf*> getRadionuclideEnsdf() {
        vector<EGS_Ensdf*> allDecays;
        for (int j=0; j<nsource; j++) {
            for(auto decays: sources[j]->getRadionuclideEnsdf()) {
                allDecays.push_back(decays);
            }
        }
        return allDecays;
    };

protected:

    int nsource;
    EGS_BaseSource **sources;  //!< The sources in the collection
    EGS_SimpleAliasTable *table;     //!< Alias table for randomly picking a source
    EGS_I64        *last_cases;//!< Last case returned from each source
    EGS_Float      *p;         //!< The probabilities
    EGS_Float      *last_flu;   //!< Saved value of source_flu
    EGS_Float Emax;            //!< Maximum energy (max of s[j]->getEmax()).
    EGS_I64        count;      //!< Independent particles delivered
    vector<EGS_I64> *p_group;  //!< Vector of sources using the same base source
    bool i_add;                //!< Set to true if parallel results have been combined

    void setUp(const vector<EGS_BaseSource *> &S, const vector<EGS_Float> &);

};

#endif
