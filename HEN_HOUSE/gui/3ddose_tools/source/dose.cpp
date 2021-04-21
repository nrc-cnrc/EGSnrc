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
#include "dose.h"

Dose::Dose(const Dose &d)
    : QObject(0) {
    // Set the number of coordinates the same
    x = d.x;
    y = d.y;
    z = d.z;

    // Go through and fill the same boundaries
    for (int i = 0; i <= x; i++) {
        cx.append(d.cx[i]);
    }
    for (int i = 0; i <= y; i++) {
        cy.append(d.cy[i]);
    }
    for (int i = 0; i <= z; i++) {
        cz.append(d.cz[i]);
    }

    // Go through and fill the same doses and errors
    val.resize(x);
    err.resize(x);
    for (int i = 0; i < d.x; i++) {
        val[i].resize(y);
        err[i].resize(y);
        for (int j = 0; j < d.y; j++) {
            val[i][j].resize(z);
            err[i][j].resize(z);
            for (int k = 0; k < d.z; k++) {
                val[i][j][k] = d.val[i][j][k];
                err[i][j][k] = d.err[i][j][k];
            }
        }
    }

    // And set the filled flag to true
    filled = d.filled;
}

void Dose::copyDose(Dose *other) {
    // Set the number of coordinates the same
    x = other->x;
    y = other->y;
    z = other->z;

    // Go through and fill the same boundaries
    for (int i = 0; i <= x; i++) {
        cx.append(other->cx[i]);
    }
    for (int i = 0; i <= y; i++) {
        cy.append(other->cy[i]);
    }
    for (int i = 0; i <= z; i++) {
        cz.append(other->cz[i]);
    }

    // Go through and fill the same doses and errors
    val.resize(x);
    err.resize(x);
    for (int i = 0; i < other->x; i++) {
        val[i].resize(y);
        err[i].resize(y);
        for (int j = 0; j < other->y; j++) {
            val[i][j].resize(z);
            err[i][j].resize(z);
            for (int k = 0; k < other->z; k++) {
                val[i][j][k] = other->val[i][j][k];
                err[i][j][k] = other->err[i][j][k];
            }
        }
    }

    // And set the filled flag to true
    filled = other->filled;
}

Dose::Dose(QString *path, int n)
    : QObject(0) {
    // If we have no path, set filled to false, otherwise, read in the data
    if (path == 0) {
        filled = 0;
    }
    else if (path->endsWith(".b3ddose")) {
        readBIn(*path, n);
    }
    else {
        readIn(*path, n);
    }
}

Dose::~Dose() {
    cx.clear();
    cy.clear();
    cz.clear();
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            val[i][j].clear();
            err[i][j].clear();
        }
        val[i].clear();
        err[i].clear();
    }
    val.clear();
    err.clear();
}

double Dose::linInterpol(double ap, double a0, double a1, double b0, double b1,
                         double e0, double e1, double *val, double *err) {
    // This algorithm finds the equation to describe the line between the points
    // (a0, b0) and (a1, b1), then uses this equation to estimate what bp value
    // would be at the point ap, the equation starts like
    //
    // bp-b0   b1-b0
    // ----- = -----
    // ap-a0   a1-a0
    //
    // and rearranging for bp we have
    //
    //           (ap-a0)*(b1-b0)
    // bp = b0 + ---------------
    //               (a1-a0)
    //
    // and error follows from normal error propagation, where only b values
    // have an error

    e0 *= b0;
    e1 *= b1; // Convert from fractional error to absolute error

    *val = b0 + ((ap-a0)*(b1-b0)/(a1-a0));
    *err = sqrt(pow(e0,2) + pow((ap-a0)/(a1-a0),2)*(pow(e0,2) + pow(e1,2)));

    *err /= *val; // Convert from absolute error to fractional error
    return *val;
}

/* Removed due to glorious amounts of inefficiency
double Dose::triInterpol (double xp, double yp, double zp, double *val,
            double *err)
{
    // Initial Parameters, (x0, y0, z0) define the center of one diagonal voxel,
    // (x1, y1, z1) define the center of another, the point of interest is
    // either within one of these voxels, or a voxel adjacent to both of these
    // voxels
    // We want to find the dose at the point (xp, yp, zp), and we do so step by
    // step.  We can find (xp, yp, zp) by linearly interpolating (xp, yp, z0)
    // and (xp, yp, z1). We can find the points (xp, yp, zn) [where n=0 or n=1]
    // by interpolating from (xp, y0, zn) and (xp, y1, zn), and the same goes
    // for finding those values
    // Basically, we use the 8 corners of the cube to interpolate the 4 points
    // along the lines of the cube parallel to the x-axis that have an x value
    // xp to interpolate the points along two lines parallel to the y-axis we
    // can make with the 4 previous points at the value yp.  Then these two
    // points can be used to interpolate the value at zp, which gives us the
    // dose at (xp, yp, zp)

    double x0, y0, z0, x1, y1, z1;
    double Dx0y0z0, Dx1y0z0, Dx0y1z0, Dx1y1z0,
    Dx0y0z1, Dx1y0z1, Dx0y1z1, Dx1y1z1;
    double Ex0y0z0, Ex1y0z0, Ex0y1z0, Ex1y1z0,
    Ex0y0z1, Ex1y0z1, Ex0y1z1, Ex1y1z1;
    int xi, yi, zi;

    // Define X
    xi = getIndex ("X", xp);
    if (xp < (cx[xi] + cx[xi-1])/2.0)
    {
    xi--;
    x0 = (cx[xi] + cx[xi-1])/2.0;
    x1 = (cx[xi] + cx[xi+1])/2.0;
    }
    else
    {
    x0 = (cx[xi] + cx[xi+1])/2.0;
    x1 = (cx[xi+1] + cx[xi+2])/2.0;
    }

    // Define Y
    yi = getIndex ("Y", yp);
    if (yp < (cy[yi] + cy[yi-1])/2.0)
    {
    yi--;
    y0 = (cy[yi] + cy[yi-1])/2.0;
    y1 = (cy[yi] + cy[yi+1])/2.0;
    }
    else
    {
    y0 = (cy[yi] + cy[yi+1])/2.0;
    y1 = (cy[yi+1] + cy[yi+2])/2.0;
    }

    // Define Z
    zi = getIndex ("Z", zp);
    if (zp < (cz[zi] + cz[zi-1])/2.0)
    {
    zi--;
    z0 = (cz[zi] + cz[zi-1])/2.0;
    z1 = (cz[zi] + cz[zi+1])/2.0;
    }
    else
    {
    z0 = (cz[zi] + cz[zi+1])/2.0;
    z1 = (cz[zi+1] + cz[zi+2])/2.0;
    }

    // Get Dose of all 8 corners of the rectangular prism
    Dx0y0z0 = getDose(x0, y0, z0); Ex0y0z0 = getError(x0, y0, z0);
    Dx1y0z0 = getDose(x1, y0, z0); Ex1y0z0 = getError(x1, y0, z0);
    Dx0y1z0 = getDose(x0, y1, z0); Ex0y1z0 = getError(x0, y1, z0);
    Dx1y1z0 = getDose(x1, y1, z0); Ex1y1z0 = getError(x1, y1, z0);
    Dx0y0z1 = getDose(x0, y0, z1); Ex0y0z1 = getError(x0, y0, z1);
    Dx1y0z1 = getDose(x1, y0, z1); Ex1y0z1 = getError(x1, y0, z1);
    Dx0y1z1 = getDose(x0, y1, z1); Ex0y1z1 = getError(x0, y1, z1);
    Dx1y1z1 = getDose(x1, y1, z1); Ex1y1z1 = getError(x1, y1, z1);

    double Dxpy0z0, Dxpy1z0, Dxpy0z1, Dxpy1z1;
    double Expy0z0, Expy1z0, Expy0z1, Expy1z1;

    // Linearly interpolate along the X axis
    linInterpol (xp, x0, x1, Dx0y0z0, Dx1y0z0, Ex0y0z0, Ex1y0z0,
        &Dxpy0z0, &Expy0z0);
    linInterpol (xp, x0, x1, Dx0y1z0, Dx1y1z0, Ex0y1z0, Ex1y1z0,
        &Dxpy1z0, &Expy1z0);
    linInterpol (xp, x0, x1, Dx0y0z1, Dx1y0z1, Ex0y0z1, Ex1y0z1,
        &Dxpy0z1, &Expy0z1);
    linInterpol (xp, x0, x1, Dx0y1z1, Dx1y1z1, Ex0y1z1, Ex1y1z1,
        &Dxpy1z1, &Expy1z1);

    double Dxpypz0, Dxpypz1;
    double Expypz0, Expypz1;

    // Linearly interpolate along the Y axis
    linInterpol (yp, y0, y1, Dxpy0z0, Dxpy1z0, Expy0z0, Expy1z0,
        &Dxpypz0, &Expypz0);
    linInterpol (yp, y0, y1, Dxpy0z1, Dxpy1z1, Expy0z1, Expy1z1,
        &Dxpypz1, &Expypz1);

    double Dxpypzp;
    double Expypzp;

    // Linearly interpolate along the Z axis
    linInterpol (zp, z0, z1, Dxpypz0, Dxpypz1, Expypz0, Expypz1,
        &Dxpypzp, &Expypzp);

    *err = Expypzp;
    *val = Dxpypzp;

    return *val;
}
*/


