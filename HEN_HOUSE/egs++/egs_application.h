/*
###############################################################################
#
#  EGSnrc egs++ application headers
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
#  Contributors:    Frederic Tessier
#                   Ernesto Mainegra-Hing
#                   Blake Walters
#                   Reid Townson
#
###############################################################################
*/


/*! \file egs_application.h
 *  \brief EGS_Application class header file
 *  \IK
 */

#ifndef EGS_APPLICATION_
#define EGS_APPLICATION_

#include "egs_base_geometry.h"
#include "egs_base_source.h"
#include "egs_simple_container.h"

#include <string>
#include <iostream>
using namespace std;

class EGS_Input;
class EGS_BaseSource;
class EGS_RandomGenerator;
class EGS_RunControl;
class EGS_GeometryHistory;
class EGS_AusgabObject;
//template <class T> class EGS_SimpleContainer;

/*! \brief A structure holding the information of one particle
  \ingroup egspp_main
 */
struct EGS_Particle {
    int        q;      //!< particle charge
    int        latch;  //!< latch variable (useful as a flag on many occasions)
    int        ir;     //!< particle region index
    EGS_Float  E;      //!< particle energy in MeV
    EGS_Float  wt;     //!< statistical weight
    EGS_Vector x;      //!< position
    EGS_Vector u;      //!< direction
};

