/*
###############################################################################
#
#  EGSnrc egs++ beampp dose scoring object
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
#  Author:          Blake Walters, 2014
#
#  Contributors:
#
###############################################################################
#
#  A dose scoring ausgab object for beampp: Allows the user to score dose in
#  specified CMs on a region-by-region and/or medium-by-medium basis. Volumes
#  of all regions are assumed to be 1 cm^3 unless explicitly specified by the
#  user.
#
###############################################################################
*/


/*! \file beam_dose_scoring.cpp
 *  \brief A dose scoring ausgab object for beampp, similar to egs_dose_scoring
 *  \BW
 */

#include <fstream>
#include <string>

#include "beam_dose_scoring.h"
#include "egs_input.h"
#include "egs_functions.h"

BEAM_DoseScoring::BEAM_DoseScoring(const string &Name,
                                   EGS_ObjectFactory *f) :
    EGS_AusgabObject(Name,f), nreg(0), m_lastCase(-1), nmedia(0),
    dose(0), doseM(0), max_dreg(-1), max_medl(0),
    score_medium_dose(false), score_region_dose(false) {
    otype = "BEAM_DoseScoring";
}

BEAM_DoseScoring::~BEAM_DoseScoring() {
    if (dose) {
        delete dose;
    }
    if (doseM) {
        delete doseM;
    }
}