// Precalculate volumes to make this faster
double Dose::triInterpol(double xp, double yp, double zp, double *val,
                         double *err) {
    // All the positions needed (in addition to the ones passed in)
    double x0, y0, z0, x1, y1, z1;

    // Indices
    int xi, yi, zi;

    // Define X
    xi = getIndex("X", xp);
    if (xp < (cx[xi] + cx[xi-1])/2.0) {
        x0 = (cx[xi] + cx[xi-1])/2.0;
        x1 = (cx[xi] + cx[xi+1])/2.0;
    }
    else {
        x0 = (cx[xi] + cx[xi+1])/2.0;
        x1 = (cx[xi+1] + cx[xi+2])/2.0;
    }

    // Define Y
    yi = getIndex("Y", yp);
    if (yp < (cy[yi] + cy[yi-1])/2.0) {
        y0 = (cy[yi] + cy[yi-1])/2.0;
        y1 = (cy[yi] + cy[yi+1])/2.0;
    }
    else {
        y0 = (cy[yi] + cy[yi+1])/2.0;
        y1 = (cy[yi+1] + cy[yi+2])/2.0;
    }

    // Define Z
    zi = getIndex("Z", zp);
    if (zp < (cz[zi] + cz[zi-1])/2.0) {
        z0 = (cz[zi] + cz[zi-1])/2.0;
        z1 = (cz[zi] + cz[zi+1])/2.0;
    }
    else {
        z0 = (cz[zi] + cz[zi+1])/2.0;
        z1 = (cz[zi+1] + cz[zi+2])/2.0;
    }

    // Precompute lengths
    double ix = xp-x0, fx = x1-xp, iy = yp-y0, fy = y1-yp, iz = zp-z0, fz = z1-zp;
    double vol = 0, weight = 0;

    // Add all 8 values together, weighted by the volume of the rectangular
    // prism formed by the point that was passed in and the one in the opposite
    // corner of the total rectangular prism volume
    *val = *err = 0;

    vol = fx*fy*fz;
    weight += vol*vol;
    *val += getDose(x0,y0,z0)*vol;
    *err += pow(getDose(x0,y0,z0)*getError(x0,y0,z0)*vol,2);

    vol = ix*fy*fz;
    weight += vol*vol;
    *val += getDose(x1,y0,z0)*vol;
    *err += pow(getDose(x1,y0,z0)*getError(x1,y0,z0)*vol,2);

    vol = fx*iy*fz;
    weight += vol*vol;
    *val += getDose(x0,y1,z0)*vol;
    *err += pow(getDose(x0,y1,z0)*getError(x0,y1,z0)*vol,2);

    vol = fx*fy*iz;
    weight += vol*vol;
    *val += getDose(x0,y0,z1)*vol;
    *err += pow(getDose(x0,y0,z1)*getError(x0,y0,z1)*vol,2);

    vol = fx*iy*iz;
    weight += vol*vol;
    *val += getDose(x0,y1,z1)*vol;
    *err += pow(getDose(x0,y1,z1)*getError(x0,y1,z1)*vol,2);

    vol = ix*fy*iz;
    weight += vol*vol;
    *val += getDose(x1,y0,z1)*vol;
    *err += pow(getDose(x1,y0,z1)*getError(x1,y0,z1)*vol,2);

    vol = ix*iy*fz;
    weight += vol*vol;
    *val += getDose(x1,y1,z0)*vol;
    *err += pow(getDose(x1,y1,z0)*getError(x1,y1,z0)*vol,2);

    vol = ix*iy*iz;
    weight += vol*vol;
    *val += getDose(x1,y1,z1)*vol;
    *err += pow(getDose(x1,y1,z1)*getError(x1,y1,z1)*vol,2);

    *val /= (x1-x0)*(y1-y0)*(z1-z0);
    *err = sqrt(*err/weight)/(*val);

    return *val;
}

void Dose::readIn(QString path, int n) {
    // Open the .3ddose file
    QFile *file;
    QTextStream *input;
    file = new QFile(path);

    // Determine the increment size of the status bar this 3ddose file gets
    double increment = Dose::MAX_PROGRESS/double (n);

    if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        input = new QTextStream(file);

        emit progressMade(increment*0.005); // Update progress bar

        // Read in the number of voxels
        *input >> x;
        *input >> y;
        *input >> z;

        // Resize x appropriately
        cx.resize(x+1);
        val.resize(x);
        err.resize(x);


        // Resize y appropriately
        cy.resize(y+1);
        for (int i = 0; i < x; i++) {
            val[i].resize(y);
            err[i].resize(y);
        }

        // Resize z appropriately
        cz.resize(z+1);
        for (int i = 0; i < x; i++)
            for (int j = 0; j < y; j++) {
                val[i][j].resize(z);
                err[i][j].resize(z);
            }

        emit progressMade(increment*0.01); // Update progress bar

        // Read in boundaries
        for (int i = 0; i <= x; i++) {
            *input >> cx[i];
        }
        for (int j = 0; j <= y; j++) {
            *input >> cy[j];
        }
        for (int k = 0; k <= z; k++) {
            *input >> cz[k];
        }

        emit progressMade(increment*0.01); // Update progress bar

        // Resize increments so that the rest of the section of the progress
        // bar is divided by two times the number of z values, so that each
        // time the loops iterate througha new z, the progress bar updates
        increment *= 0.975;
        increment = increment/(2*z);

        // Read in all the doses
        for (int k = 0; k < z; k++) {
            for (int j = 0; j < y; j++)
                for (int i = 0; i < x; i++) {
                    *input >> val[i][j][k];
                }

            emit progressMade(increment); // Update progress bar
        }

        // Read in all the errors
        for (int k = 0; k < z; k++) {
            for (int j = 0; j < y; j++)
                for (int i = 0; i < x; i++) {
                    *input >> err[i][j][k];
                }

            emit progressMade(increment); // Update progress bar
        }

        delete input;
    }
    delete file;
    filled = 1; // This Dose is now filled
}

void Dose::readBIn(QString path, int n) {
    // Open the .3ddose file
    QFile *file;
    QDataStream *input;
    file = new QFile(path);

    // Determine the increment size of the status bar this 3ddose file gets
    double increment = Dose::MAX_PROGRESS/double (n);

    if (file->open(QIODevice::ReadOnly)) {
        input = new QDataStream(file);
        input->setByteOrder(QDataStream::LittleEndian);

        emit progressMade(increment*0.005); // Update progress bar

        // Insure XYZ format
        unsigned char temp;
        *input >> temp;
        if (temp != 1) {
            delete input;
            delete file;
            return;
        }

        // Read in the number of voxels
        *input >> x;
        *input >> y;
        *input >> z;

        // Resize x appropriately
        cx.resize(x+1);
        val.resize(x);
        err.resize(x);

        // Resize y appropriately
        cy.resize(y+1);
        for (int i = 0; i < x; i++) {
            val[i].resize(y);
            err[i].resize(y);
        }

        // Resize z appropriately
        cz.resize(z+1);
        for (int i = 0; i < x; i++)
            for (int j = 0; j < y; j++) {
                val[i][j].resize(z);
                err[i][j].resize(z);
            }

        emit progressMade(increment*0.01); // Update progress bar

        // Read in boundaries
        for (int i = 0; i <= x; i++) {
            *input >> cx[i];
        }
        for (int j = 0; j <= y; j++) {
            *input >> cy[j];
        }
        for (int k = 0; k <= z; k++) {
            *input >> cz[k];
        }

        emit progressMade(increment*0.01); // Update progress bar

        // Resize increments so that the rest of the section of the progress
        // bar is divided by two times the number of z values, so that each
        // time the loops iterate througha new z, the progress bar updates
        increment *= 0.975;
        increment = increment/(2*z);

        // Read in all the doses
        for (int k = 0; k < z; k++) {
            for (int j = 0; j < y; j++)
                for (int i = 0; i < x; i++) {
                    *input >> val[i][j][k];
                }

            emit progressMade(increment); // Update progress bar
        }

        // Read in all the errors
        for (int k = 0; k < z; k++) {
            for (int j = 0; j < y; j++)
                for (int i = 0; i < x; i++) {
                    *input >> err[i][j][k];
                }

            emit progressMade(increment); // Update progress bar
        }

        delete input;
    }
    delete file;
    filled = 1; // This Dose is now filled
}

