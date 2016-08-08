/*
###############################################################################
#
#  EGSnrc egs++ base source headers
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


/*! \file egs_base_source.h
 *  \brief EGS_BaseSource class header file
 *  \IK
 *
 *  Also declares the base spectrum class and the simple source class,
 *  and provides a source creation template function.
 */

#ifndef EGS_BASE_SOURCE_
#define EGS_BASE_SOURCE_

#include "egs_vector.h"
#include "egs_object_factory.h"
#include "egs_functions.h"

#include <string>
#include <iostream>
#include "egs_math.h"

using namespace std;

class EGS_Input;
class EGS_RandomGenerator;

/*! \brief Base source class. All particle sources must be derived from
  this class.

  \ingroup Sources
  \ingroup egspp_main

  Particle sources in the EGSnrc C++ class library are classes derived
  from the base source class and compiled into dynamic shared objects
  (a.k.a. DLLs) that get dynamically loaded at run time as specified by
  the sources property <code>source definition</code>
  (see \ref Sources). The base source defines the interface to a source
  object. The main method is getNextParticle() which returns the parameter
  of a single particle distributed according to some probability distribution.
  Additional functions that must be implemented by derived classes are
  - getEmax(): returns the maximum energy of the source
  - getFluence(): returns the fluence the source has emitted so far
  - getSourceDescription(): returns a string describing the source
  - storeState(), setState(), resetCounter(), addState(): these are functions
    needed for restarting calculations or combining the results of parallel
    runs

  \todo Add time dependence

*/
class EGS_EXPORT EGS_BaseSource : public EGS_Object {

public:

    /*! \brief Construct a source named \a Name.
     *
     */
    EGS_BaseSource(const string &Name="", EGS_ObjectFactory *f = 0) :
        EGS_Object(Name,f) {};

    /*! \brief Construct a source from the input pointed to by \a inp.
     *
     *  The property tree pointed to by \a inp must contain at least
     *  the following key-value pairs:<br><br><code>
     *  name = name of this source <br>
     *  library = source library <br><br></code>
     *  plus additional information as needed by the source being created.
     */
    EGS_BaseSource(EGS_Input *input, EGS_ObjectFactory *f = 0) :
        EGS_Object(input,f) {};
    virtual ~EGS_BaseSource() {};

    /*!  \brief Get a short description of this source.
     *
     *  Derived source classes should set #description to a short
     *  string describing the source type.
     */
    const char *getSourceDescription() const {
        return description.c_str();
    };

    /*! \brief Sample the next source particle from the source probability
     *  distribution.
     *
     *  This is the main interface function of a particle source.
     *  It must be implemented in derived classes to sample a particle
     *  from the source probability distribution
     *  using the random number generator \a rndm and to set
     *  \a q to the particle charge, \a latch to the particle latch,
     *  \a E to the particle kinetic energy, \a wt to the particle
     *  statistical weight and \a x and \a u to the particle position and
     *  direction. The return value must be the number of statistically
     *  independent particles sampled so far. For simple sources this is
     *  typically equal to the number of particles sampled, but this may
     *  not be the case for more complicated sources (\em e.g. for
     *  a phase-space file source of a treatment head simulation the
     *  statistically independent particles are the electrons impinging
     *  on the bremsstrahlung target or vacuum exit window, and not individual
     *  particles in the phase-space file) or sources that implement some sort
     *  of systematic sampling of the beam area.
     */
    virtual EGS_I64 getNextParticle(EGS_RandomGenerator *rndm,
                                    int &q, int &latch,                // charge and latch
                                    EGS_Float &E, EGS_Float &wt,       // energy and weight
                                    EGS_Vector &x, EGS_Vector &u) = 0; // position and direction

    /*! \brief Set the next simulation chunk to start at \a nstart and
      to consist of \a nrun particles.

      This method is needed for parallel runs when using phase space files.
      It may also be re-implemented, if one wanted to use some sort of
      a systematic sampling of the phase space.
    */
    virtual void setSimulationChunk(EGS_I64 nstart, EGS_I64 nrun) { };

    /*! \brief Return the maximum energy of this source.
     *
     *  This pure virtual function must be implemented in derived classes
     *  to return the maximum energy of the source. This is needed to
     *  check if the cross section data from the PEGS file covers the
     *  energy range of the simulation and will be needed in future EGSnrc
     *  versions, where the cross section data initialization will be done
     *  at run time, to define the necessary energy range for the cross section
     *  data
     */
    virtual EGS_Float getEmax() const = 0;

