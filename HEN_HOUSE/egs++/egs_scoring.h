/*
###############################################################################
#
#  EGSnrc egs++ scoring headers
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


/*! \file egs_scoring.h
 *  \brief Scoring classes
 *  \IK
 */

#ifndef EGS_SCORING_
#define EGS_SCORING_

#include "egs_libconfig.h"
#include "egs_functions.h"
#include "egs_math.h"

#include <iostream>
using namespace std;

/*! \brief A class for scoring a single quantity of interest in a
  Monte Carlo simulation.

  \ingroup egspp_main

  This class implements most of the functionality needed for
  scoring a single quantity of interest in a Monte Carlo simulation,
  including a history-by-history statistical analysis.
  However, as this class is meant to be used by EGS_ScoringArray for
  collecting results of potentially a large number of quantities
  (\em e.g. a 3D dose distribution in a XYZ patient geometry),
  the history counter is represented with an unsigned 16 bit integer
  and therefore the maximum number of different histories is
  65536. Hence, for real applications, it is easier to use
  the EGS_ScoringArray class even for a single quantity of interest.
  */
class EGS_EXPORT EGS_ScoringSingle {

public:

    /*! \brief Construct a scoring object initialized to zero. */
    EGS_ScoringSingle() : sum(0), sum2(0), tmp(0), current_ncase(0) {};

    /*! \brief Add \a f to the score of the \a ncase'th statistically
      independent event.

      If \a ncase is the same as the \a ncase passed in the last call
      to this function, then \a f is simply added to the score of the
      current event, otherwise a new statistically independent event
      is started.
     */
    inline void score(unsigned short ncase, EGS_Float f) {
        if (ncase == current_ncase) {
            tmp += f;
        }
        else {
            finishCase(ncase,f);
        }
    };

    /*! \brief Finish the current 'case' (event) and start a new event
    with index \a new_case and a score of \a new_result.
    */
    inline void finishCase(unsigned short new_case, EGS_Float new_result) {
        current_ncase = new_case;
        sum += tmp;
        sum2 += tmp*tmp;
        tmp = new_result;
    };

    /*! \brief Returns the score of the current event. */
    EGS_Float currentScore() const {
        return tmp;
    };

    /*! \brief Sets \a s to the score of the current event and \a ncase
      to the index of the current event. */
    void      currentScore(EGS_Float &s, unsigned short &ncase) const {
        s = tmp;
        ncase = current_ncase;
    };

    /*! \brief Sets \a s to the sum of scores collected so far
      and \a s2 to the sum of scores squared */
    void      currentScore(double &s, double &s2) {
        s=sum;
        s2=sum2;
    };

    /*! \brief Sets \a r to the current result and \a dr to its statistical
      uncertainty assuming \a ncase statistically independent events.

      This function sets \a r to the ratio of the sum of scores collected
      so far and \a ncase and \a dr to the statistical uncertainty of \a r.
     */
    void      currentResult(EGS_I64 ncase, double &r, double &dr) {
        r = sum + tmp;
        dr = sum2 + tmp*tmp;
        r /= ncase;
        dr /= ncase;
        dr -= r*r;
        if (dr > 0) {
            dr = sqrt(dr/(ncase-1));
        }
    };

    /*! \brief Stores the state of the scoring object into the data stream
      \a data. Returns \c true on success, \a false on failure.

      This function can be used for storing intermediate results into a
      data file for later recovery in \em e.g. restarted calculations.
      The data stored is the sum of scores, sum of scores squared and the
      16 bit integer representing the last statistically independent event
      that contributed to the score.

      \sa setState().
    */
    bool storeState(ostream &data) {
        //sum += tmp; sum2 += tmp*tmp; tmp = 0;
        //data << current_ncase << "  " << sum << "  " << sum2 << endl;
        data << current_ncase << "  " << sum+tmp << "  " << sum2+tmp *tmp
             << endl;
        return data.good();
    };