void Dose::readOut(QString path, int n) {
    // This function prints out a file in the standard 3ddose format
    QFile *file;
    QTextStream *input;
    file = new QFile(path);

    // Determine the increment size of the status bar this 3ddose file gets
    double increment = Dose::MAX_PROGRESS/double (n);

    if (file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        input = new QTextStream(file);

        emit progressMade(increment*0.005); // Update progress bar

        // Print the number of voxels
        *input << QString::number(x) << tr(" ");
        *input << QString::number(y) << tr(" ");
        *input << QString::number(z) << tr("\n");

        emit progressMade(increment*0.01); // Update progress bar

        // Print the boundaries
        for (int i = 0; i <= x; i++) {
            *input << QString::number(cx[i]) << tr(" ");
        }
        *input << tr("\n");

        for (int j = 0; j <= y; j++) {
            *input << QString::number(cy[j]) << tr(" ");
        }
        *input << tr("\n");

        for (int k = 0; k <= z; k++) {
            *input << QString::number(cz[k]) << tr(" ");
        }
        *input << tr("\n");

        emit progressMade(increment*0.01); // Update progress bar

        // Resize increments so that the rest of the section of the progress
        // bar is divided by two times the number of z values, so that each
        // time the loops iterate througha new z, the progress bar updates
        increment *= 0.975;
        increment = increment/(2*z);

        // Read out doses
        for (int k = 0; k < z; k++) {
            for (int j = 0; j < y; j++)
                for (int i = 0; i < x; i++) {
                    *input << QString::number(val[i][j][k]) << tr(" ");
                }
            emit progressMade(increment); // Update progress bar
        }
        *input << tr("\n");

        // Read out errors
        for (int k = 0; k < z; k++) {
            for (int j = 0; j < y; j++)
                for (int i = 0; i < x; i++) {
                    *input << QString::number(err[i][j][k]) << tr(" ");
                }
            emit progressMade(increment); // Update progress bar
        }
        *input << tr("\n\n");

        delete input;
    }
    delete file;
}

void Dose::readBOut(QString path, int n) {
    // This function prints out a file in the standard 3ddose format
    QFile *file;
    QDataStream *input;
    file = new QFile(path);

    // Determine the increment size of the status bar this 3ddose file gets
    double increment = Dose::MAX_PROGRESS/double (n);

    if (file->open(QIODevice::WriteOnly)) {
        input = new QDataStream(file);
        input->setByteOrder(QDataStream::LittleEndian);

        emit progressMade(increment*0.005); // Update progress bar

        *input << (unsigned char)(1);
        // Print the number of voxels
        *input << x << y << z;

        emit progressMade(increment*0.01); // Update progress bar

        // Print the boundaries
        for (int i = 0; i <= x; i++) {
            *input << cx[i];
        }

        for (int j = 0; j <= y; j++) {
            *input << cy[j];
        }

        for (int k = 0; k <= z; k++) {
            *input << cz[k];
        }

        emit progressMade(increment*0.01); // Update progress bar

        // Resize increments so that the rest of the section of the progress
        // bar is divided by two times the number of z values, so that each
        // time the loops iterate througha new z, the progress bar updates
        increment *= 0.975;
        increment = increment/(2*z);

        // Read out doses
        for (int k = 0; k < z; k++) {
            for (int j = 0; j < y; j++)
                for (int i = 0; i < x; i++) {
                    *input << val[i][j][k];
                }
            emit progressMade(increment); // Update progress bar
        }

        // Read out errors
        for (int k = 0; k < z; k++) {
            for (int j = 0; j < y; j++)
                for (int i = 0; i < x; i++) {
                    *input << err[i][j][k];
                }
            emit progressMade(increment); // Update progress bar
        }

        delete input;
    }
    delete file;
}

int Dose::translate(double dx, double dy, double dz) {
    // Change all the values within cx, cy and cz appropriately
    for (int i = 0; i <= x; i++) {
        cx[i] += dx;
    }
    for (int i = 0; i <= y; i++) {
        cy[i] += dy;
    }
    for (int i = 0; i <= z; i++) {
        cz[i] += dz;
    }

    return 1;
}

int Dose::strip() {
    if (x <= 2 || y <= 2 || z <= 2) { // Don't strip if nothing will be left
        return 0;
    }

    // Remove the first and last positions in the coordinates matrics
    cx.remove(x);
    cy.remove(y);
    cz.remove(z);
    cx.remove(0);
    cy.remove(0);
    cz.remove(0);

    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            // Remove the first and last z value of each (x,y) line
            val[i][j].remove(z-1);
            val[i][j].remove(0);
            err[i][j].remove(z-1);
            err[i][j].remove(0);
        }
        // Remove the first and last y value of each x line
        val[i].remove(y-1);
        val[i].remove(0);
        err[i].remove(y-1);
        err[i].remove(0);
    }
    // Remove the first and last x values
    val.remove(x-1);
    val.remove(0);
    err.remove(x-1);
    err.remove(0);

    // Resize the variables that keep track of size
    x -= 2;
    y -= 2;
    z -= 2;

    return 1; // Success
}

void Dose::subtractDose(Dose *other) {
    QVector <double> temp; // This holds the original value of this Dose for the
    // error formula
    int n = 0;
    temp.resize(z*y*x);

    for (int k = 0; k < z; k++)
        for (int j = 0; j < y; j++)
            for (int i = 0; i < x; i++) {
                // Subtract appropriately after saving temp
                temp[n++] = val[i][j][k];
                val[i][j][k] = val[i][j][k] - other->val[i][j][k];
            }
    n = 0;

    for (int k = 0; k < z; k++)
        for (int j = 0; j < y; j++)
            for (int i = 0; i < x; i++) {
                // for the formula v = v1 - v2
                // sigma = sqrt((e1*v1)^2 + (e2*v2)^2) [where e is fractional
                // error]
                err[i][j][k] = sqrt(pow(err[i][j][k] * temp[n++],2) + pow(other->err[i][j][k] * other->val[i][j][k], 2));
                // so then e = sigma/v
                err[i][j][k] = err[i][j][k] / val[i][j][k];
            }
}

void Dose::subtractDoseWithError(Dose *other) {
    QVector <double> temp; // This holds the original value of this Dose for the
    // error formula
    int n = 0;
    temp.resize(z*y*x);

    for (int k = 0; k < z; k++)
        for (int j = 0; j < y; j++)
            for (int i = 0; i < x; i++) {
                // Subtract appropriately after saving temp
                temp[n++] = val[i][j][k];
                val[i][j][k] = val[i][j][k] - other->val[i][j][k];
            }
    n = 0;

    for (int k = 0; k < z; k++)
        for (int j = 0; j < y; j++)
            for (int i = 0; i < x; i++) {
                // for the formula v = v1 - v2
                // sigma = sqrt((e1*v1)^2 + (e2*v2)^2) [where e is fractional
                // error]
                err[i][j][k] = sqrt(pow(err[i][j][k] * temp[n++],2) + pow(other->err[i][j][k] * other->val[i][j][k], 2));
                // so then e = sigma/v
                val[i][j][k] = val[i][j][k] / err[i][j][k];
            }
}

void Dose::addDose(Dose *other) {
    QVector <double> temp; // This holds the original value of this Dose for the
    // error formula
    int n = 0;
    temp.resize(z*y*x);

    for (int k = 0; k < z; k++)
        for (int j = 0; j < y; j++)
            for (int i = 0; i < x; i++) {
                // Subtract appropriately after saving temp
                temp[n++] = val[i][j][k];
                val[i][j][k] = val[i][j][k] + other->val[i][j][k];
            }
    n = 0;

    for (int k = 0; k < z; k++)
        for (int j = 0; j < y; j++)
            for (int i = 0; i < x; i++) {
                // for the formula v = v1 - v2
                // sigma = sqrt((e1*v1)^2 + (e2*v2)^2) [where e is fractional
                // error]
                err[i][j][k] = sqrt(pow(err[i][j][k] * temp[n++],2) +
                                    pow(other->err[i][j][k] *
                                        other->val[i][j][k], 2));
                // so then e = sigma/v
                err[i][j][k] = err[i][j][k] / val[i][j][k];
            }
}

int Dose::compareDimensions(Dose *other) {
    // If the size of the coordinate arrays are off, they are not the same
    if (x != other->x || y != other->y || z != other->z) {
        return 0;
    }

    // Then return 0 if any of the indices of the coordinate arrays are off
    for (int i = 0; i <= x; i++)
        if (cx[i] != other->cx[i]) {
            return 0;
        }
    for (int i = 0; i <= y; i++)
        if (cy[i] != other->cy[i]) {
            return 0;
        }
    for (int i = 0; i <= z; i++)
        if (cz[i] != other->cz[i]) {
            return 0;
        }

    // Otherwise, they match
    return 1;
}

int Dose::divideDose(Dose *other) {
    long int count = 0;
    for (int k = 0; k < z; k++)
        for (int j = 0; j < y; j++)
            for (int i = 0; i < x; i++) // Divide appropriatly
                if (other->val[i][j][k] != 0) {
                    val[i][j][k] = val[i][j][k]/other->val[i][j][k];
                }
                else {
                    val[i][j][k] = 0.0;
                    count++;
                }

    for (int k = 0; k < z; k++)
        for (int j = 0; j < y; j++)
            for (int i = 0; i < x; i++) // Recalculate error
                if (other->val[i][j][k] != 0) {
                    err[i][j][k] = sqrt(pow(err[i][j][k], 2) + pow(other->err[i][j][k], 2));
                }
                else {
                    err[i][j][k] = 1.0;
                }
    // If v = v1/v2 then
    //    e = sqrt(e1^2 + e2^2) [where e is fractional error]
    return count;
}

