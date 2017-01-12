/*
###############################################################################
#
#  EGSnrc egs++ ensdf
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
#  Contributors:
#
###############################################################################
*/


/*! \file egs_ensdf.cpp
 *  \brief The ensdf implementation
 *  \RT
 *
 */

#include "egs_ensdf.h"

EGS_Ensdf::EGS_Ensdf(const string isotope, const string ensdf_filename,
                     const string useFluor, int verbosity) {

    verbose = verbosity;
    useFluorescence = useFluor;

    if (ensdf_file.is_open()) {
        ensdf_file.close();
    }

    radionuclide = isotope.substr(0, isotope.find_last_of("."));

    // The parent element
    //string element = radionuclide.substr(0, radionuclide.find("-"));

    egsInformation("EGS_Ensdf::EGS_Ensdf: Isotope: "
                   "%s\n",isotope.c_str());
    egsInformation("EGS_Ensdf::EGS_Ensdf: Now loading ensdf file: "
                   "\"%s\"\n",ensdf_filename.c_str());

    ensdf_file.open(ensdf_filename.c_str(),ios::in);
    if (!ensdf_file.is_open()) {
        egsWarning("\nEGS_Ensdf::EGS_Ensdf: failed to open ensdf file %s"
                   " for reading\n\n",ensdf_filename.c_str());
        return;
    }

    string line;
    vector<string> ensdf;
    while (getline(ensdf_file, line)) {
        ensdf.push_back(line);
    }

    if (ensdf_file.is_open()) {
        ensdf_file.close();
    }

    // Parse the ensdf data
    parseEnsdf(ensdf);
}

EGS_Ensdf::~EGS_Ensdf() {
    if (ensdf_file.is_open()) {
        ensdf_file.close();
    }

    for (vector<ParentRecord * >::iterator it = myParentRecords.begin();
            it!=myParentRecords.end(); it++) {
        delete *it;
        *it=0;
    }
    myParentRecords.clear();
    for (vector<NormalizationRecord * >::iterator it =
                myNormalizationRecords.begin();
            it!=myNormalizationRecords.end(); it++) {
        delete *it;
        *it=0;
    }
    myNormalizationRecords.clear();
    for (vector<LevelRecord * >::iterator it =
                myLevelRecords.begin();
            it!=myLevelRecords.end(); it++) {
        delete *it;
        *it=0;
    }
    myLevelRecords.clear();
    for (vector<BetaMinusRecord * >::iterator it =
                myBetaMinusRecords.begin();
            it!=myBetaMinusRecords.end(); it++) {
        delete *it;
        *it=0;
    }
    myBetaMinusRecords.clear();
    for (vector<BetaPlusRecord * >::iterator it =
                myBetaPlusRecords.begin();
            it!=myBetaPlusRecords.end(); it++) {
        delete *it;
        *it=0;
    }
    myBetaPlusRecords.clear();
    for (vector<GammaRecord * >::iterator it =
                myGammaRecords.begin();
            it!=myGammaRecords.end(); it++) {
        delete *it;
        *it=0;
    }
    myGammaRecords.clear();
    for (vector<AlphaRecord * >::iterator it =
                myAlphaRecords.begin();
            it!=myAlphaRecords.end(); it++) {
        delete *it;
        *it=0;
    }
    myAlphaRecords.clear();
}

string egsRemoveWhite(string myString) {
    string result = "";

    for (unsigned int i = 0; i<myString.size(); i++) {
        if (!(myString[i]==' ' || myString[i]=='\n' || myString[i]=='\t')) {
            result += myString[i];
        }
    }

    return result;
}

string egsTrimString(string myString) {
    int start = -1;
    int end = myString.size();
    while (myString[++start]==' ');
    while (myString[--end]==' ');
    return myString.substr(start,end-start+1);
}

