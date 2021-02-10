/*
###############################################################################
#
#  EGSnrc egs++ beam source
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
#  Author:          Iwan Kawrakow, 2005
#
#  Contributors:    Blake Walters
#                   Frederic Tessier
#                   Reid Townson
#                   Ernesto Mainegra-Hing
#                   Hannah Gallop
#
###############################################################################
*/


/*! \file egs_beam_source.cpp
 *  \brief A BEAM simulation source
 *  \IK
 */

#include "egs_beam_source.h"
#include "egs_input.h"
#include "egs_functions.h"
#include "egs_library.h"
#include "egs_application.h"

#include <cstdlib>
#include <cstring>

#define STRINGIFY(s) HELP_S(s)
#define HELP_S(s) #s
#define F77_NAME(fname,FNAME) STRINGIFY(F77_OBJ(fname,FNAME))
#define F77_NAME_(fname,FNAME) STRINGIFY(F77_OBJ_(fname,FNAME))

static bool EGS_BEAM_SOURCE_LOCAL inputSet = false;

EGS_BeamSource::EGS_BeamSource(EGS_Input *input, EGS_ObjectFactory *f) :
    EGS_BaseSource(input,f) {
    n_reuse_photon = 0;
    n_reuse_electron = 0;
    i_reuse_photon = 0;
    i_reuse_electron = 0;
    is_valid = false;
    mu_stored = false;
    lib = 0;
    Xmin = -veryFar;
    Xmax = veryFar;
    Ymin = -veryFar;
    Ymax = veryFar;
    wmin = -veryFar;
    wmax = veryFar;
    string beam_code;
    int err1 = input->getInput("beam code",beam_code);
    string pegs_file;
    int err2 = input->getInput("pegs file",pegs_file);
    string input_file;
    int err3 = input->getInput("input file",input_file);
    if (err1) {
        egsWarning("EGS_BeamSource: no 'beam code' input\n");
    }
    if (err2) {
        egsWarning("EGS_BeamSource: no 'pegs file' input\n");
    }
    if (err3) {
        egsWarning("EGS_BeamSource: no 'input file' input\n");
    }
    if (err1 || err2 || err3) {
        return;
    }
    string egs_home;
    int err_eh = input->getInput("egs_home",egs_home);
    if (err_eh) {
        char *eh = getenv("EGS_HOME");
        if (!eh) {
            egsWarning("EGS_BeamSource: EGS_HOME is not defined\n");
            return;
        }
        else {
            egs_home = eh;
        }
    }
    else {
        egs_home = egsExpandPath(egs_home);
    }
    string hen_house;
    int err_hh = input->getInput("hen_house",hen_house);
    if (err_hh) {
        char *hh = getenv("HEN_HOUSE");
        if (!hh) {
            egsWarning("EGS_BeamSource: HEN_HOUSE is not defined\n");
            return;
        }
        else {
            hen_house = hh;
        }
    }
    else {
        hen_house = egsExpandPath(hen_house);
    }
    string path = egs_home;
    path += "bin/";
    path += CONFIG_NAME;
    lib = new EGS_Library(beam_code.c_str(),path.c_str());

    InitFunction init = (InitFunction)
                        lib->resolve(F77_NAME_(beamlib_init,BEAMLIB_INIT));
    finish = (FinishFunction)
             lib->resolve(F77_NAME_(beamlib_finish,BEAMLIB_FINISH));
    sample = (SampleFunction)
             lib->resolve(F77_NAME_(beamlib_sample,BEAMLIB_SAMPLE));
    motionsample = (MotionSampleFunction)
                   lib->resolve(F77_NAME_(beamlib_motionsample,BEAMLIB_MOTIONSAMPLE));
    MaxEnergyFunction maxenergy = (MaxEnergyFunction)
                                  lib->resolve(F77_NAME_(beamlib_max_energy,BEAMLIB_MAX_ENERGY));
    if (!init) {
        egsWarning("EGS_BeamSource: failed to resolve the init function\n");
    }
    if (!sample) {
        egsWarning("EGS_BeamSource: failed to resolve the sample function\n");
    }
    if (!motionsample) {
        egsWarning("EGS_BeamSource: failed to resolve the motionsample function\n");
    }
    if (!finish) {
        egsWarning("EGS_BeamSource: failed to resolve the finish function\n");
    }
    if (!maxenergy) {
        egsWarning("EGS_BeamSource: failed to resolve the max. energy function\n");
    }
    if (!init || !sample || !finish || !maxenergy) {
        return;
    }

    int ipar=0, ilog=6;
    int npar=0;
    EGS_Application *app = EGS_Application::activeApplication();
    if (app) {
        ipar = app->getIparallel();
        npar = app->getNparallel();
    }

    init(&ipar,&npar,&ilog,hen_house.c_str(),egs_home.c_str(),
         beam_code.c_str(),pegs_file.c_str(),input_file.c_str(),
         hen_house.size(), egs_home.size(),
         beam_code.size(),pegs_file.size(),input_file.size());
    maxenergy(&Emax);

    is_valid = true;

    vector<EGS_Float> cutout;
    int err = input->getInput("cutout",cutout);
    if (!err && cutout.size() == 4) {
        setCutout(cutout[0],cutout[1],cutout[2],cutout[3]);
    }
    vector<string> ptype;
    ptype.push_back("electrons");
    ptype.push_back("photons");
    ptype.push_back("positrons");
    ptype.push_back("all");
    ptype.push_back("charged");
    particle_type = input->getInput("particle type",ptype,3)-1;

    vector<EGS_Float> wwindow;
    err = input->getInput("weight window",wwindow);
    if (!err && wwindow.size() == 2) {
        wmin = wwindow[0];
        wmax = wwindow[1];
    }

    int ntmp;
    err = input->getInput("reuse photons",ntmp);
    if (!err && ntmp > 1) {
        n_reuse_photon = ntmp;
    }
    err = input->getInput("reuse electrons",ntmp);
    if (!err && ntmp > 1) {
        n_reuse_electron = ntmp;
    }

    //now see if Mu is returned from the BEAM source
    //use motionsample function, tmu will be -1 if not provided by BEAM
    //have to save the values read here to use as the first particle in the
    //simulation
    motionsample(&tei,&txi,&tyi,&tzi,&tui,&tvi,&twi,&twti,&tqi,&tlatchi,&counti,&tiphati,&tmui);
    if (!mu_stored) {
        if (tmui >= 0.0 && tmui <= 1.0) {
            egsInformation("EGS_BeamSource:: Mu index passed from this source.\n");
            mu_stored = true;
        }
    }

    use_iparticle = true;

    description = beam_code;
    description += "(";
    description += input_file;
    description += ") simulation source";
    otype="EGS_BeamSource";
}

