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

map<string, unsigned short int> getElementMap() {
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

unsigned short int findZ(string element) {

    transform(element.begin(), element.end(), element.begin(), ::toupper);

    map<string, unsigned short int> elementMap = getElementMap();

    if (elementMap.find(element) != elementMap.end()) {
        return elementMap[element];
    }
    else {
        return 0;
    }
}

unsigned short int setZ(string id) {

    string element;
    for (unsigned int i=0; i < id.length(); ++i) {
        if (!isdigit(id[i])) {
            element.push_back(id[i]);
        }
    }

    unsigned short int Z = findZ(element);
    if (Z == 0) {
        egsWarning("setZ: Warning: Element does not exist "
                   "in our data (%s)\n", element.c_str());
    }

    return Z;
}

EGS_Ensdf::EGS_Ensdf(const string nuclide, const string ensdf_filename, const string relaxType, const bool allowMultiTrans, int verbosity) {

    verbose = verbosity;
    relaxationType = relaxType;
    allowMultiTransition = allowMultiTrans;

    if (ensdf_file.is_open()) {
        ensdf_file.close();
    }

    radionuclide = nuclide.substr(0, nuclide.find_last_of("."));

    // The parent element
    string element = radionuclide.substr(0, radionuclide.find("-"));
    Z = setZ(element);

    egsInformation("EGS_Ensdf::EGS_Ensdf: Nuclide: "
                   "%s\n",nuclide.c_str());
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
    for (vector<GammaRecord * >::iterator it =
                myMetastableGammaRecords.begin();
            it!=myMetastableGammaRecords.end(); it++) {
        delete *it;
        *it=0;
    }
    myMetastableGammaRecords.clear();
    for (vector<GammaRecord * >::iterator it =
                myUncorrelatedGammaRecords.begin();
            it!=myUncorrelatedGammaRecords.end(); it++) {
        delete *it;
        *it=0;
    }
    myUncorrelatedGammaRecords.clear();
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

            if (line[7]=='G') {
                // If this is related to a gamma record, keep it
                recordStack[12].push_back(line);
            }
            else {
                // General comment
                recordStack[4].push_back(line);
            }

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

    // Get X-ray and auger emissions from comments
    if (relaxationType == "ensdf") {
        if (verbose) {
            egsInformation("EGS_Ensdf::parseEnsdf: Checking for x-rays and Auger...\n");
        }

        getEmissionsFromComments();

        if (verbose > 1) {
            egsInformation("EGS_Ensdf::parseEnsdf: Done checking for x-rays and Auger.\n");
        }
    }

    // Get rid of very low emission probability particles
    double minimumIntensity = 1e-10;
    for (vector<BetaMinusRecord * >::iterator it = myBetaMinusRecords.begin();
            it!=myBetaMinusRecords.end();) {
        if ((*it)->getBetaIntensity() <= minimumIntensity) {
            if (verbose) {
                egsInformation("EGS_Ensdf::parseEnsdf: Removing beta- due to small intensity (%.1e < %.1e)\n",(*it)->getBetaIntensity(),minimumIntensity);
            }
            myBetaMinusRecords.erase(it);
        }
        else {
            it++;
        }
    }
    for (vector<BetaPlusRecord * >::iterator it = myBetaPlusRecords.begin();
            it!=myBetaPlusRecords.end();) {
        if ((*it)->getBetaIntensity() <= minimumIntensity) {
            if (verbose) {
                egsInformation("EGS_Ensdf::parseEnsdf: Removing beta+ due to small intensity (%.1e < %.1e)\n",(*it)->getBetaIntensity(),minimumIntensity);
            }
            myBetaPlusRecords.erase(it);
        }
        else {
            it++;
        }
    }
    for (vector<AlphaRecord *>::iterator it = myAlphaRecords.begin();
            it != myAlphaRecords.end();) {
        if ((*it)->getAlphaIntensity() <= minimumIntensity) {
            if (verbose) {
                egsInformation("EGS_Ensdf::parseEnsdf: Removing alpha due to small intensity (%.1e < %.1e)\n",(*it)->getAlphaIntensity(),minimumIntensity);
            }
            myAlphaRecords.erase(it);
        }
        else {
            it++;
        }
    }

    // Search through the gamma records for any with unknown levels
    // or with low emission probability
    bool printedWarning = false;
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
        else if ((*it)->getLevelRecord()->getEnergy() < epsilon) {
            // Some gamma may be emitted but the energy level is not known
            // This is reported in the lnhb data as decays from the -1 level
            // Since we cannot correlate the emission with a change of energy
            // states of the daughter, we will treat this transition independently

            if (!printedWarning) {
                egsWarning("EGS_Ensdf::parseEnsdf: Warning: Switching internal transition with unknown decay level to uncorrelated event (the emissions will still occur, but uncorrelated with disintegrations).\n");
                printedWarning = true;
            }

            myUncorrelatedGammaRecords.push_back(new GammaRecord(*it));

            egsInformation("EGS_Ensdf::parseEnsdf: Uncorrelated gamma (E,I): %f %f\n", myUncorrelatedGammaRecords.back()->getDecayEnergy(), myUncorrelatedGammaRecords.back()->getTransitionIntensity());

            // Erase the gamma record object
            myGammaRecords.erase(it);
        }
        else {
            ++it;
        }
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

    // Check for isomeric transitions with low probability
    // This was specifically implemented to handle Th-134 in the LNHB library. I haven't found any other radionuclide with a ENSDF file formatted like this.
    for (vector<LevelRecord * >::iterator it = myLevelRecords.begin();
            it!=myLevelRecords.end(); it++) {

        if(it != myLevelRecords.begin()) {
            auto itprev = std::prev(it);

            if((*it)->getEnergy() > 0 && (*it)->getEnergy() == (*itprev)->getEnergy()) {
                // Levels have equal energy
                // Check if this is an isomer (T1/2 > 0.1s as defined by ensdf format)
                // If the spin is larger this isomeric transition is probably unlikely
                // We don't have a way to extract the probability, so we'll just neglect the lower probability level
                if((*itprev)->getHalfLife() > 0.1 && (*itprev)->getSpin() < (*it)->getSpin()) {

                    egsWarning("\nEGS_Ensdf::parseEnsdf: Warning: Levels with identical energy, long half-life and different spin have been detected. Assuming a low probability isomeric transition - the lower probability level will be removed. Removing level with energy = %f, spin = %d. Decays toward and transitions away from this level will also be removed. Double check the decay scheme and report any issues!\n\n", (*it)->getEnergy(), (*it)->getSpin());

                    // Go through all the records to remove any that originate from this level that we're removing
                    for (vector<GammaRecord *>::iterator gamma = myGammaRecords.begin(); gamma != myGammaRecords.end(); gamma++) {
                        if((*gamma)->getLevelRecord() == (*it)) {
                            gamma = myGammaRecords.erase(gamma);
                            gamma--;
                        }
                    }
                    for (vector<BetaRecordLeaf *>::iterator beta = myBetaRecords.begin(); beta != myBetaRecords.end(); beta++) {
                        if((*beta)->getLevelRecord() == (*it)) {
                            beta = myBetaRecords.erase(beta);
                            beta--;
                        }
                    }
                    for (vector<BetaMinusRecord *>::iterator beta = myBetaMinusRecords.begin(); beta != myBetaMinusRecords.end(); beta++) {
                        if((*beta)->getLevelRecord() == (*it)) {
                            beta = myBetaMinusRecords.erase(beta);
                            beta--;
                        }
                    }
                    for (vector<BetaPlusRecord *>::iterator beta = myBetaPlusRecords.begin(); beta != myBetaPlusRecords.end(); beta++) {
                        if((*beta)->getLevelRecord() == (*it)) {
                            beta = myBetaPlusRecords.erase(beta);
                            beta--;
                        }
                    }
                    for (vector<AlphaRecord *>::iterator alpha = myAlphaRecords.begin(); alpha != myAlphaRecords.end(); alpha++) {
                        if((*alpha)->getLevelRecord() == (*it)) {
                            alpha = myAlphaRecords.erase(alpha);
                            alpha--;
                        }
                    }


                    // Remove the level
                    it = myLevelRecords.erase(it);
                    it--;
                }
            }
        }
    }


    // Print out a summary of the decays
    egsInformation("\nEGS_Ensdf::parseEnsdf: Summary of %s emissions:\n", radionuclide.c_str());
    egsInformation("========================\n");
    egsInformation("Energy | Intensity per 100 decays\n");
    if (myBetaRecords.size()) {
        egsInformation("Beta records:\n");
        for (vector<BetaRecordLeaf *>::iterator beta = myBetaRecords.begin();
                beta != myBetaRecords.end(); beta++) {
            egsInformation("%f %f\n", (*beta)->getFinalEnergy(), (*beta)->getBetaIntensity());
        }
    }
    if (myAlphaRecords.size()) {
        egsInformation("Alpha records:\n");
        for (vector<AlphaRecord *>::iterator alpha = myAlphaRecords.begin();
                alpha != myAlphaRecords.end(); alpha++) {
            egsInformation("%f %f\n", (*alpha)->getFinalEnergy(), (*alpha)->getAlphaIntensity());
        }
    }
    if (myGammaRecords.size()) {
        egsInformation("Gamma records (E,Igamma,Ice,Ipp):\n");
        for (vector<GammaRecord *>::iterator gamma = myGammaRecords.begin();
                gamma != myGammaRecords.end(); gamma++) {
            double icI = 0;
            double ipI = 0;
            if ((*gamma)->getICIntensity() > 0) {
                icI = (*gamma)->getGammaIntensity()*(1+(*gamma)->getICIntensity()) - (*gamma)->getGammaIntensity();
            }
            if ((*gamma)->getIPIntensity() > 0) {
                ipI = (*gamma)->getTransitionIntensity() - (*gamma)->getGammaIntensity() - icI;
            }
            egsInformation("%f %f %.4e %.4e\n", (*gamma)->getDecayEnergy(), (*gamma)->getGammaIntensity(), icI, ipI);
        }
    }
    if (myUncorrelatedGammaRecords.size()) {
        egsInformation("Uncorrelated gamma records (E,Igamma,Ice,Ipp):\n");
        for (vector<GammaRecord *>::iterator gamma = myUncorrelatedGammaRecords.begin();
                gamma != myUncorrelatedGammaRecords.end(); gamma++) {
            double icI = 0;
            double ipI = 0;
            if ((*gamma)->getICIntensity() > 0) {
                icI = (*gamma)->getGammaIntensity()*(1+(*gamma)->getICIntensity()) - (*gamma)->getGammaIntensity();
            }
            if ((*gamma)->getIPIntensity() > 0) {
                ipI = (*gamma)->getTransitionIntensity() - (*gamma)->getGammaIntensity() - icI;
            }
            egsInformation("%f %f %.4e %.4e\n", (*gamma)->getDecayEnergy(), (*gamma)->getGammaIntensity(), icI, ipI);
        }
    }
    if (xrayEnergies.size() > 0) {
        egsInformation("X-Ray records:\n");
        for (unsigned int i=0; i < xrayEnergies.size(); ++i) {
            egsInformation("%f %f\n", xrayEnergies[i], xrayIntensities[i]);
        }
    }
    if (augerEnergies.size() > 0) {
        egsInformation("Auger records:\n");
        for (unsigned int i=0; i < augerEnergies.size(); ++i) {
            egsInformation("%f %f\n", augerEnergies[i], augerIntensities[i]);
        }
    }
    egsInformation("=== End of summary ===\n");

    // Determine the final level that the gammas decay towards
    // We have to use the gamma decay energy to guess at the resulting
    // energy state of the radionuclide
    for (vector<GammaRecord *>::iterator gamma = myGammaRecords.begin();
            gamma != myGammaRecords.end(); gamma++) {

        double energy = (*gamma)->getDecayEnergy();
        double guessedLevelEnergy =
            ((*gamma)->getLevelRecord()->getEnergy() - energy);

        if (verbose) {
            egsInformation("EGS_Ensdf::parseEnsdf: Gamma "
                           "(LevelE,E,GuessedE): "
                           "%f %f %f\n",(*gamma)->getLevelRecord()->getEnergy(),
                           energy, guessedLevelEnergy);
        }

        double bestMatch = 1E10;
        LevelRecord *level = 0;
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
            egsWarning("EGS_Ensdf::parseEnsdf: Warning: Could "
                       "not find a level with energy matching decay "
                       "of gamma with energy E=%f; "
                       "assuming final level is ground state\n",energy);
            (*gamma)->setFinalLevel(myLevelRecords.front());
        }
        else {
            (*gamma)->setFinalLevel(level);
        }

        if (verbose) {
            egsInformation("EGS_Ensdf::parseEnsdf: Gamma (final level E, I, Igamma): "
                           "%f %f %f\n",level->getEnergy(), (*gamma)->getTransitionIntensity(), (*gamma)->getGammaIntensity());
        }

        (*gamma)->getFinalLevel()->cumulDisintegrationIntensity((*gamma)->getTransitionIntensity());
    }

    // Get the total gamma transition intensity of each level
    unsigned int j = 0;
    vector<double> totalLevelIntensity;
    totalLevelIntensity.resize(myLevelRecords.size());
    for (vector<LevelRecord * >::iterator it = myLevelRecords.begin();
            it!=myLevelRecords.end(); ++it) {

        totalLevelIntensity[j] = 0;
        for (vector<GammaRecord *>::iterator gamma = myGammaRecords.begin();
                gamma != myGammaRecords.end(); gamma++) {

            if ((*gamma)->getLevelRecord() == (*it)) {
                totalLevelIntensity[j] += (*gamma)->getTransitionIntensity();
            }
        }
        ++j;
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

        // No disintegrations, so this must be metastable
        if (!gotDisint) {
            // We're going to need to "fake" disintegrations toward each level
            j=0;
            for (vector<LevelRecord * >::iterator it = myLevelRecords.begin();
                    it!=myLevelRecords.end(); ++it) {

                double disintIntensity = (*it)->getDisintegrationIntensity();
                if (disintIntensity < epsilon) {
                    bool gotDecayToLevel = false;
                    for (vector<GammaRecord *>::iterator gamma = myGammaRecords.begin();
                            gamma < myGammaRecords.end(); ++gamma) {

                        // For each gamma matching the current level and parent
                        if ((*gamma)->getParentRecord() == *parent && (*gamma)->getLevelRecord() == (*it)) {

                            // Once per level, we create a zero energy disintegration
                            // with intensity equal to the gamma transition intensity
                            // leaving the level
                            if (!gotDecayToLevel) {
                                gotDecayToLevel = true;

                                // Push a copy of the gamma record
                                myMetastableGammaRecords.push_back(new GammaRecord(*gamma));

                                // Set the transition intensity to be the disintegration
                                // intensity leading towards this level
                                myMetastableGammaRecords.back()->setTransitionIntensity(totalLevelIntensity[j]);
                            }
                        }
                    }

                    if (verbose && myMetastableGammaRecords.size() > 0) {
                        egsInformation("EGS_Ensdf::parseEnsdf: Metastable nuclide "
                                       "detected.\n");
                    }
                }

                ++j;
            }
        }
    }
    if (verbose && myMetastableGammaRecords.size() < 1) {
        egsInformation("EGS_Ensdf::parseEnsdf: No metastable nuclides "
                       "detected.\n");
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
                       "routines...\n");
    }

    // Add up the beta, alpha, xray and auger decay intensities
    double totalDecayIntensity = 0;
    double totalDecayIntensityUnc = 0;
    double lastIntensity = 0;

    for (vector<BetaMinusRecord * >::iterator it = myBetaMinusRecords.begin();
            it!=myBetaMinusRecords.end(); it++) {

        if (verbose > 1) {
            egsInformation("EGS_Ensdf::normalizeIntensities: Beta- (E,I): %f %f\n",
                           (*it)->getFinalEnergy(), (*it)->getBetaIntensity());
        }

        totalDecayIntensity += (*it)->getBetaIntensity();
        totalDecayIntensityUnc += (*it)->getBetaIntensityUnc();
    }
    for (vector<BetaPlusRecord * >::iterator it = myBetaPlusRecords.begin();
            it!=myBetaPlusRecords.end(); it++) {

        if (verbose > 1) {
            egsInformation("EGS_Ensdf::normalizeIntensities: Beta+/EC (E,I): %f %f\n",
                           (*it)->getFinalEnergy(), (*it)->getBetaIntensity());
        }

        totalDecayIntensity += (*it)->getBetaIntensity();
        totalDecayIntensityUnc += (*it)->getPositronIntensityUnc() + (*it)->getECIntensityUnc();
    }
    for (vector<AlphaRecord *>::iterator alpha = myAlphaRecords.begin();
            alpha != myAlphaRecords.end(); alpha++) {

        if (verbose > 1) {
            egsInformation("EGS_Ensdf::normalizeIntensities: Alpha (E,I): %f %f\n",
                           (*alpha)->getFinalEnergy(), (*alpha)->getAlphaIntensity());
        }

        totalDecayIntensity += (*alpha)->getAlphaIntensity();
        totalDecayIntensityUnc += (*alpha)->getAlphaIntensityUnc();
    }
    for (vector<GammaRecord *>::iterator gamma = myMetastableGammaRecords.begin();
            gamma != myMetastableGammaRecords.end(); gamma++) {

        if (verbose > 1) {
            egsInformation("EGS_Ensdf::normalizeIntensities: MetastableGamma (I): %f\n",
                           (*gamma)->getTransitionIntensity());
        }

        totalDecayIntensity += (*gamma)->getTransitionIntensity();
        totalDecayIntensityUnc += (*gamma)->getGammaIntensityUnc() + (*gamma)->getICIntensityUnc() + (*gamma)->getIPIntensityUnc();
    }

    // Check that the branch probabilities add up to one
    double branchSum = 0;
    for (vector<NormalizationRecord * >::iterator norm =
                myNormalizationRecords.begin();
            norm!=myNormalizationRecords.end(); norm++) {
        branchSum += (*norm)->getBranchMultiplier();
    }
    // Currently there is only 1 case in the LNHB ensdf data where this is true
    // It is for Cf-252 fission events
    if (branchSum < 1-epsilon) {
        egsWarning("\nEGS_Ensdf::normalizeIntensities: Warning: The branching ratios of this nuclide add to less than 1 (%f). The leftover probability will be assigned to fission events. These events will return a zero energy particle and be counted as disintegrations. This is expected for Cf-252 in the LNHB collection.\n\n",branchSum);

        // Add the fission probability to the total decay intensity
        totalDecayIntensity /= branchSum;
    }
    else if (branchSum > 1+epsilon) {
        egsWarning("\nEGS_Ensdf::normalizeIntensities: Warning: The branching ratios of this nuclide add to greater than 1 (%f). This will result in overall emission rates being incorrect (e.g. number of emissions per 100 decays) when compared against the input.\n\n",branchSum);
    }

    // At this stage, totalDecayIntensity would ideally be 100, to represent
    // that all the decay probabilities added to 100%. However, this is not usually
    // achieved, due to uncertainties in the values. Thus we must adjust the numbers
    // somehow in order to model decays.
    // Option 1: Do nothing special, and the decays will get normalized to to force
    // them to add to 100%. This evenly spreads the discrepancy over all decays,
    // despite the fact that some of them might have higher uncertainties.
    // Option 2: Spread the discrepancy over all decays, scaled by the uncertainty
    // of each decay.

    // Here we do option 2:
    // Check that totalDecayIntensity != 100
    if (totalDecayIntensity > 100 + epsilon || totalDecayIntensity < 100 - epsilon) {

        // This is the discrepancy of the total decay intensity we found in the file,
        // that we will have to account for. This difference will be spread over
        // all of the decays, by adjusting the decay intensities proportional
        // to their uncertainty
        decayDiscrepancy = 100 - totalDecayIntensity;

        egsInformation("EGS_Ensdf::normalizeIntensities: Warning: The sum of the decay probabilities for this nuclide does not equal 100\%! In order for modeling to proceed, this must be accounted for. The discrepancy of %f has been distributed over all decays, proportional to the corresponding uncertainties. Note that this will also change the internal transition intensities, since they depend on the decays.\n", decayDiscrepancy);

        // Reset the disintegration intensities for each level, we will
        // need to recalculate this in order to adjust the transition
        // intensities after adjusting the decay intensities
        for (vector<LevelRecord * >::iterator it = myLevelRecords.begin();
                it!=myLevelRecords.end(); it++) {
            (*it)->resetDisintegrationIntensity();
        }

        for (vector<BetaMinusRecord * >::iterator it = myBetaMinusRecords.begin();
                it!=myBetaMinusRecords.end(); it++) {

            (*it)->setBetaIntensity((*it)->getBetaIntensity() + (*it)->getBetaIntensityUnc() / totalDecayIntensityUnc * decayDiscrepancy);

            // Add to the intensity toward this level
            (*it)->getLevelRecord()->cumulDisintegrationIntensity((*it)->getBetaIntensity());
        }

        for (vector<BetaPlusRecord * >::iterator it = myBetaPlusRecords.begin();
                it!=myBetaPlusRecords.end(); it++) {

            // The positron intensity is already normalized with the electron
            // capture intensity so they add to 1. So we multiply by the beta
            // intensity to get the correct units on each, and calculate
            // the new values
            double newPositronIntensity = (*it)->getBetaIntensity() * (*it)->getPositronIntensity() + (*it)->getPositronIntensityUnc() / totalDecayIntensityUnc * decayDiscrepancy;
            double newECIntensity = (*it)->getBetaIntensity() * (1-(*it)->getPositronIntensity()) + (*it)->getECIntensityUnc() / totalDecayIntensityUnc * decayDiscrepancy;

            // Set the total intensity for this decay based on the new values
            (*it)->setBetaIntensity(newPositronIntensity + newECIntensity);

            // Normalize the positron intensity for sampling (either positron
            // decay or EC occurs, so they add to 1)
            (*it)->setPositronIntensity(newPositronIntensity / (*it)->getBetaIntensity());

            // Add to the intensity toward this level
            (*it)->getLevelRecord()->cumulDisintegrationIntensity((*it)->getBetaIntensity());
        }

        for (vector<AlphaRecord * >::iterator it = myAlphaRecords.begin();
                it!=myAlphaRecords.end(); it++) {

            (*it)->setAlphaIntensity((*it)->getAlphaIntensity() + (*it)->getAlphaIntensityUnc() / totalDecayIntensityUnc * decayDiscrepancy);

            // Add to the intensity toward this level
            (*it)->getLevelRecord()->cumulDisintegrationIntensity((*it)->getAlphaIntensity());
        }

        for (vector<GammaRecord *>::iterator it = myMetastableGammaRecords.begin();
                it != myMetastableGammaRecords.end(); it++) {

            double icI = 0;
            double ipI = 0;
            if ((*it)->getICIntensity() > 0) {
                icI = (*it)->getGammaIntensity()*(1+(*it)->getICIntensity()) - (*it)->getGammaIntensity();
            }
            if ((*it)->getIPIntensity() > 0) {
                ipI = (*it)->getTransitionIntensity() - (*it)->getGammaIntensity() - ((*it)->getGammaIntensity()*(1+(*it)->getICIntensity()) - (*it)->getGammaIntensity());
            }

            // We need to calculate the original intensities, adjust
            // based on the uncertainty scaling, and normalize again
            double newGammaIntensity = (*it)->getGammaIntensity() + (*it)->getGammaIntensityUnc() / totalDecayIntensityUnc * decayDiscrepancy;
            double newICIntensity = icI + (*it)->getICIntensityUnc() / totalDecayIntensityUnc * decayDiscrepancy;
            double newIPIntensity = ipI + (*it)->getIPIntensityUnc() / totalDecayIntensityUnc * decayDiscrepancy;

            // Set the intensities for this decay based on the new values
            (*it)->setTransitionIntensity(newGammaIntensity + newICIntensity + newIPIntensity);
            (*it)->setGammaIntensity(newGammaIntensity);
            (*it)->setICIntensity((newICIntensity+(*it)->getGammaIntensity()) / (*it)->getGammaIntensity() - 1);

            // Add to the intensity toward this level
            (*it)->getLevelRecord()->cumulDisintegrationIntensity((*it)->getTransitionIntensity());
        }

        // For regular internal transitions there is no normalization needed
        for (vector<GammaRecord *>::iterator gamma = myGammaRecords.begin();
                gamma != myGammaRecords.end(); gamma++) {

            // Add the contribution of this transition *toward* a different level
            (*gamma)->getFinalLevel()->cumulDisintegrationIntensity((*gamma)->getTransitionIntensity());
        }

        // Print out a summary of the decays
        egsInformation("\nEGS_Ensdf::normalizeIntensities: Summary of %s decays (adjusted by %f):\n", radionuclide.c_str(), decayDiscrepancy);
        egsInformation("========================\n");
        egsInformation("Energy | Intensity per 100 decays\n");
        if (myBetaRecords.size()) {
            egsInformation("Beta records:\n");
            for (vector<BetaRecordLeaf *>::iterator beta = myBetaRecords.begin();
                    beta != myBetaRecords.end(); beta++) {
                egsInformation("%f %f\n", (*beta)->getFinalEnergy(), (*beta)->getBetaIntensity());
            }
        }
        if (myAlphaRecords.size()) {
            egsInformation("Alpha records:\n");
            for (vector<AlphaRecord *>::iterator alpha = myAlphaRecords.begin();
                    alpha != myAlphaRecords.end(); alpha++) {
                egsInformation("%f %f\n", (*alpha)->getFinalEnergy(), (*alpha)->getAlphaIntensity());
            }
        }
        if (myGammaRecords.size()) {
            egsInformation("Gamma records (E,Igamma,Ice,Ipp):\n");
            for (vector<GammaRecord *>::iterator gamma = myGammaRecords.begin();
                    gamma != myGammaRecords.end(); gamma++) {
                double icI = 0;
                double ipI = 0;
                if ((*gamma)->getICIntensity() > 0) {
                    icI = (*gamma)->getGammaIntensity()*(1+(*gamma)->getICIntensity()) - (*gamma)->getGammaIntensity();
                }
                if ((*gamma)->getIPIntensity() > 0) {
                    ipI = (*gamma)->getTransitionIntensity() - (*gamma)->getGammaIntensity() - icI;
                }
                egsInformation("%f %f %.4e %.4e\n", (*gamma)->getDecayEnergy(), (*gamma)->getGammaIntensity(), icI, ipI);
            }
        }
        if (myUncorrelatedGammaRecords.size()) {
            egsInformation("Uncorrelated gamma records (E,Igamma,Ice,Ipp):\n");
            for (vector<GammaRecord *>::iterator gamma = myUncorrelatedGammaRecords.begin();
                    gamma != myUncorrelatedGammaRecords.end(); gamma++) {
                double icI = 0;
                double ipI = 0;
                if ((*gamma)->getICIntensity() > 0) {
                    icI = (*gamma)->getGammaIntensity()*(1+(*gamma)->getICIntensity()) - (*gamma)->getGammaIntensity();
                }
                if ((*gamma)->getIPIntensity() > 0) {
                    ipI = (*gamma)->getTransitionIntensity() - (*gamma)->getGammaIntensity() - icI;
                }
                egsInformation("%f %f %.4e %.4e\n", (*gamma)->getDecayEnergy(), (*gamma)->getGammaIntensity(), icI, ipI);
            }
        }
        if (xrayEnergies.size() > 0) {
            egsInformation("X-Ray records:\n");
            for (unsigned int i=0; i < xrayEnergies.size(); ++i) {
                egsInformation("%f %f\n", xrayEnergies[i], xrayIntensities[i]);
            }
        }
        if (augerEnergies.size() > 0) {
            egsInformation("Auger records:\n");
            for (unsigned int i=0; i < augerEnergies.size(); ++i) {
                egsInformation("%f %f\n", augerEnergies[i], augerIntensities[i]);
            }
        }
        egsInformation("=== End of summary ===\n");

        // Now we have guaranteed that the decay probabilities add to 100%
        totalDecayIntensity = 100;
    }

    // Check for instances where the intensity of disintegrations that
    // lead towards a particular excited daughter level is less than
    // the intensities of gamma transitions from it.
    if (allowMultiTransition) {
        if (verbose) {
            egsInformation("EGS_Ensdf::normalizeIntensities: Comparing the cumulative disintegration intensity of each level with the gamma transition intensities... \n");
        }

        // Get the total transition intensity *away* from each level
        unsigned int j = 0;
        vector<double> totalLevelIntensity;
        totalLevelIntensity.resize(myLevelRecords.size());
        for (vector<LevelRecord * >::iterator it = myLevelRecords.begin();
                it!=myLevelRecords.end(); ++it) {

            totalLevelIntensity[j] = 0;
            for (vector<GammaRecord *>::iterator gamma = myGammaRecords.begin();
                    gamma != myGammaRecords.end(); gamma++) {

                if ((*gamma)->getLevelRecord() == (*it)) {
                    totalLevelIntensity[j] += (*gamma)->getTransitionIntensity();
                }
            }
            ++j;
        }

        // Iterate in reverse through the levels (highest level first)
        // This ensures we modify the intensities feeding the lower levels first
        j = myLevelRecords.size()-1;
        for (vector<LevelRecord * >::reverse_iterator it = myLevelRecords.rbegin();
                it!=myLevelRecords.rend(); ++it) {

            double disintIntensity = (*it)->getDisintegrationIntensity();

            if (verbose) {
                egsInformation("EGS_Ensdf::normalizeIntensities: (Level, ItoLevel, IfromLevel): %d %f %f\n", j, disintIntensity, totalLevelIntensity[j]);
            }

            // Notice that we don't do this if disintIntensity==0
            if (disintIntensity > epsilon && totalLevelIntensity[j] > disintIntensity + epsilon) {
                for (vector<GammaRecord *>::iterator gamma = myGammaRecords.begin();
                        gamma != myGammaRecords.end(); gamma++) {

                    if ((*gamma)->getLevelRecord() == (*it)) {
                        double multipleTransitionProb = (1.-disintIntensity/totalLevelIntensity[j]);

                        (*gamma)->setMultiTransitionProb(multipleTransitionProb);

                        if (verbose) {
                            egsInformation("EGS_Ensdf::normalizeIntensities: Multiple gamma transition probability (E,I): %f %f\n",(*gamma)->getDecayEnergy(), multipleTransitionProb);
                        }

                        // Reduce the transition intensities
                        (*gamma)->setTransitionIntensity(
                            (*gamma)->getTransitionIntensity() * disintIntensity/totalLevelIntensity[j]
                        );
                        (*gamma)->setGammaIntensity(
                            (*gamma)->getGammaIntensity() * disintIntensity/totalLevelIntensity[j]
                        );
                    }
                }
            }

            --j;
        }
    }

    for (vector<GammaRecord *>::iterator gamma = myUncorrelatedGammaRecords.begin();
            gamma != myUncorrelatedGammaRecords.end(); gamma++) {
        totalDecayIntensity += (*gamma)->getTransitionIntensity();
    }

    // Add X-rays and Auger electrons that are read from the ENSDF file
    // and going to be used instead of modelling correlated atomic relaxations.
    // These are modeled like independent decays, though they don't count
    // as disintegration events in the fluence.
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
                       "%f\n\n",totalDecayIntensity);
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

        if ((alpha - myAlphaRecords.begin()) == 0 && lastIntensity > epsilon) {
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

        if ((gamma - myMetastableGammaRecords.begin()) == 0 && lastIntensity > epsilon) {
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
            egsInformation("EGS_Ensdf::normalizeIntensities: MetastableGamma (I): %f\n",
                           (*gamma)->getTransitionIntensity());
        }
    }

    // Normalize uncorrelated internal transitions
    for (vector<GammaRecord *>::iterator gamma = myUncorrelatedGammaRecords.begin();
            gamma != myUncorrelatedGammaRecords.end(); gamma++) {
        (*gamma)->setTransitionIntensity(
            (*gamma)->getTransitionIntensity() / totalDecayIntensity);

        if ((gamma - myUncorrelatedGammaRecords.begin()) == 0 && lastIntensity > epsilon) {
            (*gamma)->setTransitionIntensity(
                (*gamma)->getTransitionIntensity() + lastIntensity);
        }
        else if ((gamma - myUncorrelatedGammaRecords.begin()) > 0) {
            (*gamma)->setTransitionIntensity(
                (*gamma)->getTransitionIntensity() +
                (*(gamma-1))->getTransitionIntensity());
        }
        lastIntensity = (*gamma)->getTransitionIntensity();

        if (verbose) {
            egsInformation("EGS_Ensdf::normalizeIntensities: UncorrelatedGamma (I): %f\n",
                           (*gamma)->getTransitionIntensity());
        }
    }

    // Normalize XRay emission intensities
    for (unsigned int i=0; i < xrayIntensities.size(); ++i) {

        xrayIntensities[i] /= totalDecayIntensity;

        if (i==0 && lastIntensity > epsilon) {
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

        if (i==0 && lastIntensity > epsilon) {
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

        if (verbose > 1) {
            egsInformation("EGS_Ensdf::normalizeIntensities: "
                           "totalLevelIntensity: %f\n", totalLevelIntensity[j]);
        }

        if (disintIntensity > epsilon && totalLevelIntensity[j] < disintIntensity + epsilon) {
            totalLevelIntensity[j] = disintIntensity;
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

                (*gamma)->setGammaIntensity(
                    (*gamma)->getGammaIntensity() /
                    (*gamma)->getTransitionIntensity());

                (*gamma)->setICIntensity(
                    (*gamma)->getGammaIntensity() * (1+(*gamma)->getICIntensity()));

                if (totalLevelIntensity[j] > epsilon) {
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
                                   "Gamma (level,E,I,Igamma,Ice): "
                                   "%d %f %f %f %f\n",
                                   j,(*gamma)->getDecayEnergy(), (*gamma)->getTransitionIntensity(),
                                   (*gamma)->getGammaIntensity(), (*gamma)->getICIntensity());
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

    bool containsEmissions = false;
    bool xrayContinues = false;
    bool augerContinues = false;
    bool gotTotal = false;
    vector<double>  multilineEnergies,
           multilineIntensities;
    double lineTotalIntensity = 0;
    unsigned int countNumAfterTotal = 0;
    int lineTotalType;

    for (vector<CommentRecord *>::iterator comment = myCommentRecords.begin();
            comment != myCommentRecords.end(); comment++) {

        auto commentSet = (*comment)->getComments();
        for (auto c = commentSet.begin(); c != commentSet.end(); ++c) {
            string line = *c;

            // Search for this line to ensure emissions will follow the right format
            if (line.find("{U Energy (keV)}   {U Intensity}  {U Line}") != std::string::npos) {
                containsEmissions = true;
            }

            if (containsEmissions) {
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
                                    if (*it < epsilon) {
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
                                    if (*it < epsilon) {
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
                            if (multilineIntensities[i] > epsilon) {
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
                    if (emissionLine.length() < 1 || (emissionLine.at(0) != 'X' && emissionLine.find("AUGER") == std::string::npos)) {
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
                    if (gotTotal && energy > epsilon) {
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
                            if ((energy > epsilon && intensity > epsilon) ||
                                    (gotTotal && energy > epsilon)) {
                                xrayEnergies.push_back(energy);
                                xrayIntensities.push_back(intensity);
                            }
                        }
                        else if (emissionLine.find("AUGER") != std::string::npos) {
                            if ((energy > epsilon && intensity > epsilon) ||
                                    (gotTotal && energy > epsilon)) {
                                augerEnergies.push_back(energy);
                                augerIntensities.push_back(intensity);
                            }
                        }
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

vector<GammaRecord * > EGS_Ensdf::getUncorrelatedGammaRecords() const {
    return myUncorrelatedGammaRecords;
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

// Returns the double between two indices, for the first string in the ensdf
// lines array. It is assumed that the characters in this range can be
// converted to a double
double Record::recordToDouble(int startPos, int endPos) {
    if (!lines.empty()) {
        if (lines.front().length() < startPos) {
            egsWarning("Record::recordToDouble: Warning: Record too short to "
                       "contain desired quantity\n");
            return 0;
        }
        string record = lines.front().substr(startPos-1,
                                             endPos-startPos+1);
        return atof(record.c_str());
    }
    else {
        egsWarning("Record::recordToDouble: Error: Record is empty\n");
        return 0;
    }
}

// Returns the string between two indices, for the first string in the ensdf
// lines array
string Record::recordToString(int startPos, int endPos) {
    if (!lines.empty()) {
        if (lines.front().length() < startPos) {
            egsWarning("Record::recordToString: Warning: Record too short to "
                       "contain desired quantity\n");
            return "";
        }

        return egsTrimString(lines.front().substr(startPos-1, endPos-startPos+1));
    }
    else {
        egsWarning("Record::recordToString: Error: Record is empty\n");
        return "";
    }
}

// Searches for a string in the ensdf array of lines for this record
// If the string is found, it returns the content between the end of the
// string and the next space, converting it to a double
//
// For example, for a line like this:
// 64NI2 G KC=1.112E-4  2$LC=1.09E-5   2
// If you set searchString='KC=', the return value would be 1.112E-4
//
// The notAfter string can be used to make sure the searchString is not
// preceeded by the notAfter string. This was necessary to match "PC=" but
// not "IPC="
double Record::getTag(string searchString, string notAfter="") {
    if (lines.size() > 1) {

        for (int i=1; i<lines.size(); ++i) {

            int tagPos = lines[i].find(searchString);

            if (tagPos != std::string::npos) {
                // Make sure that the string notAfter doesn't occur before
                // the search string
                size_t notAfterPos = std::string::npos;
                if (notAfter.length() > 0 && tagPos-notAfter.length() > 0) {
                    notAfterPos = lines[i].find(notAfter, tagPos-notAfter.length());
                }

                // If the "notAfter" string wasn't found, proceed and get the
                // data for the matched tag
                if (notAfterPos == std::string::npos || notAfterPos > tagPos) {
                    tagPos += searchString.length();

                    string record = lines[i].substr(tagPos, lines[i].find(" ",tagPos)-tagPos);

                    return atof(record.c_str());
                }
                else {
                    // The tag we are looking for could still exist, even though
                    // the first match failed. Look again, this time without
                    // worrying about the notAfter string because it was already
                    // found and is assumed to only exist once
                    tagPos = lines[i].find(searchString, tagPos+searchString.length());

                    if (tagPos != std::string::npos) {
                        tagPos += searchString.length();

                        string record = lines[i].substr(tagPos, lines[i].find(" ",tagPos)-tagPos);

                        return atof(record.c_str());
                    }
                }
            }
        }
    }
    return 0;
}

// Converts uncertainties in standard format into a double
// For example, 1.23E-4 (67) would be input as value='1.23E-4', and
// stdUncertainty='67'. The return value would be 0.67E-4.
double Record::parseStdUncertainty(string value, string stdUncertainty) {
    if (stdUncertainty.length() < 1) {
        return 0;
    }
    if (value.length() < 1) {
        egsInformation("Record::parseStdUncertainty: Warning: No uncertainty provided! Returning 0 uncertainty for value of %f\n", value.c_str());
        return 0;
    }

    if (stdUncertainty.length() > value.length()) {
        egsInformation("Record::parseStdUncertainty: Warning: Number of digits in uncertainty greater than number of digits in value. Returning 0 uncertainty for value of %f\n", value.c_str());
        return 0;
    }

    // Determine if there is an E for scientific notation and where it is
    size_t sciNotLoc = value.find_last_of('E');
    size_t dotLoc = value.find_last_of('.');

    // Loop backwards through value, starting at the 'E' if there is one
    int startPos;
    if (sciNotLoc != std::string::npos) {
        startPos = sciNotLoc-1;
    }
    else {
        startPos = value.length()-1;
    }
    int j;
    if (stdUncertainty.length() == 2) {
        j = 1;
    }
    else {
        j = 0;
    }
    for (int i = startPos; i >= 0; --i) {
        if (i != dotLoc) {
            if (j>=0) {
                value[i] = stdUncertainty[j--];
            }
            else {
                value[i] = '0';
            }
        }
    }

    return atof(value.c_str());
}

string Record::getStringAfter(string searchString, size_t len) {
    if (lines.size() > 1) {

        for (int i=1; i<lines.size(); ++i) {

            int tagPos = lines[i].find(searchString);

            if (tagPos != std::string::npos) {

                tagPos += searchString.length();

                string record = lines[i].substr(tagPos, len);

                return record;
            }
        }
    }
    return "";
}

// Parse a halflife from a record
// Converts the units to seconds
// Returns the halflife, or a negative number upon failure
double Record::parseHalfLife(int startPos, int endPos) {
    if (lines.empty()) {
        egsWarning("Record::parseHalfLife: Error: Record is empty\n");
        return -5;
    }
    if (lines.front().length() < startPos) {
        egsWarning("Record::parseHalfLife: Warning: Record too short to "
                   "contain desired quantity\n");
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

// Parse the spin out of the spin parity text
unsigned short Record::parseSpin(int startPos, int endPos) {
    if (lines.empty()) {
        egsWarning("Record::parseSpin: Error: Record is empty\n");
        return -5;
    }
    if (lines.front().length() < startPos) {
        egsWarning("Record::parseSpin: Warning: Record too short to "
                   "contain desired quantity\n");
        return -5;
    }

    string spinParityStr = egsTrimString(lines.front().substr(startPos-1,
                                       endPos-startPos+1));

    size_t digitIndex = -1;
    for (auto i = 0; i < spinParityStr.length(); i++) {
        if (isdigit(spinParityStr[i])) {
            digitIndex = i;
            break;
        }
    }

    unsigned short spin = spinParityStr[digitIndex] - '0';

    return spin;
}

// Parse the parity out of the spin parity text
// 0 (false) is negative, 1 (true) is positive
bool Record::parseParity(int startPos, int endPos) {
    if (lines.empty()) {
        egsWarning("Record::parseSpin: Error: Record is empty\n");
        return -5;
    }
    if (lines.front().length() < startPos) {
        egsWarning("Record::parseSpin: Warning: Record too short to "
                   "contain desired quantity\n");
        return -5;
    }

    string spinParityStr = egsTrimString(lines.front().substr(startPos-1,
                                       endPos-startPos+1));

    size_t digitIndex = -1;
    for (auto i = 0; i < spinParityStr.length(); i++) {
        if(spinParityStr[i] == '-' || spinParityStr[i] == '+') {
            digitIndex = i;
            break;
        }
    }

    bool parity;
    if(digitIndex < 0 || spinParityStr[digitIndex] == '+') {
        parity = true;
    } else {
        parity = false;
    }

    return parity;
}


// Comment Record
CommentRecord::CommentRecord(vector<string> ensdf):Record(ensdf) {
    processEnsdf();
}

void CommentRecord::processEnsdf() {
    if (!lines.empty()) {

        comments = lines;
    }
}

vector<string> CommentRecord::getComments() {
    return comments;
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

    // If the normalization is not specified, it will get initialized to zero
    // Change this to 1
    if (normalizeRelative < epsilon) {
        normalizeRelative = 1;
    }
    if (normalizeTransition < epsilon) {
        normalizeTransition = 1;
    }
    if (normalizeBranch < epsilon) {
        normalizeBranch = 1;
    }
    if (normalizeBeta < epsilon) {
        normalizeBeta = 1;
    }

    // Get the daughter element
    string element = recordToString(4, 5);

    // Get the Z
    Z = setZ(element);

    // Load atomic relaxations
    relaxations = new EGS_AtomicRelaxations();
    relaxations->loadData(Z);

    // Get the number of shells
    nshell = relaxations->getNShell(Z);

    egsInformation("NormalizationRecord::processEnsdf(): Z, nshell: %d %d\n",Z,nshell);
}

EGS_AtomicRelaxations *NormalizationRecord::getRelaxations() const {
    return relaxations;
}

int NormalizationRecord::getNShell() const {
    return nshell;
}

double NormalizationRecord::getBindingEnergy(int shell) const {
    return relaxations->getBindingEnergy(Z,shell);
}

void NormalizationRecord::relax(int shell,
                                EGS_Float ecut, EGS_Float pcut,
                                EGS_RandomGenerator *rndm, double &edep,
                                EGS_SimpleContainer<EGS_RelaxationParticle> &particles) {
    relaxations->relax(Z,shell,ecut,pcut,rndm,edep,particles);
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
    spin = parseSpin(22, 39);
    parity = parseParity(22, 39);
}

void LevelRecord::setLevelCanDecay(bool canDecayTmp) {
    canDecay = canDecayTmp;
}

bool LevelRecord::levelCanDecay() const {
    return canDecay;
}

void LevelRecord::resetDisintegrationIntensity() {
    disintegrationIntensity = 0;
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

unsigned short LevelRecord::getSpin() const {
    return spin;
}

bool LevelRecord::getParity() const {
    return parity;
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
    Record(ensdf),
    ParentRecordLeaf(myParent),
    NormalizationRecordLeaf(myNormalization),
    LevelRecordLeaf(myLevel) {

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
    if (lines.front().length() > 77) {
        string lambda;
        lambda.push_back(lines.front().at(77));
        forbidden = atoi(lambda.c_str());
    }
    else {
        forbidden = 0;
    }
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
    string betaIntensityStr = recordToString(22, 29);
    string betaIntensityUncStr = recordToString(30, 31);

    betaIntensityUnc = parseStdUncertainty(betaIntensityStr, betaIntensityUncStr);
    // If the uncertainty is 0 (i.e. not specified), set it to 100%
    if (betaIntensityUnc == 0) {
        betaIntensityUnc = betaIntensity;
    }

    if (getNormalizationRecord()) {
        double factor = getNormalizationRecord()->getBetaMultiplier() * getNormalizationRecord()->getBranchMultiplier();

        betaIntensity *= factor;
        betaIntensityUnc *= factor;
    }
}

double BetaMinusRecord::getFinalEnergy() const {
    return finalEnergy;
}

double BetaMinusRecord::getBetaIntensity() const {
    return betaIntensity;
}

double BetaMinusRecord::getBetaIntensityUnc() const {
    return betaIntensityUnc;
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
    string positronIntensityStr = recordToString(22, 29);
    string positronIntensityUncStr = recordToString(30, 31);
    ecIntensity = recordToDouble(32, 39);
    string ecIntensityStr = recordToString(32, 39);
    string ecIntensityUncStr = recordToString(40, 41);

    positronIntensityUnc = parseStdUncertainty(positronIntensityStr, positronIntensityUncStr);
    // If the uncertainty is 0 (i.e. not specified), set it to 100%
    if (positronIntensityUnc == 0) {
        positronIntensityUnc = positronIntensity;
    }

    ecIntensityUnc = parseStdUncertainty(ecIntensityStr, ecIntensityUncStr);
    // If the uncertainty is 0 (i.e. not specified), set it to 100%
    if (ecIntensityUnc == 0) {
        ecIntensityUnc = ecIntensity;
    }

    if (getNormalizationRecord()) {
        double factor = getNormalizationRecord()->getBetaMultiplier() * getNormalizationRecord()->getBranchMultiplier();

        positronIntensity *= factor;
        ecIntensity *= factor;
        positronIntensityUnc *= factor;
        ecIntensityUnc *= factor;
    }

    // The total intensity for this decay branch
    // A decay down this branch will then be split between positron or EC
    betaIntensity = positronIntensity + ecIntensity;

    // Re-normalize the positron intensity so that
    // positronIntensity + ecIntensity = 1, for easy sampling
    // We only set positronIntensity because that's all we need
    positronIntensity = positronIntensity / betaIntensity;

    // For positrons we may need to calculate the emission energy
    // E = Q - level_energy - 2*mc^2
    if (finalEnergy == 0 && positronIntensity > epsilon) {
        finalEnergy = getParentRecord()->getQ()
                      - getLevelRecord()->getEnergy() - 1.022;

        if (finalEnergy < 0.) {
            egsWarning("BetaPlusRecord::processEnsdf: Error: Final energy of "
                       "positron could not be calculated. Setting energy to zero!\n"
                      );
            finalEnergy = 0.;
        }
    }

    if (ecIntensity > 0 && getNormalizationRecord()) {
        // Get the number of shells
        int nshell = getNormalizationRecord()->getNShell();

        double icK = getTag("CK=");
        double icL = getTag("CL=");
        double icM = getTag("CM=");
        double icN = getTag("CN=");
        double icO = getTag("CO=");
        double icP = getTag("CP=");
        double icQ = getTag("CQ=");

        // The K shell
        ecShellIntensity.push_back(icK);

        // The L1, L2, L3 shells
        // TODO: Equal probability is assigned to subshells!
        //       This assumption is an approximation
        int numShellsToInclude = min(4,nshell);
        for (unsigned int i=1; i<numShellsToInclude; ++i) {
            ecShellIntensity.push_back(ecShellIntensity.back() + icL/(numShellsToInclude-1));
        }
        // Count number of shells as we go
        // Once we hit the number of shells for this element, return
        if (numShellsToInclude < 4) {
//             for (int i=0; i<ecShellIntensity.size(); ++i) {
//                 egsInformation("BetaPlusRecord::processEnsdf: Shell %d: P=%f\n",i,ecShellIntensity[i]);
//             }
            return;
        }

        // The M1-M5 shells
        numShellsToInclude = min(9,nshell);
        for (unsigned int i=4; i<numShellsToInclude; ++i) {
            ecShellIntensity.push_back(ecShellIntensity.back() + icM/(numShellsToInclude-4));
        }
        if (numShellsToInclude < 9) {
//             for (int i=0; i<ecShellIntensity.size(); ++i) {
//                 egsInformation("BetaPlusRecord::processEnsdf: Shell %d: P=%f\n",i,ecShellIntensity[i]);
//             }
            return;
        }

        // The N1-7 shells
        numShellsToInclude = min(16,nshell);
        for (unsigned int i=9; i<numShellsToInclude; ++i) {
            ecShellIntensity.push_back(ecShellIntensity.back() + icN/(numShellsToInclude-9));
        }
        if (numShellsToInclude < 16) {
//             for (int i=0; i<ecShellIntensity.size(); ++i) {
//                 egsInformation("BetaPlusRecord::processEnsdf: Shell %d: P=%f\n",i,ecShellIntensity[i]);
//             }
            return;
        }

        // The O1-7 shells
        numShellsToInclude = min(23,nshell);
        for (unsigned int i=16; i<numShellsToInclude; ++i) {
            ecShellIntensity.push_back(ecShellIntensity.back() + icO/(numShellsToInclude-16));
        }

        if (numShellsToInclude < 23) {
//             for (int i=0; i<ecShellIntensity.size(); ++i) {
//                 egsInformation("BetaPlusRecord::processEnsdf: Shell %d: P=%f\n",i,ecShellIntensity[i]);
//             }
            return;
        }

        // The P1-3 shells
        numShellsToInclude = min(26,nshell);
        for (unsigned int i=23; i<numShellsToInclude; ++i) {
            ecShellIntensity.push_back(ecShellIntensity.back() + icP/(numShellsToInclude-23));
        }

        if (numShellsToInclude < 26) {
//             for (int i=0; i<ecShellIntensity.size(); ++i) {
//                 egsInformation("BetaPlusRecord::processEnsdf: Shell %d: P=%f\n",i,ecShellIntensity[i]);
//             }
            return;
        }

        // The Q1 shell
        numShellsToInclude = 27;
        ecShellIntensity.push_back(ecShellIntensity.back() + icQ/(numShellsToInclude-26));

//         for (int i=0; i<ecShellIntensity.size(); ++i) {
//             egsInformation("BetaPlusRecord::processEnsdf: Shell %d: P=%f\n",i,ecShellIntensity[i]);
//         }
        return;
    }
}

void BetaPlusRecord::relax(int shell,
                           EGS_Float ecut, EGS_Float pcut,
                           EGS_RandomGenerator *rndm, double &edep,
                           EGS_SimpleContainer<EGS_RelaxationParticle> &particles) {
    getNormalizationRecord()->relax(shell,ecut,pcut,rndm,edep,particles);
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

double BetaPlusRecord::getPositronIntensityUnc() const {
    return positronIntensityUnc;
}

double BetaPlusRecord::getECIntensityUnc() const {
    return ecIntensityUnc;
}

void BetaPlusRecord::setBetaIntensity(double newIntensity) {
    betaIntensity = newIntensity;
}

void BetaPlusRecord::setPositronIntensity(double newIntensity) {
    positronIntensity = newIntensity;
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
    numGammaSampled = 0;
    numICSampled = 0;
    numIPSampled = 0;
    multipleTransitionProb = 0;
}

GammaRecord::GammaRecord(GammaRecord *gamma):
    Record(),
    ParentRecordLeaf(gamma->getParentRecord()),
    NormalizationRecordLeaf(gamma->getNormalizationRecord()),
    LevelRecordLeaf(gamma->getLevelRecord()) {

    numGammaSampled = gamma->numGammaSampled;
    numICSampled = gamma->numICSampled;
    numIPSampled = gamma->numIPSampled;
    decayEnergy = gamma->decayEnergy;
    transitionIntensity = gamma->transitionIntensity;
    multipleTransitionProb = gamma->multipleTransitionProb;
    gammaIntensity = gamma->gammaIntensity;
    gammaIntensityUnc = gamma->gammaIntensityUnc;
    icCoeff = gamma->icCoeff;
    icCoeffUnc = gamma->icCoeffUnc;
    ipCoeff = gamma->ipCoeff;
    ipCoeffUnc = gamma->ipCoeffUnc;
    q = gamma->q;
    finalLevel = gamma->finalLevel;
}

void GammaRecord::processEnsdf() {
    decayEnergy = recordToDouble(10, 19) / 1000.; // Convert keV to MeV
    gammaIntensity = recordToDouble(22, 29);
    string gammaIntensityStr = recordToString(22, 29);
    string gammaIntensityUncStr = recordToString(30, 31);

    // Get the internal conversion coefficient
    // Note: This might not equal the sum of the individual shell intensities,
    // because some of the shell intensities might not be known
    icCoeff = recordToDouble(56, 62);
    string icCoeffStr = recordToString(56, 62);
    string icCoeffUncStr = recordToString(63, 64);

    // If we don't find the gamma intensity, check for the first RI=
    if (gammaIntensity < epsilon) {
        gammaIntensity = getTag("RI=");
    }

    icCoeffUnc = parseStdUncertainty(icCoeffStr, icCoeffUncStr);
    // If the uncertainty is 0 (i.e. not specified), set it to 100%
    if (icCoeffUnc == 0) {
        icCoeffUnc = icCoeff;
    }

    // Check for internal pair production
    // The value and uncertainty are stored in 11 characters after IPC=
    string ipCoeffStr_tmp = getStringAfter("IPC=", 11);
    if (ipCoeffStr_tmp.length() > 0) {
        string ipCoeffStr = ipCoeffStr_tmp.substr(0, 9);
        string ipCoeffUncStr = ipCoeffStr_tmp.substr(9, 2);
        ipCoeff = atof(ipCoeffStr.c_str());
        ipCoeffUnc = atof(ipCoeffUncStr.c_str());

        if (ipCoeffUnc == 0) {
            ipCoeffUnc = ipCoeff;
        }
    }
    else {
        ipCoeff = 0;
        ipCoeffUnc = 0;
    }

    // Get the transition intensity instead if gamma still zero
    if (gammaIntensity < epsilon) {
        double ti = getTag("TI        ");
        // Calculate the gamma intensity from it
        gammaIntensity = ti / ((1+icCoeff) * (1+ipCoeff));
    }

    // Set the uncertainty on the gamma intensity
    gammaIntensityUnc = parseStdUncertainty(gammaIntensityStr, gammaIntensityUncStr);
    // If the uncertainty is 0 (i.e. not specified), set it to 100%
    if (gammaIntensityUnc == 0) {
        gammaIntensityUnc = gammaIntensity;
    }

    if (getNormalizationRecord()) {
        double factor = getNormalizationRecord()->getRelativeMultiplier() *
                        getNormalizationRecord()->getBranchMultiplier();

        gammaIntensity *= factor;
        gammaIntensityUnc *= factor;
    }

    // Calculate the total transition intensity
    transitionIntensity = gammaIntensity * (1+icCoeff) * (1+ipCoeff);

    if (icCoeff > 0 && getNormalizationRecord()) {
        // Get the number of shells
        int nshell = getNormalizationRecord()->getNShell();

        double icK = getTag("KC=");
        double icL = getTag("LC=");
        double icM = getTag("MC=");
        double icN = getTag("NC=");
        double icO = getTag("OC=");
        double icP = getTag("PC=", "I"); // Make sure "IPC" doesn't get matched
        double icQ = getTag("QC=");

        // The K shell
        icIntensity.push_back(icK / icCoeff);

        // The L1, L2, L3 shells
        // TODO: Equal probability is assigned to subshells!
        //       This assumption is an approximation
        int numShellsToInclude = min(4,nshell);
        for (unsigned int i=1; i<numShellsToInclude; ++i) {
            icIntensity.push_back(icIntensity.back() + (icL / icCoeff)/(numShellsToInclude-1));
        }
        // Count number of shells as we go
        // Once we hit the number of shells for this element, return
        if (numShellsToInclude < 4) {
//             for (int i=0; i<icIntensity.size(); ++i) {
//                 egsInformation("GammaRecord::processEnsdf: Shell %d: P=%f\n",i,icIntensity[i]);
//             }
            return;
        }

        // The M1-M5 shells
        numShellsToInclude = min(9,nshell);
        for (unsigned int i=4; i<numShellsToInclude; ++i) {
            icIntensity.push_back(icIntensity.back() + (icM / icCoeff)/(numShellsToInclude-4));
        }
        if (numShellsToInclude < 9) {
//             for (int i=0; i<icIntensity.size(); ++i) {
//                 egsInformation("GammaRecord::processEnsdf: Shell %d: P=%f\n",i,icIntensity[i]);
//             }
            return;
        }

        // The N1-7 shells
        numShellsToInclude = min(16,nshell);
        for (unsigned int i=9; i<numShellsToInclude; ++i) {
            icIntensity.push_back(icIntensity.back() + (icN / icCoeff)/(numShellsToInclude-9));
        }
        if (numShellsToInclude < 16) {
//             for (int i=0; i<icIntensity.size(); ++i) {
//                 egsInformation("GammaRecord::processEnsdf: Shell %d: P=%f\n",i,icIntensity[i]);
//             }
            return;
        }

        // The O1-7 shells
        numShellsToInclude = min(23,nshell);
        for (unsigned int i=16; i<numShellsToInclude; ++i) {
            icIntensity.push_back(icIntensity.back() + (icO / icCoeff)/(numShellsToInclude-16));
        }

        if (numShellsToInclude < 23) {
//             for (int i=0; i<icIntensity.size(); ++i) {
//                 egsInformation("GammaRecord::processEnsdf: Shell %d: P=%f\n",i,icIntensity[i]);
//             }
            return;
        }

        // The P1-3 shells
        numShellsToInclude = min(26,nshell);
        for (unsigned int i=23; i<numShellsToInclude; ++i) {
            icIntensity.push_back(icIntensity.back() + (icP / icCoeff)/(numShellsToInclude-23));
        }

        if (numShellsToInclude < 26) {
//             for (int i=0; i<icIntensity.size(); ++i) {
//                 egsInformation("GammaRecord::processEnsdf: Shell %d: P=%f\n",i,icIntensity[i]);
//             }
            return;
        }

        // The Q1 shell
        numShellsToInclude = 27;
        icIntensity.push_back(icIntensity.back() + (icQ / icCoeff)/(numShellsToInclude-26));

//         for (int i=0; i<icIntensity.size(); ++i) {
//             egsInformation("GammaRecord::processEnsdf: Shell %d: P=%f\n",i,icIntensity[i]);
//         }
        return;

    }
}

double GammaRecord::getBindingEnergy(int shell) const {
    return getNormalizationRecord()->getBindingEnergy(shell);
}

void GammaRecord::relax(int shell,
                        EGS_Float ecut, EGS_Float pcut,
                        EGS_RandomGenerator *rndm, double &edep,
                        EGS_SimpleContainer<EGS_RelaxationParticle> &particles) {
    getNormalizationRecord()->relax(shell,ecut,pcut,rndm,edep,particles);
}

double GammaRecord::getDecayEnergy() const {
    return decayEnergy;
}

double GammaRecord::getMultiTransitionProb() const {
    return multipleTransitionProb;
}

void GammaRecord::setMultiTransitionProb(double newIntensity) {
    multipleTransitionProb = newIntensity;
}

double GammaRecord::getTransitionIntensity() const {
    return transitionIntensity;
}

double GammaRecord::getGammaIntensity() const {
    return gammaIntensity;
}

double GammaRecord::getGammaIntensityUnc() const {
    return gammaIntensityUnc;
}

double GammaRecord::getICIntensity() const {
    return icCoeff;
}

double GammaRecord::getICIntensityUnc() const {
    return icCoeffUnc;
}

double GammaRecord::getIPIntensity() const {
    return ipCoeff;
}

double GammaRecord::getIPIntensityUnc() const {
    return ipCoeffUnc;
}

void GammaRecord::setTransitionIntensity(double newIntensity) {
    transitionIntensity = newIntensity;
}

void GammaRecord::setGammaIntensity(double newIntensity) {
    gammaIntensity = newIntensity;
}

void GammaRecord::setICIntensity(double newIntensity) {
    icCoeff = newIntensity;
}

int GammaRecord::getCharge() const {
    return q;
}

void GammaRecord::incrGammaSampled() {
    numGammaSampled++;
}

void GammaRecord::incrICSampled() {
    numICSampled++;
}

void GammaRecord::incrIPSampled() {
    numIPSampled++;
}

EGS_I64 GammaRecord::getGammaSampled() const {
    return numGammaSampled;
}

EGS_I64 GammaRecord::getICSampled() const {
    return numICSampled;
}

EGS_I64 GammaRecord::getIPSampled() const {
    return numIPSampled;
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

    string alphaIntensityStr = recordToString(22, 29);
    string alphaIntensityUncStr = recordToString(30, 31);

    alphaIntensityUnc = parseStdUncertainty(alphaIntensityStr, alphaIntensityUncStr);
    // If the uncertainty is 0 (i.e. not specified), set it to 100%
    if (alphaIntensityUnc == 0) {
        alphaIntensityUnc = alphaIntensity;
    }

    if (getNormalizationRecord()) {
        alphaIntensity *= getNormalizationRecord()->getBranchMultiplier();
        alphaIntensityUnc *= getNormalizationRecord()->getBranchMultiplier();
    }
}

double AlphaRecord::getFinalEnergy() const {
    return finalEnergy;
}

double AlphaRecord::getAlphaIntensity() const {
    return alphaIntensity;
}

double AlphaRecord::getAlphaIntensityUnc() const {
    return alphaIntensityUnc;
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

