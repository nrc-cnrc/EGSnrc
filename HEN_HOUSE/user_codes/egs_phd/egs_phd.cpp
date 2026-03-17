/*
###############################################################################
#
#  EGSnrc egs++ pulse-height distribution (phd) sample application
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
#
#   A small demonstration application to score a pulse-height distribution in a
#   labeled set of regions. Application specific options are set in the input
#   file, in the 'scoring options' input block, e.g.,
#
#   :start scoring options:
#
#       :start spectrum:
#           label = detector
#           Emin  = 0.0
#           Emax  = 1.0
#           bins  = 100
#           spectrum file = spectrum.dat
#       :stop spectrum:
#
#   :stop scoring options:
#
#   The included sample input file ge-example.egsinp models a germanium detector
#   inside a lead shield and scores the pulse-height distribution in the
#   germanium.
#
###############################################################################
*/


#include "egs_phd.h"

// describeUserCode
void phd_app::describeUserCode() const {
    egsInformation(
        "\n***************************************************"
        "\n*                                                 *"
        "\n*         EGSnrc spectrum application             *"
        "\n*                                                 *"
        "\n***************************************************"
        "\n\n");
}


// initScoring
int phd_app::initScoring() {

    // get scoring options input block
    EGS_Input *options = input->takeInputItem("scoring options");
    if (!options) {
        egsFatal("ERROR: no :start scoring options: input block found.\nAborting.");
    }

    // parse spectrum input options
    options = options->takeInputItem("spectrum");
    if (!options) {
        egsFatal("ERROR: no :start spectrum: input block found in scoring options.\nAborting.");
    }

    // get label of regions to score spectrum
    std::string mylabel;
    if (!options->getInput("label", mylabel)) {}
    else {
        egsFatal("ERROR: label = undefined in spectrum input block.\nAborting.");
    }

    // get energy window: default is 0 to source->getEmax
    Emin = 0;
    Emax = source->getEmax();
    EGS_Float myE;
    if (!options->getInput("Emin", myE)) {
        Emin = myE;
    }
    if (!options->getInput("Emax", myE)) {
        Emax = myE;
    }

    // get number of bins for scoring spectra (default is 100)
    int mybins = 100;
    if (!options->getInput("bins", mybins)) {
        nbin = mybins;
    }

    // get specturm output file name: default is spectrum.dat
    std::string spectrumFilename = "spectrum.dat";
    if (!options->getInput("spectrum file", spectrumFilename)) {
        m_spectrumFilename = spectrumFilename;
    }

    // set radiative splitting
    int nbrsplit;
    if (!options->getInput("radiative splitting", nbrsplit) && nbrsplit > 1) {
        the_egsvr->nbr_split = nbrsplit;
        the_egsvr->i_play_RR = 1;
        the_egsvr->prob_RR = 1.0/nbrsplit;
    }

    // clean up
    delete options;

    // set energy bin size
    Ebin = (Emax-Emin)/((double) nbin);

    // allocate scoring arrays
    nreg     = geometry->regions();
    score    = new EGS_ScoringArray(nreg);
    spectrum = new EGS_ScoringArray(nbin);

    // get list of spectrum scoring regions by label
    geometry->getLabelRegions(mylabel, spectrum_regions);

    return 0;
}


// ausgab
int phd_app::ausgab(int iarg) {

    // index of current particle and current region
    int np = the_stack->np - 1;
    int ir = the_stack->ir[np]-2;

    // score energy deposited in each region before particle is discarded
    if (iarg <= 4) {
        if (ir >= 0) {
            score->score(ir, the_epcont->edep);    // don't include weight here; see startNewShower()
        }
    }

    return 0;
}


// outputData
int phd_app::outputData() {

    int err = EGS_AdvancedApplication::outputData();
    if (err) {
        return err;
    }

    (*data_out) << "  " << Etot << endl;

    if (!score->storeState(*data_out))  {
        return 101;
    }

    if (!spectrum->storeState(*data_out)) {
        return 102;
    }

    return 0;
}


