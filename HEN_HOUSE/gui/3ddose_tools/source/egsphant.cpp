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
#include "egsphant.h"

EGSPhant::EGSPhant() {
    nx = ny = nz = 0;
}

void EGSPhant::loadEGSPhantFile(QString path) {
    QFile file(path);

    // Increment size of the status bar
    double increment;

    // Open up the file specified at path
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream input(&file);
        QString line = input.readLine();

        // read in the number of media
        int num = line.trimmed().toInt();
        media.resize(num);

        // read the media into an array
        for (int i = 0; i < num; i++) {
            media[i] = input.readLine().trimmed();
        }

        // skim over the the ESTEP info
        line = input.readLine().trimmed();

        // read in the dimensions of the egsphant file and
        // store the size and resize the matrices holding the boundaries
        input.skipWhiteSpace();
        input >> nx;
        x.fill(0,nx+1);
        input.skipWhiteSpace();
        input >> ny;
        y.fill(0,ny+1);
        input.skipWhiteSpace();
        input >> nz;
        z.fill(0,nz+1);

        // resize the 3D matrix to hold all densities
        {
            QVector <char> mz(nz, 0);
            QVector <QVector <char> > my(ny, mz);
            QVector <QVector <QVector <char> > > mx(nx, my);
            m = mx;
            QVector <double> dz(nz, 0);
            QVector <QVector <double> > dy(ny, dz);
            QVector <QVector <QVector <double> > > dx(nx, dy);
            d = dx;
        }

        // read in all the boundaries of the phantom
        input.skipWhiteSpace();
        for (int i = 0; i <= nx; i++) {
            input >> x[i];
        }
        for (int i = 0; i <= ny; i++) {
            input >> y[i];
        }
        for (int i = 0; i <= nz; i++) {
            input >> z[i];
        }

        // Determine the increment this egsphant file gets
        increment = MAX_PROGRESS/double(nz-1);

        // Read in all the media
        for (int k = 0; k < nz; k++) {
            for (int j = 0; j < ny; j++)
                for (int i = 0; i < nx; i++) {
                    input.skipWhiteSpace();
                    input >> m[i][j][k];
                }
            emit progressMade(increment); // Update progress bar
        }

        file.close();
    }
}

void EGSPhant::loadEGSPhantFilePlus(QString path) {
    QFile file(path);

    // Increment size of the status bar
    double increment;

    // Open up the file specified at path
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream input(&file);
        QString line = input.readLine();

        // read in the number of media
        int num = line.trimmed().toInt();
        media.resize(num);

        // read the media into an array
        for (int i = 0; i < num; i++) {
            media[i] = input.readLine().trimmed();
        }

        // skim over the the ESTEP info
        line = input.readLine().trimmed();

        // read in the dimensions of the egsphant file
        input.skipWhiteSpace();
        input >> nx;
        x.fill(0,nx+1);
        input.skipWhiteSpace();
        input >> ny;
        y.fill(0,ny+1);
        input.skipWhiteSpace();
        input >> nz;
        z.fill(0,nz+1);

        // resize the 3D matrix to hold all densities
        {
            QVector <char> mz(nz, 0);
            QVector <QVector <char> > my(ny, mz);
            QVector <QVector <QVector <char> > > mx(nx, my);
            m = mx;
            QVector <double> dz(nz, 0);
            QVector <QVector <double> > dy(ny, dz);
            QVector <QVector <QVector <double> > > dx(nx, dy);
            d = dx;
        }

        // read in all the boundaries of the phantom
        input.skipWhiteSpace();
        for (int i = 0; i <= nx; i++) {
            input >> x[i];
        }
        for (int i = 0; i <= ny; i++) {
            input >> y[i];
        }
        for (int i = 0; i <= nz; i++) {
            input >> z[i];
        }

        // Determine the increment this egsphant file gets
        increment = MAX_PROGRESS/double(nz-1);

        // Read in all the media
        for (int k = 0; k < nz; k++) {
            for (int j = 0; j < ny; j++)
                for (int i = 0; i < nx; i++) {
                    input.skipWhiteSpace();
                    input >> m[i][j][k];
                }
            emit progressMade(increment/100.0*10.0); // Update progress bar
        }

        // Read in all the densities
        maxDensity = 0;
        for (int k = 0; k < nz; k++) {
            for (int j = 0; j < ny; j++)
                for (int i = 0; i < nx; i++) {
                    input.skipWhiteSpace();
                    input >> d[i][j][k];
                    if (d[i][j][k] > maxDensity) {
                        maxDensity = d[i][j][k];
                    }

                }
            emit progressMade(increment/100.0*90.0); // Update progress bar
        }

        file.close();
    }
}