/*! \brief Base class for advanced EGSnrc C++ applications

  \ingroup egspp_main

  The recommended method for developing EGSnrc C++ applications is to
  derive an application class from one of the advanced application classes.
  At this point, the EGS_AdvancedApplication class,
  which is based on the mortran version of EGSnrc,
  is the only class available.
  However, when EGSnrc is reimplemented in C++ there will be a corresponding
  EGS_AdvancedCppApplication class. The EGS_Application class implements
  all functionality that is independent on the details of the physics functions
  implementation (mortran \em vs C++). The motivation behind this approach
  is that EGSnrc applications developed for the mortran back-end should
  work with as little change as possible with a future EGSnrc version
  written in C++.

  <p>EGS_Application and EGS_AdvancedApplication implement a large portion
  of the work needed to develop an EGSnrc user code. Many of the functions
  implementing particular aspects of the initialization, the simulation
  loop, the storing of intermediate results, etc., are declared virtual
  to provide a high level of flexibility, but in general only very few
  of the virtual functions must be re-implemented in derived classes.
  The following is a brief recipe for developing an EGSnrc application
  derived from EGS_AdvancedApplication. For the sake of a concrete example
  we will assume that the application calculates the dose distribution in
  all regions of the geometry specified in the input file and that the
  class derived from EGS_AdvancedApplication implementing this functionality is
  called MyDoseApplication.
  - Re-implement initScoring() to initialize the scoring of quantities of
    interest. In many cases the initialization will simply involve the
    construction of one or more EGS_ScoringArray objects, which will be
    used to accumulate the quantites of interest at run time. It is of course
    possible to have more advanced initialization controlled via a section
    in the input file that can be obtained from the #input protected data
    member (see \em e.g. \c cavity.cpp for an example). For our example case
    of a sode scoring application the implementation could look like this
    \verbatim
    int MyDoseApplication::initScoring() {
        dose = new EGS_ScoringArray(geometry->regions());
        return 0;
    }
    \endverbatim
    Here, \c dose is a class member of type \c EGS_ScoringArray *.
  - Re-implement the startNewShower() function. In most cases the
    implementation will consist of calling the EGS_ScoringArray::setHistory()
    function of all scoring array objects used for accumulating results.
    This is necessary for the history-by-history statistical analysis. In
    our example the implementation should look like this
    \verbatim
    int MyDoseApplication::startNewShower() {
        if( current_case != last_case ) {
            dose->setHistory(current_case);
            last_case = current_case;
        }
        return 0;
    }
    \endverbatim
  - Re-implement the outputData() function to output the current simulation
    results to a data file. This is necessary for the ability to restart a
    simulation or to combine the results of a parallel run. In most cases
    the implementation will involve calling the EGS_AdvancedApplication
    version first, so that data related to the state of the particle source,
    the random number generator, the EGSnrc back-end, etc., is stored, and
    than calling the storeState() function of the scoring objects. In our
    example case the implementation should look like this:
    \verbatim
    int MyDoseApplication::outputData() {
        int err = EGS_AdvancedApplication::outputData();
        if( err ) return err;
        if( !dose->storeState(*data_out) ) return 99;
        return 0;
    }
    \endverbatim
    \em i.e. if an error occurs while storing the base application data
    the function returns this error code. Otherwise it stores the dose data
    and returns a special error code if EGS_ScoringArray::storeState() fails.
  - Re-implement the readData() function to read results from a data file.
    This is necessary for restarted calculations and is basically the
    same as outputData() but now readData() and setState() are used instead
    of outputData() and storeState():
    \verbatim
    int MyDoseApplication::readData() {
        int err = EGS_AdvancedApplication::readData();
        if( err ) return err;
        if( !dose->setState(*data_in) ) return 99;
        return 0;
    }
    \endverbatim
  - Re-implement the resetCounter() function. This function is called before
    combining the results of a parallel run and should set all scoring objects
    to a 'pristine' state. In our example the implementation looks like this:
    \verbatim
    void MyDoseApplication::resetCounter() {
        EGS_AdvancedApplication::resetCounter();
        dose->reset();
    }
    \endverbatim
  - Re-implement the addState() function. This function is called from within
    the loop over parallel job results and should add the data found in the
    data stream passed as argument to the current data. In our example the
    implementation is as follows
    \verbatim
    int MyDoseApplication::addState(istream &data) {
        // **** add base data
        int err = EGS_AdvancedApplication::addState(data);
        if( err ) return err;
        // *** temporary scoring array
        EGS_ScoringArray tmp(geometry->regions());
        if( !tmp.setState(data) ) return 99; // error while reading data
        (*dose) += tmp;                      // add data
        return 0;
    }
    \endverbatim
  - Implement the ausgab() function to perform the actual scoring.
    Unfortunately, this is dependent upon the simulation back-end.
    For an application based on the mortran back-end, the relevant
    data is available from pointers to the Fortran common blocks with
    the general rule that the Fortran common block \c XXX is accessed
    via \c the_xxx, \em e.g. the particle stack is pointed to by
    \c the_stack, the \c EPCONT common that contains information about
    energy deposition, step-lengths, region indeces, etc., via \c the_epcont,
    etc. Information about the various EGSnrc common blocks is found in
    PIRS-701. The common blocks exported as C-style structures are defined
    in \c \$HEN_HOUSE/interface/egs_interface2.h. Depending on the application,
    the ausgab() function may be quite complex. In our example application
    it is very simple:
    \verbatim
    int MyDoseApplication::ausgab(int iarg) {
        if( the_epcont->edep > 0 ) { // energy is being deposited
            int np = the_stack->np - 1; // top particle on the particle stack.
            int ir = the_stack->ir[np]-2; // region index
            if( ir >= 0 ) dose->score(ir,the_epcont->edep*the_stack->wt[np]);
        }
        return 0;
    }
    \endverbatim
    In the above implementation no check is being made for the value of the
    \c iarg argument. This is because by default calls to ausgab() are
    made only for energy deposition events. However, in more advanced
    applciations, one can set up calls to ausgab for a number of other
    events (see PIRS-701, EGS_Application::AusgabCall, setAusgabCall())
    and should check the value of \c iarg to determine the type of
    event initiating the call to ausgab(). Another thing to keep in mind
    is the fact that the Fortran-style indexing is used by the mortran
    back-end so that the top stack particle is
    <code> the_stack->np - 1</code> and not \c the_stack->np. Finally,
    the convention used in the geometry package is that the outside region
    has index -1 while inside regions have indeces from 0 to the number of
    regions - 1. This convention is translated to the outside region being
    region 1 in the mortran back-end and inside regions having indeces 2 to
    number of regions + 1 and makes the subtraction of 2 from the particle
    region index necessary.
  - Re-implement the outputResults() function. The implementation should
    output the results of the simulation in a convenient form. Relatively
    short output could be put into the \c .egslog file by using the
    egsInformation() function for writing the data. Large amounts of data
    are better output into separate output files. To construct an output file
    name the application has access to the base output file name (without
    extension) via getOutputFile(), to the application directory via
    getAppDir(), to the working directory via getWorkDir(), etc.
  - Possibly re-implement the getCurrentResult() function to provide a single
    result and its uncertainty. This is not necessary
    for the proper operation of the application but is a nice feature.
    It is used to obtain a combined result for all parallel jobs during
    parallel processing execution, which is written to the job control
    file. In our example dose application one could for instance provide
    as the single result the dose in a single 'watch region':
    \verbatim
    void MyDoseApplication::getCurrentResult(double &sum, double &sum2,
                                double &norm, double &count) {
        // set number of statistically independent events
        count = current_case;
        // obtain the score in the watch region
        dose->currentScore(watch_region,sum,sum2);
        // set the normalization
        norm = 1.602e-10*count/source->getFluence()/watch_region_mass;
    }
    \endverbatim
    It is worth noting that in the process of combining the results of all
    parallel runs the run control object divides the result by the total
    number of statistically independent events. If we want the normalization
    to be in Gy/incident fluence we have to multiply back by count and
    then divide by the watch region mass, the particle fluence and multiply
    with 1.602e-10 to convert the energy deposited (counted in MeV) to J.
  - Possibly re-implement the describeUserCode() and describeSimulation()
    functions to provide a description of your user code and a description
    of the simulation being performed with both outputs going to the
    log file.

    \todo Add time dependence

 */
