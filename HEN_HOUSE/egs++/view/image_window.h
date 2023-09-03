
/*
###############################################################################
#
#  EGSnrc egs++ geometry viewer image window headers
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
#                   Manuel Stoeckl
#                   Reid Townson
#
###############################################################################
*/


#ifndef IMAGE_WINDOW_
#define IMAGE_WINDOW_

#include "renderworker.h"

#include "egs_libconfig.h"
#include "egs_functions.h"
#include "egs_visualizer.h"

#include <qpainter.h>
#include <qwidget.h>

class QTimer;
class QThread;
class QProgressDialog;

// Maximum number of regions displayed
#define N_REG_MAX 100

class ImageWindow : public QWidget {

    Q_OBJECT

public:

    struct RenderParameters pars;

    ImageWindow(QWidget *parent=0, const char *name=0);
    ~ImageWindow();

public slots:

    void render(EGS_BaseGeometry *geo, bool transform);
    void loadTracks(QString name);
    void saveView(EGS_BaseGeometry *geo, int nx, int ny, QString name, QString ext);

    void stopWorker();
    void restartWorker();

    void startTransformation();
    void endTransformation();

    void showRegions(bool show);

protected:

    void rerender(EGS_BaseGeometry *geo);

    void resizeEvent(QResizeEvent *e);
    void paintEvent(QPaintEvent *);

    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);

protected slots:

    void drawResults(RenderResults,RenderParameters);
    void trackResults(vector<size_t>);
    void handleAbort();

signals:

    void changedSize(int w, int h);
    void cameraRotation(int dx, int dy);
    void cameraZooming(int dy);
    void cameraRolling(int dx);
    void cameraTranslating(int dx, int dy);
    void cameraHoming();
    void cameraHomeDefining();
    void putCameraOnAxis(char axis);
    void leftMouseClick(int x, int y);
    void leftDoubleClick(EGS_Vector hitCoord);
    void saveComplete();
    void tracksLoaded(vector<size_t>);

    // for render thread
    void requestRender(EGS_BaseGeometry *,RenderParameters);
    void requestLoadTracks(QString);

private:
    void paintBackground(QPainter &p);

    // Navigation/Control
    QTimer  *navigationTimer;
    bool    navigating;
    bool rerenderRequested;

    // regionPicking synchronized with image on screen
    EGS_GeometryVisualizer *vis;
    bool regionsDisplayed;
    bool regionsWanted;
    QPoint xyMouse;
    QPoint lastMouse;
    int lastRegions[N_REG_MAX];

    // Worker thread handling
    QThread *thread;
    RenderWorker *worker;
    RenderResults lastResult;
    RenderParameters lastRequest;
    enum {WorkerIdle, WorkerCalculating, WorkerBackordered} renderState;
    EGS_BaseGeometry *lastRequestGeo;
    bool wasLastRequestSlow;

    // Image saving
    QString saveName;
    QString saveExtension;
    bool isSaving;
};

#endif