EGS_I64 EGS_BeamSource::getNextParticle(EGS_RandomGenerator *, int &q,
                                        int &latch, EGS_Float &E, EGS_Float &wt, EGS_Vector &x, EGS_Vector &u) {
    if (n_reuse_photon > 0 && i_reuse_photon < n_reuse_photon) {
        q = q_save;
        latch = latch_save;
        E = E_save;
        wt = wt_save;
        x = x_save;
        u = u_save;
        mu = mu_save;
        ++i_reuse_photon;
        return count;
    }
    if (n_reuse_electron > 0 && i_reuse_electron < n_reuse_electron) {
        q = q_save;
        latch = latch_save;
        E = E_save;
        wt = wt_save;
        x = x_save;
        u = u_save;
        mu = mu_save;
        ++i_reuse_electron;
        return count;
    }
    EGS_Float te,tx,ty,tz,tu,tv,tw,twt,tmu;
    int tq,tlatch,tiphat;
    bool ok;
    do {
        if (use_iparticle) {
            //reuse data for first particle read in when setting up the source
            te=tei;
            tx=txi;
            ty=tyi;
            tz=tzi;
            tu=tui;
            tv=tvi;
            tw=twi;
            twt=twti;
            tq=tqi;
            tlatch=tlatchi;
            count=counti;
            tiphat=tiphati;
            tmu=tmui;
            use_iparticle=false;
        }
        else {
            motionsample(&te,&tx,&ty,&tz,&tu,&tv,&tw,&twt,&tq,&tlatch,&count,&tiphat,&tmu);
            //sample(&te,&tx,&ty,&tz,&tu,&tv,&tw,&twt,&tq,&tlatch,&count,&tiphat);
            //egsInformation("EGS_BeamSource::getNextParticle: Got E=%g q=%d wt=%g"
            //    " x=(%g,%g,%g) latch=%d count=%lld\n",te,tq,twt,tx,ty,tz,
            //    tlatch,count);
            if (mu_stored && (tmu < 0. || tmu > 1.0)) {
                //something's wrong
                egsWarning("EGS_BeamSource::getNextParticle: Mu index is stored in this source but mu returned < 0\n");
                egsWarning("Will no longer read mu.\n");
                mu_stored = false;
            }
        }
        if (tq) {
            te -= EGS_Application::activeApplication()->getRM();
        }
        ok = true;
        if (te > Emax + epsilon) {
            ok = false;
        } //egsInformation("Emax rejection\n"); }
        if (particle_type < 2 && tq != particle_type) {
            ok = false;
        } // egsInformation("charge rejection"); }
        if (particle_type == 3 && !tq) {
            ok = false;
        }
        if (tx < Xmin || tx > Xmax || ty < Ymin || ty > Ymax) {
            ok = false;
        } // egsInformation("cutout rejection\n"); }
        if (twt < wmin || twt > wmax) {
            ok = false; // egsInformation("weight rejection\n");
        }
    }
    while (!ok);
    i_reuse_electron = n_reuse_electron;
    i_reuse_photon = n_reuse_photon;
    //egsInformation("returning particle\n");
    E = te;
    q = tq;
    latch = 0; //latch = tlatch;
    bool save_it = false;
    if (n_reuse_photon > 1 && !tq) {
        twt /= n_reuse_photon;
        i_reuse_photon = 1;
        save_it = true;
    }
    if (n_reuse_electron > 1 && tq) {
        twt /= n_reuse_electron;
        i_reuse_electron = 1;
        save_it = true;
    }
    wt = twt;
    x = EGS_Vector(tx,ty,tz);
    u = EGS_Vector(tu,tv,tw);
    mu = tmu;
    if (save_it) {
        q_save = tq;
        latch_save = 0;
        E_save = E;
        wt_save = twt;
        x_save = x;
        u_save = u;
        mu_save = mu;
    }
    return count;
}

