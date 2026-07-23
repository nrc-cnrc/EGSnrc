/*
###############################################################################
#
#  EGSnrc egs++ fluence scoring object implementation
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
#  Author:          Ernesto Mainegra-Hing, 2022
#
#  Contributors:
#
###############################################################################
*/

/*! \file egs_fluence_scoring.cpp
 *  \brief A fluence scoring ausgab object: implementation
 *  \EM
 */


#include <string>
#include <cstdlib>
#include <limits>
#include <algorithm>
#include <numeric>
#include <cmath>

#include "egs_fluence_scoring.h"
#include "egs_input.h"
#include "egs_functions.h"

static bool EGS_FLUENCE_SCORING_LOCAL inputSet = false;

#define REGIONS_ENTRIES 100
#define REGIONS_PER_LINE 25

EGS_FluenceScoring::EGS_FluenceScoring(const string &Name, EGS_ObjectFactory *f) :
    EGS_AusgabObject(Name,f), particle_name("photon"),
    scoring_charge(photon), source_charge(unknown),
    n_scoring_regions(0), n_source_regions(0),
    max_reg(-1), active_region(-1),
    flu_s(0), flu_nbin(128), flu_xmin(0.001), flu_xmax(1.0),
    norm_u(1.0), flu(0), fluT(0), flu_p(0), fluT_p(0), current_ncase(0),
    verbose(false), score_primaries(false), score_spe(false),
    m_primary(0.0), m_tot(0.0),
    m_scoring_method(score_crossing), m_i_gmfp(0), m_i_cohe(0),
    m_rayleigh_on(false),
    m_cos_min(cos(85.0 * M_PI / 180.0)), m_n_clipped(0),
    m_tot_FD(0.0), m_primary_FD(0.0) {
    otype = "EGS_FluenceScoring";
    flu_a = flu_nbin;
    flu_a /= (flu_xmax - flu_xmin);
    flu_b = -flu_xmin*flu_a;
}

/*! Destructor. */
EGS_FluenceScoring::~EGS_FluenceScoring() {
    if (flu) {
        delete [] flu;
    }
    if (fluT) {
        delete fluT;
    }
    if (flu_p) {
        delete [] flu_p;
    }
    if (fluT_p) {
        delete fluT_p;
    }
}

void EGS_FluenceScoring::initScoring(EGS_Input *inp) {

    if (!inp) {
        egsWarning("AO type %s: null input?\n",otype.c_str());
        return;
    }

    vector<string> name;
    int the_selection = 0;
    name.push_back("photon");
    name.push_back("electron");
    name.push_back("positron");
    name.push_back("undefined");
    the_selection = inp->getInput("scoring particle", name, 3);
    switch (the_selection) {
    case 0:
        particle_name="photon";
        scoring_charge = photon;
        break;
    case 1:
        particle_name="electron";
        scoring_charge = electron;
        break;
    case 2:
        particle_name="positron";
        scoring_charge = positron;
        break;
    default:
        egsFatal("\n*******  ERROR  *******\n"
                 " Undefined scoring charge!\n"
                 " Aborting!\n"
                 "************************\n");
    }

    the_selection = inp->getInput("source particle", name, 3);
    switch (the_selection) {
    case 0:
        source_charge = photon;
        break;
    case 1:
        source_charge = electron;
        break;
    case 2:
        source_charge = positron;
        break;
    default:
        source_charge = unknown;
    }

    vector<string> choice;
    choice.push_back("no");
    choice.push_back("yes");
    verbose         = inp->getInput("verbose",        choice,0);
    score_primaries = inp->getInput("score primaries",choice,0);
    score_spe      = inp->getInput("score spectrum", choice,0);

    EGS_Float norma;
    int err_norm = inp->getInput("normalization",norma);
    if (!err_norm) {
        norm_u = norma;
    }
    else {
        norm_u = 1.0;
    }

    vector<string> method_names;
    method_names.push_back("crossing");
    method_names.push_back("forced detection");
    method_names.push_back("both");
    int msel = inp->getInput("scoring method", method_names, 0);
    switch (msel) {
    case 1:  m_scoring_method = score_FD;       break;
    case 2:  m_scoring_method = score_both;     break;
    default: m_scoring_method = score_crossing; break;
    }

    if (score_spe) {
        EGS_Float flu_Emin, flu_Emax;
        EGS_Input *eGrid = inp->takeInputItem("energy grid");
        if (eGrid) {
            int err_n    = eGrid->getInput("number of bins",flu_nbin);
            int err_i    = eGrid->getInput("minimum kinetic energy",flu_Emin);
            int err_f    = eGrid->getInput("maximum kinetic energy",flu_Emax);
            if (err_n) egsFatal("\n**** EGS_FluenceScoring::initScoring"
                                    "       Missing input: number of bins.\n"
                                    "       Aborting!\n\n");
            if (err_i) egsFatal("\n**** EGS_FluenceScoring::initScoring"
                                    "       Missing input: minimum kinetic energy.\n"
                                    "       Aborting!\n\n");
            if (err_f) egsFatal("\n**** EGS_FluenceScoring::initScoring"
                                    "       Missing input: maximum kinetic energy.\n"
                                    "       Aborting!\n\n");


            vector<string> scale;
            scale.push_back("linear");
            scale.push_back("logarithmic");
            flu_s = eGrid->getInput("scale",scale,0);
            if (flu_s == 0) {
                flu_xmin = flu_Emin;
                flu_xmax = flu_Emax;
            }
            else {
                flu_xmin = log(flu_Emin);
                flu_xmax = log(flu_Emax);
            }
            flu_a = flu_nbin;
            flu_a /= (flu_xmax - flu_xmin);
            flu_b = -flu_xmin*flu_a;
            /******************************************************
              Algorithm assigns E in [Ei,Ei+1), one could add extra
              bin for E = Emax cases. Alternatively, push those
              events into last bin (bias?) during scoring.
              Which approach is correct?
            ******************************************************/
            //flu_nbin++;
        }
    }

    /*
       Get source regions where interacting particles
       are not subjected to classification
    */
    vector <int> s_start, s_stop;
    /*
    if (!inp->getInput("source regions",s_regionsString) && s_regionsString.length()>0) {
        using_all_regions = false;    // individual regions
    }
    else {*/
    /* Failed reading individual regions, try group of regions! */
    if (inp->getInput("source regions",s_regionsString) || s_regionsString.length()<=0) {
        int err1 = inp->getInput("start source region",s_start);
        int err2 = inp->getInput("stop source region",s_stop);
        if (!err1 && !err2) {
            if (s_start.size() == s_stop.size()) { // group of dose regions
                for (int i=0; i<s_start.size(); i++) {
                    int ir = s_start[i], fr = s_stop[i];
                    if (ir > fr)
                        egsFatal("\nEGS_FluenceScoring::initScoring: \n"
                                 "  Decreasing start (%d) / stop %d region in source region group %d\n"
                                 "  Aborting!\n\n", ir, fr, i);
                    for (int ireg=ir; ireg<=fr; ireg++) {
                        s_region.push_back(ireg);
                    }
                }
            }
            else egsFatal(
                    "\nEGS_FluenceScoring::initScoring: \n"
                    "   Mismatch in number of start (%d) and stop (%d)\n"
                    "   source region groups. Aborting!\n",
                    s_start.size(),s_stop.size());
        }
    }

}

void EGS_FluenceScoring::getSensitiveRegions(EGS_Input *inp) {

    string listLabel, startLabel, stopLabel;

    if (otype == "EGS_PlanarFluence" || otype == "EGS_SphericalFluence") {
        listLabel  = "contributing regions";
        startLabel = "start contributing region";
        stopLabel  = "stop contributing region";
    }
    else if (otype == "EGS_VolumetricFluence") {
        listLabel  = "scoring regions";
        startLabel = "start region";
        stopLabel  = "stop region";
    }
    else {
        egsFatal("\n*** Unknown fluence scoriung type! Aborting!\n\n");
    }

    /* get scoring regions */
    if (!inp->getInput(listLabel,f_regionsString) && f_regionsString.length()>0) {
        // Individual regions
        if (f_regionsString == "ALL") {
            score_in_all_regions = true;
        }
        else {
            score_in_all_regions = false;
        }
    }
    else {// Groups of regions
        int err1 = inp->getInput(startLabel,f_start);
        int err2 = inp->getInput(stopLabel, f_stop);
        if (!err1 && !err2) {
            if (f_start.size() == f_stop.size()) {  // group of dose regions
                for (int i=0; i<f_start.size(); i++) {
                    int ir = f_start[i], fr = f_stop[i];
                    if (ir > fr)
                        egsFatal("\nEGS_FluenceScoring::initScoring: \n"
                                 "  Decreasing start (%d) / stop (%d) in region group %d\n"
                                 "  Aborting!\n\n", ir, fr, i);
                    for (int ireg=ir; ireg<=fr; ireg++) {
                        f_region.push_back(ireg);
                    }
                }
                score_in_all_regions = false;
            }
            else egsFatal(
                    "\nEGS_FluenceScoring::initScoring: \n"
                    "   Mismatch in number of start (%d) and stop (%d)\n"
                    "   region groups. Aborting!\n",
                    f_start.size(),f_stop.size());
        }
    }

}
void EGS_FluenceScoring::getNumberRegions(const string &str, vector<int> &regs) {
    if (!app)
        egsFatal("EGS_FluenceScoring::getNumberRegions\n"
                 "  Undefined parent application! Aborting!\n");

    app->getNumberRegions(str, regs);
}

void EGS_FluenceScoring::getLabelRegions(const string &str, vector<int> &regs) {
    if (!app)
        egsFatal("EGS_FluenceScoring::getLabelRegions\n"
                 "  Undefined parent application! Aborting!\n");

    app->getLabelRegions(str, regs);
}

void EGS_FluenceScoring::setUpRegionFlags() {

    if (s_regionsString.length() > 0 && !s_region.size()) {
        getNumberRegions(s_regionsString, s_region);
        getLabelRegions(s_regionsString, s_region);
    }
    // Get the number of source regions. If too many, reset to nreg.
    n_source_regions = s_region.size() < nreg ? s_region.size() : nreg;

    if (!score_in_all_regions) {
        if (f_regionsString.length() > 0 && !f_region.size()) {
            getNumberRegions(f_regionsString, f_region);
            getLabelRegions(f_regionsString, f_region);
        }
        // Get the number of scoring regions. If too many, reset to nreg
        n_scoring_regions = f_region.size() < nreg ? f_region.size() : nreg;
    }
    else if (score_in_all_regions) {
        n_scoring_regions = nreg;
        for (int j=0; j<nreg; j++) {
            f_region.push_back(j);
        }
    }

    if (n_source_regions == nreg && score_primaries)
        egsWarning("=> Warning: Setting n_source_regions = nreg \n"
                   "            effectively supresses classification\n"
                   "            of primaries and secondaries!");
    for (int i = 0; i < n_source_regions; i++) {
        is_source[s_region[i]] = true;
    }

    for (int i = 0; i < n_scoring_regions; i++) {
        is_sensitive[f_region[i]] = true;
        if (f_region[i] > max_reg) {
            max_reg = f_region[i];
        }
    }
}

void EGS_FluenceScoring::describeMe() {

    char buf[128];

    // Report scoring regions in the geometry.
    if (n_scoring_regions == nreg) {
        description += "ALL\n";
    }
    else {
        int start = 0, stop = 0, k = 0, entries = 0;
        bool line_ended;
        /* List up to 100 scoring groups or regions */
        while (k < nreg && entries < REGIONS_ENTRIES) {
            line_ended = false;
            if (is_sensitive[k]) {
                start = k;
                entries++;
                while (is_sensitive[k] && k < nreg) {
                    k++;
                }
                stop = k-1;
                if (start < stop) {
                    sprintf(buf,"%d-%d",start,stop);
                }
                else if (k % REGIONS_PER_LINE) {
                    sprintf(buf," %d",start);
                }
                else {
                    sprintf(buf," %d\n",start);
                    line_ended = true;
                }
                if (entries == REGIONS_ENTRIES) {
                    sprintf(buf,"... %d\n",max_reg);
                    line_ended = true;
                }
                description += buf;
            }
            k++;
        }
        if (!line_ended) {
            description += "\n";
        }
    }

    if (score_primaries) {
        if (source_charge == unknown) {
            source_charge = scoring_charge;
            description += " - Unknown source charge due to multi-particle source\n";
            description += "   and no user input defining source particle type!\n";
            description += "   Defaulting to scoring charge: ";
        }
        else {
            description += " - source charge: ";
        }
        sprintf(buf,"%d\n",source_charge);
        description += buf;
    }

    if (score_spe) {
        description += " - scoring in the ";
        EGS_Float Emin = flu_s ? exp(flu_xmin):flu_xmin,
                  Emax = flu_s ? exp(flu_xmax):flu_xmax;
        sprintf(buf,"%g MeV and %g MeV energy range",Emin,Emax);
        description += buf;
        if (flu_s) {
            description += " on a logarithmic scale \n";
        }
        else {
            description += " on a linear scale \n";
        }
    }

    if (norm_u != 1.0) {
        description += "\n - Non-unity user-requested normalization = ";
        sprintf(buf,"%g\n",norm_u);
        description += buf;
    }

}
/**********************************************
 * Class EGS_PlanarFluence Implementation *
***********************************************/


EGS_PlanarFluence::EGS_PlanarFluence(const string &Name, EGS_ObjectFactory *f) :
    EGS_FluenceScoring(Name,f), hits_field(false), Nx(1), Ny(1),
    fluT_FD(0), fluT_FD_p(0), flu_FD(0), flu_FD_p(0) {
    otype = "EGS_PlanarFluence";
    m_midpoint = EGS_Vector(0,0,5);
    m_R  =  5;
    m_R2 = 25;
    field_type = circle;
    Area = M_PI * m_R2;
    m_normal = EGS_Vector(0,0,1);
    m_d = m_normal*m_midpoint;
}

/*! Destructor.  */
EGS_PlanarFluence::~EGS_PlanarFluence() {

    if (flu) {
        for (int j=0; j<Nx*Ny; j++) {
            delete flu[j];
        }
    }
    if (flu_p) {
        for (int j=0; j<Nx*Ny; j++) {
            delete flu_p[j];
        }
    }
    delete fluT_FD;
    delete fluT_FD_p;
    if (flu_FD) {
        for (int j=0; j<Nx*Ny; j++) {
            delete flu_FD[j];
        }
        delete [] flu_FD;
    }
    if (flu_FD_p) {
        for (int j=0; j<Nx*Ny; j++) {
            delete flu_FD_p[j];
        }
        delete [] flu_FD_p;
    }
}