// Parse an ensdf file to create a decay structure
void EGS_Ensdf::parseEnsdf(vector<string> ensdf) {
    /* IDs of recordStack
     * 0 Identification (not used)
     * 1 History (not used)
     * 2 Q-value(not used)
     * 3 Cross-Reference (not used)
     * 4 Comment
     * 5 Parent
     * 6 Normalization
     * 7 Level
     * 8 Beta-
     * 9 EC + Beta+
     * 10 Alpha
     * 11 Delayed Particle (not used)
     * 12 Gamma
     * 13 Reference (not used)
     * */
    for (int i = 0; i < 14; i++) {
        recordStack.push_back(vector<string>());
    }

    // Loop over each line
    // When we recognize a line as containing an important record,
    // add it to the recordStack
    // Any time we get to a new record (line[5]==' '), call buildRecords()
    for (vector<string>::iterator it = ensdf.begin(); it!=ensdf.end(); it++) {

        string line = *it;

        if (line.length() < 3) {
            continue;
        }

        if (verbose) {
            egsInformation("EGS_Ensdf::parseEnsdf: %s\n", line.c_str());
        }

        if (line[6]==' ' && line[7]==' ' && line[8]==' ') {
            // Identification

        }
        else if (line[6]==' ' && line[7]=='H' && line[8]==' ') {
            // History

        }
        else if (line[6]== ' ' && line[7]=='Q' && line[8]==' ') {
            // Q-value

        }
        else if (line[6]==' ' && line[7]=='X') {
            // Cross-Reference

        }
        else if ((line[6]=='C' || line[6]=='D' || line[6]=='T' ||
                  line[6]=='c' || line[6]=='d' || line[6]=='t')) {
            // Comment
            if (line[5]==' ') {
                buildRecords();
            }
            recordStack[4].push_back(line);

        }
        else if (line[6]==' ' && line[7]=='P') {
            //Parent
            if (line[5]==' ') {
                buildRecords();
            }
            recordStack[5].push_back(line);

        }
        else if (line[6]==' ' && line[7]=='N') {
            // Normalization
            if (line[5]==' ') {
                buildRecords();
            }
            recordStack[6].push_back(line);

        }
        if (line[6]==' ' && line[7]=='L' && line[8]== ' ') {
            // Level
            if (line[5]==' ') {
                buildRecords();
            }
            recordStack[7].push_back(line);

        }
        else if (line[6]==' ' && line[7]=='B' && line[8]==' ') {
            // Beta-
            if (line[5]==' ') {
                buildRecords();
            }
            recordStack[8].push_back(line);

        }
        else if (line[6]==' ' && line[7]=='E' && line[8]==' ') {
            // Beta+ and Electron Capture
            if (line[5]==' ') {
                buildRecords();
            }
            recordStack[9].push_back(line);

        }
        else if (line[6]==' ' && line[7]=='A' && line[8]==' ') {
            // Alpha
            if (line[5]==' ') {
                buildRecords();
            }
            recordStack[10].push_back(line);

        }
        else if (line[6]==' ' && (line[7]=='D' || line[7]==' ') &&
                 (line[8]=='N' || line[8]=='P' || line[8]=='A')) {
            // Delayed Particle
            if (line[5]==' ') {
                buildRecords();
            }
            recordStack[11].push_back(line);

        }
        else if (line[6]==' ' && line[7]=='G' && line[8]==' ') {
            // Gamma
            if (line[5]==' ') {
                buildRecords();
            }
            recordStack[12].push_back(line);
        }
    }

    // Build the records into objects
    if (!recordStack.empty()) {
        buildRecords();
    }

    // Combine the beta- and beta+ records together
    for (vector<BetaMinusRecord * >::iterator it = myBetaMinusRecords.begin();
            it!=myBetaMinusRecords.end(); it++) {

        myBetaRecords.push_back(*it);
    }

    for (vector<BetaPlusRecord * >::iterator it = myBetaPlusRecords.begin();
            it!=myBetaPlusRecords.end(); it++) {

        myBetaRecords.push_back(*it);
    }

    // For each parent record, search through all disintegration types
    // (betas, alphas) to see if any exist
    // If no disintegrations exist for the parent, but we do have internal
    // transition (IT) gammas, then we must have a metastable radionuclide
    // In this case, add the gammas to the myMetastableGammaRecords vector
    if (verbose) {
        egsInformation("EGS_Ensdf::parseEnsdf: Checking for metastable radionuclides...\n");
    }
    for (vector<ParentRecord * >::iterator parent = myParentRecords.begin();
            parent!=myParentRecords.end(); parent++) {

        bool gotDisint = false;
        for (vector<BetaRecordLeaf *>::iterator beta = myBetaRecords.begin();
                beta != myBetaRecords.end(); beta++) {

            if ((*beta)->getParentRecord() == *parent) {
                gotDisint = true;
                break;
            }
        }
        if (!gotDisint) {
            for (vector<AlphaRecord *>::iterator alpha = myAlphaRecords.begin();
                    alpha != myAlphaRecords.end(); alpha++) {

                if ((*alpha)->getParentRecord() == *parent) {
                    gotDisint = true;
                    break;
                }
            }
        }

        if (!gotDisint) {
            for (vector<GammaRecord *>::iterator gamma = myGammaRecords.begin();
                    gamma < myGammaRecords.end();) {

                if ((*gamma)->getParentRecord() == *parent) {
                    myMetastableGammaRecords.push_back(*gamma);
                    myGammaRecords.erase(gamma);
                }
                else {
                    gamma++;
                }
            }

            if (verbose && myMetastableGammaRecords.size() > 0) {
                egsInformation("EGS_Ensdf::parseEnsdf: Metastable isotope "
                               "detected.\n");
            }
        }
    }
    if (verbose && myMetastableGammaRecords.size() < 1) {
        egsInformation("EGS_Ensdf::parseEnsdf: No metastable isotopes "
                       "detected.\n");
    }

    // Get X-ray and auger emissions from comments
    if (useFluorescence == "yes") {
        if (verbose) {
            egsInformation("EGS_Ensdf::parseEnsdf: Checking for x-rays and Auger...\n");
        }

        getEmissionsFromComments();

        if (verbose > 1) {
            egsInformation("EGS_Ensdf::parseEnsdf: Done checking for x-rays and Auger.\n");
        }
    }

    // Get rid of very low emission probability particles
    double minimumIntensity = 1e-6;
    for (vector<BetaRecordLeaf *>::iterator beta = myBetaRecords.begin();
            beta != myBetaRecords.end();) {
        if ((*beta)->getBetaIntensity() <= minimumIntensity) {
            if (verbose) {
                egsInformation("EGS_Ensdf::parseEnsdf: Removing beta due to small intensity (%.1e < %.1e)\n",(*beta)->getBetaIntensity(),minimumIntensity);
            }
            myBetaRecords.erase(beta);
        }
        else {
            beta++;
        }
    }
    for (vector<AlphaRecord *>::iterator alpha = myAlphaRecords.begin();
            alpha != myAlphaRecords.end();) {
        if ((*alpha)->getAlphaIntensity() <= minimumIntensity) {
            if (verbose) {
                egsInformation("EGS_Ensdf::parseEnsdf: Removing alpha due to small intensity (%.1e < %.1e)\n",(*alpha)->getAlphaIntensity(),minimumIntensity);
            }
            myAlphaRecords.erase(alpha);
        }
        else {
            alpha++;
        }
    }

    // Search through the gamma records for any with unknown levels
    // or with low emission probability
    for (vector<GammaRecord * >::iterator it = myGammaRecords.begin();
            it!=myGammaRecords.end();) {

        if ((*it)->getTransitionIntensity() <= minimumIntensity) {
            if (verbose) {
                egsInformation("EGS_Ensdf::parseEnsdf: Removing gamma due to small intensity (%.1e < %.1e)\n",(*it)->getTransitionIntensity(),minimumIntensity);
            }
            // Throw away gammas with low probability
            // Erase the gamma record object
            myGammaRecords.erase(it);

        }
        else if ((*it)->getLevelRecord()->getEnergy() < 1e-10) {
            // Some gamma may be emitted but the energy level is not known
            // This is reported in the lnhb data as decays from the -1 level
            // Since we cannot correlate the emission with a change of energy
            // states of the daughter, we will treat this gamma as an xray
            // The halflife will be ignored

            egsWarning("EGS_Ensdf::parseEnsdf: Warning: Switching gamma with unknown decay level to X-Ray (these emissions will still occur, but uncorrelated with disintegrations).\n");

            xrayEnergies.push_back((*it)->getDecayEnergy());
            xrayIntensities.push_back((*it)->getTransitionIntensity());

            egsInformation("EGS_Ensdf::parseEnsdf: Gamma converted to X-Ray (E,I): %f %f\n", xrayEnergies.back(), xrayIntensities.back());

            // Erase the gamma record object
            myGammaRecords.erase(it);
        }
        else {
            ++it;
        }
    }

    // Check for instances where the intensity of disintegrations that
    // lead towards a particular excited daughter level is less than
    // the intensities of gamma transitions from it.
    // In these cases, there are some gamma transitions that are
    // uncorrelated with the disintegrations.
    // To account for this, we convert a fraction of those gamma transitions
    // into X-Rays.
    if (verbose) {
        egsInformation("EGS_Ensdf::parseEnsdf: Comparing the cumulative disintegration intensity of each level with the gamma transition intensities... \n");
    }
    unsigned int j = 0;
    vector<double> totalLevelIntensity;
    totalLevelIntensity.resize(myLevelRecords.size());
    for (vector<LevelRecord * >::iterator it = myLevelRecords.begin();
            it!=myLevelRecords.end(); it++) {

        double disintIntensity = (*it)->getDisintegrationIntensity();

        totalLevelIntensity[j] = 0;
        for (vector<GammaRecord *>::iterator gamma = myGammaRecords.begin();
                gamma != myGammaRecords.end(); gamma++) {

            if ((*gamma)->getLevelRecord() == (*it)) {
                totalLevelIntensity[j] += (*gamma)->getTransitionIntensity();
            }
        }

        if (totalLevelIntensity[j] > disintIntensity) {
            for (vector<GammaRecord *>::iterator gamma = myGammaRecords.begin();
                    gamma != myGammaRecords.end(); gamma++) {

                if ((*gamma)->getLevelRecord() == (*it)) {
                    // Add a faction of these gammas to the X-Rays since
                    // they are uncorrelated with disintegrations
                    xrayEnergies.push_back((*gamma)->getDecayEnergy());
                    xrayIntensities.push_back(
                        (*gamma)->getTransitionIntensity() * (1. - disintIntensity/totalLevelIntensity[j])
                    );

                    // Reduce the transition intensities
                    // Now they will add to 1
                    (*gamma)->setTransitionIntensity(
                        (*gamma)->getTransitionIntensity() * disintIntensity/totalLevelIntensity[j]
                    );
                }
            }
        }

        ++j;
    }

    for (unsigned int i=0; i < xrayEnergies.size(); ++i) {
        if (verbose) {
            egsInformation("EGS_Ensdf::parseEnsdf: XRays (E,I): %f %f\n",
                           xrayEnergies[i], xrayIntensities[i]);
        }
    }
    for (unsigned int i=0; i < augerEnergies.size(); ++i) {
        if (verbose) {
            egsInformation("EGS_Ensdf::parseEnsdf: Auger (E,I): %f %f\n",
                           augerEnergies[i], augerIntensities[i]);
        }
    }
}

