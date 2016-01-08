/*
###############################################################################
#
#  EGSnrc egs++ geometry viewer particle tracks headers
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
#  Author:          Georgi Gerganov, 2009
#
#  Contributors:    Iwan Kawrakow
#                   Frederic Tessier
#
###############################################################################
*/


#ifndef EGS_TRACK_VIEW_
#define EGS_TRACK_VIEW_

#include <iostream>
using namespace std;

#include "egs_vector.h"
#include "egs_particle_track.h"
#include "egs_rndm.h"

class EGS_TrackView : public EGS_ParticleTrackContainer {

public:

    EGS_TrackView(const char *filename);

    ~EGS_TrackView();

    bool renderTracks(int nx, int ny, EGS_Vector *image, int *abort_location=NULL);

    void setProjection(EGS_Vector pxo, EGS_Vector px_screen, EGS_Vector pv1_screen,
                       EGS_Vector pv2_screen, EGS_Float psx, EGS_Float psy);

    void setParticleVisibility(int p, bool vis) {
        if (p < 1 || p > 4) {
            return;
        }
        m_vis_particle[p] = vis;
    }

    EGS_Float getMaxE() {
        return m_maxE;
    }

protected:

    EGS_Vector  xo;         // camera position
    EGS_Vector  x_screen;   // center of projected image
    EGS_Vector  v1_screen,  // 2 perpendicular vectors on the screen
                v2_screen;
    EGS_Float   sx, sy;     // the screen size
    EGS_Float   m_maxE;     // the energy of the particle with max energy

    EGS_Float   m_scr_a;    // constants defining the plane the projection
    EGS_Float   m_scr_b;    // screen:
    EGS_Float   m_scr_c;    // m_scr_a*x + m_scr_b*y + m_scr_c*z = 1

    bool        m_vis_particle[4];
};

#endif