    /*!  \brief Return the fluence this source has emitted so far.
     *
     * This pure virtual function must be implemented in derived classes
     * to return the particle "fluence" emitted so far by the source.
     * Every source can have their own definition of "fluence".
     * The particle "fluence" will be typically used in EGSnrc C++ user
     * codes for normalization purposes. Some of the sources provided with
     * the EGSnrc C++ class library define this
     * quantity as real fluence (\em i.e. particles per unit area), whereas
     * others define it as the number of statistically independent particles
     * delivered so far (\em e.g. the phase-space file source and the BEAM
     * simulation source).
     */
    virtual EGS_Float getFluence() const = 0;

    /*! \brief Get the time of emission for the most recently sampled particle
     *
     * This method is only reimplemented by EGS_RadionuclideSource. It
     * returns the emission time of the particle that was most recently sampled.
     */
    virtual double getTime() const {};

    /*! \brief Get the shower index for radionuclide emissions
     *
     * This method is only reimplemented by EGS_RadionuclideSource. It
     * gets the index of the most recent shower.
     */
    virtual EGS_I64 getShowerIndex() const {};

    /*! \brief Prints out the sampled emissions for radionuclide spectra
     *
     * This method is only reimplemented by EGS_RadionuclideSource. It
     * prints the actual sampled intensity of each type of emission from
     * the radionuclide spectra.
     */
    virtual void printSampledEmissions() {};

    /*!  \brief Store the source state into the stream \a data_out.
     *
     * Every source should reimplement this method to store
     * the data needed to set the state of the source to its current
     * state into the stream \a data_out. This is used for restarted
     * calculations. Examples of data needed is the number of statistically
     * independent particles delivered so far by the source.
     * Should return \c true on success, \c false on failure.
     *
     * \sa setState(), addState(), resetCounter().
     */
    virtual bool storeState(ostream &data_out) const {
        return true;
    };

    /*!  \brief Set the source state based on data from the stream \a data_in.
     *
     * Every source should reimplement this method to read from the stream
     * \a data_in data previously stored using storeState() and to set its
     * state according to this data. This is used for restarted calculations.
     * Should return \c true on success, \c false on failure (\em e.g. I/O
     * error).
     *
     * \sa addState(), storeState(), resetCounter()
     */
    virtual bool setState(istream &data_in) {
        return true;
    };

    /*! \brief Add data from the stream \a data_in to the source state.
     *
     * This method is required for combining the results of parallel runs.
     * It should therefore be re-implemented in derived classes to update
     * its own state with the data read from the input stream \a data_in.
     * For instance, if the total number of statistically independent particles
     * delivered so far is the only data the source needs, then this number
     * should be read from the stream \a data_in and added to the number
     * of statistically independent particles delivered by this source.
     *
     * \sa storeState(), setState(), resetCounter().
     */
    virtual bool addState(istream &data_in) {
        return true;
    };

    /*! \brief Reset the source state.
     *
     * Derived sources should reimplement this method to reset all data
     * describing their state to a "pristine" state (\em i.e. zero particles
     * delivered so far). This is needed for combining the results of parallel
     * runs where the generic implementation of
     * EGS_Application::combineResults() uses this method to reset the source
     * state and then uses the addState() function to add the source data from
     * all other parallel jobs.
     *
     */
    virtual void resetCounter() {};

    /*! \brief Create sources from the information pointed to by \a input.
     *
     * This static function creates all sources specified by the information
     * stored in an EGS_Input object and pointed to by \a inp. It looks
     * for a composite property <code>source definition</code> in the input
     * tree. If such property exists, it looks for sub-properties
     * \c source and for each such property that contains a \c library
     * and \c name key-value pairs, loads the DSO specified by the \c
     * library key, resolves the address of the \c createSource function
     * that must be provided by the DSO and calls this function passing the
     * \c source property to it. If the \c source property contains a valid
     * information sufficient to create a particle source of the given type,
     * the \c createSource creates the source and returns a pointer to it.
     * The process is continued until there are no further \c source
     * properties in the <code>source definition</code> input.
     * All sources created in this way are added to a global list
     * list of particle sources and can be retrieved later by name using
     * the getSource() static function. The return value of this function is
     * - \c null, if there was no valid source input
     * - A pointer to the last source created, if there was no
     *   <code>simulation source</code> key in <code>source definition</code>
     * - A pointer to the source specified by name using the
     *   <code>simulation source</code> or \c null, if such source does
     *   not exist.
     *
     */
    static EGS_BaseSource *createSource(EGS_Input *);

