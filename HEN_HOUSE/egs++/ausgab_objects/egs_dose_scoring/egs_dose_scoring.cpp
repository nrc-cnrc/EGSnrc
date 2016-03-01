/*
###############################################################################
#
#  EGSnrc egs++ dose scoring object
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
#  Author:          Ernesto Mainegra-Hing, 2012
#
#  Contributors:    Frederic Tessier
#
###############################################################################
#
#  A general dose calculation tool. Since it is outside the scope of the
#  EGS_Application class, a few utility methods are needed in EGS_Application
#  to retrieve information regarding media, geometry and source.
#
#  Dose is output for each dose region without any spacial information. For a
#  more detailed dose scoring grid it is better to create a user code which
#  could produce 3D dose distributions and graph data.
#
#  Can be used in any C++ user code by entering the proper input block in
#  the ausgab object definition block.
#
#  Prints out dose in each dose scoring region.
#
#  Dose scoring regions can be defined by input using one the following
#  syntax for individual regions:
#
#  :start ausgab object definition:
#      :start ausgab object:
#          library      = egs_dose_scoring
#          name         = some_name
#          volume       = V0  V1 ... Vn
#          dose regions = IR0 IR1 ... IRn
#      :stop ausgab object:
#  :stop ausgab object definition:
#
#  or by groups of consecutive regions:
#
#  :start ausgab object definition:
#      :start ausgab object:
#          library           = egs_dose_scoring
#          name              = some_name
#          volume            = V0 V1 ... Vn
#          dose start region = IRI_0 ... IRI_N
#          dose stop region  = IRE_0 ... IRE_N
#      :stop ausgab object:
#  :stop ausgab object definition:
#
#  where one can have same number of volume entries as group of regions (n=N)
#  or same number of volume entries as number of individual regions. If there
#  are more dose scoring regions than volume entries, a default volume is use for
#  those regions without volume information. This default can be either the first
#  volume entry or 1 g/cm3.
#
#  TODO:
#
#  - Classify dose as contributed by primary or scattered particles
#  - Custom classification using latch
#  - Dose to medium calculation
#
###############################################################################
*/


/*! \file egs_dose_scoring.cpp
 *  \brief A dose scoring ausgab object: implementation
 *  \EM
 */

#include <fstream>
#include <string>

#include "egs_dose_scoring.h"
#include "egs_input.h"
#include "egs_functions.h"

EGS_DoseScoring::EGS_DoseScoring(const string &Name,
                                 EGS_ObjectFactory *f) :
    EGS_AusgabObject(Name,f), nreg(0), m_lastCase(-1), nmedia(0),
    dose(0), doseM(0), max_dreg(-1), max_medl(0),
    score_medium_dose(false), score_region_dose(false), norm_u(1.0) {
    otype = "EGS_DoseScoring";
}

EGS_DoseScoring::~EGS_DoseScoring() {
    if (dose) {
        delete dose;
    }
    if (doseM) {
        delete doseM;
    }
}

