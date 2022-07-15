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
#                   Manuel Stoeckl
#                   Reid Townson
#
###############################################################################
*/


#include "egs_track_view.h"

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>

#include <iostream>
using namespace std;

static int particleType(const EGS_ParticleTrack::ParticleInfo &pinfo) {
    switch (pinfo.q) {
    case 0:
        return 0;
    case -1:
        return 1;
    case 1:
        return 2;
    default:
        return 3;
    }
}

template <typename T>
static T *shrink(T *original, size_t old_len, size_t new_len) {
    if (old_len == new_len) {
        return original;
    }
    T *n = new T[new_len];
    memcpy(n, original, sizeof(T)*new_len);
    delete[] original;
    return n;
}

const int zero = 0;

EGS_TrackView::EGS_TrackView(const char *filename, vector<size_t> &ntracks) {
    // typedefs to keep things short
    typedef EGS_ParticleTrack::Vertex Vert;
    typedef EGS_ParticleTrack::ParticleInfo PInfo;

    // zero pointers
    for (int i=0; i<3; i++) {
        m_index[i] = NULL;
        m_points[i] = NULL;
    }

    int tot_tracks = 0;
    // Slurp file
    char *tmp_buffer = 0;
    const char *func_name = "EGS_TrackView()";

    ifstream data(filename, ios::binary | ios::ate);
    if (data.fail() || !data.good()) {
        egsWarning("%s: Unable to open track space file '%s'! No tracks loaded\n",
                   func_name, filename);
        return;
    }
    streamsize size = data.tellg();
    data.seekg(0, ios::beg);

    data.read((char *)&tot_tracks, sizeof(int));
    // very conservative sanity check to avoid a huge allocation
    if (tot_tracks * sizeof(Vert) > size - sizeof(int)) {
        egsInformation("%s: No tracks loaded: %d tracks can't fit in %d-byte file '%s'\n",
                       func_name, tot_tracks, size, filename);
        m_failed = true;
        return;
    }
    egsInformation("%s: Reading %d tracks from '%s' ...\n", func_name, tot_tracks, filename);

    tmp_buffer = new char[size-sizeof(int)];

    // May want to look into memory mapping, but only if access patterns/OS sets matter
    if (!data.read((char *)tmp_buffer, size-sizeof(int))) {
        egsWarning("%s: Unable to read %d bytes into memory! No tracks loaded\n",
                   func_name, size);
        m_failed = true;
        return;
    }
    egsInformation("%s: Original size   : %d\n", func_name, size-sizeof(int));


    char **tmp_index = new char *[tot_tracks];

    int count_num[3] = {0,0,0};
    int count_vert[3] = {0,0,0};
    m_maxE = 0;
    // Or just -inf, if available.
    m_xmax = -10000000000000;
    m_ymax = -10000000000000;
    m_zmax = -10000000000000;
    m_xmin = 10000000000000;
    m_ymin = 10000000000000;
    m_zmin = 10000000000000;

    char *loc = tmp_buffer;
    for (int i=0; i<tot_tracks; i++) {
        // The general track structure is:
        // [int nverts] [ParticleInfo b] [Vertex r]
        int nverts = *((int *)loc);
        PInfo pInfo =
            *(PInfo *)(loc+sizeof(int));

        if (nverts < 2) {
            egsWarning("Track %d has length %d < 2.", i, nverts);
        }

        int type = particleType(pInfo);
        count_num[type]++;
        count_vert[type] += nverts;

        tmp_index[i] = loc;
        int skip = sizeof(int) + sizeof(PInfo);
        loc += skip;
        for (int k=0; k<nverts; k++) {
            EGS_Float energy = ((Vert *)loc)->e;
            EGS_Vector v = ((Vert *)loc)->x;
            loc += sizeof(Vert);
            m_maxE = fmax(m_maxE, energy);
            m_xmax = fmax(m_xmax, v.x);
            m_ymax = fmax(m_ymax, v.y);
            m_zmax = fmax(m_zmax, v.z);
            m_xmin = fmin(m_xmin, v.x);
            m_ymin = fmin(m_ymin, v.y);
            m_zmin = fmin(m_zmin, v.z);
        }
    }

    // Copying data into a more efficient and compact representation.
    // This doubles the initial memory use but is more efficient afterwards.

    for (int i=0; i<3; i++) {
        // In theory we could calculate above exactly how much track
        // compression saves, so overcopies aren't necessary.
        m_points[i] = new EGS_ParticleTrack::Vertex[count_vert[i]];
        m_index[i] = new int[count_num[i]+1];
    }

    int mem_rcnt[3] = {0,0,0};
    int ind_rcnt[3] = {0,0,0};

    // Compression routine!
    for (int i=0; i<tot_tracks; i++) {
        char *ind = (char *)tmp_index[i];
        int nverts = *((int *)ind);
        PInfo pInfo =
            *(PInfo *)(ind+sizeof(int));
        char *start = ind+sizeof(int)+sizeof(PInfo);

        int type = particleType(pInfo);
        int m_off = mem_rcnt[type];
        int i_off = ind_rcnt[type];
        bool join = false;
        bool append = false;
        if (m_off > 0) {
            const Vert &v = *(Vert *)start;
            const Vert &w = m_points[type][m_off-1];
            append = w.x.x == v.x.x && w.x.y == v.x.y &&
                     w.x.z == v.x.z;
            join = append && w.e == v.e;
        }
        if (join) {
            // First point/energy pair coincides with the previous last p/e pair
            memcpy(&m_points[type][m_off],start+sizeof(Vert),sizeof(Vert)*(nverts-1));
            mem_rcnt[type] += nverts - 1;
        }
        else if (append) {
            // First point matches last point, but energies are different
            // This often concatenates things unrealistically, but to the
            // rendering code it's all the same
            memcpy(&m_points[type][m_off],start,sizeof(Vert)*nverts);
            mem_rcnt[type] += nverts;
        }
        else {
            memcpy(&m_points[type][m_off],start,sizeof(Vert)*nverts);
            m_index[type][i_off] = m_off;
            ind_rcnt[type]++;
            mem_rcnt[type] += nverts;
        }
    }
    // Sentinel at the end; get lengths
    ntracks.clear();
    for (int i=0; i<3; i++) {
        m_index[i][ind_rcnt[i]] = mem_rcnt[i];
        m_tracks[i] = size_t(ind_rcnt[i]);
        ntracks.push_back(m_tracks[i]);
    }
    // Cleanup temporaries
    delete[] tmp_index;
    delete[] tmp_buffer;
    // Resize everything to fit -- a second copy :-(.
    // But it decreases runtime memory, sometimes significantly
    for (int i=0; i<3; i++) {
        m_index[i] = shrink(m_index[i], count_num[i]+1, ind_rcnt[i]+1);
        m_points[i] = shrink(m_points[i], count_vert[i], mem_rcnt[i]);
    }

    int csze = ind_rcnt[0] + ind_rcnt[1] + ind_rcnt[2];
    int msze = mem_rcnt[0] + mem_rcnt[1] + mem_rcnt[2];

    egsInformation("%s: Compressed size  : %d\n", func_name,
                   sizeof(Vert)*msze + sizeof(void *)*csze);
    egsInformation("%s: Tracks loaded    : %d (%d %d %d)\n", func_name,
                   tot_tracks, count_num[0], count_num[1], count_num[2]);
    egsInformation("%s: Tracks compressed: %d (%d %d %d)\n", func_name,
                   ind_rcnt[0]+ind_rcnt[1]+ind_rcnt[2],
                   ind_rcnt[0], ind_rcnt[1], ind_rcnt[2]);
    m_failed = false;
}