void Dose::localDose(Dose *other) {
    QVector <double> temp; // This holds the original value of this Dose for the
    // error formula
    int n = 0;
    temp.resize(z*y*x);

    for (int k = 0; k < z; k++)
        for (int j = 0; j < y; j++)
            for (int i = 0; i < x; i++) {
                // save temp
                temp[n++] = val[i][j][k];
                // subtract distributions
                val[i][j][k] = val[i][j][k] - other->val[i][j][k];
                // if denominator is nonzero, divide and multiply by 100
                // else set to zero
                if (other->val[i][j][k] != 0) {
                    val[i][j][k] = 100*val[i][j][k]/other->val[i][j][k];
                }
                else {
                    val[i][j][k] = 0.0;
                }
            }
    n = 0;

    for (int k = 0; k < z; k++)
        for (int j = 0; j < y; j++)
            for (int i = 0; i < x; i++) {
                // for the formula v = (v1 - v2)/v2
                // sigma = sqrt((e1*v1)^2 + (e2*v2)^2) [where e is fractional
                // error]
                err[i][j][k] = sqrt(pow(err[i][j][k] * temp[n++],2) + pow(other->err[i][j][k] * other->val[i][j][k], 2));
                // so then e = sigma/v
                err[i][j][k] = err[i][j][k] / val[i][j][k];
            }
}

int Dose::getIndex(QString axis, double val) {
    // This algorithm checks to see if val is within the outer bounds of axis'
    // coord array, then checks from c[0] to c[x] until it finds a c[n] smaller
    // than val and returns it's index

    int index = -1;
    if (!axis.compare("X")) {
        if (val <= cx[0] || val >= cx[x]) {
            return index;
        }

        for (int i = 0; i <= x; i++)
            if (cx[i] <= val) {
                index = i;
            }
    }
    else if (!axis.compare("Y")) {
        if (val <= cy[0] || val >= cy[y]) {
            return index;
        }

        for (int i = 0; i <= y; i++)
            if (cy[i] <= val) {
                index = i;
            }
    }
    else if (!axis.compare("Z")) {
        if (val <= cz[0] || val >= cz[z]) {
            return index;
        }

        for (int i = 0; i <= z; i++)
            if (cz[i] <= val) {
                index = i;
            }
    }
    return index; // Will return -1 on failure to find
}

double Dose::getDose(int ix, int iy, int iz) {
    if (iz <= -1 || iy <= -1 || ix <= -1 ||
            iz >= z  || iy >= y  || ix >= x) {
        return -1;    // If outside of bounds, return -1
    }

    return val[ix][iy][iz];
}

double Dose::getError(int ix, int iy, int iz) {
    if (iz <= -1 || iy <= -1 || ix <= -1 ||
            iz >= z  || iy >= y  || ix >= x) {
        return -1;    // If outside of bounds, return -1
    }

    return err[ix][iy][iz];
}

double Dose::getDose(double px, double py, double pz) {
    // Convert real numbers to indices and return dose at index
    int ix = getIndex("X", px);
    int iy = getIndex("Y", py);
    int iz = getIndex("Z", pz);
    if (iz == -1 || iy == -1 || ix == -1) {
        return -1;    // If outside of bounds, return -1
    }

    return val[ix][iy][iz];
}

double Dose::getError(double px, double py, double pz) {
    // Convert real numbers to indices and return error at index
    int ix = getIndex("X", px);
    int iy = getIndex("Y", py);
    int iz = getIndex("Z", pz);
    if (iz == -1 || iy == -1 || ix == -1) {
        return -1;    // If outside of bounds, return -1
    }

    return err[ix][iy][iz];
}

double Dose::getMax() {
    double max = val[0][0][0];

    // Iterate through all dose to get largest and smallest dose
    for (int k = 0; k < z; k++)
        for (int j = 0; j < y; j++)
            for (int i = 0; i < x; i++)
                if (val[i][j][k] > max) {
                    max = val[i][j][k];
                }

    return max;
}

double Dose::getMinMaxAvg(double *min, double *max, double *avg) {
    *min = *max = *avg = 0;
    int flag = 0;

    // Iterate through all dose to get largest and smallest dose
    for (int k = 0; k < z; k++)
        for (int j = 0; j < y; j++)
            for (int i = 0; i < x; i++)
                if (val[i][j][k] == val[i][j][k]) { // Will return false if NaN
                    if (!flag++) {
                        *min = val[i][j][k];
                        *max = val[i][j][k];
                    }
                    else if (val[i][j][k] < *min) {
                        *min = val[i][j][k];
                    }
                    else if (val[i][j][k] > *max) {
                        *max = val[i][j][k];
                    }
                    *avg += val[i][j][k];
                }

    *avg /= flag;
    return *avg;
}

double Dose::getMinMaxAvg(double *min, double *max, double *avg,
                          EGSPhant *egsphant, QVector <char> *med) {
    *min = *max = *avg = 0;
    int flag = 0;

    // Iterate through all dose to get largest and smallest dose
    for (int k = 0; k < val[0][0].size(); k++)
        for (int j = 0; j < val[0].size(); j++)
            for (int i = 0; i < val.size(); i++)
                for (int m = 0; m < med->size(); m++) //media agreement
                    if (val[i][j][k] == val[i][j][k]) // Will return false if NaN
                        if (egsphant->getMedia((cx[i+1]+cx[i])/2.0,
                                               (cy[j+1]+cy[j])/2.0,
                                               (cz[k+1]+cz[k])/2.0) ==
                                (*med)[m]) {

                            if (!flag++) {
                                *min = val[i][j][k];
                                *max = val[i][j][k];
                            }
                            else if (val[i][j][k] < *min) {
                                *min = val[i][j][k];
                            }
                            else if (val[i][j][k] > *max) {
                                *max = val[i][j][k];
                            }
                            *avg += val[i][j][k];
                        }

    *avg /= flag;
    return *avg;
}

double Dose::getMinMaxAvg(double *min, double *max, double *avg, double xi,
                          double xf, double yi, double yf, double zi, double zf) {
    *min = *max = *avg = 0;
    int flag = 0;

    // Iterate through all dose to get largest and smallest dose
    for (int k = 0; k < val[0][0].size(); k++)
        for (int j = 0; j < val[0].size(); j++)
            for (int i = 0; i < val.size(); i++)
                if (val[i][j][k] == val[i][j][k]) // Will return false if NaN
                    if ((cx[i+1]+cx[i])/2.0 > xi && (cx[i+1]+cx[i])/2.0 < xf &&
                            (cy[j+1]+cy[j])/2.0 > yi && (cy[j+1]+cy[j])/2.0 < yf &&
                            (cz[k+1]+cz[k])/2.0 > zi && (cz[k+1]+cz[k])/2.0 < zf) {
                        if (!flag++) {
                            *min = val[i][j][k];
                            *max = val[i][j][k];
                        }
                        else if (val[i][j][k] < *min) {
                            *min = val[i][j][k];
                        }
                        else if (val[i][j][k] > *max) {
                            *max = val[i][j][k];
                        }
                        *avg += val[i][j][k];
                    }

    *avg /= flag;
    return *avg;
}

void Dose::square() {
    for (int k = 0; k < z; k++)
        for (int j = 0; j < y; j++)
            for (int i = 0; i < x; i++) {
                err[i][j][k] *= 2*val[i][j][k]; // adjust error
                val[i][j][k] *= val[i][j][k]; // square each value
            }
}

void Dose::root() {
    for (int k = 0; k < z; k++)
        for (int j = 0; j < y; j++)
            for (int i = 0; i < x; i++) {
                val[i][j][k] = sqrt(val[i][j][k]); // root each value
                err[i][j][k] /= 2*val[i][j][k]; // adjust error
            }
}

void Dose::scaleError() {
    for (int k = 0; k < z; k++)
        for (int j = 0; j < y; j++)
            for (int i = 0; i < x; i++) {
                val[i][j][k] /= err[i][j][k]; // divide each value by error
                err[i][j][k] = 1; // adjust error (to be 1)
            }
}

int Dose::scale(double factor) {
    if (factor <= 0) { // scale by positive non-zero number only
        return 0;
    }

    for (int k = 0; k < z; k++)
        for (int j = 0; j < y; j++)
            for (int i = 0; i < x; i++) {
                val[i][j][k] *= factor;    // multiply each value by factor
            }

    // Since error is fractional, it does not change

    return 1;
}

