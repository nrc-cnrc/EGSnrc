/*****************************************************************************
 *
 * EGSnrc application to model gamma spectrometry
 *
 *****************************************************************************/

//TODO: Allow (energy dependent?) detector resolution to smear signal
//TODO: Fit a Gaussian to expected peaks, and a line across for background subtraction

#include "egs_advanced_application.h"
#include "sources/egs_radionuclide_source/egs_radionuclide_source.h"
#include "egs_scoring.h"
#include "egs_interface2.h"
#include "egs_input.h"

#include <fstream>
#include <iomanip>

using namespace std;


class APP_EXPORT EGS_GammaSpecApplication : public EGS_AdvancedApplication {
public:

    // constructor
    EGS_GammaSpecApplication(int argc, char **argv) :
        EGS_AdvancedApplication(argc,argv), spectrum(0), spectrum_perf(0),
        score(0), score_perf(0), nbins(100),
        nreg(0), Etot(0), current_weight(1) {}


    // destructor
    ~EGS_GammaSpecApplication() {
        if (score) {
            delete score;
        }
        if (spectrum) {
            delete spectrum;
        }
        if (score_perf) {
            delete score_perf;
        }
        if (spectrum_perf) {
            delete spectrum_perf;
        }
    }

    // describe the application
    void describeUserCode() const;

    // initialize scoring
    int initScoring();

    // simulate a single shower
    int simulateSingleShower();

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
    void getCurrentResult(double &sum, double &sum2, double &norm,
                          double &count);

    // write spectrum to file
    void outputResponse();

protected:

    // start a new shower
    int startNewShower();

private:
    // spectrum minimum and maximum, bin size, minimum detectable energy
    EGS_Float        Emin, Emax, binWidth, minDetectorEnergy;
    int              nbins;             // number of bins to score spectra
    vector<int>      scoringRegions;    // regions in which spectrum is scored
    double           Etot;              // total energy that has entered the geometry
    int              nreg;              // number of regions in the geometry

    EGS_I64          currentSourceParticle;

    // Scoring arrays that track energy depositions
    EGS_ScoringArray *score, *score_perf;
    // Spectrum scoring arrays
    EGS_ScoringArray *spectrum, *spectrum_perf;
    // The weight of the initial particle that is currently being simulated
    EGS_Float        current_weight;

    // Vectors to hold the spectra for post-processing, and uncertainties
    vector<double> spec, specUnc, spec_perf, specUnc_perf;

    EGS_Application *app;
    vector<EGS_Float> gammaEnergies, peakEfficiency, peakEfficiencyUnc, peakEfficiency_perf, peakEfficiencyUnc_perf;

    void calculateEfficiencies(vector<double> &spectr, vector<double> &spectrUnc, vector<double> &peakEff, vector<double> &peakEffUnc, bool isPerfect);
};


// describeUserCode
void EGS_GammaSpecApplication::describeUserCode() const {
    egsInformation(
        "\n***************************************************"
        "\n*                                                 *"
        "\n*                  egs_gammaspec                  *"
        "\n*                                                 *"
        "\n***************************************************"
        "\n\n");
}