class EGS_EXPORT EGS_Application {

public:

    /*! \brief Construct an EGSnrc application.

      The constructor checks the arguments passed on the command line
      and sets up
      the name of the application, the input file name, the pegs file name,
      the output file name, the working directory, batch vs interactive
      run, parallel job execution (i_parallel and n_parallel) and reads in
      the input file into an EGS_Input object (a pointer to this object
      is available with the protected data member #input)
    */
    EGS_Application(int argc, char **argv);

    /*! \brief Destruct the EGSnrc application.

      The destructor deletes the random number generator, the run control
      object, the input object, the source and the geometry.
     */
    virtual ~EGS_Application();

    /*! \brief Initializes the EGSnrc application.

      The default implementation of this function performs
      initializations related to the geometry,
      the source, the random number generator, the run control object,
      the EGSnrc mortran back end, the cross sections and the scoring
      of quantities of interest by calling in succession the
      protected virtual functions initGeometry(),
      initSource(), initRNG(), initRunControl(), initEGSnrcBackEnd(),
      initCrossSections() and initScoring().
      - If all calls succeed (i.e. the above functions return zero)
        the return value is zero.
      - The return value is 1 if any of initGeometry(), initSource(),
        initRNG() or initRunControl() returns a non-zero status,
      - 2 if initEGSnrcBackEnd() returns a non-zero status,
      - 3 if initCrossSections() returns a non-zero status  and
      - 4 if initScoring() returns a non-zero status.

      The EGSnrc C++ application developer can change the way an EGSnrc
      application is initialized
      by re-implementing this function or one of the virtual functions
      called from it.
    */
    virtual int initSimulation();

    /*! \brief Set the simulation chunk.

      Tells the application that the next chunk of particles to be
      simulated starts at \a nstart and will consist of \a nrun particles.
      This is necessary for parallel runs using phase space files. The default
      implementation simply calls the EGS_BaseSource::setSimulationChunk()
      method.
    */
    virtual void setSimulationChunk(EGS_I64 nstart, EGS_I64 nrun);

    /*! \brief Runs an EGSnrc simulation.

     This function performs the actual simulation. In its default
     implementation, the shower loop is a loop that
     is repeated while the getNextChunk() function of the run control
     object returns a positive number of histories to run.
     Each time getNextChunk() returns a positive number of histories
     to simulate, this number is split into "batches" (the number of
     batches is obtained from the run control object using
     getNbatch()) and the startBatch() and finishBatch() functions of
     the run control object are called at the beginning and end of a
     batch. The startBatch() and finishBatch() functions are virtual
     and can be re-implemented in derived classes to do things such as
     reporting the progress of the simulation, storing intermediate
     results into files, etc. In each batch the simulateSingleShower()
     function is called the appropriate number of times. The loop over
     single showers is terminated if simulateSingleShower() returns a
     non-zero status.
     The loop over batches is terminated if either
     startBatch() or finishBatch() returns a non-zero status.
    */
    virtual int runSimulation();

    /*! \brief Analyze and output the results.

     The default implementation of this function calls
     the \c finishSimulation method of the run control object
     and simply returns its exit code if this is negative
     (indicating some sort of an error condition).
     Otherwise outputResults() and finishRun() are called in succession.
    */
    virtual int finishSimulation();

    /*! \brief Describe the simulation

     This function should produce information about the geometry,
     source, transport parameter, scoring options, etc.
     The default implementation described the geometry and the source.
     */
    virtual void describeSimulation();

    /*! \brief Simulates a single particle shower.

     The default implementation of this function obtaines particles
     from the source until a particle is inside the geometry or
     the particle trajectory enters the geometry (if no particle
     out of 100000 particles enters the geometry there is a warning issued
     and 1 is returned to the calling function). When a particle enters
     the geometry, the startNewShower(), shower() and finishShower()
     functions are called in succession. If any of these functions
     returns a non-zero status, simulateSingleShower() returns immediately
     with this status (so that, if all other calls are successful,
     the return value of finishShower() becomes the return value of
     simulateSingleShower() )
    */
    virtual int simulateSingleShower();

    /*! \brief Report the current result.

     This virtual function should be re-implemented in derived classes
     to report intermediate results during a simulation.
    */
    virtual void getCurrentResult(double &sum, double &sum2, double &norm,
                                  double &count) {
        sum = 0;
        sum2 = 0;
        norm = 1;
        count = 0;
    };

    /*! \brief Analyze the simulation results.

     This virtual function should be re-implemented in derived classes
     to perform the statistical analysis of the simulation results.
    */
    virtual void analyzeResults() {};