// Create record objects from the arrays
void EGS_Ensdf::buildRecords() {
    ParentRecord *lastParent = 0;
    if (!myParentRecords.empty()) {
        lastParent = myParentRecords.back();
    }
    NormalizationRecord *lastNormalization = 0;
    if (!myNormalizationRecords.empty()) {
        lastNormalization = myNormalizationRecords.back();
    }
    LevelRecord *lastLevel;
    if (!myLevelRecords.empty()) {
        if (!previousParent || previousParent == lastParent) {
            lastLevel = myLevelRecords.back();
        }
        else {
            lastLevel = new LevelRecord();
        }
    }
    else {
        lastLevel = new LevelRecord();
    }

    for (int i = 0; i < recordStack.size(); i++) {
        if (!recordStack[i].empty() && recordStack[i].front().length() > 5) {
            if (i==0) {

            }
            else if (i==1) {

            }
            else if (i==2) {

            }
            else if (i==3) {

            }
            else if (i==4) {
                myCommentRecords.push_back(new CommentRecord(recordStack[i]));
            }
            else if (i==5) {
                myParentRecords.push_back(new ParentRecord(recordStack[i]));
            }
            else if (i==6) {
                myNormalizationRecords.push_back(new
                                                 NormalizationRecord(recordStack[i], lastParent));
            }
            else if (i==7) {
                myLevelRecords.push_back(new LevelRecord(recordStack[i]));
                previousParent = lastParent;
            }
            else if (i==8) {
                myBetaMinusRecords.push_back(new
                                             BetaMinusRecord(recordStack[i], lastParent,
                                                     lastNormalization, lastLevel));
            }
            else if (i==9) {
                myBetaPlusRecords.push_back(new
                                            BetaPlusRecord(recordStack[i], lastParent,
                                                    lastNormalization, lastLevel));
            }
            else if (i==10) {
                myAlphaRecords.push_back(new
                                         AlphaRecord(recordStack[i], lastParent,
                                                     lastNormalization, lastLevel));
            }
            else if (i==11) {
                egsWarning("EGS_Ensdf::buildRecords: Warning: Delayed particle not "
                           "supported! Further development required.\n");
            }
            else if (i==12) {
                myGammaRecords.push_back(new
                                         GammaRecord(recordStack[i], lastParent,
                                                     lastNormalization, lastLevel));
            }

            recordStack[i].clear();
        }
    }
}

