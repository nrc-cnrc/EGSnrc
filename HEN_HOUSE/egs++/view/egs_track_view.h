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
#                   Manuel Stoeckl
#                   Reid Townson
#
###############################################################################
*/


#ifndef EGS_TRACK_VIEW_
#define EGS_TRACK_VIEW_

#include "egs_visualizer.h"
#include "egs_vector.h"
#include "egs_transformations.h"
#include "stddef.h"
#include "egs_particle_track.h"

class EGS_Matrix : private EGS_RotationMatrix {
public:
    // Suitable for mapping e1,e2,e3 onto colA, colB, colC
    EGS_Matrix(const EGS_Vector &colA,
               const EGS_Vector &colB,
               const EGS_Vector &colC) :
        EGS_RotationMatrix(colA.x,colB.x,colC.x,
                           colA.y,colB.y,colC.y,
                           colA.z,colB.z,colC.z) {}
    EGS_Matrix(const EGS_RotationMatrix &r) : EGS_RotationMatrix(r) {}
    EGS_Matrix() : EGS_RotationMatrix() {}

    EGS_Float det() const {
        return EGS_RotationMatrix::det();
    }

    EGS_Matrix inverse() const {
        EGS_Float d = det();
        if (d == 0) {
            egsWarning("Tried to invert matrix with zero determinant.");
            EGS_Vector empty(0,0,0);
            return EGS_Matrix(empty,empty,empty);
        }
        // Constructor transposes the visual structure.
        EGS_Matrix m =
            EGS_Matrix(EGS_Vector(ryy*rzz-ryz*rzy, ryz*rzx-ryx*rzz, ryx*rzy-ryy*rzx),
                       EGS_Vector(rxz*rzy-rxy*rzz, rxx*rzz-rxz*rzx, rxy*rzx-rxx*rzy),
                       EGS_Vector(rxy*ryz-rxz*ryy, rxz*ryx-rxx*ryz, rxx*ryy-rxy*ryx));
        return m.uniformScale(1 / d);
    }
    EGS_Matrix uniformScale(EGS_Float factor) const {
        return *this * EGS_Matrix(EGS_Vector(factor,0,0),
                                  EGS_Vector(0,factor,0),
                                  EGS_Vector(0,0,factor));
    }
    EGS_Matrix operator*(const EGS_RotationMatrix &m) const {
        return EGS_RotationMatrix::operator *(m);
    }
    EGS_Vector operator*(const EGS_Vector &v) const {
        return EGS_RotationMatrix::operator *(v);
    }
    void info() const {
        egsInformation(" ---------- \n");
        egsInformation("| %f %f %f |\n",rxx,rxy,rxz);
        egsInformation("| %f %f %f |\n",ryx,ryy,ryz);
        egsInformation("| %f %f %f |\n",rzx,rzy,rzz);
        egsInformation(" ---------- \n");
    }
};

class EGS_TrackView {

public:

    EGS_TrackView(const char *filename, vector<size_t> &ntracks);

    ~EGS_TrackView();

    bool renderTracks(int nx, int ny, EGS_Vector *image,
                      EGS_ClippingPlane **planes, const int n_planes,
                      int *abort_location=NULL);

    void setProjection(EGS_Vector pxo, EGS_Vector px_screen, EGS_Vector pv1_screen,
                       EGS_Vector pv2_screen, EGS_Float psx, EGS_Float psy);

    void setParticleVisibility(int p, bool vis) {
        if (p < 1 || p > 3) {
            return;
        }
        m_vis_particle[p-1] = vis;
    }

    void setEnergyScaling(bool scaling) {
        energyScaling = scaling;
    }

    EGS_Float getMaxE() {
        return m_maxE;
    }

    void setTrackIndices(const vector<size_t> &trackInd) {
        trackIndices = trackInd;
    }

protected:
    void renderTrack(EGS_ParticleTrack::Vertex *const vs, int len, EGS_Float color, int nx, int ny, EGS_Vector *image);

    // High-level camera description
    EGS_Vector  x_screen;   // center of projected image
    EGS_Vector  v1_screen,  // 2 perpendicular vectors on the screen
                v2_screen;
    EGS_Float   sx, sy;     // the screen size

    // Used to calculate track positions
    EGS_Matrix  fromWorld;  // 3x3 matrix mapping points to a camera basis
    EGS_Vector  xo;         // camera position

    EGS_Float   m_maxE;     // the maximum energy of all the particles

    bool        m_vis_particle[3];  // Extra make indices 1-3 incl.

    EGS_ParticleTrack::Vertex  *m_points[3]; // Data from file
    int        *m_index[3];       // Pointers to the starts of each track set
    size_t      m_tracks[3];      // Number of tracks in each index

    vector<size_t> trackIndices;

    EGS_ClippingPlane m_planes[14]; // Clipping planes. 0-3 are for the viewport
    int         nplanes;          // number of planes used

    EGS_Float   m_xmin,m_ymin,m_zmin, // Bounding box for particles
                m_xmax,m_ymax,m_zmax;

    bool        m_failed,
                energyScaling;   // Load error
};

#endif
