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
#                   Reid Townson
#
###############################################################################
*/

#ifndef CLIPPINGPLANESWIDGET_H
#define CLIPPINGPLANESWIDGET_H

#include "ui_clippingplanes.h"

#include <qwidget.h>

class ClippingPlanesWidget : public QWidget, public Ui::ClippingPlanesWidget {
    Q_OBJECT

public:

    ClippingPlanesWidget(QWidget *parent = 0, const char *name = 0);
    ~ClippingPlanesWidget();

    virtual int numPlanes();
    virtual bool getPlane(int j, EGS_Vector &a, EGS_Float &d);
    virtual void setCell(int i, int j, EGS_Float val);
    virtual void setCell(int i, int j, Qt::CheckState checked);
    virtual void clearCell(int i, int j);
    virtual QTableWidgetItem *getItem(int i, int j);

signals:

    void clippingPlanesChanged();

protected slots:

    virtual void languageChange() {
        retranslateUi(this);
    }
    virtual void applyClipping();

};

#endif // CLIPPINGPLANESWIDGET_H