int Dose::scaleAtPoint(double px, double py, double pz, double goal, char doTri) {
    double value, error;
    if (doTri) {
        triInterpol(px, py, pz, &value, &error);
    }
    else {
        value = getDose(px, py, pz);
        error = getError(px, py, pz);
    }
    // After finding the dose at (px, py, pz), a factor is calculated so that the voxel
    // at the position has goal dose, and each other voxel is scaled appropriately

    double factor = goal/value;
    for (int k = 0; k < z; k++)
        for (int j = 0; j < y; j++)
            for (int i = 0; i < x; i++) {
                val[i][j][k] *= factor;
            }

    // Since error is fractional, it does not change

    return 1;
}

int Dose::scaleAverage() {
    double factor = 0, error = 0, volume = 0;

    for (int k = 0; k < z; k++)
        for (int j = 0; j < y; j++)
            for (int i = 0; i < x; i++) {
                // Find the average by adding the dose of each voxel times its
                // volume (same for error using root squared) to find the total
                // dose in the volume, same for error but in quadrature
                volume = (cx[i+1]-cx[i])*(cy[j+1]-cy[j])*(cz[k+1]-cz[k]);
                factor += val[i][j][k]*volume;
                error = sqrt(pow(err[i][j][k]*val[i][j][k]*volume, 2) +
                             pow(error, 2));
            }

    // Divide the dose by the volume, to find the average dose per volume
    factor = factor/((cx[x]-cx[0])*(cy[y]-cy[0])*(cz[z]-cz[0]));
    // Divide the error by the volume, to find the average error
    error  = error/((cx[x]-cx[0])*(cy[y]-cy[0])*(cz[z]-cz[0]));


    for (int k = 0; k < z; k++)
        for (int j = 0; j < y; j++)
            for (int i = 0; i < x; i++) {
                // Then divide by the average dose per volume times the volume
                // of the voxel
                volume = (cx[i+1]-cx[i])*(cy[j+1]-cy[j])*(cz[k+1]-cz[k]);
                val[i][j][k] /= (factor*volume);
                // Then propagate error
                err[i][j][k] = sqrt(pow(err[i][j][k], 2) +
                                    pow(error*volume, 2));
            }

    return 1;
}

int Dose::scaleArea(double ix, double iy, double iz, double fx, double fy,
                    double fz) {
    if (getDose(ix, iy, iz) < 0 || getDose(fx, fy, fz) < 0) { // make sure all
        return 0;    // points exist
    }

    double factor = 0, error = 0;
    // Get indeces
    int xmax = getIndex("X", fx);
    int ymax = getIndex("Y", fy);
    int zmax = getIndex("Z", fz);

    // Find the largest dose within the defined volume
    for (int k = getIndex("Z", iz); k <= zmax; k++)
        for (int j = getIndex("Y", iy); j <= ymax; j++)
            for (int i = getIndex("X", ix); i <= xmax; i++)
                if (val[i][j][k] > factor) {
                    factor = val[i][j][k];
                    error = err[i][j][k];
                }

    // And factor all values by it
    for (int k = 0; k < z; k++)
        for (int j = 0; j < y; j++)
            for (int i = 0; i < x; i++) {
                val[i][j][k] /= factor;
                err[i][j][k] = sqrt(pow(err[i][j][k], 2) + pow(error, 2));
            }

    return 1;
}

QString Dose::plot(QString axis, double a, double b, double start, double stop) {
    // Build a QString that holds the csv data
    QString output = "";
    QString delimiter = ",";
    int initial, final, one, two;
    initial = final = one = two = 0;

    // Set up different values depending on which axis we are using
    if (!axis.compare("X")) {
        initial = getIndex(axis, start);
        if (initial == -1) {
            initial = 0;
        }
        final = getIndex(axis, stop);
        if (final == -1) {
            final = x-1;
        }
        one = getIndex("Y", a);
        two = getIndex("Z", b);
        output += "x (cm)" + delimiter + "dose" + delimiter + "error\n";
        for (int i = initial; i <= final; i++) {
            output += QString::number((cx[i]+cx[i+1])/2.0, 'E', 4);
            output += delimiter;
            output += QString::number(val[i][one][two], 'E', 4);
            output += delimiter;
            output += QString::number(err[i][one][two]*val[i][one][two],
                                      'E', 4);
            output += "\n";
        }
    }
    else if (!axis.compare("Y")) {
        initial = getIndex(axis, start);
        if (initial == -1) {
            initial = 0;
        }
        final = getIndex(axis, stop);
        if (final == -1) {
            final = y-1;
        }
        one = getIndex("X", a);
        two = getIndex("Z", b);
        output += "y (cm)" + delimiter + "dose" + delimiter + "error\n";
        for (int i = initial+1; i <= final; i++) {
            output += QString::number((cy[i]+cy[i+1])/2.0, 'E', 4);
            output += delimiter;
            output += QString::number(val[one][i][two], 'E', 4);
            output += delimiter;
            output += QString::number(err[one][i][two]*val[one][i][two],
                                      'E', 4);
            output += "\n";
        }
    }
    else if (!axis.compare("Z")) {
        initial = getIndex(axis, start);
        if (initial == -1) {
            initial = 0;
        }
        final = getIndex(axis, stop);
        if (final == -1) {
            final = z-1;
        }
        one = getIndex("X", a);
        two = getIndex("Y", b);
        output += "z (cm)" + delimiter + "dose" + delimiter + "error\n";
        for (int i = initial; i <= final; i++) {
            output += QString::number((cz[i]+cz[i+1])/2.0, 'E', 4);
            output += delimiter;
            output += QString::number(val[one][two][i], 'E', 4);
            output += delimiter;
            output += QString::number(err[one][two][i]*val[one][two][i],
                                      'E', 4);
            output += "\n";
        }
    }

    return output;
}