    /*! \brief Output the simulation results.

     This virtual function should be re-implemented in derived classes
     to output the simulation results.
     */
    virtual void outputResults() {};

    /*! \brief Combine results from parallel runs.

     The default implementation of this function first calls
     resetCounter() to set the application to a 'pristine' state and
     then loops over all files with file names of the form
     \c ofile_wX.egsdat, where \c ofile is the output file name
     for the parallel run and \a X is an integer between 1 and 100,
     adding the data from these files using addState().
     This implementation should work for almost any situation.
     Nevertheless, combineResults() is declared as virtual to give the
     possibility for re-implementation, just in case something unusual
     needs to be done to sum the results of parallel runs.
    */
    virtual int combineResults();

    /*! \brief Output intermediate results.

     This function stores the state of the application to a data
     file. Stored quantities are #last_case, the state of the
     run control object, the state of the source object and
     the state of the random number generator.
     The data is stored in <code>%coutput_file.egsdat</code>
     and #data_out points to the output data stream created by
     opening the file.
     Derived classes should re-implement to
     add their own data to the above after invoking the base class
     outputData() function.
     The data stored should be enough to be able to restart a previous
     calculation and/or to combine the results of parallel runs.
    */
    virtual int  outputData();

    /*! \brief Read intermediate results.

     Opens the data file <code>%input_file.egsdat</code> for reading,
     which must contain results of a previous simulation
     stored with outputData(), and sets #data_in to point to
     the input stream.
     It then reads and sets the state of the application from a data file.
     Read quantities are #last_case, the state of the
     run control object, the state of the source object and
     the state of the random number generator.
     Derived classes should re-implement to read their additional data
     after invoking the base class readData() function.
     This function is intended to be used for restarted calculations.
    */
    virtual int  readData();

    /*! \brief Returns a pointer to the EGS_Input object containing the
      user input to the application found in the input file.
    */
    EGS_Input *getInput() {
        return input;
    };

    /*! \brief Returns the application name */
    const string &getAppName() const {
        return app_name;
    };

    /*! \brief Returns the \c EGS_HOME directory */
    const string &getEgsHome() const {
        return egs_home;
    };

    /*! \brief Returns the \c HEN_HOUSE directory */
    const string &getHenHouse() const {
        return hen_house;
    };

    /*! \brief Returns the base name of the output file(s) */
    const string &getOutputFile() const {
        return output_file;
    };

    /*! \brief Returns the base name of the final output file(s)

    For single job runs the final output file name is the same as
    the output file name obtained via getOutputFile(). For parallel runs
    they are different with this function returning xxx wheras
    getOutputFile() returns xxx_wX with xxx indicating the output file name
    and X the job number.
    */
    const string &getFinalOutputFile() const {
        return final_output_file;
    };

    /*! \brief Constructs and returns the name of an input/output file

    Constructs the name of the file by joining the application directory,
    run directory (which may be empty, if this is the job combining a
    parallel run), if \a with_run_dir is true, the output file name, and
    the extension given as argument.
    */
    string constructIOFileName(const char *extension, bool with_run_dir) const;

    /*! \brief Returns the absolute path to the user code directory */
    const string &getAppDir() const {
        return app_dir;
    };

    /*! \brief Returns the name of the working directory */
    const string &getRunDir() const {
        return run_dir;
    };

    /*! \brief Returns the name of the working directory */
    const string &getWorkDir() const {
        return run_dir;
    };

    /*! \brief Possible calls to the user scoring function ausgab(). */
    enum AusgabCall {
        BeforeTransport = 0,     //!< before the step
        EgsCut = 1,              //!< energy below Ecut or Pcut
        PegsCut = 2,             //!< energy below AE or AP
        UserDiscard = 3,         //!< user requested discard
        ExtraEnergy = 4,         /*!< initiated when part of the energy is not
                               transfered to particles (e.g. binding energy)*/
        AfterTransport = 5,      //!< after the step
        BeforeBrems = 6,         //!< before a bremsstrahlung interaction
        AfterBrems = 7,          //!< after a bremsstrahlung interaction
        BeforeMoller = 8,        //!< before an inelastic collision (e-)
        AfterMoller = 9,         //!< after an inelastic collision (e-)
        BeforeBhabha = 10,       //!< before an inelastic collision (e+)
        AfterBhabha = 11,        //!< after an inelastic collision (e+)
        BeforeAnnihFlight = 12,  //!< before annihilation in flight
        AfterAnnihFlight = 13,   //!< after annihilation in flight
        BeforeAnnihRest = 28,    //!< before annihilation at rest
        AfterAnnihRest = 14,     //!< after annihilation at rest
        BeforePair = 15,         //!< before pair production
        AfterPair = 16,          //!< after pair production
        BeforeCompton = 17,      //!< before a Compton scattering event
        AfterCompton = 18,       //!< after a Compton scattering event
        BeforePhoto = 19,        //!< before a photo-absorption event
        AfterPhoto = 20,         //!< after a photo-absorption event
        EnteringUphi = 21,       //!< the rotation routine was just entered
        LeavingUphi = 22,        //!< about to leave the rotation routine
        //!< I consider the above 2 to be obsolete
        BeforeRayleigh = 23,     //!< before coherent scattering
        AfterRayleigh = 24,      //!< after coherent scattering
        FluorescentEvent = 25,   //!< a fluorescent transition just occured
        CosterKronigEvent = 26,  //!< a Coster-Kronig transition just occured
        AugerEvent = 27,         //!< an Auger transition just occured
        BeforePhotoNuc = 29,      //!< before a photonuclear event
        AfterPhotoNuc = 30,       //!< after a photonuclear event
        UnknownCall = 31         //!< last element in the enumeration
    };

