/*
###############################################################################
#
#  EGSnrc egs++ particle track scoring object
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
#  Author:          Iwan Kawrakow, 2009
#
#  Contributors:    Georgi Gerganov
#
###############################################################################
*/


/*! \file egs_track_scoring.cpp
 *  \brief A track scoring ausgab object: implementation
 *  \IK
 */

#include "egs_track_scoring.h"
#include "egs_input.h"
#include "egs_functions.h"

EGS_TrackScoring::EGS_TrackScoring(const string &Name, EGS_ObjectFactory *f) :
    EGS_AusgabObject(Name,f), m_pts(0), m_start(0), m_stop(1024), m_lastCase(-1),
    m_nScore(0), m_bufSize(16), m_score(false), m_didScore(false),
    m_score_photons(true), m_score_electrons(true), m_score_positrons(true), m_fnExtra("") {
    otype = "EGS_TrackScoring";
}

EGS_TrackScoring::~EGS_TrackScoring() {
    if (m_pts) {
        delete m_pts;
    }
}

void EGS_TrackScoring::setApplication(EGS_Application *App) {
    EGS_AusgabObject::setApplication(App);
    if (!app) {
        return;
    }
    if (m_pts) {
        delete m_pts;
        m_pts = 0;
    }
    if (m_bufSize < 1) {
        m_bufSize = 1024;
    }
    string fname(app->getOutputFile());
    fname += m_fnExtra;
    if (!egsIsAbsolutePath(fname)) {
        fname = egsJoinPath(app->getAppDir(),fname);
    }
    int i_parallel = -1;
    if (app->getNparallel() > 1) {
        i_parallel = app->getIparallel();
    }
    if (i_parallel >= 0) {
        char buf[16];
        sprintf(buf,"_w%d",i_parallel);
        fname += buf;
    }
    fname += ".ptracks";
    m_pts = new EGS_ParticleTrackContainer(fname.c_str(),m_bufSize);

    description = "\nParticle Track Scoring (";
    description += name;
    description += ")\n";
    description += "======================================================\n";
    description += " - Scoring photon tracks       = ";
    description += m_score_photons ? "YES\n" : "NO\n";
    description += " - Scoring electron tracks     = ";
    description += m_score_electrons ? "YES\n" : "NO\n";
    description += " - Scoring positron tracks     = ";
    description += m_score_positrons ? "YES\n" : "NO\n";
    description += " - First event to score        = ";
    char buf[32];
    sprintf(buf,"%lld\n",m_start);
    description += buf;
    description += " - Last event to score         = ";
    sprintf(buf,"%lld\n",m_stop);
    description += buf;
    description += " - Track buffer size           = ";
    sprintf(buf,"%d\n",m_bufSize);
    description += buf;
    description += " - Output file name            = ";
    description += fname;
    description += "\n\n";
}

void EGS_TrackScoring::reportResults() {
    egsInformation("\nParticle Track Scoring (%s)\n",name.c_str());
    egsInformation("======================================================\n");
    egsInformation("   Total events scored:     %lld\n",m_nScore);
    if (m_pts) {
        m_pts->reportResults(false);
    }
}


extern "C" {

    EGS_TRACK_SCORING_EXPORT EGS_AusgabObject *createAusgabObject(EGS_Input *input,
            EGS_ObjectFactory *f) {
        const static char *func = "createAusgabObject(track_scoring)";
        if (!input) {
            egsWarning("%s: null input?\n",func);
            return 0;
        }
        vector<string> sc_options;
        sc_options.push_back("no");
        sc_options.push_back("yes");
        bool scph = input->getInput("score photons",sc_options,true);
        bool scel = input->getInput("score electrons",sc_options,true);
        bool scpo = input->getInput("score positrons",sc_options,true);
        if (!scph && !scel && !scpo) {
            return 0;
        }
        EGS_I64 first = 0, last = 1024;
        input->getInput("start scoring",first);
        input->getInput("stop scoring",last);
        int bufSize = 1024;
        input->getInput("buffer size",bufSize);
        string fnExtra;
        input->getInput("file name addition",fnExtra);
        EGS_TrackScoring *result = new EGS_TrackScoring("",f);
        result->setScorePhotons(scph);
        result->setScoreElectrons(scel);
        result->setScorePositrons(scpo);
        result->setFirstEvent(first);
        result->setLastEvent(last);
        result->setBufferSize(bufSize);
        result->setFileNameExtra(fnExtra);
        result->setName(input);
        return result;
    }

}