// Normalize intensities for alpha, beta, gamma objects
void EGS_Ensdf::normalizeIntensities() {
    if (verbose) {
        egsInformation("EGS_Ensdf::normalizeIntensities: Normalizing the "
                       "emission intensities to allow for spectrum sampling "
                       "routines.\n");
    }

    // Add up the beta, alpha, xray and auger decay intensities
    double totalDecayIntensity = 0;
    double lastIntensity = 0;

    for (vector<BetaRecordLeaf *>::iterator beta = myBetaRecords.begin();
            beta != myBetaRecords.end(); beta++) {

        if (verbose > 1) {
            egsInformation("EGS_Ensdf::normalizeIntensities: Beta (E,I): %f %f\n",
                           (*beta)->getFinalEnergy(), (*beta)->getBetaIntensity());
        }

        totalDecayIntensity += (*beta)->getBetaIntensity();
    }
    for (vector<AlphaRecord *>::iterator alpha = myAlphaRecords.begin();
            alpha != myAlphaRecords.end(); alpha++) {

        if (verbose > 1) {
            egsInformation("EGS_Ensdf::normalizeIntensities: Alpha (E,I): %f %f\n",
                           (*alpha)->getFinalEnergy(), (*alpha)->getAlphaIntensity());
        }

        totalDecayIntensity += (*alpha)->getAlphaIntensity();
    }
    double totalMetastableGammaIntensity = 0;
    for (vector<GammaRecord *>::iterator gamma = myMetastableGammaRecords.begin();
            gamma != myMetastableGammaRecords.end(); gamma++) {

        if (verbose > 1) {
            egsInformation("EGS_Ensdf::normalizeIntensities: MetastableGamma (E,I): %f %f\n",
                           (*gamma)->getDecayEnergy(),
                           (*gamma)->getTransitionIntensity());
        }

        totalMetastableGammaIntensity += (*gamma)->getTransitionIntensity();
        totalDecayIntensity += (*gamma)->getTransitionIntensity();
    }
    if (totalMetastableGammaIntensity > 0 && totalMetastableGammaIntensity < 100. - 1e-10) {
        double metastableFailIntensity = 100. - totalMetastableGammaIntensity;
        totalDecayIntensity += metastableFailIntensity;

        if (verbose) {
            egsInformation("EGS_Ensdf::normalizeIntensities: MetastableGamma adds to less than 100%%. Fail chance (per 100 disintegrations): %f\n", metastableFailIntensity);
        }

        // Push a copy of the last metastable gamma onto the vector
        myMetastableGammaRecords.push_back(new GammaRecord(myMetastableGammaRecords.back()));

        // Edit that copy so that it has zero energy and the right
        // intensity corresponding to the chance of no internal transition
        myMetastableGammaRecords.back()->setTransitionIntensity(metastableFailIntensity);
    }
    for (unsigned int i=0; i < xrayIntensities.size(); ++i) {
        if (verbose > 1) {
            egsInformation("EGS_Ensdf::normalizeIntensities: XRay (E,I): %f %f\n",
                           xrayEnergies[i], xrayIntensities[i]);
        }

        totalDecayIntensity += xrayIntensities[i];
    }
    for (unsigned int i=0; i < augerIntensities.size(); ++i) {
        if (verbose > 1) {
            egsInformation("EGS_Ensdf::normalizeIntensities: Auger (E,I): %f %f\n",
                           augerEnergies[i], augerIntensities[i]);
        }

        totalDecayIntensity += augerIntensities[i];
    }

    if (verbose) {
        egsInformation("EGS_Ensdf::normalizeIntensities: totalDecayIntensity: "
                       "%f\n",totalDecayIntensity);
        egsInformation("EGS_Ensdf::normalizeIntensities: "
                       "Calculating renormalized intensities...\n");
    }

    // Normalize beta emission intensities
    for (vector<BetaRecordLeaf *>::iterator beta = myBetaRecords.begin();
            beta != myBetaRecords.end(); beta++) {

        (*beta)->setBetaIntensity(
            (*beta)->getBetaIntensity() / totalDecayIntensity);

        if ((beta - myBetaRecords.begin()) > 0) {
            (*beta)->setBetaIntensity(
                (*beta)->getBetaIntensity() + (*(beta-1))->getBetaIntensity());
        }
        lastIntensity = (*beta)->getBetaIntensity();

        if (verbose) {
            egsInformation("EGS_Ensdf::normalizeIntensities: Beta (E,I): %f %f\n",
                           (*beta)->getFinalEnergy(), (*beta)->getBetaIntensity());
        }
    }

    // Normalize alpha emission intensities
    for (vector<AlphaRecord *>::iterator alpha = myAlphaRecords.begin();
            alpha != myAlphaRecords.end(); alpha++) {

        (*alpha)->setAlphaIntensity(
            (*alpha)->getAlphaIntensity() / totalDecayIntensity);

        if ((alpha - myAlphaRecords.begin()) == 0 && lastIntensity > 1e-10) {
            (*alpha)->setAlphaIntensity(
                (*alpha)->getAlphaIntensity() + lastIntensity);
        }
        else if ((alpha - myAlphaRecords.begin()) > 0) {
            (*alpha)->setAlphaIntensity(
                (*alpha)->getAlphaIntensity() +
                (*(alpha-1))->getAlphaIntensity());
        }
        lastIntensity = (*alpha)->getAlphaIntensity();

        if (verbose) {
            egsInformation("EGS_Ensdf::normalizeIntensities: Alpha (E,I): %f %f\n",
                           (*alpha)->getFinalEnergy(), (*alpha)->getAlphaIntensity());
        }
    }

    // Normalize metastable gamma transition intensities
    for (vector<GammaRecord *>::iterator gamma = myMetastableGammaRecords.begin();
            gamma != myMetastableGammaRecords.end(); gamma++) {

        (*gamma)->setTransitionIntensity(
            (*gamma)->getTransitionIntensity() / totalDecayIntensity);

        if ((gamma - myMetastableGammaRecords.begin()) == 0 && lastIntensity > 1e-10) {
            (*gamma)->setTransitionIntensity(
                (*gamma)->getTransitionIntensity() + lastIntensity);
        }
        else if ((gamma - myMetastableGammaRecords.begin()) > 0) {
            (*gamma)->setTransitionIntensity(
                (*gamma)->getTransitionIntensity() +
                (*(gamma-1))->getTransitionIntensity());
        }
        lastIntensity = (*gamma)->getTransitionIntensity();

        if (verbose) {
            egsInformation("EGS_Ensdf::normalizeIntensities: MetastableGamma (E,I): %f %f\n",
                           (*gamma)->getDecayEnergy(), (*gamma)->getTransitionIntensity());
        }
    }

    // Normalize XRay emission intensities
    for (unsigned int i=0; i < xrayIntensities.size(); ++i) {

        xrayIntensities[i] /= totalDecayIntensity;

        if (i==0 && lastIntensity > 1e-10) {
            xrayIntensities[i] += lastIntensity;
        }
        else if (i > 0) {
            xrayIntensities[i] += xrayIntensities[i-1];
        }
        lastIntensity = xrayIntensities[i];

        if (verbose) {
            egsInformation("EGS_Ensdf::normalizeIntensities: XRay (E,I): %f %f\n",
                           xrayEnergies[i], xrayIntensities[i]);
        }
    }

    // Normalize auger emission intensities
    for (unsigned int i=0; i < augerIntensities.size(); ++i) {

        augerIntensities[i] /= totalDecayIntensity;

        if (i==0 && lastIntensity > 1e-10) {
            augerIntensities[i] += lastIntensity;
        }
        else if (i > 0) {
            augerIntensities[i] += augerIntensities[i-1];
        }
        lastIntensity = augerIntensities[i];

        if (verbose) {
            egsInformation("EGS_Ensdf::normalizeIntensities: Auger (E,I): %f %f\n",
                           augerEnergies[i], augerIntensities[i]);
        }
    }

    // Determine the final level that the gammas decay towards
    // We have to use the gamma decay energy to guess at the resulting
    // energy state of the radionuclide
    for (vector<GammaRecord *>::iterator gamma = myGammaRecords.begin();
            gamma != myGammaRecords.end(); gamma++) {

        double energy = (*gamma)->getDecayEnergy();
        double guessedLevelEnergy =
            ((*gamma)->getLevelRecord()->getEnergy() - energy);

        if (verbose) {
            egsInformation("EGS_Ensdf::normalizeIntensities: Gamma "
                           "(LevelE,E,GuessedE): "
                           "%f %f %f\n",(*gamma)->getLevelRecord()->getEnergy(),
                           energy, guessedLevelEnergy);
        }

        double bestMatch = 1E10;
        LevelRecord *level;
        for (vector<LevelRecord * >::iterator it = myLevelRecords.begin();
                it!=myLevelRecords.end(); it++) {

            double testMatch = fabs((*it)->getEnergy()-guessedLevelEnergy);

            if (testMatch < bestMatch &&
                    (testMatch < guessedLevelEnergy*0.3 || testMatch < 20)) {

                bestMatch = testMatch;
                level = (*it);
            }
        }
        if (bestMatch == 1E10) {
            egsWarning("EGS_Ensdf::normalizeIntensities: Warning: Could "
                       "not find a level with energy matching decay "
                       "of gamma with energy E=%f; "
                       "assuming final level is ground state\n",energy);
            (*gamma)->setFinalLevel(myLevelRecords.front());
        }
        else {
            (*gamma)->setFinalLevel(level);
        }

        if (verbose) {
            egsInformation("EGS_Ensdf::normalizeIntensities: Gamma (final level E, I): "
                           "%f %f\n",level->getEnergy(), (*gamma)->getTransitionIntensity());
        }

        (*gamma)->getFinalLevel()->cumulDisintegrationIntensity((*gamma)->getTransitionIntensity());
    }

    // Get the gamma transition intensities
    // and the total intensity for each level
    unsigned int j = 0;
    vector<double> totalLevelIntensity;
    totalLevelIntensity.resize(myLevelRecords.size());
    for (vector<LevelRecord * >::iterator it = myLevelRecords.begin();
            it!=myLevelRecords.end(); it++) {

        double disintIntensity = (*it)->getDisintegrationIntensity();

        totalLevelIntensity[j] = 0;
        for (vector<GammaRecord *>::iterator gamma = myGammaRecords.begin();
                gamma != myGammaRecords.end(); gamma++) {

            if ((*gamma)->getLevelRecord() == (*it)) {
                totalLevelIntensity[j] += (*gamma)->getTransitionIntensity();
            }
        }

        if (disintIntensity > 1e-10 && totalLevelIntensity[j] < disintIntensity + 1e-10) {
            totalLevelIntensity[j] = disintIntensity;
            if (verbose > 1) {
                egsInformation("EGS_Ensdf::normalizeIntensities: "
                               "disintegrationIntensity: %f\n", totalLevelIntensity[j]);
            }
        }
        ++j;
    }

    // Normalize transition intensities over each level
    j = 0;
    for (vector<LevelRecord * >::iterator it = myLevelRecords.begin();
            it!=myLevelRecords.end(); it++) {

        unsigned int i = 0;
        bool levelCanDecay = false;
        for (vector<GammaRecord *>::iterator gamma = myGammaRecords.begin();
                gamma != myGammaRecords.end(); gamma++) {

            if ((*gamma)->getLevelRecord() == (*it)) {
                levelCanDecay = true;

                if (totalLevelIntensity[j] > 1e-10) {
                    (*gamma)->setTransitionIntensity(
                        (*gamma)->getTransitionIntensity() /
                        totalLevelIntensity[j]);
                }

                if (i > 0) {
                    (*gamma)->setTransitionIntensity(
                        (*gamma)->getTransitionIntensity() +
                        (*(gamma-1))->getTransitionIntensity());
                }
                ++i;

                if (verbose > 1) {
                    egsInformation("EGS_Ensdf::normalizeIntensities: "
                                   "Gamma (level,E,I): "
                                   "%d %f %f\n",
                                   j,(*gamma)->getDecayEnergy(), (*gamma)->getTransitionIntensity());
                }
            }
        }

        // Set whether or not the level can decay
        // If there are no gammas that decay from this excited energy state,
        // then it will effectively stay in this state forever
        // In practice this means we don't have to sample for the emission of
        // transition photons
        (*it)->setLevelCanDecay(levelCanDecay);

        ++j;
    }
}