    /*! \brief Get a pointer to the source named \a Name.
     *
     * A static list of sources created so far is maintained internally
     * and this method can be used to get a source with a given name.
     * Returns a pointer to the soure, if a source named \a Name exists,
     * \c null otherwise.
     *
     */
    static EGS_BaseSource *getSource(const string &Name);

    /*! \brief Add a known source object to the source factory.
     *
     * This function adds the object \a o to the list of known sources
     * maintained internally by the static source factory. That way, an
     * application can define its own particle sources (in addition to
     * the sources provided by egspp) and use them.
     */
    static void addKnownSource(EGS_BaseSource *o);

    /*! \brief Add a known source object typeid to the source factory.
     *
     * For whatever reason dynamic_cast to EGS_BaseSource* from EGS_Object*
     * fails when an application is made into a shared library and
     * dynamically loads a source DSO. I'm therefore adding this method
     * so that source classes can add their typeid to allow for an additional
     * check in such cases.
     */
    static void addKnownTypeId(const char *name);

protected:

    /*! \brief A short source description.
     *
     * Derived source classes should set this data member to a short
     * descriptive string.
     */
    string description;

};

/*! \brief Base class for energy spectra. All energy spectra in the EGSnrc
 *  C++ class library are derived from this class.
 *
 *  \ingroup Sources
 *  \ingroup egspp_main
 *
 *  Objects derived from this class are used as the particle spectrum in
 *  sources derived from EGS_BaseSimpleSource. The following spectra are
 *  available:
 *  \link EGS_MonoEnergy a monoenergetic spectrum\endlink,
 *  \link EGS_GaussianSpectrum a Gaussian spectrum\endlink,
 *  \link EGS_DoubleGaussianSpectrum a double-Gaussian spectrum\endlink,
 *  \link EGS_UniformSpectrum a uniform spectrum\endlink and
 *  \link EGS_TabulatedSpectrum a tabulated spectrum\endlink.
 *  \link EGS_RadionuclideSpectrum a radionuclide spectrum\endlink.
 *
 */
class EGS_EXPORT EGS_BaseSpectrum {

public:

    /*! \brief Constructor.
     *
     * Initializes the counters counters #count, #sum_E and #sum_E2 to zero
     * and sets the spectrum description string #type to 'Unknown spectrum'.
     */
    EGS_BaseSpectrum() : count(0), sum_E(0), sum_E2(0),
        type("Unknown spectrum") {};

    /*! \brief Destructor. Does nothing. */
    virtual ~EGS_BaseSpectrum() {};

    /*! \brief Get the spectrum type.
     *
     * Returns a reference to the protected data member #type, which should
     * be set to a short string describing the type of the spectrum by
     * derived classes.
     */
    const string &getType() const {
        return type;
    };

    /*! \brief Sample a particle energy.
     *
     * This method uses the random number generator \a rndm and the protected
     * virtual method sample() to sample and return a particle energy.
     * It also updates the counters #count, #sum_E and #sum_E2.
     */
    inline EGS_Float sampleEnergy(EGS_RandomGenerator *rndm) {
        EGS_Float e = sample(rndm);
        count++;
        sum_E += e;
        sum_E2 += e*e;
        return e;
    };

    /*! \brief Get the charge for the most recently sampled particle
     *
     * This method is only reimplemented by EGS_RadionuclideSpectrum. It
     * returns the charge of the particle that was most recently sampled
     * using sampleEnergy().
     */
    virtual int getCharge() const {};

    /*! \brief Get the time of emission for the most recently sampled particle
     *
     * This method is only reimplemented by EGS_RadionuclideSpectrum. It
     * returns the emission time of the particle that was most recently sampled
     * using sampleEnergy().
     */
    virtual double getTime() const {};

    /*! \brief Set the maximum time of emission
     *
     * This method is only reimplemented by EGS_RadionuclideSpectrum. It
     * sets the maximum time, or the length of the experiment for
     * simulations using the time variable.
     */
    virtual void setMaximumTime(double maxTime) {};

