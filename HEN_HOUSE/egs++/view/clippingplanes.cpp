/*
###############################################################################
#
#  EGSnrc egs++ geometry viewer clipping planes extensions
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
#  Contributors:    Manuel Stoeckl
#                   Ernesto Mainegra-Hing
#                   Cody Crewson
#                   Reid Townson
#
###############################################################################
*/

#include <QtGlobal>
#include "clippingplanes.h"

#include "egs_libconfig.h"

#include <qstring.h>
#include <qheaderview.h>

#ifdef VIEW_DEBUG
    extern void (* egsWarning)(const char *, ...);
#endif

ClippingPlanesWidget::ClippingPlanesWidget(QWidget *parent, const char *name)
    : QWidget(parent) {
    setObjectName(name);
    setupUi(this);

    planeTable->setColumnWidth(0,60);
    planeTable->setColumnWidth(1,60);
    planeTable->setColumnWidth(2,60);
    planeTable->setColumnWidth(3,60);
    planeTable->setColumnWidth(4,28);
}

ClippingPlanesWidget::~ClippingPlanesWidget() {
}

void ClippingPlanesWidget::applyClipping() {
    emit clippingPlanesChanged();
}


int ClippingPlanesWidget::numPlanes() {
    return planeTable->rowCount();
}

QTableWidgetItem *ClippingPlanesWidget::getItem(int i, int j) {
    QTableWidgetItem *item = planeTable->item(i,j);
    return item;
}


bool ClippingPlanesWidget::getPlane(int j, EGS_Vector &a, EGS_Float &d) {
    // check if all row items exist and are selected.
    QTableWidgetItem *itemAx = planeTable->item(j,0),
                      *itemAy = planeTable->item(j,1),
                       *itemAz = planeTable->item(j,2),
                        *itemD = planeTable->item(j,3),
                         *itemApplied = planeTable->item(j,4);

    // Make sure all parameters for a plane exist
    if (!itemAx || !itemAy || !itemAz  || !itemD || !itemApplied) {
        return false;
    }

    // See if the checkbox in the 4th column is checked
    // Only use the plane if it is checked
    if (itemApplied->checkState() == Qt::Unchecked) {
        return false;
    }

    // transfer values from table
    bool ok;
    double ax = itemAx->text().toDouble(&ok);
    if (!ok) {
        return false;
    }
    double ay = itemAy->text().toDouble(&ok);
    if (!ok) {
        return false;
    }
    double az = itemAz->text().toDouble(&ok);
    if (!ok) {
        return false;
    }
    double nd = itemD->text().toDouble(&ok);
    if (!ok) {
        return false;
    }

    // commit values only if all are valid
    a.x = ax;
    a.y = ay;
    a.z = az;
    d = nd;
    return true;
}

void ClippingPlanesWidget::setCell(int i, int j, EGS_Float val) {
    QTableWidgetItem *item = planeTable->item(i,j);

    if (!item) {
        item = new QTableWidgetItem();
        planeTable->setItem(i,j,item);
    }

    item->setText(QString::number(val));
}

void ClippingPlanesWidget::setCell(int i, int j, Qt::CheckState checked) {
    QTableWidgetItem *item = planeTable->item(i,j);

    if (!item) {
        item = new QTableWidgetItem();
        planeTable->setItem(i,j,item);
    }

    item->setCheckState(checked);
}

void ClippingPlanesWidget::clearCell(int i, int j) {
    QTableWidgetItem *item = planeTable->item(i,j);

    if (item) {
        delete item;
    }
}
