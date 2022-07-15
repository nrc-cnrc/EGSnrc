/*
###############################################################################
#
#  EGSnrc egs++ geometry viewer render control
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
#  Author:          Manuel Stoeckl, 2015
#
#  Contributors:    Reid Townson
#
###############################################################################
*/

#include "renderworker.h"

#include "egs_visualizer.h"

#include <QDateTime>
#include <QPainter>
#include <QColor>

RenderWorker::RenderWorker() {
    vis =  new EGS_GeometryVisualizer;
    image = NULL;
    buffer = NULL;
    last_bufsize = -1;
    abort_location = 0;
}
RenderWorker::~RenderWorker() {
    delete vis;
    if (image) {
        delete[] image;
    }
    if (buffer) {
        delete[] buffer;
    }
}

void RenderWorker::loadTracks(QString fileName) {
    vector<size_t> ntracks = vis->loadTracksData(fileName.toUtf8().constData());
    emit tracksLoaded(ntracks);
}

void RenderWorker::drawAxes(const RenderParameters &p) {
    EGS_Vector axes[4];

    EGS_Vector v2_screen = p.camera_v2;
    EGS_Vector v1_screen = p.camera_v1;
    int nx = p.nx;
    int ny = p.ny;
    EGS_Float mx = p.nx * p.nxr;
    EGS_Float my = p.ny * p.nyr;
    EGS_Float sx = mx > my ? p.projection_m * mx / my : p.projection_m;
    EGS_Float sy = my > mx ? p.projection_m * my / mx : p.projection_m;
    EGS_Float dx = sx/p.nx;
    EGS_Float dy = sy/p.ny;
    EGS_Vector v0 = (p.screen_xo-p.camera);
    EGS_Float  r  = v0.length();
    EGS_Float taxis=0;
    v0.normalize();
    EGS_Vector vp;

    // origin
    vp = EGS_Vector(0,0,0) - p.camera;
    vp.normalize();
    vp = vp*(r/(v0*vp));
    axes[0].x = ((vp*v1_screen+sx/2.0)/dx-0.5);
    axes[0].y = ((vp*v2_screen+sy/2.0)/dy-0.5);

    // x axis
    vp = EGS_Vector(p.axesmax.x,0,0) - p.camera;
    vp.normalize();
    vp = vp*(r/(v0*vp));
    axes[1].x = ((vp*v1_screen+sx/2.0)/dx-0.5);
    axes[1].y = ((vp*v2_screen+sy/2.0)/dy-0.5);
    vp = EGS_Vector(p.axesmax.x+0.1*p.size, 0, 0) - p.camera;
    vp.normalize();
    vp = vp*(r/(v0*vp));
    axeslabelsX.x = ((vp*v1_screen+sx/2.0)/dx-0.5);
    axeslabelsX.y = ((vp*v2_screen+sy/2.0)/dy-0.5);

    // y axis
    vp = EGS_Vector(0,p.axesmax.y,0) - p.camera;
    vp.normalize();
    vp = vp*(r/(v0*vp));
    axes[2].x = ((vp*v1_screen+sx/2.0)/dx-0.5);
    axes[2].y = ((vp*v2_screen+sy/2.0)/dy-0.5);
    vp = EGS_Vector(0, p.axesmax.y+0.1*p.size, 0) - p.camera;
    vp.normalize();
    vp = vp*(r/(v0*vp));
    axeslabelsY.x = ((vp*v1_screen+sx/2.0)/dx-0.5);
    axeslabelsY.y = ((vp*v2_screen+sy/2.0)/dy-0.5);

    // z axis
    vp = EGS_Vector(0,0,p.axesmax.z) - p.camera;
    vp.normalize();
    vp = vp*(r/(v0*vp));
    axes[3].x = ((vp*v1_screen+sx/2.0)/dx-0.5);
    axes[3].y = ((vp*v2_screen+sy/2.0)/dy-0.5);
    vp = EGS_Vector(0, 0, p.axesmax.z+0.1*p.size) - p.camera;
    vp.normalize();
    vp = vp*(r/(v0*vp));
    axeslabelsZ.x = ((vp*v1_screen+sx/2.0)/dx-0.5);
    axeslabelsZ.y = ((vp*v2_screen+sy/2.0)/dy-0.5);

    // flag axis pixels with the (negative) distance to the axis, in z component (blue)
    int i0 = (int) axes[0].x;
    int j0 = (int) axes[0].y;
    EGS_Float deltax, deltay;
    // loop over axes
    for (int k=1; k<=3; k++) {
        // note: float->int casts overflow at high zoom levels.
        int i1 = (int) axes[k].x;
        int j1 = (int) axes[k].y;
        int di = i1 - i0;
        int dj = j1 - j0;
        // just one axis pixel; don't bother looping
        if (j1 == j0 && i1 == i0) {
            if (i1>=0 && i1<nx && j1>=0 && j1<ny) {
                image[i1+j1*nx] = EGS_Vector(100,1.0,-taxis);
            }
        }
        else {
            int n;
            if (abs(di) < abs(dj)) {
                int sign = j1 > j0 ? 1 : -1;
                deltax = sign*(float)(di) / (float)(dj);
                deltay = sign;
                n = abs(dj);
            }
            else {
                int sign = i1 > i0 ? 1 : -1;
                deltax = sign;
                deltay = sign*(float)(dj) / (float)(di);
                n = abs(di);
            }
            for (int t=0; t<=n; t++) {
                i1 = (int)(i0+t*deltax);
                j1 = (int)(j0+t*deltay);
                if (k==1) {
                    taxis = EGS_Vector(EGS_Vector(t*p.axesmax.x/n,0,0) - p.camera).length();
                }
                if (k==2) {
                    taxis = EGS_Vector(EGS_Vector(0,t*p.axesmax.y/n,0) - p.camera).length();
                }
                if (k==3) {
                    taxis = EGS_Vector(EGS_Vector(0,0,t*p.axesmax.z/n) - p.camera).length();
                }
                if (i1>=0 && i1<nx && j1>=0 && j1<ny) {
                    image[i1+j1*nx] = EGS_Vector(100,1.0,-taxis);
                }
            }
        }
    }
}

