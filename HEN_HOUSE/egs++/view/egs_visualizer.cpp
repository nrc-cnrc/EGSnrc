/*
###############################################################################
#
#  EGSnrc egs++ geometry viewer visualizer
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


#include "egs_visualizer.h"
#include "egs_light.h"
#include "egs_base_geometry.h"
#include "egs_functions.h"
#include <stdlib.h>

class EGS_PrivateVisualizer {
public:
    EGS_PrivateVisualizer() : global_ambient_light(0,0,0),
        nlight(0), ntot(0), nmat(0), nclip(0), nclip_t(0),
        m_tracks(NULL) {};
    ~EGS_PrivateVisualizer();
    void loadTracksData(const char *fname) {
        if (m_tracks) {
            delete m_tracks;
        }
        m_tracks = new EGS_TrackView(fname);
    }
    void setProjection(const EGS_Vector &camera_pos,
                       const EGS_Vector &camera_look_at, EGS_Float distance,
                       EGS_Float size_x, EGS_Float size_y);
    void setProjection(const EGS_Vector &camera_pos,
                       const EGS_Vector &Xo_screen, const EGS_Vector &V1_screen,
                       const EGS_Vector &V2_screen,EGS_Float size_x, EGS_Float size_y);
    void setGlobalAmbientLight(const EGS_Vector &light) {
        global_ambient_light = light;
    };
    void setLight(int ilight, const EGS_Vector &pos, const EGS_Vector &color) {
        setLight(ilight,new EGS_Light(pos,color));
    };
    void setLight(int ilight, EGS_Light *light) {
        if (ilight < nlight) {
            delete lights[ilight];
            lights[ilight] = light;
        }
        else {
            addLight(light);
        }
    };
    void addLight(EGS_Light *light) {
        if (nlight == ntot) {
            int nnew = ntot + 10;
            EGS_Light **l = new EGS_Light * [nnew];
            for (int j=0; j<nlight; j++) {
                l[j] = lights[j];
            }
            if (ntot > 0) {
                delete [] lights;
            }
            ntot = nnew;
            lights = l;
        }
        lights[nlight++] = light;
    };
    void addLight(const EGS_Vector &pos, const EGS_Vector &color) {
        addLight(new EGS_Light(pos,color));
    };
    void addClippingPlane(EGS_ClippingPlane *p) {
        if (nclip >= nclip_t) {
            EGS_ClippingPlane **tmp = new EGS_ClippingPlane * [nclip_t + 5];
            for (int j=0; j<nclip; j++) {
                tmp[j] = clip[j];
            }
            if (nclip_t > 0) {
                delete [] clip;
            }
            clip = tmp;
            nclip_t += 5;
        }
        clip[nclip++] = p;
    };
    void addClippingPlane(const EGS_Vector &A, EGS_Float D) {
        addClippingPlane(new EGS_ClippingPlane(A,D));
    };
    void clearClippingPlanes() {
        nclip = 0;
    };
    void setMaterialColor(int imed, const EGS_MaterialColor &Mat) {
        if (imed >= nmat) {
            EGS_MaterialColor *m = new EGS_MaterialColor [imed + 5];
            for (int j=0; j<nmat; j++) {
                m[j] = mat[j];
            }
            if (nmat > 0) {
                delete [] mat;
            }
            mat = m;
            nmat = imed+5;
        }
        mat[imed] = Mat;
    };
    void setMaterialColor(int imed, const EGS_Vector &d_color,
                          EGS_Float Alpha=1) {
        setMaterialColor(imed,EGS_MaterialColor(d_color,Alpha));
    };

    // normalize the image
    void normalizeImage(EGS_Vector *image, int n);

    // get the color for the screen point x.
    EGS_Vector getColor(const EGS_Vector &x, EGS_BaseGeometry *g,
                        const EGS_Float track_distance, const EGS_Float track_alpha, const bool track_clip, bool debug=false,
                        EGS_Vector bCol=EGS_Vector(1,1,1));
    void getRegions(const EGS_Vector &x, EGS_BaseGeometry *g, int *regions, EGS_Vector *colors, int maxreg);

    // render the entire image
    bool renderImage(EGS_BaseGeometry *g, int nx, int ny, EGS_Vector *image, int *abort_location=NULL);

    // render the particle tracks
    bool renderTracks(EGS_BaseGeometry *g, int nx, int ny, EGS_Vector *image, int *abort_location=NULL);

    // pick region
    void regionPick(int x, int y);

    void setParticleVisibility(int particle, bool vis) {
        if (m_tracks) {
            m_tracks->setParticleVisibility(particle, vis);
        }
    }

    EGS_Vector  xo;         // camera position
    EGS_Vector  x_screen;   // center of projected image
    EGS_Vector  v1_screen,  // 2 perpendicular vectors on the screen
                v2_screen;
    EGS_Float   sx, sy;     // the screen size

    EGS_Vector  global_ambient_light; // the global ambient light

    EGS_Light  **lights;              // positional lights
    int        nlight, ntot;          // and their number

    EGS_MaterialColor  *mat;          // material colors
    int                nmat;          // and their number

    EGS_ClippingPlane  **clip;        // clipping planes
    int          nclip, nclip_t;      // and their number

    EGS_TrackView   *m_tracks;
};

EGS_GeometryVisualizer::EGS_GeometryVisualizer() {
    p = new EGS_PrivateVisualizer;
}

EGS_GeometryVisualizer::~EGS_GeometryVisualizer() {
    delete p;
}

void EGS_GeometryVisualizer::loadTracksData(const char *fname) {
    if (p) {
        p->loadTracksData(fname);
    }
}

void EGS_GeometryVisualizer::setProjection(const EGS_Vector &camera_pos,
        const EGS_Vector &camera_look_at, EGS_Float distance,
        EGS_Float size_x, EGS_Float size_y) {
    p->setProjection(camera_pos,camera_look_at,distance,size_x,size_y);
}

void EGS_GeometryVisualizer::setProjection(const EGS_Vector &camera_pos,
        const EGS_Vector &Xo_screen, const EGS_Vector &V1_screen,
        const EGS_Vector &V2_screen, EGS_Float size_x, EGS_Float size_y) {
    p->setProjection(camera_pos,Xo_screen,V1_screen,V2_screen,size_x,size_y);
}

void EGS_GeometryVisualizer::setGlobalAmbientLight(const EGS_Vector &light) {
    p->setGlobalAmbientLight(light);
}

void EGS_GeometryVisualizer::addLight(const EGS_Vector &pos,
                                      const EGS_Vector &color) {
    p->addLight(pos,color);
}

void EGS_GeometryVisualizer::addLight(EGS_Light *l) {
    p->addLight(l);
}

void EGS_GeometryVisualizer::setLight(int j, const EGS_Vector &pos,
                                      const EGS_Vector &color) {
    p->setLight(j,pos,color);
}

void EGS_GeometryVisualizer::setLight(int j, EGS_Light *l) {
    p->setLight(j,l);
}

void EGS_GeometryVisualizer::setMaterialColor(int imed,
        const EGS_MaterialColor &Mat) {
    p->setMaterialColor(imed,Mat);
}

void EGS_GeometryVisualizer::setMaterialColor(int imed,
        const EGS_Vector &d_color, EGS_Float Alpha) {
    p->setMaterialColor(imed,d_color,Alpha);
}

void EGS_GeometryVisualizer::addClippingPlane(EGS_ClippingPlane *plane) {
    p->addClippingPlane(plane);
}

void EGS_GeometryVisualizer::addClippingPlane(const EGS_Vector &A, EGS_Float D) {
    p->addClippingPlane(new EGS_ClippingPlane(A,D));
}

bool EGS_GeometryVisualizer::renderImage(EGS_BaseGeometry *g, int nx, int ny,
        EGS_Vector *image, int *abort_location) {
    return p->renderImage(g,nx,ny,image,abort_location);
}

bool EGS_GeometryVisualizer::renderTracks(EGS_BaseGeometry *g, int nx, int ny,
        EGS_Vector *image, int *abort_location) {
    return p->renderTracks(g,nx,ny,image,abort_location);
}

EGS_Vector EGS_GeometryVisualizer::getColor(const EGS_Vector &x, EGS_BaseGeometry *g, const EGS_Float track_distance, const EGS_Float track_alpha, const bool track_clip) {
    return p->getColor(x,g,track_distance, track_alpha, track_clip);
}

void EGS_GeometryVisualizer::getRegions(const EGS_Vector &x, EGS_BaseGeometry *g, int *regions, EGS_Vector *colors, int maxreg) {
    p->getRegions(x,g, regions, colors, maxreg);
}

void EGS_GeometryVisualizer::regionPick(int x, int y) {
    p->regionPick(x, y);
}

void EGS_GeometryVisualizer::clearClippingPlanes() {
    p->clearClippingPlanes();
}


void EGS_PrivateVisualizer::getRegions(const EGS_Vector &x, EGS_BaseGeometry *g, int *regions, EGS_Vector *colors, int maxreg) {

    // returns a list of regions and colors (up to maxreg) encountered in the path from the camera position
    // to the screen point given by x.

    EGS_Vector  u(x-xo);
    EGS_Float   t = u.length();
    u *= (1./t);
    EGS_Vector  xs(xo);
    EGS_Float   tleft = t;
    EGS_Float   tclip = 0;
    int         jclip = -1;
    int         ireg=-1, imed;
    int         regcount=0;
    EGS_Vector  c;

    // assume we are not hitting any regions
    regions[0]=-1;

    // clipping planes
    if (nclip > 0) {

        // loop over clipping planes
        for (int j=0; j<nclip; j++) {

            // camera is "inside" clipping plane (get longest clipping distance)
            if (clip[j]->isInside(xo)) {
                EGS_Float tt = t;
                if (clip[j]->howfar(xo,u,tt)) {
                    if (tt > tclip) {
                        tclip = tt;
                        jclip = j;
                    }
                }
            }

            // camera is "outside" clipping plane (get shortest clipping distance)
            else {
                EGS_Float tt = t;
                if (clip[j]->howfar(xo,u,tt)) {
                    if (tt<tleft) {
                        tleft = tt;
                    }
                }
            }
        }

        // camera inside a clipping plane, get first region hit
        if (jclip >= 0) {
            if (tclip <= tleft) {
                xs += u*tclip;
                ireg = g->isWhere(xs);
                tleft -= tclip;
                t -= tclip;
                if (ireg >= 0) {
                    imed = g->medium(ireg);
                    if (imed >= 0) {
                        c = mat[imed].d*mat[imed].alpha;
                    }
                    // save region
                    regions[regcount]=ireg;
                    colors[regcount]=c;
                    regcount++;
                    if (regcount>=maxreg) {
                        return;
                    }
                }
            }
            else {
                return;
            }
        }
    }

    // save all regions
    do {
        c = EGS_Vector(0,0,0);
        t = tleft;
        int inew = g->howfar(ireg,xs,u,t,&imed,0);
        if (inew == ireg) {
            regions[regcount]=-1;
            return;
        }
        ireg = inew;
        if (ireg >= 0) {
            tleft -= t;
            xs += u*t;
            if (imed >= 0) {
                c = mat[imed].d*mat[imed].alpha;
            }
            regions[regcount]=ireg;
            colors[regcount]=c;
            regcount++;
            if (regcount>=maxreg) {
                return;
            }
        }
        else {
            regions[regcount]=-1;
            return;
        }
    }
    while (ireg >= 0);
}

//=============================================================================
// EGS_PrivateVisualizer::getColor
//=============================================================================
EGS_Vector EGS_PrivateVisualizer::getColor(const EGS_Vector &x,
        EGS_BaseGeometry *g,
        const EGS_Float track_distance,
        const EGS_Float track_alpha,
        const bool track_clip,
        const bool debug,
        const EGS_Vector track_color) {

    EGS_Vector  u(x-xo);                        // vector from camera to screen point
    EGS_Float   t = u.length();
    u *= (1./t);    // normalize direction vector
    EGS_Float   ttrack = track_distance;        // distance to track
    EGS_Vector  xs(xo);                         // screen position
    EGS_Float   tleft = t;                      // remaining distance
    EGS_Float   tclip = 0;                      // distance to clipping plane
    int         j_clip = -1;                    // clipping plane index
    int         ireg=-1, imed;                  // region and medium number
    EGS_Vector  c, c1;                          // colors
    EGS_Float   a=1, a1;                        // alphas
    EGS_Vector  cTrack = track_color*track_alpha;      // track color


    if (debug) egsWarning("getColor(xo=(%g,%g,%g),u=(%g,%g,%g),axis_distance=%g\n",
                              xo.x,xo.y,xo.z,u.x,u.y,u.z,track_distance);

    // handle clipping planes
    if (nclip > 0) {

        // loop over clipping planes
        for (int j=0; j<nclip; j++) {

            if (clip[j]->isInside(xo)) {                                        // we are inside the clipping plane
                EGS_Float tt = t;
                if (clip[j]->howfar(xo,u,tt)) {                                 // calculate distance to clipping plane
                    if (tt > tclip) {                                           // if this clipping plane is further away:
                        tclip = tt;                                             // save distance to furthest clipping plane
                        j_clip = j;                                             // save index of furthest clipping plane
                    }
                }
                else {
                    if (ttrack>0) {
                        return cTrack;    // don't forget the axis!
                    }
                    return c;
                }
            }

            else {                                                              // we are outside clipping plane
                if (clip[j]->howfar(xo,u,t)) {
                    tleft = t;    // remaining distance is that of the clipping plane
                }
            }
        }

        // there is at least one clipping plane for current ray
        if (j_clip >= 0) {

            // draw track that are in front of clipping plane
            if (ttrack>0 && ttrack<=tclip) {
                c = track_color*track_alpha*a;
                a = a*(1-track_alpha);
            }

            if (tclip <= tleft) {

                // move to clipping plane and get region number there
                xs += u*tclip;
                ireg = g->isWhere(xs);

                // decrease distances
                t       -= tclip;
                tleft   -= tclip;
                ttrack  -= tclip;

                // ray hits a region
                if (ireg >= 0) {
                    imed = g->medium(ireg);
                    if (imed >= 0) {
                        a1 = mat[imed].alpha;;
                        c1 = global_ambient_light.getScaled(mat[imed].d);
                        EGS_Vector n = clip[j_clip]->getNormal();
                        for (int j=0; j<nlight; j++) {
                            c1 += lights[j]->getColor(xs,n,mat[imed].d);
                        }
                        c += c1*a1*a;
                        a = a*(1-a1);
                        if (a < 0.001) {
                            return c;
                        }
                    }
                }
            }
            else {
                return c;
            }
        }
    }

    // first hit
    if (ireg < 0) {
        EGS_Vector n;
        int inew = g->howfar(ireg,xs,u,t,&imed,&n);
        if (debug) {
            egsWarning("enter: inew=%d t=%g imed=%d n=(%g,%g,%g)\n", inew,t,imed,n.x,n.y,n.z);
        }

        // hitting a track first
        if (ttrack>0 && (t>=ttrack || inew<0)) {
            c = track_color*track_alpha*a;
            a = a*(1-track_alpha);
            if (a < 0.001) {
                return c;
            }
        }

        // hitting a surface
        if (inew >= 0) {
            xs += u*t;
            if (imed >= 0) {
                a1 = mat[imed].alpha;;
                c1 = global_ambient_light.getScaled(mat[imed].d);
                for (int j=0; j<nlight; j++) {
                    c1 += lights[j]->getColor(xs,n,mat[imed].d);
                }
                c += c1*a1*a;
                a = a*(1-a1);
                if (a < 0.001) {
                    return c;
                }
            }

            // update region index and distances
            ireg    = inew;
            tleft  -= t;
            ttrack -= t;
        }
    }
    if (ireg < 0) {
        return c;
    }

    // translucent points
    if (debug) {
        egsWarning("c=(%g,%g,%g) a=%g\n",c.x,c.y,c.z,a);
    }
    do {
        EGS_Vector  n;
        int imed_new;
        t = tleft;

        int inew = g->howfar(ireg,xs,u,t,&imed_new,&n);

        // hitting a track
        if (ttrack>0 && t>=ttrack) {
            c += track_color*track_alpha*a;
            a = a*(1-track_alpha);
        }

        // avoid getting stuck
        if (inew == ireg) {
            break;
        }

        EGS_Vector c1;
        xs += u*t;

        // new region is not outside, new material is not vacuum, and there is a change in material
        if (inew >= 0 && imed_new >= 0 && imed_new != imed) {
            a1 = mat[imed_new].alpha;
            c1 = global_ambient_light.getScaled(mat[imed_new].d);
            for (int j=0; j<nlight; j++) {
                c1 += lights[j]->getColor(xs,n,mat[imed_new].d);
            }
            c += c1*a*a1;
            a = a*(1-a1);
            if (a < 0.001) {
                break;
            }
        }

        // update region index and distances
        ireg = inew;
        imed = imed_new;
        tleft  -= t;
        ttrack -= t;

    }
    while (ireg >= 0);

    if (ttrack>0) {
        return c+(cTrack)*a;    // draw the axis if not hit before
    }
    return c;
}

bool EGS_PrivateVisualizer::renderImage(EGS_BaseGeometry *g, int nx, int ny, EGS_Vector *image, int *abort_location) {

    EGS_Float dx = sx/nx, dy = sx/ny;
    EGS_Float rmax=1, gmax=1, bmax=1;
    EGS_Float ttrack=0, track_alpha = 1;

    bool debug = image ? false : true;
    if (debug) {
        egsWarning("\n*** renderImage(%d,%d)\n",nx,ny);
    }

    // render geometry in image buffer
    for (int j=0; j<ny; j++) {
        EGS_Float yy = -sy/2 + dy*(j+0.5);
        EGS_Vector xy(x_screen + v2_screen*yy);
        // Stop if abort condition is true
        if (abort_location && *abort_location) {
            return false;
        }
        for (int i=0; i<nx; i++) {
            EGS_Float xx = -sx/2 + dx*(i+0.5);
            EGS_Vector xp(xy + v1_screen*xx);
            EGS_Vector bCol(1,1,1);

            ttrack = -1;
            if (image) {
                int idx = i+j*nx;

                // negative z means we have a track, at a distance ttrack = -z
                if (image[idx].z<0) {
                    ttrack = -image[idx].z;
                    if (((image[idx].x > 0) || (image[idx].y > 0)) && m_tracks) {
                        track_alpha = 0.2+0.8*image[idx].y;
                        if (image[idx].x == 1.0) {
                            bCol.x=1;
                            bCol.y=1;
                            bCol.z=0.0;
                        }
                        if (image[idx].x == 2.0) {
                            bCol.x=1.0;
                            bCol.y=0.0;
                            bCol.z=0;
                        }
                        if (image[idx].x == 3.0) {
                            bCol.x=0;
                            bCol.y=0.0;
                            bCol.z=1.0;
                        }
                        if (image[idx].x == 4.0) {
                            bCol.x=1;
                            bCol.y=1;
                            bCol.z=1;
                        }
                        if (image[idx].x == 100.0) {
                            bCol.x=1;
                            bCol.y=1;
                            bCol.z=1;
                        }
                    }
                    image[idx] = EGS_Vector(0,0,0);
                }
            }

            EGS_Vector v = getColor(xp,g,ttrack,track_alpha,0,debug,bCol);
            if (debug) {
                egsWarning("ix=%d iy=%d v=(%g,%g,%g)\n",i,j,v.x,v.y,v.z);
            }
            if (v.x > rmax) {
                rmax = v.x;
            }
            if (v.y > gmax) {
                gmax = v.y;
            }
            if (v.z > bmax) {
                bmax = v.z;
            }
            if (image) {
                image[i+j*nx] = v;
            }
        }
    }

    // normalizeImage(image,nx*ny); return image;
    if (image) {
        if (rmax > 1 || gmax > 1 || bmax > 1) {
            EGS_Vector aux(1/rmax,1/gmax,1/bmax);
            for (int j=0; j<nx*ny; j++) {
                image[j].scale(aux);
            }
        }
    }
    return true;
}

bool EGS_PrivateVisualizer::renderTracks(EGS_BaseGeometry *g, int nx, int ny, EGS_Vector *image, int *abort_location) {
    if (m_tracks) {
        return m_tracks->renderTracks(nx, ny, image, abort_location);
    }
    return true;
}

void EGS_PrivateVisualizer::regionPick(int x, int y) {
    egsWarning("in region pick: xMouse=%d yMouse=%d\n",  x, y);

    // convert mouse coordinates to image coordinates;
}


void EGS_PrivateVisualizer::normalizeImage(EGS_Vector *image, int n) {
    EGS_Float rmax=1, gmax=1, bmax=1;
    for (int j=0; j<n; j++) {
        if (image[j].x > rmax) {
            rmax = image[j].x;
        }
        if (image[j].y > gmax) {
            gmax = image[j].y;
        }
        if (image[j].z > bmax) {
            bmax = image[j].z;
        }
    }
    if (rmax > 1 || gmax > 1 || bmax > 1) {
        EGS_Vector aux(1/rmax,1/gmax,1/bmax);
        for (int j=0; j<n; j++) {
            image[j].scale(aux);
        }
    }
}

EGS_PrivateVisualizer::~EGS_PrivateVisualizer() {
    if (nlight > 0) {
        for (int j=0; j<nlight; j++) {
            delete lights[j];
        }
        delete [] lights;
    }
    if (nmat  > 0) {
        delete [] mat;
    }
    if (nclip_t > 0) {
        for (int j=0; j<nclip; j++) {
            delete clip[j];
        }
        delete [] clip;
    }
}

void EGS_PrivateVisualizer::setProjection(const EGS_Vector &camera_pos,
        const EGS_Vector &camera_look_at, EGS_Float distance,
        EGS_Float size_x, EGS_Float size_y) {
    xo = camera_pos;
    EGS_Vector u(camera_look_at - xo);
    u.normalize();
    x_screen = xo + u*distance;
    v2_screen = EGS_Vector(0,u.z,-u.y);
    v2_screen.normalize();
    v1_screen = u%v2_screen;
    sx = size_x;
    sy = size_y;
}

void EGS_PrivateVisualizer::setProjection(const EGS_Vector &camera_pos,
        const EGS_Vector &Xo_screen, const EGS_Vector &V1_screen,
        const EGS_Vector &V2_screen,EGS_Float size_x, EGS_Float size_y) {
    xo = camera_pos;
    x_screen = Xo_screen;
    v1_screen = V1_screen;
    v2_screen = V2_screen;
    sx = size_x;
    sy = size_y;
    if (m_tracks) {
        m_tracks->setProjection(xo, x_screen, v1_screen, v2_screen, sx, sy);
    }
}

#ifdef HAVE_PNG
#include <png.h>
#include <fstream>

std::ofstream *png_ostream = 0;

static void egs_png_warning(png_structp, png_const_charp message) {
    egsWarning("libpng warning: %s\n",message);
}

static void egs_png_write_func(png_structp png_ptr, png_bytep data,
                               png_size_t length) {
    png_ostream->write((char *)data,length);
    //if( nw != length )
    //    png_error( png_ptr, "write error" );
}

static void egs_png_flush_func(png_structp png_ptr) {
    png_ostream->flush();
}

bool EGS_GeometryVisualizer::makePngImage(EGS_BaseGeometry *g,
        int xsize, int ysize, const char *fname) {
    // checks
    if (!fname) {
        egsWarning("EGS_GeometryVisualizer::makePngImage: file name must"
                   " not be null\n");
        return false;
    }
    png_ostream = new std::ofstream(fname, std::ios::binary | std::ios::trunc);
    if (!png_ostream) {
        egsWarning("EGS_GeometryVisualizer::makePngImage: "
                   "failed to open file %s for writing\n",fname);
        return false;
    }
    if (xsize <= 0 || ysize <= 0) {
        egsWarning("EGS_GeometryVisualizer::makePngImage: "
                   "image size must be greater than zero\n");
        return false;
    }

    // render the image
    EGS_Vector *image = new EGS_Vector [xsize*ysize];
    if (!p->renderTracks(g,xsize,ysize,image)) {
        egsWarning("Error while rendering particle tracks\n");
        return false;
    }

    if (!p->renderImage(g,xsize,ysize,image)) {
        egsWarning("Error while rendering image\n");
        return false;
    }

    // initialize png
    png_structp png_ptr;
    png_infop info_ptr;
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,0,0,0);
    if (!png_ptr) {
        return false;
    }
    png_set_error_fn(png_ptr, 0, 0, egs_png_warning);
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, 0);
        return false;
    }
    if (setjmp(png_ptr->jmpbuf)) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return false;
    }
    //png_set_compression_level(png_ptr,Z_BEST_COMPRESSION);
    png_set_write_fn(png_ptr, (void *)this, egs_png_write_func,
                     egs_png_flush_func);
    info_ptr->channels = 3;
    png_set_IHDR(png_ptr, info_ptr, xsize, ysize, 8, PNG_COLOR_TYPE_RGB, 0,0,0);
    info_ptr->sig_bit.red = 8;
    info_ptr->sig_bit.green = 8;
    info_ptr->sig_bit.blue = 8;

    // the follwoing should only be done on little endian machines
    png_set_bgr(png_ptr);

    // write the header.
    png_write_info(png_ptr, info_ptr);

    // the follwoing should be PNG_FILLER_BEFORE for big endian machines
    png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);

    // fill the image
    unsigned int **rows = new unsigned int *[ysize];
    unsigned int rgb = 0xff << 24;
    for (int j=0; j<ysize; j++) {
        rows[j] = new unsigned int [xsize];
        for (int i=0; i<xsize; i++) {
            EGS_Vector v = image[i+j*xsize];
            int r = (int)(v.x*255), g = (int)(v.y*255), b = (int)(v.z*255);
            rows[j][i] = (rgb | (r << 16) | (g << 8) | b);
        }
    }
    png_write_image(png_ptr, (unsigned char **)rows);

    // write the end
    png_write_end(png_ptr, info_ptr);

    png_destroy_write_struct(&png_ptr, &info_ptr);
    delete [] image;
    png_ostream->close();
    delete png_ostream;
    for (int j=0; j<ysize; j++) {
        delete rows[j];
    }
    delete [] rows;

    return true;

}

#endif

void EGS_GeometryVisualizer::setParticleVisibility(int particle, bool vis) {
    p->setParticleVisibility(particle, vis);
}