    /*! \brief Get the shower index for radionuclide emissions
     *
     * This method is only reimplemented by EGS_RadionuclideSpectrum. It
     * gets the index of the most recent shower produced using sampleEnergy().
     */
    virtual EGS_I64 getShowerIndex() const {};

    /*! \brief Get the spectrum weight for radionuclide spectra
     *
     * This method is only reimplemented by EGS_RadionuclideSpectrum. It
     * gets the weight of the spectrum to balance emissions from multiple
     * spectra.
     */
    virtual EGS_Float getSpectrumWeight() const {};

    /*! \brief Set the spectrum weight for radionuclide spectra
     *
     * This method is only reimplemented by EGS_RadionuclideSpectrum. It
     * sets the weight of the spectrum to balance emissions from multiple
     * spectra. This allows a source to normalize the spectrum weights.
     */
    virtual void setSpectrumWeight(EGS_Float newWeight) {};

    /*! \brief Prints out the sampled emissions for radionuclide spectra
     *
     * This method is only reimplemented by EGS_RadionuclideSpectrum. It
     * prints the actual sampled intensity of each type of emission from
     * the radionuclide spectra.
     */
    virtual void printSampledEmissions() {};

    /*! \brief Get the maximum energy of this spectrum.
     *
     * This pure virtual method must be reimplemented by derived classes
     * to return the maximum spectrum energy.
     */
    virtual EGS_Float maxEnergy() const = 0;

    /*! \brief Get the average energy of the spectrum.
     *
     * This pure virtual method must be reimplemented by derived classes
     * to return the expected average energy of the spectrum.
     */
    virtual EGS_Float expectedAverage() const = 0;

    /*! \brief Store the state of the spectrum object into the stream
     * \a data_out.
     *
     * The default implementation of this method stores the counters
     * #count, #sum_E and #sum_E2 and returns \c true, if the operation
     * succeeded, \c false otherwise. Derived spectrum classes may add
     * additional information to the stream, if needed.
     *
     * storeState(), setState(), addState() and resetCounter() are needed
     * for restarted calculations and for combining the results of
     * parallel runs.
     */
    virtual bool storeState(ostream &data_out) const {
        if (!egsStoreI64(data_out,count)) {
            return false;
        }
        data_out << " " << sum_E << " " << sum_E2 << endl;
        if (!data_out.good() || data_out.fail()) {
            return false;
        }
        return true;
    };

    /*! \brief Set the state of the spectrum object from the data in
     * the stream \a data_in.
     *
     * The default implementation of this method is to read the data
     * stored using storeState() (\em i.e. #count, #sum_E and #sum_E2)
     * and to return \c true, if the operation succeeded, \a false
     * otherwise. Derived spectrum classes should re-implement this method
     * if they are storing additional data in the storeState() method.
     *
     * storeState(), setState(), addState() and resetCounter() are needed
     * for restarted calculations and for combining the results of
     * parallel runs.
     */
    virtual bool setState(istream &data_in) {
        if (!egsGetI64(data_in,count)) {
            return false;
        }
        data_in >> sum_E >> sum_E2;
        if (data_in.eof() || !data_in.good() || data_in.fail()) {
            return false;
        }
        return true;
    };

    /*! \brief Add to the state of this object the data from the stream
     * \a data_in.
     *
     * The default implementation of this method adds to #count, #sum_E
     * and #sum_E2 the corresponding data read from the \a data_in.
     * Derived spectrum classes should re-implement this method if they
     * are storing extra information using storeState().
     *
     * storeState(), setState(), addState() and resetCounter() are needed
     * for restarted calculations and for combining the results of
     * parallel runs.
     */
    virtual bool addState(istream &data_in) {
        EGS_I64 count_save = count;
        double sum_E_save = sum_E, sum_E2_save = sum_E2;
        if (!setState(data_in)) {
            return false;
        }
        count += count_save;
        sum_E += sum_E_save;
        sum_E2 += sum_E2_save;
        return true;
    };

    /*! \brief Reset the state of this spectrum object.
     *
     * The defualt implementation of this method sets #count, #sum_E
     * and #sum_E2 to zero. It should be re-implemented by derived classes
     * if additional data is needed to describe the state of the spectrum
     * object to reset this data as well.
     *
     * storeState(), setState(), addState() and resetCounter() are needed
     * for restarted calculations and for combining the results of
     * parallel runs.
     */
    virtual void resetCounter() {
        count = 0;
        sum_E = 0;
        sum_E2 = 0;
    };