void EGS_Ensdf::getEmissionsFromComments() {
    if (verbose) {
        egsInformation("EGS_Ensdf::getEmissionsFromComments: Attempting to obtain x-ray and Auger emissions from the ENSDF comments. This assumes a particular comment format...\n");
    }

    bool xrayContinues = false;
    bool augerContinues = false;
    bool gotTotal = false;
    vector<double>  multilineEnergies,
           multilineIntensities;
    double lineTotalIntensity;
    unsigned int countNumAfterTotal = 0;
    int lineTotalType;

    for (vector<CommentRecord *>::iterator comment = myCommentRecords.begin();
            comment != myCommentRecords.end(); comment++) {

        string line = (*comment)->getComment();

        // Check for the end of multi-line records
        // and average them together
        if (line.length() < 48 ||
                ((xrayContinues || augerContinues) && line.at(30) != '|')) {

            // If we just finished going through a series of
            // lines that started with a "total" line at the top
            // then we'll check to make sure they have intensities assigned
            if (gotTotal) {

                // In the event that a zero intensity is in one of the lines
                // following the "total" line, ALL of those following
                // lines are assigned an equal fraction of the total
                // intensity. This is an imperfect work-around for insufficient
                // data. Using this method, the correct energies are used,
                // rather than assigning a single averaged "total" energy line.
                if (countNumAfterTotal > 0) {
                    // X-rays
                    if (lineTotalType == 0) {
                        bool containsZeroIntensity = false;
                        for (std::vector<double>::iterator it = xrayIntensities.end()-countNumAfterTotal; it != xrayIntensities.end(); ++it) {
                            if (*it < 1e-10) {
                                containsZeroIntensity = true;
                                break;
                            }
                        }

                        if (containsZeroIntensity) {
                            for (std::vector<double>::iterator it = xrayIntensities.end()-countNumAfterTotal; it != xrayIntensities.end(); ++it) {
                                *it = lineTotalIntensity / countNumAfterTotal;
                            }
                        }

                        // Auger
                    }
                    else if (lineTotalType == -1) {
                        bool containsZeroIntensity = false;
                        for (std::vector<double>::iterator it = augerIntensities.end()-countNumAfterTotal; it != augerIntensities.end(); ++it) {
                            if (*it < 1e-10) {
                                containsZeroIntensity = true;
                                break;
                            }
                        }

                        if (containsZeroIntensity) {
                            for (std::vector<double>::iterator it = augerIntensities.end()-countNumAfterTotal; it != augerIntensities.end(); ++it) {
                                *it = lineTotalIntensity / countNumAfterTotal;
                            }
                        }
                    }
                }

                gotTotal = false;
                countNumAfterTotal = 0;
                lineTotalIntensity = 0.;
            }

            if ((xrayContinues || augerContinues)
                    && multilineEnergies.size() > 0) {

                double energySum = 0;
                double intensitySum = 0;
                unsigned int numNonzeroE = 0;
                unsigned int numNonzeroI = 0;
                for (unsigned int i=0; i < multilineEnergies.size(); ++i) {
                    if (multilineEnergies[i] > 0) {
                        energySum += multilineEnergies[i];
                        numNonzeroE++;
                    }
                }
                for (unsigned int i=0; i < multilineIntensities.size(); ++i) {
                    if (multilineIntensities[i] > 1e-10) {
                        intensitySum += multilineIntensities[i];
                        numNonzeroI++;
                    }
                }
                double energy;
                if (numNonzeroE > 0) {
                    energy = energySum / numNonzeroE;
                }
                double intensity;
                if (numNonzeroI > 0) {
                    intensity = intensitySum / numNonzeroI;
                }

                if (numNonzeroE > 0 && numNonzeroI > 0) {
                    if (xrayContinues) {
                        xrayEnergies.push_back(energy);
                        xrayIntensities.push_back(intensity);
                    }
                    else {
                        augerEnergies.push_back(energy);
                        augerIntensities.push_back(intensity);
                    }
                }

                multilineEnergies.clear();
                multilineIntensities.clear();
            }

            xrayContinues = false;
            augerContinues = false;
        }

        // Check for records containing XRays or Auger electrons
        if (line.length() > 48) {

            string emissionLine = egsTrimString(line.substr(47));

            // See if the line is an XRay or Auger
            if (emissionLine.at(0) != 'X' &&
                    emissionLine.find("AUGER") == std::string::npos) {
                continue;
            }

            string eStr = egsTrimString(line.substr(13, 15));

            // If we have a range in energy (e.g. 0.1-0.3)
            // Find the average
            size_t eDash = eStr.find('-');
            double energy;
            if (eDash!=std::string::npos) {
                if (eStr.length() > eDash+1) {
                    double e1 = atof(eStr.substr(0, eDash).c_str());
                    double e2 = atof(eStr.substr(eDash+1).c_str());
                    energy = (e1 + e2) / 2;
                }
                else {
                    energy = atof(eStr.substr(0, eDash).c_str());
                }
            }
            else {
                energy = atof(eStr.c_str());
            }

            // Convert the energy from keV to MeV
            energy /= 1000.;

            // Get the intensity
            string iStr = egsTrimString(line.substr(32, 9));
            double intensity = atof(iStr.c_str());

            // If this is a line coming after a "total" line,
            // increment a counter. This will be used in the
            // event that the lines following the "total"
            // have zero intensity assigned
            if (gotTotal && energy > 1e-10) {
                countNumAfterTotal++;
            }

            // If this line is the total of the next lines, we will
            // skip this line and use the individual ones
            // However, record the total intensity in case we need it
            if (emissionLine.find("(total)") != std::string::npos) {
                gotTotal = true;
                lineTotalIntensity = intensity;
                if (emissionLine.find("AUGER") != std::string::npos) {
                    lineTotalType = -1;
                }
                else {
                    lineTotalType = 0;
                }
                continue;
            }

            // Multi-line records have a bar '|' at 30
            // We will store the data and average them later
            if (line.at(30) == '|') {
                if (emissionLine.at(0) == 'X') {
                    xrayContinues = true;
                }
                else if (emissionLine.find("AUGER") != std::string::npos) {
                    augerContinues = true;
                }

                multilineEnergies.push_back(energy);
                multilineIntensities.push_back(intensity);

            }
            else {
                if (emissionLine.at(0) == 'X') {
                    if ((energy > 1e-10 && intensity > 1e-10) ||
                            (gotTotal && energy > 1e-10)) {
                        xrayEnergies.push_back(energy);
                        xrayIntensities.push_back(intensity);
                    }
                }
                else if (emissionLine.find("AUGER") != std::string::npos) {
                    if ((energy > 1e-10 && intensity > 1e-10) ||
                            (gotTotal && energy > 1e-10)) {
                        augerEnergies.push_back(energy);
                        augerIntensities.push_back(intensity);
                    }
                }
            }
        }
    }
}

vector<double > EGS_Ensdf::getXRayIntensities() const {
    return xrayIntensities;
}

vector<double > EGS_Ensdf::getXRayEnergies() const {
    return xrayEnergies;
}

