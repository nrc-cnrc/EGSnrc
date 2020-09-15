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
#ifndef PREVIEWER_H
#define PREVIEWER_H

#define TRUE 1
#define FALSE 0
#include "dose.h"
#include "egsphant.h"

/*******************************************************************************
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Hover Label Class~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*******************************************************************************/
// This class is created to overwrite one of the QWidget functions to be able to
// keep track of the position of the mouse, and to know whether it is over the
// widget or not, so as to be able to give the coordinates of the seed

class HoverLabel : public QLabel { // It inherits QLabel publicly
private:
    Q_OBJECT // This line is necessary to create custom SLOTs, ie, functions
    // that define what happens when you click on buttons
public:
    void mouseMoveEvent(QMouseEvent *event); // Overwrite mouseMoveEvent to send
    // a signal to MainWindow
    void wheelEvent(QWheelEvent *event); // Overwrite wheelEvent to send
    // a signal to MainWindow
signals:
    void mouseMoved(int width, int height); // This is the signal to be
    // sent in mouseMoveEvent
    void mouseWheelUp();   // Detect the wheel being scrolled over the image
    void mouseWheelDown(); // to shift depth appropriately
};

/******************************************************************************
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Examine Window Class~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*******************************************************************************/
class  Previewer: public QWidget {
private:
    Q_OBJECT // This line is necessary to create custom SLOTs, ie, functions
    // that define what happens when you click on buttons
public:
    // The constructor
    Previewer(QWidget *parent, QVector <Dose *> *d);

    // The destructor
    ~Previewer();

    // Data
    QWidget *mom;
    QVector <Dose *> *data;
    EGSPhant phant;
    QVector <QVector <QLineF> > solid;
    QVector <QVector <QLineF> > dashed;
    QVector <QVector <QLineF> > dotted;
    QPixmap phantPicture;

    // Universal
    QErrorMessage *errors;
    QWidget *window;
    QGridLayout *layout;
    QPushButton *close;
    QPushButton *savePic;
    QPushButton *loadPhant;
    QLabel *pose;

    // Image Frame
    HoverLabel *image;
    QScrollArea *imageFrame;
    QPixmap *picture;

    // Picture Type Frame
    QGroupBox *typeFrame;
    QGridLayout *typeLayout;
    QRadioButton *typeMed;
    QRadioButton *typeDen;

    // Image Dimensions
    QGroupBox *dimFrame;
    QStringList *dimItems;
    QComboBox *dimBox;
    LineInput *dimai, *dimbi, *dimaf, *dimbf;
    QLabel *depth;
    QSlider *dimc;
    QToolButton *dimcLeft, *dimcRight;
    QGridLayout *dimLayout;

    // Image Resolution
    QLineEdit *resEdit;
    QHBoxLayout *resLayout;
    QGroupBox *resFrame;

    // Contour Selector
    QFrame *contourFrame;
    QGridLayout *contourLayout;
    QCheckBox *wash, *legend;
    QVector <QPushButton *> *colors;
    QVector <QLineEdit *> *doses;

    // Dose Selector
    QFrame *doseFrame;
    QGridLayout *doseLayout;
    QVector <QComboBox *> *distribs;
    QVector <QLabel *> *lines;

public slots:
    // Methods
    void mouseGeom(int width, int height); // This updates pose when a
    // mouse signal is received
    void changeDim(); // Change labeling on dimensions
    void changeDepthLeft();
    void changeDepthRight();
    void checkBounds();
    void changeColor(); // Change isodose contour color
    void loadPhantom();
    void updateImage(); // This updates the image
    void updateContour(); // This updates the contours
    void updateWash(); // This updates the wash
    void updateDoses(); // This updates the selecteable isodose contours
    void redraw();
    void saveImage();
    void closePreview();
    void flipExtraDoses();

public:
// LAYOUT FUNCTIONS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    void createLayout();
    void connectLayout();

// OTHER~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    QColor getColor(double d, QVector <double> *t, QVector < QVector <int> > *c); // Determine color shade for wash

// PROGRESS BAR~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    double *remainder;
    QWidget *progWin;
    QGridLayout *progLayout;
    QProgressBar *progress;

public slots:
    void updateProgress(double n);
};

#endif
