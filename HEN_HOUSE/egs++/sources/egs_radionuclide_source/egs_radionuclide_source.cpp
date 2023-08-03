/*
###############################################################################
#
#  EGSnrc egs++ radionuclide source
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
#  Author:          Reid Townson, 2016
#
#  Contributors:    Martin Martinov
#                   Ernesto Mainegra-Hing
#
###############################################################################
*/


/*! \file egs_radionuclide_source.cpp
 *  \brief A radionuclide source
 *  \RT
 */

#include "egs_radionuclide_source.h"
#include "egs_input.h"
#include "egs_math.h"
#include "egs_application.h"

EGS_RadionuclideSource::EGS_RadionuclideSource(EGS_Input *input,
        EGS_ObjectFactory *f) : EGS_BaseSource(input,f),
    baseSource(0), q_allowed(0), decays(0), activity(1), sCount(0) {

    int err;
    vector<int> tmp_q;
    err = input->getInput("charge", tmp_q);
    if (!err) {
        if (std::find(q_allowed.begin(), q_allowed.end(), -1) != q_allowed.end()
                && std::find(q_allowed.begin(), q_allowed.end(), 0) != q_allowed.end()
                && std::find(q_allowed.begin(), q_allowed.end(), 1) != q_allowed.end()
                && std::find(q_allowed.begin(), q_allowed.end(), 2) != q_allowed.end()
           ) {
            q_allowAll = true;
        }
        else {
            q_allowAll = false;
        }
        q_allowed = tmp_q;
    }
    else {
        q_allowAll = true;
        q_allowed.push_back(-1);
        q_allowed.push_back(1);
        q_allowed.push_back(0);
        q_allowed.push_back(2);
    }

    // Create the decay spectra
    count = 0;
    sCount = 0;
    Emax = 0;
    unsigned int i = 0;
    EGS_Float spectrumWeightTotal = 0;
    disintegrationOccurred = true;
    ishower = -1;
    time = 0;
    lastDisintTime = 0;
    while (input->getInputItem("spectrum")) {

        egsInformation("**********************************************\n");

        decays.push_back(createSpectrum(input));

        // If spectrum creation failed skip to the next spectrum block
        // We check the getShowerIndex function to ensure this cast to a
        // radionuclide spectrum succeeded. Other spectra will fail!
        if (!decays[i] || decays[i]->getShowerIndex() != -1) {
            decays.pop_back();
            continue;
        }

        EGS_Float spectrumMaxE = decays[i]->maxEnergy();
        if (spectrumMaxE > Emax) {
            Emax = spectrumMaxE;
        }

        spectrumWeightTotal += decays[i]->getSpectrumWeight();

        ++i;
    }
    if (decays.size() < 1) {
        egsFatal("\nEGS_RadionuclideSource: Error: No spectrum of type EGS_RadionuclideSpectrum was defined.\n");
    }

    // Normalize the spectrum weights
    for (i=0; i<decays.size(); ++i) {
        decays[i]->setSpectrumWeight(
            decays[i]->getSpectrumWeight() / spectrumWeightTotal);

        if (i > 0) {
            decays[i]->setSpectrumWeight(
                decays[i]->getSpectrumWeight() +
                decays[i-1]->getSpectrumWeight());
        }
    }

    // Get the activity
    EGS_Float tmp_A;
    err = input->getInput("activity", tmp_A);
    if (!err) {
        activity = tmp_A;
    }
    else {
        activity = 1;
    }

    egsInformation("EGS_RadionuclideSource: Activity [disintegrations/s]: %e\n",
                   activity);

    // Get the experiment time
    // This puts a limit on emission times - particles beyond the
    // limit are discarded
    err = input->getInput("experiment time", experimentTime);
    if (err) {
        experimentTime = 0.;
    }

    if (experimentTime > 0.) {
        egsInformation("EGS_RadionuclideSource: Experiment time [s]: %e\n",
                       experimentTime);
    }
    else {
        egsInformation("EGS_RadionuclideSource: Experiment time will not limit the simulation.\n");
    }

    // Get the active application
    app = EGS_Application::activeApplication();

    // Check for deprecated inputs
    string dummy;
    err = input->getInput("source type",dummy);
    int err2 = input->getInput("geometry",dummy);
    if (!err || !err2) {
        egsWarning("\nEGS_RadionuclideSource: Warning: Inputs for defining the radionuclide source as an isotropic or collimated source (e.g. 'source type') are deprecated. Please see the documentation - define the type of source separately and then refer to it using the new 'base source' input.\n");
    }

    // Import base source
    err = input->getInput("base source",sName);
    if (err) {
        egsFatal("\nEGS_RadionuclideSource: Error: Base source must be defined\n"
                 "using 'base source = some_name'\n");
    }
    baseSource = EGS_BaseSource::getSource(sName);
    if (!baseSource) {
        egsFatal("\nEGS_RadionuclideSource: Error: no source named %s"
                 " is defined\n",sName.c_str());
    }

    // Initialize emission type to signify nothing has happened yet
    emissionType = 99;

    // Finish
    setUp();
}