void applyParameters(EGS_GeometryVisualizer *vis, const struct RenderParameters &p) {
    EGS_Float mx = p.nx*p.nxr;
    EGS_Float my = p.ny*p.nyr;

    EGS_Float xscale, yscale, xdelta,ydelta;
    EGS_Vector center;
    if (mx > my) {
        xscale = p.projection_m*mx/my;
        yscale = p.projection_m;
        xdelta = (mx - my) / my;
        ydelta = 0.0;
    }
    else {
        xscale = p.projection_m;
        yscale = p.projection_m*my/mx;
        xdelta = 0.0;
        ydelta = (my - mx) / mx;
    }

    center = p.screen_xo -
             p.screen_v1 * (p.projection_m-xscale)*0.5 -
             p.screen_v2 * (p.projection_m-yscale)*0.5 -
             p.screen_v1 * (p.projection_m*xdelta)*0.5 -
             p.screen_v2 * (p.projection_m*ydelta)*0.5;

    vis->setProjection(p.camera,center,p.screen_v1,p.screen_v2,xscale,yscale);

    // set lights, planes, materials.
    for (size_t i=0; i<p.lights.size(); i++) {
        // all lights are white by default
        vis->setLight(i,new EGS_Light(p.lights[i]));
    }
    vis->clearClippingPlanes();
    for (size_t i=0; i<p.clipping_planes.size(); i++) {
        vis->addClippingPlane(new EGS_ClippingPlane(p.clipping_planes[i]));
    }
    for (size_t i=0; i<p.material_colors.size(); i++) {
        vis->setMaterialColor(i,p.material_colors[i]);
    }
    vis->setGlobalAmbientLight(p.global_ambient_light);
    vis->setShowRegions(p.show_regions);
    vis->setAllowRegionSelection(p.allowRegionSelection);
    vis->setDisplayColors(p.displayColors);
    vis->setEnergyScaling(p.energyScaling);
    vis->setScoreColors(p.scoreColor);
    vis->setDoseTransparency(p.doseTransparency);
}

void RenderWorker::render(EGS_BaseGeometry *g, struct RenderParameters p) {
    const struct RenderResults &r = renderSync(g, p);
    if (r.img.isNull()) {
        emit aborted();
        return;
    }
    emit rendered(r, p);
}

struct RenderResults RenderWorker::renderSync(EGS_BaseGeometry *g, struct RenderParameters p) {
    struct RenderResults r;
    r.img = QImage();
    r.elapsedTime = -1;
    r.timePerPixel = -1;

    // wall-clock time, not CPU time; to optimize response
    QTime time_i1 = QTime::currentTime();

    applyParameters(vis, p);

