/*
###############################################################################
#
#  EGSnrc egs++ dose scoring object headers
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
#  Contributors:    Reid Townson
#                   Blake Walters
#                   Max Orok
#                   Martin Martinov
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


/*! \file egs_dose_scoring.h
 *  \brief A dose scoring ausgab object
 *  \IK
 */

#ifndef EGS_DOSE_SCORING_
#define EGS_DOSE_SCORING_

#include "egs_ausgab_object.h"
#include "egs_application.h"
#include "egs_scoring.h"
#include "egs_base_geometry.h"

#ifdef WIN32

    #ifdef BUILD_DOSE_SCORING_DLL
        #define EGS_DOSE_SCORING_EXPORT __declspec(dllexport)
    #else
        #define EGS_DOSE_SCORING_EXPORT __declspec(dllimport)
    #endif
    #define EGS_DOSE_SCORING_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_DOSE_SCORING_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_DOSE_SCORING_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_DOSE_SCORING_EXPORT
        #define EGS_DOSE_SCORING_LOCAL
    #endif

#endif

/*! \brief A dose scoring object: header

\ingroup AusgabObjects

This ausgab object can be used to calculate dose in identical voxels
during run time and to output this information at the end of the
calculation. This ausgab object is specified via:
\verbatim
:start ausgab object:
  library      = egs_dose_scoring
  name         = some_name
  medium dose  = yes # no  (default)
  region dose  = no  # yes (default)
  volume       =  v1 ... vn  # in cm**3
  dose regions = ir1 ... irn # individual regions
:stop ausgab object:
\endverbatim
Users can enter either a unique volume, indicating identical
volumes for all dose scoring regions, or individual volumes for
each dose region, in which case the number of entries must be
the same. Another available option is the medium dose entry
which requests the dose deposited in each medium to be scored.
This is of great use for instance for organ dose calculations.

Another possibility is to provide groups of consecutive regions
to score dose as shown in the example below:
\verbatim
:start ausgab object:
  library           = egs_dose_scoring
  name              = some_name
  medium dose  = yes # no  (default)
  region dose  = no  # yes (default)
  volume            =  v_1 ... v_n        # in cm**3
  dose start region = iri_1 ... iri_N
  dose stop region  =  ire_1 ... ire_N
:stop ausgab object:
\endverbatim
In this case the user can enter either the same number of volumes as groups
of dose scoring regions or individual volumes for each region, in which case
the number of volume entries n should be \f$n = \sum_i \left(ire_{i} - iri_{i}\right)\f$.

A volume value is necessary to compute the mass for each dose scoring zone. For
this reason, if there are more dose scoring regions than volume entries, either
the first volume entry or a default of 1 g/cm3 is used.

If one of the geometries in the simulation is an EGS_XYZGeometry, then the user
has the option to output the dose in the voxels of this geometry to a file.  Currently,
the only available output format is the .3ddose format, which is familiar to users
of DOSXYZnrc (see DOSXYZnrc users manual for more details). In this case,
masses of each voxel are available through the member function getVolume of
EGS_XYZGeometry.  An example input to obtain a .3ddose file for an EGS_XYZGeometry is given below:

\verbatim
:start ausgab object:
  library           = egs_dose_scoring
  name              = some_name
  medium dose = no
  region dose = no
  :start output dose file:
     geometry name = must name an EGS_XYZGeometry
     file type = 3ddose (currently the only format available)
  :stop output dose file:
:stop ausgab object:
\endverbatim

Output is to the file some_name.3ddose.  Note that in this example, region doses for the
simulation geometry have been turned off to avoid outputting the dose for every voxel
in the EGS_XYZGeometry to the screen/.egslog file.

TODO:
 - Classify in primary, scattered and total dose
 - Specify for wich media to score or not the dose

*/

class EGS_DOSE_SCORING_EXPORT EGS_DoseScoring : public EGS_AusgabObject {

public:

    EGS_DoseScoring(const string &Name="", EGS_ObjectFactory *f = 0);

    ~EGS_DoseScoring();

    int processEvent(EGS_Application::AusgabCall iarg) {

        int ir = app->top_p.ir, imed = ir>=0 ? app->getMedium(ir):-1;
        EGS_Float edep = app->getEdep();

        /**** energy deposition in a medium ***/
        if (iarg <= 4 && imed >= 0 && edep > 0 && doseM) {
            doseM->score(imed, edep*app->top_p.wt);
        }

        //score in file array if requested
        if (ir >=0 && doseF && iarg <=4 && df_reg[ir] >= 0 && edep) {
            doseF->score(df_reg[ir], edep*app->top_p.wt);
        }

        /*** Check if scoring in current region ***/
        if (ir >= 0 && dose) {
            if (d_reg_index[ir]<0) {
                return 0;
            }
        }

        /**** energy deposition in current region ***/
        if (iarg <= 4 && ir >= 0 && edep > 0 && dose) {
            dose->score(d_reg_index[ir], edep*app->top_p.wt);
        }
        return 0;
    };