    /*! \brief Set the state of the scoring object from the data stream \a data.

      The data extracted from the stream is the 16 bit integer representing
      the last statistically independent event that contributed to the score,
      the sum of scores and the sum of scores squared
      (both double precision).

      \sa storeState()
     */
    bool setState(istream &data) {
        data >> current_ncase >> sum >> sum2;
        tmp = 0;
        return data.good();
    };

    /*! \brief Reset the scoring object to a pristine state (\em i.e. all
      counters set to zero).
     */
    void reset() {
        current_ncase = 0;
        tmp = 0;
        sum = 0;
        sum2 = 0;
    };

    /*! \brief Combine the results of two scoring objects.

     This operator adds the results of the scoring object \a x to the
     results collected so far by the invoking object, assuming that
     \a x is statistically independent. This method is useful for
     combining parallel runs.
    */
    EGS_ScoringSingle &operator+=(const EGS_ScoringSingle &x) {
        sum += tmp + x.sum + x.tmp;
        sum2 += tmp*tmp + x.sum2 + x.tmp*x.tmp;
        current_ncase = 0;
        tmp = 0;
        return *this;
    };


protected:

    /*! Sum of scores collected so far by the scoring object. */
    double         sum;
    /*! Sum of scores squared collected so far by the scoring object. */
    double         sum2;
    /*! The score of the current event. */
    EGS_Float      tmp;
    /*! The index of the current statistically independent event */
    unsigned short current_ncase;

};

/*! \brief A class for scoring an array of quantities (\em e.g. a dose
  distribution) in a Monte Carlo simulation.

  \ingroup egspp_main

 The EGS_ScoringArray class implements all functionality needed for scoring
 a single quantity of interest or an array of quantities of interest in
 a Monte Carlo simulation, including history-by-history statistical
 analysis. Internally it makes use of the EGS_ScoringSingle class for
 accumulating the result in each element of the array but it also maintains
 a 64 bit integer indicating the last statistically independent event that
 contributed to any of the elements of the scoring array.
*/
class EGS_EXPORT EGS_ScoringArray {

public:

    /*! \brief Construct a scoring array with \a N elements.

      All elements are initialized to zero. \a N must be greater than zero.
     */
    EGS_ScoringArray(int N);

    /*! \brief Destructor. Deallocates all allocated memory */
    ~EGS_ScoringArray();

    /*! \brief Set the current statistically independent event to \a ncase.

      This function must be called before starting the simulation of the
      next statistically independent event (history).
    */
    void setHistory(EGS_I64 ncase);

    /*! \brief Add \a f to the score in the element \a ireg.

      Uses EGS_ScoringSingle::score() of the scoring object in the
      region \a ireg
     */
    inline void score(int ireg, EGS_Float f) {
        result[ireg].score(current_ncase_short,f);
    };

    /*! \brief Returns the score in element \a ireg from the last
     statistically indepent event that contributed to \a ireg (which may be
     the current event)

     \sa thisHistoryScore()
     */
    EGS_Float currentScore(int ireg) const {
        return result[ireg].currentScore();
    };

    /*! \brief Returns the score in \a ireg in the current event. */
    EGS_Float thisHistoryScore(int ireg) const {
        EGS_Float res;
        unsigned short nc;
        result[ireg].currentScore(res,nc);
        return nc == current_ncase_short ? res : 0;
    };

    /*! \brief Sets \a s and \a s2 to the sum of scores and sum of scores
      squared in element \a ireg.

      \sa EGS_ScoringSingle::currentScore(double,double).
     */
    void currentScore(int ireg, double &s, double &s2) {
        result[ireg].currentScore(s,s2);
    };

    /*! \brief Sets \a r to the result in region \a ireg and \a dr to its
      statistical uncertainty

      \sa EGS_ScoringSingle::currentResult(double,double)
     */
    void currentResult(int ireg, double &r, double &dr) {
        result[ireg].currentResult(current_ncase,r,dr);
    };