// readData
int phd_app::readData() {

    int err = EGS_AdvancedApplication::readData();
    if (err) {
        return err;
    }

    (*data_in) >> Etot;

    if (!score->setState(*data_in)) {
        return 101;
    }

    if (!spectrum->setState(*data_in)) {
        return 102;
    }

    return 0;
}


// resetCounter
void phd_app::resetCounter() {
    EGS_AdvancedApplication::resetCounter();
    score->reset();
    spectrum->reset();
    Etot = 0;
}


// addState
int phd_app::addState(istream &data) {

    int err = EGS_AdvancedApplication::addState(data);
    if (err) {
        return err;
    }

    double etot_tmp;
    data >> etot_tmp;
    Etot += etot_tmp;

    EGS_ScoringArray tmp_score(nreg+2);
    if (!tmp_score.setState(data)) {
        return 101;
    }
    (*score) += tmp_score;

    EGS_ScoringArray tmp_spectrum(nreg+2);
    if (!tmp_spectrum.setState(data)) {
        return 102;
    }
    (*spectrum) += tmp_spectrum;

    return 0;
}


// outputResults
void phd_app::outputResults() {

    egsInformation("\n\n last case = %d Etot = %g\n", (int)current_case,Etot);
    egsInformation("\n spectrum scoring regions: ");

    for (int k=0; k<spectrum_regions.size(); k++) {
        egsInformation("%d ", spectrum_regions[k]);
    }
    egsInformation("\n");

    double norm = ((double)current_case)/Etot;
    outputResponse(spectrum);
}


// outputResponse
void phd_app::outputResponse(EGS_ScoringArray *spec) {

    std::fstream spectrum_file;
    spectrum_file.open(m_spectrumFilename.c_str(), std::fstream::out);

    // print spectrum bins
    spectrum_file << scientific;
    spectrum_file << setprecision(6);
    spectrum_file << setw(16) << Emin << setw(16) << 0 << setw(16) << 0 << endl;
    double Espectrum = 0.0;
    for (int i=0; i<nbin; i++) {
        double y, dy;
        double x = Emin + (i+1)*Ebin;
        spectrum->currentResult(i, y, dy);
        Espectrum += y*(Emin+(i+0.5)*Ebin);
        y = y*current_case;
        dy = dy*current_case;
        spectrum_file << setw(16) << x
                      << setw(16) << y
                      << setw(16) << dy
                      << endl;
    }

    // report total energy fraction recorded in spectrum
    Espectrum = Espectrum * current_case / Etot ;
    egsInformation(" total energy fraction recorded in spectrum: %g", Espectrum);

    // close spectrum file
    spectrum_file.close();
}


// getCurrentResult
void phd_app::getCurrentResult(double &sum,
                                    double &sum2,
                                    double &norm,
                                    double &count) {
    score->currentScore(0, sum, sum2);
}


// startNewShower
int phd_app::startNewShower() {

    Etot += p.E*p.wt;
    initial_weight = p.wt;

    int res = EGS_Application::startNewShower();
    if (res) {
        return res;
    }

    // Whenever we are about to start a new history, first score
    // the results for the previous history into the PHD spectrum
    // Caution: This means that the very last history will not be scored
    // into the PHD spectrum
    if (current_case != last_case) {
        // sum all energy deposited in the spectrum regions for the current shower
        EGS_Float myEnergy = 0.0;
        for (int k=0; k<spectrum_regions.size(); k++) {
            myEnergy += score->thisHistoryScore(spectrum_regions[k]);
        }

        // calculate spectrum bin number and score count
        if (myEnergy > 1e-9) {
            int mybin = (int)((myEnergy-Emin)/Ebin);
            if (mybin == nbin) {
                mybin--;
            }
            if (mybin >= 0 && mybin < nbin) {
                spectrum->score(mybin, initial_weight);          // apply incident particle weight here to the bin count
            }
        }

        // Reset the scoring arrays for the new history that's about to start
        score->setHistory(current_case);
        spectrum->setHistory(current_case);
        last_case = current_case;
    }

    return 0;
}


// main application macro
APP_MAIN(phd_app);
