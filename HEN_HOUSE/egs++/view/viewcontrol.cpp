/*
###############################################################################
#
#  EGSnrc egs++ geometry viewer ciew control extensions
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
#                   Ernesto Mainegra-Hing
#                   Manuel Stoeckl
#
###############################################################################
*/

#include "image_window.h"
#include "saveimage.h"
#include "clippingplanes.h"
#include "viewcontrol.h"

#include "egs_libconfig.h"
#include "egs_functions.h"
#include "egs_base_geometry.h"
#include "egs_visualizer.h"
#include "egs_timer.h"
#include "egs_input.h"

#include <qmessagebox.h>
#include <qapplication.h>
#include <qstring.h>
#include <qcolordialog.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qprogressdialog.h>

#include <cmath>
#include <cstdlib>
#include <vector>
using namespace std;

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

/*
static unsigned char standard_red[] = {
   255, 0,     0,    0,    255,     255,    128,     0,         0,
// red, green, blue, cyan, magenta, yellow, darkred, darkgreen, darkblue,
     0,      128,         128,        192,        128};
// darkcyan, darkmagenta, darkyellow, lightgray,  darkgray
static unsigned char standard_green[] = {
     0, 255,   0,   255,      0,     255,    0,       128,       128,
   128,   0, 128,   192,    128};
static unsigned char standard_blue[] = {
     0,   0, 255,   255,    255,      0,      0,     0,       128,
   128, 128,   0,   192,    128};
static char* standard_colors[] = {
 "red","green","blue","cyan","magenta","yellow","darkred","darkgreen",
 "darkblue","darkcyan","darkmagenta","darkyellow","lightgray","darkgray"};
*/

static unsigned char standard_red[] = {
    255,   0,   0,   0, 255, 255, 128,   0,   0,   0, 128, 128, 191, 80,
    0,  0,   0,  0,  85, 255, 192, 128
};
static unsigned char standard_green[] = {
    0, 255,   0, 255,   0, 255,   0, 128,   0, 128,   0, 128,   0,  0,
    191, 80,   0,  0, 170, 170, 192, 128
};
static unsigned char standard_blue[] = {
    0,   0, 255, 255, 255,   0,   0,   0, 128, 128, 128,   0,   0,  0,
    0,  0, 191, 80, 127, 127, 192, 128
};
//
// EMH commented out since NOT in use anywhere !!!
//
// const static char* standard_colors[] = {
//      "red","green","blue","cyan","magenta","yellow","darkred","darkgreen",
//       "darkblue","darkcyan","darkmagenta","darkyellow","medium red",
//       "very dark red","medium green","very dark green","medium blue",
//       "very dark blue","nameless1","nameless2","lightgray","darkgray"};


GeometryViewControl::GeometryViewControl(QWidget *parent, const char *name)
    : QDialog(parent) {
    setObjectName(name);
    setupUi(this);

#ifdef VIEW_DEBUG
    egsWarning("In init()\n");
#endif
    g = 0;
    theta = 0;
    c_theta = cos(theta);
    s_theta = sin(theta);
    phi = 0;
    c_phi = cos(phi);
    s_phi = sin(phi);
    a_light = 0.25;
    int ilight = (int)(a_light*ambientLight->maximum());
    ambientLight->blockSignals(true);
    ambientLight->setValue(ilight);
    ambientLight->blockSignals(false);
    distance = 25;
    //dCourse->setValue(0);
//     dFine->setValue(25);
    dfine = 250;
    projection_scale = 1;
    look_at = EGS_Vector();
    setLookAtLineEdit();
    projection_x = 15;
    projection_y = 15;
    setProjectionLineEdit();
    p_light = EGS_Vector(0,0,distance);
    setLightLineEdit();
    camera = p_light;
    screen_xo = EGS_Vector();
    screen_v1 = EGS_Vector(1,0,0);
    screen_v2 = EGS_Vector(0,1,0);

    // various state variables
    showAxes = this->showAxesCheckbox->isChecked();
    showAxesLabels = this->showAxesLabelsCheckbox->isChecked();
    showRegions = this->showRegionsCheckbox->isChecked();
    showTracks = this->showTracksCheckbox->isChecked();
    showPhotonTracks = this->showPhotonsCheckbox->isChecked();
    showElectronTracks = this->showElectronsCheckbox->isChecked();
    showPositronTracks = this->showPositronsCheckbox->isChecked();
    showOtherTracks = this->showOthersCheckbox->isChecked();

    // camera orientation vectors (same as the screen vectors)
    camera_v1 = screen_v1;
    camera_v2 = screen_v2;

    // camera home position
    camera_home = camera;
    camera_home_v1 = camera_v1;
    camera_home_v2 = camera_v2;
    dfine_home = dfine;

    m_colors = 0;
    nmed = 0;

    gview = new ImageWindow(this,"gview");
    gview->resize(512,512);

    // connect signals and slots for mouse navigation
    connect(gview, SIGNAL(cameraRotation(int, int)), this, SLOT(cameraRotate(int, int)));
    connect(gview, SIGNAL(cameraZooming(int)), this, SLOT(cameraZoom(int)));
    connect(gview, SIGNAL(cameraHoming()), this, SLOT(cameraHome()));
    connect(gview, SIGNAL(cameraHomeDefining()), this, SLOT(cameraHomeDefine()));
    connect(gview, SIGNAL(cameraTranslating(int, int)), this, SLOT(cameraTranslate(int, int)));
    connect(gview, SIGNAL(cameraRolling(int)), this, SLOT(cameraRoll(int)));
    connect(gview, SIGNAL(putCameraOnAxis(char)), this, SLOT(cameraOnAxis(char)));
    connect(gview, SIGNAL(leftMouseClick(int,int)), this, SLOT(reportViewSettings(int,int)));

    save_image = new SaveImage(this,"save image");

    cplanes = new ClippingPlanesWidget;
    setExtension(cplanes);
    connect(cplanes,SIGNAL(clippingPlanesChanged()),
            this,SLOT(setClippingPlanes()));

    // set the widget to show near the left-upper corner of the screen
    move(QPoint(25,25));

}


