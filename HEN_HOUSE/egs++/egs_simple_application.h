/*
###############################################################################
#
#  EGSnrc egs++ simple application headers
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


/*! \file egs_simple_application.h
 *  \brief Header file for developing simple applications
 *  \IK
 */

#ifndef EGS_SIMPLE_APPLICATION_
#define EGS_SIMPLE_APPLICATION_

#include "egs_libconfig.h"
#include "egs_base_geometry.h"

class EGS_BaseSource;
class EGS_RandomGenerator;
class EGS_Input;

/*! \brief A base class for developing simple EGSnrc applications

  The easiest way to develop a C++ user code for EGSnrc is to derive
  from the EGS_SimpleApplication class and implement the ausgab() function
  to perform the scoring of the quantites of interest, possibly
  reimplement the startHistory() and endHistory() functions (called
  before/after the simulation of a single shower within the shower loop
  performed in run()), and implement initializitions and output of the
  results. An example of a simple C++ application for EGSnrc that uses
  a class derived from EGS_SimpleApplication is tutor2pp.

  A better way of developing C++ user codes for EGSnrc is to derive
  from the EGS_AdvancedApplication class.
*/
class APP_EXPORT EGS_SimpleApplication {

public:

    //
    // ******************* constructor and destructor *************************
    //
    /*! \brief Construct a simple application object.

    The command line arguments must be passed to the EGS_SimpleApplication
    constructor so that the name of the application, the input file, the output
    file and the pegs file can be determined. The input file is then read in,
    geometry, source and random number generator are initialized and the
    egs_init function is called. The egs_init function, which is part of the
    mortran back-end, sets default values for all variables needed in the
    simulation, creates a temporary working directory for the run and
    opens various Fortran I/O units needed by the mortran back-end.
    The media present in the geometry are then added to the mortran back-end
    and the HATCH subroutine is called to initialize the cross section data.
    Finally, a check is made that the cross section data files cover
    the energy range needed based on the maximum energy of the source.
    */
    EGS_SimpleApplication(int argc, char **argv);

    /*! Destructor.

    Deletes the geometry, the source, the random number generator and
    the input object.
    */
    virtual ~EGS_SimpleApplication();

    //
    // ************************ geometry functions **********************
    //
    /*! \brief See the EGSnrc \link EGS_BaseGeometry::howfar() howfar
      geometry specification \endlink */
    inline int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
                      EGS_Float &t, int *newmed) {
        return g->howfar(ireg,x,u,t,newmed);
    };
    /*! \brief See the EGSnrc \link EGS_BaseGeometry::hownear() hownear
      geometry specification \endlink */
    inline EGS_Float hownear(int ireg,const EGS_Vector &x) {
        return g->hownear(ireg,x);
    };
    /*! \brief Get the medium index in region \a ireg */
    inline int getMedium(int ireg) {
        return g->medium(ireg);
    };

    //
    // ******************** ausgab ***************************************
    //
    /*! Scoring of results.

    This function must be reimplemented by the derived class to perform the
    scoring of the quantities of interest during the simulation.
    ausgab() is called with a corresponding integer argument on various
    events during the simulation (see PIRS-701).
    */
    virtual int ausgab(int) {
        return 0;
    };

    //
    // ********* finish the simulation
    //
    /*! \brief Finish the simulation.

    The default implemenmtation simply calls the egsFinish() function,
    which moves all output from the temporary working directory to the
    user code directory and deletes the temporary working directory.
    */
    virtual void finish();

    //
    // ********* run a simulation
    //
    /*! \brief Run the simulation.

    The default implementation of this function simulates #ncase showers,
    calling startHistory() and endHistory() before/after the shower is
    simulated. The progress of the simulation is reported #nreport
    times during the simulation. This number can bechanged using
    setNProgress().
    */
    virtual int run();

    //
    // ********* how often to report the progress of the simulation
    //
    /*! \brief Set the number of times the shower loop in run() reports
      the progress of the simulation to \a nprog.
     */
    void setNProgress(int nprog) {
        nreport = nprog;
    };

    //
    // ********* functions to be executed before and after a shower.
    //
    /*! \brief Start a new shower.

    This function is called from within the shower loop in run() before
    the simulation of the next particle starts. Could be reimplemented
    in derived classes to \em e.g. inform scoring objects that a new
    statistically independent event begins.
    */
    virtual void startHistory(EGS_I64) {};

    /*! Finish a shower.

    This function is called from within the shower loop in run() after
    the simulation of a particle has finished. Could be reimplemented in
    derived classes to \em e.g. score a pulse-height distribution.
    */
    virtual void endHistory() {};

    //
    // ******** fill an array with random numbers using the random
    //          number generator of this application.
    //
    /*! \brief Fill the array pointed to by \a r with \a n random numbers.
     */
    void fillRandomArray(int n, EGS_Float *r);

    //
    // ******** get various directories and file names
    //
    /*! Get the \c EGS_HOME directory */
    const char *egsHome() const;
    /*! Get the \c HEN_HOUSE directory */
    const char *henHouse() const;
    /*! Get the name of the PEGS file */
    const char *pegsFile() const;
    /*! Get the name of the input file */
    const char *inputFile() const;
    /*! Get the name of the output file */
    const char *outputFile() const;
    /*! Get the name of the user code */
    const char *userCode() const;
    /*! Get the working directory */
    const char *workDir() const;

    //
    // ******** the parallel run index and number of parallel jobs
    //
    /*! The index of this job in a parallel run */
    int iParallel() const;
    /*! Number of parallel jobs */
    int nParallel() const;

