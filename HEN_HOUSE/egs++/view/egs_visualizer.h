/*
###############################################################################
#
#  EGSnrc egs++ geometry viewer visualizer headers
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
#  Contributors:    Frederic Tessier
#
###############################################################################
*/


#ifndef EGS_GEOMETRY_VISUALIZER_
#define EGS_GEOMETRY_VISUALIZER_

#include "egs_vector.h"
#include "egs_math.h"
#include "egs_track_view.h"

class EGS_Light;
class EGS_MaterialColor;
class EGS_BaseGeometry;
class EGS_PrivateVisualizer;


// Clipping plane class
class EGS_ClippingPlane {
    EGS_Vector a;
    EGS_Float  d;
public:
    EGS_ClippingPlane(const EGS_Vector &A, EGS_Float D) : a(A), d(D) {
        EGS_Float norm = 1/a.length();
        d *= norm;
        a *= norm;
    };
    bool isInside(const EGS_Vector &x) const {
        return (x*a >= d);
    };
    bool howfar(const EGS_Vector &x, const EGS_Vector &u, EGS_Float &t) const {
        EGS_Float xp = a*x, up = a*u;
        if ((xp >= d && up < 0) || (xp < d && up > 0)) {
            EGS_Float tt = (d-xp)/up;
            if (tt <= t) {
                t = tt;
                return true;
            }
        }
        return false;
    };
    EGS_Float hownear(const EGS_Vector &x) const {
        return fabs(x*a-d);
    };
    const EGS_Vector &getNormal() const {
        return a;
    };
};


// GeometryVisualizer class
class EGS_GeometryVisualizer {

public:

    EGS_GeometryVisualizer();
    ~EGS_GeometryVisualizer();

    void loadTracksData(const char *fname);

    void setProjection(const EGS_Vector &camera_pos,
                       const EGS_Vector &camera_look_at, EGS_Float distance,
                       EGS_Float size_x, EGS_Float size_y);
    void setProjection(const EGS_Vector &camera_pos,
                       const EGS_Vector &Xo_screen, const EGS_Vector &V1_screen,
                       const EGS_Vector &V2_screen, EGS_Float size_x, EGS_Float size_y);

    void setGlobalAmbientLight(const EGS_Vector &light);

    void addLight(const EGS_Vector &pos, const EGS_Vector &color);
    void addLight(EGS_Light *l);
    void setLight(int light, const EGS_Vector &pos, const EGS_Vector &color);
    void setLight(int light, EGS_Light *l);

    void addClippingPlane(EGS_ClippingPlane *p);
    void addClippingPlane(const EGS_Vector &A, EGS_Float D);
    void clearClippingPlanes();

    void setMaterialColor(int imed, const EGS_MaterialColor &Mat);
    void setMaterialColor(int imed, const EGS_Vector &d_color,
                          EGS_Float Alpha=1);

    //EGS_Vector *renderImage(EGS_BaseGeometry *, int xsize, int ysize);
    bool renderImage(EGS_BaseGeometry *, int nx, int ny, EGS_Vector *image, int *abort_location=NULL);
    bool renderTracks(EGS_BaseGeometry *, int nx, int ny, EGS_Vector *image, int *abort_location=NULL);
    EGS_Vector getColor(const EGS_Vector &x, EGS_BaseGeometry *g, const EGS_Float axis_distance, const EGS_Float track_alpha, const bool track_clip);
    void getRegions(const EGS_Vector &x, EGS_BaseGeometry *g, int *regions, EGS_Vector *colors, int maxreg);



    // region picking
    void regionPick(int x, int y);

#ifdef HAVE_PNG
    bool makePngImage(EGS_BaseGeometry *, int xsize, int ysize,
                      const char *fname);
#endif

    void setParticleVisibility(int particle, bool vis);

private:

    EGS_PrivateVisualizer  *p;

};

#endif