GeometryViewControl::~GeometryViewControl() {
}

void GeometryViewControl::reloadInput() {

    // check that the file (still) exists
    QFile file(filename);
    if (!file.exists()) {
        egsWarning("\nFile %s does not exist anymore!\n\n",filename.toUtf8().constData());
        return;
    }

    // read the input file again
    EGS_Input input;
    input.setContentFromFile(filename.toUtf8().constData());

    // clear the current geometry
    gview->stopWorker();
    // Don't accept any more reload requests during this reload
    reloadButton->blockSignals(true);
    qApp->processEvents();
    // delete geometry
    if (g) {
        delete g;
        g = 0;
    }
    EGS_BaseGeometry::clearGeometries();

    // restart from scratch (copied from main.cpp)
    EGS_BaseGeometry *newGeom = EGS_BaseGeometry::createGeometry(&input);
    if (!newGeom) egsFatal("\nThe input file %s seems to not define a valid"
                               " geometry\n\n",filename.toUtf8().constData());
    EGS_Float xmin = -50, xmax = 50;
    EGS_Float ymin = -50, ymax = 50;
    EGS_Float zmin = -50, zmax = 50;
    EGS_Input *vc = input.takeInputItem("view control");
    std::vector<EGS_UserColor> user_colors;
    if (vc) {
        EGS_Float tmp;
        if (!vc->getInput("xmin",tmp)) {
            xmin = tmp;
        }
        if (!vc->getInput("xmax",tmp)) {
            xmax = tmp;
        }
        if (!vc->getInput("ymin",tmp)) {
            ymin = tmp;
        }
        if (!vc->getInput("ymax",tmp)) {
            ymax = tmp;
        }
        if (!vc->getInput("zmin",tmp)) {
            zmin = tmp;
        }
        if (!vc->getInput("zmax",tmp)) {
            zmax = tmp;
        }
        EGS_Input *uc;
        while ((uc = vc->takeInputItem("set color")) != 0) {
            vector<string> inp;
            int err = uc->getInput("set color",inp);
            if (!err && (inp.size() == 4 || inp.size() == 5)) {
                EGS_UserColor ucolor;
                ucolor.medname = inp[0];
                sscanf(inp[1].c_str(),"%d",&ucolor.red);
                sscanf(inp[2].c_str(),"%d",&ucolor.green);
                sscanf(inp[3].c_str(),"%d",&ucolor.blue);
                if (inp.size() == 5) {
                    sscanf(inp[4].c_str(),"%d",&ucolor.alpha);
                }
                else {
                    ucolor.alpha = 255;
                }
                user_colors.push_back(ucolor);
            }
            else {
                qWarning("Wrong 'set color' input");
            }
            delete uc;
        }
        delete vc;
    }
    // Start loading process
    gview->restartWorker();
    setGeometry(newGeom,user_colors,xmin,xmax,ymin,ymax,zmin,zmax,1);
    reloadButton->blockSignals(false);
}

void GeometryViewControl::setFilename(QString str) {
    filename = str;
}

void GeometryViewControl::setTracksFilename(QString str) {
    filename_tracks = str;
}

void GeometryViewControl::checkboxAxes(bool toggle) {
    this->showAxesLabelsCheckbox->setEnabled(toggle);
    showAxes = toggle;
    if (toggle==false) {
        showAxesLabels = false;
    }
    else {
        showAxesLabels = showAxesLabelsCheckbox->isChecked();
    }
    updateView();
}

void GeometryViewControl::checkboxAxesLabels(bool toggle) {
    showAxesLabels = toggle;
    updateView();
}

void GeometryViewControl::checkboxShowRegions(bool toggle) {
    showRegions = toggle;
}

void GeometryViewControl::checkboxShowTracks(bool toggle) {
    showTracks = toggle;
    updateView();
}

void GeometryViewControl::cameraHome() {

    // reset camera to home position
    camera = camera_home;
    camera_v1 = camera_home_v1;
    camera_v2 = camera_home_v2;

    // reset look_at
    look_at = look_at_home;
    setLookAtLineEdit();
    updateLookAtLineEdit();

    // reset zoom level
    dfine = dfine_home;
    projection_x = projection_scale*dfine;
    projection_y = projection_scale*dfine;

    // debug information
#ifdef VIEW_DEBUG
    egsWarning("In cameraHome()\n");
#endif

    // render
    setCameraPosition();
}