void BEAM_DoseScoring::setApplication(EGS_Application *App) {

    app=App;
    bapp = dynamic_cast<BEAMpp_Application *>(app);
    if (!bapp) {
        return;
    }
    // Get the number of regions in the geometry.
    nreg = app->getnRegions();
    // Get the number of media in the input file
    nmedia = app->getnMedia();
    // Get list of regions for which to score dose;
    int d_start,d_end;
    // first, clear the array of media indices in the dose CMs
    for (int i=0; i<d_cms.size(); i++) {
        vector <int> med_ind;
        for (int j=0; j<nmedia; j++) {
            med_ind.push_back(-1);
        }
        cm_med.push_back(med_ind);
    }
    for (int i=0; i<d_cms.size(); i++) {
        if (d_cms[i]==0) {
            d_start = 0;
        }
        else {
            d_start = bapp->CM_reg_end(d_cms[i]-1)+1;
        }
        d_end = bapp->CM_reg_end(d_cms[i]);
        for (int j=d_start; j<=d_end; j++) {
            //note we need to use IsRegionReal instead of isRealRegion because
            //the latter registers virtual regions as real in beampp
            if (bapp->IsRegionReal(j)) {
                d_region.push_back(j);
                cm_med[i][app->getMedium(j)]=1;
            }
        }
    }
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
    // Flag dose scoring regions in geometry
    // and set their volume.
    if (d_region.size()) { // specific dose regions provided
        // Get maximum dose scoring region
        for (vector<int>::iterator it = d_region.begin(); it < d_region.end(); it++) {
            if (*it > max_dreg) {
                max_dreg = *it;
            }
        }
        for (int j=0; j<nreg; j++) {
            d_reg_index.push_back(-1);
            d_reg_cm_ind.push_back(-1);
            vol.push_back(1.0);// set to 1.0
        }
        for (int i=0; i<d_region.size(); i++) {
            d_reg_index[d_region[i]]=i;
            if (d_region[i] < vol_list.size()) { //check to make sure we won't exceed array size
                vol[d_region[i]] = vol_list[d_region[i]];
            }
            for (int j=0; j<d_cms.size(); j++) {
                if (d_region[i]<=bapp->CM_reg_end(d_cms[j])) {
                    d_reg_cm_ind[d_region[i]]=j;
                    break;
                }
            }
        }
        if (score_region_dose) {
            dose = new EGS_ScoringArray(d_region.size());
        }
    }
    // If requested request memory for medium dose in each CM
    if (score_medium_dose) {
        doseM =  new EGS_ScoringArray(nmedia*d_cms.size());
    }

    description = "\n*******************************************\n";
    description +=  "BEAM Dose Scoring Object (";
    description += name;
    description += ")\n";
    description += "*******************************************\n";
    description +="\n Dose will be calculated in CM no.'s (start region - end region):\n";
    for (int i=0; i<d_cms.size(); i++) {
        if (d_cms[i]==0) {
            sprintf(buf,"%d (0 - %d)\n",d_cms[i],bapp->CM_reg_end(d_cms[i]));
        }
        else {
            sprintf(buf,"%d (%d - %d)\n",d_cms[i],bapp->CM_reg_end(d_cms[i]-1)+1,bapp->CM_reg_end(d_cms[i]));
        }
        description += buf;
    }
    description += "\n";
    if (dose) {
        description += " - Region dose will be calculated\n";
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
    description += "\n*******************************************\n\n";
    if (d_region.size()) {
        d_region.clear();
    }
}

void BEAM_DoseScoring::reportResults() {
    egsInformation("\n======================================================\n");
    egsInformation("BEAM Dose Scoring Object(%s)\n",name.c_str());
    egsInformation("======================================================\n");
    EGS_Float normD = 1., normE=1.;
    int count = 0;
    EGS_Float F = app->getFluence();
    egsInformation("=> last case = %lld fluence = %g\n", m_lastCase, F);
    /* Normalize to actual source fluence */
    egsInformation("\n Dose output for CM no.'s:\n");
    for (int i=0; i<d_cms.size(); i++) {
        egsInformation(" %d \n",d_cms[i]);
    }
    normE = m_lastCase/F;
    normD = 1.602e-10*normE;
    int irmax_digits = getDigits(max_dreg);
    string line;
    double r,dr;
    if (dose) {
        if (normE==1) {
            egsInformation("\n\n==> Summary of region dosimetry (per particle)\n");
            egsInformation(
                "CM %-*s %-*s %-*s rho/[g/cm3]  V/cm3      Edep/[MeV]              D/[Gy]            %n\n",
                irmax_digits,"ir",irmax_digits,"CM reg.",max_medl,"medium",&count);
        }
        else {
            egsInformation("\n==> Summary of region dosimetry (per fluence)\n");
            egsInformation(
                "CM %-*s %-*s %-*s rho/[g/cm3]  V/cm3    Edep/[MeV*cm2]            D/[Gy*cm2]         %n\n",
                irmax_digits,"ir",irmax_digits,"CM reg.",max_medl,"medium",&count);
        }
        line.append(count,'-');
        egsInformation("%s\n",line.c_str());
        /* Compute deposited energy and dose */
        int ir_cm,fr_cm;
        for (int i=0; i<d_cms.size(); i++) {
            if (d_cms[i]==0) {
                ir_cm=0;
            }
            else {
                ir_cm = bapp->CM_reg_end(d_cms[i]-1)+1;
            }
            fr_cm = bapp->CM_reg_end(d_cms[i]);
            for (int ireg = ir_cm; ireg <= fr_cm; ireg++) {
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
                    egsInformation("%-2d %-*d %-*d %-*s %7.3f   %7.3f  %10.4e +/- %-7.3f%%  %10.4e +/- %-7.3f%%\n",
                                   d_cms[i],max(irmax_digits,2),ireg,max(irmax_digits,7),ireg-ir_cm,max_medl,app->getMediumName(imed),rho,vol[ireg],r*normE,dr*100.,r*normD/mass,dr*100.);
                }
            }
            egsInformation("%s\n",line.c_str());
        }
    }
    if (doseM) {
        vector<EGS_Float> massM(d_cms.size()*nmedia,0);
        int imed = 0;
        for (int ir=0; ir<nreg; ir++) {
            if (d_reg_index[ir]>=0) {
                imed = app->getMedium(ir);
                massM[imed+d_reg_cm_ind[ir]*nmedia] += app->getMediumRho(imed)*vol[ir];
            }
        }
        if (normE==1) {
            egsInformation("\n\n==> Summary of media dosimetry (per particle)\n");
            egsInformation(
                "CM %*s %*s     Edep/[MeV]              D/[Gy]            %n\n",
                max_medl/2,"medium",max_medl/2," ",&count);
        }
        else {
            egsInformation("\n\n==> Summary of media dosimetry (per fluence)\n");
            egsInformation(
                "CM %*s %*s     Edep/[MeV*cm2]              D/[Gy*cm2]            %n\n",
                max_medl/2,"medium",max_medl/2," ",&count);
        }
        line="";
        line.append(count,'-');
        egsInformation("%s\n",line.c_str());
        /* Compute deposited energy in medium (MeV)*/
        for (int i=0; i<d_cms.size(); i++) {
            for (int im=0; im<nmedia; im++) {
                doseM->currentResult(im+i*nmedia,r,dr);
                if (cm_med[i][im] >= 0) {
                    if (r > 0) {
                        dr = dr/r;
                    }
                    else {
                        dr=1;
                    }
                    egsInformation(
                        "%-2d %-*s %10.4e +/- %-7.3f%% %10.4e +/- %-7.3f%%\n",
                        d_cms[i],max_medl,app->getMediumName(im),r*normE,dr*100.,r*normD/massM[im+i*nmedia],dr*100.);
                }
            }
            egsInformation("%s\n",line.c_str());
        }
    }
    egsInformation("\n======================================================\n");
}