    /*! \brief Create and return a pointer to a spectrum object from the
     * information pointed to by \a inp.
     *
     * This static method looks for a property named <code>spectrum</code>
     * in the input tree pointed to by \a inp and if such property exists,
     * creates a spectrum object according to the information it contains.
     * Otherwise \c null is returned.
     * The <code>spectrum</code> property must define the spectrum type
     * using a <br><code>
     * type = type of the spectrum <br>
     * </code> key-value pair and has enough information needed by the
     * spectrum type being constructed.
     */
    static EGS_BaseSpectrum *createSpectrum(EGS_Input *inp);

    /*! \brief Get the average sampled energy and its statistical uncertainty.
     *
     * This function assigns the average energy sampled so far to \a e and
     * its statistical uncertainty to \a de.
     */
    void getSampledAverage(EGS_Float &e, EGS_Float &de) const {
        if (count > 1) {
            e = sum_E/count;
            de = sum_E2/count;
            de -= e*e;
            if (de > 0) {
                de = sqrt(de/(count-1));
            }
        }
    };

    /*! \brief Report the average energy (expected and actually sampled).
     *
     * This function prints information about the expected and actually
     * sampled average energy of the spectrum using egsInformation().
     */
    void reportAverageEnergy() const;

protected:

    /*! \brief Sample an energy from the spectrum energy distribution.
     *
     * This pure virtual method must be implemented by derived classes to
     * sample and return a particle energy using the random number
     * generator \a rndm.
     */
    virtual EGS_Float sample(EGS_RandomGenerator *rndm) = 0;

    /*! \brief Number of times the sampleEnergy() method was called.*/
    EGS_I64 count;

    /*! \brief Sum of energies sampled so far. */
    double  sum_E;

    /*! \brief Sum of energies squared sampled so far. */
    double  sum_E2;

    /*! \brief A short string describing the spectrum that must be set by
        derived classes.
     */
    string  type;

};

/*! \brief Base class for 'simple' particle sources.
 *
 * \ingroup Sources
 * \ingroup egspp_main
 *
 * A 'simple' particle source source is a source with a fixed particle
 * charge for which the energy distribution (the spectrum) is decoupled
 * from the probability distribution for direction and position.
 * Hence, to sample a particle, this type of source samples the energy
 * from a \link EGS_BaseSpectrum spectrum object \endlink
 * and the position and direction from another,
 * energy independent distribution using getPositionDirection().
 * Except for phase space file and BEAM simulation sources, all other
 * sources ever implemented and used in NRC mortran user codes are of
 * this type. The EGS_BaseSimpleSource class is the base class for such
 * sources and implements various algorithms that are common to all
 * 'simple' sources.
*/
class EGS_EXPORT EGS_BaseSimpleSource : public EGS_BaseSource {

public:

    /*! \brief Constructor
     *
     * Construct a 'simple' particle source with charge \a Q,
     * spectrum \a Spec and name \a Name. The spectrum \em must not
     * be \c null. The newly created source takes ownership of the
     * spectrum object and will delete it in the destructor.
     */
    EGS_BaseSimpleSource(int Q, EGS_BaseSpectrum *Spec,
                         const string &Name="", EGS_ObjectFactory *f=0) :
        EGS_BaseSource(Name,f), q(Q), s(Spec), count(0) { };

    /*! \brief Construct a 'simple' particle source from the information
     * pointed to by \a input.
     *
     * In addition to the information needed by the
     * \link EGS_BaseSource::EGS_BaseSource(EGS_Input *,EGS_ObjectFactory *)
     * base source class constructor \endlink, \a input must contain
     * the information required to create a spectrum, a particle charge
     * key-value pair and whatever else information the derived source
     * class needs.
     */
    EGS_BaseSimpleSource(EGS_Input *input, EGS_ObjectFactory *f=0);

    /*! \brief Destructor
     *
     * Deletes the spectrum object of the 'simple' source.
     */
    ~EGS_BaseSimpleSource() {
        if (s) {
            delete s;
        }
    };

    /*! \brief Is this a valid source?
     *
     * The default implementation of this function returns \c true,
     * if the spectrum object of the source is not \c null and \c false
     * otherwise. Should be re-implemented in derived classes to check
     * that the additional prerequisites such sources need are fullfilled.
     * This is used in the template source creation function
     * createSourceTemplate() to check if the input was sufficient to
     * construct the desired source.
     */
    virtual bool isValid() const {
        return (s != 0);
    };