void EGS_PlanarFluence::setApplication(EGS_Application *App) {

    EGS_AusgabObject::setApplication(App);

    if (!app) {
        return;
    }

    /***********************************************************
       Defaults to charge of application source. This applies most
       of the time, except for bremsstrahlung targets and multiple
       particle sources such as radiactive sources.
       Can be changed via 'source particle' input.
    *************************************************************/
    if (source_charge == unknown) {
        source_charge = (ParticleType)app->sourceCharge();
    }


    // Get the number of regions in the geometry.
    nreg = app->getnRegions();

    /* Initialize arrays with defaults */
    for (int j=0; j<nreg; j++) {

        if (!score_in_all_regions) {
            is_sensitive.push_back(false);
        }
        else {
            is_sensitive.push_back(true);
        }

        is_source.push_back(false);
    }

    /* Flag scoring and source regions*/
    setUpRegionFlags();

    /* Setup fluence scoring arrays, Nx*Ny = 1 for the circle */
    fluT = new EGS_ScoringArray(Nx*Ny);
    if (score_spe) {
        flu  = new EGS_ScoringArray* [Nx*Ny];
        for (int j = 0; j < Nx*Ny; j++) {
            flu[j] = new EGS_ScoringArray(flu_nbin);
        }
    }

    if (score_primaries) {
        fluT_p = new EGS_ScoringArray(Nx*Ny);
        if (score_spe) {
            flu_p  = new EGS_ScoringArray* [Nx*Ny];
            for (int j = 0; j < Nx*Ny; j++) {
                flu_p[j] = new EGS_ScoringArray(flu_nbin);
            }
        }
    }

    if (m_scoring_method != score_crossing) {
        if (scoring_charge != photon) {
            egsWarning("EGS_PlanarFluence: FD scoring is only valid for photons."
                       " Falling back to crossing estimator.\n");
            m_scoring_method = score_crossing;
        }
        else {
            m_i_gmfp = app->getMFPInterpolators();
            m_i_cohe = app->getCoheInterpolators();
            m_rayleigh_on = app->isRayleighOn();
            if (!m_i_gmfp) {
                egsWarning("EGS_PlanarFluence: FD scoring requires EGS_AdvancedApplication."
                           " Falling back to crossing estimator.\n");
                m_scoring_method = score_crossing;
            }
        }
    }

    if (m_scoring_method == score_both) {
        fluT_FD = new EGS_ScoringArray(Nx*Ny);
        if (score_spe) {
            flu_FD = new EGS_ScoringArray* [Nx*Ny];
            for (int j = 0; j < Nx*Ny; j++) {
                flu_FD[j] = new EGS_ScoringArray(flu_nbin);
            }
        }
        if (score_primaries) {
            fluT_FD_p = new EGS_ScoringArray(Nx*Ny);
            if (score_spe) {
                flu_FD_p = new EGS_ScoringArray* [Nx*Ny];
                for (int j = 0; j < Nx*Ny; j++) {
                    flu_FD_p[j] = new EGS_ScoringArray(flu_nbin);
                }
            }
        }
    }

    describeMe();
}

void EGS_PlanarFluence::initScoring(EGS_Input *inp) {

    EGS_Input *pScoringInput;

    if (!inp) {
        egsFatal("AO type %s: null input?\n",otype.c_str());
        return;
    }
    else {
        pScoringInput = inp->takeInputItem("planar scoring");
        if (! pScoringInput) {
            egsFatal("AO type %s: Missing planar scoring input?\n",otype.c_str());
            return;
        }
    }

    EGS_FluenceScoring::initScoring(inp);// common inputs

    /* Planar scoring by default from all regions */
    score_in_all_regions = true;
    EGS_FluenceScoring::getSensitiveRegions(pScoringInput);

    // Specific plane input
    vector<EGS_Float> tmp_field;
    int err2 = pScoringInput->getInput("scoring circle",tmp_field);
    if (err2) {
        int err3 =  pScoringInput->getInput("scoring rectangle",tmp_field);
        if (err3 || tmp_field.size() != 4) {
            egsWarning(
                "\n\n***  Wrong/missing 'scoring rectangle' input "
                "setting it to 10 cm X 10 cm field at origin!\n\n");
            m_midpoint = EGS_Vector();
            ax = 10;
            ay = 10;
            field_type = rectangle;
            Area = ax*ay;
        }
        else {
            EGS_Float xmin = tmp_field[0],xmax = tmp_field[1],
                      ymin = tmp_field[2],ymax = tmp_field[3];
            /* scoring plane location in space */
            m_midpoint = EGS_Vector((xmax+xmin)/2.,(ymax+ymin)/2.,0); // at origin by default
            /* scoring plane normal */
            m_normal = EGS_Vector(0,0,1); // default normal along positive z-axis
            /* define unit vectors on right-handed scoring plane */
            ux = EGS_Vector(1,0,0);
            uy = EGS_Vector(0,1,0);
            /* Request a scoring plane transformation for initial position and orientation */
            EGS_AffineTransform *t = EGS_AffineTransform::getTransformation(pScoringInput);
            if (t) {
                t->rotate(m_normal) ;
                t->transform(m_midpoint);
                t->rotate(ux);
                t->rotate(uy);
                delete t;
            }

            /* get screen resolution */
            vector<int> screen;
            int err4 = pScoringInput->getInput("resolution",screen);
            if (err4) {
                Nx=1;
                Ny=1;
                egsWarning(
                    "\n\n***  Missing/wrong 'resolution' input "
                    "     Scoring in the whole field\n\n");

            }
            else if (screen.size()==1) {
                Nx = screen[0];
                Ny = screen[0];
            }
            else if (screen.size()==2) {
                Nx = screen[0];
                Ny = screen[1];
            }
            else if (screen.size()> 2) {
                Nx = screen[0];
                Ny = screen[1];
                egsWarning(
                    "\n\n***  Too many 'resolution' inputs\n"
                    "     Using first two entries\n\n");

            }

            ax = xmax - xmin;
            ay = ymax-ymin;
            vx = ax/Nx;
            vy = ay/Ny;
            field_type = rectangle;
            Area = vx*vy;
        }
    }
    else if (tmp_field.size() != 4) {
        egsWarning(
            "\n\n***  Wrong/missing 'scoring circle' input "
            "setting it to 10 cm diameter field at origin!\n\n");
        m_midpoint = EGS_Vector();
        m_R  =  5;
        m_R2 = 25;
        field_type = circle;
        Area = M_PI * m_R2;
    }
    else {
        vector<EGS_Float> tmp_normal;
        int err1 = pScoringInput->getInput("scoring plane normal",tmp_normal);
        if (err1 || tmp_normal.size() != 3) {
            egsWarning(
                "\n\n***  Wrong/missing 'scoring plane normal' input. "
                "Set along positive z-axis\n\n");
            m_normal = EGS_Vector(0,0,1);// Default along positive z-axis
        }
        else {
            m_normal = EGS_Vector(tmp_normal[0],tmp_normal[1],
                                  tmp_normal[2]);
            m_normal.normalize();
        }
        m_midpoint = EGS_Vector(tmp_field[0],tmp_field[1],
                                tmp_field[2]);
        m_R = tmp_field[3];
        m_R2 = tmp_field[3]*tmp_field[3];
        field_type = circle;
        Area = M_PI * m_R2;
    }

    m_d = m_normal*m_midpoint;

    EGS_Float cos_theta_min = cos(85.0 * M_PI / 180.0);
    pScoringInput->getInput("cos theta min", cos_theta_min);
    if (cos_theta_min <= 0.0 || cos_theta_min >= 1.0) {
        m_cos_min = 0.0;
    }
    else {
        m_cos_min = cos_theta_min;
    }
}

void EGS_PlanarFluence::describeMe() {
    char buf[128];
    sprintf(buf,"\nPlanar %s fluence scoring\n",particle_name.c_str());
    description =  buf;
    description += "================================\n";

    description += " - scoring field normal      = ";
    sprintf(buf,"(%g %g %g)\n",m_normal.x,m_normal.y,m_normal.z);
    description += buf;
    description += " - scoring field center      = ";
    sprintf(buf,"(%g %g %g)\n",m_midpoint.x,m_midpoint.y,m_midpoint.z);
    description += buf;
    if (field_type == circle) {
        description += " - scoring field radius      = ";
        sprintf(buf,"%g cm\n",m_R);
        description += buf;
    }
    else if (field_type == rectangle) {
        description += " - scoring field             = ";
        sprintf(buf,"%g cm X %g cm\n",ax,ay);
        description += buf;
        description += " - scoring field resolution  = ";
        sprintf(buf,"%d X %d\n",Nx,Ny);
        description += buf;
    }
    description += " - scoring field distance from origin = ";
    sprintf(buf,"%g cm\n",m_d);
    description += buf;

    description += " - estimator                  = ";
    switch (m_scoring_method) {
    case score_FD:   description += "forced detection\n"; break;
    case score_both: description += "crossing + forced detection (separate arrays)\n"; break;
    default:         description += "boundary crossing\n"; break;
    }

    description +=" - scoring from region(s): ";

    EGS_FluenceScoring::describeMe();

}

void EGS_PlanarFluence::score(const EGS_Particle &p, const int &ivoxel) {
    EGS_Float up = p.u*m_normal, aup = fabs(up);
    /**********************************************************
     Prevent large weights from particles very close to plane.
    ***********************************************************/
    if (aup < 0.08) {
        aup = 0.0871557;    // Limit incident angle to 85 degrees
    }
    EGS_Float e = p.q ? p.E - app->getRM() : p.E;
    if (flu_s) {
        e = log(e);   // log scale
    }
    EGS_Float ae;
    int je;
    EGS_Float auxp = p.wt/aup;
    /* Score total fluence in corresponding pixel */
    fluT->score(ivoxel,auxp);
    if (score_primaries && !p.latch) {
        fluT_p->score(ivoxel,auxp);
    }
    /* Score differential fluence in corresponding pixel */
    if (score_spe && e > flu_xmin && e <= flu_xmax) {
        ae = flu_a*e + flu_b;
        /******************************************************
          Algorithm assigns E in [Ei,Ei+1), hence push events
          with E = Emax into last bin (bias?) during scoring.
          Alternatively add extra bin for E = Emax cases.
          Which approach is correct?
        ******************************************************/
        je = min((int)ae,flu_nbin-1);//je = (int) ae;
        /******************************************************/
        if (ivoxel < 0 || ivoxel > Nx*Ny) {
            egsFatal("\n-> Scoring out of bounds, ivoxel = %d\n",ivoxel);
        }
        EGS_ScoringArray *aux   = flu[ivoxel];
        if (je < 0 || je >= flu_nbin) {
            egsFatal("\n-> Scoring out of bounds, ibin = %d ae = %g E = %g MeV\n",je,ae,e);
        }
        aux->score(je,auxp);
        if (score_primaries && !p.latch) {
            flu_p[ivoxel]->score(je,auxp);
        }
    }
}

void EGS_PlanarFluence::scoreFD(const EGS_Particle &p, int ixy, EGS_Float dist) {
    EGS_Vector x = p.x;
    EGS_Vector u = p.u;
    int ireg     = p.ir;
    int newmed   = app->getMedium(ireg);
    int imed     = -1;
    EGS_Float gmfp, sigma = 0, cohfac = 1;
    EGS_Float gle = log(p.E);
    double Lambda = 0;
    EGS_Float ttot = 0;

    while (true) {
        if (imed != newmed) {
            imed = newmed;
            if (imed >= 0 && m_i_gmfp) {
                gmfp = m_i_gmfp[imed].interpolateFast(gle);
                if (m_rayleigh_on && m_i_cohe) {
                    cohfac = m_i_cohe[imed].interpolateFast(gle);
                    gmfp  *= cohfac;
                }
                sigma = 1.0 / gmfp;
            }
            else {
                sigma = 0;
            }
        }
        EGS_Float tstep = 1e35;
        int inew = app->howfar(ireg, x, u, tstep, &newmed);

        if (ttot + tstep >= dist) {
            Lambda += (dist - ttot) * sigma;
            break;
        }
        Lambda += tstep * sigma;
        ttot   += tstep;
        x      += u * tstep;
        if (inew < 0) {
            return;  // exited geometry before reaching plane
        }
        ireg = inew;
    }

    if (Lambda >= 80) {
        return;  // fully attenuated
    }
    EGS_Float aup = fabs(p.u * m_normal);
    if (m_cos_min > 0.0 && aup < m_cos_min) {
        aup = m_cos_min;
        ++m_n_clipped;
    }
    EGS_Float contrib = p.wt * exp(-Lambda) / aup;

    /* In score_both mode write to the separate FD arrays; otherwise to the main arrays. */
    bool both = (m_scoring_method == score_both);
    EGS_ScoringArray *fT   = both ? fluT_FD   : fluT;
    EGS_ScoringArray *fTp  = both ? fluT_FD_p : fluT_p;
    EGS_ScoringArray **fl  = both ? flu_FD    : flu;
    EGS_ScoringArray **flp = both ? flu_FD_p  : flu_p;
    EGS_Float &rtot = both ? m_tot_FD   : m_tot;
    EGS_Float &rpri = both ? m_primary_FD : m_primary;

    rtot += p.wt;
    fT->score(ixy, contrib);
    if (score_primaries && !p.latch) {
        rpri += p.wt;
        fTp->score(ixy, contrib);
    }
    if (score_spe) {
        EGS_Float e = p.q ? p.E - app->getRM() : p.E;
        if (flu_s) {
            e = log(e);
        }
        if (e > flu_xmin && e <= flu_xmax) {
            int je = min((int)(flu_a*e + flu_b), flu_nbin - 1);
            fl[ixy]->score(je, contrib);
            if (score_primaries && !p.latch) {
                flp[ixy]->score(je, contrib);
            }
        }
    }
}

