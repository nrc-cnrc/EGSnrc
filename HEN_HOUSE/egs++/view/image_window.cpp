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

#include "image_window.h"

#include <QResizeEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QThread>
#include <QProgressDialog>
#include <QTimer>
#include <QDateTime>

// The below shim exists to ensure that QThread by default
// runs an event loop for older Qt versions.
#if QT_VERSION < 0x040400
class Thread : public QThread {
public:
    Thread() : QThread() {}
    void run() {
        exec();
    }
};
#else
typedef QThread Thread;
#endif



ImageWindow::ImageWindow(QWidget *parent, const char *name) :
    QWidget(parent,Qt::Window) {
    setObjectName(name);

    navigationTimer = new QTimer(this);
    navigationTimer->setSingleShot(true);
    connect(navigationTimer, SIGNAL(timeout()), this, SLOT(endTransformation()));

    navigating=false;
    setMouseTracking(true);
    rerenderRequested = false;

    renderState = WorkerIdle;
    lastResult.elapsedTime = -1.;

    lastRequestGeo = NULL;
    wasLastRequestSlow = false;
    regionsDisplayed = true;

    isSaving = false;

    vis = new EGS_GeometryVisualizer;

    // register types so they can be transfered accross thread
    qRegisterMetaType<RenderParameters>("RenderParameters");
    qRegisterMetaType<RenderResults>("RenderResults");

    // Initialize render worker and put it in a thread
    restartWorker();

    // disable Qt's background refill for the Widget, so we can paint
    // over our existing buffer when picking regions
    setAttribute(Qt::WA_OpaquePaintEvent);
}

ImageWindow::~ImageWindow() {
    stopWorker();
    delete navigationTimer;
    delete vis;
}

void ImageWindow::render(EGS_BaseGeometry *geo, bool transform) {
    if (transform) {
        startTransformation();
    }
    rerender(geo);
}

void ImageWindow::rerender(EGS_BaseGeometry *geo) {
    if (!thread) {
        // Don't bother if the thread has been disabled.
        return;
    }

    lastRequestGeo = geo;

    if (renderState == WorkerCalculating || renderState == WorkerBackordered) {
        // Abort slow full detail renders when fast response is needed
        if (navigating && wasLastRequestSlow && !isSaving) {
            worker->abort_location = 1;
        }
        // Busy, will call once current task is complete
        renderState = WorkerBackordered;
        return;
    }
    // It should be the case that renderState == WorkerIdle.

    // Draw at full scale only if we are not currently in a transformation
    int nx=this->width(),ny=this->height();
    int nxr=1,nyr=1;
    wasLastRequestSlow = false;
    if (!navigating) {
        // Full detail, keep defaults. May be slow.
        wasLastRequestSlow = true;
    }
    else if (lastResult.elapsedTime <= 0) {
        // No previous measurements, so guess
        nxr = 4;
        nyr = 4;
    }
    else {
        // Dynamic scaling. Ignore the fixed costs of rendering, and tune
        // the pixel-dependent costs to take a specific amount of time.
        EGS_Float target = 30.0; // msecs per frame
        EGS_Float timePerPixel = lastResult.timePerPixel;
        EGS_Float scale = nx*ny * timePerPixel / target;
        if (scale > 1) {
            int sc = (int)sqrt(scale);
            nxr = sc;
            nyr = sc;
            if (nxr * nyr < scale) {
                nxr++;
            }
            if (nxr * nyr < scale) {
                nyr++;
            }
            // prefer divisors
            if (nx % nxr != 0 && nx % (nxr+1) == 0) {
                nxr++;
            }
            if (ny % nyr != 0 && ny % (nyr+1) == 0) {
                nyr++;
            }
        }
        else {
            // Fast enough that preemption isn't necessary
        }
    }
    // Determine # of pixels needed
    int nnx = nx/nxr;
    int nny = ny/nyr;
    if (nnx*nxr < nx) {
        nnx++;
    }
    if (nny*nyr < ny) {
        nny++;
    }
    pars.nx = nnx;
    pars.ny = nny;
    pars.nxr = nxr;
    pars.nyr = nyr;
#ifdef VIEW_DEBUG
    egsWarning(" nx=%d ny=%d nnx=%d nny=%d nxr=%d nyr=%d\n",
               nx,ny,nnx,nny,nxr,nyr);
#endif

    renderState = WorkerCalculating;
    emit requestRender(lastRequestGeo, pars);
}

void ImageWindow::saveView(EGS_BaseGeometry *geo, int nx, int ny, QString name, QString ext) {
    if (isSaving) {
        // Ignore new image request, as the old hasn't completed
        return;
    }

    lastRequestGeo = geo;

    saveName = name;
    saveExtension = ext;
    // Temporarily change parameters to render at new resolution
    int oldnx = pars.nx;
    int oldny = pars.ny;
    RenderRequestType oldrq = pars.requestType;
    pars.nx = nx;
    pars.ny = ny;
    pars.requestType = SavedImage;
    // Queue directly to maintain correct state
    emit requestRender(lastRequestGeo, pars);

    pars.nx = oldnx;
    pars.ny = oldny;
    pars.requestType = oldrq;

    isSaving = true;
}