    /*! \brief Sample the next source particle from the source probability
     * distribution.
     *
     * Uses the \link EGS_BaseSpectrum::sampleEnergy() sampleEnergy() \endlink
     * function of the spectrum object to obtain the particle energy,
     * the pure virtual function getPositionDirection() to obtain the
     * position and direction, and the virtual setLatch() function to obtain
     * a particle latch. Hence, particle source derived from
     * EGS_BaseSimpleSource only need to implement these two functions.
     * Increments #count by one.
     */
    virtual EGS_I64 getNextParticle(EGS_RandomGenerator *rndm,
                                    int &Q, int &latch, EGS_Float &E, EGS_Float &wt,
                                    EGS_Vector &x, EGS_Vector &u) {
        Q = q;
        E = s->sampleEnergy(rndm);
        getPositionDirection(rndm,x,u,wt);
        setLatch(latch);
        return ++count;
    };

    /*! \brief Sample a particle position and direction.
     *
     * This pure virtual function must be implemented in classes deriving
     * from EGS_BaseSimpleSource to sample a particle position and direction
     * using the random number generator \a rndm and to set \a x to the
     * position, \a u to the direction and \a wt to the statistical weight
     * of the particle. This function is needed by getNextParticle().
     */
    virtual void getPositionDirection(EGS_RandomGenerator *rndm,
                                      EGS_Vector &x, EGS_Vector &u, EGS_Float &wt) = 0;

    /*! \brief Get the maximum energy of the source.
     *
     * Simply uses the \link EGS_BaseSpectrum::maxEnergy() maxEnergy() \endlink
     * method of the spectrum object.
     */
    virtual EGS_Float getEmax() const {
        return s->maxEnergy();
    };

    /*! \brief Store the fluence state of this source to the data stream
     * \a data_out.
     *
     * Must be implemented in derived classes to store the data needed to set
     * the state of this source object to its current state. This is needed
     * in restarted calculations.
     *
     * \sa EGS_BaseSource::storeState(), EGS_BaseSource::setState(),
     * EGS_BaseSource::addState() and EGS_BaseSource::resetCounter().
     */
    virtual bool storeFluenceState(ostream &data_out) const {
        return true;
    };

    /*! \brief Store the source state to the data stream \a data_out.
     *
     * Uses the \link EGS_BaseSpectrum::storeState() storeState() \endlink
     * of the spectrum object and the storeFluenceState() virtual function.
     */
    virtual bool storeState(ostream &data_out) const {
        if (!egsStoreI64(data_out,count)) {
            return false;
        }
        if (!s->storeState(data_out)) {
            return false;
        }
        if (!storeFluenceState(data_out)) {
            return false;
        }
        return true;
    };

    /*! \brief Add the source state from the stream \a data to the
     * current state.
     *
     * Uses the \link EGS_BaseSpectrum::addState() addState() \endlink
     * of the spectrum object and the addFluenceData() virtual function.
     */
    virtual bool addState(istream &data) {
        EGS_I64 count_save = count;
        if (!egsGetI64(data,count)) {
            return false;
        }
        if (!s->addState(data)) {
            return false;
        }
        if (!addFluenceData(data)) {
            return false;
        }
        count += count_save;
        return true;
    };

    /*! \brief Reset the source to a state with zero sampled particles.
     *
     * Uses the \link EGS_BaseSpectrum::resetCounter() resetCounter() \endlink
     * function of the spectrum object and the virtual function
     * resetFluenceCounter().
     */
    virtual void resetCounter() {
        count = 0;
        s->resetCounter();
        resetFluenceCounter();
    };

    /*! \brief Add fluence data from the stream \a data to the current state.
     *
     * This virtual function should be re-implemented in derived classes if
     * the storeFluenceState() function was re-implemented to store additional
     * data.
     *
     * \sa storeFluenceState(), setFluenceState(), resetFluenceCounter(),
     * setState(), storeState(), resetCounter() and addState().
     */
    virtual bool addFluenceData(istream &data) {
        return true;
    }

    /*! \brief Reset the data related to the sampling of positions and
     * directions to a state with zero sampled particles.
     *
     * This virtual method should be re-implemented in derived classes to
     * reset all data associated with the sampling of particle positions and
     * directions (if there is such data).
     *
     * \sa storeFluenceState(), setFluenceState(), addFluenceData(),
     * setState(), storeState(), resetCounter() and addState().
     */
    virtual void resetFluenceCounter() { };

