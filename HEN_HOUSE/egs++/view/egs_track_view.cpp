/*
###############################################################################
#
#  EGSnrc egs++ geometry viewer particle tracks
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
#                   Ernesto Mainegra-Hing
#
###############################################################################
*/


#include "egs_track_view.h"
#include <stdlib.h>
#include <math.h>

double det(EGS_Vector v1, EGS_Vector v2, EGS_Vector v3) {
    return (v1.x*v2.y*v3.z + v1.y*v2.z*v3.x + v1.z*v2.x*v3.y -
            v1.z*v2.y*v3.x - v1.y*v2.x*v3.z - v1.x*v2.z*v3.y);
}

EGS_TrackView::EGS_TrackView(const char *filename) {
    m_trspFile = NULL;
    int err = readDataFile(filename);
    if (err) {
        return;
    }
    m_maxE = 0;
    for (int i = 0; i < m_nTracks; i++) {
        for (int j = 0; j < m_buffer[i]->getNumVertices(); j++) {
            if (m_buffer[i]->getVertex(j)->e > m_maxE) {
                m_maxE = m_buffer[i]->getVertex(j)->e;
            }
        }
    }
}

EGS_TrackView::~EGS_TrackView() {
    // nothing to do -> pass execution to parent Destructor
}

bool EGS_TrackView::renderTracks(int nx, int ny, EGS_Vector *image, int *abort_location) {
    EGS_Vector tmpv(0,0,0), tmpv2(0,0,0), tmpv3(0,0,0);
    int di, xxx1, xxx2, yyy1, yyy2;
    double e1, e2, dst1, dst2;
    double xx1, xx2, yy1, yy2, s1, s2, dx, dy, dd, de, cx, cy, gd, ge;

    for (int i = 0; i < m_nTracks; i++) {

        if (abort_location && *abort_location) {
            return false;
        }

        // no reason to render the track if it has less than 2 vertices
        if (m_buffer[i]->getNumVertices() < 2) {
            continue;
        }
        EGS_ParticleTrack::ParticleInfo *pInfo = m_buffer[i]->getParticleInfo();

        // check if the user wants to see this type of particles
        if (pInfo->q == 0 && !m_vis_particle[1]) {
            continue;
        }
        else if (pInfo->q == -1 && !m_vis_particle[2]) {
            continue;
        }
        else if (pInfo->q == 1 && !m_vis_particle[3]) {
            continue;
        }
        else if ((pInfo->q > 1 || pInfo->q < -1) && !m_vis_particle[4]) {
            continue;
        }

        // get initial vertex of the track - begining
        EGS_ParticleTrack::Vertex *v1 = m_buffer[i]->getVertex(0);

        // calculate distance from camera to projection plane along (v1-xo)
        s1 = (1 - m_scr_a*xo.x - m_scr_b*xo.y - m_scr_c*xo.z) /
             (m_scr_a*(v1->x.x-xo.x)+m_scr_b*(v1->x.y-xo.y)+m_scr_c*(v1->x.z-xo.z));

        // calculate coordinates of the projection point
        tmpv = (xo + ((v1->x-xo)*s1) - x_screen);
        xx1 = tmpv * v1_screen;
        yy1 = tmpv * v2_screen;

        // convert to pixel coordinates
        xxx1 = (sx / 2 + xx1) / (sx / nx);
        yyy1 = (sy / 2 + yy1) / (sy / ny);

        // get energy of the particle at this vertex
        e1 = v1->e;

        // get distance between camera and vertex
        dst1 = (v1->x - xo).length();

        /* set the color of this particle
             1.0 = photon = yellow
             2.0 = electron = red
             3.0 = positron = blue
             4.0 = unknown = white
        */
        if (pInfo->q == 0) {
            tmpv2.x = 1.0;
        }
        else if (pInfo->q == -1) {
            tmpv2.x = 2.0;
        }
        else if (pInfo->q == 1)  {
            tmpv2.x = 3.0;
        }
        else {
            tmpv2.x = 4.0;
        }

        for (int j = 1; j < m_buffer[i]->getNumVertices(); j++) {

            // get the next vertex along the current track
            EGS_ParticleTrack::Vertex *v2 = m_buffer[i]->getVertex(j);
            s2 = (1 - m_scr_a*xo.x - m_scr_b*xo.y - m_scr_c*xo.z) /
                 (m_scr_a*(v2->x.x-xo.x)+m_scr_b*(v2->x.y-xo.y)+m_scr_c*(v2->x.z-xo.z));

//            double myw = (v2->x.x*v2->x.x + v2->x.y*v2->x.y) / (1.0*v2->x.z*v2->x.z);
//           if (myw > 1) break;

            // calculate coordinates of the projection point for the new vertex
            tmpv = (xo + ((v2->x-xo)*s2) - x_screen);
            xx2 = tmpv * v1_screen;
            yy2 = tmpv * v2_screen;

            // convert to pixel coordinates
            xxx2 = (sx / 2 + xx2) / (sx / nx);
            yyy2 = (sy / 2 + yy2) / (sy / ny);

            // get energy of the particle at this vertex
            e2 = v2->e/m_maxE;

//             // set transparency based on distance to z axis
//             double myd = (v2->x.x*v2->x.x + v2->x.y*v2->x.y)/9;
//             e2 = exp(-sqrt(myd));

            // get distance between camera and vertex
            dst2 = (v2->x - xo).length();

            // calculate some help variables
            dx = (xxx2 - xxx1);
            dy = (yyy2 - yyy1);
            dd = (dst2 - dst1);
            de = (e2 - e1);
            gd = dst1;
            ge = e1;
            if (dx == 0.0000 && dy == 0.0000) {
                continue;
            }

            // iterate along the longer side
            if (abs(dx) > abs(dy)) {
                di = (xxx1 > xxx2) ? -1 : 1;
                cy = yyy1;
                dy = dy / abs(dx);
                dd = dd / abs(dx);
                de = de / abs(dx);

                // for each x calculate the corresponding y
                for (int cx = xxx1; cx != xxx2; cx += di) {
                    // render only if we are inside the screen
                    if (cx >= 0 && cx < nx && cy >= 0 && cy < ny) {

                        // grab what's already in the image buffer at this location
                        tmpv3 = image[cx + ((int)(cy))*nx];
                        if (tmpv3.z < 0) {                                      // there is already a track there
                            if (-gd > tmpv3.z) {
                                tmpv3.z = -gd;    // current track is closer
                            }
                            if (tmpv2.x > tmpv3.x) {
                                tmpv3.x = tmpv2.x;    // put "important" particles in front
                            }
                            tmpv3.y += (1-tmpv3.y)*e2;                          // compound transparency values
                        }
                        else {
                            tmpv3.x = tmpv2.x;
                            tmpv3.y=e2;
                            tmpv3.z=-gd;         // just update with current track info
                        }
                        // write new values to image buffer
                        image[cx + ((int)(cy))*nx] = tmpv3;
                    }
                    cy += dy;
                    gd += dd;
                    ge += de;
                }
            }
            else {
                di = (yyy1 > yyy2) ? -1 : 1;
                cx = xxx1;
                dx = dx / abs(dy);
                dd = dd / abs(dy);
                de = de / abs(dy);

                // for each y calculate the corresponding x
                for (int cy = yyy1; cy != yyy2; cy += di) {
                    // render only if we are inside the screen
                    if (cx >= 0 && cx < nx && cy >= 0 && cy < ny) {

                        // grab what's already in the image buffer at this location
                        tmpv3 = image[(int)(cx) + cy*nx];
                        if (tmpv3.z < 0) {                                      // there is already a track there
                            if (-gd > tmpv3.z) {
                                tmpv3.z = -gd;    // current track is closer
                            }
                            if (tmpv2.x > tmpv3.x) {
                                tmpv3.x = tmpv2.x;    // put "important" particles in front
                            }
                            tmpv3.y += (1-tmpv3.y)*e2;                          // compound transparency values
                        }
                        else {
                            tmpv3.x = tmpv2.x;
                            tmpv3.y=e2;
                            tmpv3.z=-gd;         // just update with current track info
                        }
                        // write new values to image buffer
                        image[(int)(cx) + cy*nx] = tmpv3;
                    }
                    cx += dx;
                    gd += dd;
                    ge += de;
                }
            }

            // save already calculated data for the next segment
            v1 = v2;
            s1 = s2;
            xx1 = xx2;
            yy1 = yy2;
            xxx1 = xxx2;
            yyy1 = yyy2;
            e1 = e2;
            dst1 = dst2;
        }
    }
    return true;
}