// initScoring
int EGS_GammaSpecApplication::initScoring() {
    app = EGS_Application::activeApplication();

    egsInformation("\nInitializing scoring for egs_gammaspec:\n");
    egsInformation("======================================================\n");

    // parse scoring options
    EGS_Input *options = input->takeInputItem("scoring options");
    if (!options) {
        egsFatal("Error: no :start scoring options: input block found.\nAborting.\n");
    }

    // parse spectrum input options
    options = options->takeInputItem("output spectrum");
    if (!options) {
        egsFatal("Error: no ':start output spectrum:' input block found in scoring options.\nAborting.\n");
    }

    // label of regions to score spectrum
    string scoringRegions_str;
    if (!options->getInput("scoring regions", scoringRegions_str)) {}
    else {
        egsFatal("Error: 'scoring regions' undefined in 'output spectrum' input block.\nAborting.\n");
    }

    // Convert any labels into region numbers
    app->getNumberRegions(scoringRegions_str, scoringRegions);
    app->getLabelRegions(scoringRegions_str, scoringRegions);

    // energy window (default is 0 to source->getEmax)
    Emin = 0;
    Emax = source->getEmax();
    EGS_Float myE;
    if (!options->getInput("minimum spectrum energy", myE)) {
        Emin = myE;
    }
    if (!options->getInput("maximum spectrum energy", myE)) {
        Emax = myE;
    }
    egsInformation("Minimum output spectrum energy = %f MeV\n", Emin);
    egsInformation("Maximum output spectrum energy = %f MeV\n", Emax);

    // Get detector energy resolution
    if (!options->getInput("minimum detectable energy", myE)) {
        minDetectorEnergy = myE;
    }
    else {
        minDetectorEnergy = 1e-6;
    }
    egsInformation("Minimum energy resolved by detector = %f MeV\n", minDetectorEnergy);

    // number of bins for scoring spectra (default is 1000)
    int mybins = 1000;
    if (!options->getInput("number of bins", mybins)) {
        nbins = mybins;
    }
    egsInformation("Number of spectrum bins = %d\n", nbins);
    spec.resize(nbins);
    specUnc.resize(nbins);
    spec_perf.resize(nbins);
    specUnc_perf.resize(nbins);

    // set energy bin size
    binWidth = (Emax-Emin)/(double)nbins;
    egsInformation("Bin width = %f MeV\n", binWidth);

    // Get the gamma energies that will be used to calculate efficiency
    // These ones are manually input by user
    options->getInput("gamma analysis energies", gammaEnergies);

    // These ones are automatically obtained from the radionuclide source
    vector<string> allowed;
    allowed.push_back("no");
    allowed.push_back("yes");
    int useRadionuclideGammas = options->getInput("automatic analysis energies",allowed,1);
    if (useRadionuclideGammas) {
        egsInformation("\nGetting gamma analysis energies automatically from radionuclide source...\n");
        if (gammaEnergies.size() > 0) {
            egsInformation("Note that both the 'gamma analysis energies' input and gamma energies automatically extracted from the radionuclide decay scheme will be used. Make sure they don't overlap!\n");
        }

        vector<EGS_Ensdf *> decays = source->getRadionuclideEnsdf();
        // There may be several radionuclides represented by one radionuclide source
        // So we loop through all of them to get all the possible gamma energies
        for (auto dec: decays) {
            for (auto gamma: dec->getGammaRecords()) {
                gammaEnergies.push_back(gamma->getDecayEnergy());
            }
            for (auto gamma: dec->getUncorrelatedGammaRecords()) {
                gammaEnergies.push_back(gamma->getDecayEnergy());
            }
        }
    }

    if (gammaEnergies.size() > 0) {
        egsInformation("\nGamma analysis energies =");
        for (const auto &value: gammaEnergies) {
            egsInformation(" %f", value);
        }
        egsInformation("\n");

        peakEfficiency.resize(gammaEnergies.size());
        peakEfficiencyUnc.resize(gammaEnergies.size());
        peakEfficiency_perf.resize(gammaEnergies.size());
        peakEfficiencyUnc_perf.resize(gammaEnergies.size());
    }

    delete options;

    // allocate scoring arrays
    nreg     = geometry->regions();
    score    = new EGS_ScoringArray(nreg);
    spectrum = new EGS_ScoringArray(nbins);
    score_perf    = new EGS_ScoringArray(nreg);
    spectrum_perf = new EGS_ScoringArray(nbins);

    currentSourceParticle = 0;

    // return
    return 0;
}


// ausgab
int EGS_GammaSpecApplication::ausgab(int iarg) {

    // index of current particle and current region
    int np = the_stack->np - 1;
    int ir = the_stack->ir[np]-2;
    int charge = the_stack->iq[np];

    // score energy deposited in each region before particle is discarded
    if (iarg <= 4) {
        if (ir >= 0) {
            // don't include weight here; see simulateSingleShower()
            score->score(ir,the_epcont->edep);
            score_perf->score(ir,the_epcont->edep);
        }
    }

    return 0;
}