int EGS_PlanarFluence::hitsField(const EGS_Particle &p, EGS_Float *dist) {
    if (field_type==circle) {
        EGS_Float xp = p.x*m_normal, up = p.u*m_normal;
        if ((up > 0 && m_d > xp) ||
                (up < 0 && m_d < xp)) {
            EGS_Float t = (m_d - xp)/up;
            EGS_Vector x1(p.x + p.u*t - m_midpoint);
            if (dist) {
                *dist = t;
            }
            return x1.length2() < m_R*m_R ? 0:-1;
        }
        return -1;
    }
    else if (field_type == rectangle) {
        EGS_Float xp = p.x*m_normal, up = p.u*m_normal;
        if ((up > 0 && m_d > xp) || (up < 0 && m_d < xp)) {
            EGS_Float t = (m_d - xp)/up; // distance to plane along u
            EGS_Vector x1(p.x + p.u*t - m_midpoint);// vector on scoring plane
            EGS_Float xcomp = x1*ux;// x-direction component
            EGS_Float ycomp = x1*uy;// y-direction component
            xcomp = 2*xcomp + ax;
            ycomp = 2*ycomp + ay;
            if (xcomp > 0 && xcomp < 2*ax &&
                    ycomp > 0 && ycomp < 2*ay) {
                int i = int(xcomp/(2*vx)),
                    j = int(ycomp/(2*vy)),
                    k = i + j*Nx;
                if (dist) {
                    *dist = t;
                }
                return k;
            }
        }
        return -1;
    }
    else {
        return -1;
    }
}

void EGS_PlanarFluence::ouputPlanarFluence(EGS_ScoringArray *fT, const double &norma) {
    double fe,dfe,dfer;
    int count = 0;
    int ix_digits = getDigits(Nx);
    int iy_digits = getDigits(Ny);
    int xy_digits = getDigits(Nx*Ny);

    if (field_type == circle) {
        egsInformation("\n  pixel#    Flu/(MeV*cm2)   DFlu/(MeV*cm2)\n"
                       "-----------------------------------------------------\n");
    }
    else {
        egsInformation("\n  %*s %*s pixel#    Flu/(MeV*cm2)   DFlu/(MeV*cm2)\n"
                       "-----------------------------------------------------\n",
                       iy_digits,"iy",ix_digits,"ix",&count);
    }
    if (field_type == circle) {
        int k = 0;
        egsInformation("   %*d      ",xy_digits,k);
        fT->currentResult(k,fe,dfe);
        if (fe > 0) {
            dfer = 100*dfe/fe;
        }
        else {
            dfer = 100;
        }
        egsInformation(" %10.4le +/- %10.4le [%-7.3lf\%]\n",fe*norma,dfe*norma,dfer);
    }
    else {
        for (int j=0; j<Ny; j++) {
            for (int i=0; i<Nx; i++) {
                int k = i + j*Nx;
                egsInformation("   %*d  %*d  %*d      ",iy_digits,j,ix_digits,i,xy_digits,k);
                fT->currentResult(k,fe,dfe);
                if (fe > 0) {
                    dfer = 100*dfe/fe;
                }
                else {
                    dfer = 100;
                }
                egsInformation(" %10.4le +/- %10.4le [%-7.3lf\%]\n",fe*norma,dfe*norma,dfer);
            }
        }
    }
}

void EGS_PlanarFluence::ouputResults() {

    EGS_Float src_norm = 1.0,          // default to number of histories in this run
              Fsrc = app->getFluence();// Fluence or number of primary histories
    egsInformation("\n\n last case = %lld source particles or fluence = %g\n\n",
                   current_ncase, Fsrc);

    if (Fsrc) {
        src_norm = Fsrc/current_ncase;    // fluence or primary histories per histories run
    }

    string normLabel = src_norm == 1 ? "history" : "MeV-1 cm-2";
    string src_type = app->sourceType();
    if (src_type == "EGS_BeamSource") {
        normLabel = "primary history";
        egsInformation("\n\n %s normalization = %g (primary histories per particle)\n\n",
                       src_type.c_str(), src_norm);
    }
    else if (src_type == "EGS_CollimatedSource" ||
             (src_type == "EGS_ParallelBeam" && src_norm != 1)) {
        egsInformation("\n\n %s normalization = %g (fluence per particle)\n\n",
                       src_type.c_str(), src_norm);
    }
    else {
        egsInformation("\n\n %s normalization = %g (histories per particle)\n\n",
                       src_type.c_str(), src_norm);

    }


    double norm = 1.0/src_norm;  //per fluence or particle depending on source
    norm /= Area;         //per unit area
    norm *= norm_u;       // times user-requested normalization

    //egsInformation("  Normalization = %g\n",norm);

    egsInformation("\n\n            Integral fluence\n"
                   "            ================\n\n");

    if (m_scoring_method == score_both) {
        egsInformation("\n\n  [crossing]  Total %s fluence\n", particle_name.c_str());
        ouputPlanarFluence(fluT, norm);
        if (score_primaries) {
            egsInformation("\n\n  [crossing]  Primary fluence\n");
            ouputPlanarFluence(fluT_p, norm);
        }
        egsInformation("\n\n  [FD]        Total %s fluence\n", particle_name.c_str());
        ouputPlanarFluence(fluT_FD, norm);
        if (score_primaries) {
            egsInformation("\n\n  [FD]        Primary fluence\n");
            ouputPlanarFluence(fluT_FD_p, norm);
        }
    }
    else {
        egsInformation("\n\n               Total %s fluence\n", particle_name.c_str());
        ouputPlanarFluence(fluT, norm);
        if (score_primaries) {
            egsInformation("\n\n                   Primary fluence\n");
            ouputPlanarFluence(fluT_p, norm);
        }
    }

    if (score_spe) {
        norm *= flu_a; // per unit bin width
        if (m_scoring_method == score_both) {
            outputSpectrum(flu,    flu_p,    norm, "_cross_" + particle_name + ".agr");
            outputSpectrum(flu_FD, flu_FD_p, norm, "_FD_"    + particle_name + ".agr");
        }
        else {
            outputSpectrum(flu, flu_p, norm, "_" + particle_name + ".agr");
        }
    }

}

void EGS_PlanarFluence::outputSpectrum(EGS_ScoringArray **fl_set,
                                       EGS_ScoringArray **flp_set,
                                       double norm,
                                       const string &suffix) const {
    string spe_name = app->constructIOFileName(suffix.c_str(), true);
    ofstream spe_output(spe_name.c_str(), ios::out);
    if (!spe_output) {
        egsFatal("\n EGS_PlanarFluence: Error: Failed to open file %s\n", spe_name.c_str());
        exit(1);
    }
    spe_output << "# " << particle_name << " fluence\n";
    spe_output << "# \n";
    spe_output << "@    legend 0.2, 0.8\n";
    spe_output << "@    legend box linestyle 0\n";
    spe_output << "@    legend font 4\n";
    spe_output << "@    xaxis  label \"energy / MeV\"\n";
    spe_output << "@    xaxis  label char size 1.560000\n";
    spe_output << "@    xaxis  label font 4\n";
    spe_output << "@    xaxis  ticklabel font 4\n";
    spe_output << "@    yaxis  label \"fluence / MeV\\S-1\\Ncm\\S-2\"\n";
    spe_output << "@    yaxis  label char size 1.560000\n";
    spe_output << "@    yaxis  label font 4\n";
    spe_output << "@    yaxis  ticklabel font 4\n";
    spe_output << "@    title \"" << particle_name << " fluence\"\n";
    spe_output << "@    title font 4\n";
    spe_output << "@    title size 1.500000\n";
    spe_output << "@    subtitle \"for each scoring region\"\n";
    spe_output << "@    subtitle font 4\n";
    spe_output << "@    subtitle size 1.000000\n";

    egsInformation("\n\n            Differential fluence\n"
                   "            ====================\n\n");

    int i_graph = 0;
    double fe, dfe;
    for (int j=0; j<Ny; j++) {
        for (int i=0; i<Nx; i++) {
            int k = i + j*Nx;
            if (verbose) {
                egsInformation("\nPixel # %d :", k);
            }
            spe_output << "@    s" << i_graph << " errorbar linestyle 0\n";
            spe_output << "@    s" << i_graph << " legend \"Voxel # " << k << "\"\n";
            spe_output << "@target G0.S" << i_graph << "\n";
            spe_output << "@type xydy\n";
            if (verbose) {
                egsInformation("\n\n           Total \n\n");
                egsInformation("\n   Emid/MeV    Flu/(MeV*cm2)   DFlu/(MeV*cm2)\n"
                               "---------------------------------------------\n");
            }
            for (int l=0; l<flu_nbin; l++) {
                fl_set[k]->currentResult(l, fe, dfe);
                EGS_Float e = (l+0.5-flu_b)/flu_a;
                if (flu_s) { e = exp(e); }
                spe_output << e << " " << fe*norm << " " << dfe*norm << "\n";
                if (verbose) egsInformation("%11.6f  %14.6e  %14.6e\n", e, fe*norm, dfe*norm);
            }
            spe_output << "&\n";

            if (score_primaries) {
                if (verbose) {
                    egsInformation("\n\n           Primary\n\n");
                    egsInformation("\n   Emid/MeV    Flu/(MeV*cm2)   DFlu/(MeV*cm2)\n"
                                   "---------------------------------------------\n");
                }
                spe_output << "@    s" << ++i_graph << " errorbar linestyle 0\n";
                spe_output << "@    s" << i_graph << " legend \"Voxel # " << k << " (primary)\"\n";
                spe_output << "@target G0.S" << i_graph << "\n";
                spe_output << "@type xydy\n";
                for (int l=0; l<flu_nbin; l++) {
                    flp_set[k]->currentResult(l, fe, dfe);
                    EGS_Float e = (l+0.5-flu_b)/flu_a;
                    if (flu_s) { e = exp(e); }
                    spe_output << e << " " << fe*norm << " " << dfe*norm << "\n";
                    if (verbose) egsInformation("%11.6f  %14.6e  %14.6e\n", e, fe*norm, dfe*norm);
                }
                spe_output << "&\n";
            }
            i_graph++;
        }
    }
    spe_output.close();
}

void EGS_PlanarFluence::reportResults() {
    egsInformation("\nFluence Scoring (%s)\n",name.c_str());
    egsInformation("======================================================\n");
    if (m_scoring_method == score_both) {
        egsInformation("   [crossing] Total %ss reaching field:   %g\n",particle_name.c_str(),m_tot);
        egsInformation("   [crossing] Primary %ss reaching field: %g\n",particle_name.c_str(),m_primary);
        egsInformation("   [FD]       Total aimed at field:        %g\n",m_tot_FD);
        egsInformation("   [FD]       Primary aimed at field:      %g\n",m_primary_FD);
    }
    else {
        egsInformation("   Total %ss reaching field:       %g\n",particle_name.c_str(),m_tot);
        egsInformation("   Primary %ss reaching field:     %g\n",particle_name.c_str(),m_primary);
    }
    egsInformation("======================================================\n");

    ouputResults();

}