void ImageWindow::loadTracks(QString name) {
    emit requestLoadTracks(name);
}

void ImageWindow::stopWorker() {
    if (thread) {
        // stop the present task
        worker->abort_location = 1;
        thread->quit();
        thread->wait();
        delete thread;
        delete worker;
        lastRequestGeo = NULL;
        worker = NULL;
        thread = NULL;
    }
}


void ImageWindow::restartWorker() {
    worker = new RenderWorker();
    thread = new Thread();
    worker->moveToThread(thread);
    connect(this, SIGNAL(requestRender(EGS_BaseGeometry *,RenderParameters)),
            worker, SLOT(render(EGS_BaseGeometry *,RenderParameters)));
    connect(this, SIGNAL(requestLoadTracks(QString)), worker, SLOT(loadTracks(QString)));
    connect(worker, SIGNAL(rendered(RenderResults,RenderParameters)), this, SLOT(drawResults(RenderResults,RenderParameters)));
    connect(worker, SIGNAL(aborted()), this, SLOT(handleAbort()));
    thread->start();
    renderState = WorkerIdle;
}

void ImageWindow::startTransformation() {
    navigationTimer->stop();
    navigationTimer->start(500);
    navigating = true;
}

void ImageWindow::endTransformation() {
    navigationTimer->stop();
    navigating = false;
    if (lastRequestGeo) {
        render(lastRequestGeo, false);
    }
}

void ImageWindow::resizeEvent(QResizeEvent *e) {
#ifdef VIEW_DEBUG
    egsWarning("In resizeEvent(): size is %d %d old size is: %d %d" " shown: %d\n",width(),height(),e->oldSize().width(), e->oldSize().height(),isVisible());
#endif
    QWidget::resizeEvent(e);

    if (e->size() != e->oldSize() && lastRequestGeo) {
        // treat this as a transformation, since more resizes tend to follow
        render(lastRequestGeo, true);
    }
}

void ImageWindow::paintEvent(QPaintEvent *) {
    if (!thread) {
        // in geometry change period
        return;
    }

    const RenderParameters &q = lastRequest;
    const RenderResults &r = lastResult;
    // only draw if there was already a request
    if (r.img.isNull() || lastRequestGeo == NULL) {
        return;
    }

    bool wasRerenderRequested = rerenderRequested;
    rerenderRequested = false;
    if (wasRerenderRequested) {
        QPainter p(this);
        p.drawImage(QPoint(0,0),r.img);
        p.end();
    }

    if (!navigating) {
        // Don't recalculate an identical point, unless
        // the rerender wiped everything.
        if (!wasRerenderRequested && xyMouse == lastMouse) {
            return;
        }
        lastMouse = xyMouse;

        int w = (q.nx*q.nxr);
        int h = (q.ny*q.nyr);
        EGS_Float xscreen, yscreen;
        xscreen = (xyMouse.x()-w/2)*q.projection_x/w;
        yscreen = -(xyMouse.y()-h/2)*q.projection_y/h;
        EGS_Vector xp(q.screen_xo + q.screen_v2*yscreen + q.screen_v1*xscreen);

        int maxreg=N_REG_MAX;
        int regions[N_REG_MAX];
        EGS_Vector colors[N_REG_MAX];
        vis->getRegions(xp, lastRequestGeo, regions, colors, maxreg);
        if (!wasRerenderRequested && memcmp(regions, lastRegions, sizeof(lastRegions)) == 0) {
            return;
        }
        memcpy(lastRegions, regions, sizeof(lastRegions));

        int x0 = 10;
        int y0 = 20;
        int s  = 10;
        int dy = 15;
        QPainter p(this);
        QRect coveredRegion(0,0,64,h);
        p.setClipRect(coveredRegion);

        // The below code is very CPU-inefficient. Please optimize!
        if (regions[0]>=0) {
            p.fillRect(coveredRegion,QColor(0,0,0));
            p.setPen(QColor(255,255,255));
            p.drawText(x0-1,y0,"Regions");
            y0+=10;
            regionsDisplayed=true;
        }
        else {
            if (regionsDisplayed) {
                regionsDisplayed=false;
                // repaint just the eclipsed region (painter has clip)
                p.drawImage(QPoint(0,0),r.img);
            }
            p.end();
            return;
        }

        QFont font(p.font());

        // Numbers *should* be rendered in tabular format,
        // so we need only iterate on the longest number.
        // If that assumption proves wrong, then we can
        // iterate on the longest string, since point sizes
        // shouldn't change that.
        int mxval = 0;
        for (int j=0; j<maxreg; j++) {
            if (regions[j] < 0) {
                break;
            }
            if (regions[j] > mxval) {
                mxval = regions[j];
            }
        }
        QString mxstring = QString::number(mxval);

        while (1) {
            int wmax = p.fontMetrics().width(mxstring);
            if (wmax < 41) {
                break;
            }
            int npix = font.pixelSize(), npoint = font.pointSize();
            if (npix > 0) {
                font.setPixelSize(npix-1);
                //qWarning("Reducing font size to %d pixels",npix-1);
            }
            else {
                font.setPointSize(npoint-1);
                //qWarning("Reducing font size to %d points",npoint-1);
            }
            p.setFont(font);
        }

        for (int reg = 0; reg < maxreg && regions[reg] >= 0; reg++) {
            p.fillRect(x0, y0+reg*dy, s, s,
                       QColor((int)(255*colors[reg].x), (int)(255*colors[reg].y),
                              (int)(255*colors[reg].z)));
            p.setPen(QColor(255,255,255));
            p.drawRect(x0, y0+reg*dy, s, s);
            p.drawText(x0+s+3,y0+reg*dy+s,QString::number(regions[reg]));
            if (reg+1 == maxreg) {
                p.drawText(x0,y0+(reg+1)*dy+s,"...");
            }
        }
        p.end();
    }
}