void GeometryViewControl::cameraOnAxis(char axis) {

    // set look_at point to the center of the object
    look_at = center;

    // get the distance to the look_at point
    EGS_Vector v0(look_at-camera);
    EGS_Float dist = v0.length();

    // put the camera on required axis
    if (axis=='x') {
        camera    = EGS_Vector(center.x+dist,center.y,center.z);
        camera_v1 = EGS_Vector(0,1,0);
        camera_v2 = EGS_Vector(0,0,1);
    }
    else if (axis=='X') {
        camera    = EGS_Vector(center.x-dist,center.y,center.z);
        camera_v1 = EGS_Vector(0,-1,0);
        camera_v2 = EGS_Vector(0,0,1);
    }
    else if (axis=='y') {
        camera    = EGS_Vector(center.x,center.y+dist,center.z);
        camera_v1 = EGS_Vector(-1,0,0);
        camera_v2 = EGS_Vector(0,0,1);
    }
    else if (axis=='Y') {
        camera    = EGS_Vector(center.x,center.y-dist,center.z);
        camera_v1 = EGS_Vector(1,0,0);
        camera_v2 = EGS_Vector(0,0,1);
    }
    else if (axis=='z') {
        camera    = EGS_Vector(center.x,center.y,center.z+dist);
        camera_v1 = EGS_Vector(1,0,0);
        camera_v2 = EGS_Vector(0,1,0);
    }
    else if (axis=='Z') {
        camera    = EGS_Vector(center.x,center.y,center.z-dist);
        camera_v1 = EGS_Vector(1,0,0);
        camera_v2 = EGS_Vector(0,-1,0);
    }

    // debug information
#ifdef VIEW_DEBUG
    egsWarning("In cameraOnAxis()\n");
#endif

    // render
    setCameraPosition();
}

// slots for camera view buttons
void GeometryViewControl::camera_x()  {
    cameraOnAxis('x');
}
void GeometryViewControl::camera_y()  {
    cameraOnAxis('y');
}
void GeometryViewControl::camera_z()  {
    cameraOnAxis('z');
}
void GeometryViewControl::camera_mx() {
    cameraOnAxis('X');
}
void GeometryViewControl::camera_my() {
    cameraOnAxis('Y');
}
void GeometryViewControl::camera_mz() {
    cameraOnAxis('Z');
}

void GeometryViewControl::cameraHomeDefine() {

    // reset camera to home position
    camera_home = camera;
    camera_home_v1 = camera_v1;
    camera_home_v2 = camera_v2;
    look_at_home = look_at;
    dfine_home = dfine;

    // debug information
#ifdef VIEW_DEBUG
    egsWarning("In cameraHomeDefine()\n");
#endif
}


void GeometryViewControl::cameraTranslate(int dx, int dy) {

    // distance from camera to aim point
    EGS_Float r = (camera-look_at).length();

    // compute displacement (in world units)
    EGS_Float tx = -r*dx*0.001;
    EGS_Float ty = r*dy*0.001;

    // translation
    camera  = camera  + camera_v2*ty + camera_v1*tx;
    look_at = look_at + camera_v2*ty + camera_v1*tx;

    // debug information
#ifdef VIEW_DEBUG
    egsWarning("In cameraTranslate()\n");
#endif

    // adjust look_at lineEdit
    setLookAtLineEdit();
    updateLookAtLineEdit();

    // render
    setCameraPosition();
}


void GeometryViewControl::cameraRotate(int dx, int dy) {

    // camera position vector and camera distance
    EGS_Vector v0 = camera - look_at;
    EGS_Float r = v0.length();

    // new position of camera
    v0 = v0 + (camera_v2*dy + camera_v1*-dx)*0.05*r;
    v0.normalize();
    camera = look_at + v0*r;

    // camera aim vector (unit vector pointing at look_at point)
    v0 *= -1;

    // new up and side vectors, v1 and v2
    camera_v1 = v0.times(camera_v2);
    camera_v1.normalize();
    camera_v2 = camera_v1.times(v0);
    camera_v2.normalize();

    // render
    setCameraPosition();
}

void GeometryViewControl::cameraRoll(int dx) {

    // camera aim vector
    EGS_Vector v0 = look_at - camera;
    v0.normalize();

    // new up vector
    camera_v1 = camera_v1 - camera_v2 * 0.1 * -dx;
    camera_v1.normalize();
    camera_v2 = camera_v1.times(v0);
    camera_v2.normalize();

    // render
    setCameraPosition();
}

void GeometryViewControl::cameraZoom(int dy) {
    static vector<EGS_Float> a_scale;
    static vector<EGS_Float> a_size;
    int dfine_max = 1000;
//Last step zooming in for current scale
    if (dfine+dy <= 1) {// negative dy
        a_scale.push_back(projection_scale);
        a_size.push_back(size);
        dfine = dfine_max;
        size = projection_x;
        projection_scale = size/EGS_Float(dfine);
    }
//Last step zooming out for current scale
    else if (dfine > dfine_max - dy) {
        if (a_scale.size()) {
            projection_scale = a_scale.back();
            size = a_size.back();
            dfine = int(projection_x/projection_scale);
            a_scale.pop_back();
            a_size.pop_back();
        }
        else {} //do nothing ...
    }
    else {
// 1 pixel mouse motion in y = 1 unit change in dfine
        dfine += dy;
    }
    projection_x = projection_scale*dfine;
    projection_y = projection_scale*dfine;

// debug information
#ifdef VIEW_DEBUG
    egsWarning("In cameraZoom(%d)\n", dy);
    egsWarning(" new dfine = %d\n", dfine);
    egsWarning(" size = %g projection_x = %g projection_scale = %g\n", size,projection_x,projection_scale);
#endif

// render
    updateView();
}