QString Dose::plot(double xi, double xf, double yi, double yf, double zi, double zf, EGSPhant *egsphant, QVector <char> *med,
                   double *min, double *eMin, double *max, double *eMax, double *avg, double *eAvg, double *meanErr, double *maxErr,
                   double *totVol, double *nVox, QVector <double> *Dx, QVector <double> *Vx) {
    // Build a QString that holds the csv data
    QString output = "";
    QString delimiter = ",";
    int n = 0;
    double total = 0;

    // Iterate through every voxel to find the number of DVHpoints needed
    // for either the whole thing, a region or several media
    if (egsphant != NULL) { // If we do have an egsphant file
        for (int k = 0; k < val[0][0].size(); k++)
            for (int j = 0; j < val[0].size(); j++)
                for (int i = 0; i < val.size(); i++)
                    for (int m = 0; m < med->size(); m++) //media agreement
                        if (egsphant->getMedia((cx[i+1]+cx[i])/2.0, (cy[j+1]+cy[j])/2.0, (cz[k+1]+cz[k])/2.0) == (*med)[m])
                            if (!n++) { // Initialize min and max with dose
                                // within the volume
                                *max = *min = val[i][j][k];
                                *maxErr = err[i][j][k];
                            }
    }
    else if ((xi != xf) && (yi != yf) && (zi != zf)) { // If region has nonzero volume
        for (int k = 0; k < val[0][0].size(); k++)
            for (int j = 0; j < val[0].size(); j++)
                for (int i = 0; i < val.size(); i++)
                    if ((cx[i+1]+cx[i])/2.0 > xi && (cx[i+1]+cx[i])/2.0 < xf && (cy[j+1]+cy[j])/2.0 > yi && (cy[j+1]+cy[j])/2.0 < yf &&
                            (cz[k+1]+cz[k])/2.0 > zi && (cz[k+1]+cz[k])/2.0 < zf)
                        if (!n++) { // Initialize min and max with dose
                            // within the volume
                            *max = *min = val[i][j][k];
                            *maxErr = err[i][j][k];
                        }
    }
    else { // If we are analyzing entire 3ddose file
        n = val.size()*val[0].size()*val[0][0].size();
        *max = *min = val[0][0][0];
        *maxErr = err[0][0][0];
    }

    // Allocate enough data to hold every dose
    DVHpoint *data = new DVHpoint [n];
    n = 0;
    double v = 0;

    // Aditional values defaults
    *eMin = *eMax = *eAvg = *meanErr = *avg = *totVol = 0;

    // Iterate through every voxel for either the whole thing, a region or
    // several media
    if (egsphant != NULL) { // If we do point to an egsphant file
        for (int k = 0; k < val[0][0].size(); k++)
            for (int j = 0; j < val[0].size(); j++)
                for (int i = 0; i < val.size(); i++)
                    for (int m = 0; m < med->size(); m++) // Media agreement
                        if (egsphant->getMedia((cx[i+1]+cx[i])/2.0,
                                               (cy[j+1]+cy[j])/2.0,
                                               (cz[k+1]+cz[k])/2.0) ==
                                (*med)[m]) {
                            // Compute averages first
                            v = (cx[i+1]-cx[i])*(cy[j+1]-cy[j])*(cz[k+1]-cz[k]);
                            *totVol += v;
                            *avg += val[i][j][k]*v;
                            *eAvg += pow(err[i][j][k],2)*v;
                            *meanErr += err[i][j][k]*v;

                            // Check mins & maxs
                            if (*max < val[i][j][k]) {
                                *max = val[i][j][k];
                                *eMax = err[i][j][k];
                            }
                            if (*min > val[i][j][k]) {
                                *min = val[i][j][k];
                                *eMin = err[i][j][k];
                            }
                            if (*maxErr < err[i][j][k]) {
                                *maxErr = err[i][j][k];
                            }

                            // Add voxel dose to data[n]
                            data[n].dose = val[i][j][k];
                            // Add voxel volume to data[n]
                            data[n].vol = v;
                            n++;
                        }
    }
    else if ((xi != xf) && (yi != yf) && (zi != zf)) { // If region has nonzero
        for (int k = 0; k < val[0][0].size(); k++)
            for (int j = 0; j < val[0].size(); j++)
                for (int i = 0; i < val.size(); i++) {
                    // Check if boundaries agree
                    if ((cx[i+1]+cx[i])/2.0 > xi && (cx[i+1]+cx[i])/2.0 < xf &&
                            (cy[j+1]+cy[j])/2.0 > yi && (cy[j+1]+cy[j])/2.0 < yf &&
                            (cz[k+1]+cz[k])/2.0 > zi && (cz[k+1]+cz[k])/2.0 < zf) {
                        // Compute averages first
                        v = (cx[i+1]-cx[i])*(cy[j+1]-cy[j])*(cz[k+1]-cz[k]);
                        *totVol += v;
                        *avg += val[i][j][k]*v;
                        *eAvg += pow(err[i][j][k],2)*v;
                        *meanErr += err[i][j][k]*v;

                        // Check mins & maxs
                        if (*max < val[i][j][k]) {
                            *max = val[i][j][k];
                            *eMax = err[i][j][k];
                        }
                        if (*min > val[i][j][k]) {
                            *min = val[i][j][k];
                            *eMin = err[i][j][k];
                        }
                        if (*maxErr < err[i][j][k]) {
                            *maxErr = err[i][j][k];
                        }

                        // Add voxel dose to data[n]
                        data[n].dose = val[i][j][k];
                        // Add voxel volume to data[n]
                        data[n].vol = v;
                        n++;
                    }
                }
    }
    else { // If we are analyzing entire 3ddose file
        for (int k = 0; k < val[0][0].size(); k++)
            for (int j = 0; j < val[0].size(); j++)
                for (int i = 0; i < val.size(); i++) {
                    // Compute averages first
                    v = (cx[i+1]-cx[i])*(cy[j+1]-cy[j])*(cz[k+1]-cz[k]);
                    *totVol += v;
                    *avg += val[i][j][k]*v;
                    *eAvg += pow(err[i][j][k],2)*v;
                    *meanErr += err[i][j][k]*v;

                    // Check mins & maxs
                    if (*max < val[i][j][k]) {
                        *max = val[i][j][k];
                        *eMax = err[i][j][k];
                    }
                    if (*min > val[i][j][k]) {
                        *min = val[i][j][k];
                        *eMin = err[i][j][k];
                    }
                    if (*maxErr < err[i][j][k]) {
                        *maxErr = err[i][j][k];
                    }

                    // Get an index
                    n = 1 + i + (val.size())*j + (val.size())*(val[0].size())*k;
                    // Add voxel dose to data[n-1]
                    data[n-1].dose = val[i][j][k];
                    // Add voxel volume to data[n-1]
                    data[n-1].vol = v;
                }

    }

    *avg /= (*totVol);
    *eAvg = sqrt(*eAvg/(*totVol));
    *meanErr /= (*totVol);
    *nVox = n;

    // This uses a mergesort algorithm to sort all the doses (and their appro-
    // priate volumes) from largest to smallest
    merge(data, n);

    // Once all the DVHpoints in data have been sorted by dose, start at the
    // DVHpoint with the second smallest dose and add the volume of the smallest
    // dose DVHpoint.  Then for the third smallest dose of a DVHpoint add the
    // volume of the second smallest dose of a DVHpoint (which is the sum of
    // what was originally the smallest and second smallest volume) and continue
    // until at the very end you have the total volume at data[0]
    for (int i = n-2; i >= 0; i--) {
        data[i].vol += data[i+1].vol;
    }
    total = data[0].vol; // Set total to the volume over which we histogram

    // Here we compute Dx and Vx by using our newly sorted and cumulative data
    for (int i = 0; i < Vx->size(); i++) {
        (*Vx)[i] = doseSearch(data, 0, n/2, n, &((*Vx)[i]));
    }
    double tempVol;
    for (int i = 0; i < Dx->size(); i++) {
        tempVol = (*Dx)[i]/100.0*(*totVol);
        tempVol = (*Dx)[i]/100.0*(*totVol);
        (*Dx)[i] = volSearch(data, 0, n/2, n, &tempVol);
    }

    double increment = 1; // Set an increment to avoid creating csv data
    int index = 0;        // with more than 1000 data points
    if (n > 1000) {
        increment = double(n-1)/999.0;
    }

    output += "dose" + delimiter + "volume (%)\n";
    for (int i = 0; i < n && i < 1000; i++) {
        index = int(increment*i);
        output += QString::number(data[index].dose, 'E', 8);
        output += delimiter;
        // We take the fraction of the total and turn it into a percentage
        output += QString::number(data[index].vol/total*100.0, 'E', 8);
        output += "\n";
    }
    delete[] data;
    return output;
}

double Dose::volSearch(DVHpoint *data, int i, int c, int f, double *vol) {
    // This algorithm uses the fact that the data is sorted to eliminate half
    // of the possible data on which the point volume could lie, then when it
    // finds it, it returns the dose
    // This algorithm is O(log(n))

    if (c == i || (data[c].vol >= *vol && *vol > data[c+1].vol)) {
        return data[c].dose;
    }
    else if (c == f || (data[c-1].vol >= *vol && *vol > data[c].vol)) {
        return data[c-1].dose;
    }
    else if (data[c].vol > *vol) {
        return volSearch(data, c, (f+c)/2, f, vol);
    }
    else {
        return volSearch(data, i, (i+c)/2, c, vol);
    }
}

double Dose::doseSearch(DVHpoint *data, int i, int c, int f, double *dose) {
    // This algorithm uses the fact that the data is sorted to eliminate half
    // of the possible data on which the point dose could lie, then when it
    // finds it, it returns the volume
    // This algorithm is O(log(n))

    if (c == i || (data[c].dose <= *dose && *dose < data[c+1].dose)) {
        return data[c].vol;
    }
    else if (c == f || (data[c-1].dose <= *dose && *dose < data[c].dose)) {
        return data[c-1].vol;
    }
    else if (data[c].dose < *dose) {
        return doseSearch(data, c, (f+c)/2, f, dose);
    }
    else {
        return doseSearch(data, i, (i+c)/2, c, dose);
    }
}