void EGSPhant::loadbEGSPhantFile(QString path) {
    QFile file(path);

    // Increment size of the status bar
    double increment;

    // Open up the file specified at path
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream input(&file);
        input.setByteOrder(QDataStream::LittleEndian);

        // read in the number of media
        unsigned char num;
        input >> num;
        media.resize(num);

        // read the media into an array
        char *temp2;
        for (int i = 0; i < num; i++) {
            input >> temp2;
            media[i] = QString(temp2);
        }

        // skim over the the ESTEP info
        double temp;
        for (int i = 0; i < num; i++) {
            input >> temp;
        }

        // read in the dimensions of the egsphant file
        // store the size and resize the matrices holding the boundaries
        input >> nx;
        x.fill(0,nx+1);
        input >> ny;
        y.fill(0,ny+1);
        input >> nz;
        z.fill(0,nz+1);

        // resize the 3D matrix to hold all densities
        {
            QVector <char> mz(nz, 0);
            QVector <QVector <char> > my(ny, mz);
            QVector <QVector <QVector <char> > > mx(nx, my);
            m = mx;
            QVector <double> dz(nz, 0);
            QVector <QVector <double> > dy(ny, dz);
            QVector <QVector <QVector <double> > > dx(nx, dy);
            d = dx;
        }

        // read in all the boundaries of the phantom
        for (int i = 0; i <= nx; i++) {
            input >> x[i];
        }
        for (int i = 0; i <= ny; i++) {
            input >> y[i];
        }
        for (int i = 0; i <= nz; i++) {
            input >> z[i];
        }

        // Determine the increment this egsphant file gets
        increment = MAX_PROGRESS/double(nz-1);

        // Read in all the media
        for (int k = 0; k < nz; k++) {
            for (int j = 0; j < ny; j++)
                for (int i = 0; i < nx; i++) {
                    input >> num;
                    m[i][j][k] = num;
                }
            emit progressMade(increment); // Update progress bar
        }

        file.close();
    }
}

void EGSPhant::loadbEGSPhantFilePlus(QString path) {
    QFile file(path);

    // Increment size of the status bar
    double increment;

    // Open up the file specified at path
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream input(&file);
        input.setByteOrder(QDataStream::LittleEndian);

        // read in the number of media
        unsigned char num;
        input >> num;
        media.resize(num);

        // read the media into an array
        char *temp2;
        for (int i = 0; i < num; i++) {
            input >> temp2;
            media[i] = QString(temp2);
        }

        // skim over the the ESTEP info
        double temp;
        for (int i = 0; i < num; i++) {
            input >> temp;
        }

        // read in the dimensions of the egsphant file
        // store the size and resize the matrices holding the boundaries
        input >> nx;
        x.fill(0,nx+1);
        input >> ny;
        y.fill(0,ny+1);
        input >> nz;
        z.fill(0,nz+1);

        // resize the 3D matrix to hold all densities
        {
            QVector <char> mz(nz, 0);
            QVector <QVector <char> > my(ny, mz);
            QVector <QVector <QVector <char> > > mx(nx, my);
            m = mx;
            QVector <double> dz(nz, 0);
            QVector <QVector <double> > dy(ny, dz);
            QVector <QVector <QVector <double> > > dx(nx, dy);
            d = dx;
        }

        // read in all the boundaries of the phantom
        for (int i = 0; i <= nx; i++) {
            input >> x[i];
        }
        for (int i = 0; i <= ny; i++) {
            input >> y[i];
        }
        for (int i = 0; i <= nz; i++) {
            input >> z[i];
        }

        // Determine the increment this egsphant file gets
        increment = MAX_PROGRESS/double(nz-1);

        // Read in all the media
        for (int k = 0; k < nz; k++) {
            for (int j = 0; j < ny; j++)
                for (int i = 0; i < nx; i++) {
                    input >> num;
                    m[i][j][k] = num;
                }
            emit progressMade(increment/100.0*50.0); // Update progress bar
        }

        // Read in all the densities
        maxDensity = 0;
        for (int k = 0; k < nz; k++) {
            for (int j = 0; j < ny; j++)
                for (int i = 0; i < nx; i++) {
                    input >> d[i][j][k];
                    if (d[i][j][k] > maxDensity) {
                        maxDensity = d[i][j][k];
                    }
                }
            emit progressMade(increment/100.0*50.0); // Update progress bar
        }

        file.close();
    }
}

char EGSPhant::getMedia(double px, double py, double pz) {
    int ix, iy, iz;
    ix = iy = iz = -1;

    // Find the index of the boundary that is less than px, py and pz
    for (int i = 0; i < nx; i++)
        if (px <= x[i+1]) {
            ix = i;
            break;
        }
    if (px < x[0]) {
        ix = -1;
    }

    for (int i = 0; i < ny; i++)
        if (py <= y[i+1]) {
            iy = i;
            break;
        }
    if (py < y[0]) {
        iy = -1;
    }

    for (int i = 0; i < nz; i++)
        if (pz <= z[i+1]) {
            iz = i;
            break;
        }
    if (pz < z[0]) {
        iz = -1;
    }

    // This is to insure that no area outside the vectors is accessed
    if (ix < nx && ix >= 0 && iy < ny && iy >= 0 && iz < nz && iz >= 0) {
        return m[ix][iy][iz];
    }

    return 0; // We are not within our bounds
}


