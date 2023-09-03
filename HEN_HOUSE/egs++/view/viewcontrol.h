/*
###############################################################################
#
#  EGSnrc egs++ geometry viewer view control extensions
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
#                   Reid Townson
#
###############################################################################
*/

#ifndef GEOMETRYVIEWCONTROL_H
#define GEOMETRYVIEWCONTROL_H

#include "ui_viewcontrol.h"

#include <vector>
#include "egs_user_color.h"
#include "egs_vector.h"

#include <QMainWindow>

class EGS_BaseGeometry;
class EGS_GeometryVisualizer;
class ImageWindow;
class QLabel;
class QImage;
class SaveImage;
class ClippingPlanesWidget;


class GeometryViewControl : public QMainWindow, public Ui::GeometryViewControl {
    Q_OBJECT

public:

    GeometryViewControl(QWidget *parent = 0, const char *name = 0);
    virtual ~GeometryViewControl();

    virtual void setFilename(QString str);
    virtual void setTracksFilename(QString str);
    virtual void setCameraPosition();
    virtual void setProjectionLineEdit();
    virtual void setLightLineEdit();
    virtual void setLookAtLineEdit();
    virtual void updateLookAtLineEdit();
    virtual void setCameraLineEdit();
    virtual void updateCameraLineEdit();
    virtual int setGeometry(EGS_BaseGeometry *geom, const std::vector<EGS_UserColor> &ucolors, EGS_Float xmin, EGS_Float xmax, EGS_Float ymin, EGS_Float ymax, EGS_Float zmin, EGS_Float zmax, bool justReloading);
    virtual void updateView(bool transform = false);
    virtual bool loadInput(bool first_time, EGS_BaseGeometry *simGeom = 0);
    virtual void loadConfig(QString configFilename);
    virtual EGS_Vector getHeatMapColor(EGS_Float value);
    virtual void updateRegionTable();
    virtual void updateRegionTable(int imed);
    virtual void updateAusgabObjects(bool loadUserDose=false);
    virtual void initColorSwatches();

public slots:

    virtual void reloadInput();
    virtual void selectInput();
    virtual void loadDose();
    virtual void loadConfig();
    virtual void saveConfig();
    virtual void updateSimulationGeometry(int ind);
    virtual void checkboxAxes(bool toggle);
    virtual void checkboxAxesLabels(bool toggle);
    virtual void checkboxShowRegions(bool toggle);
    virtual void cameraHome();
    virtual void cameraOnAxis(char axis);
    virtual void camera_x();
    virtual void camera_y();
    virtual void camera_z();
    virtual void camera_mx();
    virtual void camera_my();
    virtual void camera_mz();
    virtual void cameraHomeDefine();
    virtual void cameraTranslate(int dx, int dy);
    virtual void cameraRotate(int dx, int dy);
    virtual void cameraRoll(int dx);
    virtual void cameraZoom(int dy);
    virtual void thetaRotation(int Theta);
    virtual void phiRotation(int Phi);
    virtual void changeAmbientLight(int alight);
    virtual void changeTransparency(int t);
    virtual void moveLightChanged(int toggle);
    virtual void setLightPosition();
    virtual void setLookAt();
    virtual void setLookPosition();
    virtual void loadTracksDialog();
    virtual void viewAllMaterials();
    virtual void reportViewSettings(int x, int y);
    virtual void setRotationPoint(EGS_Vector hitCoord);
    virtual void quitApplication();
    virtual void updateColorLabel(int med);
    virtual void changeColor();
    virtual void setBackgroundColor();
    virtual void setTextColor();
    virtual void setAxisColor();
    virtual void setPhotonColor();
    virtual void setElectronColor();
    virtual void setPositronColor();
    virtual void setEnergyScaling(bool toggle);
    virtual void saveImage();
    virtual void setClippingPlanes();
    virtual void showPhotonsCheckbox_toggled(bool toggle);
    virtual void showElectronsCheckbox_toggled(bool toggle);
    virtual void showPositronsCheckbox_toggled(bool toggle);
    virtual void startTransformation();
    virtual void endTransformation();
    virtual void toggleRegion(int i, int j);
    virtual void showAllRegions();
    virtual void hideAllRegions();
    virtual void enlargeFont();
    virtual void shrinkFont();
    virtual void setFontSize(int size);
    virtual void doseCheckbox_toggled();
    virtual void changeDoseTransparency(int t);
    virtual void changeTrackMin();
    virtual void changeTrackMaxP(int t);
    virtual void changeTrackMaxE(int t);
    virtual void changeTrackMaxPo(int t);
    virtual void updateTracks(vector<size_t> ntracks);

private:

    ClippingPlanesWidget *cplanes;
    ImageWindow *gview;
    SaveImage *save_image;

    QString filename;
    QString filename_tracks;
    QString userDoseFile;
    int nmed;
    QRgb *m_colors;
    QColor  backgroundColor,
            textColor,
            axisColor,
            photonColor,
            electronColor,
            positronColor;
    int zoomlevel_home;
    int zoomlevel;
    EGS_Float a_light;
    EGS_Float s_phi;
    EGS_Float c_phi;
    EGS_Float phi;
    EGS_Float s_theta;
    EGS_Float c_theta;
    EGS_Float theta;
    EGS_Float distance;
    EGS_Float size;
    EGS_Float doseTransparency;
    EGS_Vector axesmax;
    EGS_Vector center;
    EGS_Vector camera_home_v2;
    EGS_Vector camera_home_v1;
    EGS_Vector camera_home;
    EGS_Vector camera_v2;
    EGS_Vector camera_v1;
    EGS_Vector camera;
    EGS_Vector screen_v2;
    EGS_Vector screen_v1;
    EGS_Vector screen_xo;
    EGS_Vector p_light;
    EGS_Vector look_at_home;
    EGS_Vector look_at;
    EGS_BaseGeometry *g;
    bool showAxes;
    bool showAxesLabels;
    bool showTracks;
    bool showPhotonTracks;
    bool showElectronTracks;
    bool showPositronTracks;
    vector<bool> show_regions;
    bool    allowRegionSelection,
            energyScaling;
    vector<vector<EGS_Float>> scoreArrays;
    vector<string> geometryNames;
    EGS_BaseGeometry *origSimGeom;

protected slots:

    virtual void languageChange() {
        retranslateUi(this);
    }

};

#endif // GEOMETRYVIEWCONTROL_H