    /*! \brief Set the data related to the sampling of positions and
     * directions to a state contained in the stream \a data.
     *
     * This virtual method should be re-implemented in derived classes to
     * set the data associated with the sampling of particle positions and
     * directions to the state extracted from the data stream \a data.
     *
     * \sa storeFluenceState(), addFluenceState(), addFluenceData(),
     * setState(), storeState(), resetCounter() and addState().
     */
    virtual bool setFluenceState(istream &data) {
        return true;
    };

    /*! \brief Set the source state according to the data in the stream \a data.
     *
     * Uses the \link EGS_BaseSpectrum::setState() setState() \endlink
     * method of the spectrum object and the setFluenceState() virtual
     * function.
     */
    virtual bool setState(istream &data) {
        if (!egsGetI64(data,count)) {
            return false;
        }
        if (!s->setState(data)) {
            return false;
        }
        if (!setFluenceState(data)) {
            return false;
        }
        return true;
    };

protected:

    /*! Set the particle latch.
     *
     * This virtual function can be re-implemented in derived classes to
     * set the particle latch according to some condition. The default
     * implementation sets the latch to zero.
     */
    virtual void setLatch(int &latch) {
        latch = 0;
    };

    /*! \brief The charge of this simple source */
    int              q;

    /*! \brief The energy spectrum of this source. */
    EGS_BaseSpectrum *s;

    /*! \brief A short description of the source type. */
    string           type;

    /*! \brief Number of statistically independent particles delivered so far.*/
    EGS_I64          count;

};

/*! \brief A template source creation function.
 *
 * This template function is used by the \c %createSource functions of the
 * various source dynamic shared objects provided with the EGSnrc C++
 * class library to create sources from a given input. The variable
 * \a name should point to a null-terminated string that will be used
 * to output warnings in the form "createSource(name):..." if
 * the creation of a source fails.
 */
template <class T>
EGS_BaseSource *createSourceTemplate(EGS_Input *input,
                                     EGS_ObjectFactory *f, const char *name) {
    EGS_BaseSource::addKnownTypeId(typeid(T).name());
    if (!input) {
        egsWarning("createSource(%s): null input?\n",name);
        return 0;
    }
    T *res = new T(input,f);
    if (!res->isValid())  {
        egsWarning("createSource(%s): the input is not "
                   "sufficient to create a valid source\n",name);
        delete res;
        return 0;
    }
    return res;
};

#endif