void EGS_DoseScoring::setApplication(EGS_Application *App) {
    EGS_AusgabObject::setApplication(App);
    if (!app) {
        return;
    }
    // Get the number of regions in the geometry.
    nreg = app->getnRegions();
    // Get the number of media in the input file
    nmedia = app->getnMedia();
    // determine maximum medium name length
    char buf[32];
    int count = 0;
    int imed=0;
    for (imed=0; imed < nmedia; imed++) {
        sprintf(buf,"%s%n",app->getMediumName(imed),&count);
        if (count > max_medl) {
            max_medl = count;
        }
    }
    if (d_region.size()>nreg)
        egsWarning("\n*********************************************"
                   "\n WARNING:"
                   "\nRequesting %d dose scoring regions, but there"
                   "\nare only %d geometrical regions!"
                   "\nscoring will be done in all regions!"
                   "\n*********************************************",
                   d_region.size(),nreg);
    // Flag dose scoring regions in geometry
    // and set their volume.
    if (score_region_dose) {
        if (d_region.size() && d_region.size() < nreg) { // specific dose regions provided
            // Get maximum dose scoring region
            for (vector<int>::iterator it = d_region.begin(); it < d_region.end(); it++) {
                if (*it > max_dreg) {
                    max_dreg = *it;
                }
            }
            for (int j=0; j<nreg; j++) {
                d_reg_index.push_back(-1);
                vol.push_back(vol_list[0]);// set to either 1.0 or first volume entered
            }
            for (int i=0; i<d_region.size(); i++) {
                d_reg_index[d_region[i]]=i;
                if (i < vol_list.size()) {
                    vol[d_region[i]] = vol_list[i];
                }
            }
            if (score_region_dose) {
                dose = new EGS_ScoringArray(d_region.size());
            }
        }
        else { // scoring in all regions
            max_dreg = nreg-1;
            for (int j=0; j<nreg; j++) {
                d_reg_index.push_back(j);
                if (j < vol_list.size()) {
                    vol.push_back(vol_list[j]);
                }
                else { // more regions than volumes, use default 1 g/cm3 or first volume entered
                    vol.push_back(vol_list[0]);
                }
            }
            if (score_region_dose) {
                dose = new EGS_ScoringArray(nreg);
            }
        }
    }
    // If requested request memory for medium dose
    if (score_medium_dose) {
        doseM =  new EGS_ScoringArray(nmedia);
    }
    if (!score_region_dose) { // setup volumes
        if (vol_list.size() == 1) {
            vol.push_back(vol_list[0]);
        }
        else {
            for (int j=0; j<nreg; j++) {
                if (j < vol_list.size()) {
                    vol.push_back(vol_list[j]);
                }
                else { // more regions than volumes, use default 1 g/cm3 or first volume entered
                    vol.push_back(vol_list[0]);
                }
            }
        }
    }

    description = "\n*******************************************\n";
    description +=  "Dose Scoring Object (";
    description += name;
    description += ")\n";
    description += "*******************************************\n";
    if (dose) {
        description +="\n - Regions in dose calculator :";
        sprintf(buf,"%d\n\n",dose->regions());
        description += buf;
    }
    if (doseM) {
        description += " - Medium dose will be calculated\n";
    }
    description += "\n--------------------------------------\n";
    sprintf(buf,"%*s %*s rho/[g/cm**3]\n",max_medl/2,"medium",max_medl/2," ");
    description += buf;
    description += "--------------------------------------\n";
    for (imed=0; imed < nmedia; imed++) {
        sprintf(buf,"%-*s",max_medl,app->getMediumName(imed));
        description += buf;
        description += " ";
        sprintf(buf,"%5.2f",app->getMediumRho(imed));
        description += buf;
        description += "\n";
    }
    description += "--------------------------------------\n";
    if (norm_u != 1.0) {
        description += " Non-unity user-requested normalization = ";
        sprintf(buf,"%g\n",norm_u);
        description += buf;
    }
    description += "\n*******************************************\n\n";
    vol_list.clear();
    if (d_region.size()) {
        d_region.clear();
    }
}