// void GeometryViewControl::cameraZoom(int dy) {
//
//  // 1 pixel mouse motion in y = 1 unit change in dfine
//  dfine += dy;
//  if (dfine<0)   dfine = 0;
//  if (dfine>1000) dfine = 1000;
//  projection_x = projection_scale*dfine;
//  projection_y = projection_scale*dfine;
// //   dFine->setValue((int)dfine);
//
// // debug information
// #ifdef VIEW_DEBUG
//     egsWarning("In cameraZoom(%d)\n", dy);
//     egsWarning(" new dfine = %d\n", dfine);
//     egsWarning(" size = %g projection_x = %g projection_scale = %g\n", size, projection_scale*dfine,projection_scale);
// #endif
//     cout << " size = " << size << "projection_x = " << projection_x << " projection_scale = " << projection_scale << endl;
//   // render
//   renderImage();
// }

void GeometryViewControl::thetaRotation(int Theta) {
#ifdef VIEW_DEBUG
    egsWarning("In thetaRotation(%d)\n",Theta);
#endif
    theta = Theta*M_PI/180;
    c_theta = cos(theta);
    s_theta = sin(theta);
    setCameraPosition();
}


void GeometryViewControl::phiRotation(int Phi) {
#ifdef VIEW_DEBUG
    egsWarning("In phiRotation(%d)\n",Phi);
#endif
    phi = Phi*M_PI/180;
    c_phi = cos(phi);
    s_phi = sin(phi);
    setCameraPosition();
}

void GeometryViewControl::setCameraPosition() {

    screen_xo = look_at - (camera-look_at)*(1/3.);
//  camera    = look_at + EGS_Vector(s_theta*s_phi,s_theta*c_phi,c_theta)*3*distance;
//  screen_xo = look_at - EGS_Vector(s_theta*s_phi,s_theta*c_phi,c_theta)*distance;

#ifdef VIEW_DEBUG
    egsWarning("camera: (%g,%g,%g) screen: (%g,%g,%g)\n",camera.x,camera.y,camera.z,screen_xo.x,screen_xo.y,screen_xo.z);
#endif

    screen_v1 = camera_v1;
    screen_v2 = camera_v2;
    if (moveLight->isChecked()) {
        p_light = camera;
        setLightLineEdit();
    }
    updateView();
}

void GeometryViewControl::changeDfine(int newdfine) {
#ifdef VIEW_DEBUG
    egsWarning("In changeDfine(%d)\n",newdfine);
#endif
    //projection_x = dfine; projection_y = dfine;
    projection_x = projection_scale*newdfine;
    projection_y = projection_scale*newdfine;
    dfine = newdfine;
    //egsWarning("dfine = %d projection_x = %g\n",projection_x,dfine);
    updateView();
    //distance = dfine + dCourse->value();
    //setCameraPosition();
}


// void GeometryViewControl::changeDcourse( int dcourse ) {
// #ifdef VIEW_DEBUG
//     egsWarning("In changeDcourse(%d)\n",dcourse);
// #endif
//     distance = dcourse + dFine->value();
//     setCameraPosition();
// }


void GeometryViewControl::changeAmbientLight(int alight) {
#ifdef VIEW_DEBUG
    egsWarning("In changeAmbientLight(%d)\n",alight);
#endif
    a_light = alight;
    updateView(true);
}


void GeometryViewControl::changeTransparency(int t) {
    int med = materialCB->currentIndex();
    QRgb c = m_colors[med];
    m_colors[med] = qRgba(qRed(c), qGreen(c), qBlue(c), t);
#ifdef VIEW_DEBUG
    egsWarning("In changeTransperancy(%d): set color to %d\n",t,m_colors[med]);
#endif
    setMaterialColor(med);
    updateView(true);
}


void GeometryViewControl::moveLightChanged(int toggle) {
#ifdef VIEW_DEBUG
    egsWarning("In moveLightChanged(%d)\n",toggle);
#endif
    if (!toggle) {
        lightPosGroup->setEnabled(true);
    }
    else {
        lightPosGroup->setEnabled(false);
    }
}


void GeometryViewControl::setLightPosition() {
#ifdef VIEW_DEBUG
    egsWarning("In setLightPosition()\n");
#endif
    bool ok_x, ok_y, ok_z;
    EGS_Float xx = lightX->text().toDouble(&ok_x),
              yy = lightY->text().toDouble(&ok_y),
              zz = lightZ->text().toDouble(&ok_z);
    if (ok_x) {
        p_light.x = xx;
    }
    else {
        lightX->setText(QString("%1").arg((double)p_light.x,0,'g',4));
    }
    if (ok_y) {
        p_light.y = yy;
    }
    else {
        lightY->setText(QString("%1").arg((double)p_light.y,0,'g',4));
    }
    if (ok_z) {
        p_light.z = zz;
    }
    else {
        lightZ->setText(QString("%1").arg((double)p_light.z,0,'g',4));
    }
    if (ok_x || ok_y || ok_z) {
        updateView();
    }
}


