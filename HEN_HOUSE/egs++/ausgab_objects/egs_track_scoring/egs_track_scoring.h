/*
###############################################################################
#
#  EGSnrc egs++ particle track scoring object headers
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


/*! \file egs_track_scoring.h
 *  \brief A track scoring ausgab object
 *  \IK
 */

#ifndef EGS_TRACK_SCORING_
#define EGS_TRACK_SCORING_

#include "egs_ausgab_object.h"
#include "egs_application.h"
#include "egs_particle_track.h"

#ifdef WIN32

    #ifdef BUILD_TRACK_SCORING_DLL
        #define EGS_TRACK_SCORING_EXPORT __declspec(dllexport)
    #else
        #define EGS_TRACK_SCORING_EXPORT __declspec(dllimport)
    #endif
    #define EGS_TRACK_SCORING_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_TRACK_SCORING_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_TRACK_SCORING_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_TRACK_SCORING_EXPORT
        #define EGS_TRACK_SCORING_LOCAL
    #endif

#endif

/*! \brief A track scoring object: header

\ingroup AusgabObjects

This ausgab object can be used to store pareticle track information during
run time and to output this information into a file. The data collected
by this ausgab object can then be used to visualize the geometry and particle
tracks with \c egs_view.
This ausgab object is specified via
\verbatim
:start ausgab object:
    library = egs_track_scoring
    name    = some_name
    score photons   = yes or no # optional, yes assumed if missing
    score electrons = yes or no # optional, yes assumed if missing
    score positrons = yes or no # optional, yes assumed if missing
    start scoring   = event_number # optional, 0 assumed if missing
    stop  scoring   = event_number # optional, 1024 assumed if missing
    buffer size     = size         # optional, 1024 assumed if missing
    file name addition = some_string # optional, empty string assumed if missing
:stop ausgab object:
\endverbatim
The output file name is normally constructed from the output file name,
the string specified by <code>file name addition</code> (if present and not empty),
and <code>_wJob</code> in case of parallel runs. The extension given is <code>ptracks</code>.
Using <code>start scoring</code> and <code>stop scoring</code> one can select a
range of histories for which to score the particle track info. One can also
select specific particle type(s).
*/
class EGS_TRACK_SCORING_EXPORT EGS_TrackScoring : public EGS_AusgabObject {

public:

    EGS_TrackScoring(const string &Name="", EGS_ObjectFactory *f = 0);

    ~EGS_TrackScoring();

    int processEvent(EGS_Application::AusgabCall iarg) {
        if (m_score) {
            if (!m_didScore) {
                m_didScore = true;
                ++m_nScore;
            }
            int np = app->Np;
            if (m_pts->isScoringParticle(np)) {
                if (iarg == EGS_Application::AfterTransport) {
                    m_pts->addVertex(np,new EGS_ParticleTrack::Vertex(app->top_p.x,app->top_p.E));
                }
                else if (iarg != EGS_Application::BeforeTransport) {
                    m_pts->stopScoringParticle(np);
                }
            }
            else if (iarg == EGS_Application::BeforeTransport) {
                int q = app->top_p.q;
                if ((q ==  0 && m_score_photons) ||
                        (q == -1 && m_score_electrons) ||
                        (q ==  1 && m_score_positrons)) {
                    m_pts->startNewTrack(np);
                    m_pts->setCurrentParticleInfo(new EGS_ParticleTrack::ParticleInfo(q));
                    m_pts->addVertex(np,new EGS_ParticleTrack::Vertex(app->top_p.x,app->top_p.E));
                }
            }
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
            m_didScore = false;
        }
        if (ncase < m_start || ncase > m_stop) {
            m_score = false;
        }
        else {
            m_score = true;
        }
    };

    void setScorePhotons(bool score)   {
        m_score_photons = score;
    };
    void setScoreElectrons(bool score) {
        m_score_electrons = score;
    };
    void setScorePositrons(bool score) {
        m_score_positrons = score;
    };
    void setFirstEvent(EGS_I64 first)  {
        m_start = first;
    };
    void setLastEvent(EGS_I64 last)    {
        m_stop = last;
    };
    void setBufferSize(int size)       {
        m_bufSize = size;
    };
    void setFileNameExtra(const string &extra) {
        m_fnExtra = extra;
    };

protected:

    EGS_ParticleTrackContainer  *m_pts;               //!< The particle track container

    EGS_I64                     m_start;              //!< Minimum event index for which to score tracks
    EGS_I64                     m_stop;               //!< Maximum event index for which to score tracks
    EGS_I64                     m_lastCase;           //!< The event set via setCurrentCase()
    EGS_I64                     m_nScore;             //!< Number of events for which tracks were scored

    int                         m_bufSize;            //!< The track container size

    bool                        m_score;              //!< Should tracks be scored?
    bool                        m_didScore;           //!< Did the last event score tracks?

    bool                        m_score_photons;      //!< Score photon tracks?
    bool                        m_score_electrons;    //!< Score electron tracks?
    bool                        m_score_positrons;    //!< Score positron tracks?

    string                      m_fnExtra;            //!< String to append to output file name

};

#endif
