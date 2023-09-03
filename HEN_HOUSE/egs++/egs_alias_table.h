/*
###############################################################################
#
#  EGSnrc egs++ alias table headers
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
#  Contributors:    Ernesto Mainegra-Hing
#                   Marc Chamberland
#
###############################################################################
*/


/*! \file egs_alias_table.h
 *  \brief EGS_AliasTable class header file
 *  \IK
 */

#ifndef EGS_ALIAS_TABLE_
#define EGS_ALIAS_TABLE_

#include "egs_libconfig.h"
#include "egs_rndm.h"
#include <vector>

typedef EGS_Float(*EGS_AtFunction)(EGS_Float,void *);

/*! \brief A class for sampling random values from a given probability
  distribution using the alias table technique.

  \ingroup egspp_main

  Instances of this class can be used to efficiently sample random variables
  from any probability distribution. The probability distribution is
  represented internally as an alias table. EGS_AliasTable objects can
  be initialized from a distribution tabulated as discrete values, as a
  histogram, or as function values at bin edges and a linear interpolation
  between these values (see \link
  EGS_AliasTable::EGS_AliasTable(int,const EGS_Float*,const EGS_Float*,int)
  this constructor \endlink),
  or from a function within a specified interval (see \link
  EGS_AliasTable::EGS_AliasTable(EGS_Float,EGS_Float,EGS_Float,int,EGS_AtFunction,void *) this constructor \endlink)
  Random values are then drawn using the sample() method.
*/
class EGS_EXPORT EGS_AliasTable {

public:

    /*! \brief Construct an empty (uninitialized) alias table

      The table should be initialized using one of the initialize() methods
      before using it for sampling.
    */
    EGS_AliasTable() : n(0) {};

    /*! \brief Copy constructor. Performs a deep copy */
    EGS_AliasTable(const EGS_AliasTable &t) : n(0) {
        copy(t);
    };

    /*! \brief Construct an alias table from the tabulated distribution
      with \a N bins with probabilities \a f at the bins \a x.

      If \a Type = 0, the \a N \a x values are interpreted as discrete
      abscissas, \em i.e. the probability distribution is assumend to be
      \f$\sum f_i \delta(x - x_i)\f$, where \f$\delta\f$
      is Dirac's delta function.
      If \a Type is 1, the distribution is assumed to be a histogram
      with \a N-1 bins with the \f$i\f$'th bin between \f$x_i\f$ and
      \f$x_{i+1}\f$ having a probability \f$f_i\f$.
      If \a Type is 2, the distribution is assumed to be continuous
      with a linear variation between the \f$x_i,f_i\f$ and
      \f$x_{i+1},f_{i+1}\f$.
     */
    EGS_AliasTable(int N, const EGS_Float *x, const EGS_Float *f,
                   int Type = 1) : n(0) {
        initialize(N,x,f,Type);
    };


    /*! \brief Construct an alias table for sampling values in the interval
      \a xmin ... \a xmax for the function \a func.

      The interval \a xmin ... \a xmax is divided into interpolation
      intervals in such a way that the error using a linear interpolation
      does not exceed \a accu. However, if the number of bins needed to
      achieve an interpolation accuracy of \a accu is larger than
      \a nmax, no more than \a nmax bins are used.
      Internally the alias table will be set to be of type 2.
     */
    EGS_AliasTable(EGS_Float xmin, EGS_Float xmax, EGS_Float accu, int nmax,
                   EGS_AtFunction func, void *data) : n(0) {
        initialize(xmin,xmax,accu,nmax,func,data);
    };

    /*! \brief Initialize the alias table

    See \link EGS_AliasTable::EGS_AliasTable(int,const EGS_Float*,const EGS_Float*,int=1) the constructor \endlink with corresponding arguments.
    */
    void initialize(int N, const EGS_Float *x, const EGS_Float *f,
                    int Type = 1);

    /*! \brief Initialize the alias table

      See the \link
      EGS_AliasTable::EGS_AliasTable(EGS_Float,EGS_Float,EGS_Float,int,EGS_AtFunction,void*)
      constructor \endlink with corresponding arguments.
      Return value is 0 on success (the accuracy goal was satisfied with
      less than \a nmax bins), 1 if it was not possible to get the required
      interpolation accuracy with \a nmax bins.
     */
    int initialize(EGS_Float xmin, EGS_Float xmax, EGS_Float accu, int nmax,
                   EGS_AtFunction func, void *data);

    /*! \brief Get a random point from this table using the RNG \a rndm. */
    EGS_Float sample(EGS_RandomGenerator *rndm) const;

    /*! \brief Get a random bin from this table.  */
    int sampleBin(EGS_RandomGenerator *rndm) const;

    /*! \brief Get the average of the probability distribution represented
      by this alias table object. */
    EGS_Float getAverage() const {
        return average;
    };

    /*! \brief Get the maximum abscissa of this alias table object. */
    EGS_Float getMaximum() const {
        return xi[n-1];
    };

private:

    int       n;     //!< number of subintervals
    int       np;    //!< =n for type=0, =n-1 else.
    EGS_Float *fi;   //!< array of function values
    EGS_Float *xi;   //!< array of coordinates
    EGS_Float *wi;   //!< array of bin branching probabilities
    EGS_Float average;
    int       *bin;  //!< bins
    int       type;  /*!< 0 => sum of delta functions, 1 => histogram
                      2 => linear interpolation between bin edges */

    void      copy(const EGS_AliasTable &t);
    void      clear();
    void      allocate(int N, int Type);
    void      make();


};

/*! \brief A class for sampling random bins from a given probability
  distribution using the alias table technique.

  \ingroup egspp_main

  Instances of this class can be used to efficiently sample random bins
  from any probability distribution. This is a special case of the
  more general \ref EGS_AliasTable class that has a more efficient
  implementation and is easier to use when only sampling random
  bin indeces is needed
*/

class EGS_EXPORT EGS_SimpleAliasTable {

public:

    /*! \brief Constructor

    Construct a simple alias table having \c N bins with probabilities given
    by the array \c f
    */
    EGS_SimpleAliasTable(int N, const EGS_Float *f);

    /*! \brief Destructor */
    ~EGS_SimpleAliasTable();

    /*! \brief Sample a random bin

    Returns a random bin according to the bin probabilities for this alias table
    */
    int sample(EGS_RandomGenerator *rndm) const {
        int bin = (int)(rndm->getUniform()*n);
        return rndm->getUniform() < wi[bin] ? bin : bins[bin];
    };

private:

    int       n;          //!< number of subintervals
    EGS_Float *wi;        //!< array of bin branching probabilities
    int       *bins;      //!< bins

};


#endif