void GeometryViewControl::setLookAt() {
#ifdef VIEW_DEBUG
    egsWarning("In setLookAt()\n");
#endif
    bool ok_x, ok_y, ok_z;
    EGS_Float xx = lookX->text().toDouble(&ok_x),
              yy = lookY->text().toDouble(&ok_y),
              zz = lookZ->text().toDouble(&ok_z);
    if (ok_x) {
        look_at.x = xx;
    }
    else {
        lookX->setText(QString("%1").arg((double)look_at.x,0,'g',4));
    }
    if (ok_y) {
        look_at.y = yy;
    }
    else {
        lookY->setText(QString("%1").arg((double)look_at.y,0,'g',4));
    }
    if (ok_z) {
        look_at.z = zz;
    }
    else {
        lookZ->setText(QString("%1").arg((double)look_at.z,0,'g',4));
    }
    if (ok_x || ok_y || ok_z) {

        // camera pointing vector
        EGS_Vector v0 = look_at-camera;
        v0.normalize();

        // new up and side vectors, v1 and v2
        camera_v1 = v0.times(camera_v2);
        camera_v1.normalize();
        camera_v2 = camera_v1.times(v0);
        camera_v2.normalize();

        // update camera position
        setCameraPosition();
    }
}

void GeometryViewControl::loadTracksDialog() {
#ifdef VIEW_DEBUG
    egsWarning("In loadTracksDialog()\n");
#endif
    filename_tracks = QFileDialog::getOpenFileName(this,
                      "Select geometry definition file");
    if (filename_tracks.isEmpty()) {
        return;
    }
    gview->loadTracks(filename_tracks);
}

void GeometryViewControl::setProjectionSize() {
}


void GeometryViewControl::viewAllMaterials() {
#ifdef VIEW_DEBUG
    egsWarning("In viewAllMaterials()\n");
#endif
}

void GeometryViewControl::reportViewSettings(int x,int y) {
    // convert mouse coordinates to screen coordinate
    int w=gview->width();
    int h=gview->height();
    EGS_Float xscreen, yscreen;
    xscreen = (x-w/2)*projection_x/w;
    yscreen = -(y-h/2)*projection_y/h;
    EGS_Vector xp(screen_xo + screen_v2*yscreen + screen_v1*xscreen);
    egsWarning("In reportViewSettings(%d,%d): xp=(%g,%g,%g)\n",x,y,xp.x,xp.y,xp.z);
    EGS_Vector u(xp-camera);
    u.normalize();
    egsWarning(" camera=(%15.10f,%15.10f,%15.10f), u=(%14.10f,%14.10f,%14.10f)\n",camera.x,camera.y,camera.z,u.x,u.y,u.z);
}

void GeometryViewControl::quitApplication() {
    delete gview;
    close();
}

void GeometryViewControl::setProjectionLineEdit() {
}

void GeometryViewControl::setLightLineEdit() {
    lightX->setText(QString("%1").arg((double)p_light.x,0,'g',4));
    lightY->setText(QString("%1").arg((double)p_light.y,0,'g',4));
    lightZ->setText(QString("%1").arg((double)p_light.z,0,'g',4));
}

void GeometryViewControl::setLookAtLineEdit() {
    lookX->setText(QString("%1").arg((double)look_at.x,0,'g',4));
    lookY->setText(QString("%1").arg((double)look_at.y,0,'g',4));
    lookZ->setText(QString("%1").arg((double)look_at.z,0,'g',4));
}

void GeometryViewControl::updateLookAtLineEdit() {
    lookX->repaint();
    lookY->repaint();
    lookZ->repaint();
}

void GeometryViewControl::setMaterialColor(int /*j*/) {
//    EGS_Float r = ((EGS_Float) qRed(m_colors[j]))/255.;
//    EGS_Float g = ((EGS_Float) qGreen(m_colors[j]))/255.;
//    EGS_Float b = ((EGS_Float) qBlue(m_colors[j]))/255.;
//    EGS_Float alpha = ((EGS_Float) qAlpha(m_colors[j]))/255.;
//    vis->setMaterialColor(j,EGS_Vector(r,g,b),alpha);
}

