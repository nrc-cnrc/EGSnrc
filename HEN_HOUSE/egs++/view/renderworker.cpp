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
###############################################################################
*/

#include "renderworker.h"
#include "egs_visualizer.h"

#include "qdatetime.h"

RenderWorker::RenderWorker() {
    vis =  new EGS_GeometryVisualizer;
    image = NULL;
    nx_last = -1;
    ny_last = -1;
    abort_location = 0;
}
RenderWorker::~RenderWorker() {
    delete vis;
    if (image) {
        delete[] image;
    }
}

void RenderWorker::loadTracks(QString fileName) {
    vis->loadTracksData(fileName.toUtf8().constData());
}

void RenderWorker::drawAxes(const RenderParameters &p) {
    EGS_Vector axes[4];

    EGS_Vector v2_screen = p.camera_v2;
    EGS_Vector v1_screen = p.camera_v1;
    EGS_Float sx = p.projection_x;
    EGS_Float sy = p.projection_y;
    int nx = p.nx;
    int ny = p.ny;
    EGS_Float dx = sx/p.nx;
    EGS_Float dy = sx/p.ny;
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
        int i1 = (int) axes[k].x;
        int j1 = (int) axes[k].y;
        int n = abs(i1-i0);
        if (abs(j1-j0)>n) {
            n = abs(j1-j0);
        }
        // more than one axis pixel: loop over axis pixels
        if (n>0) {
            deltax = (i1-i0)/(float)n;
            deltay = (j1-j0)/(float)n;
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
        // just one axis pixel
        else if (i1>=0 && i1<nx && j1>=0 && j1<ny) {
            image[i1+j1*nx] = EGS_Vector(100,1.0,-taxis);
        }
    }
}

void applyParameters(EGS_GeometryVisualizer *vis, const struct RenderParameters &p) {
    vis->setProjection(p.camera,p.screen_xo,p.screen_v1,p.screen_v2,p.projection_x,p.projection_y);
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
}

void RenderWorker::render(EGS_BaseGeometry *g, struct RenderParameters p) {
    // wall-clock time, not CPU time; to optimize response
    QTime itime = QTime::currentTime();

    applyParameters(vis, p);

    // create image buffer, if new
    if (p.nx != nx_last || p.ny != ny_last) {
        delete[] image;
        image = new EGS_Vector[p.nx*p.ny];
        nx_last = p.nx;
        ny_last = p.ny;
    }

    // modifies image and sets axeslabels
    if (p.draw_axes) {
        drawAxes(p);
    }

    QTime pretracktime = QTime::currentTime();
    // render tracks
    if (p.draw_tracks) {
        vis->setParticleVisibility(1,p.show_photons);
        vis->setParticleVisibility(2,p.show_electrons);
        vis->setParticleVisibility(3,p.show_positrons);
        vis->setParticleVisibility(4,p.show_other);
        if (!vis->renderTracks(g,p.nx,p.ny,image,&abort_location)) {
            emit aborted();
            return;
        }
    }
    QTime posttracktime = QTime::currentTime();

    // render main geometry
    if (!vis->renderImage(g,p.nx,p.ny,image,&abort_location)) {
        emit aborted();
        return;
    }

    // transfer to image
    QImage img(p.nx, p.ny, QImage::Format_ARGB32);
    for (int j=0; j<p.ny; j++) {
        uint *sl = (uint *) img.scanLine(j);
        for (int i=0; i<p.nx; i++) {
            EGS_Vector v = image[i+(p.ny-j-1)*p.nx];
            int r = (int)(v.x*255), g = (int)(v.y*255), b = (int)(v.z*255);
            *(sl+i) = qRgb(r,g,b);
        }
    }

    QTime ftime = QTime::currentTime();

    struct RenderResults r;
    r.img = img;
    r.elapsedTime = itime.msecsTo(ftime);
    r.trackTime = pretracktime.msecsTo(posttracktime);
    r.axeslabelsX = axeslabelsX;
    r.axeslabelsY = axeslabelsY;
    r.axeslabelsZ = axeslabelsZ;
    emit rendered(r, p);
}