static char spec_msg1[] = "EGS_RadionuclideSource::createSpectrum:";

EGS_RadionuclideSpectrum *EGS_RadionuclideSource::createSpectrum(EGS_Input *input) {

    // Read inputs for the spectrum
    if (!input) {
        egsWarning("%s got null input?\n",spec_msg1);
        return 0;
    }
    EGS_Input *inp = input;
    bool delete_it = false;
    if (!input->isA("spectrum")) {
        inp = input->takeInputItem("spectrum");
        if (!inp) {
            egsWarning("%s no 'spectrum' input!\n",spec_msg1);
            return 0;
        }
        delete_it = true;
    }

    egsInformation("EGS_RadionuclideSource::createSpectrum: Initializing radionuclide spectrum...\n");

    string nuclide;
    int err = inp->getInput("nuclide",nuclide);
    if (err) {
        err = inp->getInput("isotope",nuclide);
        if (err) {
            err = inp->getInput("radionuclide",nuclide);
            if (err) {
                egsWarning("%s wrong/missing 'nuclide' input\n",spec_msg1);
                return 0;
            }
        }
    }

    EGS_Float relativeActivity;
    err = inp->getInput("relative activity",relativeActivity);
    if (err) {
        relativeActivity = 1;
    }

    // Determine whether to sample X-Rays and Auger electrons
    // using the ensdf data (options: eadl, ensdf or none)
    string tmp_relaxType, relaxType;
    err = inp->getInput("atomic relaxations", tmp_relaxType);
    if (!err) {
        relaxType = tmp_relaxType;
    }
    else {
        relaxType = "eadl";
    }
    if (inp->compare(relaxType,"ensdf")) {
        relaxType = "ensdf";
        egsInformation("EGS_RadionuclideSource::createSpectrum: Fluorescence and auger from the ensdf file will be used.\n");
    }
    else if (inp->compare(relaxType,"eadl")) {
        relaxType = "eadl";
        egsInformation("EGS_RadionuclideSource::createSpectrum: Fluorescence and auger from the ensdf file will be ignored. EADL relaxations will be used.\n");
    }
    else if (inp->compare(relaxType,"none") || inp->compare(relaxType,"off") || inp->compare(relaxType,"no")) {
        relaxType = "off";
        egsInformation("EGS_RadionuclideSource::createSpectrum: Fluorescence and auger from the ensdf file will be ignored. No relaxations following radionuclide disintegrations will be modelled.\n");
    }
    else {
        egsFatal("EGS_RadionuclideSource::createSpectrum: Error: Invalid selection for 'atomic relaxations'. Use 'eadl' (default), 'ensdf' or 'off'.\n");
    }

    // Determine whether to output beta energy spectra to files
    // (options: yes or no)
    string tmp_outputBetaSpectra, outputBetaSpectra;
    err = inp->getInput("output beta spectra", tmp_outputBetaSpectra);
    if (!err) {
        outputBetaSpectra = tmp_outputBetaSpectra;

        if (inp->compare(outputBetaSpectra,"yes")) {
            egsInformation("EGS_RadionuclideSource::createSpectrum: Beta energy spectra will be output to files.\n");
        }
        else if (inp->compare(outputBetaSpectra,"no")) {
            egsInformation("EGS_RadionuclideSource::createSpectrum: Beta energy spectra will not be output to files.\n");
        }
        else {
            egsFatal("EGS_RadionuclideSource::createSpectrum: Error: Invalid selection for 'output beta spectra'. Use 'no' (default) or 'yes'.\n");
        }
    }
    else {
        outputBetaSpectra = "no";
    }

    // Determine whether to score alpha energy locally or discard it
    // By default, the energy is discarded
    string tmp_alphaScoring;
    bool scoreAlphasLocally = false;
    err = inp->getInput("alpha scoring", tmp_alphaScoring);
    if (!err) {
        if (inp->compare(tmp_alphaScoring,"local")) {
            scoreAlphasLocally = true;
            egsInformation("EGS_RadionuclideSource::createSpectrum: Alpha particles will deposit energy locally, in the same region as creation.\n");
        }
        else if (inp->compare(tmp_alphaScoring,"discard")) {
            scoreAlphasLocally = false;
            egsInformation("EGS_RadionuclideSource::createSpectrum: Alpha particles will be discarded (no transport or energy deposition).\n");
        }
        else {
            egsFatal("EGS_RadionuclideSource::createSpectrum: Error: Invalid selection for 'alpha scoring'. Use 'discard' (default) or 'local'.\n");
        }
    }

    // Determine whether to score alpha energy locally or discard it
    // By default, the energy is discarded
    string tmp_allowMultiTransition;
    bool allowMultiTransition = false;
    err = inp->getInput("extra transition approximation", tmp_allowMultiTransition);
    if (!err) {
        if (inp->compare(tmp_allowMultiTransition,"on")) {
            allowMultiTransition = true;
            egsInformation("EGS_RadionuclideSource::createSpectrum: Extra transition approximation is on. If the intensity away from a level in a radionuclide daughter is larger than the intensity feeding the level (e.g. decays to that level), then additional transitions away from that level will be sampled. They will not be correlated with decays, but the spectrum will produce emission rates to match both the decay intensities and the internal transition intensities from the ensdf file.\n");
        }
        else if (inp->compare(tmp_allowMultiTransition,"off")) {
            allowMultiTransition = false;
            egsInformation("EGS_RadionuclideSource::createSpectrum: Extra transition approximation is off.\n");
        }
        else {
            egsFatal("EGS_RadionuclideSource::createSpectrum: Error: Invalid selection for 'extra transition approximation'. Use 'off' (default) or 'on'.\n");
        }
    }

    // For ensdf input, first check for the input argument
    string ensdf_file;
    err = inp->getInput("ensdf file",ensdf_file);

    // If not passed as input, find the ensdf file in the
    // directory $HEN_HOUSE/spectra/lnhb/ensdf/
    if (err) {

        EGS_Application *app = EGS_Application::activeApplication();
        if (app) {
            ensdf_file = egsJoinPath(app->getHenHouse(),"spectra");
            ensdf_file = egsJoinPath(ensdf_file.c_str(),"lnhb");
            ensdf_file = egsJoinPath(ensdf_file.c_str(),"ensdf");
        }
        else {
            char *hen_house = getenv("HEN_HOUSE");
            if (!hen_house) {

                egsWarning("EGS_RadionuclideSource::createSpectrum: "
                           "No active application and HEN_HOUSE not defined.\n"
                           "Assuming local directory for spectra\n");
                ensdf_file = "./";
            }
            else {
                ensdf_file = egsJoinPath(hen_house,"spectra");
                ensdf_file = egsJoinPath(ensdf_file.c_str(),"lnhb");
                ensdf_file = egsJoinPath(ensdf_file.c_str(),"ensdf");
            }
        }
        ensdf_file = egsJoinPath(ensdf_file.c_str(),nuclide.append(".txt"));
    }

    // Check that the ensdf file exists
    ifstream ensdf_fh;
    ensdf_fh.open(ensdf_file.c_str(),ios::in);
    if (!ensdf_fh.is_open()) {
        egsWarning("EGS_RadionuclideSource::createSpectrum: failed to open ensdf file %s"
                   " for reading\n",ensdf_file.c_str());
        return 0;
    }
    ensdf_fh.close();

    // Create the spectrum
    EGS_RadionuclideSpectrum *spec = new EGS_RadionuclideSpectrum(nuclide, ensdf_file, relativeActivity, relaxType, outputBetaSpectra, scoreAlphasLocally, allowMultiTransition);

    return spec;
}