vector<double > EGS_Ensdf::getAugerIntensities() const {
    return augerIntensities;
}

vector<double > EGS_Ensdf::getAugerEnergies() const {
    return augerEnergies;
}

vector<ParentRecord * > EGS_Ensdf::getParentRecords() const {
    return myParentRecords;
}

vector<LevelRecord * > EGS_Ensdf::getLevelRecords() const {
    return myLevelRecords;
}

vector<BetaRecordLeaf * > EGS_Ensdf::getBetaRecords() const {
    return myBetaRecords;
}

vector<GammaRecord * > EGS_Ensdf::getGammaRecords() const {
    return myGammaRecords;
}

vector<GammaRecord * > EGS_Ensdf::getMetastableGammaRecords() const {
    return myMetastableGammaRecords;
}

vector<AlphaRecord * > EGS_Ensdf::getAlphaRecords() const {
    return myAlphaRecords;
}

Record::Record() {};
Record::Record(vector<string> ensdf) {
    if (!ensdf.empty()) {
        lines = ensdf;
    }
}

Record::~Record() {

}

vector<string> Record::getRecords() const {
    return lines;
}

double Record::recordToDouble(int startPos, int endPos) {
    if (!lines.empty()) {
        if (lines.front().length() < startPos) {
            egsWarning("Record::recordToDouble: Warning: Record too short to "
                       "contain desired quantity\n");
            return -1;
        }
        string record = lines.front().substr(startPos-1,
                                             endPos-startPos+1);
        return atof(record.c_str());
    }
    else {
        egsWarning("Record::recordToDouble: Error: Record is empty\n");
        return -1;
    }
}

// Parse a halflife from a record
// Converts the units to seconds
// Returns the halflife, or a negative number upon failure
double Record::parseHalfLife(int startPos, int endPos) {
    if (lines.empty()) {
        egsWarning("Record::parseHalfLife: Error: Record is empty\n");
        return -5;
    }

    string halfLifeStr = egsTrimString(lines.front().substr(startPos-1,
                                       endPos-startPos+1));

    // Return -1 for stable
    if (halfLifeStr.substr(0,5).compare("STABLE") == 0) {
        return -1;
    }

    // Store the length of the numeric part of the string in i
    unsigned int numLength;
    for (numLength = 0; numLength < halfLifeStr.length(); numLength++) {
        if (!isdigit(halfLifeStr[numLength])
                && halfLifeStr.at(numLength) != '.') {

            break;
        }
    }

    // If there was no numeric component return -2
    if (halfLifeStr.size() < numLength+2) {
        return -2;
    }

    // Get the numeric part
    double hl = atof(halfLifeStr.substr(0, numLength).c_str());

    // Convert to units of seconds
    if (halfLifeStr.size()>numLength+2) {
        string units = halfLifeStr.substr(numLength+1, 2);
        if (units.compare("Y ") == 0) {
            hl *= 31556925.26;
        }
        else if (units.compare("D ") == 0) {
            hl *= 86400;
        }
        else if (units.compare("H ") == 0) {
            hl *= 3600;
        }
        else if (units.compare("M ") == 0) {
            hl *= 60;
        }
        else if (units.compare("S ") == 0) {
            hl *= 1;
        }
        else if (units.compare("MS") == 0) {
            hl *= 1E-3;
        }
        else if (units.compare("US") == 0) {
            hl *= 1E-6;
        }
        else if (units.compare("NS") == 0) {
            hl *= 1E-9;
        }
        else if (units.compare("PS") == 0) {
            hl *= 1E-12;
        }
        else if (units.compare("FS") == 0) {
            hl *= 1E-15;
        }
        else if (units.compare("AS") == 0) {
            hl *= 1E-18;
        }
        else {
            return -3;
        }
    }
    else if (halfLifeStr.size()>numLength+1) {
        string units = halfLifeStr.substr(numLength+1, 1);
        if (units.compare("Y") == 0) {
            hl *= 31556925.26;
        }
        else if (units.compare("D") == 0) {
            hl *= 86400;
        }
        else if (units.compare("H") == 0) {
            hl *= 3600;
        }
        else if (units.compare("M") == 0) {
            hl *= 60;
        }
        else if (units.compare("S") == 0) {
            hl *= 1;
        }
        else {
            return -3;
        }
    }
    else {
        hl = -4;
    }

    return hl;
}

unsigned short int Record::setZ(string id) {

    string element;
    for (unsigned int i=0; i < id.length(); ++i) {
        if (!isdigit(id[i])) {
            element.push_back(id[i]);
        }
    }

    unsigned short int Z = findZ(element);
    if (Z == 0) {
        egsWarning("EGS_Ensdf::createIsotope: Warning: Element does not exist "
                   "in our data (%s)\n", element.c_str());
    }

    return Z;
}

map<string, unsigned short int> Record::getElementMap() {
    map<string, unsigned short int> elementTable;
    elementTable["H"] = 1;
    elementTable["HE"] = 2;
    elementTable["LI"] = 3;
    elementTable["BE"] = 4;
    elementTable["B"] = 5;
    elementTable["C"] = 6;
    elementTable["N"] = 7;
    elementTable["O"] = 8;
    elementTable["F"] = 9;
    elementTable["NE"] = 10;
    elementTable["NA"] = 11;
    elementTable["MG"] = 12;
    elementTable["AL"] = 13;
    elementTable["SI"] = 14;
    elementTable["P"] = 15;
    elementTable["S"] = 16;
    elementTable["CL"] = 17;
    elementTable["AR"] = 18;
    elementTable["K"] = 19;
    elementTable["CA"] = 20;
    elementTable["SC"] = 21;
    elementTable["TI"] = 22;
    elementTable["V"] = 23;
    elementTable["CR"] = 24;
    elementTable["MN"] = 25;
    elementTable["FE"] = 26;
    elementTable["CO"] = 27;
    elementTable["NI"] = 28;
    elementTable["CU"] = 29;
    elementTable["ZN"] = 30;
    elementTable["GA"] = 31;
    elementTable["GE"] = 32;
    elementTable["AS"] = 33;
    elementTable["SE"] = 34;
    elementTable["BR"] = 35;
    elementTable["KR"] = 36;
    elementTable["RB"] = 37;
    elementTable["SR"] = 38;
    elementTable["Y"] = 39;
    elementTable["ZR"] = 40;
    elementTable["NB"] = 41;
    elementTable["MO"] = 42;
    elementTable["TC"] = 43;
    elementTable["RU"] = 44;
    elementTable["RH"] = 45;
    elementTable["PD"] = 46;
    elementTable["AG"] = 47;
    elementTable["CD"] = 48;
    elementTable["IN"] = 49;
    elementTable["SN"] = 50;
    elementTable["SB"] = 51;
    elementTable["TE"] = 52;
    elementTable["I"] = 53;
    elementTable["XE"] = 54;
    elementTable["CS"] = 55;
    elementTable["BA"] = 56;
    elementTable["LA"] = 57;
    elementTable["CE"] = 58;
    elementTable["PR"] = 59;
    elementTable["ND"] = 60;
    elementTable["PM"] = 61;
    elementTable["SM"] = 62;
    elementTable["EU"] = 63;
    elementTable["GD"] = 64;
    elementTable["TB"] = 65;
    elementTable["DY"] = 66;
    elementTable["HO"] = 67;
    elementTable["ER"] = 68;
    elementTable["TM"] = 69;
    elementTable["YB"] = 70;
    elementTable["LU"] = 71;
    elementTable["HF"] = 72;
    elementTable["TA"] = 73;
    elementTable["W"] = 74;
    elementTable["RE"] = 75;
    elementTable["OS"] = 76;
    elementTable["IR"] = 77;
    elementTable["PT"] = 78;
    elementTable["AU"] = 79;
    elementTable["HG"] = 80;
    elementTable["TL"] = 81;
    elementTable["PB"] = 82;
    elementTable["BI"] = 83;
    elementTable["PO"] = 84;
    elementTable["AT"] = 85;
    elementTable["RN"] = 86;
    elementTable["FR"] = 87;
    elementTable["RA"] = 88;
    elementTable["AC"] = 89;
    elementTable["TH"] = 90;
    elementTable["PA"] = 91;
    elementTable["U"] = 92;
    elementTable["NP"] = 93;
    elementTable["PU"] = 94;
    elementTable["AM"] = 95;
    elementTable["CM"] = 96;
    elementTable["BK"] = 97;
    elementTable["CF"] = 98;
    elementTable["ES"] = 99;
    elementTable["FM"] = 100;
    elementTable["MD"] = 101;
    elementTable["NO"] = 102;
    elementTable["LR"] = 103;
    elementTable["RF"] = 104;
    elementTable["DB"] = 105;
    elementTable["SG"] = 106;
    elementTable["BH"] = 107;
    elementTable["HS"] = 108;
    elementTable["MT"] = 109;
    elementTable["DS"] = 110;
    elementTable["RG"] = 111;
    elementTable["CN"] = 112;
    elementTable["UUT"] = 113;
    elementTable["UUQ"] = 114;
    elementTable["UUP"] = 115;
    elementTable["UUH"] = 116;
    elementTable["UUS"] = 117;
    elementTable["UUO"] = 118;

    return elementTable;
}

