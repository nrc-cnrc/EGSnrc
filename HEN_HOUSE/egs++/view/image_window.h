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
#
###############################################################################
*/


#ifndef IMAGE_WINDOW_
#define IMAGE_WINDOW_

#include "viewcontrol.h"
#include "renderworker.h"

#include "egs_libconfig.h"
#include "egs_functions.h"
#include "egs_visualizer.h"

#include <qpainter.h>

class QTimer;
class QThread;
class QProgressDialog;

// Maximum number of regions displayed
#define N_REG_MAX 30

class ImageWindow : public QWidget {

    Q_OBJECT

public:

    struct RenderParameters pars;

    ImageWindow(QWidget *parent=0, GeometryViewControl* gvc=0,
                const char *name=0);
    ~ImageWindow();
    
public slots:

    void rerender(EGS_BaseGeometry* geo);
    void loadTracks(QString name);
    void requestRegionPick();
    void saveView(EGS_BaseGeometry* geo, int nx, int ny, QString name, QString ext);

protected:

    void resizeEvent(QResizeEvent *e);
    void paintEvent (QPaintEvent *);

    void mouseReleaseEvent (QMouseEvent *event);

    //virtual void mousePressEvent ( QMouseEvent * e )
    //virtual void mouseReleaseEvent ( QMouseEvent * e )

    void mouseMoveEvent (QMouseEvent *event);

    void wheelEvent (QWheelEvent *event);

    void keyPressEvent (QKeyEvent *event);

protected slots:

    void drawResults(RenderResults,RenderParameters);

signals:

    void changedSize(int w, int h);
	void cameraRotation(int dx, int dy);
	void cameraZooming(int dy);
    void cameraRolling(int dx);
    void cameraTranslating(int dx, int dy);
    void cameraHoming();
    void cameraHomeDefining();
	void startTransformation();
	void endTransformation();
    void putCameraOnAxis(char axis);
    void leftMouseClick(int x, int y);
    void renderAndDebug();

    // for render thread
    void requestRender(EGS_BaseGeometry*,RenderParameters);
    void requestLoadTracks(QString);

private:
    void paintBackground(QPainter& p);

    // Navigation/Control
    bool    resizing;
    QTimer  *navigationTimer;
    bool    navigating;
    bool rerenderRequested;
    bool regionPickRequested;

    // regionPicking synchronized with image on screen
    EGS_GeometryVisualizer* vis;
    bool regionsDisplayed;
    // TODO avoid duplicate work!
    QPoint xyMouse;
    QPoint lastMouse;
    int lastRegions[N_REG_MAX];



    // Necessary hooks
    GeometryViewControl* gcontrol;

    // Worker thread handling
    QThread* thread;
    RenderWorker* worker;
    RenderResults lastResult;
    RenderParameters lastRequest;
    enum {WorkerIdle, WorkerCalculating, WorkerBackordered} renderState;
    int requestNo;//purely for tracking
    EGS_BaseGeometry* lastRequestGeo;

    // Image saving
    bool renderForImage;
    QString saveName;
    QString saveExtension;
    QProgressDialog* saveProgress;
};

#endif