    /*! \brief Turns on or off a call to the user scoring function ausgab.

     This function is re-implemented in EGS_AdvancedApplication to also
     modify the mortran array iausfl
    */
    virtual void setAusgabCall(AusgabCall call, bool on_or_off) {
        ausgab_flag[call] = on_or_off;
    };

    /*! \brief Returns the number of parallel jobs executing

     The number of parallel jobs is taken from the command line
     argument <code>-P n</code> (or <code>--parallel n</code>) but is reset to 1 if
     there was no job number specified (see getIparallel()) or
     the job number was greater than the number of parallel jobs.
    */
    int getNparallel() const {
        return n_parallel;
    };

    /*! \brief Returns the job number in a parallel run.

     The job number is specified on the command line using
     <code>-j n</code> or <code>--job n</code> and requires that the number of parallel
     jobs was also specified using <code>-P n</code>.
    */
    int getIparallel() const {
        return i_parallel;
    };

    /*! \brief Returns the first job number in a parallel run.

     The first job number is specified on the command line using
     <code>-f n</code> or <code>--first-job n</code> and requires that the number of parallel
     jobs was also specified using <code>-P n</code>.
    */
    int getFirstParallel() const {
        return first_parallel;
    };

    /*! \brief Calculates distance to a boundary along the current direction.

     This function implements the EGSnrc howfar geometry specification
    */
    inline int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
                      EGS_Float &t, int *newmed) {

        geometry->resetErrorFlag();
        EGS_Float twant = t;
        int inew = geometry->howfar(ireg,x,u,t,newmed);
        storeGeometryStep(ireg,inew,x,u,twant,t);
        if (geometry->getLastError()) {
            reportGeometryError();
        }
        return inew;

        //return geometry->howfar(ireg,x,u,t,newmed);
    };

    /*! \brief Calculates nearest distance to a boundary in any direction.

     This function implements the EGSnrc hownear geometry specification
    */
    inline EGS_Float hownear(int ireg,const EGS_Vector &x) {
        return geometry->hownear(ireg,x);
    };

    /*! \brief Returns the medium index in region ireg using C-style indexing.*/
    inline int getMedium(int ireg) {
        return geometry->medium(ireg);
    };

    /*! \brief Returns true if \a ireg is a real region, false otherwise

      This method is needed because of the region indexing style used by
      some composite geometries which results in a larger number of
      regions than actual regions. This method can be used in such cases
      to check if a region exists.
     */
    bool isRealRegion(int ireg) {
        return geometry->isRealRegion(ireg);
    }
    int  isWhere(EGS_Vector &r) {
        return geometry->isWhere(r);
    }

    /*! \brief Gets numbers out of \a str and pushes them onto \a regs

      Finds integer numbers in \a str and pushes them onto the vector \a regs.
      For an input string containing a mixture of labels and region numbers,
      this extracts the region numbers.

      Usually you will do something like:
      \verbatim
      string regionString;
      vector<int> regionVector;
      int err1 = input->getInput("cavity regions",regionString);
      geom->getNumberRegions(regionString, regionVector);
      geom->getLabelRegions(regionString, regionVector);
      \endverbatim
     */
    void getNumberRegions(const string &str, vector<int> &regs) {
        geometry->getNumberRegions(str, regs);
    }

    /*! \brief Gets the regions for the labels in \a str and pushes onto \a regs

      This function is used after \a getNumberRegions. It looks for labels
      in \a str, finds the corresponding local region numbers, and pushes those
      region numbers onto the region number vector \a regs.

      The \a regs vector is sorted by this function, and duplicates are removed!
     */
    void getLabelRegions(const string &str, vector<int> &regs) {
        geometry->getLabelRegions(str, regs);
    }

    /*! \brief User scoring function for accumulation of results and VRT implementation

      This function first calls the processEvent() method of the ausgab objects
      registered with the application and then proceeds to call ausgab().

    */
    int userScoring(int iarg, int ir=-1);

    /*! \brief User scoring function.

     This function should be re-implemented in derived classes
     to perform the actual scoring of the quantities of interest.
    */
    virtual int ausgab(int) {
        return 0;
    };

    /*! \brief Start the transport of a new particle.

     This function is called just before the transport of a new particle
     begins and should be re-implemented in derived classes.
     Its default re-implementation in EGS_AdvancedApplication is
     to set the medium index of the current particle region.
     It can be re-implemented in derived classes to set other
     quantities such as ecut, pcut, etc. in order to have such
     quantities varying on a region-by-region bases.
    */
    virtual void startNewParticle() { };

    /*! \brief Particle enters new region.

     This function is intended for setting up more sophisticated
     transport schemes that vary on a region-by-region basis.
     In the mortran sources when there is a region change the
     macros $electron_region_change or $photon_region_change are
     executed. In their default implementation these macros
     set ecut or pcut to ecut_new or pcut_new, rhor to rhor_new,
     medium to medium_new, the region index to irnew and
     smaxir to smax_new. In this way, if the user wants to implement
     a region-by-region variation of the above quantites, the xxx_new
     variables must be set to their new values in howfar, if a region
     change will take place at the end of the step. For varying
     additional quantities, the user can replace the
     $electron_region_change and/or $photon_region_change macros
     with calls to a function, which calls enterNewRegion() and
     provide an implementation of enterNewRegion().
    */
    virtual void enterNewRegion() { };

    /*! \brief Fill an array with random numbers using the application's RNG

     This function is called from within the mortran sources to
     get a new set of n random numbers.
     Its default implementation uses the application
     random number generator to fill the array pointed to by rns
     with n random numbers.
    */
    virtual void fillRandomArray(int n, EGS_Float *rns);

    /*! \brief Get the active application.

     This static function returns a pointer to the currently active
     application, which can be changed via a call to setActiveApplication().
     Its primary use is to obtain the pointer to the active EGS_Application
     instance in the C-style functions (howfar, hownear, ausgab, etc)
     needed for the mortran subroutines in order to call the corresponding
     EGS_Application method.
     In most cases there will be a single EGSnrc application and the pointer
     to it will be set-up automatically when constructing the only
     EGS_Application class. However, in more advanced application the user
     may want to have several EGS_Application classes (e.g. the particle
     source itself may be derived from EGS_Application). In such cases,
     the setActiveApplication() function must be called by each
     EGS_Application instance before transporting particles.
    */
    static EGS_Application *activeApplication();

    /*! \brief Set the active EGS_Application class.

     This function can be called from derived EGS_Application classes
     before transporting particles with a pointer to itself in
     situations where there is more than one instance of
     EGS_Application-derived classes

     \sa activeApplication().
    */
    static void setActiveApplication(EGS_Application *);

    /*! \brief Returns the number of random numbers used.

     The default implementation is to simply return the numbers
     used reported by the random number generator.
     Can be re-implemented in derived classes for more complex
     situations with correlated runs, etc.
     */
    virtual EGS_I64 randomNumbersUsed() const;

    /*! \brief Get the number of electron steps taken.

     The number of all steps taken (single scattering or condensed history) is
     assigned to \a all_steps, the number of condensed history steps to
     \a ch_steps.
     Double precision numbers are used here as this is how
     steps are counted in the mortran back-end.
     */
    virtual void getElectronSteps(double &ch_steps, double &all_steps) const {
        ch_steps = 0;
        all_steps = 0;
    };

    /*! \brief Add data from a parallel job.

      This function is called from within the loop over parallel jobs
      in the combineResults() function and must add the data found in the
      stream \a data to the results accumulated so far by the application.
      The default implementation reads and adds data related to the
      particle source, the random number generator, the run control
      object and the number of statistically independent events
      simulated (\em i.e. the data saved in outputData()).

      \sa outputData(), readData(), resetCounter().
    */
    virtual int addState(istream &data);

    /*! \brief Reset the application to a 'pristine' state.

      This function is called from within combineResults() and
      must reset all variables and objects collecting information about the
      simulation. The default implementation sets #current_case and
      #last_case to zero and calls the \c resetCounter() functions of
      the run control object, the random number generator and
      the source.
     */
    virtual void resetCounter();

    /*! \brief Describe the user code

      This virtual function should be re-implemented in derived classes
      to output some useful and descriptive information about the user
      code to go into the log file. The default implementation does
      nothing.
     */
    virtual void describeUserCode() const {};

    /*! \brief Write an information message
     */
    virtual void appInformation(const char *);

    /*! \brief Write a warning message
     */
    virtual void appWarning(const char *);

    /*! \brief Write a warning message and exit.
     */
    virtual void appFatal(const char *);

    /*! \brief Check if a device holding a given stream is full
     */
    void checkDeviceFull(FILE *);

    /*! \brief Finds a command line argument.

      This function checks the \a argc command line arguments pointed to by
      \a argv for existence of an argument \a name1 or \a name2. If the
      argument exists, it sets \a arg to the next command line argument and
      returns \c true. Otherwise it returns \c false.

      \sa checkEnvironmentVar()
    */
    static bool getArgument(int &argc, char **argv,
                            const char *name1, const char *name2, string &arg);

    /*! \brief Finds a command line argument.

      This function is similar to getArgument(), but if the arguments
      \a n1 and \a n2 are not given, it sets \a var from the environment
      variable \a env (if it is defined).
    */
    static void checkEnvironmentVar(int &argc, char **argv, const char *env,
                                    const char *n1, const char *n2, string &var);