unsigned short int Record::findZ(string element) {

    transform(element.begin(), element.end(), element.begin(), ::toupper);

    map<string, unsigned short int> elementMap = getElementMap();

    if (elementMap.find(element) != elementMap.end()) {
        return elementMap[element];
    }
    else {
        return 0;
    }
}

// Comment Record
CommentRecord::CommentRecord(vector<string> ensdf):Record(ensdf) {
    processEnsdf();
}

void CommentRecord::processEnsdf() {
    if (!lines.empty()) {

        comment = lines.front();
    }
}

string CommentRecord::getComment() {
    return comment;
}

// Parent Record
ParentRecord::ParentRecord(vector<string> ensdf):Record(ensdf) {
    processEnsdf();
}

void ParentRecord::processEnsdf() {
    halfLife = parseHalfLife(40, 49);

    // Ground state Q-value in keV
    // (total energy available for g.s. -> g.s. transition
    // It will always be a positive number
    // We convert to MeV
    Q = recordToDouble(65, 74) / 1000.;

    // If the Q was not contained in the record it returned -1
    if (Q == -0.001) {
        egsWarning("ParentRecord::processEnsdf: Warning: No Q-value given, any "
                   "positron records will give errors\n");
        Q = 0.;
    }
}

double ParentRecord::getHalfLife() const {
    return halfLife;
}

double ParentRecord::getQ() const {
    return Q;
}

ParentRecord *ParentRecordLeaf::getParentRecord() const {
    return getBranch();
}

ParentRecordLeaf::ParentRecordLeaf(ParentRecord
                                   *myRecord):Leaf<ParentRecord>(myRecord) {

}

// Normalization Record
NormalizationRecord::NormalizationRecord(vector<string> ensdf,
        ParentRecord *myParent):Record(ensdf), ParentRecordLeaf(myParent) {
    processEnsdf();
}

void NormalizationRecord::processEnsdf() {
    normalizeRelative = recordToDouble(10, 19);
    normalizeTransition = recordToDouble(22, 29);
    normalizeBranch = recordToDouble(32, 39);
    normalizeBeta = recordToDouble(42, 49);
}

// Multiplier for converting relative photon intensity to photons per 100
// decays in the parent through the decay branch or to photons per 100 neutron
// captures in an (n,gamma) reaction. Required if the absolute photon intensity
// can be calculated
double NormalizationRecord::getRelativeMultiplier() const {
    return normalizeRelative;
}

// Multiplier for convert relative transition intensity (including conversion
// electrons) to transitions per 100 decays of the parent through this decay
// branch or per 100 neutron captures in an (n,gamma) reaction
double NormalizationRecord::getTransitionMultiplier() const {
    return normalizeTransition;
}

// Branching ratio multiplier for converting intensity per 100 decays
// through this decay branch to intensity per 100 decays of the parent nuclide
double NormalizationRecord::getBranchMultiplier() const {
    return normalizeBranch;
}

// Multiplier for converting relative beta- and electron capture intensities to
// intensities per 100 decays through this decay branch. Required if known
double NormalizationRecord::getBetaMultiplier() const {
    return normalizeBeta;
}

NormalizationRecord *NormalizationRecordLeaf::getNormalizationRecord()
const {
    return getBranch();
}

NormalizationRecordLeaf::NormalizationRecordLeaf(NormalizationRecord
        *myRecord):Leaf<NormalizationRecord>(myRecord) {

}

// Level Record
LevelRecord::LevelRecord() {
    energy = 0;
    halfLife = 0;
    disintegrationIntensity = 0;
}
LevelRecord::LevelRecord(vector<string> ensdf):
    Record(ensdf) {
    processEnsdf();
    disintegrationIntensity = 0;
}

void LevelRecord::processEnsdf() {
    energy = recordToDouble(10, 19) / 1000.; // Convert keV to MeV
    halfLife = parseHalfLife(40, 49);
}

void LevelRecord::setLevelCanDecay(bool canDecayTmp) {
    canDecay = canDecayTmp;
}

bool LevelRecord::levelCanDecay() const {
    return canDecay;
}

void LevelRecord::cumulDisintegrationIntensity(double disintIntensity) {
    disintegrationIntensity += disintIntensity;
}

double LevelRecord::getDisintegrationIntensity() const {
    return disintegrationIntensity;
}

double LevelRecord::getEnergy() const {
    return energy;
}

double LevelRecord::getHalfLife() const {
    return halfLife;
}

LevelRecord *LevelRecordLeaf::getLevelRecord() const {
    return getBranch();
}

LevelRecordLeaf::LevelRecordLeaf(LevelRecord
                                 *myRecord):Leaf<LevelRecord>(myRecord) {

}

// Beta Record
BetaRecordLeaf::BetaRecordLeaf(vector<string> ensdf,
                               ParentRecord *myParent,
                               NormalizationRecord *myNormalization,
                               LevelRecord *myLevel):
    ParentRecordLeaf(myParent),
    NormalizationRecordLeaf(myNormalization),
    LevelRecordLeaf(myLevel),
    Record(ensdf) {

    numSampled = 0;

    // Set the Z and atomic weight for the daughter of this decay
    string id = egsRemoveWhite(lines.front().substr(0,5));
    Z = setZ(id);

    string atomicWeight;
    for (unsigned int i=0; i < id.length(); ++i) {
        if (!isdigit(id[i])) {
            break;
        }
        else {
            atomicWeight.push_back(id[i]);
        }
    }
    A = atoi(atomicWeight.c_str());

    // Get the forbiddenness
    string lambda;
    lambda.push_back(lines.front().at(77));
    forbidden = atoi(lambda.c_str());
}
int BetaRecordLeaf::getCharge() const {
    return q;
}

