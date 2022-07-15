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

#ifndef RENDERWORKER_H
#define RENDERWORKER_H

#include "egs_visualizer.h"
#include "egs_light.h"

#include <qobject.h>
#include <qimage.h>

#include <vector>

using std::vector;

typedef enum {
    ForScreen, SavedImage
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
    // Material colors
    vector<EGS_MaterialColor> material_colors;
    // Regions to show/hide
    vector<bool> show_regions;
    // Whether or not to allow show_regions to be used
    bool allowRegionSelection;
    // Transparency of dose
    EGS_Float doseTransparency;

    // Unordered map is much more efficient when there is not
    // dose in every region
    unordered_map<size_t, EGS_Float> score;
    unordered_map<size_t, EGS_Vector> scoreColor;

    // lights
    vector<EGS_Light> lights;
    EGS_Vector global_ambient_light;
    // track rendering
    bool draw_tracks;
    bool show_photons;
    bool show_electrons;
    bool show_positrons;
    vector<size_t> trackIndices;
    // viewport
    EGS_Vector camera;
    EGS_Vector camera_v1;
    EGS_Vector camera_v2;
    EGS_Vector screen_xo;
    EGS_Vector screen_v1;
    EGS_Vector screen_v2;
    EGS_Float projection_m;
    // drawing axes (labels are offthread)
    bool draw_axes;
    bool draw_axeslabels;
    EGS_Vector axesmax;
    EGS_Float size;
    // Purpose of request
    RenderRequestType requestType;

    // Various display colors
    // 0 - background
    // 1 - text
    // 2 - axis
    // 3 - photons
    // 4 - electrons
    // 5 - positrons
    vector<EGS_Vector> displayColors;
    bool energyScaling;
};

struct RenderResults {
    QImage img;
    // misc info
    EGS_Float elapsedTime; // Total render time
    EGS_Float timePerPixel; // Pixel-dependent time / num pixels before scaling
};

void applyParameters(EGS_GeometryVisualizer *, const struct RenderParameters &);

class RenderWorker : public QObject {
    Q_OBJECT

public:

    RenderWorker();
    virtual ~RenderWorker();

    // Set this to a nonzero value to make all renders fail asap.
    int abort_location;

    // Synchronous rendering (blocks and returns results)
    struct RenderResults renderSync(EGS_BaseGeometry *g, struct RenderParameters params);

public slots:

    void loadTracks(QString fileName);
    // Asynchronous rendering (values returned via signals)
    void render(EGS_BaseGeometry *g, struct RenderParameters params);

signals:

    void aborted();
    void rendered(struct RenderResults, struct RenderParameters params);
    void tracksLoaded(vector<size_t> ntracks);

private:
    void drawAxes(const struct RenderParameters &);

    EGS_GeometryVisualizer *vis;
    EGS_Vector *image;
    QRgb *buffer;
    EGS_Vector axeslabelsX;
    EGS_Vector axeslabelsY;
    EGS_Vector axeslabelsZ;
    int last_bufsize;
};

#endif // RENDERWORKER_H
