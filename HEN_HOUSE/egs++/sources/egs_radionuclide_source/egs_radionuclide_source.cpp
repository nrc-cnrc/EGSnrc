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
#  Contributors:
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
        EGS_ObjectFactory *f) : EGS_BaseSource(input,f), shape(0),
    geom(0), regions(0), nrs(0), min_theta(0), max_theta(M_PI),
    min_phi(0), max_phi(2*M_PI), gc(IncludeAll), q_allowed(0), decays(0),
    activity(0) {

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
    Emax = 0;
    unsigned int i = 0;
    EGS_Float spectrumWeightTotal = 0;
    disintegrationOccurred = true;
    ishower = -1;
    time = 0;
    while (input->getInputItem("spectrum")) {

        egsInformation("**********************************************\n");

        decays.push_back(EGS_BaseSpectrum::createSpectrum(input));

        // If spectrum creation failed skip to the next spectrum block
        if (!decays[i]) {
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
        egsWarning("EGS_RadionuclideSource: Error: No spectrum was defined\n");
        return;
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

    // Calculate the duration of the experiment
    // Based on ncase and activity
    app = EGS_Application::activeApplication();
    EGS_Input *inp = app->getInput();
    EGS_Input *irc = 0;
    double ncase_double = 0;
    if (inp) {
        irc = inp->getInputItem("run control");

        EGS_Input *icontrol = irc->getInputItem("run control");
        if (!icontrol) {
            egsWarning("EGS_RadionuclideSource: no 'run control' "
                       "input to determine 'ncase'\n");
        }

        err = icontrol->getInput("number of histories", ncase_double);
        if (err) {
            err = icontrol->getInput("ncase", ncase_double);
            if (err) {
                egsWarning("EGS_RadionuclideSource: missing/wrong 'ncase' or "
                           "'number of histories' input\n");
            }
        }
    }
    else {
        egsWarning("EGS_RadionuclideSource: no 'run control' "
                   "input to determine 'ncase'\n");
    }

    // Create the shape for source emissions
    vector<EGS_Float> pos;
    EGS_Input *ishape = input->takeInputItem("shape");
    if (ishape) {
        shape = EGS_BaseShape::createShape(ishape);
        delete ishape;
    }
    if (!shape) {
        string sname;
        err = input->getInput("shape name",sname);
        if (err)
            egsWarning("EGS_RadionuclideSource: missing/wrong inline shape "
                       "definition and missing wrong 'shape name' input\n");
        else {
            shape = EGS_BaseShape::getShape(sname);
            if (!shape) egsWarning("EGS_RadionuclideSource: a shape named %s"
                                       " does not exist\n");
        }
    }
    string geom_name;
    err = input->getInput("geometry",geom_name);
    if (!err) {
        geom = EGS_BaseGeometry::getGeometry(geom_name);
        if (!geom) egsWarning("EGS_RadionuclideSource: no geometry named %s\n",
                                  geom_name.c_str());
        else {
            vector<string> reg_options;
            reg_options.push_back("IncludeAll");
            reg_options.push_back("ExcludeAll");
            reg_options.push_back("IncludeSelected");
            reg_options.push_back("ExcludeSelected");
            gc = (GeometryConfinement) input->getInput("region "
                    "selection",reg_options,0);
            if (gc == IncludeSelected || gc == ExcludeSelected) {
                vector<int> regs;
                err = input->getInput("selected regions",regs);
                if (err || regs.size() < 1) {
                    egsWarning("EGS_RadionuclideSource: region selection %d "
                               "used  but no 'selected regions' input "
                               "found\n",gc);
                    gc = gc == IncludeSelected ? IncludeAll : ExcludeAll;
                    egsWarning(" using %d\n",gc);
                }
                nrs = regs.size();
                regions = new int [nrs];
                for (int j=0; j<nrs; j++) {
                    regions[j] = regs[j];
                }
            }
        }
    }
    EGS_Float tmp_theta;
    err = input->getInput("min theta", tmp_theta);
    if (!err) {
        min_theta = tmp_theta/180.0*M_PI;
    }

    err = input->getInput("max theta", tmp_theta);
    if (!err) {
        max_theta = tmp_theta/180.0*M_PI;
    }

    err = input->getInput("min phi", tmp_theta);
    if (!err) {
        min_phi = tmp_theta/180.0*M_PI;
    }

    err = input->getInput("max phi", tmp_theta);
    if (!err) {
        max_phi = tmp_theta/180.0*M_PI;
    }

    buf_1 = cos(min_theta);
    buf_2 = cos(max_theta);

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

    EGS_I64 ishowerOld = decays[i]->getShowerIndex();

    for (EGS_I64 j=0; j<=1e6; ++j) {

        E = decays[i]->sampleEnergy(rndm);

        // Skip zero energy particles
        if (E < epsilon) {
            continue;
        }

        q = decays[i]->getCharge();

        // Check if the charge is allowed
        // If so, break out of the loop and keep the particle
        // Otherwise the loop will continue generating particles until
        // one matches the q_allowed criteria
        if (q_allowAll || std::find(q_allowed.begin(), q_allowed.end(), q) != q_allowed.end()) {
            break;
        }

        if (j == 1e6) {
            egsWarning("EGS_RadionuclideSource::getNextParticle: Error: Could not generate a particle after 1e6 tries. Spectrum will be wrong.\n");
            E = 0;
        }
    }

    EGS_I64 ishowerNew = decays[i]->getShowerIndex();
    if (ishowerNew > ishowerOld) {
        disintegrationOccurred = true;

        time += -log(1.-rndm->getUniform()) / activity * (ishowerNew - ishowerOld);
        ishower += (ishowerNew - ishowerOld);
    }
    else {
        disintegrationOccurred = false;
        time += decays[i]->getTime();
    }

    getPositionDirection(rndm,x,u,wt);
    latch = 0;

    // Now that we have the position of the particle, we can
    // find the region and deposit any sub-threshold contributions locally
    // This includes edep from relaxations, and alpha particle energy
    EGS_Float edep = decays[i]->getEdep();
    if (edep > 0) {
        app->setEdep(edep);
        int ireg = geom->isWhere(x);
        app->userScoring(3, ireg);
    }

    return ++count;
}

void EGS_RadionuclideSource::getPositionDirection(EGS_RandomGenerator *rndm,
        EGS_Vector &x, EGS_Vector &u, EGS_Float &wt) {

    bool ok = true;
    do {
        x = shape->getRandomPoint(rndm);
        if (geom) {
            if (gc == IncludeAll) {
                ok = geom->isInside(x);
            }
            else if (gc == ExcludeAll) {
                ok = !geom->isInside(x);
            }
            else if (gc == IncludeSelected) {
                ok = false;
                int ireg = geom->isWhere(x);
                for (int j=0; j<nrs; ++j) {
                    if (ireg == regions[j]) {
                        ok = true;
                        break;
                    }
                }
            }
            else {
                ok = true;
                int ireg = geom->isWhere(x);
                for (int j=0; j<nrs; ++j) {
                    if (ireg == regions[j]) {
                        ok = false;
                        break;
                    }
                }
            }
        }
    }
    while (!ok);

    u.z = rndm->getUniform()*(buf_1 - buf_2) - buf_1;

    EGS_Float sinz = 1-u.z*u.z;
    if (sinz > epsilon) {
        sinz = sqrt(sinz);
        EGS_Float cphi, sphi;
        EGS_Float phi = min_phi +(max_phi - min_phi)*rndm->getUniform();
        cphi = cos(phi);
        sphi = sin(phi);
        u.x = sinz*cphi;
        u.y = sinz*sphi;
    }
    else {
        u.x = 0;
        u.y = 0;
    }
    wt = 1;
}

void EGS_RadionuclideSource::setUp() {
    otype = "EGS_RadionuclideSource";
    if (!isValid()) {
        description = "Invalid radionuclide source";
    }
    else {
        description = "Radionuclide source from a shape of type ";
        description += shape->getObjectType();
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

        if (geom) {
            geom->ref();
        }
    }
}

bool EGS_RadionuclideSource::storeState(ostream &data_out) const {
    for (unsigned int i=0; i<decays.size(); ++i) {
        if (!decays[i]->storeState(data_out)) {
            return false;
        }
    }
    return true;
}

bool EGS_RadionuclideSource::addState(istream &data) {
    for (unsigned int i=0; i<decays.size(); ++i) {
        if (!decays[i]->setState(data)) {
            return false;
        }
        ishower += decays[i]->getShowerIndex();
    }
    return true;
}

void EGS_RadionuclideSource::resetCounter() {
    for (unsigned int i=0; i<decays.size(); ++i) {
        decays[i]->resetCounter();
    }
    ishower = 0;
    count = 0;
}

bool EGS_RadionuclideSource::setState(istream &data) {
    for (unsigned int i=0; i<decays.size(); ++i) {
        if (!decays[i]->setState(data)) {
            return false;
        }
    }
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
