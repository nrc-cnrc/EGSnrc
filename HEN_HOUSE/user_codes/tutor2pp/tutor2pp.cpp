/*
###############################################################################
#
#  EGSnrc egs++ tutor2pp application
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
#                   Reid Townson
#
###############################################################################
#
#  A simple EGSnrc application using the C++ interface. It implements the
#  functionality of the original tutor2 tutorial code written in mortran
#  except that now, due to the use of the general geometry and source packages
#  included in egs++, any geometry or any source can be used, not just a
#  monoenergetic 20 MeV source of electrons incident on a 1 mm plate of
#  tantalum as un tutor2.
#
###############################################################################
*/


//! We derive from EGS_SimpleApplication => we must include its header file
#include "egs_simple_application.h"
//! We use a scoring array object => need its header file.
#include "egs_scoring.h"
//! egs_interface2.h must be included by any C++ application.
#include "egs_interface2.h"
//! To get access to egsInformation(), etc.
#include "egs_functions.h"

class APP_EXPORT Tutor2_Application : public EGS_SimpleApplication {

    EGS_ScoringArray *edep;   // our scoring object
    int              nreg;    // number of regions in the geometry.

public:

    /*! \brief Constructor.

     Simply calls the EGS_SimpleApplication constructor passing the command
     line arguments to it and then constructs a scorring array object that will
     collect the energy deposited in the various regions during run time.
     In the EGS_SimpleApplication constructor the input file is read in,
     geometry, source and random number generators are initialized and the
     \c egs_init function is called. The \c egs_init function sets default
     values, creates a temporary working directory for the run and opens
     various Fortran I/O units needed by the mortran back-end. The media
     present in the geometry are then added to the mortran back-end and the
     HATCH subroutine is called to initialize the cross section data.
     Finally, a check is made that the cross section data files cover
     the energy range needed based on the maximum energy of the source.
    */
    Tutor2_Application(int argc, char **argv) :
        EGS_SimpleApplication(argc,argv) {
        nreg = g->regions();
        /*! We initialize the scorring array to have 2 more regions than the
            geometry so that we can collect the transmitted and reflected
            energy fractions in additions to the energy fractions deposited
            in the nreg regions of the geometry.
        */
        edep = new EGS_ScoringArray(nreg+2);
    };

    /*! \brief Destructor.

     It is a good coding practice to deallocate memory when objects
     go out of scope. That's what we do in the destructor of our application.
     */
    ~Tutor2_Application() {
        delete edep;
    };

    /*! \brief Scoring.

     Every application derived from EGS_SimpleApplication must provide an
     implementation of the ausgab function to score the quantities of
     interest. For the possible values of the iarg argument see the
     EGSnrc manual (PIRS-701). In tutor2pp we are only interested in energy
     deposition => only need \a iarg <= 4 calls.
     The particle stack is available to us via the pointer \c the_stack,
     the \c EPCONT common block via the pointer \c the_epcont.
     We have to remember that the stack pointer (\c the_stack->np), which
     points to the last particle on the stack, uses Fortran style indexing
     (\em i.e., it goes from 1 to np)
    */
    int ausgab(int iarg) {
        if (iarg <= 4) {
            //! Get the stack pointer and currect particle region index.
            int np = the_stack->np - 1;
            int ir = the_stack->ir[np]-1;
            /*! Per definition region index=0 corresponds to the outside,
               regions 1...nreg to the nreg regions inside the geometry.
               If the particle is outside, we say that it is 'reflected'
               if its direction cosine with respect to the z-axis is negative
               and use region 0 of the scoring array to score its energy.
               If the particle is outside but moving forward, we say that
               the particle is 'transmitted' and use region nreg+1 to score
               its energy.
            */
            if (ir == 0 && the_stack->w[np] > 0) {
                ir = nreg+1;
            }
            /*! Now simply use the score method of the EGS_ScoringArray class
                to record the energy deposited. */
            edep->score(ir,the_epcont->edep*the_stack->wt[np]);
        }

        return 0;
    };

    /*! \brief Statistics

     This function gets called before each new history.
     The argument icase is the history number returned from the
     source when sampling the next particle. In many cases this
     will be the same as the history number counter in the shower loop,
     but there are situations where this is not the case (e.g.
     for a phase space file source icase will be the number of statistically
     independent particles read so far from the file, which will be
     different than the number of particles read).
     We simply call here the setHistory() method of the scoring object.
     This is sufficient to get a history-by-history statistical analysis
     for the deposited energy fractions.
    */
    void startHistory(EGS_I64 icase) {
        edep->setHistory(icase);
    };


    /*! Output of results.

     Here we simply use the reportResults() method of the scoring array
     object, which requires as argumenst a normalization constant, a
     title (reproduced in the output),
     a boolean flag and a format string. The format string will be used
     to output the result and its uncertainty in each region. The flag
     determines if absolute uncertainties should be printed (false) or
     relative uncertainties (true). The result in each region is multiplied
     by the normalization constant. Note that the reportResults()
     method of the scoring object automatically
     divides by the number of statistically independent events.
     We want our results to be normalized as fractions of the energy
     imparted into the geometry => we need last_case/Etot as a normalization
     constant. The quantities last_case and Etot are collected by the
     EGS_Application base class during the run.
    */
    void reportResults() {
        double norm = ((double)last_case)/Etot;
        egsInformation(" last case = %d Etot = %g\n",(int)last_case,Etot);
        edep->reportResults(norm,
                            "Reflected/deposited/transmitted energy fraction",false,
                            "  %d  %9.5f +/- %9.5f %c\n");
    };

};

#ifdef BUILD_APP_LIB
APP_LIB(Tutor2_Application);
#else
APP_MAIN(Tutor2_Application);
#endif
