/*
###############################################################################
#
#  EGSnrc egs++ beampp dose scoring object headers
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


/*! \file beam_dose_scoring.h
 *  \brief A dose scoring ausgab object for beampp
 *  \BW
 */

#ifndef BEAM_DOSE_SCORING_
#define BEAM_DOSE_SCORING_

#include "egs_ausgab_object.h"
#include "egs_application.h"
#include "beampp_class.h"
#include "egs_scoring.h"
#include "egs_interface2.h"

#ifdef WIN32

    #ifdef BUILD_DOSE_SCORING_DLL
        #define BEAM_DOSE_SCORING_EXPORT __declspec(dllexport)
    #else
        #define BEAM_DOSE_SCORING_EXPORT __declspec(dllimport)
    #endif
    #define BEAM_DOSE_SCORING_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define BEAM_DOSE_SCORING_EXPORT __attribute__ ((visibility ("default")))
        #define BEAM_DOSE_SCORING_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define BEAM_DOSE_SCORING_EXPORT
        #define BEAM_DOSE_SCORING_LOCAL
    #endif

#endif


/*! \brief A dose scoring object: header

\ingroup AusgabObjects

An ausgab object for BEAMpp which can be used to calculate dose to
regions or dose to media within user-specified CMs.  It is identical to
the EGS_DoseScoring class with some minor modifications.

*/

class BEAM_DOSE_SCORING_EXPORT BEAM_DoseScoring : public EGS_AusgabObject {

public:

    BEAM_DoseScoring(const string &Name="", EGS_ObjectFactory *f = 0);

    ~BEAM_DoseScoring();

    int processEvent(EGS_Application::AusgabCall iarg) {

        int ir = app->top_p.ir, imed = ir>=0 ? app->getMedium(ir):-1;
        EGS_Float edep = app->getEdep();

        if (d_reg_index[ir]<0) {
            return 0;
        }

        /**** energy deposition in current region ***/
        if (iarg <= 4 && ir >= 0 && edep > 0 && dose) {
            dose->score(d_reg_index[ir], edep*app->top_p.wt);
        }

        /**** energy deposition in a medium ***/
        //put it here so it doesn't get scored if this is not a scoring region
        if (iarg <= 4 && imed >= 0 && edep > 0 && doseM) {
            doseM->score(imed+nmedia*d_reg_cm_ind[ir], edep*app->top_p.wt);
        }
        return 0;
    };

    bool needsCall(EGS_Application::AusgabCall iarg) const {
        return true;
    };

    void setApplication(EGS_Application *App);

    void reportResults();

    void setCurrentCase(EGS_I64 ncase) {
        if (ncase != m_lastCase) {
            m_lastCase = ncase;
            if (dose) {
                dose->setHistory(ncase);
            }
            if (doseM) {
                doseM->setHistory(ncase);
            }
        }
    };
    int getDigits(int i) {
        int imax = 10;
        while (i>=imax) {
            imax*=10;
        }
        return (int)log10((float)imax);
    };

    void setVol(const vector<EGS_Float> volin) {
        vol_list=volin;
    };
    void setVol(const EGS_Float volin) {
        vol_list.push_back(volin);
    };
    void setDoseCMs(const vector <int> dcm) {
        d_cms=dcm;
    };
    void setMediumScoring(bool flag) {
        score_medium_dose=flag;
    };
    void setRegionScoring(bool flag) {
        score_region_dose=flag;
    };

    bool storeState(ostream &data) const;
    bool setState(istream &data);
    void resetCounter();
    bool addState(istream &data);
    int addTheStates(istream &data);

protected:
    BEAMpp_Application *bapp;
    EGS_ScoringArray *dose;  //!< Scoring in each dose scoring region
    EGS_ScoringArray *doseM;  //!< Scoring dose in each medium
    vector <EGS_Float>  vol_list; // Input list of region volumes
    vector <int> d_cms;       // List of CMs for which dose is to be output
    vector <int> d_region;        // Input list of dose scoring regions  d_reg[i] = ir
    vector <int> d_reg_index;     // list index for dose scoring regions d_reg_index[ir]= 0..d_reg.size()-1
    vector <int> d_reg_cm_ind;    // dose CM index for each dose region.  Allows quick scoring of medium dose.
    vector < vector <int> > cm_med; // list of media present in CMs in which dose is scored
    vector <EGS_Float>  vol;      // geometrical region volumes
    int nreg,     // number of regions in the geometry
        nmedia;   // number of media in the input file
    int max_dreg, // maximum dose region number
        max_medl; // maximum medium name length
    EGS_I64    m_lastCase;   //!< The event set via setCurrentCase()
    bool score_medium_dose,
         score_region_dose;
};

#endif