void EGS_DoseScoring::reportResults() {
    egsInformation("\n======================================================\n");
    egsInformation("Dose Scoring Object(%s)\n",name.c_str());
    egsInformation("======================================================\n");
    EGS_Float normD = 1., normE=1.;
    int count = 0;
    EGS_Float F = app->getFluence();
    egsInformation("=> last case = %lld fluence = %g\n", m_lastCase, F);
    /* Normalize to actual source fluence */
    normE = m_lastCase/F*norm_u;
    normD = 1.602e-10*normE;
    int irmax_digits = getDigits(max_dreg);
    string line;
    double r,dr;
    if (dose) {
        if (normE==1) {
            egsInformation("\n\n==> Summary of region dosimetry (per particle)\n");
            egsInformation(
                "%*s %*s %*s rho/[g/cm3]  V/cm3      Edep/[MeV]              D/[Gy]            %n\n",
                irmax_digits,"ir",max_medl/2,"medium",max_medl/2," ",&count);
        }
        else {
            egsInformation("\n==> Summary of region dosimetry (per fluence)\n");
            egsInformation(
                "%*s %*s %*s rho/[g/cm3]  V/cm3    Edep/[MeV*cm2]            D/[Gy*cm2]         %n\n",
                irmax_digits,"ir",max_medl/2,"medium",max_medl/2," ",&count);
        }
        line.append(count,'-');
        egsInformation("%s\n",line.c_str());

        /* Compute deposited energy and dose */
        for (int ireg = 0; ireg < nreg; ireg++) {
            if (d_reg_index[ireg]>=0) {
                if (!(app->isRealRegion(ireg))) {
                    continue;
                }
                int imed = app->getMedium(ireg);
                EGS_Float rho = app->getMediumRho(imed);
                EGS_Float mass = vol[ireg]*rho;
                dose->currentResult(d_reg_index[ireg],r,dr);
                if (r > 0) {
                    dr = dr/r;
                }
                else {
                    dr=1;
                }
                egsInformation("%*d %-*s %7.3f   %8.4f %10.4e +/- %-7.3f%% %10.4e +/- %-7.3f%%\n",
                               irmax_digits,ireg,max_medl,app->getMediumName(imed),rho,vol[ireg],r*normE,dr*100.,r*normD/mass,dr*100.);
            }
        }
        egsInformation("%s\n",line.c_str());
    }
    if (doseM) {
        vector<EGS_Float> massM(nmedia,0);
        int imed = 0;
        for (int ir=0; ir<nreg; ir++) {
          if(app->isRealRegion(ir)) {
            imed = app->getMedium(ir);
            EGS_Float volume = vol.size() > 1 ? vol[ir]:vol[0];
            massM[imed] += app->getMediumRho(imed)*volume;
          }
        }
        if (normE==1) {
            egsInformation("\n\n==> Summary of media dosimetry (per particle)\n");
            egsInformation(
                "%*s %*s     Edep/[MeV]              D/[Gy]            %n\n",
                max_medl/2,"medium",max_medl/2," ",&count);
        }
        else {
            egsInformation("\n\n==> Summary of media dosimetry (per fluence)\n");
            egsInformation(
                "%*s %*s     Edep/[MeV*cm2]              D/[Gy*cm2]            %n\n",
                max_medl/2,"medium",max_medl/2," ",&count);
        }
        line="";
        line.append(count,'-');
        egsInformation("%s\n",line.c_str());
        /* Compute deposited energy in medium (MeV)*/
        for (int im=0; im<nmedia; im++) {
            doseM->currentResult(im,r,dr);
            if (r > 0) {
                dr = dr/r;
                egsInformation(
                    "%-*s %10.4e +/- %-7.3f%% %10.4e +/- %-7.3f%%\n",
                    max_medl,app->getMediumName(im),r*normE,dr*100.,r*normD/massM[im],dr*100.);
            }
        }
        egsInformation("%s\n",line.c_str());
    }
    egsInformation("\n======================================================\n");
}

bool EGS_DoseScoring::storeState(ostream &data) const {
    //egsInformation("Storing EGS_DoseScoring...\n");
    if (!egsStoreI64(data,m_lastCase)) {
        return false;
    }
    data << endl;
    if (dose  && !dose->storeState(data)) {
        return false;
    }
    if (doseM && !doseM->storeState(data)) {
        return false;
    }
    return true;
}

bool  EGS_DoseScoring::setState(istream &data) {
    if (!egsGetI64(data,m_lastCase)) {
        return false;
    }
    if (dose  && !dose->setState(data)) {
        return false;
    }
    if (doseM && !doseM->setState(data)) {
        return false;
    }
    return true;
}

bool  EGS_DoseScoring::addState(istream &data) {
    int err = addTheStates(data);
    if (err) {
        egsInformation("Error code: %d",err);
        return false;
    }
    return true;
}
int  EGS_DoseScoring::addTheStates(istream &data) {
    EGS_I64 tmp_case;
    if (!egsGetI64(data,tmp_case)) {
        return 4401;
    }
    m_lastCase += tmp_case;
    if (dose) {
        EGS_ScoringArray tmp(nreg);
        if (!tmp.setState(data)) {
            return 4402;
        }
        (*dose) += tmp;
    }
    if (doseM) {
        EGS_ScoringArray tmpM(nmedia);
        if (!tmpM.setState(data)) {
            return 4403;
        }
        (*doseM) += tmpM;
    }
    return 0;
}

