/*
###############################################################################
#
#  EGSnrc egs++ pulse-height distribution (phd) sample application headers
#  Copyright (C) 2018 National Research Council Canada
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
#  Authors:         Frederic Tessier, 2018
#
#  Contributors:
#
###############################################################################
*/


#ifndef EGS_PHD_H
#define EGS_PHD_H

#include "egs_advanced_application.h"
#include "egs_interface2.h"
#include "egs_scoring.h"
#include "egs_input.h"

#include <fstream>
#include <iomanip>


// application class
class APP_EXPORT phd_app : public EGS_AdvancedApplication {

    int                 nbin;                  // number of bins to score spectra
    EGS_Float           Emin, Emax, Ebin;      // spectrum minimum, maximum, and bin size
    std::string         m_spectrumFilename;    // file name for spectrum output
    std::vector<int>    spectrum_regions;      // regions in which spectrum is scored
    double              Etot;                  // total energy that has entered the geometry
    int                 nreg;                  // number of regions in the geometry

    EGS_ScoringArray    *score;                // scoring array for deposited energy
    EGS_ScoringArray    *spectrum;             // scoring array for spectrum
    EGS_Float           initial_weight;        // the weight of the incident particle

public:

    // constructor
    phd_app(int argc, char **argv) :
        EGS_AdvancedApplication(argc, argv),
        nbin(100),
        Emin(0),
        Emax(1),
        Ebin(100),
        Etot(0),
        nreg(0),
        score(0),
        spectrum(0),
        initial_weight(1) {}


    // destructor
    ~phd_app() {
        if (score) {
            delete score;
        }
        if (spectrum) {
            delete spectrum;
        }
    }

private:

    // describe the application
    void describeUserCode() const;

    // initialize scoring
    int initScoring();

    // accumulate quantities of interest at run time
    int ausgab(int iarg);

    // output intermediate results to the .egsdat file
    int outputData();

    // read results from a .egsdat file
    int readData();

    // reset the variables used for accumulating results
    void resetCounter();

    // add simulation results
    int addState(istream &data);

    // output the results of a simulation
    void outputResults();

    // get the current simulation result
    void getCurrentResult(double &sum,
                          double &sum2,
                          double &norm,
                          double &count);

    // write spectrum to file
    void outputResponse(EGS_ScoringArray *spec);

    // start a new shower
    int startNewShower();

};


#endif // EGS_PHD_H