double EGSPhant::getDensity(double px, double py, double pz) {
    int ix, iy, iz;
    ix = iy = iz = -1;

    // Find the index of the boundary that is less than px, py and pz
    for (int i = 0; i < nx; i++)
        if (px <= x[i+1]) {
            ix = i;
            break;
        }
    if (px < x[0]) {
        ix = -1;
    }

    for (int i = 0; i < ny; i++)
        if (py <= y[i+1]) {
            iy = i;
            break;
        }
    if (py < y[0]) {
        iy = -1;
    }

    for (int i = 0; i < nz; i++)
        if (pz <= z[i+1]) {
            iz = i;
            break;
        }
    if (pz < z[0]) {
        iz = -1;
    }

    // This is to insure that no area outside the vectors is accessed
    if (ix < nx && ix >= 0 && iy < ny && iy >= 0 && iz < nz && iz >= 0) {
        return d[ix][iy][iz];
    }

    return 0; // We are not within our bounds
}

int EGSPhant::getIndex(QString axis, double p) {
    int index = -1;

    if (!axis.compare("x axis")) {
        for (int i = 0; i < nx; i++)
            if (p < x[i+1]) {
                index = i;
                break;
            }
        if (p < x[0]) {
            index = -1;
        }
    }
    else if (!axis.compare("y axis")) {
        for (int i = 0; i < ny; i++)
            if (p < y[i+1]) {
                index = i;
                break;
            }
        if (p < y[0]) {
            index = -1;
        }
    }
    else if (!axis.compare("z axis")) {
        for (int i = 0; i < nz; i++)
            if (p < z[i+1]) {
                index = i;
                break;
            }
        if (p < z[0]) {
            index = -1;
        }
    }

    return index; // -1 if we are out of bounds
}

QImage EGSPhant::getEGSPhantPicMed(QString axis, double ai, double af,
                                   double bi, double bf, double d, int res) {
    // Create a temporary image
    int width  = (af-ai)*res;
    int height = (bf-bi)*res;
    QImage image(height, width, QImage::Format_RGB32);
    double hInc, wInc, cInc;
    double h, w, c = 0;

    // Calculate the size (in cm) of pixels, and then the range for grayscaling
    wInc = 1/double(res);
    hInc = 1/double(res);
    cInc = 255.0/(media.size()-1);

    for (int i = height-1; i >= 0; i--)
        for (int j = 0; j < width; j++) {
            // determine the location of the current pixel in the phantom
            h = (double(bi)) + hInc * double(i);
            w = (double(ai)) + wInc * double(j);

            // get the media, which differs based on axis through which image is
            // sliced
            if (!axis.compare("x axis")) {
                c = getMedia(d, h, w) - 49;
            }
            else if (!axis.compare("y axis")) {
                c = getMedia(h, d, w) - 49;
            }
            else if (!axis.compare("z axis")) {
                c = getMedia(h, w, d) - 49;
            }

            // finally, paint the pixel
            image.setPixel(i, j, qRgb(int(cInc*c), int(cInc*c), int(cInc*c)));
        }

    return image; // return the image created
}

QImage EGSPhant::getEGSPhantPicDen(QString axis, double ai, double af,
                                   double bi, double bf, double d, int res) {
    // Create a temporary image
    int width  = (af-ai)*res;
    int height = (bf-bi)*res;
    QImage image(height, width, QImage::Format_RGB32);
    double hInc, wInc, cInc;
    double h, w, c = 0;

    // Calculate the size (in cm) of pixels, and then the range for grayscaling
    wInc = 1/double(res);
    hInc = 1/double(res);
    cInc = 255.0/maxDensity;

    for (int i = height-1; i >= 0; i--)
        for (int j = 0; j < width; j++) {
            // determine the location of the current pixel in the phantom
            h = (double(bi)) + hInc * double(i);
            w = (double(ai)) + wInc * double(j);

            // get the density, which differs based on axis through which image
            // os sliced
            if (!axis.compare("x axis")) {
                c = getDensity(d, h, w);
            }
            else if (!axis.compare("y axis")) {
                c = getDensity(h, d, w);
            }
            else if (!axis.compare("z axis")) {
                c = getDensity(h, w, d);
            }

            // finally, paint the pixel
            image.setPixel(i, j, qRgb(int(cInc*c), int(cInc*c), int(cInc*c)));
        }

    return image; // return the image created
}