// simulate one shower
int EGS_GammaSpecApplication::simulateSingleShower() {

    // call base class function
    int err = EGS_AdvancedApplication::simulateSingleShower();

    // =======================
    // For perfect detectors
    // sum all energy deposited in the detector for this shower
    EGS_Float myEnergy = 0.0;
    int size = scoringRegions.size();
    for (int k=0; k<size; k++) {
        myEnergy += score_perf->thisHistoryScore(scoringRegions[k]);
    }

    // calculate spectrum bin number
    if (myEnergy > minDetectorEnergy) {
        int mybin = (int)(myEnergy/binWidth);
        if (mybin == nbins) {
            mybin--;
        }
        if (mybin >= 0 && mybin < nbins) {
            // apply particle weight here to the bin count
            spectrum_perf->score(mybin,current_weight);
        }
    }
    // =======================

    return err;
}

// outputData
int EGS_GammaSpecApplication::outputData() {
    int err = EGS_AdvancedApplication::outputData();
    if (err) {
        return err;
    }
    (*data_out) << "  " << Etot << endl;
    (*data_out) << "  " << currentSourceParticle << endl;
    if (!score->storeState(*data_out)) {
        return 101;
    }
    if (!spectrum->storeState(*data_out)) {
        return 102;
    }
    if (!score_perf->storeState(*data_out)) {
        return 101;
    }
    if (!spectrum_perf->storeState(*data_out)) {
        return 102;
    }
    return 0;
}


// readData
int EGS_GammaSpecApplication::readData() {
    int err = EGS_AdvancedApplication::readData();
    if (err) {
        return err;
    }
    (*data_in) >> Etot;
    (*data_in) >> currentSourceParticle;
    if (!score->setState(*data_in)) {
        return 101;
    }
    if (!spectrum->setState(*data_in)) {
        return 102;
    }
    if (!score_perf->setState(*data_in)) {
        return 101;
    }
    if (!spectrum_perf->setState(*data_in)) {
        return 102;
    }
    return 0;
}

// resetCounter
void EGS_GammaSpecApplication::resetCounter() {
    EGS_AdvancedApplication::resetCounter();
    score->reset();
    spectrum->reset();
    score_perf->reset();
    spectrum_perf->reset();
    Etot = 0;
    currentSourceParticle = 0;
}

// addState
int EGS_GammaSpecApplication::addState(istream &data) {
    int err = EGS_AdvancedApplication::addState(data);
    if (err) {
        return err;
    }
    double etot_tmp;
    data >> etot_tmp;
    Etot += etot_tmp;
    EGS_I64 count_tmp;
    data >> count_tmp;
    currentSourceParticle += count_tmp;
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
    if (!tmp_score.setState(data)) {
        return 103;
    }
    (*score_perf) += tmp_score;
    if (!tmp_spectrum.setState(data)) {
        return 104;
    }
    (*spectrum_perf) += tmp_spectrum;
    return 0;
}


// outputResults
void EGS_GammaSpecApplication::outputResults() {
    egsInformation("\n======================================================\n");
    egsInformation("Results output for egs_gammaspec:\n");
    egsInformation("======================================================\n");

    egsInformation("=> last case = %lld fluence = %g\n", current_case, source->getFluence());

    egsInformation("\nTotal energy emitted from source = %g MeV\n", Etot);

    outputResponse();

    egsInformation("\nSpectrum scoring regions:\n");
    for (int k=0; k<scoringRegions.size(); k++) {
        egsInformation("%d ", scoringRegions[k]);
    }
    egsInformation("\n");

    // Print the emissions sampled
    source->printSampledEmissions();
}