QString Dose::stat(double min, double max, double nBin,
                   double xi, double xf, double yi, double yf, double zi,
                   double zf, EGSPhant *egsphant, QVector <char> *med,
                   double *totVol, double *nVox, double *chi, double *rms,
                   double *area1, double *area2) {
    // Build a QString that holds csv data
    QString output = "";
    QString delimiter = ",";
    QVector <double> range;
    QVector <int> freq;
    for (int i = 0; i < nBin; i++) {
        range += min + ((max-min)/double(nBin) * i);
        freq += 0;
    }
    range += max;

    // Additional values defaults
    *nVox = *totVol = *rms = *chi = *area1 = *area2 = 0;

    // Iterate through every voxel to find the number of DVHpoints needed
    // for either the whole thing, a region or several media
    if (egsphant != NULL) { // If we do point to an egsphant file
        for (int k = 0; k < val[0][0].size(); k++)
            for (int j = 0; j < val[0].size(); j++)
                for (int i = 0; i < val.size(); i++)
                    for (int m = 0; m < med->size(); m++) //media agreement
                        if (egsphant->getMedia((cx[i+1]+cx[i])/2.0,
                                               (cy[j+1]+cy[j])/2.0,
                                               (cz[k+1]+cz[k])/2.0) ==
                                (*med)[m]) {
                            if (val[i][j][k] < min) // If the dose is under our
                            { /*Do nothing*/ }      // minimum value, then do
                            else {                  // nothing
                                // Iterate through every bin, and if the dose is
                                // less than the upper limit of bin p, then
                                // add 1 to the frequency of that bin, add its
                                // volume to the total, count the voxel, add to
                                // RMS and add to Chi-Squared
                                for (int p = 0; p < nBin; p++)
                                    if (val[i][j][k] < range[p+1]) {
                                        freq[p]++;
                                        *totVol += (cx[i+1]-cx[i])*
                                                   (cy[j+1]-cy[j])*
                                                   (cz[k+1]-cz[k]);
                                        (*nVox)++;
                                        *rms += pow(val[i][j][k],2);
                                        *chi += pow(val[i][j][k]/
                                                    err[i][j][k],2);
                                        break; // stop iterating higher bins
                                    }
                            }
                        }
    }
    else if ((xi != xf) && (yi != yf) && (zi != zf)) { // If region has nonzero
        // volume
        for (int k = 0; k < val[0][0].size(); k++)
            for (int j = 0; j < val[0].size(); j++)
                for (int i = 0; i < val.size(); i++)
                    if ((cx[i+1]+cx[i])/2.0 > xi && (cx[i+1]+cx[i])/2.0 < xf &&
                            (cy[j+1]+cy[j])/2.0 > yi && (cy[j+1]+cy[j])/2.0 < yf &&
                            (cz[k+1]+cz[k])/2.0 > zi && (cz[k+1]+cz[k])/2.0 < zf) {
                        if (val[i][j][k] < min) // If the dose is under our
                        { /*Do nothing*/ }      // minimum value, then do
                        else {                  // nothing
                            // Iterate through every bin, and if the dose is
                            // less than the upper limit of bin p, then
                            // add 1 to the frequency of that bin, add its
                            // volume to the total, count the voxel, add to RMS
                            // and add to Chi-Squared
                            for (int p = 0; p < nBin; p++)
                                if (val[i][j][k] < range[p+1]) {
                                    freq[p]++;
                                    *totVol += (cx[i+1]-cx[i])*
                                               (cy[j+1]-cy[j])*
                                               (cz[k+1]-cz[k]);
                                    (*nVox)++;
                                    *rms += pow(val[i][j][k],2);
                                    *chi += pow(val[i][j][k]/err[i][j][k],2);
                                    break; // stop iterating higher bins
                                }
                        }
                    }
    }
    else { // If we are analyzing entire 3ddose file
        for (int k = 0; k < val[0][0].size(); k++)
            for (int j = 0; j < val[0].size(); j++)
                for (int i = 0; i < val.size(); i++) {
                    if (val[i][j][k] < min) // If the dose is under our
                    { /*Do nothing*/ }      // minimum value or over our
                    if (val[i][j][k] > max) // maximum value, then do nothing
                    { /*Do nothing*/ }
                    else {
                        // Iterate through every bin, and if the dose is
                        // less than the upper limit of bin p, then
                        // add 1 to the frequency of that bin, add its
                        // volume to the total, count the voxel, add to RMS and
                        // add to Chi-Squared
                        for (int p = 0; p < nBin; p++) {
                            if (val[i][j][k] < range[p+1]) {
                                freq[p]++;
                                *totVol += (cx[i+1]-cx[i])*
                                           (cy[j+1]-cy[j])*
                                           (cz[k+1]-cz[k]);
                                (*nVox)++;
                                *rms += pow(val[i][j][k],2);
                                *chi += pow(val[i][j][k]/err[i][j][k],2);
                                break; // stop iterating higher bins
                            }
                        }
                    }
                }
    }

    *rms = sqrt((*rms)/(*nVox));
    *chi = (*chi)/(*nVox);

    output += "volume" + delimiter + "dose (normalized)\n";
    // Make the histogram output
    for (int p = 0; p < range.size()-1; p++) {
        //output += QString::number((range[p]+range[p+1])/2.0, 'E', 8);
        output += QString::number(range[p+1], 'E', 8);
        output += delimiter;

        // We take the fraction of the total and turn it into a percentage
        output += QString::number(freq[p]/(*nVox), 'E', 8);
        output += "\n";

        // Calculate the integrals from -2 to 2
        if (range[p] > -2 && range[p+1] < 2) {
            *area2 += double(freq[p])/(*nVox);
        }
        else if (range[p] < -2 && range[p+1] > -2)
            *area2 += double(freq[p])/(*nVox)
                      *(2.0+range[p+1])/(range[p+1]-range[p]);
        else if (range[p] < 2 && range[p+1] > 2)
            *area2 += double(freq[p])/(*nVox)
                      *(-2.0+range[p+1])/(range[p+1]-range[p]);

        // Calculate the integrals from -1 to 1
        if (range[p] > -1 && range[p+1] < 1) {
            *area1 += double(freq[p])/(*nVox);
        }
        else if (range[p] < -1 && range[p+1] > -1)
            *area1 += double(freq[p])/(*nVox)
                      *(1.0+range[p+1])/(range[p+1]-range[p]);
        else if (range[p] < 1 && range[p+1] > 1)
            *area1 += double(freq[p])/(*nVox)
                      *(-1.0+range[p+1])/(range[p+1]-range[p]);
    }
    return output;
}

void Dose::bubble(DVHpoint *data, int n) {
    // This sorting algorithm is only for testing purposes, never to be used
    // to sort DVHpoints due to its inefficiency
    // For example, using this to get the DVH of a 100 x 100 x 100 voxel 3ddose
    // file would take over 50000 times longer than merge to sort
    // This algorithm is O(n*(n-1))

    DVHpoint temp;
    for (int i = 0; i < n-1; i++)
        for (int j = 0; j < n-i; j++)
            if (data[j].dose > data[j+1].dose) {
                temp = data[j];
                data[j] = data[j+1];
                data[j+1] = temp;
            }
}

void Dose::merge(DVHpoint *data, int n) {
    // The following sorting algorithm is referred to as a bottom-up (not
    // recursive) mergesort.  The idea of the algorithm is that adding two
    // sorted arrays of size n/2 into a sorted array of size n should only take
    // n iterations.
    // This is can be thought out pretty simply, we have to sorted arrays, l and
    // r, and a final array d.  l and r are of size n/2 and d is of size n.  We
    // can start by comparing l[0] and r[0].  The largest value of the two is
    // guaranteed to be the largest value of all n data points, so the larger
    // of l[0] and r[0] is added at d[0].  Then another comparison is made
    // between index 0 of the array that did not have the larger value and
    // index 1 of the array that did, and the larger of the two is set as d[1].
    // This process is repeated until d is filled (ie, if d[0] = l[0] then d[1]
    // will be the greater of l[1] and r[0]).
    // The above 'merging' algorithm can be applied to giant unsorted data sets
    // by first merging every 2 indices together (so n = 2 and n/2 = 1).  Once
    // every 2 indices are sorted, we can then go through and merge every 4
    // indices (so n = 4 and n/2 = 2), which works because we have already
    // the array by 2s.  So this is continued until all that's left of the array
    // are two separately sorted halves, which are then merged together in one
    // final iteration
    // This algorithm is O(n*log(n)), which makes sense because we sort all n
    // data points separately log(n) times.   This log is base 2.

    if (n <= 1) { // If our array is size 1 or less quit
        return;
    }

    int subn = 1, i = 0; // subn the size of subsections that are being
    // submerged and i is the current index of the array
    // at which we are at
    while (subn < n) { // While we are still submerging sections that are smaller
        // than the size of the array
        i = 0; // Reset the index to 0
        while (i < n - subn) { // And iterate through n/(2*subn) sections, truncated
            if (i + (2 * subn) < n) // submerge two subn sized portions of data
                // of the array
            {
                submerge(data, i, i + subn - 1, i + 2 * subn - 1);
            }
            else // Or submerge a subn sized section and whatever is left of the
                // array
            {
                submerge(data, i, i + subn - 1, n - 1);
            }
            i += 2 * subn; // Move the index two submerge the next 2 subsections
        }
        subn *= 2; // Double the size of subsection to be merged
    }
}

void Dose::submerge(DVHpoint *data, int i, int c, int f) {
    int l = i, r = c+1, j = 0; // We have three indices, l for one subsection,
    // r the other, and j for the new sorted array
    DVHpoint *temp = new DVHpoint [f-i+1]; // Set aside memory for sorted array
    while (l <= c && r <= f) { // While we have yet to iterate through either
        // subsection
        if (data[l].dose > data[r].dose) { // If value at r index is smaller then
            temp[j++] = data[r++];    // add it to temp and move to next r
        }
        else {                           // If value at l index is smaller then
            temp[j++] = data[l++];    // add it to temp and move to next l
        }
    }
    while (l <= c) { // Add all the remaining ls to temp (if any)
        temp[j++] = data[l++];
    }
    while (r <= f) { // Add all the remaining rs to temp (if any)
        temp[j++] = data[r++];
    }

    for (int k = 0; k < j; k++) {
        data[i+k] = temp[k];    // Reassign all the data values to the temp values
    }

    delete[] temp; // Delete temp
}

