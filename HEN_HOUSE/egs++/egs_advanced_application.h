/*
###############################################################################
#
#  EGSnrc egs++ advanced application headers
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
#                   Frederic Tessier
#                   Reid Townson
#
###############################################################################
*/


/*! \file egs_advanced_application.h
    \brief EGS_AdvancedApplication class header file
    \IK
*/

#ifndef EGS_ADVANCED_APPLICATION_
#define EGS_ADVANCED_APPLICATION_

#include "egs_libconfig.h"
#include "egs_application.h"
#include <vector>

class EGS_Input;
class EGS_Interpolator;

/*! \brief Base class for advanced EGSnrc applications based on the
     mortran EGSnrc back-end.

  The EGS_AdvancedApplication class provides implementation of functions that
  are specific to the mortran version of EGSnrc but independent from the user
  code. EGSnrc C++ application using the mortran version of EGSnrc should be
  derived from this class. See the EGS_Application
  class description for a quick guideline on writing EGSnrc C++ applications.

 */
class APP_EXPORT EGS_AdvancedApplication : public EGS_Application {

public:

    //
    // ********* constructor and destructor
    //
    /*! \brief Constructor.

    The EGS_AdvancedApplication constructors basically consist of a call
    to the \link EGS_Application::EGS_Application()
    EGS_Application \endlink constructor.
    */
    EGS_AdvancedApplication(int argc, char **argv);

    /*! \brief Destructor.

      Does nothing in addition to the EGS_Application destructor.
    */
    virtual ~EGS_AdvancedApplication();

    /*! \brief Turns on or off a call to the user scoring function ausgab()

      This function is re-implemented to also set the corresponding
      \c iausfl element in the \c the_epcont mortran back-end structure to
      \a on_or_off.
    */
    void setAusgabCall(AusgabCall call, bool on_or_off);

    /*! \brief Finish a simulation.

      The default implementation consists of calling the egsFinish()
      function, which removes the temporary working directory after
      moving all output to the user code directory, and the
      egsSetDefaultIOFunctions() function, which makes the output
      of egsInformation(), egsWarning() and egsFatal() to go to the
      application standard output and standard error.
    */
    virtual void finishRun();


    /*! \brief Output intermediate results.

      Re-implemented to store information from the mortran back-end
      in addition to the information sored by
      EGS_Application::outputData().
      The additional data being stored is the RNG array used by the
      mortran backend and the number of steps taken so far.
     */
    int outputData();

    /*! \brief Read intermediate results.

    Re-implemented to read the additional data stored by outputData().
    */
    int readData();

    /*! \brief Add data from a parallel job.

      Re-implemented to add the additional data stored in outputData(),
      \em i.e. the random number array used by the mortran back-end and
      the number of electron steps taken so far.
     */
    int addState(istream &);

    /*! \brief Reset the application to a 'pristine' state.

      Re-implemented to set the number of electron steps and condensed
      history steps taken to zero in addition to the data reset in
      EGS_Application::resetCounter().
     */
    void resetCounter();

    /*! \brief Finish the simulation

      Re-implemented to also output the combined results of parallel
      runs, if necessary. This part is unfortunately dependent on the
      mortran back-end and must therefore be done here instead of
      in the base class implementation. The dependence
      comes from the fact that when using the mortran back-end all I/O
      is redirected to a Fortran function so that there are no conflicts
      between I/O in the C++ portion and I/O in the mortran back-end.
     */
    int finishSimulation();

    /*! \brief Describe the simulation.

      Re-implemented to also output the values of the various cross section
      and trannsport parameter options.
     */
    void describeSimulation();

    /*! \brief Get the number of random numbers used.

      Re-implemented to take into account the random numbers used by
      the mortran back-end.
    */
    EGS_I64 randomNumbersUsed() const;

    /*! \brief Get the number of condensed history and all electron steps.

      Implemented using the egsGetSteps() function provided by the
      mortran back-end.
     */
    void getElectronSteps(double &ch_steps, double &all_steps) const;

    /*! Save the state of the RNG */
    virtual void saveRNGState();