    // create image buffer if new size is far enough away from previous size
    // it overallocates slightly (~16%) if `buffer` is not used.
    int new_bufsize = p.nx * p.ny;
    if (new_bufsize > last_bufsize || last_bufsize > 3*new_bufsize) {
        if (image) {
            delete[] image;
        }
        if (buffer) {
            delete[] buffer;
        }
        image = new EGS_Vector[new_bufsize];
        buffer = new QRgb[new_bufsize];
        last_bufsize = new_bufsize;
    }

    // modifies image and sets axeslabels
    if (p.draw_axes) {
        drawAxes(p);
    }

    // render tracks
    if (p.draw_tracks) {
        vis->setTrackIndices(p.trackIndices);
        vis->setParticleVisibility(1,p.show_photons);
        vis->setParticleVisibility(2,p.show_electrons);
        vis->setParticleVisibility(3,p.show_positrons);
        if (!vis->renderTracks(p.nx,p.ny,image,&abort_location)) {
            // Undo track drawing and rezero image
            memset(image, 0, sizeof(EGS_Vector));
            return r;
        }
    }

    // render main geometry
    QTime time_r1 = QTime::currentTime();
    if (!vis->renderImage(g,p.nx,p.ny,image,&abort_location)) {
        return r;
    }
    QTime time_r2 = QTime::currentTime();

    // RGB32 saves 1+ image traversals over other formats w/ Qt4.7-8 & X11
    QImage img(p.nx*p.nxr, p.ny*p.nyr, QImage::Format_RGB32);
    if (p.nxr == 1 && p.nyr == 1) {
        // special case, straight to image
        for (int j=0; j<p.ny; j++) {
            QRgb *sl = (QRgb *) img.scanLine(j);
            for (int i=0; i<p.nx; i++) {
                EGS_Vector v = image[i+(p.ny-j-1)*p.nx];
                quint8 r = (quint8)(v.x*255), g = (quint8)(v.y*255), b = (quint8)(v.z*255);
                sl[i] = qRgb(r,g,b);
            }
        }
    }
    else {
        // Copy image into buffer
        for (int j=0; j<p.ny; j++) {
            // flip the image vertically in this phase (it shouldn't matter where)
            int topd = (p.ny-j-1)*p.nx;
            int botu = j*p.nx;
            for (int i=0; i<p.nx; i++) {
                EGS_Vector v = image[i+topd];
                quint8 r = (quint8)(v.x*255), g = (quint8)(v.y*255), b = (quint8)(v.z*255);
                buffer[botu+i] = qRgb(r,g,b);
            }
        }
        // Fast image scaling routine.
        int line_len = sizeof(QRgb)*p.nx*p.nxr;
        for (int j=0; j<p.ny; j++) {
            int start_row = j*p.nyr;
            QRgb *base = (QRgb *) img.scanLine(start_row);
            QRgb *al = buffer + j*p.nx;
            // Create a single image line
            for (int i=0,ib=0,mx=p.nxr; i<p.nx; i++,mx+=p.nxr) {
                QRgb v = al[i];
                for (; ib<mx; ib++) {
                    base[ib] = v;
                }
            }
            // Duplicate that line (p.nyr-1) times
            for (int i=1; i<p.nyr; i++) {
                memcpy(img.scanLine(start_row+i),base,line_len);
            }
        }
    }
    // Since we already have the image at full scale, draw the axes labels on it.
    {
        QPainter q(&img);
        if (p.draw_axeslabels) {
            q.setPen(QColor((int)(255*p.displayColors[1].x), (int)(255*p.displayColors[1].y), (int)(255*p.displayColors[1].z)));
            q.drawText((int)(p.nxr*axeslabelsX.x-3),p.nyr*p.ny-(int)(p.nyr*axeslabelsX.y-3),"x");
            q.drawText((int)(p.nxr*axeslabelsY.x-3),p.nyr*p.ny-(int)(p.nyr*axeslabelsY.y-3),"y");
            q.drawText((int)(p.nxr*axeslabelsZ.x-3),p.nyr*p.ny-(int)(p.nyr*axeslabelsZ.y-3),"z");
        }
    }

    QTime time_i2 = QTime::currentTime();

    r.img = img;
    r.elapsedTime = time_i1.msecsTo(time_i2);
    r.timePerPixel = ((EGS_Float)time_r1.msecsTo(time_r2)) / (p.nx * p.ny);

    return r;
}
