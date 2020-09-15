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
#ifndef DOSE_H
#define DOSE_H

#include "egsphant.h"

// This structure holds a dose and a volume, this is essentially a point on a
// DVH, and I made it a struct for minimal overhead
struct DVHpoint {
    double dose;
    double vol;
};

class Dose : public QObject {
    Q_OBJECT

signals:
    void progressMade(double n); // Update the progress bar

public:
    // The constructor uses the n to determine how many .3ddose files will be
    // read in, so as to progress the progress bar accordingly
    Dose(QString *path = 0, int n = 1);
    Dose(const Dose &d); // Copy constructor, used to make a deep copy
    ~Dose();

    QString title; // The name of the dose file as it will appear in the GUI

    int x, y, z; // The number of x, y and z voxels
    QVector <double> cx, cy, cz; // The actual x, y and z coordinates
    QVector < QVector < QVector <double> > > val; // The values
    QVector < QVector < QVector <double> > > err; // The fractional errors
    char filled; // Flag that says if the dose file is empty of not

    // Interpolate lineary at a point ap between a0 and a1 (which have dose b0
    // and b1 respectively), val and err get passed the dose and error at ap,
    // the fucntion also returns the error
    double linInterpol(double ap, double a0, double a1, double b0, double b1,
                       double e0, double e1, double *val, double *err);
    // Interpolate the dose and error of the point (xp, yp, zp), function passes
    // value to val and error to err, and return val
    double triInterpol(double xp, double yp, double zp, double *val,
                       double *err);

    // Returns 1 if the dimensions of this and other are the same, else 1
    int compareDimensions(Dose *other);

    // Read in a .3ddose file, again with the n to be used by the progress bar
    void readIn(QString path, int n);
    void readBIn(QString path, int n);

    // Save data as a .3ddose file, again n to be used by the progress bar
    void readOut(QString path, int n);
    void readBOut(QString path, int n);

    // This copies other
    void copyDose(Dose *other);

    // Square each dose in this
    void square();

    // Root each dose in this
    void root();

    // Add other to this
    void addDose(Dose *other);

    // Subtract other from this
    void subtractDose(Dose *other);

    // Used solely for statistical comparison
    void subtractDoseWithError(Dose *other);

    // Divide this by other
    int divideDose(Dose *other); // Returns the number of voxels divided by zero

    // Calculate dose difference normalized by local voxel dose
    void localDose(Dose *other);

    // Translate the origin by dx, dy and dz
    int translate(double dx, double dy, double dz);

    // Remove the outer layers of voxels
    int strip();

    // Returns the index of the coordinate matrix at val
    int getIndex(QString axis, double val);

    // These functions return dose at a point in real space or at an index
    double getDose(double px, double py, double pz);
    double getError(double px, double py, double pz);
    double getDose(int ix, int iy, int iz);
    double getError(int ix, int iy, int iz);

    // Returns the max dose of this
    double getMax();

    // Returns the min, max and avg dose of this
    double getMinMaxAvg(double *min, double *max, double *avg);
    double getMinMaxAvg(double *min, double *max, double *avg, double xi,
                        double xf, double yi, double yf, double zi, double zf);
    double getMinMaxAvg(double *min, double *max, double *avg,
                        EGSPhant *egsphant, QVector <char> *med);

    // Scales all the dose such that the dose at (px, py, pz) is goal
    // (usually 1)
    int scaleAtPoint(double px, double py, double pz, double goal, char doTri);

    // Scale this by factor
    int scale(double factor);

    // Divide each val entry in this by corresponding err entry
    void scaleError();

    // Scale this so that the average dose is 1
    int scaleAverage();

    // Scale this so that the maximum value within the rectangle defined by
    // (ix, iy, iz) and (fx, fy, fz) is 1
    int scaleArea(double ix, double iy, double iz, double fx, double fy,
                  double fz);

    // Return a QString to be read by xmgrace; the curve will be along axis at
    // the point (a, b) and will range from start to stop
    QString plot(QString axis, double a, double b, double start, double stop);

    // Return a QString containing DV for xmgrace to H
    QString plot(double xi, double xf, double yi, double yf, double zi,
                 double zf, EGSPhant *egsphant, QVector <char> *med,
                 double *min, double *eMin, double *max, double *eMax,
                 double *avg, double *eAvg, double *meanErr, double *maxErr,
                 double *totVol, double *nVox, QVector <double> *Dx,
                 QVector <double> *Vx);

    // Return a QString containing bin values for an xmgrace histogram
    QString stat(double min, double max, double nBin,
                 double xi, double xf, double yi, double yf, double zi,
                 double zf, EGSPhant *egsphant, QVector <char> *med,
                 double *totVol, double *nVox, double *chi, double *rms,
                 double *area1, double *area2);

    // Sorts DVH data to go from highest dose to lowest
    void merge(DVHpoint *data, int n);
    void submerge(DVHpoint *data, int i, int c, int f);
    double doseSearch(DVHpoint *data, int i, int c, int f, double *dose);
    double volSearch(DVHpoint *data, int i, int c, int f, double *vol);
    void bubble(DVHpoint *data, int n); // This is here for testing, do not use

    // Return the this title
    const QString getTitle();

    // Set this title
    void setTitle(QString name);

    // Get isodose points
    void getContour(QVector <QVector <QLineF> > *con, QVector <double> doses,
                    QString axis, double depth, double ai, double af,
                    double bi, double bf, int res);
    int interp(int x1, int x2, double y1, double y2, double y0);

    // Progress bar resolution
    const static int MAX_PROGRESS = 1000000000;
};

/*******************************************************************************
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Line Input Class~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*******************************************************************************/
class LineInput : public QWidget {
public:
    // Constructor that takes in everything needed
    LineInput(QWidget *parent, QString lab, QString val = "");
    // Destructor
    ~LineInput();

    // Variables needed
    QString *label;
    QLabel *title;
    QLineEdit *text;
    QVBoxLayout *layout;

    // Functions needed
    QString getText();
    QString getLabel();
    void setText(QString s);
    void setText(double d);
};

#endif