int GeometryViewControl::setGeometry(
    EGS_BaseGeometry *geom,
    const std::vector<EGS_UserColor> &ucolors,
    EGS_Float xmin, EGS_Float xmax, EGS_Float ymin, EGS_Float ymax,
    EGS_Float zmin, EGS_Float zmax, int justReloading) {
    if (!geom) {
        egsWarning("setGeometry(): got null geometry\n");
        return 1;
    }
    g = geom;

    if (!filename_tracks.isEmpty()) {
        gview->loadTracks(filename_tracks);
    }

#ifdef VIEW_DEBUG
    qDebug("Got %d user defined colors",int(ucolors.size()));
#endif

    // first save user's current material settings if reloading
    int nSave = materialCB->count();
    QRgb *saveColors  = new QRgb[nSave];
    QString *saveName = new QString[nSave];
    QString saveCurrent = materialCB->currentText();
    if (justReloading && m_colors) {
        for (int i=0; i<nSave; i++) {
            saveColors[i] = m_colors[i];
            saveName[i]   = materialCB->itemText(i);
        }
    }

    // delete m_colors
    if (m_colors && nmed > 0) {
        delete [] m_colors;
    }

#ifdef VIEW_DEBUG
    egsWarning("In setGeometry(), geometry name is %s\n",g->getName().c_str());
#endif

    // get number of media from geometry
    nmed = g->nMedia();
    if (nmed < 1) {
        QMessageBox::critical(this,"Geometry error",
                              "The geometry defines no media",QMessageBox::Ok,0,0);
        return 1;
    }

    // set up material combo box items
    materialCB->clear();
    m_colors = new QRgb [nmed];
    for (int j=0; j<nmed; j++) {
        materialCB->insertItem(j, g->getMediumName(j));
    }
    int nstandard = sizeof(standard_red)/sizeof(unsigned char);
    int js = 0;
    {
        for (int j=0; j<nmed; j++) {
            string med_name = g->getMediumName(j);
            unsigned int i;
            for (i=0; i<ucolors.size(); ++i) if (med_name == ucolors[i].medname) {
                    break;
                }
            if (i < ucolors.size()) {
                m_colors[j] = qRgba(ucolors[i].red,ucolors[i].green,ucolors[i].blue,ucolors[i].alpha);
            }
            else {
                m_colors[j] = qRgba(standard_red[js], standard_green[js], standard_blue[js], 255);
                if ((++js) >= nstandard) {
                    js = 0;
                }
            }
        }
    }

    /*
    if( nmed <= nstandard ) {
        for(int j=0; j<nmed; j++) m_colors[j] = qRgba(standard_red[j], standard_green[j], standard_blue[j], 255);
    }
    else {
        egsWarning(" too many colors\n"); // TODO
        for(int j=0; j<nmed; j++) {
            int jj = j%nstandard;
            m_colors[j] = qRgba(standard_red[jj], standard_green[jj], standard_blue[jj], 255);
        }
    }
    */

    // copy user's saved setting (for media names that were defined before the reload)
    materialCB->setCurrentIndex(0);
    if (justReloading) {
        for (int j=0; j<nmed; j++) {
            QString medName = g->getMediumName(j);
            for (int k=0; k<nSave; k++) {
                if (materialCB->itemText(j) == saveName[k]) {
                    m_colors[j] = saveColors[k];
                }
            }
            if (materialCB->itemText(j) == saveCurrent) {
                materialCB->setCurrentIndex(j);
            }
        }
    }
    // add swatches in the combo box
    QPixmap pixmap(10,10);
    {
        for (int j=0; j<nmed; j++) {
            pixmap.fill(m_colors[j]);
            materialCB->setItemIcon(j,pixmap);
        }
    }

    // clean up saved settings
    delete [] saveColors;
    delete [] saveName;

    //showColor->setPaletteBackgroundColor(QColor(m_colors[materialCB->currentItem()]));
    {
        for (int j=0; j<nmed; j++) {
            setMaterialColor(j);
        }
    }

    QProgressDialog progress("Analyzing geometry...","&Cancel",0,130,this);
    progress.setObjectName("progress");
    progress.setMinimumDuration(500);
    EGS_Float dx = (xmax-xmin)/100, dy = (ymax-ymin)/100, dz = (zmax-zmin)/100;
    bool found = false;
    EGS_Vector xo(0,0,0);
    int ireg = g->isWhere(xo);
    // egsInformation("for (0,0,0) ireg=%d\n",ireg);
    if (ireg >= 0) {
        found = true;
        progress.setValue(100);
    }
    else {
        progress.setMaximum(130);
        // look for an inside point.
        for (int k=0; k<100; k++) {
            progress.setValue(k);
            qApp->processEvents();
            if (progress.wasCanceled()) {
                return 2;
            }
            xo.z = zmin + dz*(k+0.5);
            for (int i=0; i<100; i++) {
                xo.x = xmin + dx*(i+0.5);
                for (int j=0; j<100; j++) {
                    xo.y = ymin + dy*(j+0.5);
                    ireg = g->isWhere(xo);
                    if (ireg >= 0) {
                        found = true;
                        break;
                    }
                }
                if (found) {
                    break;
                }
            }
            if (found) {
                break;
            }
        }
        if (!found) {
            progress.setValue(132);
            qApp->processEvents();
            QMessageBox::critical(this,"Geometry error",
                                  "Failed to find a point that is inside the geometry",
                                  QMessageBox::Ok,0,0);
            return 3;
        }
        progress.setValue(100);
        qApp->processEvents();
        if (progress.wasCanceled()) {
            return 2;
        }
#ifdef VIEW_DEBUG
        egsWarning("found xo = (%g,%g,%g)\n",xo.x,xo.y,xo.z);
#endif
    }

    EGS_Vector pmin(1e10,1e10,1e10), pmax(-1e10,-1e10,-1e10);
    //egsWarning("xo = (%g,%g,%g)\n",xo.x,xo.y,xo.z);
    for (int isize=0; isize<6; isize++) {
        progress.setValue(100+5*isize);
        qApp->processEvents();
        if (progress.wasCanceled()) {
            return 2;
        }
        //egsWarning("================ side %d\n",isize);
        int max_step = g->getMaxStep();
        for (int i=0; i<100; i++) {
            for (int j=0; j<100; j++) {
                EGS_Vector u;
                if (isize == 0) {
                    u = EGS_Vector(xmin+dx*i,ymin+dy*j,zmin);
                }
                else if (isize == 1) {
                    u = EGS_Vector(xmin+dx*i,ymin+dy*j,zmax);
                }
                else if (isize == 2) {
                    u = EGS_Vector(xmin,ymin+dy*j,zmin+dz*i);
                }
                else if (isize == 3) {
                    u = EGS_Vector(xmax,ymin+dy*j,zmin+dz*i);
                }
                else if (isize == 4) {
                    u = EGS_Vector(xmin+dx*i,ymin,zmin+dz*j);
                }
                else {
                    u = EGS_Vector(xmin+dx*i,ymax,zmin+dz*j);
                }
                u -= xo;
                u.normalize();
                EGS_Vector x(xo);
                int ir = ireg;
                int nstep = 0;
                do {
                    EGS_Float t = 1e30;
                    int inew = g->howfar(ir,x,u,t);
                    if (inew == ir) {
                        break;
                    }
                    if (++nstep > max_step) {
                        if (nstep == max_step+1) {
                            egsWarning("\nMore than %d steps through geometry?\n",max_step);
                            egsWarning("u = (%g,%g,%g)\n",u.x,u.y,u.z);
                        }
                        egsWarning("ir=%d inew=%d x=(%g,%g,%g) t=%g\n",ir,inew,x.x,x.y,x.z,t);
                        if (nstep > max_step + 50) {
                            egsWarning("\nToo many steps, ignoring ray\n");
                            x = xo;
                            break;
                        }
                    }
                    x += u*t;
                    ir = inew;
                }
                while (ir >= 0);
                //egsWarning("u = (%g,%g,%g) xf = (%g,%g,%g)\n",u.x,u.y,u.z,
                //        x.x,x.y,x.z);
                if (x.x < pmin.x) {
                    pmin.x = x.x;
                }
                if (x.x > pmax.x) {
                    pmax.x = x.x;
                }
                if (x.y < pmin.y) {
                    pmin.y = x.y;
                }
                if (x.y > pmax.y) {
                    pmax.y = x.y;
                }
                if (x.z < pmin.z) {
                    pmin.z = x.z;
                }
                if (x.z > pmax.z) {
                    pmax.z = x.z;
                }
            }
        }
    }
    center = (pmin + pmax)*0.5;
    if (fabs(center.x) < 0.001) {
        center.x = 0;
    }
    if (fabs(center.y) < 0.001) {
        center.y = 0;
    }
    if (fabs(center.z) < 0.001) {
        center.z = 0;
    }
#ifdef VIEW_DEBUG
    egsWarning(" center: (%g,%g,%g)\n",center.x,center.y,center.z);
    egsWarning(" xmin: (%g,%g,%g)\n",pmin.x,pmin.y,pmin.z);
    egsWarning(" xmax: (%g,%g,%g)\n",pmax.x,pmax.y,pmax.z);
#endif
    EGS_Float xsize = (pmax.x - pmin.x)/2;
    EGS_Float ysize = (pmax.y - pmin.y)/2;
    EGS_Float zsize = (pmax.z - pmin.z)/2;
    size = xsize;
    if (ysize > size) {
        size = ysize;
    }
    if (zsize > size) {
        size = zsize;
    }
    axesmax  = pmax + EGS_Vector(size, size, size)*0.3;

    if (!justReloading) {
        distance = size*3.5; // ~2*sqrt(3);
        look_at = center;
        look_at_home = look_at;
        setLookAtLineEdit();
        if (distance > 60000) {
            egsWarning("too big: %g\n",size);
            distance = 9999;
            projection_x = 100;
            projection_y = 100;
        }
        else {
            //projection_x = 7*size; projection_y = 7*size;
            projection_x = 5*size;
            projection_y = 5*size;
            EGS_Float proj_max = 2*projection_x;
#ifdef VIEW_DEBUG
            egsWarning(" projection: %d max. projection: %d\n",(int) projection_x,
                       (int) proj_max+1);
#endif
            int dfine_max = 1000;//dFine->maxValue();
            //dFine->setMaxValue((int) proj_max+1);
            //dFine->setValue((int) projection_x);
            projection_scale = proj_max/dfine_max;
            //dFine->setValue((int) (projection_x/projection_scale));
            dfine = (int)(projection_x/projection_scale);
        }
        setProjectionLineEdit();
        //p_light = look_at+EGS_Vector(s_theta*s_phi,s_theta*s_phi,c_theta)*distance;
        p_light = look_at+EGS_Vector(s_theta*s_phi,s_theta*s_phi,c_theta)*3*distance;
        setLightLineEdit();
        camera = p_light;
        screen_xo = look_at-EGS_Vector(s_theta*s_phi,s_theta*s_phi,c_theta)*distance;
        screen_v1 = EGS_Vector(c_phi,-s_phi,0);
        screen_v2 = EGS_Vector(c_theta*s_phi,c_theta*c_phi,-s_theta);

        // camera orientation vectors (same as the screen vectors)
        camera_v1 = screen_v1;
        camera_v2 = screen_v2;

        // save camera home position
        camera_home = camera;
        camera_home_v1 = camera_v1;
        camera_home_v2 = camera_v2;
        dfine_home = dfine;
    }

    updateView();

    return 0;
}