EGS_TrackView::~EGS_TrackView() {}

bool EGS_TrackView::renderTracks(int nx, int ny, EGS_Vector *image,
                                 EGS_ClippingPlane **planes, const int ext_planes,
                                 int *abort_location) {
    if (m_failed) {
        return true;
    }
    // (nx-1) and (ny-1) because the range is 0..nx-1. Lest we fall offscreen
    EGS_Matrix toWorld(sx/(nx-1)*v1_screen,// y vector
                       sy/(ny-1)*v2_screen,// x vector
                       x_screen - xo); // normal to plane
    fromWorld = toWorld.inverse();

    nplanes = ext_planes;
    for (int i=0; i<ext_planes; i++) {
        m_planes[i] = EGS_ClippingPlane(-1*planes[i]->a,-1*planes[i]->d);
    }
    // Four normal vectors to the camera cone's faces
    EGS_Vector n1 = (0.5*sy*v2_screen + x_screen - xo) % (sx * v1_screen);
    EGS_Vector n2 = (-0.5*sy*v2_screen + x_screen - xo) % (sx * v1_screen);
    EGS_Vector n3 = (0.5*sx*v1_screen + x_screen - xo) % (sy * v2_screen);
    EGS_Vector n4 = ((-0.5*sx*v1_screen + x_screen - xo) % (sy * v2_screen));
    m_planes[nplanes++] = EGS_ClippingPlane(n1,n1*xo);
    m_planes[nplanes++] = EGS_ClippingPlane(-1*n2,-1*n2*xo);
    m_planes[nplanes++] = EGS_ClippingPlane(-1*n3,-1*n3*xo);
    m_planes[nplanes++] = EGS_ClippingPlane(n4,n4*xo);
    // Prune clipping planes that cannot have an effect. (It's enough
    // to check all corners of a convex hull, like this bounding box)
    EGS_Vector bbox[8] = {
        EGS_Vector(m_xmin,m_ymin,m_zmin),
        EGS_Vector(m_xmin,m_ymin,m_zmax),
        EGS_Vector(m_xmin,m_ymax,m_zmin),
        EGS_Vector(m_xmin,m_ymax,m_zmax),
        EGS_Vector(m_xmax,m_ymin,m_zmin),
        EGS_Vector(m_xmax,m_ymin,m_zmax),
        EGS_Vector(m_xmax,m_ymax,m_zmin),
        EGS_Vector(m_xmax,m_ymax,m_zmax)
    };
    int gp = 0;
    for (int i=0; i<nplanes; i++) {
        for (int j=0; j<8; j++) {
            if (!m_planes[i].isInside(bbox[j])) {
                m_planes[gp] = m_planes[i];
                gp++;
                break;
            }
        }
    }
    nplanes = gp;

    // Ensure abort-location points to a valid zero location :-)
    if (!abort_location) {
        abort_location = (int *)&zero;
    }

    for (int k=0; k<3; k++) {
        if (m_vis_particle[k]) {
            /* set the color of this particle
                 1.0 = photon = yellow
                 2.0 = electron = red
                 3.0 = positron = blue
            */
            EGS_Float color = (k+1);

            int min, max;
            if (trackIndices.size()) {
                min = trackIndices[2*k];
                // Avoid out of bounds error, max out at m_tracks
                max = trackIndices[2*k+1] > m_tracks[k] ? m_tracks[k] : trackIndices[2*k+1];
            }
            else {
                min = 0;
                max = m_tracks[k];
            }
            for (int i=min; i<max; i++) {
                if (*abort_location) {
                    return false;
                }
                int len = m_index[k][i+1] - m_index[k][i];
                renderTrack(&m_points[k][m_index[k][i]], len, color, nx, ny, image);
            }
        }
    }
    return true;
}