EGS_I64 EGS_RadionuclideSource::getNextParticle(EGS_RandomGenerator *rndm, int
        &q, int &latch, EGS_Float &E, EGS_Float &wt, EGS_Vector &x, EGS_Vector
        &u) {

    unsigned int i = 0;
    if (decays.size() > 1 && disintegrationOccurred) {
        // Sample a uniform random number
        EGS_Float uRand = rndm->getUniform();

        // Sample which spectrum to use
        for (i=0; i<decays.size(); ++i) {
            if (uRand < decays[i]->getSpectrumWeight()) {
                break;
            }
        }
    }

    // Check if the emission time is within the experiment time limit
    if (experimentTime <= 0. || time < experimentTime) {
        // Keep this particle
    }
    else {
        // If the particle was emitted outside the
        // experiment time window, just set the energy to zero to discard
        E = 0;
        return ishower+1;
    }

    EGS_I64 ishowerOld = decays[i]->getShowerIndex();

    E = decays[i]->sample(rndm);

    EGS_I64 ishowerNew = decays[i]->getShowerIndex();
    if (ishowerNew > ishowerOld) {
        disintegrationOccurred = true;
        time = lastDisintTime + -log(1.-rndm->getUniform()) / activity * (ishowerNew - ishowerOld);

        lastDisintTime = time;
        ishower += (ishowerNew - ishowerOld);
    }
    else {
        disintegrationOccurred = false;
        // The time returned from the spectrum is just the time
        // since the last disintegration event, so this is only
        // non-zero for internal transitions. This is why the
        // times are added here - each transition occurs only after
        // the delay of the previous.
        time += decays[i]->getTime();
    }

    q = decays[i]->getCharge();
    int qTemp(q), latchTemp(latch);
    EGS_Float ETemp(E);
    baseSource->getNextParticle(rndm, qTemp, latchTemp, ETemp, wt, x, u);
    sCount++;
    latch = 0;

    if (disintegrationOccurred) {
        xOfDisintegration = x;
    }
    else {
        x = xOfDisintegration;
    }

    // Now that we have the position of the particle, we can
    // find the region and deposit any sub-threshold contributions locally
    // This includes edep from relaxations, and alpha particle energy
    EGS_Float edep = decays[i]->getEdep();
    if (edep > 0) {
        app->setEdep(edep);
        int ireg = app->isWhere(x);
        app->userScoring(3, ireg);
    }

    // Check if the charge is allowed
    if (q_allowAll || std::find(q_allowed.begin(), q_allowed.end(), q) != q_allowed.end()) {
        // Keep the particle
    }
    else {
        // Don't transport the particle
        E = 0;
    }

    // If the energy is zero, also set the weight to zero
    // If you don't do this, electrons will still be given their rest
    // mass energy in shower()
    if (E < epsilon) {
        wt = 0;
    }

    return ishower+1;
}