protected:

    EGS_I64             ncase;    //!< Number of showers to simulate
    int                 nreport;  //!< How often to report the progress
    EGS_BaseGeometry    *g;       //!< The simulation geometry
    EGS_BaseSource      *source;  //!< The particle source
    EGS_RandomGenerator *rndm;    //!< The random number generator.
    EGS_Input           *input;   //!< The input found in the input file.
    double              sum_E,  //!< sum of E*wt of particles from the source
                        sum_E2, //!< sum of E*E*wt of particles from the source
                        sum_w,  //!< sum of weights
                        sum_w2, //!< sum of weights squared
                        Etot;   /*!< same as sum_E but only for particles
                                     entering the geometry */
    EGS_I64             last_case; //!< last statistically independent event

};

#endif

#define APP_MAIN(app_name) \
    int main(int argc, char **argv) { \
        app_name app(argc,argv); \
        app.run(); \
        app.reportResults(); \
        app.finish(); \
        return 0; \
    }

#define APP_LIB(app_name) \
    extern "C" {\
        APP_EXPORT EGS_SimpleApplication* createApplication(int argc, char **argv) {\
            return new app_name(argc,argv);\
        }\
    }


/*!
    \example tutor2pp.cpp
    \dontinclude tutor2pp.cpp

    The tutor2pp application calculates the deposited, transmitted and
    reflected energy fractions for any \link Sources source \endlink
    of electrons or photons that
    can be defined with the egspp source package, incident on
    any \link Geometry geometry \endlink
    that can be constructed  using the egspp
    geometry package. The corresponding original tutorial
    mortran user code \c tutor2, which calculates the deposited,
    transmitted and reflected energy loss fractions for 20 MeV
    electrons perpendicularly incident on a semi-infinite 1 mm plate
    of Tantalum, only provides a very small fraction of the functionality
    offered by tutor2pp.

    The complete source code of the application is found
    at the end of this documentation page.

    To obtain the required functionality we implement a
    Tutor2_Application class derived from EGS_SimpleApplication
    \skipline Tutor2_Application
    \until public:
    The meaning of the private data members \c edep and \c nreg will
    become clear in the following.
    The constructor of the Tutor2_Application class takes the command line
    arguments given to the application as arguments and passes them
    to the EGS_SimpleApplication constructor,
    \skip Tutor2_Application(
    \until EGS_SimpleApplication
    which reads the input file and initializes the source, the geometry, the
    cross section data and the random number generator, opens I/O
    units  necessary for the mortran back-end, creates a temporary working
    directory for the run  and checks that the cross section data
    covers the energy range defined by the source. This corresponds to
    steps 0, 1, 2, 3, 4 and 6 from the recipe
    for writing EGSnrc mortran user codes (see PIRS--701).
    We then obtain the number of regions from the geometry
    \line nreg
    and initialize a EGS_ScoringArray object that will be used to
    collect the energy deposited in the various regions
    \skip edep
    \until }
    This corresponds to step 5 from the recipe
    for writing EGSnrc mortran user codes.
    We initialize the scoring array to have 2 more regions than the
    geometry so that we can collect the energy reflected (defined as
    the energy of particles exiting the geometry and moving backwards
    and scored in region 0 of \c edep)
    and the energy transmitted (defined as
    the energy of particles exiting the geometry and moving forward
    and scored in region <code>nreg+1</code> of \c edep)
    in addition to the energy deposited in the \c nreg regions of
    the geometry (scored in regions 1 ... \c nreg of \c edep).
    This is all that is needed in the Tutor2_Application constructor.

    To demonstrate good coding habits we provide a destructor for the
    Tutor2_Application that deallocates the memory needed by the
    scoring array
    \skipline ~Tutor2_Application

    The next step is the implementation of the actual scoring which
    is accomplished by re-implementing the \c ausgab function to
    collect energy deposition. Energy deposition is indicated by
    values of its argument less or equal to 4
    \skip ausgab(int iarg)
    \until if
    We obtain the index of the top particle on the stack and its
    region index from the \c the_stack, which is a pointer to the
    Fortran common block \c STACK
    \skipline np =
    Note that the mortran back-end uses Fortran style indexing and
    therefore we have to use \c the_stack->np-1 for the particle index.
    The convention for region numbers used by the geometry package is
    that the outside region has index -1 and inside regions are between 0
    and \c nreg-1. However, this is translated to the convention typically
    adopted by EGSnrc mortran user codes that the outside region is region
    1 and inside regions are 2 to \c nreg+1 by EGS_SimpleApplication.
    We have to therefore subtract 1 from the region index and change it
    to \c nreg+1 if it is 0 (particle is outside) and the particle is
    movinge forward to obtain the \c edep region for collecting the
    energy:
    \skipline if(
    We then simply use the \link EGS_ScoringArray::score score() \endlink
    function of the scoring array object to score the energy
    \skip edep
    \until }
    \until }
    This is all about \c ausgab. We automatically obtain history-by-history
    statistical analysis, provided we inform our scoring object each time
    the simulation of a new history begins. This is accomplished by
    reimplementing the \c startHistory function, which is called from
    within the \link EGS_SimpleApplication::run() shower loop \endlink
    before transporting a new particle from the source:
    \skipline startHistory

    The final step in the implementation of the Tutor2_Application class
    is to report the simulation results. For this purpose we define
    a separate function \c reportResults, which will be called from our
    main program.
    In the implementation of this function we simply use the
    \link EGS_ScoringArray::reportResults() reportResults() \endlink
    method of the scoring array. This method divides the accumulated
    results by the number of statistically independent events. Hence,
    in order to get fractions of the energy imparted into the phantom,
    we have to multiply by this number (available in the
    \link EGS_SimpleApplication::last_case last_case \endlink
    data member) and divide by the total energy that entered the phantom
    (available in the \link EGS_SimpleApplication::Etot
     Etot \endlink data member). Our
    \c reportResults tfunction herefore looks like this
    \skipline void reportResults()
    \until };
    and completes the implementation of the Tutor2_Application class.


    The main program of the tutor2pp application is very simple:
    we simple construct an instance of the Tutor2_Application class,
    passing the command line arguments to it,
    \skipline int main
    \skipline Tutor2_Application app
    call its inherited shower loop function,
    \skipline app.run
    report the results using our \c reportResults function,
    \skipline app.report
    and finish the simulation colling the inherited \c finish method
    \skipline app.finish
    \until }
    That's all. With 32 lines of code (excluding comments and blank lines)
    we have implemented a complete EGSnrc C++ application, which provides
    much more functionality than the original \c tutor2 mortran application
    (97 lines of code excluding comments and blank lines).

    \dontinclude tutor2pp/Makefile
    We now need a Makefile for the \c tutor2pp application. We include
    the general EGSnrc config file, the general egspp config file and
    the config file describing our C++ compiler:
    \skipline EGS_CONFIG
    \until egspp_
    We then set the name of our user code
    \skipline USER
    We will be using the generic Makefile for C++ applications provided
    with the distribution (see below). This generic Makefile offers the
    possibility to add extra files with mortran macros and mortran code
    when building the EGSnrc mortran functions and subroutines by
    setting the \c EGSPP_USER_MACROS make variable. In our case we
    don't use this capability and therefore leave \c EGSPP_USER_MACROS empty:
    \skipline EGSPP_USER_MACROS
    We also need to specify the base class from which our application
    class was derived:
    \skipline EGS_BASE
    and add dependences for the tutor2pp.cpp file. In our case the only
    additional dependence is the egs_scoring.h file, which we include
    in order to get access to the definition of the EGS_ScoringArray class:
    \skipline other_dep
    The make variable \c ABS_EGSPP is defined in \c egspp1.spec
    to be the absolute path to the egspp sources (\c \$HEN_HOUSE/egs++/)
    Finally we simply include the generic Makefile for EGSnrc C++
    applications, which provides rules for compiling the various sources
    and building the application:
    \skipline include

    We now can build our application by simply going to the \c tutor2pp
    directory and typing \c make. The resulting executable will be
    called \c tutor2pp (\c tutor2pp.exe on Windows) and will be put
    into the \c bin/\$my_machine subdirectory of \c \$EGS_HOME.

    To run the application, we simply type
    \verbatim
    tutor2pp -i test1 -p tutor_data
    \endverbatim
    with \c test1 being the name of the input file without extension
    (\c test1.egsinp is an example input file provided with the
    distribution) and \c tutor_data the name of the PEGS data file
    for the media of the geometry defined in \c test1. Alternatively,
    we can use the \c egs_gui graphical user interface.
    We can also set the number of histories to be run on the command line
    with the \c -n option:
    \verbatim
    tutor2pp -i test1 -p tutor_data -n 1000000
    \endverbatim
    (all applications derived from EGS_SimpleApplication automatically
     understand the \c -n option).

    \anchor a_source
    Here is the complete source code of the tutor2pp application:

*/
