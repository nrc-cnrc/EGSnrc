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
#  Contributors:
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
        }
    };

    EGS_I64 getNextParticle(EGS_RandomGenerator *rndm,
                            int &q, int &latch, EGS_Float &E, EGS_Float &wt,
                            EGS_Vector &x, EGS_Vector &u) {
        int j = table->sampleBin(rndm);
        EGS_I64 this_case = sources[j]->getNextParticle(rndm,q,latch,E,wt,x,u);
        count += this_case - last_cases[j];
        last_cases[j] = this_case;
        return count;
    };
    EGS_Float getEmax() const {
        return Emax;
    };
    EGS_Float getFluence() const {
        EGS_Float flu = 0;
        for (int j=0; j<nsource; j++) {
            flu += sources[j]->getFluence();
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
            if (!sources[j]->setState(data)) {
                return false;
            }
        }
        return true;
    }

    void resetCounter() {
        EGS_BaseSource::resetCounter();
        count = 0;
        for (int j=0; j<nsource; ++j) {
            last_cases[j] = 0;
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
            if (!sources[j]->addState(data_in)) {
                return false;
            }
        }
        return true;
    };

    bool isValid() const {
        return (nsource > 0);
    };

protected:

    int nsource;
    EGS_BaseSource **sources;  //!< The sources in the collection
    EGS_AliasTable *table;     //!< Alias table for randomly picking a source
    EGS_I64        *last_cases;//!< Last case returned from each source
    EGS_Float      *p;         //!< The probabilities
    EGS_Float Emax;            //!< Maximum energy (max of s[j]->getEmax()).
    EGS_I64        count;      //!< Independent particles delivered

    void setUp(const vector<EGS_BaseSource *> &S, const vector<EGS_Float> &);

};

#endif