void EGS_RadionuclideSource::setUp() {
    otype = "EGS_RadionuclideSource";
    if (!isValid()) {
        description = "Invalid radionuclide source";
    }
    else {
        description = "Radionuclide production in base source ";
        description += sName;

        description += " with:";
        if (std::find(q_allowed.begin(), q_allowed.end(), -1) !=
                q_allowed.end()) {
            description += " electrons";
        }
        if (std::find(q_allowed.begin(), q_allowed.end(), 0) != q_allowed.end()) {
            description += " photons";
        }
        if (std::find(q_allowed.begin(), q_allowed.end(), 1) != q_allowed.end()) {
            description += " positrons";
        }
        if (std::find(q_allowed.begin(), q_allowed.end(), 1) != q_allowed.end()) {
            description += " alphas";
        }

        description += "\nBase source description:\n";
        description += baseSource->getSourceDescription();
    }
}

bool EGS_RadionuclideSource::storeState(ostream &data_out) const {
    for (unsigned int i=0; i<decays.size(); ++i) {
        if (!decays[i]->storeState(data_out)) {
            return false;
        }
    }
    egsStoreI64(data_out,count);
    egsStoreI64(data_out,sCount);
    baseSource->storeState(data_out);

    return true;
}

bool EGS_RadionuclideSource::addState(istream &data) {
    for (unsigned int i=0; i<decays.size(); ++i) {
        if (!decays[i]->setState(data)) {
            return false;
        }
        ishower += decays[i]->getShowerIndex();
    }
    EGS_I64 tmp_val;
    egsGetI64(data,tmp_val);
    count += tmp_val;
    egsGetI64(data,tmp_val);
    sCount += tmp_val;
    baseSource->addState(data);

    return true;
}

void EGS_RadionuclideSource::resetCounter() {
    for (unsigned int i=0; i<decays.size(); ++i) {
        decays[i]->resetCounter();
    }
    ishower = 0;
    count = 0;
    sCount = 0;
    baseSource->resetCounter();
}

bool EGS_RadionuclideSource::setState(istream &data) {
    for (unsigned int i=0; i<decays.size(); ++i) {
        if (!decays[i]->setState(data)) {
            return false;
        }
    }
    egsGetI64(data,count);
    egsGetI64(data,sCount);
    baseSource->setState(data);

    return true;
}

extern "C" {

    EGS_RADIONUCLIDE_SOURCE_EXPORT EGS_BaseSource *createSource(EGS_Input
            *input, EGS_ObjectFactory *f) {
        return
            createSourceTemplate<EGS_RadionuclideSource>(input,f,"radionuclide "
                    "source");
    }

}