void BetaRecordLeaf::incrNumSampled() {
    numSampled++;
}

EGS_I64 BetaRecordLeaf::getNumSampled() const {
    return numSampled;
}

unsigned short int BetaRecordLeaf::getZ() const {
    return Z;
}

unsigned short int BetaRecordLeaf::getAtomicWeight() const {
    return A;
}

unsigned short int BetaRecordLeaf::getForbidden() const {
    return forbidden;
}

void BetaRecordLeaf::setSpectrum(EGS_AliasTable *bspec) {
    spectrum = bspec;
}

EGS_AliasTable *BetaRecordLeaf::getSpectrum() const {
    return spectrum;
}

// Beta- Record
BetaMinusRecord::BetaMinusRecord(vector<string> ensdf,
                                 ParentRecord *myParent,
                                 NormalizationRecord *myNormalization,
                                 LevelRecord *myLevel):
    BetaRecordLeaf(ensdf, myParent,
                   myNormalization, myLevel) {
    processEnsdf();
    q = -1;
    myLevel->cumulDisintegrationIntensity(betaIntensity);
}

void BetaMinusRecord::processEnsdf() {
    finalEnergy = recordToDouble(10, 19) / 1000.; // Convert keV to MeV
    betaIntensity = recordToDouble(22, 29);
    if (getNormalizationRecord()) {
        betaIntensity *= getNormalizationRecord()->getBetaMultiplier() *
                         getNormalizationRecord()->getBranchMultiplier();
    }
}

double BetaMinusRecord::getFinalEnergy() const {
    return finalEnergy;
}

double BetaMinusRecord::getBetaIntensity() const {
    return betaIntensity;
}

void BetaMinusRecord::setBetaIntensity(double newIntensity) {
    betaIntensity = newIntensity;
}

// Beta+ Record (and Electron Capture)
BetaPlusRecord::BetaPlusRecord(vector<string> ensdf,
                               ParentRecord *myParent,
                               NormalizationRecord *myNormalization,
                               LevelRecord *myLevel):
    BetaRecordLeaf(ensdf, myParent,
                   myNormalization, myLevel) {
    processEnsdf();
    q = 1;
    myLevel->cumulDisintegrationIntensity(betaIntensity);
}

void BetaPlusRecord::processEnsdf() {
    finalEnergy = recordToDouble(10, 19) / 1000.; // Convert keV to MeV
    positronIntensity = recordToDouble(22, 29);
    ecIntensity = recordToDouble(32, 39);
    if (getNormalizationRecord()) {
        positronIntensity *= getNormalizationRecord()->getBetaMultiplier() *
                             getNormalizationRecord()->getBranchMultiplier();
        ecIntensity *= getNormalizationRecord()->getBetaMultiplier() *
                       getNormalizationRecord()->getBranchMultiplier();
    }

    // The total intensity for this decay branch
    // A decay down this branch will then be split between positron or EC
    betaIntensity = positronIntensity + ecIntensity;

    // Re-normalize the intensities to make it easier to sample which occurs
    positronIntensity = positronIntensity / betaIntensity;
    ecIntensity = positronIntensity + ecIntensity / betaIntensity;

    // For positrons we may need to calculate the emission energy
    // E = Q - level_energy - 2*mc^2
    if (finalEnergy == 0 && positronIntensity > 1e-10) {
        finalEnergy = getParentRecord()->getQ()
                      - getLevelRecord()->getEnergy() - 1.022;

        if (finalEnergy < 0.) {
            egsWarning("BetaPlusRecord::processEnsdf: Error: Final energy of "
                       "positron could not be calculated. Setting energy to zero!\n"
                      );
            finalEnergy = 0.;
        }
    }
}

double BetaPlusRecord::getFinalEnergy() const {
    return finalEnergy;
}

double BetaPlusRecord::getBetaIntensity() const {
    return betaIntensity;
}

double BetaPlusRecord::getPositronIntensity() const {
    return positronIntensity;
}

double BetaPlusRecord::getECIntensity() const {
    return ecIntensity;
}

void BetaPlusRecord::setBetaIntensity(double newIntensity) {
    betaIntensity = newIntensity;
}

void BetaPlusRecord::setECIntensity(double newIntensity) {
    ecIntensity = newIntensity;
}

// Gamma Record
GammaRecord::GammaRecord(vector<string> ensdf,
                         ParentRecord *myParent,
                         NormalizationRecord *myNormalization,
                         LevelRecord *myLevel):
    Record(ensdf),
    ParentRecordLeaf(myParent),
    NormalizationRecordLeaf(myNormalization),
    LevelRecordLeaf(myLevel) {
    processEnsdf();
    q = 0;
    numSampled = 0;
}

GammaRecord::GammaRecord(GammaRecord *gamma):
    Record(),
    ParentRecordLeaf(gamma->getParentRecord()),
    NormalizationRecordLeaf(gamma->getNormalizationRecord()),
    LevelRecordLeaf(gamma->getLevelRecord()) {

    q = gamma->q;
    decayEnergy = 0;
    transitionIntensity = 0;
    numSampled = 0;
}

void GammaRecord::processEnsdf() {
    decayEnergy = recordToDouble(10, 19) / 1000.; // Convert keV to MeV
    transitionIntensity = recordToDouble(22, 29);

    if (getNormalizationRecord()) {
        transitionIntensity *=
            getNormalizationRecord()->getRelativeMultiplier() *
            getNormalizationRecord()->getBranchMultiplier();
    }
}

double GammaRecord::getDecayEnergy() const {
    return decayEnergy;
}

double GammaRecord::getTransitionIntensity() const {
    return transitionIntensity;
}

void GammaRecord::setTransitionIntensity(double newIntensity) {
    transitionIntensity = newIntensity;
}

int GammaRecord::getCharge() const {
    return q;
}

void GammaRecord::incrNumSampled() {
    numSampled++;
}

EGS_I64 GammaRecord::getNumSampled() const {
    return numSampled;
}

LevelRecord *GammaRecord::getFinalLevel() const {
    return finalLevel;
}

void GammaRecord::setFinalLevel(LevelRecord *newLevel) {
    finalLevel = newLevel;
}

// Alpha Record
AlphaRecord::AlphaRecord(vector<string> ensdf,
                         ParentRecord *myParent,
                         NormalizationRecord *myNormalization,
                         LevelRecord *myLevel):
    Record(ensdf),
    ParentRecordLeaf(myParent), NormalizationRecordLeaf(myNormalization),
    LevelRecordLeaf(myLevel) {

    processEnsdf();
    q = 2;
    numSampled = 0;
    myLevel->cumulDisintegrationIntensity(alphaIntensity);
}

void AlphaRecord::processEnsdf() {
    finalEnergy = recordToDouble(10, 19) / 1000.; // Convert keV to MeV
    alphaIntensity = recordToDouble(22, 29);
}

double AlphaRecord::getFinalEnergy() const {
    return finalEnergy;
}

double AlphaRecord::getAlphaIntensity() const {
    return alphaIntensity;
}

void AlphaRecord::setAlphaIntensity(double newIntensity) {
    alphaIntensity = newIntensity;
}

int AlphaRecord::getCharge() const {
    return q;
}

void AlphaRecord::incrNumSampled() {
    numSampled++;
}

EGS_I64 AlphaRecord::getNumSampled() const {
    return numSampled;
}