// outputResponse
void EGS_GammaSpecApplication::outputResponse() {

    // TODO: Should only really output these for the final job because this data is duplicated in the egsdat file
    string specFilename = constructIOFileName("-spec.txt",true);
    string specFilename_perf = constructIOFileName("-spec-perf.txt",true);

    fstream spec_f, spec_perf_f;
    spec_f.open(specFilename.c_str(), fstream::out);
    spec_perf_f.open(specFilename_perf.c_str(), fstream::out);

    // =======================
    // For non-perfect detectors
    spec_f << scientific;
    spec_f << setprecision(6);
    double totalE = 0.0;
    for (int i=0; i<nbins; i++) {
        double x = Emin + (i+1)*binWidth;
        spectrum->currentResult(i, spec[i], specUnc[i]);
        if (spec[i] > 0) {
            totalE += spec[i]*(Emin + (i+0.5) * binWidth);
        }
        spec_f  << setw(16) << x-binWidth/2.
                << setw(16) << spec[i]
                << setw(16) << specUnc[i]
                << endl;
    }

    // report total energy fraction recorded in spectrum
    totalE = totalE * current_case / Etot;
    egsInformation("Total energy fraction recorded in raw non-perfect detector spectrum = %g\n", totalE);

    // =======================
    // For perfect detectors
    spec_perf_f << scientific;
    spec_perf_f << setprecision(6);
    totalE = 0.0;
    for (int i=0; i<nbins; i++) {
        double x = Emin + (i+1)*binWidth;
        spectrum_perf->currentResult(i, spec_perf[i], specUnc_perf[i]);
        if (spec_perf[i] > 0) {
            // Switch to relative uncertainty just for the normalization
            specUnc_perf[i] /= spec_perf[i];
            // Normalize to 'per decay' instead of 'per source particle'
            spec_perf[i] *= double(currentSourceParticle) / current_case;
            specUnc_perf[i] *= spec_perf[i];

            totalE += spec_perf[i]*(Emin + (i+0.5) * binWidth);
        }
        spec_perf_f  << setw(16) << x-binWidth/2.
                     << setw(16) << spec_perf[i]
                     << setw(16) << specUnc_perf[i]
                     << endl;
    }

    // report total energy fraction recorded in spectrum
    totalE = totalE * current_case / Etot;
    egsInformation("Total energy fraction recorded in raw perfect detector spectrum = %g\n", totalE);

    // close spectrum files
    spec_f.close();
    spec_perf_f.close();

    // Do background correction and calculate efficiency
    calculateEfficiencies(spec, specUnc, peakEfficiency, peakEfficiencyUnc, false);
    calculateEfficiencies(spec_perf, specUnc_perf, peakEfficiency_perf, peakEfficiencyUnc_perf, true);

    // Print summing corrections
    egsInformation("\n=== Coincidence summing correction ===\n\n");
    egsInformation("Gamma energy [MeV] | Summing correction (perfect/non-perfect) | Uncertainty [%%]\n");
    for (size_t i=0; i<gammaEnergies.size(); ++i) {
        EGS_Float summingCorrection;
        if (peakEfficiency[i] > 0) {
            summingCorrection = peakEfficiency_perf[i] / peakEfficiency[i];
        }
        else {
            summingCorrection = 0;
        }
        EGS_Float summingCorrectionUnc = sqrt(pow(peakEfficiencyUnc_perf[i], 2) + pow(peakEfficiencyUnc[i], 2));

        egsInformation("%f %f %f\n", gammaEnergies[i], summingCorrection, summingCorrectionUnc);
    }
}

