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

#ifndef RENDERWORKER_H
#define RENDERWORKER_H

#include "egs_visualizer.h"
#include "egs_light.h"

#include <qobject.h>
#include <qimage.h>

#include <vector>

using std::vector;

typedef enum {
    Transformation, FullDetail, SavedImage
} RenderRequestType;

// This struct is designed to be passed by value.
struct RenderParameters {
    // Desired image size
    int nx;
    int ny;
    int nxr;
    int nyr;
    // Clipping planes
    vector<EGS_ClippingPlane> clipping_planes;
    // material colors
    vector<EGS_MaterialColor> material_colors;
    // lights
    vector<EGS_Light> lights;
    EGS_Vector global_ambient_light;
    // track rendering
    bool draw_tracks;
    bool show_photons;
    bool show_electrons;
    bool show_positrons;
    bool show_other;
    // viewport
    EGS_Vector camera;
    EGS_Vector camera_v1;
    EGS_Vector camera_v2;
    EGS_Vector screen_xo;
    EGS_Vector screen_v1;
    EGS_Vector screen_v2;
    EGS_Float projection_x;
    EGS_Float projection_y;
    // drawing axes (labels are offthread)
    bool draw_axes;
    bool draw_axeslabels;
    EGS_Vector axesmax;
    EGS_Float size;
    // passthrough: request type. Does this belong in the struct?
    RenderRequestType requestType;
};

struct RenderResults {
    QImage img;
    // scaling rules
    EGS_Vector axeslabelsX;
    EGS_Vector axeslabelsY;
    EGS_Vector axeslabelsZ;
    // misc info
    EGS_Float elapsedTime;
    EGS_Float trackTime;
};

void applyParameters(EGS_GeometryVisualizer *, const struct RenderParameters &);

class RenderWorker : public QObject {
    Q_OBJECT

public:

    RenderWorker();
    virtual ~RenderWorker();

    // Set this to a nonzero value to make all renders fail asap.
    int abort_location;

public slots:

    void loadTracks(QString fileName);

    void render(EGS_BaseGeometry *g, struct RenderParameters params);

signals:

    void aborted();
    void rendered(struct RenderResults, struct RenderParameters params);

private:
    void drawAxes(const struct RenderParameters &);

    EGS_GeometryVisualizer *vis;
    EGS_Vector *image;
    EGS_Vector axeslabelsX;
    EGS_Vector axeslabelsY;
    EGS_Vector axeslabelsZ;
    int nx_last;
    int ny_last;
};

#endif // RENDERWORKER_H