void EGS_TrackView::setProjection(EGS_Vector pxo, EGS_Vector px_screen, EGS_Vector pv1_screen,
                                  EGS_Vector pv2_screen, EGS_Float psx, EGS_Float psy) {

    // set camera and screen variables
    xo = pxo;
    sx = psx;
    sy = psy;
    x_screen = px_screen;
    v1_screen = pv1_screen;
    v2_screen = pv2_screen;
    v1_screen.normalize();
    v2_screen.normalize();

    // calculate the equation of the projection plane defined by the screen:
    //      a*x + b*y + c*z = 1
    EGS_Vector ps1(x_screen);
    EGS_Vector ps2(x_screen + v1_screen);
    EGS_Vector ps3(x_screen + v2_screen);
    double d = det(ps1, ps2, ps3);
    EGS_Vector a1 = ps1;
    a1.x = 1;
    EGS_Vector a2 = ps2;
    a2.x = 1;
    EGS_Vector a3 = ps3;
    a3.x = 1;
    double d1 = det(a1, a2, a3);
    EGS_Vector b1 = ps1;
    b1.y = 1;
    EGS_Vector b2 = ps2;
    b2.y = 1;
    EGS_Vector b3 = ps3;
    b3.y = 1;
    double d2 = det(b1, b2, b3);
    EGS_Vector c1 = ps1;
    c1.z = 1;
    EGS_Vector c2 = ps2;
    c2.z = 1;
    EGS_Vector c3 = ps3;
    c3.z = 1;
    double d3 = det(c1, c2, c3);

    // avoid seg faults
    if (d == 0.0000) {
        d += 0.001;
    }
    m_scr_a = d1/d;
    m_scr_b = d2/d;
    m_scr_c = d3/d;
};