void EGS_DoseScoring::resetCounter() {
    m_lastCase = 0;
    if (dose) {
        dose->reset();
    }
    if (doseM) {
        doseM->reset();
    }
}

//*********************************************************************
// Process input for this ausgab object
//
// volume: Set to 1 cm3 by default
//         One entry       => same size dose scoring regions
//         Several entries => volume for each dose region
//                            SAME as dose region number
//                            or else default value used
//
// dose scoring regions: Defaults to all regions in geometry
//         dose regions           => Individual regions
//         dose start/stop region => Groups of consecutive
//                                   dose regions
//
// If there is a mismatch between number of regions and volumes
// default volume values will be used. Default volume will be either
// the first volume entry or 1 g/cm3 if no volume entry found.
//
// TODO:
// - Classify in primary, scattered and total dose
// - Specify for wich media to score or not the dose
//
//**********************************************************************
extern "C" {

    EGS_DOSE_SCORING_EXPORT EGS_AusgabObject *createAusgabObject(EGS_Input *input,
            EGS_ObjectFactory *f) {
        const static char *func = "createAusgabObject(dose_scoring)";
        if (!input) {
            egsWarning("%s: null input?\n",func);
            return 0;
        }
        vector <EGS_Float> v_in;// voxel volume[s] in g/cm3
        /* get voxel volume */
        input->getInput("volume",v_in);
        /* get dose scoring mode: region dose, medium dose or both */
        vector<string> allowed_mode;
        allowed_mode.push_back("no");
        allowed_mode.push_back("yes");
        int d_in_medium = input->getInput("medium dose",allowed_mode,0);
        int d_in_region = input->getInput("region dose",allowed_mode,1);

        /* get dose regions */
        vector <int> d_regions;
        bool using_all_regions=true;
        vector <int> d_start, d_stop;
        if (!input->getInput("dose regions",d_regions)&& d_regions.size()>0) {
            using_all_regions = false;    // individual regions
        }
        else {
            int err1 = input->getInput("dose start region",d_start);
            int err2 = input->getInput("dose stop region",d_stop);
            if (!err1 && !err2) {
                if (d_start.size()==d_stop.size()) { // group of dose regions
                    for (int i=0; i<d_start.size(); i++) {
                        int ir = d_start[i], fr = d_stop[i];
                        for (int ireg=ir; ireg<=fr; ireg++) {
                            d_regions.push_back(ireg);
                        }
                    }
                    using_all_regions = false;
                }
                else egsWarning(
                        "%s: Mismatch in start and stop dose region groups !!!\n"
                        " Calculating dose in ALL regions.\n",func);
            }
        }
        EGS_Float norma = 1.0;
        int err04 = input->getInput("normalization",norma);

        //================================================
        // Check if one volume for each group requested.
        // If not just pass the volumes read and if there
        // is a mismatch, then the first volume element
        // or 1g/cm3 will be used.
        //=================================================
        vector <EGS_Float> volin;
        // groups of regions with same volume
        if (! using_all_regions && v_in.size()== d_start.size()) {
            for (int i=0; i<d_start.size(); i++) {
                int ir = d_start[i], fr = d_stop[i];
                for (int ireg=ir; ireg<=fr; ireg++) {
                    volin.push_back(v_in[i]);
                }
            }
        }
        else { // all other possibilities handled here
            volin = v_in;
        }
        //=================================================

        /* Setup dose scoring object with input parameters */
        EGS_DoseScoring *result = new EGS_DoseScoring("",f);
        if (volin.size()==1) {
            result->setVol(volin[0]);    // one size for all regions
        }
        else if (volin.size()) {
            result->setVol(volin);    // regions with their volumes
        }
        else {
            result->setVol(1.0);    // default value if no entry
        }
        if (!using_all_regions) {
            result->setDoseRegions(d_regions);
        }
        if (d_in_medium) {
            result->setMediumScoring(true);
        }
        if (d_in_region) {
            result->setRegionScoring(true);
        }
        result->setName(input);
        if (!err04) {
            result->setUserNorm(norma);
        }
        return result;
    }

}
