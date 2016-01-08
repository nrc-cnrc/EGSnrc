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
    saveProgress = NULL;
    regionsDisplayed = true;

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
};

/* no longer in Qt4. What was this supposed to do?
  void polish() {
      //QDialog::polish();
      QWidget::polish();
      QWidget *topl = topLevelWidget();
      //egsWarning("In polish: position: %d %d\n",pos().x(),pos().y());
      //if( !topl ) egsWarning("Null top level widget!\n");
      QWidget *parent = parentWidget();
      if( !parent ) parent = topl;
      //egsWarning("parent: %s\n",parent->name());
      if( parent ) {
          QPoint point = parent->mapToGlobal(QPoint(0,0));
          //egsWarning("parent: %d %d\n",point.x(),point.y());
          //QRect my_frame = frameGeometry();
          //egsWarning("my geometry: %d %d %d %d\n",my_frame.left(),
          //     my_frame.right(),my_frame.top(),my_frame.bottom());
          int gview_x = point.x();
          int gview_y = point.y() + parent->height();
          //egsWarning("moving to %d %d\n",gview_x,gview_y);
          move(gview_x,gview_y);
          //my_frame = frameGeometry();
          //egsWarning("my geometry: %d %d %d %d\n",my_frame.left(),
          //     my_frame.right(),my_frame.top(),my_frame.bottom());
      }
  };
  */

void ImageWindow::render(EGS_BaseGeometry *geo, bool transform) {
    if (transform) {
        startTransformation();
    }
    if (navigating) {
        pars.requestType = Transformation;
    }
    else {
        pars.requestType = FullDetail;
    }
    rerender(geo);
}

void ImageWindow::rerender(EGS_BaseGeometry *geo) {
    if (!thread) {
        // Don't bother if the thread has been disabled
        return;
    }

    lastRequestGeo = geo;

    // Determine scaling depending on if the request is to
    // draw at full detail or to be interactive. This flag _should_ be local

    if (pars.requestType == FullDetail) {
        pars.nx = this->width();
        pars.ny = this->height();
        pars.nxr = 1;
        pars.nyr = 1;
    }
    else if (pars.requestType == Transformation) {
        // Dynamically select a good render mode
        int nx=this->width(),ny=this->height();
        int nxr=nx,nyr=ny;
        if (lastResult.elapsedTime > 0) {
            // Cut out the track time because that is a fixed overhead.
            // Otherwise, when you have a few hundred megabytes of tracks,
            // the view decays to a single pixel and you _still_ get lag.
            // The proper fix is some sort of track level-of-detail mechanism.
            EGS_Float timePerPixel = (lastResult.elapsedTime - lastResult.trackTime) / (lastRequest.nx * lastRequest.ny);
            EGS_Float target = 30.0; // msecs per frame
            EGS_Float scale = nx*ny * timePerPixel / target;

            if (scale > 1) {
                scale = 1./sqrt(scale);
                int nnx = (int)(scale*nx), nny = (int)(scale*ny);
                if (nnx < 1) {
                    nnx = 1;
                }
                if (nny < 1) {
                    nny = 1;
                }
                nxr = nx/nnx;
                if (nxr*nnx != nx) {
                    nxr++;
                }
                nyr = ny/nny;
                if (nyr*nny != ny) {
                    nyr++;
                }
                nnx = nx/nxr;
                nny = ny/nyr;
                if (nnx*nxr < nx) {
                    nnx++;
                }
                if (nny*nyr < ny) {
                    nny++;
                }
#ifdef VIEW_DEBUG
                egsWarning(" nx=%d ny=%d nnx=%d nny=%d nxr=%d nyr=%d\n",
                           nx,ny,nnx,nny,nxr,nyr);
#endif
                nx = nnx;
                ny = nny;
            }
            else {
                nxr = 1;
                nyr = 1;
            }
        }
        else {
            // First-time scale values.
            nxr = 4;
            nyr = 4;
            int nnx = nx/nxr, nny = ny/nyr;
            if (nnx*nxr < nx) {
                nx = nnx+1;
            }
            else {
                nx = nnx;
            }
            if (nny*nyr < ny) {
                ny = nny+1;
            }
            else {
                ny = nny;
            }
        }
        pars.nx = nx;
        pars.ny = ny;
        pars.nxr = nxr;
        pars.nyr = nyr;
    }
    else {
        // sizes subject to external control.
    }

    // TODO: need some fast-abort method for the worker, to cancel
    // old jobs IFF they are of lower priority than the current one

    if (renderState == WorkerIdle) {
        activeRequestType = pars.requestType;
        emit requestRender(lastRequestGeo,pars);
        renderState = WorkerCalculating;
    }
    else if (renderState == WorkerCalculating) {
        // abort only to interrupt a full-detail calculation by a transformation
        // since the latter makes the former invalid.
        if (activeRequestType == FullDetail && pars.requestType == Transformation) {
            worker->abort_location = 1;
        }
        renderState = WorkerBackordered;
    }
}