typedef struct {
    int x;
    int y;
} screenpt;

static screenpt projectToScreen(const EGS_Vector &v, double nx_half, double ny_half,
                                const EGS_Matrix &fromWorld) {
    // This projection function consumes the majority of the time not spent
    // clipping points. One `could` in theory, pre-subtract xo from every point.
    EGS_Vector tmpv2 = fromWorld * v;
    screenpt p;
    p.x = tmpv2.x / tmpv2.z + nx_half;
    p.y = tmpv2.y / tmpv2.z + ny_half;
    return p;
}

void EGS_TrackView::renderTrack(EGS_ParticleTrack::Vertex *const vs, int len, EGS_Float color, int nx, int ny, EGS_Vector *image) {
    bool prev_clipped = true;
    int xxx2, yyy2;
    double dst2, e2 = 0;
    double nx_half = nx / 2.;
    double ny_half = ny / 2.;
    for (int k=0; k<len-1; k++) {
        int xxx1, yyy1;
        double dst1, e1;
        EGS_ParticleTrack::Vertex v1 = vs[k];
        EGS_ParticleTrack::Vertex v2 = vs[k+1];
        EGS_Float t2 = 0;
        if (prev_clipped) {
            // Need to calculate values for both points since the first one
            // falls outside the desired convex hull. The line may be discarded.
            prev_clipped = false;
            bool skip = false;
            EGS_Float t1 = 0;
            for (int i=0; i<nplanes; i++) {
                EGS_ClippingPlane p = m_planes[i];
                // Being inside a clipping plane is _inclusive_.
                EGS_Float d1 = v1.x * p.a;
                EGS_Float d2 = v2.x * p.a;
                bool in1 = d1 >= p.d;
                bool in2 = d2 >= p.d;
                if (in1 && in2) {
                    continue;
                }
                else if (in1) {
                    t2 = fmax(t2, (d2-p.d)/(d2 - d1));
                    prev_clipped = true;
                }
                else if (in2) {
                    t1 = fmax(t1, (p.d-d1)/(d2 - d1));
                }
                else {
                    skip = true;
                    break;
                }
            }
            // Line completely clipped
            if (t1 + t2 > 1 || skip) {
                prev_clipped = true;
                continue;
            }
            EGS_Vector f1 = (v1.x + t1 * (v2.x - v1.x)) - xo;
            screenpt r1 = projectToScreen(f1,nx_half,ny_half,fromWorld);
            xxx1 = r1.x;
            yyy1 = r1.y;
            dst1 = f1.length();
            if (energyScaling) {
                e1 = v1.e / m_maxE;
            }
        }
        else {
            // The first point is in the legal zone; only need the values
            // for the second point (whether or not it is clipped.). The line
            // will not be totally clipped out.
            prev_clipped = false;
            for (int i=0; i<nplanes; i++) {
                EGS_ClippingPlane p = m_planes[i];
                // Being inside a clipping plane is _inclusive_.
                EGS_Float d2 = v2.x * p.a;
                bool in2 = d2 >= p.d;
                if (!in2) {
                    EGS_Float d1 = v1.x * p.a;
                    t2 = fmax(t2, (d2-p.d)/(d2 - d1));
                    prev_clipped = true;
                }
            }
            // First point uses previous values
            xxx1 = xxx2;
            yyy1 = yyy2;
            dst1 = dst2;
            if (energyScaling) {
                e1 = e2;
            }
        }
        EGS_Vector f2 = (v2.x - t2 * (v2.x - v1.x)) - xo;
        screenpt r2 = projectToScreen(f2,nx_half,ny_half,fromWorld);
        xxx2 = r2.x;
        yyy2 = r2.y;
        dst2 = f2.length();
        if (energyScaling) {
            e2 = v2.e / m_maxE;
        }

        if (xxx1 == xxx2 && yyy1 == yyy2) {
            continue;
        }
#ifdef VIEW_DEBUG
        if (xxx1 < 0 || xxx2 < 0 || yyy1 < 0 || yyy2 < 0 ||
                xxx1 >= nx || xxx2 >= nx || yyy1 >= ny || yyy2 >= ny) {
            egsWarning("Ooops %d %d %d %d out of bounds 0..%d by 0..%d inclusive\n",xxx1,xxx2,yyy1,yyy2, nx-1, ny-1);
            continue;
        }
#endif

        //// Filter out low-energy traces
        // if (e1 < 0.1) continue;

        //// set transparency based on distance to z axis
        // double myd = (v2->x.x*v2->x.x + v2->x.y*v2->x.y)/9;
        // e2 = exp(-sqrt(myd));

        //// Filter out traces outside a certain cone
        // double myw = (v2->x.x*v2->x.x + v2->x.y*v2->x.y) / (1.0*v2->x.z*v2->x.z);
        // if (myw > 1) break;

        // Useful helper variables
        double ddx = (xxx2 - xxx1);
        double ddy = (yyy2 - yyy1);
        double dd = (dst2 - dst1);
        double gd = dst1;
        double de, ge;

        if (energyScaling) {
            de = (e2 - e1);
            ge = e1;
        }

        // iterate along the longer side
        if (abs(xxx2 - xxx1) > abs(yyy2 - yyy1)) {
            double cy = yyy1;
            ddy = ddy / abs(ddx);
            int di = (xxx1 > xxx2) ? -1 : 1;
            dd = dd / abs(ddx);
            if (energyScaling) {
                de = de / abs(ddx);
            }

            // for each x calculate the corresponding y
            for (int cx = xxx1; cx != xxx2; cx += di) {
                // grab what's already in the image buffer at this location
                EGS_Vector tmpv3 = image[cx + ((int)(cy))*nx];
                if (tmpv3.z < 0) { // there is already a track there
                    if (-gd > tmpv3.z) {
                        tmpv3.z = -gd; // current track is closer
                    }
                    if (color > tmpv3.x) {
                        tmpv3.x = color; // put "important" particles in front [warning: may teleport particles through a sheet of lesser particles]
                    }
                    if (energyScaling) {
                        tmpv3.y += (1-tmpv3.y)*e2; // compound transparency values
                    }
                }
                else {
                    tmpv3.x = color;
                    if (energyScaling) {
                        tmpv3.y=e2;
                    }
                    tmpv3.z=-gd; // just update with current track info
                }
                // write new values to image buffer
                image[cx + ((int)(cy))*nx] = tmpv3;
                cy += ddy;
                gd += dd;
                if (energyScaling) {
                    ge += de;
                }
            }
        }
        else {
            double cx;
            int di = (yyy1 > yyy2) ? -1 : 1;
            cx = xxx1;
            ddx = ddx / abs(ddy);
            dd = dd / abs(ddy);
            if (energyScaling) {
                de = de / abs(ddy);
            }

            // for each y calculate the corresponding x
            for (int cy = yyy1; cy != yyy2; cy += di) {
                // grab what's already in the image buffer at this location
                EGS_Vector tmpv3 = image[(int)(cx) + cy*nx];
                if (tmpv3.z < 0) { // there is already a track there
                    if (-gd > tmpv3.z) {
                        tmpv3.z = -gd; // current track is closer
                    }
                    if (color > tmpv3.x) {
                        tmpv3.x = color; // put "important" particles in front
                    }
                    if (energyScaling) {
                        tmpv3.y += (1-tmpv3.y)*e2; // compound transparency values
                    }
                }
                else {
                    tmpv3.x = color;
                    if (energyScaling) {
                        tmpv3.y=e2;
                    }
                    tmpv3.z=-gd; // just update with current track info
                }
                // write new values to image buffer
                image[(int)(cx) + cy*nx] = tmpv3;

                cx += ddx;
                gd += dd;
                if (energyScaling) {
                    ge += de;
                }
            }
        }
    }
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
}
