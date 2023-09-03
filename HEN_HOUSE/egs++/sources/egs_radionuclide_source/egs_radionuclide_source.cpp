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

        decays.push_back(static_cast<EGS_RadionuclideSpectrum *>(EGS_BaseSpectrum::createSpectrum(input)));

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
        return ++count;
    }

    EGS_I64 ishowerOld = decays[i]->getShowerIndex();

    E = decays[i]->sampleEnergy(rndm);

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

    return ++count;
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