// Run through all 3ddose values and create points for
void Dose::getContour(QVector <QVector <QLineF> > *con,
                      QVector <double> doses, QString axis, double depth,
                      double ai, double af, double bi, double bf,
                      int res) {
    // Use marching squares to determine lines of the contour by looking at
    // rectangles formed by 4 neighbouring doses and determining the contour
    // lines going through the rectangle

    // Find the slice to be used
    con->clear();
    con->resize(doses.size());
    int n = getIndex(axis, depth);
    if (n == -1) {
        return;
    }
    int cases = 0, tl = 0, tr = 0, bl = 0, br = 0;
    int fa, fb, flag = false;
    QVector <int> px, py;
    QVector <QVector <double> > d;
    QVector <double> temp;

    px.clear();
    py.clear();
    // Setup dose and pixel arrays
    if (!axis.compare("X")) {
        for (int i = 0; i < y; i++)
            if (cy[i] > bi && cy[i+1] < bf) {
                px.append(int(((cy[i]+cy[i+1])/2.0-bi)*double(res)));
                temp.clear();
                for (int j = 0; j < z; j++)
                    if (cz[j] > ai && cz[j+1] < af) {
                        temp.append(val[n][i][j]);
                        if (!flag)
                            py.append(int(((cz[j]+cz[j+1])/2.0-ai)
                                          *double(res)));
                    }
                flag++;
                d.append(temp);
            }
    }
    else if (!axis.compare("Y")) {
        for (int i = 0; i < x; i++)
            if (cx[i] > bi && cx[i+1] < bf) {
                px.append(int(((cx[i]+cx[i+1])/2.0-bi)*double(res)));
                temp.clear();
                for (int j = 0; j < z; j++)
                    if (cz[j] > ai && cz[j+1] < af) {
                        temp.append(val[i][n][j]);
                        if (!flag)
                            py.append(int(((cz[j]+cz[j+1])/2.0-ai)
                                          *double(res)));
                    }
                flag++;
                d.append(temp);
            }
    }
    else if (!axis.compare("Z")) {
        for (int i = 0; i < x; i++)
            if (cx[i] > bi && cx[i+1] < bf) {
                px.append(int(((cx[i]+cx[i+1])/2.0-bi)*double(res)));
                temp.clear();
                for (int j = 0; j < y; j++)
                    if (cy[j] > ai && cy[j+1] < af) {
                        temp.append(val[i][j][n]);
                        if (!flag)
                            py.append(int(((cy[j]+cy[j+1])/2.0-ai)
                                          *double(res)));
                    }
                flag++;
                d.append(temp);
            }
    }


    for (int i = 0; i < px.size()-1; i++)
        for (int j = 0; j < py.size()-1; j++)
            for (int p = 0; p < doses.size(); p++) {
                // Cases tells us the number of vertices above the
                // contour dose
                // Bottom Left
                cases  = bl = (d[i][j] > doses[p]);
                // Bottom Right
                cases += br = (d[i+1][j] > doses[p]);
                // Top Left
                cases += tl = (d[i][j+1] > doses[p]);
                // Top Right
                cases += tr = (d[i+1][j+1] > doses[p]);

                if (cases == 1) { // We have 1 vertex within contour, use linear
                    // interpolation to determine line
                    if (bl) {
                        fa = interp(px[i], px[i+1],
                                    d[i][j], d[i+1][j],
                                    doses[p]);
                        fb = interp(py[j], py[j+1],
                                    d[i][j], d[i][j+1],
                                    doses[p]);
                        (*con)[p].append(QLineF(fa, py[j], px[i], fb));
                    }
                    else if (br) {
                        fa = interp(px[i], px[i+1],
                                    d[i][j], d[i+1][j],
                                    doses[p]);
                        fb = interp(py[j], py[j+1],
                                    d[i+1][j], d[i+1][j+1],
                                    doses[p]);
                        (*con)[p].append(QLineF(fa, py[j], px[i+1], fb));
                    }
                    else if (tl) {
                        fa = interp(px[i], px[i+1],
                                    d[i][j+1], d[i+1][j+1],
                                    doses[p]);
                        fb = interp(py[j], py[j+1],
                                    d[i][j], d[i][j+1],
                                    doses[p]);
                        (*con)[p].append(QLineF(fa, py[j+1], px[i], fb));
                    }
                    else if (tr) {
                        fa = interp(px[i], px[i+1],
                                    d[i][j+1], d[i+1][j+1],
                                    doses[p]);
                        fb = interp(py[j], py[j+1],
                                    d[i+1][j], d[i+1][j+1],
                                    doses[p]);
                        (*con)[p].append(QLineF(fa, py[j+1], px[i+1], fb));
                    }
                }
                else if (cases == 3) { // We have 3 vertices within contour, use
                    // linear interpolation to determine line
                    if (!bl) {
                        fa = interp(px[i], px[i+1],
                                    d[i][j], d[i+1][j],
                                    doses[p]);
                        fb = interp(py[j], py[j+1],
                                    d[i][j], d[i][j+1],
                                    doses[p]);
                        (*con)[p].append(QLineF(fa, py[j], px[i], fb));
                    }
                    else if (!br) {
                        fa = interp(px[i], px[i+1],
                                    d[i][j], d[i+1][j],
                                    doses[p]);
                        fb = interp(py[j], py[j+1],
                                    d[i+1][j], d[i+1][j+1],
                                    doses[p]);
                        (*con)[p].append(QLineF(fa, py[j], px[i+1], fb));
                    }
                    else if (!tl) {
                        fa = interp(px[i], px[i+1],
                                    d[i][j+1], d[i+1][j+1],
                                    doses[p]);
                        fb = interp(py[j], py[j+1],
                                    d[i][j], d[i][j+1],
                                    doses[p]);
                        (*con)[p].append(QLineF(fa, py[j+1], px[i], fb));
                    }
                    else if (!tr) {
                        fa = interp(px[i], px[i+1],
                                    d[i][j+1], d[i+1][j+1],
                                    doses[p]);
                        fb = interp(py[j], py[j+1],
                                    d[i+1][j], d[i+1][j+1],
                                    doses[p]);
                        (*con)[p].append(QLineF(fa, py[j+1], px[i+1], fb));
                    }
                }
                else if (cases == 2) { // We have 2 vertices within contour, use
                    // linear interpolation to determine line
                    // The first two are simple single line cases
                    if ((bl && br) || (tl && tr)) { // Horizontal line
                        fa = interp(py[j], py[j+1],
                                    d[i][j], d[i][j+1],
                                    doses[p]);
                        fb = interp(py[j], py[j+1],
                                    d[i+1][j], d[i+1][j+1],
                                    doses[p]);
                        (*con)[p].append(QLineF(px[i], fa, px[i+1], fb));
                    }
                    else if ((bl && tl) || (br && tr)) { // Vertical line
                        fa = interp(px[i], px[i+1],
                                    d[i][j], d[i+1][j],
                                    doses[p]);
                        fb = interp(px[i], px[i+1],
                                    d[i][j+1], d[i+1][j+1],
                                    doses[p]);
                        (*con)[p].append(QLineF(fa, py[j], fb, py[j+1]));
                    }
                    else if ((bl && tr)) { // First ambiguous case
                        fa = interp(px[i], px[i+1],
                                    d[i][j], d[i+1][j],
                                    doses[p]);
                        fb = interp(py[j], py[j+1],
                                    d[i][j], d[i][j+1],
                                    doses[p]);
                        (*con)[p].append(QLineF(fa, py[j], px[i], fb));
                        fa = interp(px[i], px[i+1],
                                    d[i][j+1], d[i+1][j+1],
                                    doses[p]);
                        fb = interp(py[j], py[j+1],
                                    d[i+1][j], d[i+1][j+1],
                                    doses[p]);
                        (*con)[p].append(QLineF(fa, py[j+1], px[i+1], fb));
                    }
                    else if ((br && tl)) { // Second ambiguous case
                        fa = interp(px[i], px[i+1],
                                    d[i][j], d[i+1][j],
                                    doses[p]);
                        fb = interp(py[j], py[j+1],
                                    d[i+1][j], d[i+1][j+1],
                                    doses[p]);
                        (*con)[p].append(QLineF(fa, py[j], px[i+1], fb));
                        fa = interp(px[i], px[i+1],
                                    d[i][j+1], d[i+1][j+1],
                                    doses[p]);
                        fb = interp(py[j], py[j+1],
                                    d[i][j], d[i][j+1],
                                    doses[p]);
                        (*con)[p].append(QLineF(fa, py[j+1], px[i], fb));
                    }
                }
            }
}

int Dose::interp(int x1, int x2, double y1, double y2, double y0) {
    return int(x1+(y0-y1)*(x2-x1)/(y2-y1));
}

const QString Dose::getTitle() {
    return title;
}

void Dose::setTitle(QString name) {
    title = name;
}

/*******************************************************************************
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Line Input Class~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*******************************************************************************/
LineInput::LineInput(QWidget *parent, QString lab, QString val)
    : QWidget(parent) {
    label = new QString(lab);
    title = new QLabel(lab);
    text = new QLineEdit(val);

    // Style settings
    QString style;
    style  = "QLineEdit";
    //style += "{";
    //style += "background-color: rgb(250, 250, 255)";
    //style += "}";
    setStyleSheet(style);

    layout = new QVBoxLayout();
    layout->addWidget(title);
    layout->addWidget(text);
    layout->setSpacing(0);
    setLayout(layout);
}

LineInput::~LineInput() {
    delete label;
    delete text;
    delete title;
    delete layout;
}

QString LineInput::getText() {
    return text->text();
}

void LineInput::setText(QString s) {
    text->setText(s);
}

void LineInput::setText(double d) {
    text->setText(QString::number(d));
}

QString LineInput::getLabel() {
    return *label;
}