void EGS_GammaSpecApplication::calculateEfficiencies(vector<double> &spectr, vector<double> &spectrUnc, vector<double> &peakEff, vector<double> &peakEffUnc, bool isPerfect) {
    if (isPerfect) {
        egsInformation("\n=== Efficiency for 'perfect' detector ===\n");
    }
    else {
        egsInformation("\n=== Efficiency for 'non-perfect' detector ===\n");
    }

    // Loop through the whole spectrum to get the total efficiency and full energy peak efficiency
    EGS_Float totalEff = 0, fullEnergyPeakEff = 0,
              totalEffUnc = 0, fullEnergyPeakEffUnc = 0;
    for (size_t i=0; i<spectr.size(); ++i) {
        totalEff += spectr[i];
        totalEffUnc += pow(spectrUnc[i], 2);

        if (spectr[i] > fullEnergyPeakEff) {
            fullEnergyPeakEff = spectr[i];
            fullEnergyPeakEffUnc = spectrUnc[i];
        }
    }
    totalEffUnc = sqrt(totalEffUnc) / totalEff * 100;
    fullEnergyPeakEffUnc = fullEnergyPeakEffUnc / fullEnergyPeakEff * 100;
    egsInformation("Total efficiency = %f %% +- %f %%\n", totalEff*100, totalEffUnc);
    egsInformation("Full energy peak efficiency = %f %% +- %f %%\n", fullEnergyPeakEff*100, fullEnergyPeakEffUnc);

    // Do processing of gamma peak efficiencies
    // If there's no array provided, just return
    if (gammaEnergies.size() < 1) {
        return;
    }
    egsInformation("\nGamma energy [MeV] | Peak efficiency (background subtracted) [%%] | Uncertainty [%%]\n");

    double background, backgroundUnc;
    for (size_t i=0; i<gammaEnergies.size(); ++i) {
        size_t ind1 = ceil(gammaEnergies[i]/binWidth-1);
        size_t ind2 = ceil(gammaEnergies[i]/binWidth-0.5);

        if (ind1 > 0 && ind1 < nbins && ind2 > 0 && ind2 < nbins) {
            background = (spectr[ind1-1] + spectr[ind2+1])/2;
            backgroundUnc = (pow(spectrUnc[ind1-1],2) + pow(spectrUnc[ind2+1],2))/4;
        }
        else {
            background = 0;
            backgroundUnc = 0;
        }

        peakEff[i] = 0;
        peakEffUnc[i] = 0;
        for (size_t j=ind1; j<ind2+1; ++j) {
            peakEff[i] += spectr[j] - background;
            peakEffUnc[i] += pow(spectrUnc[j],2) + backgroundUnc;
        }

        if (peakEff[i] < 0) {
            peakEff[i] = 0;
            peakEffUnc[i] = 0;
        }
        else if (peakEff[i] > 0) {
            peakEffUnc[i] = sqrt(peakEffUnc[i]) / peakEff[i] * 100;
        }

        egsInformation("%f %f %f\n", gammaEnergies[i], peakEff[i]*100, peakEffUnc[i]);
    }

    egsInformation("\nPeak efficiency calculations may combine two bins when the peak is between bins, and the background is the average of the two bins outside the peak bins. For the spectrum array that is indexed from 0, the peak efficiency is the sum of the range from index ind1 to ind2, and the background averages the bins ind1-1 and ind2+2. The following table shows which bins were used for each peak, and the calculated background:\n");
    egsInformation("\nGamma energy [MeV] | ind1 | ind2 | Background [%%]\n");

    for (size_t i=0; i<gammaEnergies.size(); ++i) {
        size_t ind1 = ceil(gammaEnergies[i]/binWidth-1);
        size_t ind2 = ceil(gammaEnergies[i]/binWidth-0.5);

        if (ind1 > 0 && ind1 < nbins && ind2 > 0 && ind2 < nbins) {
            background = (spectr[ind1-1] + spectr[ind2+1])/2;
            backgroundUnc = (pow(spectrUnc[ind1-1],2) + pow(spectrUnc[ind2+1],2))/4;
        }
        else {
            background = 0;
            backgroundUnc = 0;
        }

        egsInformation("%f %d %d %f\n", gammaEnergies[i], ind1, ind2, background*100);
    }
}


// getCurrentResult
void EGS_GammaSpecApplication::getCurrentResult(double &sum, double &sum2, double &norm, double &count) {
    score->currentScore(0,sum,sum2);
}