void GeometryViewControl::updateView(bool transform) {
    // transfer
    RenderParameters &rp = gview->pars;
    rp.axesmax = axesmax;
    rp.camera = camera;
    rp.camera_v1 = camera_v1;
    rp.camera_v2 = camera_v2;
    //r.clipping_planes
    rp.draw_axes = showAxes;
    rp.draw_axeslabels = showAxesLabels;
    rp.draw_tracks = showTracks;
    float a = 0.01 * a_light;
    rp.global_ambient_light = EGS_Vector(a,a,a);

    rp.lights = vector<EGS_Light>();
    rp.lights.push_back(EGS_Light(p_light, EGS_Vector(1,1,1)));

    rp.material_colors = vector<EGS_MaterialColor>();
    for (int j=0; j<nmed; j++) {
        EGS_Float r = ((EGS_Float) qRed(m_colors[j]))/255.;
        EGS_Float g = ((EGS_Float) qGreen(m_colors[j]))/255.;
        EGS_Float b = ((EGS_Float) qBlue(m_colors[j]))/255.;
        EGS_Float alpha = ((EGS_Float) qAlpha(m_colors[j]))/255.;
        rp.material_colors.push_back(EGS_MaterialColor(EGS_Vector(r,g,b),alpha));
    }

    rp.clipping_planes = vector<EGS_ClippingPlane>();
    for (int j=0; j<cplanes->numPlanes(); j++) {
        EGS_Vector a;
        EGS_Float d;
        if (cplanes->getPlane(j,a,d)) {
            rp.clipping_planes.push_back(EGS_ClippingPlane(a,d));
        }
    }

    rp.projection_x = projection_x;
    rp.projection_y = projection_y;
    rp.screen_v1 = screen_v1;
    rp.screen_v2 = screen_v2;
    rp.screen_xo = screen_xo;
    rp.show_electrons = showElectronTracks;
    rp.show_other = showOtherTracks;
    rp.show_photons = showPhotonTracks;
    rp.show_positrons = showPositronTracks;
    rp.size = size;

    gview->render(g, transform);
}