    int processEvent(EGS_Application::AusgabCall iarg, int ir) {

        if (ir == -1) {
            ir = app->top_p.ir;
        }

        int imed = ir>=0 ? app->getMedium(ir):-1;
        EGS_Float edep = app->getEdep();

        /**** energy deposition in a medium ***/
        if (iarg <= 4 && imed >= 0 && edep > 0 && doseM) {
            doseM->score(imed, edep*app->top_p.wt);
        }

        //score in file array if requested
        if (ir >= 0 && doseF && iarg <=4 && df_reg[ir] >= 0 && edep) {
            doseF->score(df_reg[ir], edep*app->top_p.wt);
        }

        /*** Check if scoring in current region ***/
        if (ir >= 0 && dose) {
            if (d_reg_index[ir]<0) {
                return 0;
            }
        }

        /**** energy deposition in current region ***/
        if (iarg <= 4 && ir >= 0 && edep > 0 && dose) {
            dose->score(d_reg_index[ir], edep*app->top_p.wt);
        }
        return 0;
    };

    bool needsCall(EGS_Application::AusgabCall iarg) const {
        if (iarg <= 4) {
            return true;
        }
        else {
            return false;
        }
    };

    void setApplication(EGS_Application *App);

    void getNumberRegions(const string &str, vector<int> &regs);

    void getLabelRegions(const string &str, vector<int> &regs);

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
            if (doseF) {
                doseF->setHistory(ncase);
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

    EGS_Float getRealRho(int ireg) {
        int med = dose_geom->medium(ireg);
        return dose_geom->getRelativeRho(ireg)*app->getMediumRho(med);
    }

    void setVol(const vector<EGS_Float> volin) {
        vol_list=volin;
    };
    void setVol(const EGS_Float volin) {
        vol_list.push_back(volin);
    };
    void setDoseRegions(const vector <int> d_reg) {
        d_region=d_reg;
    };
    void setDoseRegions(const string d_reg) {
        d_regionString=d_reg;
    };
    void setMediumScoring(bool flag) {
        score_medium_dose=flag;
    };
    void setRegionScoring(bool flag) {
        score_region_dose=flag;
    };
    void setOutputFile(bool flag, EGS_BaseGeometry *dgeom, int ftype) {
        output_dose_file=flag;
        dose_geom= dgeom;
        file_type = ftype;
    };
    bool getOutputFile(EGS_BaseGeometry *&dgeom, int &ftype) {
        dgeom = dose_geom;
        ftype = file_type;
        return output_dose_file;
    };
    void setUserNorm(const EGS_Float &normi) {
        norm_u=normi;
    };
    void outputDoseFile(const EGS_Float &normD);

    bool storeState(ostream &data) const;
    bool setState(istream &data);
    void resetCounter();
    bool addState(istream &data);
    int addTheStates(istream &data);

protected:

    EGS_ScoringArray *dose;  //!< Scoring in each dose scoring region
    EGS_ScoringArray *doseM;  //!< Scoring dose in each medium
    vector <EGS_Float>  vol_list; // Input list of region volumes
    vector <int> d_region;        // Input list of dose scoring regions  d_reg[i] = ir
    string d_regionString;
    vector <int> d_reg_index;     // list index for dose scoring regions d_reg_index[ir]= 0..d_reg.size()-1
    vector <EGS_Float>  vol;      // geometrical region volumes
    EGS_Float norm_u;
    int nreg,     // number of regions in the geometry
        nmedia;   // number of media in the input file
    int max_dreg, // maximum dose region number
        max_medl; // maximum medium name length
    EGS_I64    m_lastCase;   //!< The event set via setCurrentCase()
    bool score_medium_dose,
         score_region_dose;

    EGS_BaseGeometry *dose_geom; //EGS_XYZGeometry for which to output dose to file
    EGS_ScoringArray *doseF;  //!< Scoring dose in each voxel in EGS_XYZGeometry
    vector<int> df_reg; //array mapping global reg. no. onto reg. no. in EGS_XYZGeometry
    bool output_dose_file; //set to true if outputting a dose file
    int file_type;           //output file type--currently only .3ddose (file_type=0) supported
    string df_name;          //output file name--put here for convenience sake
};

#endif