    /*! Reset the RNG state */
    virtual void resetRNGState();

    void appInformation(const char *msg);
    void appWarning(const char *msg);
    void appFatal(const char *msg);

    /*! \brief Start the transport of a new particle.

     This function is called just before the transport of a new particle
     begins and should be re-implemented in derived classes.
     Its default re-implementation in EGS_AdvancedApplication is
     to get the relative mass density in the current particle region
     and pass it to the Mortran back-end part of the system.
     It can be re-implemented in derived classes to set other
     quantities such as ecut, pcut, etc. in order to have such
     quantities varying on a region-by-region bases.
    */
    void startNewParticle();
    void enterNewRegion();

    /*! \brief Custom Rayleigh data setup.

     Set media and corresponding ff file names for
     custom Rayleigh data.
    */
    void setRayleighData(const vector<string> &str_medium,
                         const vector<string> &str_file);
    /*! \brief Set EII flag and xsection file name.

    The EII input is of a mixed type, i.e., one should be able to turn
    this option on/off, but there is also the possibility of using an
    arbitrary EII xsection compilation, including the available EII
    xsections with EGSnrc.

    */
    void setEIIData(EGS_I32 len);

    //************************************************************
    // Utility functions for use with ausgab dose scoring objects
    //************************************************************
    EGS_Float getMediumRho(int ind);
    EGS_Float getEdep();
    void setEdep(EGS_Float edep);
    EGS_Float getEcut();
    EGS_Float getPcut();
    /* Needed by some sources */
    EGS_Float getRM();

protected:

    int              nmed;      //!< number of media
    EGS_Interpolator *i_ededx;  //!< electron stopping power interpolator
    EGS_Interpolator *i_pdedx;  //!< positron stopping power interpolator
    EGS_Interpolator *i_esig;   //!< electron cross section  interpolator
    EGS_Interpolator *i_psig;   //!< positron cross section  interpolator
    EGS_Interpolator *i_ebr1;   //!< electron branching      interpolator
    EGS_Interpolator *i_pbr1;   //!< positron branching 1    interpolator
    EGS_Interpolator *i_pbr2;   //!< positron branching 2    interpolator

    EGS_Interpolator *i_gmfp;   //!< photon mean-free-path interpolator
    EGS_Interpolator *i_gbr1;   //!< photon branching 1 interpolator
    EGS_Interpolator *i_gbr2;   //!< photon branching 2 interpolator
    EGS_Interpolator *i_cohe;   //!< photon Rayleigh interpolator
    EGS_Interpolator *i_photonuc;   //!< photonuclear interpolator

    int n_rng_buffer;           //!< Size of the RNG buffer
    int i_rng_buffer;           //!< Pointer to the RNG buffer
    EGS_Float *rng_buffer;      //!< RNG buffer

    /*! \brief Initialize the EGSnrc mortran back-end.

      This function transfers the various file and directory names,
      number of pareallel jobs, parallel job index and the batch \em vs
      interactive run flag, obtained in the EGS_Application constructor
      from the command line arguments, to the appropriate EGSnrc
      common block and calls the \c egs_init1 mortran subroutine.
      If the simulation is a batch run and therefore all output should
      go to a log file instead of standard output/error, the egsInformation,
      egsWarning and egsFatal variables are changed to point to functions
      which redirect the output to a Fortran subroutine.
    */
    int initEGSnrcBackEnd();

    /*! \brief Initialize the run-time cross section data.

      This function initializes the cross section data and transport parameter/
      cross section options. It first looks for transport parameter input
      within a section of the input file delimited by <code>
      MC transport parameter</code> and if found, sets parameter such as
      <code> ecut, pcut, smax, ibr_nist </code>, etc. It then adds all
      media present in the geometry to the mortran back-end using
      egsAddMedium() and calls the \c HATCH subroutine. If this succeeds,
      the various interpolators (#i_ededx, #i_pdedx, etc) are initialized
      to use the cross section data just obtained. Finally, information
      about the media found in the geometry along with their cutoff energies
      and the values of all transport parameter and cross section
      options are printed using egsInformation.
    */
    int initCrossSections();