    /*! Reports the results collected so far using egsInformation().

     This function outputs the results collected so far using
     egsInformation() after printing the character string pointed to by \a
     title.
     The results are normalized using \a norm,
     \em i.e. the result output is sum of scores collected so far divided
     by the number of statistically independent events and multiplied
     with \a norm.
     The default output is of the form <code><br>
       element index   result +/- uncertainty <br></code>
     for each element in the array, one element per line.
     If \a relative_error is \c true, the uncertainty output will be as
     percent of the result, otherwise the absolute value of the uncertainties
     will be output. The default format string used is
     <code> %d  %g  +/-  %g</code>, this can be modified by passing a non-null
    pointer to a format string with \a format.
    */
    void reportResults(double norm, const char *title, bool relative_error,
                       const char *format = 0);

    /*! \brief Stores the state of the scoring array object into the data
      stream \a data.

      This function can be used for storing intermediate results into a
      data file for later recovery in \em e.g. restarted calculations.
      The information stored in the number of element in the array #nreg,
      the current short integer history counter, the current history
      counter, the current history counter divided by 65536 and
      the data from each of the #nreg elements using their
      EGS_ScoringSingle::storeData function.
    */
    bool storeState(ostream &data) {
        data << nreg << "  " << current_ncase_short << endl;
        if (!egsStoreI64(data,current_ncase)) {
            return false;
        }
        if (!egsStoreI64(data,current_ncase_65536)) {
            return false;
        }
        data << endl;
        for (int j=0; j<nreg; j++) {
            if (!result[j].storeState(data)) {
                return false;
            }
        }
        return true;
    };

    /*! \brief Sets the state fof the scoring array object from the
      data in the input stream \a data.

      This function is useful for seting the state of the scoring object
      to a state previously stored using storeState() in \em e.g.
      restarted simulations.
    */
    bool setState(istream &data) {
        int nreg1;
        data >> nreg1 >> current_ncase_short;
        if (!data.good() || nreg1 < 1) {
            return false;
        }
        if (!egsGetI64(data,current_ncase)) {
            return false;
        }
        if (!egsGetI64(data,current_ncase_65536)) {
            return false;
        }
        if (nreg1 != nreg) {
            if (nreg > 0) {
                delete [] result;
            }
            nreg = nreg1;
            result = new EGS_ScoringSingle [nreg];
        }
        for (int j=0; j<nreg; j++) {
            if (!result[j].setState(data)) {
                return false;
            }
        }
        return true;
    };

    /*! \brief Reset the scoring array to a pristine state. */
    void reset() {
        current_ncase = 0;
        current_ncase_65536 = 0;
        current_ncase_short = 0;
        for (int j=0; j<nreg; j++) {
            result[j].reset();
        }
    };

    /*! \brief Add the results of \a x to the rtesults of the invoking
      object.

      This operator is useful for \em e.g. combining the results of
      parallel runs.
    */
    EGS_ScoringArray &operator+=(const EGS_ScoringArray &x) {
        current_ncase += x.current_ncase;
        current_ncase_65536 = current_ncase >> 16;
        EGS_I64 aux = current_ncase - (current_ncase_65536 << 16);
        current_ncase_short = (unsigned short) aux;
        for (int j=0; j<nreg; j++) {
            result[j] += x.result[j];
        }
        return *this;
    };

    /*! \brief Returns the number of bins (or elements or regions, the
      most appropriate term depending on the way the scorring array is being
      used).
    */
    int bins() const {
        return nreg;
    };

    /*! \brief Returns the number of regions (or elements or bins, the
      most appropriate term depending on the way the scorring array is being
      used).
    */
    int regions() const {
        return nreg;
    };

protected:

    /*! Current statistically indepent event set with setHistory(). */
    EGS_I64           current_ncase;
    /*! current_ncase divided by 65536 */
    EGS_I64           current_ncase_65536;
    /*! Number of elements (bins, regions) the scorring array has. Set in the
      object constructor. */
    int               nreg;
    /*! The nreg scoring elements */
    EGS_ScoringSingle *result;
    /*! current_ncase%65536. This is needed because the individual elements
      of the array only use an unsigned 16 bit integer for their history
      number.
    */
    unsigned short    current_ncase_short;

};

#endif