EGS_RadionuclideSpectrum::EGS_RadionuclideSpectrum(const string nuclide, const string ensdf_file,
        const EGS_Float relativeActivity, const string relaxType, const string outputBetaSpectra, const bool scoreAlphasLocally, const bool allowMultiTransition) {

    // For now, hard-code verbose mode
    // 0 - minimal output
    // 1 - some output of ensdf data and normalized intensities
    // 2 - verbose output
    int verbose = 0;

    // Read in the data file for the nuclide
    // and build the decay structure
    decays = new EGS_Ensdf(nuclide, ensdf_file, relaxType, allowMultiTransition, verbose);

    // Normalize the emission and transition intensities
    decays->normalizeIntensities();

    // Get the beta energy spectra
    betaSpectra = new EGS_RadionuclideBetaSpectrum(decays, outputBetaSpectra);

    // Get the particle records from the decay scheme
    myBetas = decays->getBetaRecords();
    myAlphas = decays->getAlphaRecords();
    myGammas = decays->getGammaRecords();
    myMetastableGammas = decays->getMetastableGammaRecords();
    myUncorrelatedGammas = decays->getUncorrelatedGammaRecords();
    myLevels = decays->getLevelRecords();
    xrayIntensities = decays->getXRayIntensities();
    xrayEnergies = decays->getXRayEnergies();
    augerIntensities = decays->getAugerIntensities();
    augerEnergies = decays->getAugerEnergies();

    // Initialization
    currentLevel = 0;
    Emax = 0;
    currentTime = 0;
    ishower = -1; // Start with ishower -1 so first shower has index 0
    totalGammaEnergy = 0;
    relaxationType = relaxType;
    scoreAlphasLocal = scoreAlphasLocally;

    // Get the maximum energy for emissions
    for (vector<BetaRecordLeaf *>::iterator beta = myBetas.begin();
            beta != myBetas.end(); beta++) {

        double energy = (*beta)->getFinalEnergy();
        if (Emax < energy) {
            Emax = energy;
        }
    }
    for (vector<AlphaRecord *>::iterator alpha = myAlphas.begin();
            alpha != myAlphas.end(); alpha++) {

        double energy = (*alpha)->getFinalEnergy();
        if (Emax < energy) {
            Emax = energy;
        }
    }
    for (vector<GammaRecord *>::iterator gamma = myGammas.begin();
            gamma != myGammas.end(); gamma++) {

        double energy = (*gamma)->getDecayEnergy();
        if (Emax < energy) {
            Emax = energy;
        }
    }
    for (vector<GammaRecord *>::iterator gamma = myUncorrelatedGammas.begin();
            gamma != myUncorrelatedGammas.end(); gamma++) {

        double energy = (*gamma)->getDecayEnergy();
        if (Emax < energy) {
            Emax = energy;
        }
    }
    for (unsigned int i=0; i < xrayEnergies.size(); ++i) {
        numSampledXRay.push_back(0);
        if (Emax < xrayEnergies[i]) {
            Emax = xrayEnergies[i];
        }
    }
    for (unsigned int i=0; i < augerEnergies.size(); ++i) {
        numSampledAuger.push_back(0);
        if (Emax < augerEnergies[i]) {
            Emax = augerEnergies[i];
        }
    }

    // Set the weight of the spectrum
    spectrumWeight = relativeActivity;

    if (verbose) {
        egsInformation("EGS_RadionuclideSpectrum: Emax: %f\n",Emax);
        egsInformation("EGS_RadionuclideSpectrum: Relative activity: %f\n",relativeActivity);
    }

    // Set the application
    app = EGS_Application::activeApplication();
};


