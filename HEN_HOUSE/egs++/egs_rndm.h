/*
###############################################################################
#
#  EGSnrc egs++ random number generators headers
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


/*! \file egs_rndm.h
 *  \brief Base random number generator
 *  \IK
 */

#ifndef EGS_RANDOM_GENERATOR_
#define EGS_RANDOM_GENERATOR_

#include "egs_libconfig.h"
#include "egs_math.h"
#include "egs_functions.h"

class EGS_Input;

/*! \brief Base random number generator class. All random number generators
 * should be derived from this class.
 *
 * \ingroup egspp_main
 *
 * A random number generator (RNG) is a central part of every Monte Carlo
 * simulation. In order to have the possibility of using different
 * RNGs, the egspp class library defines a RNG as an abstract class.
 * However, for the sake of efficiency, the main method for drawing
 * random numbers getUniform() is implemented as an inline function, which
 * returns the next number in an array of random numbers, possibly after
 * calling the pure virtual method fillArray() to fill the array with
 * random numbers. Appart from getUniform(), which returns a random number
 * uniformely distributed between zero (inclusive) and one (exclusive),
 * EGS_RandomGenerator implements some frequently needed algorithms such
 * as picking a random azimuthal angle ( getAzimuth() ) angle and picking
 * a Gaussian distributed number ( getGaussian() ).
 *
 * \todo Should not EGS_RandomGenerator be derived from EGS_Object,
 * so that dynamically loading RNG DSOs is automatically implemented?
 */
class EGS_EXPORT EGS_RandomGenerator {

public:

    /*! \brief Construct a RNG that has an array of \a n points to
     * hold random numbers.
     *
     * Memory gets allocated and #rarray points to it. The pointer
     * #ip is set to point beyond the array (so that at the next call
     * of getUniform() #rarray gets filled via a call to fillArray() ).
     */
    EGS_RandomGenerator(int n=128);

    /*! \brief Copy constructor.
     *
     * Being able to save the state of a RNG is useful in advanced applications
     * such as correlated sampling.
     */
    EGS_RandomGenerator(const EGS_RandomGenerator &r) : np(0) {
        copyBaseState(r);
    };

    /*! \brief Destructor.
     *
     * Deallocates the memory pointed to by #rarray.
     */
    virtual ~EGS_RandomGenerator() {
        delete [] rarray;
    };

    /*! \brief Returns a random number uniformly distributed between
     * zero (inclusive) and 1 (exclusive).
     *
     * Uses the virtual method fillArray() to fill the array #rarray,
     * if the pointer #ip points beyond the last element of #rarray.
     */
    inline EGS_Float getUniform() {
        if (ip >= np) {
            fillArray(np,rarray);
            ip = 0;
        }
        return rarray[ip++];
    };

    /*! \brief Returns the number of random numbers generated so far.
     *
     * This is useful for simulation diagnostics purposes.
     * \sa numbersUsed()
     */
    EGS_I64 numbersGenerated() const {
        return count;
    };

    /*! \brief Returns the number of random numbers used so far.
     *
     * This is normally different than numbersGenerated() as some of the
     * numbers in the array #rarray may not have been used yet.
     */
    EGS_I64 numbersUsed() const {
        return ip<np ? count - np + ip : count;
    };

    /*! \brief Sets \a cphi and \a sphi to the cosine and sine of a random
     * angle uniformely distributed between 0 and \f$2 \pi \f$.
     *
     * The default implementation uses a box method as this is faster on
     * AMD and Intel CPUs. If the preprocessor macro FAST_SINCOS is defined,
     * then an azimuthal angle \f$\phi\f$ is drawn uniformly between
     * 0 and \f$2 \pi \f$ and \a cphi and \a sphi are calculated using the
     * cosine and sine functions.
     */
    inline void getAzimuth(EGS_Float &cphi, EGS_Float &sphi) {
#ifndef FAST_SINCOS
        register EGS_Float xphi,xphi2,yphi,yphi2,rhophi;
        do {
            xphi = 2*getUniform() - 1;
            xphi2 = xphi*xphi;
            yphi = getUniform();
            yphi2 = yphi*yphi;
            rhophi = xphi2 + yphi2;
        }
        while (rhophi > 1);
        cphi = (xphi2 - yphi2)/rhophi;
        sphi = 2*xphi*yphi/rhophi;
#else
        EGS_Float phi = 2*M_PI*getUniform();
        cphi = cos(phi);
        sphi = sin(phi);
#endif
    };

    /*! \brief Returns a Gaussian distributed random number with mean zero
     * and standard deviation 1.
     */
    inline EGS_Float getGaussian() {
        if (have_x) {
            have_x = false;
            return the_x;
        }
        EGS_Float r = sqrt(-2*log(1-getUniform()));
        EGS_Float cphi, sphi;
        getAzimuth(cphi,sphi);
        have_x = true;
        the_x = r*sphi;
        return r*cphi;
    };