void ImageWindow::loadTracks(QString name) {
    emit requestLoadTracks(name);
}


void ImageWindow::saveView(EGS_BaseGeometry *geo, int nx, int ny, QString name, QString ext) {
    saveName = name;
    saveExtension = ext;
    // Temporarily change parameters to render at new resolution
    int oldnx = pars.nx;
    int oldny = pars.ny;
    RenderRequestType oldrq = pars.requestType;
    pars.nx = nx;
    pars.ny = ny;
    pars.requestType = SavedImage;
    rerender(geo);
    pars.nx = oldnx;
    pars.ny = oldny;
    pars.requestType = oldrq;
    saveProgress = new QProgressDialog("Saving image", "&Cancel", 0, 2, this);
    saveProgress->setMinimumDuration(500);
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
        startTransformation();
        render(lastRequestGeo, false);
    }
};

void ImageWindow::paintBackground(QPainter &p) {
    const RenderParameters &q = lastRequest;
    const RenderResults &r = lastResult;
    if (q.nxr == 1 && q.nyr == 1) {
        p.drawImage(QPoint(0,0),r.img);
    }
    else {
        p.drawImage(QRect(0,0,q.nxr * q.nx, q.nyr * q.ny),r.img);
    }

    if (q.draw_axeslabels) {
        p.setPen(QColor(255,255,255));
        p.drawText((int)(q.nxr*r.axeslabelsX.x-3),q.nyr*q.ny-(int)(q.nyr*r.axeslabelsX.y-3),"x");
        p.drawText((int)(q.nxr*r.axeslabelsY.x-3),q.nyr*q.ny-(int)(q.nyr*r.axeslabelsY.y-3),"y");
        p.drawText((int)(q.nxr*r.axeslabelsZ.x-3),q.nyr*q.ny-(int)(q.nyr*r.axeslabelsZ.y-3),"z");
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
        paintBackground(p);
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
                paintBackground(p);
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
};

void ImageWindow::wheelEvent(QWheelEvent *event) {
#ifdef VIEW_DEBUG
    egsWarning("In wheelEvent(): mouse location = (%d, %d)\n", event->x(), event->y());
    egsWarning("  Buttons: %0x\n", event->buttons());
#endif
    startTransformation();
    emit cameraZooming(event->delta()/20);
};

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
};

void ImageWindow::drawResults(RenderResults r, RenderParameters q) {
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

    if (lastRequest.requestType == SavedImage) {
        if (saveProgress) {
            if (!saveProgress->wasCanceled()) {
                lastResult.img.save(saveName, saveExtension.toLatin1().constData());
            }
            delete saveProgress;
            saveProgress = NULL;
        }
    }
    else {
        if (saveProgress) {
            saveProgress->setValue(1);
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
    }
}