    /*! \brief Simulate a single shower.

      The default implementation of this function is to put the particle
      found in the protected data member #p into the mortran particle
      stack as the first and only particle and to call egsShower().
    */
    int shower();

    bool final_job;  //!< Is this the final job of a parallel run ?

    /*! \brief Helper function used in initCrossSections() and
      describeSimulation(). */
    int  helpInit(EGS_Input *, bool do_hatch);

    /*! \brief Holds the CVS revision number of the
      egs_advanced_application.cpp file. */
    static string base_revision;

    int    io_flag; //!< determines how to write info

};

#endif

/*! \example tutor7pp.cpp
    \dontinclude tutor7pp.cpp

    The tutor7pp application is a re-implementation of
    the original tutor7 mortran application in C++ using
    EGS_AdvancedApplication. It calculates the deposited, transmitted
    and reflected energy fractions in addition to the pulse height
    distribution in requested geometry regions. However,
    unlike the original tutor7 application, which can only calculate
    these quantites for a slab geometry for a pencil beam incident
    at a user defined angle, tutor7pp can perform the simulation in
    any geometry that can be defined using the \c egspp
    \ref Geometry "geometry package" for any particle source that can be
    defined using the \c egspp \ref Sources "source package".
    In addition, tutor7pp provides the functionality of being able
    to restart a calculation, run a calculation in parallel and
    automatically recombine the results, limit the simulation time
    to a user defined maximum time, or abort the simulation if
    a user defined statistical uncertainty has been reached.
    Despite this added functionality, the tutor7pp source code is
    much shorter than the original tutor7 implementation in mortran.
    The reader is encouraged to study the guidelines for
    developing EGSnrc C++ applications provided in the
    \link EGS_Application EGS_Application class documentation\endlink
    before reading this example.

    To implement the required functionality we derive an application
    class from EGS_AdvancedApplication
    \skipline #include
    \until class APP_EXPORT Tutor7_Application
    The various include files are needed to gain access to the
    declarations of the classes and functions that will be used.
    We then declare various private data members:
    \until string revision
    Their function is briefly explained by the comments and will
    become more clear in what follows. We now need a constructor
    for our application class that takes the command line arguments
    as input, passes them to the base class constructor and
    initializes its data
    \until };
    The call to the EGS_AdvancedApplication constructor
    checks the arguments passed on the command line and sets up the name of the
    application, the input file name, the pegs file name, the output file name,
    the working directory, batch vs interactive run,
    parallel job execution (i_parallel and n_parallel) and reads in the input
    file into an EGS_Input object (a pointer to this object is available with the
    protected data member EGS_Application::input). To demonstrate good coding
    habits, we declare a destructor for our application that deallocates the
    memory used
    \until };
    We now declare the various virtual functions that we wish to re-implement:
    the function to describe our user code,
    \until void describeUserCode
    the function to initialize the scoring variables,
    \until int initScoring
    the function for scoring the quantities of interest at run time,
    \until int ausgab
    the functions needed for restarting simulations and parallel runs,
    \until int addState
    the function to output the results of the simulation,
    \until void outputResults()
    the function that makes the current result known to the
    \link EGS_RunControl run control object\endlink,
    \until );
    and the protected function called before the simulation of a new particle
    \until };
    This completes our application class declaration.

    We now provide implementations of the above functions. First
    we have to declare the static data member \c %revision:
    \until ::revision
    Note that the revision string is automatically updated by CVS,
    which we use for version control at the NRC, each time we commit
    a new version of the tutor7pp application. The \c %describeUserCode()
    function simply outputs the name of our application and the revision
    numbers of the base application class and our application class using
    egsInformation:
    \until }
    This is useful for determining which
    version was used to obtain a given result from the
    logfile produced by the application. The egsSimplifyCVSKey function used
    above removes the \$'s from CVS keys.

    The next step is to initialize the scoring variables in
    \c %initScoring(). We will need a scoring array object to
    collect energy depositions in all regions of the geometry and
    the outside region (divided into energy transmitted, defined as
    the energy of particles exiting the geometry and moving forward, and
    energy reflected, defined as the energy of particles exiting
    the geometry and moving backwards)
    from which we can calculate energy deposition fractions. For this purpose
    we obtain the number of regions in the geometry
    \until initScoring
    \skipline nreg
    and initialize a scoring array with \c nreg+2 regions pointed to
    by our class variable \c score:
    \until = new
    We want the user to be able to specify the geometry regions for
    which to calculate a pulse height distribution (PHD)
    by including
    \verbatim
    :start scoring options:
        pulse height regions = 1 5 77
        pulse height bins = 125 50 500
    :stop scoring options:
    \endverbatim
    in their input file, with the <code>pulse height regions</code> key
    specifying all regions for which a PHD calculation is requested and
    the <code>pulse height bins</code> key the number of bins to use
    for each PHD region (obviously, the number of integer inputs to these
    two keys must be the same). To implement this functionality,
    we interogate the \link EGS_Application::input input object \endlink,
    which contains all input found in the input file, for a composite key
    named <code>scoring options</code>
    \skipline options
    and if such property exists (\em i.e. \c options is not \c null),
    check for the <code>pulse height regions</code> and
    <code>pulse height bins</code> keys:
    \skipline vector
    \until int err1
    If the keys exist (\em i.e. \c err and \c err1 are zero),
    we check that the number of inputs is the same, or, alternatively
    there is only a single input to the <code>pulse height bins</code>
    key (in this case we use the same number of bins for all PHDs),
    and if this is not satisfied issue a warning so that
    the user is made aware of a potential mistake in the input file,
    \until );
    otherwise we have to initialize scoring arrays for the
    requested PHDs. We first alocate a temporary array of pointers to scoring
    objects and set all of them to \c null:
    \until = 0;
    We then loop over the number of PHD regions
    \until {
    For each requested PHD region we set the number of bins
    (possibly warning the user, if the number is less than 1),
    \until );
    and check that the region index is valid
    \until );
    If the number of bins is positive and the region index is valid,
    we check if there is not already a PHD scoring object for this region
    initilized and warn the user if there is
    \until );
    (perhaps the user has made a mistake and input the same PHD region
    twice), otherwise we construct a scoring array with \c nb bins,
    increment the number of requested PHDs by one and store the
    pointer to the newly constructed scoring array in \c tmp:
    \until }
    \until }
    When the loop over requested PHD regions is completed, \c nph has
    the number of valid PHD requests and the elements of \c tmp
    that are not \c null have the pointers to the scorring array objects
    allocated for the calculation of the PHDs. We then allocate an
    array of pointers to scorring array objects with the required size,
    \until pheight
    an array with region indeces for each requested PHD,
    \until ph_regions
    and an array with bin widths for each PHD
    \until ph_de
    We then obtain the maximum source energy
    \until Emax
    and loop over all regions
    \until {
    If \c tmp[j] is not \c null, we set the \c iph't element
    of the just allocated arrays to contain the pointer to the
    scorring array, the region index of the PHD, and the bin width:
    \until }
    \until }
    \until }
    We then delete \c tmp to avoid memory leaks
    \until }
    This completes the initilization of PHD scoring.
    If the user has provided <code>scoring options</code> input but
    the input is incorrect (indicated by \c err and/or \c err1 not being
    zero), we warn them so that they can check their input file
    \until );
    We then delete the <code>scoring options</code> input as it is no longer
    needed
    \until return 0
    \until }
    (Note: if we were expecting that someone might want to extend
    the functionality of our tutor7pp application by deriving from the
    Tutor7_Application class, we should have introduced a protected data member
    to point to the <code>scroring options</code> property and should not
    delete it here. In this case we also should have split the
    class declaration into a separate header file).
    This completes the \c %initScoring() implementation.

    The implementation of our \c %ausgab() function is very simple:
    for enertgy deposition events (\em i.e. \c iarg<=4)
    we determine the region index of the top particle on the particle stack
    and score the energy deposited weighted with the statistical weight
    of the particle into the corresponding scoring region using
    the \link EGS_ScoringArray::score() score() \endlink
    method of our scoring array object:
    \until }
    \until }
    The calculation of requested pulse height distributions will be done
    in the \c %startNewShower() function (see below).

    The \c %outputData() function must store the accumulated results to a
    data stream (click on \c %outputData() below to see the base class
    documentation of this virtual function). We use the base class
    implementation to store the data of the base application and
    return the generated error code, if an error occured:
    \until Tutor7_Application::outputData
    \skipline int err =
    \until return err
    We then write our own data to the data stream pointed to by
    \link EGS_Application::data_out data_out\endlink:
    the total energy deposited so far
    \skipline << endl
    the energy fractions accumulated in the scorring array
    \until storeState
    and the pulse height distribution results
    \until }
    \until }
    Note that \link EGS_Application::data_out data_out\endlink is
    set up to point to the output stream created by opening the
    <code>.egsdat</code> file for writing in the base application class.

    The function directly innverse to \c %outputData() is \c %readData().
    The implementation is therefore basically the same as \c %outputData()
    except that now we read the data from the stream pointed to by
    \link EGS_Application::data_in data_in\endlink:
    \until Tutor7_Application::readData
    \skipline int err =
    \until return err
    \skipline >> Etot
    \until }
    \until }

    The \c %resetCounter() function, which is needed for combining parallel runs
    and is called before the loop over available data files, must put our
    application class into a state with zero particles simulated. We therefore
    use the base class function to reset the data of the base application class,
    \until Tutor7_Application::resetCounter
    \skipline resetCounter
    set the total energy deposited to zero and use the
    \link EGS_ScoringArray::reset reset() \endlink functions of the scoring
    arrays to reset these:
    \skipline Etot
    \until }

    The \c %addState() function is used to combine the results of parallel runs.
    It is therefore similar to the \c %readData() function except that now
    the data read from the input stream is added to the existing data instead
    of replacing the state of the application with these data.
    We therefore use the base class \c %addState() function,
    \until Tutor7_Application::addState
    \skipline nt err =
    \until return err
    add the total energy deposited into the geometry,
    \skipline etot_tmp
    construct a temporary scoring array object with \c nreg+2 regions,
    \until EGS_ScoringArray tmp
    read the data into this object,
    \until setState
    and add it to the existing data
    \until score
    We do the same for the pulse height distributions:
    \until }
    \until }

    The \c %outputResults() function must output the results of a simulation
    into the logfile (or the standard output, if the application is run
    interactively). In our implementation we first inform the user about the
    last statistically independent history simulated and the total energy
    deposited into the geometry:
    \until Etot)
    We then use the \link EGS_ScoringArray::reportResults() reportResults()
    \endlink method of the energy fractions scoring array to report
    the deposited energy fractions:
    \until );
    Because the scoring array object divides the results by the number of
    statistically independent showers (\c %last_case) but we want the normalization
    to be fraction of the energy deposited, we set the normalization constant
    to be <code>last_case/Etot</code>. The string passed as a second argument
    will be output as a title and the \c false argument indicates that we
    want the uncertainties to be absolute instead of relative as percent of
    the local result. The last argument is a format string used to output
    the result. If omitted or set to \c null, a default format string
    will be used (using a %g format for outputing the floating numbers).
    If the user has requested the calculation of one or more
    pulse height distributions (\em i.e. \c nph>0), we output a title
    depending on the number of distributions calculated:
    \until );
    \until );
    and then in a loop over the number of pulse height distributions
    calculated
    \until );
    we have a loop over the number of bins for the pulse height distribution
    in this region, obtain the result from the scoring array and
    output a data triplet consisting of the midpoint energy of the bin,
    the result in the bin and the uncertainty of the result:
    \until }
    \until }
    \until }
    \until }
    Because the scoring array object already divides by the number of
    statistically independent events, we only divide by the bin width
    to have the results normalized as counts per incident particle per MeV.

    The \c %getCurrentResult() function is used by the
    \link EGS_RunControl run control object \endlink to provide
    a single result for our simulation that is the combined
    result from all jobs in parallel runs. We arbitrarily decide
    to define the single result of our simulation monitored at run time
    as the energy fraction reflected (stored in the zeroth region of
    the \c score scoring array). Hence, the implementation of this
    function looks like this:
    \until }

    In the \c %startNewShower() function we must collect the
    energy being imparted into the geometry and inform our scoring
    objects when the simulation of a new statistically independent
    event begins. In addition, we implement the scoring of the
    requested pulse height distributions (PHD) here.
    We start by collecting the energy entering the geometry
    \until +=
    The parameters of the particle that is about to be simulated are
    available in the EGS_Application::p protected data member.
    We then check if the particle about to be simulated
    is statistically independent from the last particle simulated
    by comparing the inherited protected data members
    EGS_Application::current_case and EGS_Application::last_case:
    \until {
    If they are the same, then the two particles belong to the
    same statistically independent case and we don't need to do
    anything. This can happen \em e.g. when using a
    \link EGS_PhspSource phase space file \endlink or a
    \link EGS_BeamSource BEAM simulation source \endlink,
    see \em e.g. [Walters \em et.al., Med.Phys. 29 (2002) 2745]
    for a detailed discussion of the correct implementation of
    a history-by-history statistical analysis.
    If the user has requested the calculation of PHDs
    (\em i.e. \c nph>0), we loop over all
    requested PHDs
    \until j<nph
    inform the scoring array used to collect this PHD that a
    new statistically independent event is about to begin,
    \until setHistory
    obtain the energy deposited in the PHD region
    in the last statistically independent shower,
    \until edep
    and, if this energy is greater than zero, determine
    the PHD bin to which such an energy deposition belongs
    and add a count to this PHD bin
    \until }
    \until }
    \until }
    Note that in the above we divide by \c current_weight.
    This is needed because all energy depositions are weighted
    with the particle weights that may not be constant and also
    may be different from unity (\em e.g. for a
    \link EGS_CollimatedSource collimated point source \endlink).
    We remember the statistical weight of the particles intiating
    the showers in \c current_weight (see below).
    We then inform the energy deposition scoring array that a new
    statistically independent event begins
    \until score
    and set the last simulated case to the current case
    \until }
    and remember the statistical weight of the incident particle.
    \until }

    This completes the implementation of our Tutor7_Application class.
    One needs more code compared to an application derived from
    EGS_SimpleApplication (see \em e.g.
    <a href=tutor2pp_8cpp-example.html>this example</a>).
    However, a closer examination shows that a significant part
    of the code is neede to implement the restart/parallel runs/combine
    results functionality and to provide the user with the
    capability to specify the regions for PHD calculation in
    the input file. Without this added functionality, the tutor7pp
    source code would be not much longer than tutor2pp.

    We now need a short \c main function. We construct an
    instance of the Tutor7_Application class passing the command line
    arguments to its constructor
    \until Tutor7_Application
    We then initialize the application
    \until int err
    and return the error code if some error occured
    (\em e.g. the cross section data for the media in the geometry
    was not found in the specified PEGS data file, there was a
    mistake in the specification of the geometry, etc.)
    \until return err
    We then run the simulation
    \until err =
    and return the error code if some error occured,
    \until return err
    otherwise we finish the simulation
    \until }
    That's all.

    The final step is to provide a Makefile for our application.
    We will use the standard makefile for building C++ applications
    provided with the EGSnrc distribution. To do so, we include the
    make config files,
    \dontinclude Makefile
    \skipline EGS_CONFIG
    \until my_machine
    set the application name
    \skipline USER_CODE
    specify that no additional macros are needed to build the
    EGSnrc mortran back-end,
    \skipline EGSPP_USER_MACROS
    specify that we are deriving from EGS_AdvancedApplication,
    \skipline EGS_BASE_APPLICATION
    specify the set of mortran sources to be the predefined set of
    sources for advanced applications,
    \skipline CPP_SOURCES
    the additional header files that our application depends upon,
    \skipline other_dep_user_code
    and finally include the standard set of rules for building C++
    applications
    \skipline include

    Here is the complete source code of the tutor7pp application:
*/