protected:

    /*! \brief Initialize the simulation geometry

      The default implementation of this function initializes the
      simulation geometry from the input provided in an input file
      and available in the EGS_Application input object data member.
      Returns zero if the geometry initialization is successful,
      -1 if there is no input or 1 if the geometry construction failed.
      The user may re-implement this function to provide their
      own geometry initialization (e.g. a fixed geometry constructed
      directly instead of using an input file).
      This function is called from within the default implementation of
      the initSimulation() function.
    */
    virtual int initGeometry();

    /*! \brief Initialize the particle source

      The default implementation of this function initializes the
      particle source from the input provided in an input file
      and available in the EGS_Application input object data member.
      Returns zero if the source initialization is successful,
      -1 if there is no input or 1 if the source construction failed.
      The user may re-implement this function to provide their
      own particle source initialization (e.g. a fixed source
      constructed directly instead of using an input file).
      This function is called from within the default implementation of
      the initSimulation() function.
    */
    virtual int initSource();

    /*! \brief Initialize the EGSnrc cross sections and
      cross section/transport options

     This function is re-implemented in the EGS_AdvancedApplication
     class, from which EGSnrc applications using the mortran EGSnrc
     physics subroutines should be derived. The defualt implementation
     is to set transport parameter and cross section options
     from input between :start MC Transport parameter: and
     :stop MC Transport parameter: in the input file,
     to add the media names found in the geometry to the EGSnrc back-end
     and to call the egsHatch() function to get the cross section data.
     This function is called from within the default implementation of
     the initSimulation() function.
    */
    virtual int initCrossSections() {
        return 0;
    };

    /*! \brief Initialize the scoring of quantities of interest.

     This function is called from within the default implementation of
     the initSimulation() function and must be re-implemented by
     the user to do all initializations related to the scoring of
     quantities of interest.
    */
    virtual int initScoring() {
        return 0;
    };

    /*! \brief Construct the run control object.

     This function is called from within the default implementation of
     the initSimulation() function and is expected to construct and
     initialize a run control object.
     A run control object provides quantities such as the number of
     particles to run, how many batches, how many histories in
     the next chunk for parallel runs, etc., that are needed in
     the runSimulation() method. The default implementation looks
     for input between :start run control: and :stop run control:
     in the input file and contructs the run control object from it.
     Two default run control objects can be used:
       - simple: no parallel execution, etc.
       - parallel execution control using a job control file in the
         user code directory (which must therefore be on a NFS).
    */
    virtual int initRunControl();

    /*! \brief Initialize the random number generator.

      The default implementation of this function looks for the input
      section defined by :start rng definition: and :stop rng definition:
      in the input file and constucts a random generator of the given
      type using the initial seeds specified.
      If no such input is found, the default EGSnrc random number generator
      is used (currently ranmar).
      This function is called from within the default implementation of
      the initSimulation() function.
    */
    virtual int initRNG();

    /*! \brief Initialize the EGSnrc backend.

      This function is re-implemented in the EGS_AdvancedApplication
      class, from which EGSnrc applications using the mortran EGSnrc
      physics subroutines should be derived, to call
      the egs_set_defaults and egs_init1 mortran subroutines.
      egs_set_defaults sets default values for all variables
      needed in the simulation. egs_init1 opens the pegs file and
      various other data files, opens the input file and creates the
      working directory.
      This function is called from within the default implementation of
      the initSimulation() function.
    */
    virtual int initEGSnrcBackEnd() {
        return 0;
    };

    /*! \brief Initialize ausgab objects.

      This function scans the input file for user input delimeted
      by <code>:start ausgab object definition:</code> and
      <code>:stop ausgab object definition:</code> and creates
      ausgab objects as requested by the user.
    */
    void initAusgabObjects();

    /*! \brief Adds an ausgab object to the list of ausgab objects */
    void addAusgabObject(EGS_AusgabObject *o);

    /*! \brief Called just before the shower() function.

     This function is called just before the shower()
     function from within the default implementation of
     the simulateSingleShower() function. If the return
     value is not zero, shower() is not called and this value
     is used as a return value of simulateSingleShower()
    */
    virtual int startNewShower();

    /*! \brief Called just after the shower() function.

     This function is called just after the shower()
     function from within the default implementation of
     the simulateSingleShower() function and its return value is returned
     as the return value of simulateSingleShower()
    */
    virtual int finishShower() {
        return 0;
    };

    /*! \brief Simulate a single shower.

     This function is called from within the simulateSingleShower() function
     and should transport one particle with parameters stored in the
     p protected data member of EGS_Application.
     The default implementation does nothing.
     This function is reimplemented in EGS_AdvancedApplication
     to call the mortran EGSnrc shower subroutine.
    */
    virtual int shower() {
        return 0;
    };

    virtual void finishRun() { };

    void storeGeometryStep(int ireg, int inew, const EGS_Vector &x,
                           const EGS_Vector &u, EGS_Float twant, EGS_Float t);

    void reportGeometryError();

    EGS_Input           *input;         //!< the input to this simulation.
    EGS_BaseGeometry    *geometry;      //!< the geometry of this simulation
    EGS_BaseSource      *source;        //!< the particle source
    EGS_RandomGenerator *rndm;          //!< the random number generator
    EGS_RunControl      *run;           //!< the run control object.

    bool    ausgab_flag[UnknownCall]; //!< on/off flags for ausgab calls

    string  app_name;           //!< The application name
    string  egs_home;           //!< The EGS_HOME directory
    string  hen_house;          //!< The HEN_HOUSE directory
    string  app_dir;            //!< The user code directory
    string  run_dir;            //!< The working directory during the run
    string  egs_config;         //!< The EGSnrc config
    string  input_file;         //!< The input file name
    string  output_file;        //!< The output file name (no extension)
    string  final_output_file;  //!< The final output file name
    string  pegs_file;          //!< The pegs file name
    string  abs_pegs_file;      //!< The pegs file name including absolute path

    int     n_parallel,  //!< Number of parallel jobs
            i_parallel,  //!< Job index in parallel runs
            first_parallel; //!< first parallel job number
    bool    batch_run;   //!< Interactive or batch run.
    bool    simple_run;  //!< Use a simple run control even for parallel runs
    bool    is_pegsless; //!< set to true if a pegsless run

    EGS_Particle p;      /*!< Parameters of the particle that just
                             entered the geometry */
    EGS_I64      current_case; //!< The current case as returned from the source
    EGS_I64      last_case;    //!< The last case simulated.

    /*! \brief data output stream

     Points to the data stream opened for output in
     outputData()
    */
    ostream *data_out;

    /*! \brief data input stream

     Points to the data stream opened for input in
     readData()
    */
    istream *data_in;

    /*! \brief the index of this application.

     */
    int app_index;

    /*! \brief The ausgab objects */
    EGS_SimpleContainer<EGS_AusgabObject *> a_objects_list;

    /*! \brief The ausgab objects for the various ausgab calls */
    EGS_SimpleContainer<EGS_AusgabObject *> *a_objects;

    EGS_GeometryHistory *ghistory;

