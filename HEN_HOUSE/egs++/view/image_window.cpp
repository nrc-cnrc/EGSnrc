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
#                   Max Orok
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
    regionsWanted = false;

    vis = new EGS_GeometryVisualizer;

    // register types so they can be transfered accross thread
    qRegisterMetaType<RenderParameters>("RenderParameters");
    qRegisterMetaType<RenderResults>("RenderResults");
    qRegisterMetaType<vector<size_t>>("vector<size_t>");

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
    if (!geo) {
        return;
    }
    if (transform) {
        startTransformation();
    }
    rerender(geo);
}

void ImageWindow::rerender(EGS_BaseGeometry *geo) {
    if (!thread || !geo) {
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
    pars.requestType = ForScreen;
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
    connect(worker, SIGNAL(tracksLoaded(vector<size_t>)), this, SLOT(trackResults(vector<size_t>)));
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

void ImageWindow::showRegions(bool show) {
    regionsWanted = show;
    if (regionsWanted) {
        rerenderRequested = true;
    }
    update();
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

    bool wasRerenderRequested = true;
    rerenderRequested = false;
    if (wasRerenderRequested) {
        QPainter p(this);
        p.drawImage(QPoint(0,0),r.img);
        p.end();
    }

    if (regionsDisplayed && !regionsWanted) {
        QPainter p(this);
        // repaint just the eclipsed region (painter has clip)
        p.drawImage(QPoint(0,0),r.img);
        p.end();
    }

    if (!navigating && regionsWanted) {
        // Don't recalculate an identical point, unless
        // the rerender wiped everything.
        if (!wasRerenderRequested && xyMouse == lastMouse) {
            return;
        }
        lastMouse = xyMouse;

        int w = (q.nx*q.nxr);
        int h = (q.ny*q.nyr);
        EGS_Float xscreen, yscreen;
        EGS_Float xscale = w > h ? q.projection_m * w / h : q.projection_m;
        EGS_Float yscale = h > w ? q.projection_m * h / w : q.projection_m;
        xscreen = (xyMouse.x()-w/2)*xscale/w;
        yscreen = -(xyMouse.y()-h/2)*yscale/h;
        EGS_Vector xp(q.screen_xo + q.screen_v2*yscreen + q.screen_v1*xscreen);

        int maxreg = N_REG_MAX;
        int regions[maxreg];
        EGS_Vector colors[N_REG_MAX];
        EGS_Vector hitCoord(0,0,0);
        EGS_Float hitScore = 0;
        vis->getRegions(xp, lastRequestGeo, regions, colors, maxreg, hitCoord, q.score, hitScore);
        if (!wasRerenderRequested && memcmp(regions, lastRegions, sizeof(lastRegions)) == 0) {
            return;
        }
        memcpy(lastRegions, regions, sizeof(lastRegions));

        int x0 = 10;
        int y0 = 20;
        int s  = 10;
        int dy = 15;
        QPainter p(this);

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

        for (EGS_I64 loopCount=0; loopCount<=loopMax; ++loopCount) {
            if (loopCount == loopMax) {
                egsFatal("ImageWindow::paintEvent: Too many iterations were required! Input may be invalid, or consider increasing loopMax.");
                return;
            }
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

        // The below code is very CPU-inefficient. Please optimize!
        if (regions[0]>=0) {
            // Background for the region list
            if (hitScore > 0.) {
                p.fillRect(QRect(0,0,79,h),QColor((int)(255*q.displayColors[0].x), (int)(255*q.displayColors[0].y), (int)(255*q.displayColors[0].z)));
            }
            else {
                p.fillRect(QRect(0,0,64,h),QColor((int)(255*q.displayColors[0].x), (int)(255*q.displayColors[0].y), (int)(255*q.displayColors[0].z)));
            }
            // Text color for the region list
            p.setPen(QColor((int)(255*q.displayColors[1].x), (int)(255*q.displayColors[1].y), (int)(255*q.displayColors[1].z)));

            p.drawText(9,y0,"Regions");
            y0+=10;
            regionsDisplayed=true;

            // Get the hit coordinates and score
            QString hitx = QString::number(hitCoord.x);
            QString hity = QString::number(hitCoord.y);
            QString hitz = QString::number(hitCoord.z);
            QString score;
            if (hitScore > 0.) {
                score = QString::number(hitScore);
            }

            // Determine the max number of digits to calculate the background fill
            int nChar = hitx.length();
            if (hity.length() > nChar) {
                nChar = hity.length();
            }
            else if (hitz.length() > nChar) {
                nChar = hitz.length();
            }
            else if (hitScore > 0. && score.length() > nChar) {
                nChar = score.length();
            }
            if (nChar > 4) {
                nChar -= 5;
            }

            if (hitScore > 0.) {
                p.fillRect(QRect(79,h-79,nChar*10,79),QColor((int)(255*q.displayColors[0].x), (int)(255*q.displayColors[0].y), (int)(255*q.displayColors[0].z)));
                p.drawText(9,h-79,"Surface");
                p.drawText(9,h-60,hitx);
                p.drawText(9,h-45,hity);
                p.drawText(9,h-30,hitz);

                if (q.scoreColor.count(regions[0])) {
                    EGS_Vector sc = q.scoreColor.at(regions[0]);
                    p.fillRect(x0, h-15-s, s, s, QColor((int)(255*sc.x),(int)(255*sc.y),(int)(255*sc.z)));
                }
                p.drawRect(x0, h-15-s, s, s);
                p.drawText(x0+s+3,h-15,score);
            }
            else {
                p.fillRect(QRect(64,h-64,nChar*10,64),QColor((int)(255*q.displayColors[0].x), (int)(255*q.displayColors[0].y), (int)(255*q.displayColors[0].z)));
                p.drawText(9,h-64,"Surface");
                p.drawText(9,h-45,hitx);
                p.drawText(9,h-30,hity);
                p.drawText(9,h-15,hitz);
            }
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

        for (int reg = 0; reg < maxreg && regions[reg] >= 0; reg++) {
            p.fillRect(x0, y0+reg*dy, s, s,
                       QColor((int)(255*colors[reg].x), (int)(255*colors[reg].y),
                              (int)(255*colors[reg].z)));
            p.setPen(QColor((int)(255*q.displayColors[1].x), (int)(255*q.displayColors[1].y), (int)(255*q.displayColors[1].z)));
            p.drawRect(x0, y0+reg*dy, s, s);
            p.drawText(x0+s+3,y0+reg*dy+s,QString::number(regions[reg]));
            if (reg+1 == maxreg) {
                p.drawText(x0,y0+(reg+1)*dy+s,"...");
            }
        }
        p.end();
    }
}

void ImageWindow::mouseDoubleClickEvent(QMouseEvent *event) {
#ifdef VIEW_DEBUG
    egsWarning("In mouseDoubleClickEvent(): mouse location = (%d, %d)\n", event->x(), event->y());
    egsWarning("  Mouse buttons: %0x\n", event->button());
#endif
    // 500 msec before returning to full resolution (after button released)
    if (navigating) {
        navigationTimer->start(500);
        navigating=false;
    }
    else if (event->button() == Qt::LeftButton) {

#ifdef VIEW_DEBUG
        egsWarning("double click event at %d %d\n",event->x(),event->y());
#endif

        const RenderParameters &q = lastRequest;
        int w = (q.nx*q.nxr);
        int h = (q.ny*q.nyr);
        EGS_Float xscreen, yscreen;
        EGS_Float xscale = w > h ? q.projection_m * w / h : q.projection_m;
        EGS_Float yscale = h > w ? q.projection_m * h / w : q.projection_m;
        xscreen = (xyMouse.x()-w/2)*xscale/w;
        yscreen = -(xyMouse.y()-h/2)*yscale/h;

        // Get the coordinate on the surface
        EGS_Vector hitCoord(0,0,0);
        EGS_Vector xp(q.screen_xo + q.screen_v2*yscreen + q.screen_v1*xscreen);
        vis->getFirstHit(xp, lastRequestGeo, hitCoord);

#ifdef VIEW_DEBUG
        egsWarning("double click xp %f %f %f %f %f\n",q.screen_xo.x, q.screen_xo.y,q.screen_xo.z, yscreen, xscreen);
#endif

        emit leftDoubleClick(hitCoord);

#ifdef VIEW_DEBUG
        egsWarning("double click hit (x,y,z) = %f %f %f\n",hitCoord.x, hitCoord.y, hitCoord.z);
#endif
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

void ImageWindow::trackResults(vector<size_t> ntracks) {
    emit tracksLoaded(ntracks);
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