EGS_BeamSource::~EGS_BeamSource() {
    if (lib) {
        if (is_valid) {
            finish();
        }
        delete lib;
    }
}

extern "C" {

    static void setInputs() {
        inputSet = true;

        setBaseSourceInputs(false, false);

        srcBlockInput->getSingleInput("library")->setValues({"EGS_Beam_Source"});

        // Format: name, isRequired, description, vector string of allowed values
        srcBlockInput->addSingleInput("beam code", true, "The name of the BEAMnrc user code");
        srcBlockInput->addSingleInput("pegs file", true, "The name of the PEGS file to be used in the BEAMnrc simulation");
        srcBlockInput->addSingleInput("input file", true, "The name of the input file specifying the BEAMnrc simulation");
        srcBlockInput->addSingleInput("cutout", false, "Cutout of a rectangle defined by x1, y1, x2, y2");
        srcBlockInput->addSingleInput("particle type", false, "The type of particle.", {"all", "electrons", "photons", "positrons", "charged"});
        srcBlockInput->addSingleInput("weight window", true, "wmin wmax");
    }

    EGS_BEAM_SOURCE_EXPORT string getExample() {
        string example;
        example =
{R"(
    # Example of egs_beam_source
    #:start source:
        library = egs_beam_source
        name = my_source
        beam code = BEAM_EX10MeVe
        pegs file = 521icru
        particle type = all
    :stop source:
)"};
        return example;
    }

    EGS_BEAM_SOURCE_EXPORT shared_ptr<EGS_BlockInput> getInputs() {
        if(!inputSet) {
            setInputs();
        }
        return srcBlockInput;
    }

    EGS_BEAM_SOURCE_EXPORT EGS_BaseSource *createSource(EGS_Input *input,
            EGS_ObjectFactory *f) {
        return
            createSourceTemplate<EGS_BeamSource>(input,f,"beam source");
    }

}