private:

    static int n_apps; //!< Number of applications constructed so far.

public:

    EGS_Particle top_p;  //!< The top particle on the stack (i.e., the particle being transported)
    int          Np;     //!< The index of the top particle on the stack
    //************************************************************
    // Utility functions for use with ausgab dose scoring objects
    //************************************************************
    EGS_Float getFluence() {
        return source->getFluence();
    };
    int       getnRegions() {
        return geometry->regions();
    };
    int       getnMedia() {
        return geometry->nMedia();
    };
    const char *getMediumName(int ind) {
        return geometry->getMediumName(ind);
    };
    virtual EGS_Float getMediumRho(int ind) {
        return -1.0;
    };
    virtual EGS_Float getEdep() {
        return 0.0;
    };
    virtual void setEdep(EGS_Float edep) {};
    virtual EGS_Float getEcut() {
        return 0.0;
    };
    virtual EGS_Float getPcut() {
        return 0.0;
    };
    virtual EGS_Float getRM() {
        return -1.0;
    };
};

#define APP_MAIN(app_name) \
    int main(int argc, char **argv) { \
        app_name app(argc,argv); \
        int err = app.initSimulation(); \
        if( err ) return err; \
        err = app.runSimulation(); \
        if( err < 0 ) return err; \
        return app.finishSimulation(); \
    }

#define APP_SIMPLE_MAIN(app_name) \
    int main(int argc, char **argv) { \
        app_name app(argc,argv); \
        app.run(); \
        app.reportResults(); \
        app.finish(); \
        return 0; \
    }

#define APP_LIB(app_name) \
    extern "C" {\
        APP_EXPORT EGS_Application* createApplication(int argc, char **argv) {\
            return new app_name(argc,argv);\
        }\
    }


#endif