void EGS_RadionuclideSpectrum::printSampledEmissions() {

    egsInformation("\nSampled %s emissions:\n", decays->radionuclide.c_str());
    egsInformation("========================\n");

    if (ishower < 1) {
        egsWarning("EGS_RadionuclideSpectrum::printSampledEmissions: Warning: The number of disintegrations (tracked by `ishower`) is less than 1.\n");
        return;
    }

    egsInformation("Energy | Intensity per 100 decays (adjusted by %f)\n", decays->decayDiscrepancy);
    if (myBetas.size() > 0) {
        egsInformation("Beta records:\n");
    }
    for (vector<BetaRecordLeaf *>::iterator beta = myBetas.begin();
            beta != myBetas.end(); beta++) {

        egsInformation("%f %f\n", (*beta)->getFinalEnergy(),
                       ((EGS_Float)(*beta)->getNumSampled()/(ishower+1))*100);
    }
    if (myAlphas.size() > 0) {
        egsInformation("Alpha records:\n");
    }
    for (vector<AlphaRecord *>::iterator alpha = myAlphas.begin();
            alpha != myAlphas.end(); alpha++) {

        egsInformation("%f %f\n", (*alpha)->getFinalEnergy(),
                       ((EGS_Float)(*alpha)->getNumSampled()/(ishower+1))*100);
    }
    if (myGammas.size() > 0) {
        egsInformation("Gamma records (E,Igamma,Ice,Ipp):\n");
    }
    EGS_I64 totalNumSampled = 0;
    for (vector<GammaRecord *>::iterator gamma = myGammas.begin();
            gamma != myGammas.end(); gamma++) {

        totalNumSampled += (*gamma)->getGammaSampled();
        egsInformation("%f %f %.4e %.4e\n", (*gamma)->getDecayEnergy(),
                       ((EGS_Float)(*gamma)->getGammaSampled()/(ishower+1))*100,
                       ((EGS_Float)(*gamma)->getICSampled()/(ishower+1))*100,
                       ((EGS_Float)(*gamma)->getIPSampled()/(ishower+1))*100
                      );
    }
    if (myGammas.size() > 0) {
        if (totalNumSampled > 0) {
            egsInformation("Average gamma energy: %f\n",
                           totalGammaEnergy / totalNumSampled);
        }
        else {
            egsInformation("Zero gamma transitions occurred.\n");
        }
    }
    if (myUncorrelatedGammas.size() > 0) {
        egsInformation("Uncorrelated gamma records (E,Igamma,Ice,Ipp):\n");
    }
    for (vector<GammaRecord *>::iterator gamma = myUncorrelatedGammas.begin();
            gamma != myUncorrelatedGammas.end(); gamma++) {

        egsInformation("%f %f %.4e %.4e\n", (*gamma)->getDecayEnergy(),
                       ((EGS_Float)(*gamma)->getGammaSampled()/(ishower+1))*100,
                       ((EGS_Float)(*gamma)->getICSampled()/(ishower+1))*100,
                       ((EGS_Float)(*gamma)->getIPSampled()/(ishower+1))*100
                      );
    }
    if (xrayEnergies.size() > 0) {
        egsInformation("X-Ray records:\n");
    }
    for (unsigned int i=0; i < xrayEnergies.size(); ++i) {
        egsInformation("%f %f\n", xrayEnergies[i],
                       ((EGS_Float)numSampledXRay[i]/(ishower+1))*100);
    }
    if (augerEnergies.size() > 0) {
        egsInformation("Auger records:\n");
    }
    for (unsigned int i=0; i < augerEnergies.size(); ++i) {
        egsInformation("%f %f\n", augerEnergies[i],
                       ((EGS_Float)numSampledAuger[i]/(ishower+1))*100);
    }
    egsInformation("\n");
}