    /*! \brief Create a RNG object from the information pointed to by
     * \a inp and return a pointer to it.
     *
     * The input \a inp must either be itself, or it mist have a property
     * with key <code>rng definition</code>, otherwise the return value
     * of this function will be \c null. The \a sequence parameter is
     * usefull to automatically adjust the initial random number seeds
     * in parallel runs.
     *
     * Note: it is planned to extend this function to be able to create
     * RNG objects by dinamically loading RNG dynamic shared objects,
     * but this functionality is not there yet. For now, the only RNG
     * type available is a ranmar RNG.
     */
    static EGS_RandomGenerator *createRNG(EGS_Input *inp, int sequence=0);

    /*! \brief Returns a pointer to the default egspp RNG.
     *
     * The default egspp RNG is ranmar. The RNG is initialized with
     * by increasing the second ranmar default initial seed by
     * \a sequence.
     */
    static EGS_RandomGenerator *defaultRNG(int sequence=0);

    /*! \brief Fill the array of \a n elements pointed to by \a array with
     * random numbers.
     *
     * This pure virtual function must be implemented by derived RNG classes
     * to fill the array \a array with \a n random numbers uniformly
     * distributed between 0 (inclusive) and 1 (exclusive). The reason for
     * having the interval defined to exclude unity is the fact that
     * traditionally RNGs used with the EGS system have had this property
     * and therefore in many algorithms this behaviour is assumed
     * (\em e.g. log(1-r) is used without checking if 1-r is 0).
     */
    virtual void fillArray(int n, EGS_Float *array) = 0;

    //@{
    //! \name state_functions
    /*! \brief Functions for storing, seting and reseting the state of a RNG
     * object.
     *
     * These functions are useful for being able to perform restarted
     * calculations and to combine the result of parallel runs.
     * They are used by the default implementations of the
     * outputData(), readData(), etc. of the EGS_Application class.
     * The base RNG class implementation stores/reads #count, #np, #ip and
     * #rarray to the output or input stream \a data.
     * Derived RNG classes should should store/read additional data needed
     * to set their state to be the same as at the time of executing one
     * of these functions by reimplementing the pure virtual functions
     * storePrivateState() and setPrivateState().
     */
    bool storeState(ostream &data);
    bool setState(istream &data);
    bool addState(istream &data);
    void resetCounter() {
        count = 0;
    };
    //@}

    /*! \brief Get a copy of the RNG */
    virtual EGS_RandomGenerator *getCopy() = 0;

    /*! \brief Set the state of the RNG from another RNG */
    virtual void setState(EGS_RandomGenerator *r) = 0;

    /*! \brief Save the RNG state */
    virtual void saveState() = 0;

    /*! \brief Reset the state to a previously saved state */
    virtual void resetState() = 0;

    /*! \brief The memory needed to store the rng state */
    virtual int  rngSize() const = 0;

    /*! \brief Describe this RNG
     *
     * Derived RNG classes should reimplement this method to output
     * some information about the RNG being used in ther simulation.
     * This is handy for EGSnrc C++ applications.
     */
    virtual void describeRNG() const {};

protected:

    EGS_I64   count;  //!< random number generated so far
    int       np;     //!< size of the array rarray
    int       ip;     //!< pointer to the next rarray element in the sequence
    EGS_Float *rarray;//!< array with random numbers of size np.

    /*! \brief Store the state of the RNG to the output stream \a data.
     *
     * Derived RNG classes must implement this function to store as much data
     * as needed to the output stream \a data so that a RNG
     * object can be set to exactly the same using these data by
     * invoking setPrivateState().
     *
     * \sa storeState(), setState().
     */
    virtual bool storePrivateState(ostream &data) =  0;

    /*! \brief Set the state of the RNG object from the data in the input
     * stream \a data.
     *
     * This method must be implemented in derived RNG classes. It should read
     * the same data as stored by storePrivateState() in the same order from
     * the input stream \a data.
     */
    virtual bool setPrivateState(istream &data) =  0;

    void copyBaseState(const EGS_RandomGenerator &r);

    void allocate(int n);

    /*! \brief The memory needed to store the base state */
    int  baseSize() const {
        return 2*sizeof(EGS_I32) + sizeof(EGS_I64) + sizeof(bool) +
               sizeof(EGS_Float) + np*sizeof(EGS_Float);
    };


private:

    bool      have_x; //!< used for sampling Gaussian random numbers
    EGS_Float the_x;  //!< used for sampling Gaussian random numbers

};

#endif
