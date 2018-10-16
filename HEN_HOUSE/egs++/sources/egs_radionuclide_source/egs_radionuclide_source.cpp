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
        EGS_ObjectFactory *f) : EGS_BaseSource(input,f),
    sourceType(""), shape(0),
    geom(0), regions(0), nrs(0), min_theta(0), max_theta(M_PI),
    min_phi(0), max_phi(2*M_PI), gc(IncludeAll),
    source_shape(0), target_shape(0), ctry(0), dist(1),
    q_allowed(0), decays(0), activity(1) {

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
        egsWarning("\nEGS_RadionuclideSource: Error: No spectrum of type EGS_RadionuclideSpectrum was defined.\n\n");
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

    // ==========================
    // Source type = isotropic
    // ==========================

    err = input->getInput("source type",sourceType);
    if (err || input->compare(sourceType,"isotropic")) {
        sourceType = "isotropic";

        egsInformation("EGS_RadionuclideSource: Source type: %s\n",sourceType.c_str());

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
    }

    // ==========================
    // Source type = collimated
    // ==========================

    else if (input->compare(sourceType,"collimated")) {
        sourceType = "collimated";

        egsInformation("EGS_RadionuclideSource: Source type: %s\n",sourceType.c_str());

        EGS_Input *ishape = input->takeInputItem("source shape");
        if (ishape) {
            source_shape = EGS_BaseShape::createShape(ishape);
            delete ishape;
        }
        if (!source_shape) {
            string sname;
            int err = input->getInput("source shape name",sname);
            if (err)
                egsWarning("EGS_RadionuclideSource: missing/wrong inline source "
                           "shape definition and missing/wrong 'source shape name' input\n");
            else {
                source_shape = EGS_BaseShape::getShape(sname);
                if (!source_shape)
                    egsWarning("EGS_RadionuclideSource: a shape named %s"
                               " does not exist\n",sname.c_str());
            }
        }
        ishape = input->takeInputItem("target shape");
        if (ishape) {
            target_shape = EGS_BaseShape::createShape(ishape);
            delete ishape;
        }
        if (!target_shape) {
            string sname;
            int err = input->getInput("target shape name",sname);
            if (err)
                egsWarning("EGS_RadionuclideSource: missing/wrong inline target"
                           "shape definition and missing/wrong 'target shape name' input\n");
            else {
                target_shape = EGS_BaseShape::getShape(sname);
                if (!target_shape)
                    egsWarning("EGS_RadionuclideSource: a shape named %s"
                               " does not exist\n",sname.c_str());
            }
        }
        if (target_shape) {
            if (!target_shape->supportsDirectionMethod())
                egsWarning("EGS_RadionuclideSource: the target shape %s, which is"
                           " of type %s, does not support the getPointSourceDirection()"
                           " method\n",target_shape->getObjectName().c_str(),
                           target_shape->getObjectType().c_str());
        }
        EGS_Float auxd;
        int errd = input->getInput("distance",auxd);
        if (!errd) {
            dist = auxd;
        }

    }
    else {
        egsFatal("EGS_RadionuclideSource: a source type named %s"
                 " does not exist\n",sourceType.c_str());
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
        ishowerOld = ishowerNew;
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

    getPositionDirection(rndm,x,u,wt);
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

    return ++count;
}

void EGS_RadionuclideSource::getPositionDirection(EGS_RandomGenerator *rndm,
        EGS_Vector &x, EGS_Vector &u, EGS_Float &wt) {

    if (sourceType == "isotropic") {
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
        u.z = buf_1 - rndm->getUniform()*(buf_1 - buf_2);
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
    else if (sourceType == "collimated") {
        x = source_shape->getRandomPoint(rndm);
        int ntry = 0;
        do {
            target_shape->getPointSourceDirection(x,rndm,u,wt);
            ntry++;
            if (ntry > 10000) {
                egsFatal("EGS_RadionuclideSource::getPositionDirection:\n"
                         "  my target shape %s, which is of type %s, failed to\n"
                         "  return a positive weight after 10000 attempts\n",
                         target_shape->getObjectName().c_str(),
                         target_shape->getObjectType().c_str());
            }
        }
        while (wt <= 0);

        if (disintegrationOccurred) {
            ctry += ntry-1;
        }
    }
}

void EGS_RadionuclideSource::setUp() {
    otype = "EGS_RadionuclideSource";
    if (!isValid()) {
        description = "Invalid radionuclide source";
    }
    else {
        description = "Radionuclide source of type ";

        if (sourceType == "isotropic") {
            description += sourceType;
            description += " and a shape of type ";
            description += shape->getObjectType();

        }
        else if (sourceType == "collimated") {
            description += sourceType;
            description += " from a shape of type ";
            description += source_shape->getObjectType();
            description += " onto a shape of type ";
            description += target_shape->getObjectType();
        }

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

        if (sourceType == "isotropic") {
            if (geom) {
                geom->ref();
            }
        }
    }
}

bool EGS_RadionuclideSource::storeState(ostream &data_out) const {
    for (unsigned int i=0; i<decays.size(); ++i) {
        if (!decays[i]->storeState(data_out)) {
            return false;
        }
    }
    egsStoreI64(data_out,count);
    egsStoreI64(data_out,ctry);

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
    count = tmp_val;
    egsGetI64(data,tmp_val);
    ctry = tmp_val;

    return true;
}

void EGS_RadionuclideSource::resetCounter() {
    for (unsigned int i=0; i<decays.size(); ++i) {
        decays[i]->resetCounter();
    }
    ishower = 0;
    count = 0;
    ctry = 0;
}

bool EGS_RadionuclideSource::setState(istream &data) {
    for (unsigned int i=0; i<decays.size(); ++i) {
        if (!decays[i]->setState(data)) {
            return false;
        }
    }
    egsGetI64(data,count);
    egsGetI64(data,ctry);

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
