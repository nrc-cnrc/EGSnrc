/*
###############################################################################
#
#  EGSnrc egs++ interpolator headers
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


/*! \file egs_interpolator.h
 *  \brief A class for run-time interpolations
 *  \IK
 */

#ifndef EGS_INTERPOLATOR_
#define EGS_INTERPOLATOR_

#include "egs_libconfig.h"

typedef EGS_Float(*EGS_InterpolatorFuncion)(EGS_Float,void *);

/*! \brief A class for fast run-time interpolations.

  \ingroup egspp_main

 */
class EGS_EXPORT EGS_Interpolator {

public:

    /*! \brief Create an empty (unitialized) interpolator */
    EGS_Interpolator();

    /*! \brief Create an interpolator for the data \a values using \a nbin bins.

     The data is assumed to be uniformely distributed between \a Xmin and
     \a Xmax.
    */
    EGS_Interpolator(int nbin, EGS_Float Xmin, EGS_Float Xmax,
                     const EGS_Float *values);

    /*! \brief Create an interpolator for the function \a func
      in \a Xmin...Xmax

      The interval \a Xmin...Xmax is divided into \a nbin uniform bins
      and the interpolator is initialized to perform a linear intepolation
      between the function values returned by \a func at the bin edges.
     */
    EGS_Interpolator(int nbin, EGS_Float Xmin, EGS_Float Xmax,
                     EGS_InterpolatorFuncion func,
                     void *data);

    /*! \brief Create an interpolator for the function \a func
        in \a Xmin...Xmax

     The interval \a Xmin...Xmax is divided into a sufficient number of bins
     so that the relative interpolation error does not exeed \a accu.
     If the number of required bins becomes greater than \a nmax,
     \a nmax bins are used.
     */
    EGS_Interpolator(EGS_Float Xmin, EGS_Float Xmax,
                     EGS_InterpolatorFuncion func, void *data,
                     int nmax = 1024, EGS_Float accu = 1e-4);

    /*! \brief Create an interpolator using the existing coefficients
      \a a and \a b.

     The interpolation is given as a[i] + x*b[i], where i is the index
     corresponding to x. This type of interpolator is used to have
     convenient access to the mortran EGSnrc interpolation arrays
     without duplicating the interpolation coefficients.
     */
    EGS_Interpolator(int nbin, EGS_Float Xmin, EGS_Float Xmax,
                     EGS_Float *a, EGS_Float *b);

    /*! \brief Destructor */
    ~EGS_Interpolator();

    /*! \brief Initialize the interpolator

    See the \link
    EGS_Interpolator::EGS_Interpolator(int,EGS_Float,EGS_Float,const EGS_Float *)
    constructor with same argument list \endlink
    for more details
    */
    void initialize(int nbin, EGS_Float Xmin, EGS_Float Xmax,
                    const EGS_Float *values);

    /*! \brief Initialize the interpolator

     See the \link
     EGS_Interpolator::EGS_Interpolator(int,EGS_Float,EGS_Float,EGS_InterpolatorFuncion,void*)
     constructor with the same argument list \endlink
     for more details
    */
    void initialize(int nbin, EGS_Float Xmin, EGS_Float Xmax,
                    EGS_InterpolatorFuncion func,
                    void *data);

    /* \brief Initialize the interpolator

    See the \link
    EGS_Interpolator::EGS_Interpolator(int,EGS_Float,EGS_Float,EGS_InterpolatorFuncion,void*,int,EGS_Float)
    constructor with the same argument list \endlink for more details
    */
    void initialize(EGS_Float Xmin, EGS_Float Xmax,
                    EGS_InterpolatorFuncion func, void *data,
                    int nmax = 1024, EGS_Float accu = 1e-4);

    /*! \brief Initialize the interpolator

      See the \link
      EGS_Interpolator::EGS_Interpolator(int,EGS_Float,EGS_Float,EGS_Float*,EGS_Float*)
      constructor with the same argument list \endlink
     for more details
    */
    void initialize(int nbin, EGS_Float Xmin, EGS_Float Xmax,
                    EGS_Float *a, EGS_Float *b);

    /*! \brief Interpolate the function value at \a x.

      This function returns the function value at \a x, if \a x is within the
      initialized interpolation interval \a Xmin...Xmax, \a f(Xmin) if
      \a x < \a Xmin and \a f(Xmax), if \a x > \a Xmax.

      \sa interpolateFast().
     */
    inline EGS_Float interpolate(EGS_Float x) const {
        if (x > xmin && x < xmax) {
            int i = (int)(ax + bx*x);
            if (i < 0) {
                i=0;
            }
            return a[i] + b[i]*x;
        }
        else if (x <= xmin) {
            return fmin;
        }
        else {
            return fmax;
        }
    };

    /*! \brief Interpolate the function value at \a x.

     This function can be used when it is guaranteed that \a x is in the
     interval \a xmin...xmax as no checks are made.

     \sa interpolate().
    */
    inline EGS_Float interpolateFast(EGS_Float x) const {
        int i = (int)(ax + bx*x);
        return a[i] + b[i]*x;
    };

    /*! \brief Get the interpolation index corresponding to \a x.

      \sa getIndexFast()
     */
    inline int getIndex(EGS_Float x) const {
        if (x > xmin && x < xmax) {
            return (int)(ax + bx*x);
        }
        else if (x <= xmin) {
            return 0;
        }
        else {
            return n-1;
        }
    };

    /*! \brief Get the interpolation index corresponding to \a x.

    This function can only be used if it is guaranteed that \a x is in the
    interval \a xmin...xmax as no checks are made.

    \sa getIndex()
    */
    inline int getIndexFast(EGS_Float x) const {
        return (int)(ax + bx*x);
    };

    /*! \brief Interpolate the function value at \a x assuming that \a belongs
      to the interpolation interval \a i.

      This function can be used together with getIndex() or getIndexFast()
      for performing the interpolation. If several quantities are to be
      interpolated for the same value of x from different interpolator objects
      all initialized within the same interval using the same number of bins,
      it is much faster to first call getIndex() or getIndexFast() to get
      the interpolation index and then use this function instead of repeating
      the interpolation index determination in each individual call of
      interpolate() or interpolateFast(). The
      getIndexFast() function closely corresponds to the
      <code>\$SET INTERVAL </code> mortran macro and
      interpolateFast(int,EGS_Float) to the
      <code>\$EVALUATE USING</code> mortran macro.
    */
    inline EGS_Float interpolateFast(int i, EGS_Float x) const {
        return a[i] + b[i]*x;
    };

    /*! \brief Get the lower interpolation interval limit. */
    EGS_Float getXmin() const {
        return xmin;
    };
    /*! \brief Get the upper interpolation interval limit. */
    EGS_Float getXmax() const {
        return xmax;
    };

private:

    int            n;     //!< number of bins
    EGS_Float ax, bx;     //!< convert x to an index.
    EGS_Float *a, *b;     //!< interpolation coefficients.
    EGS_Float xmin,xmax;  //!< interpolation interval.
    EGS_Float fmin,fmax;  //!< function values at interval boundaries.
    bool    own_data;
    //!< true if the interpolator owns the data pointed to by a and b.
    void clear();
    void check(int nbin, EGS_Float Xmin, EGS_Float Xmax);
};

#endif