/*! \example sources/egs_point_source/egs_point_source.cpp

A point source is a special case of an
\link EGS_IsotropicSource isotropic source \endlink.
Due to its simplicity, it provides a good example for discussing
the implementation of a new source class.

The point source header file \c %egs_point_source.h includes
the base source header file, the header file of the EGS_Vector
class to gain access to the methods manipulating 3D vectors, and
the random number generator (RNG) header file to get access to the
declaration of the various RNG methods needed to sample particles
\dontinclude sources/egs_point_source/egs_point_source.h
\skipline egs_base_source.h
\until egs_rndm.h
It then defines a macro \c EGS_POINT_SOURCE_EXPORT that will be used
to mark DSO symbols as exports when building the point source
DSO and as imports when linking against the DSO:
\until EGS_POINT_SOURCE_LOCAL
\until EGS_POINT_SOURCE_LOCAL
\until endif
\until endif

We derive the point source class from EGS_BaseSimpleSource
instead of EGS_BaseSource because a point source is a "simple"
source (\em i.e consists of a spectrum and an energy-independent
probability distribution for position and direction):
\until public:
The private variables \c xo (point source position) and
\c valid will be set in the constructors (see below).
We provide two methods for constructing a point source: directly
by passing a charge, a spectrum, a source position and a name
to the constructor,
\until };
and from the information provided by an EGS_Input object
\until EGS_Input
The implementation of the second constructor is in the
.cpp file and will be discussed below.

The main method that must be implemented in a derived simple source
class is \link EGS_BaseSimpleSource::getPositionDirection
getPositionDirection() \endlink to sample the position and direction
of a single particle and to set its statistical weight
\skipline getPositionDirection
\until {
Sampling the particle position for a point source is easy: it is simply
the point source position
\until xo
Sampling the direction is also easy: one samples \f$u_z\f$ uniformely
between -1 and 1, picks a random azimuthal angle \f$\phi\f$ between
0 and \f$2 \pi\f$ and sets \f$ u_x = \sqrt{1 - u_z^2} \cos \phi,
u_y = \sqrt{1 - u_z^2} \sin \phi\f$:
\until else
The statistical weight of the particle is always unity
\until };

The next function to implement is \c %getFluence(), which should return
the particle "fluence" emitted so far by the source. We arbitrarily
decide to define the "fluence" of our point source to be the
number of particles emitted by the source (we also could have
picked the number of particles per unit solid angle or real fluence
at a certain distance from the source) and therefore the
\c %getFluence() implementation is very easy:
\until };
Here, \c count is a protected data member inhereted from
EGS_BaseSimpleSource, which is initialized to zero in the
simple source constructor and is incremented by one in each
invocation of the \link EGS_BaseSimpleSource::getNextParticle()
getNextParticle() \endlink function.

The next task is to implement the functions that store the state
of a point source object to a data stream and restore the state
from a data stream. This is needed for the ability to restart
simulations. Because our source class is very simple, no
data needs to be stored/restored in addition to the data already
stored/restored by EGS_BaseSource and EGS_BaseSimpleSource.
The implementation of these functions is therefore extremely simple:
\until setFluenceState

The next step in the header file is to provide a \c %isValid
function, which should return \c true, if the source object is
valid and \c false otherwise (\em e.g. it does not have a spectrum).
\c %isValid() is needed so that the \c %createSource() function that
must be provided by the source DSO (see below) can be implemented by
simply using the template createSourceTemplate():
\until isValid()

The final step in the point source declaration is to specify a
protected function that will be used to set up the source type
and description
\until };

The \c %egs_point_source.cpp file provides implementation of the
constructor that creates a point source object from information
stored in an EGS_Input object, the \c %setUp() function and
the C-style source creation function \c %createSource() that must be provided
by each source DSO. It includes the point source header file and
the input object header file
\dontinclude egs_point_source.cpp
\skipline egs_point_source.h
\until egs_input.h

The point source constructor calls the
\link EGS_BaseSimpleSource::EGS_BaseSimpleSource(EGS_Input*,EGS_ObjectFactory*)
 base simple source \endlink constructor passing the input as argument
\until {
This sets the name of the source from the \c name key, sets the charge
from a \c charge key and constructs the spectrum from the information
in a \c spectrum composite property using
EGS_BaseSpectrum::createSpectrum(). The only task remaining for a
complete point source definition is to determine the position of
the source:
\until int err
The position was successfully set if \c err=0 and the number of inputs
to the \c position key is exactly 3 (defining a 3D position), otherwise
a warning is issued and the source is set to be invalid:
\until }
The source type and description is then set using \c %setUp().
\until }

The implementation of the \c %setUp function sets the type string
to "EGS_PointSource" and the description string to a short sentence
constructed from the charge and spectrum type:
\until }
\until }

The \c %createSource C-style function implementation simply uses the
createSourceTemplate() function:
\until }
\until }
The conditions for being able to use createSourceTemplate() to construct
a source are
- The source provides a constructor that takes pointers to an EGS_Input
  object and an object factory as arguments
- The source provides a \c %isValid() function returning a
  boolean value.

This completes the implementation of our point source class.

The Makefile is very similar to the Makefile of a geometry DSO
(see \em e.g. <a href="geometry_2example2_2geometry__example2_8cpp-example.html">
this example</a>): we include the egspp config setting for the make system,
\dontinclude sources/egs_point_source/Makefile
\skipline EGS_CONFIG
\until my_machine
set the pre-processor defines to include the \c BUILD_POINT_SOURCE_DLL
macro,
\until DEFS
define the DSO name and files needed to build the DSO,
\until lib_files
set the dependences to be set of header files common for all sources,
\until
extra_dep
include the rules for building a DSO,
\until include
and make the dependences take effect
\until make_depend

We can now build our point source DSO by typing \c make and use it
for simulations by including input such as
\verbatim
:start source:
    library = egs_points_source
    name = some_name
    charge = -1
    :start spectrum:
       definition of a spectrum
    :stop spectrum:
    position = 15 -3 7
:stop source:
\endverbatim
in the source definition section of the input file.

This is the complete Makefile:
\include sources/egs_point_source/Makefile
This is the complete header file:
\include sources/egs_point_source/egs_point_source.h
This is the complete source file:

*/