// startNewShower
int EGS_GammaSpecApplication::startNewShower() {

    // =======================
    // For non-perfect detectors

    // Do some scoring for the previous shower, if this particle is part
    // of a new decay and it's not a perfect detector
    if (current_case != last_case) {
        // Sum all energy deposited in the detector for the previous shower
        EGS_Float myEnergy = 0.0;
        int size = scoringRegions.size();
        for (int k=0; k<size; k++) {
            myEnergy += score->thisHistoryScore(scoringRegions[k]);
        }

        // Calculate spectrum bin number
        if (myEnergy > minDetectorEnergy) {
            int mybin = (int)(myEnergy/binWidth);
            if (mybin == nbins) {
                mybin--;
            }
            if (mybin >= 0 && mybin < nbins) {
                // Apply particle weight here to the bin count
                spectrum->score(mybin,current_weight);
            }
        }
    }
    // =======================

    // Add up a tally of all of the emitted energy
    Etot += p.E*p.wt;

    current_weight = p.wt;
    int res = EGS_Application::startNewShower();
    if (res) {
        return res;
    }

    score_perf->setHistory(currentSourceParticle);
    spectrum_perf->setHistory(currentSourceParticle);

    if (current_case != last_case) {

        score->setHistory(current_case);
        spectrum->setHistory(current_case);
        last_case = current_case;
    }

    currentSourceParticle++;

    return 0;
}

extern "C" {
    APP_EXPORT shared_ptr<EGS_InputStruct> getAppSpecificInputs() {
        shared_ptr<EGS_InputStruct> appInput = make_shared<EGS_InputStruct>();

        shared_ptr<EGS_BlockInput> scoreBlock = appInput->addBlockInput("scoring options");
        scoreBlock->setAppName("egs_phd");

        shared_ptr<EGS_BlockInput> specBlock = scoreBlock->addBlockInput("output spectrum");
        specBlock->addSingleInput("scoring regions", true, "A list of regions (or labels) that denote the sensitive regions for scoring.");
        specBlock->addSingleInput("minimum spectrum energy", false, "The minimum energy for the energy bins. Defaults to 0.");
        specBlock->addSingleInput("maximum spectrum energy", false, "The maximum energy for the energy bins. Defaults to the maximum energy in the source.");
        specBlock->addSingleInput("number of bins", false, "The number of energy bins. Defaults to 1000.");
        specBlock->addSingleInput("automatic analysis energies", false, "Get the analysis energies automatically, using all of the gamma energies from the radionuclide decay scheme. They will be combined with the manually entered 'gamma analysis energies', so make sure they don't overlap. Only works for egs_radionuclide_source. Defaults to yes.", {"Yes", "No"});
        specBlock->addSingleInput("gamma analysis energies", false, "The energies to use for analysis. I.e. coincidence summing corrections will be calculated for each of these.");
        specBlock->addSingleInput("minimum detectable energy", false, "The minimum energy from a single shower that can be detected in the sensitive regions. If the total energy deposited by the shower is less than this value, it is not added to the recorded spectrum. Defaults to 1e-6.");

        return appInput;
    }

    APP_EXPORT string getAppSpecificExample() {
        string example;
        example = {
        R"(
:start scoring options:

    :start output spectrum:
        scoring regions = crystal_no_dead_label # The region numbers or region label(s) denoting the sensitive regions

        minimum spectrum energy  = 0.0 # Optional, MeV, default=0, minimum energy in output spectrum
        maximum spectrum energy = 0.4 # Optional, MeV, defaults=maximum in source, maximum energy in output spectrum
        number of bins  = 2000 # Optional, default=1000, number of bins in output spectrum

        # Optional, yes or no, default=yes, get the analysis energies automatically, using all of the gamma energies from the radionuclide decay scheme. They will be combined with the manually entered 'gamma analysis energies', so make sure they don't overlap. Only works for egs_radionuclide_source.
        automatic analysis energies = no

        # Example: these are peaks of interest for Ba-133, in MeV
        # Optional, coincidence summing corrections will be calculated for each of these
        gamma analysis energies = .0309 .035 .0531 .0796 .0810 .1606 .2232 .2764 .3028 .356 .3838

        # Optional, MeV, default=1e-6, the minimum energy from a single shower that can be detected in the sensitive regions. If the total energy deposited by the shower is less than this value, it is not added to the recorded spectrum.
        minimum detectable energy = 1e-6
    :stop output spectrum:

:stop scoring options:
)"};
        return example;
    }
}

#ifdef BUILD_APP_LIB
    APP_LIB(EGS_GammaSpecApplication);
#else
    APP_MAIN(EGS_GammaSpecApplication);
#endif