void GeometryViewControl::updateColorLabel(int med) {
    // showColor->setPaletteBackgroundColor(QColor(m_colors[med]));
    transparency->setValue(qAlpha(m_colors[med]));
}

void GeometryViewControl::changeColor() {
#ifdef VIEW_DEBUG
    egsWarning("In changeColor()\n");
    egsWarning(" widget size = %d %d\n",width(),height());
#endif
    int med = materialCB->currentIndex();
    bool ok;
    QRgb newc = QColorDialog::getRgba(m_colors[med],&ok,this);
    if (ok) {
        m_colors[med] = newc;
        // showColor->setPaletteBackgroundColor(QColor(newc));
        QPixmap pixmap(10,10);
        pixmap.fill(m_colors[med]);
        materialCB->setItemIcon(med, pixmap);
        transparency->setValue(qAlpha(newc));
        setMaterialColor(med);
        updateView();
    }
}

void GeometryViewControl::saveImage() {
    int res = save_image->exec();
#ifdef VIEW_DEBUG
    egsWarning("GeometryViewControl::saveImage(): got %d\n",res);
#endif
    if (!res) {
        return;
    }
    int nx, ny;
    save_image->getImageSize(&nx,&ny);
    QString format = save_image->getImageFormat();
    QString fname = save_image->getImageFileName();
#ifdef VIEW_DEBUG
    egsWarning("\nAbout to save %dx%d image into file %s in format %s\n\n",
               nx,ny,fname.toUtf8().constData(),format.toUtf8().constData());
#endif
    gview->saveView(g,nx,ny,fname,format);
}


void GeometryViewControl::showHideOptions() {
#ifdef VIEW_DEBUG
    egsWarning("In showHideOptions(): shown = %d\n",
               cplanes->isVisible());
#endif
    showExtension(moreButton->isChecked());
//     if( !cplanes->isVisible() ) {
//         showExtension(true); //moreButton->setText("Hide");
//     }
//     else {
//         showExtension(false); //moreButton->setText("More...");
//     }
}

void GeometryViewControl::setClippingPlanes() {
#ifdef VIEW_DEBUG
    egsWarning("In GeometryViewControl::setClippingPlanes():\n");
#endif
//    vis->clearClippingPlanes();
//    for(int j=0; j<cplanes->numPlanes(); j++) {
//        EGS_Vector a; EGS_Float d;
//        if( cplanes->getPlane(j,a,d) ) {
//#ifdef VIEW_DEBUG
//            egsWarning("  got plane (%g,%g,%g) %g\n",a.x,a.y,a.z,d);
//#endif
////            vis->addClippingPlane(a,d);
//        }
//    }
    updateView();
}

void GeometryViewControl::showPhotonsCheckbox_toggled(bool toggle) {
    showPhotonTracks = toggle;
    updateView();
}

void GeometryViewControl::showElectronsCheckbox_toggled(bool toggle) {
    showElectronTracks = toggle;
    updateView();
}


void GeometryViewControl::showPositronsCheckbox_toggled(bool toggle) {
    showPositronTracks = toggle;
    updateView();
}


void GeometryViewControl::showOthersCheckbox_toggled(bool toggle) {
    showOtherTracks = toggle;
    updateView();
}

void GeometryViewControl::startTransformation() {
    gview->startTransformation();
}

void GeometryViewControl::endTransformation() {
    gview->endTransformation();
}