bool BEAM_DoseScoring::storeState(ostream &data) const {
    //egsInformation("Storing BEAM_DoseScoring...\n");
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

bool  BEAM_DoseScoring::setState(istream &data) {
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

bool  BEAM_DoseScoring::addState(istream &data) {
    int err = addTheStates(data);
    if (err) {
        egsInformation("Error code: %d",err);
        return false;
    }
    return true;
}

int  BEAM_DoseScoring::addTheStates(istream &data) {
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

void BEAM_DoseScoring::resetCounter() {
    m_lastCase = 0;
    if (dose) {
        dose->reset();
    }
    if (doseM) {
        doseM->reset();
    }
}


extern "C" {

    BEAM_DOSE_SCORING_EXPORT EGS_AusgabObject *createAusgabObject(EGS_Input *input,
            EGS_ObjectFactory *f) {
        const static char *func = "createAusgabObject(beam_dose_scoring)";
        if (!input) {
            egsWarning("%s: null input?\n",func);
            return 0;
        }
        /* get CMs to score dose in */
        vector <int> d_cms;
        int err1=input->getInput("dose CMs",d_cms);
        if (err1) {
            egsWarning("%s: No CMs input.  Dose will not be scored.\n",func);
            return 0;
        }
        /* get dose scoring mode: region dose, medium dose or both */
        vector<string> allowed_mode;
        allowed_mode.push_back("no");
        allowed_mode.push_back("yes");
        int d_in_medium = input->getInput("medium dose",allowed_mode,0);
        int d_in_region = input->getInput("region dose",allowed_mode,1);
        if (d_in_medium == 0 && d_in_region == 0) {
            egsWarning("%s: Neither dose in regions nor dose in medium specified.  Dose will not be scored.\n",func);
            return 0;
        }
        vector <int> vreg_start,vreg_stop;
        vector <EGS_Float> v_in,volin;
        //give the user the option to input volumes of regions if they know them
        int err2 = input->getInput("start volume region",vreg_start);
        int err3 = input->getInput("stop volume region",vreg_stop);
        int err4 = input->getInput("volume",v_in);
        if (err4) {
            egsWarning("%s: No dose zone volumes input.  Will assume 1 cm^3 for all regions.\n",func);
        }
        else {
            if (!err2 && !err3) {
                if (vreg_start.size()==vreg_stop.size()) {
                    if (v_in.size()<vreg_start.size()) {
                        egsWarning("%s: No volumes input for region groups %d to %d.\n Will use last input volume for remaining groups.\n",func,v_in.size()+1,vreg_start.size());
                        for (int i=v_in.size()+1; i<vreg_start.size(); i++) {
                            v_in.push_back(v_in[v_in.size()-1]);
                        }
                    }
                    for (int i=0; i<vreg_start.size(); i++) {
                        if (volin.size()<=vreg_stop[i]) {
                            //enlarge to volin array
                            for (int j=volin.size(); j<=vreg_stop[i]; j++) {
                                volin.push_back(1.0);
                            }
                        }
                        for (int j=vreg_start[i]; j<=vreg_stop[i]; j++) {
                            volin[j]=v_in[i];
                        }
                    }
                }
                else {
                    egsWarning("%s: Unequal number of start and stop regions.  Will assume 1 cm^3 for all regions.\n",func);
                }
            }
        }
        //=================================================

        /* Setup dose scoring object with input parameters */
        BEAM_DoseScoring *result = new BEAM_DoseScoring("",f);
        if (volin.size()) {
            result->setVol(volin);
        }
        else {
            result->setVol(1.0);
        }
        result->setDoseCMs(d_cms);
        if (d_in_medium) {
            result->setMediumScoring(true);
        }
        if (d_in_region) {
            result->setRegionScoring(true);
        }
        result->setName(input);
        return result;
    }

}