bool EGS_PlanarFluence::storeState(ostream &data) const {
    if (!egsStoreI64(data,current_ncase)) {
        return false;
    }
    data << endl;
    data << m_tot << " " << m_primary;
    data << endl;
    if (!data.good()) {
        return false;
    }

    if (!fluT->storeState(data)) {
        return false;
    }

    if (score_spe) {
        for (int j=0; j<Nx*Ny; j++) {
            if (!flu[j]->storeState(data)) {
                return false;
            }
        }
    }

    if (score_primaries) {
        if (!fluT_p->storeState(data)) {
            return false;
        }
        if (score_spe) {
            for (int j=0; j<Nx*Ny; j++) {
                if (!flu_p[j]->storeState(data)) {
                    return false;
                }
            }
        }
    }

    if (m_scoring_method == score_both) {
        data << m_tot_FD << " " << m_primary_FD << endl;
        if (!data.good()) {
            return false;
        }
        if (!fluT_FD->storeState(data)) {
            return false;
        }
        if (score_spe) {
            for (int j=0; j<Nx*Ny; j++) {
                if (!flu_FD[j]->storeState(data)) {
                    return false;
                }
            }
        }
        if (score_primaries) {
            if (!fluT_FD_p->storeState(data)) {
                return false;
            }
            if (score_spe) {
                for (int j=0; j<Nx*Ny; j++) {
                    if (!flu_FD_p[j]->storeState(data)) {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

bool  EGS_PlanarFluence::setState(istream &data) {
    if (!egsGetI64(data,current_ncase)) {
        return false;
    }
    data >> m_tot >> m_primary;
    if (!data.good()) {
        return false;
    }

    if (!fluT->setState(data)) {
        return false;
    }

    if (score_spe) {
        for (int j=0; j<Nx*Ny; j++) {
            if (!flu[j]->setState(data)) {
                return false;
            }
        }
    }

    if (score_primaries) {

        if (!fluT_p->setState(data)) {
            return false;
        }

        if (score_spe) {
            for (int j=0; j<Nx*Ny; j++) {
                if (!flu_p[j]->setState(data)) {
                    return false;
                }
            }
        }

    }

    if (m_scoring_method == score_both) {
        data >> m_tot_FD >> m_primary_FD;
        if (!data.good()) {
            return false;
        }
        if (!fluT_FD->setState(data)) {
            return false;
        }
        if (score_spe) {
            for (int j=0; j<Nx*Ny; j++) {
                if (!flu_FD[j]->setState(data)) {
                    return false;
                }
            }
        }
        if (score_primaries) {
            if (!fluT_FD_p->setState(data)) {
                return false;
            }
            if (score_spe) {
                for (int j=0; j<Nx*Ny; j++) {
                    if (!flu_FD_p[j]->setState(data)) {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

bool  EGS_PlanarFluence::addState(istream &data) {
    EGS_I64 tmp_case;
    if (!egsGetI64(data,tmp_case)) {
        return false;
    }
    current_ncase += tmp_case;
    EGS_Float tmp_tot, tmp_primary;
    data >> tmp_tot >> tmp_primary;
    if (!data.good()) {
        return false;
    }
    m_primary += tmp_primary;
    m_tot     += tmp_tot;

    EGS_ScoringArray tgT(Nx*Ny);
    if (!tgT.setState(data)) {
        return false;
    }
    (*fluT) += tgT;

    if (score_spe) {
        EGS_ScoringArray tg(flu_nbin);
        for (int j=0; j<Nx*Ny; j++) {
            if (!tg.setState(data)) {
                return false;
            }
            (*flu[j]) += tg;
        }
    }

    if (score_primaries) {

        EGS_ScoringArray tgT_p(Nx*Ny);
        if (!tgT_p.setState(data)) {
            return false;
        }
        (*fluT_p) += tgT_p;

        if (score_spe) {
            EGS_ScoringArray tg_p(flu_nbin);
            for (int j=0; j<Nx*Ny; j++) {
                if (!tg_p.setState(data)) {
                    return false;
                }
                (*flu_p[j]) += tg_p;
            }
        }
    }

    if (m_scoring_method == score_both) {
        EGS_Float tmp_tot_FD, tmp_primary_FD;
        data >> tmp_tot_FD >> tmp_primary_FD;
        if (!data.good()) {
            return false;
        }
        m_tot_FD     += tmp_tot_FD;
        m_primary_FD += tmp_primary_FD;

        EGS_ScoringArray tgT_FD(Nx*Ny);
        if (!tgT_FD.setState(data)) {
            return false;
        }
        (*fluT_FD) += tgT_FD;

        if (score_spe) {
            EGS_ScoringArray tg_FD(flu_nbin);
            for (int j=0; j<Nx*Ny; j++) {
                if (!tg_FD.setState(data)) {
                    return false;
                }
                (*flu_FD[j]) += tg_FD;
            }
        }

        if (score_primaries) {
            EGS_ScoringArray tgT_FD_p(Nx*Ny);
            if (!tgT_FD_p.setState(data)) {
                return false;
            }
            (*fluT_FD_p) += tgT_FD_p;

            if (score_spe) {
                EGS_ScoringArray tg_FD_p(flu_nbin);
                for (int j=0; j<Nx*Ny; j++) {
                    if (!tg_FD_p.setState(data)) {
                        return false;
                    }
                    (*flu_FD_p[j]) += tg_FD_p;
                }
            }
        }
    }

    return true;
}


/**********************************************
 * Class EGS_VolumetricFluence Implementation *
***********************************************/


EGS_VolumetricFluence::EGS_VolumetricFluence(const string &Name, EGS_ObjectFactory *f) :
    EGS_FluenceScoring(Name,f), flu_stpwr(stpwr)
#ifdef DEBUG
    ,one_bin(0), multi_bin(0), max_step(-100.0), n_step_bins(10000)
#endif
{
    otype = "EGS_VolumetricFluence";
}

/*! Destructor.  */
EGS_VolumetricFluence::~EGS_VolumetricFluence() {

    if (flu) {
        for (int j=0; j<n_scoring_regions; j++) {
            delete flu[j];
        }
    }
    if (flu_p) {
        for (int j=0; j<n_scoring_regions; j++) {
            delete flu_p[j];
        }
    }

#ifdef DEBUG
    if (binDist) {
        delete binDist;
    }
    if (scoring_charge) {
        delete stepDist;
        delete relStepDiff;
    }
#endif

}

void EGS_VolumetricFluence::setApplication(EGS_Application *App) {

    EGS_AusgabObject::setApplication(App);

    if (!app) {
        return;
    }

    /***********************************************************
       Defaults to charge of application source. This applies most
       of the time, except for bremsstrahlung targets and multiple
       particle sources such as radiactive sources.
       Can be changed via 'source particle' input.
    *************************************************************/
    if (source_charge == unknown) {
        source_charge = (ParticleType)app->sourceCharge();
    }

    // Get the number of regions in the geometry.
    nreg = app->getnRegions();

    /* Initialize arrays with defaults */
    for (int j=0; j<nreg; j++) {
        is_sensitive.push_back(false);
        is_source.push_back(false);
        volume.push_back(vol_list[0]);// set to either 1.0 or first volume entered
    }

    /* Flag scoring and source regions*/
    setUpRegionFlags();

    /* Update arrays with user inputs */
    for (int i = 0; i < n_scoring_regions; i++) {
        if (i < vol_list.size()) {
            volume[f_region[i]] = vol_list[i];
        }
    }

    /* Setup fluence scoring arrays */
    fluT = new EGS_ScoringArray(nreg);
    if (score_spe) {
        flu  = new EGS_ScoringArray* [nreg];
        for (int j = 0; j < nreg; j++) {
            if (is_sensitive[j]) {
                flu[j] = new EGS_ScoringArray(flu_nbin);
            }
        }
    }
    if (score_primaries) {
        fluT_p = new EGS_ScoringArray(nreg);
        if (score_spe) {
            flu_p  = new EGS_ScoringArray* [nreg];
            for (int j = 0; j < nreg; j++) {
                if (is_sensitive[j]) {
                    flu_p[j] = new EGS_ScoringArray(flu_nbin);
                }
            }
        }
    }
#ifdef DEBUG
    binDist = new EGS_ScoringArray(flu_nbin);
    int imed_max_range;
#endif

    EGS_Float flu_Emin = flu_s ? exp(flu_xmin) : flu_xmin,
              flu_Emax = flu_s ? exp(flu_xmax) : flu_xmax;
    EGS_Float bw = flu_s ?(log(flu_Emax / flu_Emin))/flu_nbin :
                   (flu_Emax - flu_Emin) /flu_nbin;
    flu_a_i = bw;
    flu_a = 1.0/bw;

    /* Initialize data required to score charged particle fluence */
    if (scoring_charge) {
        EGS_Float expbw;
        /* Pre-calculated values for faster evaluation on log scale */
        if (flu_s) {
            expbw   = exp(bw); // => (Emax/Emin)^(1/nbin)
            r_const = 1/(expbw-1);
            DE      = new EGS_Float [flu_nbin];
            a_const = new EGS_Float [flu_nbin];
            for (int i = 0; i < flu_nbin; i++) {
                DE[i]      = flu_Emin*pow(expbw,i)*(expbw-1);
                a_const[i] = 1/flu_Emin*pow(1/expbw,i);
            }
        }
        /* Do not score below ECUT - PRM */
        if (flu_Emin < app->getEcut() - app->getRM()) {
            flu_Emin = app->getEcut() - app->getRM() ;
            /* Decrease number of bins, preserve bin width */
            flu_nbin = flu_s ?
                       ceil((log(flu_Emax / flu_Emin))/bw) :
                       ceil((flu_Emax - flu_Emin) /bw);
        }

        /* Pre-calculated values for faster 1/stpwr evaluation */
        if (flu_stpwr) {

            int n_media = app->getnMedia();

            EGS_Float lnE, lnEmin, lnEmax, lnEmid;

            i_dedx = new EGS_Interpolator [n_media];// stp powers
            dedx_i = new EGS_Interpolator [n_media];// its inverse
            for (int j = 0; j < n_media; j++) {
                /* Gets charged partcle stopping powers based on the scoring charge */
                i_dedx[j] = *(app->getDEDX(j, scoring_charge));
                EGS_Float Emin = i_dedx[j].getXmin();
                EGS_Float Emax = i_dedx[j].getXmax();
                int n = 1 + i_dedx[j].getIndex(Emax);// getIndex returns lower bin limit?
                EGS_Float bwidth = (Emax - Emin)/n;
#ifdef DEBUG
                egsInformation("---> stpwr in %s : Emin=%g Emax=%g n=%d bw=%g\n",
                               app->getMediumName(j), exp(Emin), exp(Emax), n, bwidth);
#endif
                int nbins = n + 1;
                EGS_Float spwr_i[nbins];
                for (int k = 0; k < nbins; k++) {
                    spwr_i[k] = 1.0 / i_dedx[j].interpolate(Emin+k*bwidth);
                }
                dedx_i[j].initialize(nbins, Emin, Emax, spwr_i);
#ifdef DEBUG
                Emin = dedx_i[j].getXmin();
                Emax = dedx_i[j].getXmax();
                n = 1 + dedx_i[j].getIndex(Emax);// getIndex returns lower bin limit?
                bwidth = (Emax - Emin)/n;
                egsInformation("---> 1/stpwr in %s : lnEmin=%g lnEmax=%g n=%d bw=%g index(Emax)=%d\n",
                               app->getMediumName(j), Emin, Emax, n, bwidth, dedx_i[j].getIndexFast(Emax));
                for (int k = 0; k < nbins; k++) {
                    egsInformation(" L(%g MeV) = %g MeV/cm 1/L = %g cm/MeV\n",
                                   exp(Emin+k*bwidth),
                                   i_dedx[j].interpolate(Emin + k*bwidth),
                                   dedx_i[j].interpolate(Emin + k*bwidth));
                }
#endif
            }

            /* Determine stpwr at middle of each fluence scoring bin */
            lnEmin  = flu_s ? log(0.5*flu_Emin*(expbw+1)) : 0;
            Lmid_i  = new EGS_Float [flu_nbin*n_media];
            for (int j = 0; j < n_media; j++) {
#ifdef DEBUG
                EGS_Float med_max_step = 0, logEmax, logEmin;
#endif
                for (int i = 0; i < flu_nbin; i++) {
                    lnEmid = flu_s ? lnEmin + i*bw : log(flu_Emin+bw*(i+0.5));
                    Lmid_i[i+j*flu_nbin] = 1/i_dedx[j].interpolate(lnEmid);
                    //egsInformation(" 1/L(%g MeV) = %g cm/MeV\n",exp(lnEmid),Lmid_i[i+j*flu_nbin]);
#ifdef DEBUG
                    // Set max_step to maximum range
                    if (i > 0) {
                        logEmin = flu_s ? lnEmin + (i-1)*bw : log(flu_Emin+bw*(i-1));
                        logEmax = flu_s ? lnEmin +     i*bw : log(flu_Emin+bw*i);
                        if (i_dedx[j].interpolate(logEmax) < i_dedx[j].interpolate(logEmin)) {
                            med_max_step += 1.02*(exp(logEmax) - exp(logEmin))/i_dedx[j].interpolate(logEmax);
                        }
                        else {
                            med_max_step += 1.02*(exp(logEmax) - exp(logEmin))*i_dedx[j].interpolate(logEmin);
                        }
                    }
#endif
                }
#ifdef DEBUG
                if (med_max_step > max_step) {
                    max_step = med_max_step;
                    imed_max_range = j;
                }
#endif
            }
        }
    }

#ifdef DEBUG
    EGS_Float RCSDA = max_step;
    //max_step /= 4.0; // Set to a quarter of the CSDA range
    //if ( max_step > 1.0 ) max_step = 1.0;
    max_step = 1.0;// reset to 1 cm
    step_a = n_step_bins/max_step;
    step_b = 0;
    relStepDiff = new EGS_ScoringArray(n_step_bins);
    stepDist    = new EGS_ScoringArray(n_step_bins);
    eCases = 0;
    egsInformation("\n===> RCSDA(%s) = %g cm  for Emax = %g MeV, max_step = %g cm bin width = %g cm\n",
                   app->getMediumName(imed_max_range), RCSDA,
                   flu_s ? exp(flu_Emax) : flu_Emax, max_step, 1.0/step_a);
#endif
    describeMe();
}

/*
   Takes user inputs and sets up simulation parameters not requiring
   invoking an application method. This is later done in setApplication.
*/
void EGS_VolumetricFluence::initScoring(EGS_Input *inp) {

    EGS_Input *vScoringInput;

    if (!inp) {
        egsFatal("AO type %s: null input?\n",otype.c_str());
        return;
    }
    else {
        vScoringInput = inp->takeInputItem("volumetric scoring");
        if (! vScoringInput) {
            egsFatal("AO type %s: Missing volumetric scoring input?\n",otype.c_str());
            return;
        }
    }

    EGS_FluenceScoring::initScoring(inp);// common inputs

    /* Volumetric scoring by default turned off */
    score_in_all_regions = false;
    EGS_FluenceScoring::getSensitiveRegions(vScoringInput);

    /* get region volume[s] in g/cm3 */
    vector <EGS_Float> v_in;
    vScoringInput->getInput("volumes",v_in);

    //================================================
    // Check if one volume for each group requested.
    // Otherwise pass volumes read and if there is
    // a mismatch, then the first volume element
    // or 1g/cm3 will be used.
    //=================================================
    if (! score_in_all_regions && v_in.size() == f_start.size()) {
        // groups of regions with same volume
        for (int i=0; i<f_start.size(); i++) {
            int i_r = f_start[i], f_r = f_stop[i];
            for (int ireg=i_r; ireg<=f_r; ireg++) {
                vol_list.push_back(v_in[i]);
            }
        }
    }
    else if (v_in.size()) {
        vol_list = v_in;
    }
    else {
        vol_list.push_back(1.0);
    }

    /* Initialize data required to score charged particle fluence */
    if (scoring_charge) {
        vector<string> method;
        method.push_back("flurz");
        method.push_back("stpwr");   // 3rd order
        method.push_back("stpwrO5"); // 5th order
        flu_stpwr = eFluType(vScoringInput->getInput("method",method,1));

    }

}

void EGS_VolumetricFluence::describeMe() {
    char buf[128];
    sprintf(buf,"\nVolumetric %s fluence scoring\n",particle_name.c_str());
    description =  buf;
    description += "===================================\n";

    description +=" - scoring in region(s): ";

    EGS_FluenceScoring::describeMe();

    if (scoring_charge) {
        if (flu_stpwr) {
            if (flu_stpwr == stpwr) {
                description += "   O(eps^3) approach: accounts for change in stpwr\n";
                description +=                "   along the step with eps=edep/Emid\n";
            }
            else if (flu_stpwr == stpwrO5) {
                description += "   O(eps^5) approach: accounts for change in stpwr\n";
                description += "   along the step with eps=edep/Emid\n";
            }
        }
        else {
            description += "   Fluence calculated a-la-FLURZ using Lave=EDEP/TVSTEP.\n";
        }
    }

    // if (norm_u != 1.0) {
    //     description += "\n - Non-unity user-requested normalization = ";
    //     sprintf(buf,"%g\n",norm_u);
    //     description += buf;
    // }

}

void EGS_VolumetricFluence::ouputVolumetricFluence(EGS_ScoringArray *fT, const double &norma) {
    double fe,dfe,dfer;
    int count = 0;
    int ir_digits = getDigits(nreg);

    //egsInformation("-> norma = %10.4le\n", norma);

    egsInformation("\n  region#    Flu/(MeV*cm2)   DFlu/(MeV*cm2)\n"
                   "-----------------------------------------------------\n");
    for (int k=0; k<nreg; k++) {
        if (!is_sensitive[k]) {
            continue;
        }
        double norm = norma/volume[k];               //per volume
        egsInformation("  %*d      ",ir_digits,k);
        fT->currentResult(k,fe,dfe);
        if (fe > 0) {
            dfer = 100*dfe/fe;
        }
        else {
            dfer = 100;
        }
        //egsInformation(" %10.4le +/- %10.4le [%-7.3lf%] %10.4le\n",fe,dfe,dfer,norm);
        egsInformation(" %10.4le +/- %10.4le [%-7.3lf%]\n",fe*norm,dfe*norm,dfer);
    }
}

void EGS_VolumetricFluence::ouputResults() {


    EGS_Float src_norm = 1.0,          // default to number of histories in this run
              Fsrc = app->getFluence();// Fluence or number of primary histories
    egsInformation("\n\n last case = %lld source particles or fluence = %g\n\n",
                   current_ncase, Fsrc);

    if (Fsrc) {
        src_norm = Fsrc/current_ncase;    // fluence or primary histories per histories run
    }

    string normLabel = src_norm == 1 ? "history" : "MeV-1 cm-2";
    string src_type = app->sourceType();
    if (src_type == "EGS_BeamSource") {
        normLabel = "primary history";
        egsInformation("\n\n %s normalization = %g (primary histories per particle)\n\n",
                       src_type.c_str(), src_norm);
    }
    else if (src_type == "EGS_CollimatedSource" ||
             (src_type == "EGS_ParallelBeam" && src_norm != 1)) {
        egsInformation("\n\n %s normalization = %g (fluence per particle)\n\n",
                       src_type.c_str(), src_norm);
    }
    else {
        egsInformation("\n\n %s normalization = %g (histories per particle)\n\n",
                       src_type.c_str(), src_norm);

    }
#ifdef DEBUG
    EGS_Float fbins, d_fbins;
    egsInformation("\nNumber of covered bins distribution\n"
                   "--------------------------------------\n");

    int tot_bins = one_bin + multi_bin;
    egsInformation("\none_bin = %d [%-7.3lf\%] multi_bin = %d [%-7.3lf\%]\n",
                   one_bin, 100.0*one_bin/tot_bins, multi_bin,100.0*multi_bin/tot_bins);

    egsInformation("\n  # bins        freq         unc           percentage  \n");
    int bdigits = getDigits(flu_nbin);
    for (int i=0; i<flu_nbin; i++) {
        binDist->currentResult(i,fbins, d_fbins);
        if (fbins) {
            d_fbins = 100.0*d_fbins/fbins;
            fbins = current_ncase*fbins;
            egsInformation("   %*d        %11.2f  [%-7.3lf\%]     %11.2f \%\n",
                           bdigits, i+1, fbins, d_fbins, 100.*fbins/tot_bins);
        }
    }

    if (scoring_charge) {
        egsInformation("\nDistribution of ratio between computed and taken steps\n"
                       "------------------------------------------------------\n"
                       "   (Omitted bins with differences less than 0.01\%)\n");
        /*
         //egsInformation(" Relative step difference: %8.4f%% +/- %8.4f%% [%8.4lf%%]\n",
         //egsInformation(" Relative step ratio: %10.4le +/- %10.4le [%8.4lf%%]\n",
         */

        EGS_Float step_diff, step_diff_std, step_diff_err, stepMid;
        EGS_Float f_scores, f_scores_std;
        int idigits = getDigits(eCases);
        egsInformation("\n   step / cm        # scores      ratio       rel unc\n"
                       "---------------------------------------------------------\n");
        for (int i=0; i < n_step_bins; i++) {
            stepDist->currentResult(i, f_scores, f_scores_std);
            if (f_scores > 0) {
                stepMid = (i + 0.5 - step_b)/step_a;
                relStepDiff->currentResult(i, step_diff, step_diff_std);
                if (step_diff > 0) {
                    step_diff_err = 100*step_diff_std/step_diff;
                }
                else {
                    step_diff_err = 100;
                }
                if (abs(step_diff/f_scores - 1.0) > 0.0001D+0)
                    egsInformation("   %10.4le      %*d      %10.4le [%8.4lf\%]\n",
                                   stepMid, idigits, int(f_scores*current_ncase), step_diff/f_scores, step_diff_err);
            }
        }

    }

#endif

    EGS_Float norm  = 1.0/src_norm;              // per particle or fluence
    norm *= norm_u;                    // user-requested normalization

    egsInformation("\n\n                 Integral fluence output\n"
                   "                 =======================\n\n");

    egsInformation("\n\n                 Total %s fluence\n", particle_name.c_str());
    ouputVolumetricFluence(fluT, norm);

    if (score_primaries) {
        egsInformation("\n\n                   Primary fluence\n");
        ouputVolumetricFluence(fluT_p, norm);
    }

    if (verbose) {
        egsInformation("\nbw = %g nbins = %d\n", flu_a_i, flu_nbin);
    }

    if (score_spe) {
        string suffix = "_" + particle_name + ".agr";
        string spe_name = app->constructIOFileName(suffix.c_str(),true);
        ofstream spe_output(spe_name.c_str(),ios::out);
        if (!spe_output) {
            egsFatal("\n EGS_VolumetricFluence: Error: Failed to open file %s\n",spe_name.c_str());
            exit(1);
        }

        spe_output << "# Volumetric " << particle_name.c_str() << " fluence \n";
        spe_output << "# \n";
        spe_output << "@    legend 0.2, 0.8\n";
        spe_output << "@    legend box linestyle 0\n";
        spe_output << "@    legend font 4\n";
        spe_output << "@    xaxis  label \"energy / MeV\"\n";
        spe_output << "@    xaxis  label char size 1.560000\n";
        spe_output << "@    xaxis  label font 4\n";
        spe_output << "@    xaxis  ticklabel font 4\n";
        if (src_norm == 1 || normLabel == "primary history") {
            spe_output << "@    yaxis  label \"fluence / MeV\\S-1\\Ncm\\S-2\"\n";
        }
        else {
            spe_output << "@    yaxis  label \"fluence / MeV\\S-1\"\n";
        }
        spe_output << "@    yaxis  label char size 1.560000\n";
        spe_output << "@    yaxis  label font 4\n";
        spe_output << "@    yaxis  ticklabel font 4\n";
        spe_output << "@    title \""<< particle_name.c_str() << " fluence" <<"\"\n";
        spe_output << "@    title font 4\n";
        spe_output << "@    title size 1.500000\n";
        spe_output << "@    subtitle \"for each scoring region\"\n";
        spe_output << "@    subtitle font 4\n";
        spe_output << "@    subtitle size 1.000000\n";

        if (verbose)
            egsInformation("\n\n                 Differential fluence output\n"
                           "                 =============================\n\n");
        int i_graph = 0;
        double fe, dfe;
        norm *= scoring_charge ? 1 : flu_a;//per bin width <- implicit for charged particles!
        // Loop through the number of regions in the geometry.
        for (int j = 0; j < nreg; j++) {

            if (!is_sensitive[j]) {
                continue;
            }

            double norma = norm/volume[j];                 //per volume

            egsInformation("region # %d : ",j);

            if (verbose) {
                egsInformation("Volume[%d] = %g", j, volume[j]);
                egsInformation(" Normalization = Ncase/Fsrc/V = %g\n",norma);
            }
            else {
                egsInformation(" See Grace plot file %s\n", spe_name.c_str());
            }

            if (verbose) {
                egsInformation("\nTotal fluence:\n");
            }
            spe_output<<"@    s"<< i_graph <<" errorbar linestyle 0\n";
            spe_output<<"@    s"<< i_graph <<" legend \""<< "total (ir # " << j <<")\"\n";
            spe_output<<"@target G0.S"<< i_graph <<"\n";
            spe_output<<"@type xydy\n";
            if (verbose) egsInformation("\n   Emid/MeV    Flu/(MeV-1*cm-2)   DFlu/(MeV-1*cm-2)\n"
                                            "---------------------------------------------------\n");
            for (int i=0; i<flu_nbin; i++) {
                flu[j]->currentResult(i,fe,dfe);
                EGS_Float e = (i+0.5-flu_b)/flu_a;
                if (flu_s) {
                    e = exp(e);
                }
                spe_output << e <<" "<< fe *norma <<" "<< dfe *norma << "\n";
                if (verbose) egsInformation("%11.6f  %14.6e  %14.6e\n",
                                                e, fe*norma, dfe*norma);
            }
            spe_output << "&\n";

            if (score_primaries) {
                if (verbose) {
                    egsInformation("\nPrimary fluence:\n");
                }
                spe_output<<"@    s"<< ++i_graph <<" errorbar linestyle 0\n";
                spe_output<<"@    s"<< i_graph <<" legend \""<< "primary (ir # " << j <<")\"\n";
                spe_output<<"@target G0.S"<< i_graph <<"\n";
                spe_output<<"@type xydy\n";
                if (verbose) egsInformation("\n   Emid/MeV    Flu/(MeV-1*cm-2)   DFlu/(MeV-1*cm-2)\n"
                                                "---------------------------------------------------\n");
                for (int i=0; i<flu_nbin; i++) {
                    flu_p[j]->currentResult(i,fe,dfe);
                    EGS_Float e = (i+0.5-flu_b)/flu_a;
                    if (flu_s) {
                        e = exp(e);
                    }
                    spe_output << e <<" "<< fe *norma <<" "<< dfe *norma << "\n";
                    if (verbose) egsInformation("%11.6f  %14.6e  %14.6e\n",
                                                    e, fe*norma, dfe*norma);
                }
                spe_output << "&\n";
            }
            i_graph++;
        }

        spe_output.close();

    }

}

void EGS_VolumetricFluence::reportResults() {

    egsInformation("\nFluence Scoring (%s)\n",name.c_str());
    egsInformation("======================================================\n");

    ouputResults();

}

bool EGS_VolumetricFluence::storeState(ostream &data) const {
    if (!egsStoreI64(data,current_ncase)) {
        return false;
    }
    data << endl;

    if (!data.good()) {
        return false;
    }

#ifdef DEBUG
    if (scoring_charge) {
        if (!relStepDiff->storeState(data)) {
            return false;
        }
        if (!stepDist->storeState(data)) {
            return false;
        }
    }
#endif

    if (!fluT->storeState(data)) {
        return false;
    }
    if (score_spe) {
        for (int j = 0; j < nreg; j++) {
            if (is_sensitive[j]) {
                if (!flu[j]->storeState(data)) {
                    return false;
                }
            }
        }
    }

    if (score_primaries) {
        if (!fluT_p->storeState(data)) {
            return false;
        }
        if (score_spe) {
            for (int j=0; j < nreg; j++) {
                if (is_sensitive[j]) {
                    if (!flu_p[j]->storeState(data)) {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

bool  EGS_VolumetricFluence::setState(istream &data) {

    if (!egsGetI64(data,current_ncase)) {
        return false;
    }

    if (!data.good()) {
        return false;
    }

#ifdef DEBUG
    if (scoring_charge) {
        if (!relStepDiff->setState(data)) {
            return false;
        }
        if (!stepDist->setState(data)) {
            return false;
        }
    }
#endif

    if (!fluT->setState(data)) {
        return false;
    }
    if (score_spe) {
        for (int j=0; j<nreg; j++) {
            if (is_sensitive[j]) {
                if (!flu[j]->setState(data)) {
                    return false;
                }
            }
        }
    }

    if (score_primaries) {
        if (!fluT_p->setState(data)) {
            return false;
        }
        if (score_spe) {
            for (int j=0; j < nreg; j++) {
                if (is_sensitive[j]) {
                    if (!flu_p[j]->setState(data)) {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

bool  EGS_VolumetricFluence::addState(istream &data) {
    EGS_I64 tmp_case;
    if (!egsGetI64(data,tmp_case)) {
        return false;
    }
    current_ncase += tmp_case;

    if (!data.good()) {
        return false;
    }

#ifdef DEBUG
    if (scoring_charge) {
        EGS_ScoringArray tmpRelStepDiff(1);
        if (!tmpRelStepDiff.setState(data)) {
            return false;
        }
        (*relStepDiff) += tmpRelStepDiff;
    }
#endif
    /* fluence objects */

    EGS_ScoringArray tgT(nreg);
    if (!tgT.setState(data)) {
        return false;
    }
    (*fluT) += tgT;

    if (score_spe) {
        EGS_ScoringArray tg(flu_nbin);
        for (int j = 0; j < nreg; j++) {
            if (is_sensitive[j]) {
                if (!tg.setState(data)) {
                    return false;
                }
                (*flu[j]) += tg;
            }
        }
    }

    if (score_primaries) {
        EGS_ScoringArray tgT_p(nreg);
        if (!tgT_p.setState(data)) {
            return false;
        }
        (*fluT_p) += tgT_p;
        if (score_spe) {
            EGS_ScoringArray tg_p(flu_nbin);
            for (int j=0; j < nreg; j++) {
                if (is_sensitive[j]) {
                    if (!tg_p.setState(data)) {
                        return false;
                    }
                    (*flu_p[j]) += tg_p;
                }
            }
        }
    }

    return true;
}

/**********************************************
 * Class EGS_SphericalFluence Implementation *
***********************************************/

EGS_SphericalFluence::EGS_SphericalFluence(const string &Name, EGS_ObjectFactory *f) :
    EGS_FluenceScoring(Name, f),
    n_sph(0), N_theta(1), N_phi(1), N_ang(1),
    cos_theta_bins(true), dir_filter(sph_both),
    m_has_crossings(false),
    fluT_x_p(0), m_hist_dirty(false) {
    otype    = "EGS_SphericalFluence";
    m_center = EGS_Vector(0, 0, 0);
    m_axis   = EGS_Vector(0, 0, 1);
    m_perp1  = EGS_Vector(1, 0, 0);
    m_perp2  = EGS_Vector(0, 1, 0);
}

EGS_SphericalFluence::~EGS_SphericalFluence() {
    int n_total = n_sph * N_ang;
    if (flu) {
        for (int k = 0; k < n_total; k++) {
            delete flu[k];
        }
    }
    if (flu_p) {
        for (int k = 0; k < n_total; k++) {
            delete flu_p[k];
        }
    }
    delete fluT_x_p;
}

void EGS_SphericalFluence::initScoring(EGS_Input *inp) {

    if (!inp) {
        egsFatal("AO type %s: null input?\n", otype.c_str());
        return;
    }

    EGS_Input *sScoringInput = inp->takeInputItem("spherical scoring");
    if (!sScoringInput) {
        egsFatal("AO type %s: Missing 'spherical scoring' input block.\n", otype.c_str());
        return;
    }

    EGS_FluenceScoring::initScoring(inp);  // common inputs (particle, energy grid, etc.)

    score_in_all_regions = true;           // default: all regions contribute
    EGS_FluenceScoring::getSensitiveRegions(sScoringInput);

    /* Sphere center */
    vector<EGS_Float> tmp_center;
    int err_c = sScoringInput->getInput("scoring sphere center", tmp_center);
    if (!err_c && tmp_center.size() == 3) {
        m_center = EGS_Vector(tmp_center[0], tmp_center[1], tmp_center[2]);
    }
    else {
        egsWarning("\n\n*** EGS_SphericalFluence: Wrong/missing 'scoring sphere center',"
                   " defaulting to origin.\n\n");
        m_center = EGS_Vector(0, 0, 0);
    }

    /* Sphere radii (required) */
    vector<EGS_Float> tmp_radii;
    if (sScoringInput->getInput("scoring sphere radii", tmp_radii) || tmp_radii.empty()) {
        egsFatal("\n*** EGS_SphericalFluence: Missing 'scoring sphere radii'. Aborting!\n\n");
    }
    for (size_t i = 0; i < tmp_radii.size(); i++) {
        if (tmp_radii[i] <= 0) {
            egsFatal("\n*** EGS_SphericalFluence: radius %g <= 0. Aborting!\n\n",
                     tmp_radii[i]);
        }
    }
    sort(tmp_radii.begin(), tmp_radii.end());
    tmp_radii.erase(unique(tmp_radii.begin(), tmp_radii.end()), tmp_radii.end());
    n_sph = (int)tmp_radii.size();
    m_R   = tmp_radii;
    m_R2.resize(n_sph);
    for (int i = 0; i < n_sph; i++) {
        m_R2[i] = m_R[i] * m_R[i];
    }

    /* Direction filter */
    vector<string> dir_names;
    dir_names.push_back("both");
    dir_names.push_back("inward");
    dir_names.push_back("outward");
    int dir_sel = sScoringInput->getInput("scoring direction", dir_names, 0);
    switch (dir_sel) {
    case 1:  dir_filter = sph_inward;  break;
    case 2:  dir_filter = sph_outward; break;
    default: dir_filter = sph_both;    break;
    }

    /* Polar axis for angular binning */
    vector<EGS_Float> tmp_axis;
    int err_ax = sScoringInput->getInput("polar axis", tmp_axis);
    if (!err_ax && tmp_axis.size() == 3) {
        m_axis = EGS_Vector(tmp_axis[0], tmp_axis[1], tmp_axis[2]);
        m_axis.normalize();
    }
    else {
        m_axis = EGS_Vector(0, 0, 1);
    }

    /* Build orthonormal frame (m_perp1, m_perp2) perpendicular to m_axis */
    if (fabs(m_axis.x) < 0.9) {
        m_perp1 = EGS_Vector(1, 0, 0);
    }
    else {
        m_perp1 = EGS_Vector(0, 1, 0);
    }
    EGS_Float proj = m_perp1 * m_axis;
    m_perp1.x -= m_axis.x * proj;
    m_perp1.y -= m_axis.y * proj;
    m_perp1.z -= m_axis.z * proj;
    m_perp1.normalize();
    /* m_perp2 = m_axis × m_perp1 */
    m_perp2.x = m_axis.y*m_perp1.z - m_axis.z*m_perp1.y;
    m_perp2.y = m_axis.z*m_perp1.x - m_axis.x*m_perp1.z;
    m_perp2.z = m_axis.x*m_perp1.y - m_axis.y*m_perp1.x;

    /* Angular bin counts */
    if (sScoringInput->getInput("N theta bins", N_theta) || N_theta < 1) {
        N_theta = 1;
    }
    if (sScoringInput->getInput("N phi bins", N_phi) || N_phi < 1) {
        N_phi = 1;
    }
    N_ang = N_theta * N_phi;

    /* Theta binning mode */
    vector<string> bin_names;
    bin_names.push_back("cos_theta");
    bin_names.push_back("theta");
    cos_theta_bins = (sScoringInput->getInput("theta binning", bin_names, 0) == 0);

    /* Precompute bin areas divided by R² (multiply by R_k² for sphere k).
       Equal cos_theta: ΔA/R² = 4π / N_ang  (identical for every bin).
       Equal theta:     ΔA/R² = (2π/N_phi) × |cosθ_i − cosθ_{i+1}|  (varies with itheta). */
    m_ang_area.resize(N_ang);
    if (cos_theta_bins) {
        EGS_Float area = 4.0*M_PI / N_ang;
        for (int i = 0; i < N_ang; i++) {
            m_ang_area[i] = area;
        }
    }
    else {
        EGS_Float dtheta = M_PI / N_theta;
        for (int itheta = 0; itheta < N_theta; itheta++) {
            EGS_Float area_strip = (2.0*M_PI / N_phi) *
                                   fabs(cos(itheta*dtheta) - cos((itheta+1)*dtheta));
            for (int iphi = 0; iphi < N_phi; iphi++) {
                m_ang_area[itheta*N_phi + iphi] = area_strip;
            }
        }
    }

    m_crossings.reserve(2 * n_sph);

    /* Grazing-angle cap.  The estimator scores w/|u·n̂| = w/|cos θ| at each
       crossing.  When |cos θ| → 0 (particle nearly tangent to the sphere,
       at θ ≈ 90° or 270° from the outward normal) a single crossing can
       contribute an arbitrarily large score and dominate the batch mean,
       making the result unreliable even when the stated uncertainty appears
       small.  The threshold is therefore expressed directly as a minimum
       |cos θ| value: crossings with |u·n̂| < cos_theta_min are clamped to
       cos_theta_min in the denominator.
       Default: cos(85°) ≈ 0.0872, matching EGS_PlanarFluence.
       Set cos theta min = 0 to disable the cap (exact, unbiased estimator)
       — only safe when grazing crossings cannot occur (e.g. a point source
       at the sphere centre).  Clipped crossings are always counted and
       reported so the user can assess the bias. */
    EGS_Float cos_theta_min = cos(85.0 * M_PI / 180.0);
    sScoringInput->getInput("cos theta min", cos_theta_min);
    if (cos_theta_min <= 0.0 || cos_theta_min >= 1.0) {
        m_cos_min = 0.0;   // no cap — exact estimator
    }
    else {
        m_cos_min = cos_theta_min;
    }
}

void EGS_SphericalFluence::setApplication(EGS_Application *App) {

    EGS_AusgabObject::setApplication(App);

    if (!app) {
        return;
    }

    if (source_charge == unknown) {
        source_charge = (ParticleType)app->sourceCharge();
    }

    nreg = app->getnRegions();

    for (int j = 0; j < nreg; j++) {
        is_sensitive.push_back(score_in_all_regions ? true : false);
        is_source.push_back(false);
    }

    setUpRegionFlags();

    int n_total = n_sph * N_ang;

    fluT = new EGS_ScoringArray(n_total);
    if (score_spe) {
        flu = new EGS_ScoringArray* [n_total];
        for (int k = 0; k < n_total; k++) {
            flu[k] = new EGS_ScoringArray(flu_nbin);
        }
    }
    if (score_primaries) {
        fluT_p = new EGS_ScoringArray(n_total);
        if (score_spe) {
            flu_p = new EGS_ScoringArray* [n_total];
            for (int k = 0; k < n_total; k++) {
                flu_p[k] = new EGS_ScoringArray(flu_nbin);
            }
        }
    }

    if (m_scoring_method != score_crossing) {
        egsWarning("EGS_SphericalFluence: FD scoring not yet supported."
                   " Falling back to crossing estimator.\n");
        m_scoring_method = score_crossing;
    }

    if (score_primaries) {
        int n_total = n_sph * N_ang;
        fluT_x_p = new EGS_ScoringArray(n_total);
        m_hist_T.assign(n_total, 0.0);
        m_hist_P.assign(n_total, 0.0);
    }

    describeMe();
}

void EGS_SphericalFluence::describeMe() {
    char buf[256];
    sprintf(buf, "\nSpherical %s fluence scoring\n", particle_name.c_str());
    description  = buf;
    description += "==================================\n";

    sprintf(buf, " - sphere center      = (%g %g %g) cm\n",
            m_center.x, m_center.y, m_center.z);
    description += buf;

    description += " - sphere radii      = ";
    for (int i = 0; i < n_sph; i++) {
        sprintf(buf, "%g", m_R[i]);
        description += buf;
        if (i < n_sph - 1) {
            description += ", ";
        }
    }
    description += " cm\n";

    description += " - estimator         = boundary crossing\n";

    description += " - scoring direction = ";
    switch (dir_filter) {
    case sph_inward:  description += "inward only\n";  break;
    case sph_outward: description += "outward only\n"; break;
    default:          description += "both\n";         break;
    }

    if (N_ang > 1) {
        sprintf(buf, " - angular bins      = %d theta x %d phi (%s)\n",
                N_theta, N_phi, cos_theta_bins ? "equal area" : "equal angle");
        description += buf;
        sprintf(buf, " - polar axis        = (%g %g %g)\n",
                m_axis.x, m_axis.y, m_axis.z);
        description += buf;
    }
    else {
        description += " - angular bins      = total only\n";
    }

    if (m_cos_min > 0.0) {
        sprintf(buf, " - grazing cap       = |cos θ| < %.6f (θ near 90°/270°) → biased\n",
                m_cos_min);
        description += buf;
    }
    else {
        description += " - grazing cap       = none (exact estimator, |cos θ| unbounded)\n";
    }

    description += " - contributing from region(s): ";
    EGS_FluenceScoring::describeMe();
}

void EGS_SphericalFluence::flushHistoryCrossTerms() const {
    if (!m_hist_dirty) {
        return;
    }
    if (fluT_x_p) {
        int n_total = n_sph * N_ang;
        for (int k = 0; k < n_total; k++) {
            fluT_x_p->score(k, m_hist_T[k] * m_hist_P[k]);
            m_hist_T[k] = 0.0;
            m_hist_P[k] = 0.0;
        }
    }
    m_hist_dirty = false;
}

void EGS_SphericalFluence::findCrossings(const EGS_Particle &p) {
    m_crossings.clear();

    /* Precompute shared quantities (same for all spheres) */
    EGS_Float dx = p.x.x - m_center.x;
    EGS_Float dy = p.x.y - m_center.y;
    EGS_Float dz = p.x.z - m_center.z;
    EGS_Float b  = 2.0*(dx*p.u.x + dy*p.u.y + dz*p.u.z);
    EGS_Float d2 = dx*dx + dy*dy + dz*dz;

    for (int isph = 0; isph < n_sph; isph++) {
        EGS_Float c    = d2 - m_R2[isph];
        EGS_Float disc = b*b - 4.0*c;
        if (disc <= 0) {
            continue;
        }
        EGS_Float sq = sqrt(disc);
        EGS_Float t1 = 0.5*(-b - sq);
        EGS_Float t2 = 0.5*(-b + sq);

        for (int j = 0; j < 2; j++) {
            EGS_Float t = (j == 0) ? t1 : t2;
            /* Reject ghost crossings produced by floating-point error when the
               particle sits at the sphere surface.  After a legitimate outward
               crossing the particle's position satisfies |x|² = R²|u|²; if
               |u|² < 1 in floating point the particle appears slightly inside,
               giving t₂ ≈ R·|δ|/2 where δ = |u|²−1 ~ ε_machine.  The natural
               threshold R·√ε scales correctly with radius and with EGS_Float
               precision (float vs. double), while remaining far below any real
               geometry step. */
            if (t < m_R[isph] * std::sqrt(std::numeric_limits<EGS_Float>::epsilon())) {
                continue;
            }
            /* Outward unit normal at crossing point */
            EGS_Float invR = 1.0 / m_R[isph];
            EGS_Vector n_hat((p.x.x + p.u.x*t - m_center.x)*invR,
                             (p.x.y + p.u.y*t - m_center.y)*invR,
                             (p.x.z + p.u.z*t - m_center.z)*invR);

            EGS_Float udotn = p.u.x*n_hat.x + p.u.y*n_hat.y + p.u.z*n_hat.z;
            bool is_outward = (udotn > 0);

            if (dir_filter == sph_inward  &&  is_outward) {
                continue;
            }
            if (dir_filter == sph_outward && !is_outward) {
                continue;
            }

            CrossInfo ci;
            ci.isph      = isph;
            ci.t         = t;
            ci.n_hat     = n_hat;
            ci.is_outward = is_outward;
            ci.iang      = getAngularBin(n_hat);
            m_crossings.push_back(ci);
        }
    }
    m_has_crossings = !m_crossings.empty();
}

int EGS_SphericalFluence::getAngularBin(const EGS_Vector &n_hat) const {
    if (N_ang == 1) {
        return 0;
    }

    EGS_Float cos_theta = n_hat.x*m_axis.x + n_hat.y*m_axis.y + n_hat.z*m_axis.z;
    if (cos_theta >  1.0) { cos_theta =  1.0; }
    if (cos_theta < -1.0) { cos_theta = -1.0; }

    int itheta;
    if (cos_theta_bins) {
        itheta = (int)((1.0 - cos_theta) * 0.5 * N_theta);
        if (itheta >= N_theta) { itheta = N_theta - 1; }
    }
    else {
        EGS_Float theta = acos(cos_theta);
        itheta = (int)(theta * N_theta / M_PI);
        if (itheta >= N_theta) { itheta = N_theta - 1; }
    }

    if (N_phi == 1) {
        return itheta;
    }

    EGS_Float cp  = n_hat.x*m_perp1.x + n_hat.y*m_perp1.y + n_hat.z*m_perp1.z;
    EGS_Float sp  = n_hat.x*m_perp2.x + n_hat.y*m_perp2.y + n_hat.z*m_perp2.z;
    EGS_Float phi = atan2(sp, cp);
    if (phi < 0) { phi += 2.0*M_PI; }

    int iphi = (int)(phi * N_phi / (2.0*M_PI));
    if (iphi >= N_phi) { iphi = N_phi - 1; }

    return itheta * N_phi + iphi;
}

void EGS_SphericalFluence::scoreAtCrossing(const CrossInfo &ci, const EGS_Particle &p) {
    EGS_Float udotn = p.u.x*ci.n_hat.x + p.u.y*ci.n_hat.y + p.u.z*ci.n_hat.z;
    EGS_Float aup   = fabs(udotn);
    if (m_cos_min > 0.0 && aup < m_cos_min) {
        aup = m_cos_min;
        ++m_n_clipped;
    }
    EGS_Float auxp = p.wt / aup;
    int k = ci.isph * N_ang + ci.iang;

    m_tot += p.wt;
    if (score_primaries && !p.latch) {
        m_primary += p.wt;
    }

    fluT->score(k, auxp);
    if (score_primaries && !p.latch) {
        fluT_p->score(k, auxp);
    }
    if (fluT_x_p) {
        m_hist_T[k] += auxp;
        if (!p.latch) {
            m_hist_P[k] += auxp;
        }
        m_hist_dirty = true;
    }

    if (score_spe) {
        EGS_Float e = p.q ? p.E - app->getRM() : p.E;
        if (flu_s) {
            e = log(e);
        }
        if (e > flu_xmin && e <= flu_xmax) {
            int je = min((int)(flu_a*e + flu_b), flu_nbin - 1);
            flu[k]->score(je, auxp);
            if (score_primaries && !p.latch) {
                flu_p[k]->score(je, auxp);
            }
        }
    }
}

int EGS_SphericalFluence::processEvent(EGS_Application::AusgabCall iarg) {

    int q  = app->top_p.q;
    int ir = app->top_p.ir;

    if (iarg == EGS_Application::BeforeTransport) {
        m_has_crossings = false;
        if (q == scoring_charge && ir >= 0 && is_sensitive[ir]) {
            findCrossings(app->top_p);
            m_x0 = app->top_p.x;
        }
    }
    else if (iarg == EGS_Application::AfterTransport && m_has_crossings) {
        EGS_Vector xstep = app->top_p.x - m_x0;
        EGS_Float  step  = xstep.length();
        for (size_t i = 0; i < m_crossings.size(); i++) {
            if (m_crossings[i].t <= step + 1e-8) {
                scoreAtCrossing(m_crossings[i], app->top_p);
            }
        }
    }

    if (score_primaries && ir >= 0 && !is_source[ir]) {
        flagSecondaries(iarg, q);
    }

    return 0;
}

void EGS_SphericalFluence::ouputSphericalFluence(EGS_ScoringArray *fT,
        const double &norma, int isph) {
    double fe, dfe;
    egsInformation("\n  Sphere R = %g cm\n", m_R[isph]);
    egsInformation("  ----------------------\n");
    for (int iang = 0; iang < N_ang; iang++) {
        int k = isph*N_ang + iang;
        fT->currentResult(k, fe, dfe);
        if (dfe < 0) dfe = 0.0;
        double dfer = (fe > 0) ? 100*dfe/fe : 100;
        double norm_k = norma / (m_ang_area[iang] * m_R2[isph]);
        int itheta = iang / N_phi;
        int iphi   = iang % N_phi;
        egsInformation("   [theta=%d phi=%d]: %12.5e +/- %-7.3f%%\n",
                       itheta, iphi, fe*norm_k, dfer);
    }
}

void EGS_SphericalFluence::ouputResults() {

    EGS_Float src_norm = 1.0, Fsrc = app->getFluence();
    egsInformation("\n\n last case = %lld source particles or fluence = %g\n\n",
                   current_ncase, Fsrc);
    if (Fsrc) {
        src_norm = Fsrc / current_ncase;
    }

    string src_type = app->sourceType();
    if (src_type == "EGS_BeamSource") {
        egsInformation("\n %s normalization = %g (primary histories per particle)\n",
                       src_type.c_str(), src_norm);
    }
    else if (src_type == "EGS_CollimatedSource" ||
             (src_type == "EGS_ParallelBeam" && src_norm != 1)) {
        egsInformation("\n %s normalization = %g (fluence per particle)\n",
                       src_type.c_str(), src_norm);
    }
    else {
        egsInformation("\n %s normalization = %g (histories per particle)\n",
                       src_type.c_str(), src_norm);
    }

    double norm = norm_u / src_norm;

    flushHistoryCrossTerms();

    // Helper: extract scaled value and percentage uncertainty from a scoring bin.
    // Guards against negative variance from floating-point cancellation.
    auto getR = [](EGS_ScoringArray *arr, int k, double nk,
                   double &val, double &unc_pct) {
        double r, dr;
        arr->currentResult(k, r, dr);
        if (dr < 0) dr = 0.0;
        val = r * nk;
        unc_pct = (r > 0) ? 100.0 * dr / r : 100.0;
    };

    // Correlated ratio B = T/P uncertainty using cross-term array.
    // cov(T̄, P̄) = (E[T·P] - T̄·P̄) / N, then
    // var(B̂)     = (dT² - 2B·cov + B²·dP²) / P̄²
    auto corrRatio = [this](EGS_ScoringArray *fT, EGS_ScoringArray *fP,
                            EGS_ScoringArray *fX, int k,
                            double &B, double &Bpct) {
        double T_r, dT, P_r, dP, TP_r, dummy;
        fT->currentResult(k, T_r, dT);
        fP->currentResult(k, P_r, dP);
        if (dT < 0) dT = 0.0;
        if (dP < 0) dP = 0.0;
        B = (P_r > 0) ? T_r / P_r : 0.0;
        if (B <= 0 || current_ncase <= 0) {
            Bpct = 100.0;
            return;
        }
        double TP_r_mean = 0.0;
        if (fX) {
            fX->currentResult(k, TP_r_mean, dummy);
        }
        double cov = (TP_r_mean - T_r * P_r) / current_ncase;
        double var_B = (dT*dT - 2.0*B*cov + B*B*dP*dP) / (P_r * P_r);
        Bpct = (var_B > 0) ? 100.0 * sqrt(var_B) / B : 0.0;
    };

    // Format strings for one data cell and one ratio cell
    const char *dcol = "  %12.5e +/- %-7.3f%%";
    const char *bcol = "  %7.4f +/- %-7.3f%%";

    if (N_ang == 1) {
        // ------------------------------------------------------------------
        // Compact table: one row per sphere
        // ------------------------------------------------------------------
        egsInformation("\n\n==> Spherical %s fluence [cm^-2 per source particle]\n",
                       particle_name.c_str());

        // ---- Table: total fluence ----
        if (score_primaries) {
            egsInformation("\n  %-14s  %-30s  %-30s  %-22s\n",
                           "R [cm]", "Phi_total", "Phi_primary", "tot/pri");
            egsInformation("  %s\n", string(100, '-').c_str());
        }
        else {
            egsInformation("\n  %-14s  %-30s\n", "R [cm]", "Phi_total");
            egsInformation("  %s\n", string(46, '-').c_str());
        }

        for (int isph = 0; isph < n_sph; isph++) {
            double nk = norm / (m_ang_area[0] * m_R2[isph]);
            double Tv, Tu;
            getR(fluT, isph, nk, Tv, Tu);
            egsInformation("  %-14.6g", m_R[isph]);
            egsInformation(dcol, Tv, Tu);

            if (score_primaries) {
                double Pv, Pu;
                getR(fluT_p, isph, nk, Pv, Pu);
                double B, Bunc;
                corrRatio(fluT, fluT_p, fluT_x_p, isph, B, Bunc);
                egsInformation(dcol, Pv, Pu);
                egsInformation(bcol, B, Bunc);
            }
            egsInformation("\n");
        }

        int w = score_primaries ? 100 : 46;
        egsInformation("  %s\n", string(w, '-').c_str());
    }
    else {
        // ------------------------------------------------------------------
        // Block output for angular maps (N_ang > 1)
        // ------------------------------------------------------------------
        egsInformation("\n\n            Integral fluence [cm^-2 per source particle]\n"
                       "            ================================================\n\n");

        egsInformation("\n\n               Total %s fluence\n", particle_name.c_str());
        for (int isph = 0; isph < n_sph; isph++) {
            ouputSphericalFluence(fluT, norm, isph);
        }
        if (score_primaries) {
            egsInformation("\n\n                   Primary %s fluence\n", particle_name.c_str());
            for (int isph = 0; isph < n_sph; isph++) {
                ouputSphericalFluence(fluT_p, norm, isph);
            }
        }
    }

    if (score_spe) {
        double norm_spe = norm * flu_a;
        outputSphericalSpectrum(flu, flu_p, norm_spe, "_");
    }
}

void EGS_SphericalFluence::outputSphericalSpectrum(EGS_ScoringArray **fl_set,
                                                   EGS_ScoringArray **flp_set,
                                                   double norm_spe,
                                                   const string &infix) const {
    for (int isph = 0; isph < n_sph; isph++) {
        char suffix_buf[128];
        sprintf(suffix_buf, "_sphere%d%s%s.agr", isph, infix.c_str(), particle_name.c_str());
        string spe_name = app->constructIOFileName(suffix_buf, true);
        ofstream spe_output(spe_name.c_str(), ios::out);
        if (!spe_output) {
            egsFatal("\n EGS_SphericalFluence: Error: Failed to open file %s\n",
                     spe_name.c_str());
            exit(1);
        }

        spe_output << "# Spherical " << particle_name << " fluence at R="
                   << m_R[isph] << " cm\n";
        spe_output << "# \n";
        spe_output << "@    legend 0.2, 0.8\n";
        spe_output << "@    legend box linestyle 0\n";
        spe_output << "@    legend font 4\n";
        spe_output << "@    xaxis  label \"energy / MeV\"\n";
        spe_output << "@    xaxis  label char size 1.560000\n";
        spe_output << "@    xaxis  label font 4\n";
        spe_output << "@    xaxis  ticklabel font 4\n";
        spe_output << "@    yaxis  label \"fluence / MeV\\S-1\\Ncm\\S-2\"\n";
        spe_output << "@    yaxis  label char size 1.560000\n";
        spe_output << "@    yaxis  label font 4\n";
        spe_output << "@    yaxis  ticklabel font 4\n";
        spe_output << "@    title \"" << particle_name << " fluence at R="
                   << m_R[isph] << " cm\"\n";
        spe_output << "@    title font 4\n";
        spe_output << "@    title size 1.500000\n";
        spe_output << "@    subtitle \"angular bins\"\n";
        spe_output << "@    subtitle font 4\n";
        spe_output << "@    subtitle size 1.000000\n";

        int i_graph = 0;
        double fe, dfe;

        for (int iang = 0; iang < N_ang; iang++) {
            int k = isph*N_ang + iang;
            double norm_k = norm_spe / (m_ang_area[iang] * m_R2[isph]);
            int itheta = iang / N_phi;
            int iphi   = iang % N_phi;
            char legend[64];
            if (N_ang == 1) {
                sprintf(legend, "total");
            }
            else {
                sprintf(legend, "theta-bin %d, phi-bin %d", itheta, iphi);
            }

            spe_output << "@    s" << i_graph << " errorbar linestyle 0\n";
            spe_output << "@    s" << i_graph << " legend \"" << legend << "\"\n";
            spe_output << "@target G0.S" << i_graph << "\n";
            spe_output << "@type xydy\n";
            for (int l = 0; l < flu_nbin; l++) {
                fl_set[k]->currentResult(l, fe, dfe);
                EGS_Float e = (l + 0.5 - flu_b) / flu_a;
                if (flu_s) { e = exp(e); }
                spe_output << e << " " << fe*norm_k << " " << dfe*norm_k << "\n";
            }
            spe_output << "&\n";

            if (score_primaries) {
                char legend_p[64];
                if (N_ang == 1) {
                    sprintf(legend_p, "primary");
                }
                else {
                    sprintf(legend_p, "theta-bin %d, phi-bin %d (primary)", itheta, iphi);
                }
                spe_output << "@    s" << ++i_graph << " errorbar linestyle 0\n";
                spe_output << "@    s" << i_graph << " legend \"" << legend_p << "\"\n";
                spe_output << "@target G0.S" << i_graph << "\n";
                spe_output << "@type xydy\n";
                for (int l = 0; l < flu_nbin; l++) {
                    flp_set[k]->currentResult(l, fe, dfe);
                    EGS_Float e = (l + 0.5 - flu_b) / flu_a;
                    if (flu_s) { e = exp(e); }
                    spe_output << e << " " << fe*norm_k << " " << dfe*norm_k << "\n";
                }
                spe_output << "&\n";
            }
            i_graph++;
        }
        spe_output.close();
    }
}

void EGS_SphericalFluence::reportResults() {
    egsInformation("\nFluence Scoring (%s)\n", name.c_str());
    egsInformation("======================================================\n");
    egsInformation("   Total %ss crossing sphere(s): %g\n",
                   particle_name.c_str(), m_tot);
    if (score_primaries) {
        egsInformation("   Primary %ss:                 %g\n",
                       particle_name.c_str(), m_primary);
    }
    if (m_cos_min > 0.0) {
        egsInformation("   Crossings with |cos θ| < %.6f: %lld", m_cos_min, m_n_clipped);
        if (m_tot > 0) {
            egsInformation(" (%.4g%%)", 100.0 * m_n_clipped / m_tot);
        }
        egsInformation("\n");
        if (m_n_clipped > 0) {
            egsInformation("   *** Results are biased for clipped crossings ***\n");
        }
    }
    egsInformation("======================================================\n");
    ouputResults();
}

bool EGS_SphericalFluence::storeState(ostream &data) const {
    flushHistoryCrossTerms();
    if (!egsStoreI64(data, current_ncase)) {
        return false;
    }
    data << endl;
    data << m_tot << " " << m_primary << " " << m_n_clipped << endl;
    if (!data.good()) {
        return false;
    }
    int n_total = n_sph * N_ang;
    if (!fluT->storeState(data)) {
        return false;
    }
    if (score_spe) {
        for (int k = 0; k < n_total; k++) {
            if (!flu[k]->storeState(data)) {
                return false;
            }
        }
    }
    if (score_primaries) {
        if (!fluT_p->storeState(data)) {
            return false;
        }
        if (score_spe) {
            for (int k = 0; k < n_total; k++) {
                if (!flu_p[k]->storeState(data)) {
                    return false;
                }
            }
        }
        if (!fluT_x_p->storeState(data)) {
            return false;
        }
    }
    return true;
}

bool EGS_SphericalFluence::setState(istream &data) {
    if (!egsGetI64(data, current_ncase)) {
        return false;
    }
    data >> m_tot >> m_primary >> m_n_clipped;
    if (!data.good()) {
        return false;
    }
    int n_total = n_sph * N_ang;
    if (!fluT->setState(data)) {
        return false;
    }
    if (score_spe) {
        for (int k = 0; k < n_total; k++) {
            if (!flu[k]->setState(data)) {
                return false;
            }
        }
    }
    if (score_primaries) {
        if (!fluT_p->setState(data)) {
            return false;
        }
        if (score_spe) {
            for (int k = 0; k < n_total; k++) {
                if (!flu_p[k]->setState(data)) {
                    return false;
                }
            }
        }
        if (!fluT_x_p->setState(data)) {
            return false;
        }
    }
    fill(m_hist_T.begin(), m_hist_T.end(), 0.0);
    fill(m_hist_P.begin(), m_hist_P.end(), 0.0);
    m_hist_dirty = false;
    return true;
}

bool EGS_SphericalFluence::addState(istream &data) {
    EGS_I64 tmp_case;
    if (!egsGetI64(data, tmp_case)) {
        return false;
    }
    current_ncase += tmp_case;
    EGS_Float tmp_tot, tmp_primary;
    EGS_I64   tmp_clipped;
    data >> tmp_tot >> tmp_primary >> tmp_clipped;
    if (!data.good()) {
        return false;
    }
    m_tot       += tmp_tot;
    m_primary   += tmp_primary;
    m_n_clipped += tmp_clipped;

    int n_total = n_sph * N_ang;

    EGS_ScoringArray tgT(n_total);
    if (!tgT.setState(data)) {
        return false;
    }
    (*fluT) += tgT;

    if (score_spe) {
        EGS_ScoringArray tg(flu_nbin);
        for (int k = 0; k < n_total; k++) {
            if (!tg.setState(data)) {
                return false;
            }
            (*flu[k]) += tg;
        }
    }
    if (score_primaries) {
        EGS_ScoringArray tgT_p(n_total);
        if (!tgT_p.setState(data)) {
            return false;
        }
        (*fluT_p) += tgT_p;
        if (score_spe) {
            EGS_ScoringArray tg_p(flu_nbin);
            for (int k = 0; k < n_total; k++) {
                if (!tg_p.setState(data)) {
                    return false;
                }
                (*flu_p[k]) += tg_p;
            }
        }
        EGS_ScoringArray tgT_x_p(n_total);
        if (!tgT_x_p.setState(data)) {
            return false;
        }
        (*fluT_x_p) += tgT_x_p;
    }

    return true;
}

extern "C" {

    static void setInputs() {
        inputSet = true;

        setBaseAusgabObjectInputs();

        ausBlockInput->getSingleInput("library")->setValues({"EGS_Fluence_Scoring"});

        // Format: name, isRequired, description, vector string of allowed values
        auto typePtr = ausBlockInput->addSingleInput("type", true, "Whether to score volumetric, planar or spherical fluence", {"planar", "volumetric", "spherical"});
        ausBlockInput->addSingleInput("scoring particle", true, "Which type of particle we are scoring the fluence for", {"photon", "electron", "positron"});
        ausBlockInput->addSingleInput("source particle", false, "Optional, only required to score primary fluence. Defaults to source particles if all the same, or if mixed, defaults to scoring particle.", {"photon", "electron", "positron"});
        ausBlockInput->addSingleInput("score primaries", false, "Whether to score primary fluence as well as differential fluence. Defaults to no.", {"no", "yes"});
        auto specPtr = ausBlockInput->addSingleInput("score spectrum", false, "Whether to score a spectrum. If so, add an 'energy grid' input block. Defaults to no.", {"no", "yes"});
        ausBlockInput->addSingleInput("verbose", false, "Print extra information about the fluence scoring. Defaults to no.", {"no", "yes"});
        ausBlockInput->addSingleInput("normalization", false, "Perform a multiplicative normalization to the results. Defaults to 1.");

        auto blockPtr = ausBlockInput->addBlockInput("energy grid");
        blockPtr->addDependency(specPtr, "yes");
        blockPtr->addSingleInput("number of bins", true, "The number of energy bins in the spectrum");
        blockPtr->addSingleInput("minimum kinetic energy", true, "The minimum energy to score in the spectrum");
        blockPtr->addSingleInput("maximum kinetic energy", true, "The maximum energy to score in the spectrum");
        blockPtr->addSingleInput("scale", false, "Whether to score on a linear or logarithmic scale. Default is linear.", {"linear", "logarithmic"});

        blockPtr = ausBlockInput->addBlockInput("volumetric scoring");
        blockPtr->addDependency(typePtr, "volumetric");
        auto regionPtr = blockPtr->addSingleInput("scoring regions", false, "A list of regions to score fluence in");
        auto startPtr = blockPtr->addSingleInput("start region", false, "For a series of region ranges, list the 'starting region' for each range here (inclusive)");
        auto stopPtr = blockPtr->addSingleInput("stop region", false, "For a series of region ranges, list the 'ending region' for each range here (inclusive)");
        blockPtr->addSingleInput("volumes", false, "Either a single volume, which will be the same for all regions or a list of individual volumes for each region or region group. Default is 1 cm^3");
        blockPtr->addSingleInput("method", false, "The algorithm to use for charged particle scoring (ignored for photon scoring). See documentation for full explanation. Defaults to 'stpwr'.", {"flurz", "stpwr", "stpwrO5"});

        regionPtr->addDependency(startPtr, "", true);
        regionPtr->addDependency(stopPtr, "", true);
        startPtr->addDependency(regionPtr, "", true);
        stopPtr->addDependency(regionPtr, "", true);

        blockPtr = ausBlockInput->addBlockInput("planar scoring");
        blockPtr->addDependency(typePtr, "planar");
        regionPtr = blockPtr->addSingleInput("contributing regions", false, "A list of regions to score fluence in");
        startPtr = blockPtr->addSingleInput("start contributing region", false, "For a series of region ranges, list the 'starting region' for each range here (inclusive)");
        stopPtr = blockPtr->addSingleInput("stop contributing region", false, "For a series of region ranges, list the 'ending region' for each range here (inclusive)");
        auto circlePtr = blockPtr->addSingleInput("scoring circle", false, "The center point and radius of the circle: x y z R");
        auto circleNormPtr = blockPtr->addSingleInput("scoring plane normal", false, "The unit vector for the normal of the scoring circle: ux uy uz");
        auto rectPtr = blockPtr->addSingleInput("scoring rectangle", false, "The x and y ranges for the rectangle edges, initially defined in the x-y plane. Use a transformation input block to rotate. Format is: xmin xmax ymin ymax");

        regionPtr->addDependency(startPtr, "", true);
        regionPtr->addDependency(stopPtr, "", true);
        startPtr->addDependency(regionPtr, "", true);
        stopPtr->addDependency(regionPtr, "", true);

        circlePtr->addDependency(rectPtr, "", true);
        circleNormPtr->addDependency(rectPtr, "", true);
        rectPtr->addDependency(circlePtr, "", true);

        addTransformationBlock(blockPtr);

        blockPtr = ausBlockInput->addBlockInput("spherical scoring");
        blockPtr->addDependency(typePtr, "spherical");
        regionPtr = blockPtr->addSingleInput("contributing regions", false, "A list of regions to score fluence in");
        startPtr = blockPtr->addSingleInput("start contributing region", false, "For a series of region ranges, list the 'starting region' for each range here (inclusive)");
        stopPtr = blockPtr->addSingleInput("stop contributing region", false, "For a series of region ranges, list the 'ending region' for each range here (inclusive)");
        blockPtr->addSingleInput("scoring sphere center", false, "The center of the scoring sphere(s): x y z. Default is origin.");
        blockPtr->addSingleInput("scoring sphere radii", true, "A list of sphere radii (cm) sharing a common center. Radii are sorted and deduplicated.");
        blockPtr->addSingleInput("scoring direction", false, "Which crossings to score: both (default), inward, or outward.", {"both", "inward", "outward"});
        blockPtr->addSingleInput("N theta bins", false, "Number of polar angle bins. Default 1 (no angular resolution).");
        blockPtr->addSingleInput("N phi bins", false, "Number of azimuthal angle bins. Default 1 (no phi resolution).");
        blockPtr->addSingleInput("polar axis", false, "Polar axis for angular binning as a unit vector: ax ay az. Default 0 0 1 (z-axis).");
        blockPtr->addSingleInput("theta binning", false, "Bin uniformly in cos_theta (equal-area, default) or theta (equal-angle).", {"cos_theta", "theta"});
        blockPtr->addSingleInput("cos theta min", false, "Minimum |cos θ| = |u·n̂| allowed in the denominator of the fluence estimator w/|cos θ|. When |cos θ| → 0 (particle nearly tangent to the sphere, θ ≈ 90° or 270° from the outward normal) a single crossing can dominate the batch mean. Crossings with |u·n̂| below this threshold use the threshold value instead. Default: cos(85°) ≈ 0.0872. Set to 0 to disable the cap (exact estimator — only safe when grazing crossings cannot occur).");

        regionPtr->addDependency(startPtr, "", true);
        regionPtr->addDependency(stopPtr, "", true);
        startPtr->addDependency(regionPtr, "", true);
        stopPtr->addDependency(regionPtr, "", true);
    }

    EGS_FLUENCE_SCORING_EXPORT string getExample() {
        string example;
        example = {
            R"(
    # Example of egs_fluence_scoring for planar fluence
    #:start ausgab object:
        name    = id-string            # Arbitrary identifying string
        library = egs_fluence_scoring  # Library name
        type    = planar               # Score on circular or square field
        scoring particle = photon, or electron, or positron
        source particle  = photon, or electron, or positron
                                    # Optional. Only required to score primary fluence.
                                    # Defaults to source particles if all the same.
                                    # In the case of multiple particles,
                                    # defaults to scoring particle. Useful for
                                    # bremsstrahlung targets and radioactive sources.
        score primaries = yes or no    # Defaults to `no`.
        score spectrum  = yes or no    # Defaults to `no`.
        verbose         = yes or no    # Defaults to `no`.
        normalization   = norm         # User-requested normalization. Defaults to 1.
        #########
        # If scoring spectrum, define energy grid
        # Default: 128 linear energy bins between 1 keV and 1 MeV
        #########
        :start energy grid:
            number of bins = nbins
            minimum kinetic energy = Emin
            maximum kinetic energy = Emax
            scale = linear or logarithmic # Defaults to `linear`.
        :stop energy grid:
        ########
        # Define scoring based on type
        ########
        :start planar scoring:
            # Define contributing regions
            contributing regions = ir1 ir2 ... irn
            ### Alternatively:
            # start contributing region = iri_1, iri_2, ..., iri_n
            # stop contributing region =  irf_1, irf_2, ..., irf_n
            ###
            ################################
            # If a circular field desired:
            ################################
            scoring circle = x y z R
            scoring plane normal = ux uy uz
            ########################################################
            # If a rectangular field desired:
            #
            #scoring rectangle = xmin xmax ymin ymax
            #####
            # See documentation for EGS_AffineTransform
            #####
            #:start transformation:
            #   rotation = 2, 3 or 9 floating point numbers
            #   translation = tx, ty, tz
            #:stop transformation:
            ##########################################################
        :stop planar scoring:
    :stop ausgab object:

    # Example of egs_fluence_scoring for volumetric fluence
    #:start ausgab object:
        name    = id-string            # Arbitrary identifying string
        library = egs_fluence_scoring  # Library name
        type    = volumetric           # Score in a volume
        scoring particle = photon, or electron, or positron
        source particle  = photon, or electron, or positron
                                    # Optional. Only required to score primary fluence.
                                    # Defaults to source particles if all the same.
                                    # In the case of multiple particles,
                                    # defaults to scoring particle. Useful for
                                    # bremsstrahlung targets and radioactive sources.
        score primaries = yes or no    # Defaults to `no`.
        score spectrum  = yes or no    # Defaults to `no`.
        verbose         = yes or no    # Defaults to `no`.
        normalization   = norm         # User-requested normalization. Defaults to 1.
        # If scoring spectrum, define energy grid
        # Default: 128 linear energy bins between 1 keV and 1 MeV
        :start energy grid:
            number of bins = nbins
            minimum kinetic energy = Emin
            maximum kinetic energy = Emax
            scale = linear or logarithmic # Defaults to `linear`.
        :stop energy grid:
        :start volumetric scoring:
            scoring regions = ir1 ir2 ... irn
            ### Alternatively:
            #start region = iri_1, iri_2, ..., iri_n
            #stop region  = irf_1, irf_2, ..., irf_n
            ###
            volumes = V1, V2, ..., VN # Enter as many as scoring regions. If same number
                                    # of entries as group of regions, assumes groups of
                                    # equal volume regions. If only one entry, assumes
                                    # equal volumes in all regions. Defaults to 1.
            method  = flurz or stpwr or stpwrO5 # For charged particle scoring.
                    #
                    # flurz   => FLURZnrc algorithm
                    #
                    # Path length at each energy interval from energy
                    # deposited EDEP and total particle step TVSTEP.
                    # Assumes stopping power constancy along the particle's
                    # step. It might introduce artifacts if ESTEPE or the
                    # scoring bin width are too large.
                    #
                    # stpwr   => Accounts for stopping power variation
                    #          along the particle's step. More accurate
                    #          than method used in FLURZnrc albeit about
                    #          about 10% slower in electron beam cases.
                    #
                    # Uses an O(3) series expansion of the integral of the
                    # inverse of the stopping power with respect to energy.
                    # Stopping power is represented as a linear interpolation
                    # over a log energy grid.
                    #
                    # stpwrO5 => Uses an O(5) series expansion. Slightly slower.
                    #
                    # Defaults to `stpwr`.
        :stop volumetric scoring:
    :stop ausgab object:
)"};
        return example;
    }

    EGS_FLUENCE_SCORING_EXPORT shared_ptr<EGS_BlockInput> getInputs() {
        if (!inputSet) {
            setInputs();
        }
        return ausBlockInput;
    }

    EGS_FLUENCE_SCORING_EXPORT EGS_AusgabObject *
    createAusgabObject(EGS_Input *input, EGS_ObjectFactory *f) {
        const static char *func = "createAusgabObject(fluence_scoring)";
        if (!input) {
            egsWarning("%s: null input?\n",func);
            return 0;
        }

        string type;
        int error = input->getInput("type",type);
        if (!error && input->compare("planar",type)) {
            EGS_PlanarFluence *result = new EGS_PlanarFluence("", f);
            result->setName(input);
            result->initScoring(input);
            return result;
        }
        else if (!error && input->compare("volumetric",type)) {
            EGS_VolumetricFluence *result = new EGS_VolumetricFluence("", f);
            result->setName(input);
            result->initScoring(input);
            return result;
        }
        else if (!error && input->compare("spherical",type)) {
            EGS_SphericalFluence *result = new EGS_SphericalFluence("", f);
            result->setName(input);
            result->initScoring(input);
            return result;
        }
        else {
            egsFatal("Invalid fluence type input?\n\n\n");
            return 0;
        }
    }
}
