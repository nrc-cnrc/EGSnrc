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
#ifndef INTERFACE_H
#define INTERFACE_H

#define TRUE 1
#define FALSE 0
#include "egsphant.h"
#include "dose.h"
#include "previewer.h"
#include "3ddt_logo.xpm"

/*******************************************************************************
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Main Window Class~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*******************************************************************************/
// Declaration of the Main Window class, its variables and its methods
class Interface : public QWidget {
private:
    Q_OBJECT // This line is necessary to create custom SLOTs, ie, functions
    // that define what happens when you click on buttons
public:
    Interface();
    ~Interface();

public slots:
    // All the function calls when something is pressed
    void refresh(); // Reset all on screen values properly
    void addDose(); // Add one or more 3ddose files
    void outputDose(); // Save distributions
    void removeDose(); // Remove distributions
    void copyDose(); // Add one or more 3ddose files
    void renameDose(); // Add one or more 3ddose files
    void normalize(); // Scale or normalize distributions
    void translate(); // Translate distributions
    void strip(); // Remove the outer voxels of distributions
    void divide(); // Divide distributions by another distribution
    void subtract(); // Subtract distributions by another distribution
    void add(); // Subtract distributions by another distribution
    void plot(); // Create an plot
    void plotAxis(); // Of a uninterpolated line along an axis
    void plotLine(); // Of a interpolated line defined abitrarily
    void plotDVH(); // Of an overall DVH
    void plotDVHReg(); // Of the DVH of a specific volume
    void plotDVHMed(); // Of the DVH on certain media in an egsphant file
    void stat(); // Create an plot
    void statH(Dose *comp); // Of an overall comparison histogram
    void statHReg(Dose *comp); // Of the comparison histogram of a specific
    // volume
    void statHMed(Dose *comp); // Of the comparison histogram on certain media
    // in an egsphant file
    void doseAtPoints(); // Output a text file with the dose at several points
    // defined in an input text file
    void rebinBounds(); // Change 3ddose file boundaries based on those defined
    // in an input text file
    //void Grace(QString path); // Call xmgrace to display the plot at path
    //void doneGrace(); // Close xmgrace
    void selectEGSFile(); // Select egsphant file for plotDVHMed()
    void selectStatEGSFile(); // Select egsphant file for comparison
    void showPreview(); // Load previewer and disable back end

public:
    Previewer *previewer; // The preview window
    QVector <Dose *> *data; // The vector that stores all the dose data
    QMessageBox badInput; // The error message that pops when input is bad
    QMessageBox done; // The message that pops up to say a task is complete

// PROGRESS BAR~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    double *remainder;
    QWidget *progWin;
    QGridLayout *progLayout;
    QProgressBar *progress;

public slots:
    void updateProgress(double n);

public:
// LAYOUT~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    QGridLayout *mainLayout;
    QPushButton *close;
    QPushButton *preview;

    QLabel *logo;

    QGroupBox *doseFrame;
    QCheckBox *tri;
    QListWidget *doseList;
    QGridLayout *doseLayout;

    QGroupBox *ioFrame;
    QPushButton *iButton;
    QPushButton *oButton;
    QPushButton *cButton;
    QPushButton *rButton;
    QPushButton *dButton;
    QGridLayout *ioLayout;

    QGroupBox *normFrame;
    QStringList *normItems;
    QComboBox *normBox;
    QPushButton *normButton;
    QStackedLayout *normStack;
    QGridLayout *normLayout;
    LineInput *factor;
    QWidget *scaleWidget;
    QGridLayout *scaleLayout;
    LineInput *normx, *normy, *normz;
    QWidget *pointWidget;
    QGridLayout *pointLayout;
    LineInput *normxi, *normyi, *normzi, *normxf, *normyf, *normzf;
    QWidget *lineWidget;
    QGridLayout *lineLayout;
    QWidget *aveWidget;
    QWidget *perWidget;

    QGroupBox *transFrame;
    QPushButton *transButton;
    LineInput *transx, *transy, *transz;
    QGridLayout *transLayout;

    QGroupBox *statFrame;
    QPushButton *statButton;
    QWidget *statTotWidget;
    QGridLayout *statTotLayout;
    QStackedLayout *statStack;
    QStringList *statItems;
    QComboBox *statBox;
    QStringList *statOptItems;
    QComboBox *statOptBox;
    QWidget *statRegWidget;
    QGridLayout *statRegLayout;
    LineInput *statxi, *statyi, *statzi, *statxf, *statyf, *statzf;
    LineInput *statMin;
    LineInput *statMax;
    LineInput *statBin;
    QLineEdit *statEgsFile;
    QPushButton *statEgsBrowse;
    QListWidget *statEgsMeds;
    QGridLayout *statEgsLayout;
    QWidget *statEgsWidget;
    QCheckBox *statExtra;
    QCheckBox *statAutoBounds;
    EGSPhant *statEgsphant;
    QGridLayout *statLayout;

    QGroupBox *plotFrame;
    QPushButton *plotButton;
    LineInput *plotxi, *plotyi, *plotzi, *plotxf, *plotyf, *plotzf, *plotRes;
    QGridLayout *plotLineLayout;
    QWidget *plotLineWidget;
    QGridLayout *plotAxisLayout;
    QWidget *plotAxisWidget;
    QWidget *plotDVHWidget;
    QGridLayout *plotDVHLayout;
    QStackedLayout *plotStack;
    QStringList *plotItems;
    QComboBox *plotBox;
    LineInput *plota, *plotb, *plotci, *plotcf;
    QWidget *dvhWidget;
    QGridLayout *dvhLayout;
    LineInput *dvhxi, *dvhyi, *dvhzi, *dvhxf, *dvhyf, *dvhzf;
    QLineEdit *egsDVHFile;
    QPushButton *egsDVHBrowse;
    QListWidget *egsDVHMeds;
    QGridLayout *egsDVHLayout;
    QWidget *egsDVHWidget;
    QCheckBox *dvhExtra;
    QLabel *dvhDoseLab;
    QLineEdit *dvhDose;
    QLabel *dvhVolLab;
    QLineEdit *dvhVol;
    EGSPhant *egsphant;
    QGridLayout *plotLayout;

    QGroupBox *mathFrame;
    QPushButton *ratioButton;
    QPushButton *diffButton;
    QPushButton *sumButton;
    QGridLayout *mathLayout;

    QGroupBox *newFrame;
    QPushButton *stripButton;
    QPushButton *pointButton;
    QPushButton *rebinButton;
    QGridLayout *newLayout;

    // Declaring tab layout methods
    void createLayout();
    void connectLayout();
};

#endif