EGS_Float EGS_RadionuclideSpectrum::sample(EGS_RandomGenerator *rndm) {

    // The energy of the sampled particle
    EGS_Float E;
    // Local energy depositions
    edep = 0;
    // Time delay of this particle
    currentTime = 0;
    // The type of emission particle
    emissionType = 0;

    // Check for relaxation particles due to shell vacancies in the daughter
    // These are created from internal transitions or electron capture
    if (relaxParticles.size() > 0) {

        // Get the energy and charge of the last particle on the list
        EGS_RelaxationParticle p = relaxParticles.pop();
        E = p.E;
        currentQ = p.q;

        emissionType = 1;

        return E;
    }

    // Sample a uniform random number
    EGS_Float u = rndm->getUniform();

    // If the daughter is in an excited state
    // check for transitions
    if (currentLevel && currentLevel->levelCanDecay() && currentLevel->getEnergy() > epsilon) {

        for (vector<GammaRecord *>::iterator gamma = myGammas.begin();
                gamma != myGammas.end(); gamma++) {

            if ((*gamma)->getLevelRecord() == currentLevel) {

                if (u < (*gamma)->getTransitionIntensity()) {

                    // A gamma transition may either be a gamma emission
                    // or an internal conversion electron
                    EGS_Float u2 = 0;
                    if ((*gamma)->getGammaIntensity() < 1) {
                        u2 = rndm->getUniform();
                    }

                    // Sample how long
                    // it took for this transition to occur
                    // time = -halflife / ln(2) * log(1-u)
                    double hl = currentLevel->getHalfLife();
                    if (hl > 0) {
                        currentTime = -hl * log(1.-rndm->getUniform()) /
                                      0.693147180559945309417232121458176568075500134360255254120680009493393;
                    }

                    // Determine whether multiple gamma transitions occur
                    if (rndm->getUniform() < (*gamma)->getMultiTransitionProb()) {
                        multiTransitions.push_back(currentLevel);
                    }

                    // Update the level of the daughter
                    currentLevel = (*gamma)->getFinalLevel();

                    // If a gamma emission occurs
                    if (u2 < (*gamma)->getGammaIntensity()) {

                        (*gamma)->incrGammaSampled();

                        currentQ = (*gamma)->getCharge();

                        E = (*gamma)->getDecayEnergy();

                        totalGammaEnergy += E;

                        emissionType = 2;

                        return E;

                    }
                    else if (u2 < (*gamma)->getICIntensity()) {
                        (*gamma)->incrICSampled();
                        currentQ = -1;
                        emissionType = 3;

                        if ((*gamma)->icIntensity.size()) {

                            // Determine which shell the conversion electron
                            // comes from. This will create a shell vacancy
                            EGS_Float u3 = rndm->getUniform();

                            for (unsigned int i=0; i<(*gamma)->icIntensity.size(); ++i) {
                                if (u3 < (*gamma)->icIntensity[i]) {

                                    E = (*gamma)->getDecayEnergy() - (*gamma)->getBindingEnergy(i);

                                    //                                     egsInformation("test %d %f %f %f\n",i,(*gamma)->getDecayEnergy(),decays->getRelaxations()->getBindingEnergy(decays->Z,i),E);

                                    // Add relaxation particles to the source stack
                                    if (relaxationType == "eadl") {

                                        // Generate relaxation particles for a
                                        // shell vacancy i
                                        (*gamma)->relax(i,app->getEcut()-app->getRM(),app->getPcut(),rndm,edep,relaxParticles);
                                    }

                                    // Return the conversion electron
                                    return E;
                                }
                            }
                        }
                        return 0;
                    }
                    else {
                        (*gamma)->incrIPSampled();
                        emissionType = 13;

                        // Internal pair production results in a positron
                        // and electron pair

                        //TODO: This is left for future work, we need to
                        // determine the energies of the electron/positron
                        // pair (sample uniformly?) and then determine the
                        // corresponding directions. It might be best to do
                        // this in the source instead of the spectrum.

                        currentQ = 1;
                        return 0;
                    }
                }
            }
        }

        currentLevel = 0;
        return 0;
    }

    // If we have determined that multiple transitions will occur from some
    // levels, here we set the current level to an excited state, and return.
    // The radionuclide source will then sample again using the excited level.
    if (multiTransitions.size() > 0) {
        currentLevel = multiTransitions.back();
        multiTransitions.pop_back();
        return 0;
    }

    // ============================
    // Sample which decay occurs
    // ============================
    currentTime = 0;

    // Beta-, beta+ and electron capture
    for (vector<BetaRecordLeaf *>::iterator beta = myBetas.begin();
            beta != myBetas.end(); beta++) {
        if (u < (*beta)->getBetaIntensity()) {

            // Increment the shower number
            ishower++;

            // Increment the counter of betas and get the charge
            (*beta)->incrNumSampled();
            currentQ = (*beta)->getCharge();

            // Set the energy level of the daughter
            currentLevel = (*beta)->getLevelRecord();

            // For beta+ records we decide between
            // branches for beta+ or electron capture
            if (currentQ == 1) {
                // For positron emission, continue as usual
                if ((*beta)->getPositronIntensity() > epsilon && rndm->getUniform() < (*beta)->getPositronIntensity()) {

                }
                else {

                    if (relaxationType == "eadl" && (*beta)->ecShellIntensity.size()) {
                        // Determine which shell the electron capture
                        // occurs in. This will create a shell vacancy
                        EGS_Float u3 = rndm->getUniform();

                        for (unsigned int i=0; i<(*beta)->ecShellIntensity.size(); ++i) {
                            if (u3 < (*beta)->ecShellIntensity[i]) {

                                // Generate relaxation particles for a
                                // shell vacancy i
                                (*beta)->relax(i,app->getEcut()-app->getRM(),app->getPcut(),rndm,edep,relaxParticles);

                                emissionType = 4;

                                return 0;
                            }
                        }
                    }

                    // For electron capture, there is no emitted particle
                    // (only a neutrino)
                    // so we return a 0 energy particle
                    emissionType = 4;
                    return 0;
                }
                emissionType = 5;
            }
            else {
                emissionType = 6;
            }

            // Sample the energy from the spectrum alias table
            E = (*beta)->getSpectrum()->sample(rndm);

            return E;
        }
    }

    // Alphas
    for (vector<AlphaRecord *>::iterator alpha = myAlphas.begin();
            alpha != myAlphas.end(); alpha++) {
        if (u < (*alpha)->getAlphaIntensity()) {

            // Increment the shower number
            ishower++;

            // Increment the counter of alphas and get the charge
            (*alpha)->incrNumSampled();
            currentQ = (*alpha)->getCharge();

            // Set the energy level of the daughter
            currentLevel = (*alpha)->getLevelRecord();

            // Score alpha energy depositions locally,
            // because alpha transport is not modeled in EGSnrc.
            // This is an approximation!
            if (scoreAlphasLocal) {
                edep += (*alpha)->getFinalEnergy();
            }

            emissionType = 7;

            // For alphas we simulate a disintegration but the
            // transport will not be performed so return 0
            return 0;
        }
    }

    // Metastable "decays" that will result in internal transitions
    for (vector<GammaRecord *>::iterator gamma = myMetastableGammas.begin();
            gamma != myMetastableGammas.end(); gamma++) {
        if (u < (*gamma)->getTransitionIntensity()) {

            // Increment the shower number
            ishower++;

            // Set the energy level of the daughter as though a
            // disintegration just occurred
            currentLevel = (*gamma)->getLevelRecord();

            emissionType = 8;

            // No particle returned
            return 0;
        }
    }

    // Uncorrelated internal transitions
    for (vector<GammaRecord *>::iterator gamma = myUncorrelatedGammas.begin();
            gamma != myUncorrelatedGammas.end(); gamma++) {
        if (u < (*gamma)->getTransitionIntensity()) {

            // A gamma transition may either be a gamma emission
            // or an internal conversion electron
            EGS_Float u2 = 0;
            if ((*gamma)->getGammaIntensity() < 1) {
                u2 = rndm->getUniform();
            }

            // If a gamma emission occurs
            if (u2 < (*gamma)->getGammaIntensity()) {

                (*gamma)->incrGammaSampled();

                currentQ = (*gamma)->getCharge();

                E = (*gamma)->getDecayEnergy();

                totalGammaEnergy += E;

                emissionType = 11;

                return E;

            }
            else if (u2 < (*gamma)->getICIntensity()) {
                (*gamma)->incrICSampled();
                currentQ = -1;
                emissionType = 12;

                if ((*gamma)->icIntensity.size()) {

                    // Determine which shell the conversion electron
                    // comes from. This will create a shell vacancy
                    EGS_Float u3 = rndm->getUniform();

                    for (unsigned int i=0; i<(*gamma)->icIntensity.size(); ++i) {
                        if (u3 < (*gamma)->icIntensity[i]) {

                            E = (*gamma)->getDecayEnergy() - (*gamma)->getBindingEnergy(i);

                            // Add relaxation particles to the source stack
                            if (relaxationType == "eadl") {

                                // Generate relaxation particles for a
                                // shell vacancy i
                                (*gamma)->relax(i,app->getEcut()-app->getRM(),app->getPcut(),rndm,edep,relaxParticles);
                            }

                            // Return the conversion electron
                            return E;
                        }
                    }
                }
                return 0;
            }
            else {
                (*gamma)->incrIPSampled();
                emissionType = 14;

                // Internal pair production results in a positron
                // and electron pair

                //TODO: This is left for future work, we need to
                // determine the energies of the electron/positron
                // pair (sample uniformly?) and then determine the
                // corresponding directions. It might be best to do
                // this in the source instead of the spectrum.

                currentQ = 1;
                return 0;
            }
        }
    }

    // XRays from the ensdf
    for (unsigned int i=0; i < xrayIntensities.size(); ++i) {
        if (u < xrayIntensities[i]) {

            numSampledXRay[i]++;
            currentQ = 0;

            E = xrayEnergies[i];

            emissionType = 9;

            return E;
        }
    }

    // Auger electrons from the ensdf
    for (unsigned int i=0; i < augerIntensities.size(); ++i) {
        if (u < augerIntensities[i]) {

            numSampledAuger[i]++;
            currentQ = -1;

            E = augerEnergies[i];

            emissionType = 10;

            return E;
        }
    }

    // If we get here, fission occurs
    // Count it as a disintegration and return 0
    ishower++;
    return 0;
}
