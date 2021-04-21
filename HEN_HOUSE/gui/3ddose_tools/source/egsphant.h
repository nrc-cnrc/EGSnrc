/*
###############################################################################
#
#  3ddose_tools GUI for EGSnrc 3ddose file analysis
#  Copyright (C) 2020 Rowan Thomson and Martin Martinov
#
#  This file is part of EGSnrc.
#
#  EGSnrc is free software: you can redistribute it and/or modify it under
#  the terms of the GNU Affero General Public License as published by the Free
#  Software Foundation, either version 3 of the License, or (at your option)
#  any later version.
#
#  EGSnrc is distributed in the hope that it will be useful, but WITHOUT
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License
#  for more details.
#
#  To see the GNU Affero General Public License at:
# <http://www.gnu.org/licenses/>.
#
###############################################################################
#
#  Author:          Martin Martinov, 2020
#
#  Contributors:    Rowan Thomson, 2020
#
###############################################################################
*/
#ifndef EGSPHANT_H
#define EGSPHANT_H

#include <QtWidgets>
#include <iostream>
#include <math.h>

class EGSPhant : public QObject {
    Q_OBJECT

signals:
    void progressMade(double n); // Update the progress bar

public:
    EGSPhant();

    int nx, ny, nz; // these hold the number of voxels
    QVector <double> x, y, z; // these hold the boundaries of the above voxels
    QVector <QVector <QVector <char> > > m; // this holds all the media
    QVector <QVector <QVector <double> > > d; // this holds all the densities
    QVector <QString> media; // this holds all the possible media
    double maxDensity;

    void loadEGSPhantFile(QString path);
    void loadEGSPhantFilePlus(QString path);
    void loadbEGSPhantFile(QString path);
    void loadbEGSPhantFilePlus(QString path);

    char getMedia(double px, double py, double pz);
    double getDensity(double px, double py, double pz);
    int getIndex(QString axis, double p);
    QImage getEGSPhantPicDen(QString axis, double ai, double af,
                             double bi, double bf, double d, int res);
    QImage getEGSPhantPicMed(QString axis, double ai, double af,
                             double bi, double bf, double d, int res);

    // Image Processing
    void loadMaps();

    // Progress bar resolution
    const static int MAX_PROGRESS = 1000000000;
};

#endif