void ImageWindow::mouseReleaseEvent(QMouseEvent *event) {
#ifdef VIEW_DEBUG
    egsWarning("In mouseReleaseEvent(): mouse location = (%d, %d)\n", event->x(), event->y());
    egsWarning("  Mouse buttons: %0x\n", event->button());
#endif
    // 500 msec before returning to full resolution (after button released)
    if (navigating) {
        navigationTimer->start(500);
        navigating=false;
    }
    else if (event->button() == Qt::LeftButton) {
        egsWarning("release event at %d %d\n",event->x(),event->y());
        emit leftMouseClick(event->x(),event->y());
    }
}

void ImageWindow::mouseMoveEvent(QMouseEvent *event) {
    int dx = event->x()-xyMouse.x();
    int dy = event->y()-xyMouse.y();
    xyMouse = event->pos();

    // set up navigation
    if (event->buttons() & (Qt::LeftButton|Qt::MidButton)) {
        // Keep the timer off so long holds with depressed button work
        navigating  = true;
        navigationTimer->stop();
    }

    // navigate
    if (event->buttons() & Qt::LeftButton) {
        // camera roll
        if (event->modifiers() & Qt::ShiftModifier) {
            emit cameraRolling(dx);
        }
        // camera translate
        else if (event->modifiers() & Qt::ControlModifier) {
            emit cameraTranslating(dx, dy);
        }
        // camera rotate
        else {
            emit cameraRotation(dx, dy);
        }
    }
    else if (event->buttons() & Qt::MidButton) {
        // camera zoom
        emit cameraZooming(-dy);
    }
    else {
        // picking
        this->update();
    }
}

void ImageWindow::wheelEvent(QWheelEvent *event) {
#ifdef VIEW_DEBUG
    egsWarning("In wheelEvent(): mouse location = (%d, %d)\n", event->x(), event->y());
    egsWarning("  Buttons: %0x\n", event->buttons());
#endif
    startTransformation();
    emit cameraZooming(event->delta()/20);
}

void ImageWindow::keyPressEvent(QKeyEvent *event) {
#ifdef VIEW_DEBUG
    egsWarning("In keyPressEvent()\n");
#endif
    if (event->key() == Qt::Key_Home) {
        if (event->modifiers() & Qt::AltModifier) {
            emit cameraHomeDefining();
        }
        else {
            emit(cameraHoming());
        }
    }
    else if (event->key() == Qt::Key_X) {
        emit putCameraOnAxis('x');
    }
    else if (event->key() == Qt::Key_Y) {
        emit putCameraOnAxis('y');
    }
    else if (event->key() == Qt::Key_Z) {
        emit putCameraOnAxis('z');
    }
//    else if (event->key() == Qt::Key_D) emit renderAndDebug();
    else {
        (event->ignore());
    }
}

void ImageWindow::drawResults(RenderResults r, RenderParameters q) {
    if (q.requestType == SavedImage) {
        // Short circuit images (they don't show up)
        r.img.save(saveName, saveExtension.toLatin1().constData());
        isSaving = false;
        emit saveComplete();
        return;
    }

    lastResult = r;
    lastRequest = q;
    // update the render thread status and queue next image if necessary
    switch (renderState) {
    case WorkerBackordered:
        renderState = WorkerIdle;
        rerender(lastRequestGeo);
        break;
    case WorkerCalculating:
        renderState = WorkerIdle;
        break;
    case WorkerIdle:
        qCritical("Yikes! Unexpected request fulfillment.");
        break;
    }

    rerenderRequested = true;

    if (!this->isVisible()) {
        this->show();
    }

    // Synchronize the local visualizer precisely with
    // what is currently on screen.
    applyParameters(vis, lastRequest);

    repaint();
}

void ImageWindow::handleAbort() {
    // Clear abort flag on worker.
    if (worker) {
        worker->abort_location = 0;
        if (renderState == WorkerBackordered) {
            // i.e., another task to run
            renderState = WorkerIdle;
            rerender(lastRequestGeo);
        }
        else if (renderState == WorkerCalculating) {
            renderState = WorkerIdle;
        }
    }
}
