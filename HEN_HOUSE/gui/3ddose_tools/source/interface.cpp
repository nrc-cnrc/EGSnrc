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
#include "interface.h"

/*******************************************************************************
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Interface Structors~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*******************************************************************************/
// Constructor
Interface::Interface() {
    data = new QVector <Dose *> ();
    badInput.setWindowTitle("Warning");
    badInput.resize(400,150);

    createLayout();
    connectLayout();

    refresh();
}

// Destructor
Interface::~Interface() {
    delete previewer;
    for (int i = 0; i < data->size(); i++) {
        delete (*data)[i];
    }
    delete data;

    delete tri;
    delete doseList;
    delete doseLayout;
    delete doseFrame;

    delete logo;

    delete iButton;
    delete oButton;
    delete cButton;
    delete rButton;
    delete dButton;
    delete ioLayout;
    delete ioFrame;

    delete statBin;
    delete statMin;
    delete statMax;
    delete statButton;
    delete statTotLayout;
    delete statTotWidget;
    delete statStack;
    delete statItems;
    delete statBox;
    delete statOptItems;
    delete statOptBox;
    delete statxi;
    delete statyi;
    delete statzi;
    delete statxf;
    delete statyf;
    delete statzf;
    delete statRegLayout;
    delete statRegWidget;
    delete statEgsFile;
    delete statEgsBrowse;
    delete statEgsMeds;
    delete statEgsLayout;
    delete statEgsWidget;
    delete statExtra;
    delete statAutoBounds;
    delete statEgsphant;
    delete statLayout;
    delete statFrame;

    delete normItems;
    delete normBox;
    delete normButton;
    delete factor;
    delete scaleLayout;
    delete scaleWidget;
    delete normx;
    delete normy;
    delete normz;
    delete pointLayout;
    delete pointWidget;
    delete normxi;
    delete normyi;
    delete normzi;
    delete normxf;
    delete normyf;
    delete normzf;
    delete lineLayout;
    delete lineWidget;
    delete aveWidget;
    delete perWidget;
    delete normStack;
    delete normLayout;
    delete normFrame;

    delete transButton;
    delete transx;
    delete transy;
    delete transz;
    delete transLayout;
    delete transFrame;

    delete dvhExtra;
    delete dvhDoseLab;
    delete dvhVolLab;
    delete dvhDose;
    delete dvhVol;
    delete plotItems;
    delete plotBox;
    delete plota;
    delete plotb;
    delete plotci;
    delete plotcf;
    delete plotxi;
    delete plotyi;
    delete plotzi;
    delete plotxf;
    delete plotyf;
    delete plotzf;
    delete plotRes;
    delete plotLineLayout;
    delete plotLineWidget;
    delete plotAxisLayout;
    delete plotAxisWidget;
    delete dvhxi;
    delete dvhyi;
    delete dvhzi;
    delete dvhxf;
    delete dvhyf;
    delete dvhzf;
    delete dvhLayout;
    delete dvhWidget;
    delete egsDVHFile;
    delete egsDVHBrowse;
    delete egsDVHMeds;
    delete egsDVHLayout;
    delete egsDVHWidget;
    delete egsphant;
    delete plotButton;
    delete plotStack;
    delete plotLayout;
    delete plotFrame;

    delete ratioButton;
    delete diffButton;
    delete sumButton;
    delete mathLayout;
    delete mathFrame;

    delete stripButton;
    delete pointButton;
    delete rebinButton;
    delete newLayout;
    delete newFrame;

    delete remainder;
    delete progress;
    delete progLayout;
    delete progWin;

    delete close;
    delete preview;
    delete mainLayout;
}

/*******************************************************************************
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Dose Tools~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*******************************************************************************/
void Interface::addDose() {
    QStringList paths =
        QFileDialog::getOpenFileNames(0, tr("Load Files"), 0,
                                      tr("3ddose (*.3ddose *.b3ddose)"));

    // Set up the progress bar
    progress->reset();
    *remainder = 0;
    progWin->setWindowTitle("Importing Dose Data");
    progWin->show();
    progWin->activateWindow();
    progWin->raise();

    // Read in as many 3ddose files as there are paths
    for (int i = 0; i < paths.size(); i++) {
        int n = paths.at(i).count("/");
        QString p = paths.at(i);
        QString s = p.section("/", n, n);

        data->insert(0, new Dose(0));

        // This connects receiving a progress signal to updating the bar
        connect((*data)[0], SIGNAL(progressMade(double)),
                this, SLOT(updateProgress(double)));

        if (p.endsWith(".b3ddose")) {
            (*data)[0]->readBIn(p, paths.size());
        }
        else {
            (*data)[0]->readIn(p, paths.size());
        }

        disconnect((*data)[0], SIGNAL(progressMade(double)),
                   this, SLOT(updateProgress(double)));

        if (p.endsWith(".b3ddose")) {
            doseList->insertItem(0, s.left(s.length()-8));
            (*data)[0]->setTitle(s.left(s.length()-8));
        }
        else {
            doseList->insertItem(0, s.left(s.length()-7));
            (*data)[0]->setTitle(s.left(s.length()-7));
        }
    }

    progWin->hide();

    refresh();
}

void Interface::renameDose() {
    QString name = "";
    // Read in as many 3ddose files as there are paths
    for (int i = 0; i < data->size(); i++)
        if (doseList->selectedItems().contains(doseList->item(i))) {
            name = QInputDialog::getText((QWidget *)this,
                                         tr("Rename"),
                                         tr("New Name"));
            if (name != "") {
                doseList->item(i)->setText(name);
                (*data)[i]->setTitle(name);
            }
        }

    progWin->hide();

    refresh();
}

void Interface::copyDose() {
    QString name = "";

    // Read in as many 3ddose files as there are paths
    for (int i = 0; i < data->size(); i++)
        if (doseList->selectedItems().contains(doseList->item(i))) {
            name  = (*data)[i]->title + " copy";
            data->insert(0, new Dose(*(*data)[i]));

            (*data)[0]->title = name;
            doseList->insertItem(0, name);
            i++;
        }

    progWin->hide();

    refresh();
}

void Interface::outputDose() {
    QString path;
    int count = 0;

    for (int i = 0; i < data->size(); i++)
        if (doseList->selectedItems().contains(doseList->item(i))) {
            count++;
        }

    // Set up the progress bar
    progress->reset();
    *remainder = 0;
    progWin->setWindowTitle("Exporting Dose Data");
    progWin->show();
    progWin->activateWindow();
    progWin->raise();

    // Save all selected distributions
    for (int i = 0; i < data->size(); i++)
        if (doseList->selectedItems().contains(doseList->item(i))) {
            path =
                QFileDialog::getSaveFileName(0, tr("Save File ") +
                                             doseList->item(i)->text(),
                                             doseList->item(i)->text(),
                                             tr("3ddose (*.3ddose *.b3ddose)"));
            if (path != "") {
                if (!(path.endsWith(tr(".3ddose")) || path.endsWith(tr(".b3ddose")))) {
                    path += tr(".3ddose");
                }

                connect((*data)[i], SIGNAL(progressMade(double)),
                        this, SLOT(updateProgress(double)));

                if (path.endsWith(".b3ddose")) {
                    (*data)[i]->readBOut(path, count);
                }
                else {
                    (*data)[i]->readOut(path, count);
                }

                disconnect((*data)[i], SIGNAL(progressMade(double)), this, SLOT(updateProgress(double)));
            }
        }

    progWin->hide();

    refresh();
}

void Interface::removeDose() {
    QVector <bool> del;
    bool flag = false;
    del.fill(false, data->size());
    done.setText("The distributions ");

    // Remove all selected distributions
    for (int i = 0; i < data->size(); i++)
        if (doseList->selectedItems().contains(doseList->item(i))) {
            del[i] = true;
            if (flag) {
                done.setText(done.text() + ", " + doseList->item(i)->text());
            }
            else {
                done.setText(done.text() + doseList->item(i)->text());
                flag = true;
            }
        }

    for (int i = data->size()-1; i >= 0; i--)
        if (del[i]) {
            data->remove(i);
        }

    // Then clear the table and reload it
    doseList->clear();
    for (int i = 0; i < data->size(); i++) {
        doseList->addItem((*data)[i]->getTitle());
    }

    done.setText(done.text() + " have been removed.");
    refresh();
    done.exec();
}

void Interface::doseAtPoints() {
    QFile *file;
    QTextStream *input;

    QString path = QFileDialog::getOpenFileName(0, tr("Open File"), 0,
                   tr("Points File (*.txt)"));

    if (path == 0) {
        return;
    }

    file = new QFile(path);

    QString s;
    QString tempS;
    QVector <QVector <double>*> points;
    QVector <double> *temp;
    double x, y, z, v, e;

    if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        input = new QTextStream(file);
        int i, f;
        while (!input->atEnd()) {
            tempS = input->readLine();
            i = 0, f = 0;

            // Read until the first digit or negative sign
            while (!(tempS.at(i).isDigit() ||
                     tempS.at(i) == QChar('-'))) {
                i++;
            }

            // First point starts here
            f = i;

            // Keep reading until we have any delimiter that isn't in
            // '-1234.5678E+90e'
            while ((tempS.at(f).isDigit() ||
                    tempS.at(f) == QChar('.') ||
                    tempS.at(f) == QChar('-') ||
                    tempS.at(f) == QChar('+') ||
                    tempS.at(f) == QChar('e') ||
                    tempS.at(f) == QChar('E')) &&
                    f < tempS.size()) {
                f++;
            }

            // Convert point's string to double
            x = tempS.mid(i, f-i).toDouble();

            // Read until the first digit or negative sign
            i = f;
            while (!(tempS.at(i).isDigit() ||
                     tempS.at(i) == QChar('-'))) {
                i++;
            }

            // Second point starts here
            f = i;

            // Keep reading until we have any delimiter that isn't in
            // '-1234.5678E+90e'
            while ((tempS.at(f).isDigit() ||
                    tempS.at(f) == QChar('.') ||
                    tempS.at(f) == QChar('-') ||
                    tempS.at(f) == QChar('+') ||
                    tempS.at(f) == QChar('e') ||
                    tempS.at(f) == QChar('E')) &&
                    f < tempS.size()) {
                f++;
            }

            // Convert point's string to double
            y = tempS.mid(i, f-i).toDouble();

            // Read until the first digit or negative sign
            i = f;
            while (!(tempS.at(i).isDigit() ||
                     tempS.at(i) == QChar('-'))) {
                i++;
            }

            // Second point starts here
            f = i;

            // Keep reading until we have any delimiter that isn't in
            // '-1234.5678E+90e'
            while ((tempS.at(f).isDigit() ||
                    tempS.at(f) == QChar('.') ||
                    tempS.at(f) == QChar('-') ||
                    tempS.at(f) == QChar('+') ||
                    tempS.at(f) == QChar('e') ||
                    tempS.at(f) == QChar('E')) &&
                    f < tempS.size()) {
                f++;
            }

            // Convert point's string to double
            z = tempS.mid(i, f-i).toDouble();

            // Add a new QVector of doubles (ie, [x,y,z]) to QVector containing
            // all points
            points += temp = new QVector <double> ();
            *temp += x;
            *temp += y;
            *temp += z;
        }

        delete input;
    }
    delete file;

    // Boundary Check
    for (int j = 0; j < points.size(); j++)
        for (int i = 0; i < data->size(); i++)
            if (doseList->selectedItems().contains(doseList->item(i))) {
                QString bounds = QString("(Xi to Xf, Yi to Yf, Zi to Zf) ") + tr("bounds are:\n(")
                                 +QString::number((*data)[i]->cx[0])+" to "+QString::number((*data)[i]->cx[(*data)[i]->x])+ QString(", ")
                                 +QString::number((*data)[i]->cy[0])+" to "+QString::number((*data)[i]->cy[(*data)[i]->y])+ QString(", ")
                                 +QString::number((*data)[i]->cz[0])+" to "+QString::number((*data)[i]->cz[(*data)[i]->z])+ QString(").");
                if ((*data)[i]->getIndex("X",(*(points[j]))[0]) < 0 ||
                        (*data)[i]->getIndex("Y",(*(points[j]))[1]) < 0 ||
                        (*data)[i]->getIndex("Z",(*(points[j]))[2]) < 0) {
                    badInput.setText(tr("Some points in file are ") +
                                     tr("outside of selected ") +
                                     tr("distributions.  ") + bounds.toLatin1());
                    badInput.exec();
                    return;
                }
            }

    path = QFileDialog::getSaveFileName(0, tr("Save File"), 0,
                                        tr("Dose at Points (*.txt)"));
    if (path == 0) {
        return;
    }

    if (!path.endsWith(".txt")) {
        path += ".txt";
    }

    file = new QFile(path);

    // Grace output
    if (file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        input = new QTextStream(file);

        //*input << "X\t\t|Y\t\t|Z\t\t|";
        *input << "||                                              ||";

        for (int i = 0; i < data->size(); i++)
            if (doseList->selectedItems().contains(doseList->item(i))) {
                s = (*data)[i]->title;
                if (s.length()>30) {
                    s = s.left(30);
                }

                *input << qSetFieldWidth(30) << center << s
                       << qSetFieldWidth(0) << "|";
                if (i != data->size()-1) {
                    *input << "|";
                }


            }

        *input << "\n";
        *input << "|       X       |       Y       |        Z      |";
        for (int i = 0; i < data->size(); i++) {
            *input << "     Value     |  Uncertainty  |";
        }
        *input << "\n\n";

        for (int j = 0; j < points.size(); j++) {
            x = (*(points[j]))[0];
            y = (*(points[j]))[1];
            z = (*(points[j]))[2];

            *input << "|" << qSetFieldWidth(15)
                   << QString::number(x, 'E', 4)
                   << qSetFieldWidth(0) << "|" << qSetFieldWidth(15)
                   << QString::number(y, 'E', 4)
                   << qSetFieldWidth(0) << "|" << qSetFieldWidth(15)
                   << QString::number(z, 'E', 4)
                   << qSetFieldWidth(0) << "|";

            for (int i = 0; i < data->size(); i++)
                if (doseList->selectedItems().contains(doseList->item(i))) {
                    if (tri->isChecked()) {
                        (*data)[i]->triInterpol(x, y, z, &v, &e);
                    }
                    else {
                        v = (*data)[i]->getDose(x, y, z);
                        e = (*data)[i]->getError(x, y, z);
                    }

                    *input << qSetFieldWidth(15) << QString::number(v, 'E', 4)
                           << qSetFieldWidth(0) << "|" << qSetFieldWidth(15)
                           << QString::number(e*v, 'E', 4)
                           << qSetFieldWidth(0) << "|";
                }

            *input << "\n";
        }

        delete input;
    }
    delete file;

    for (int j = 0; j < points.size(); j++) {
        delete points[j];
    }

    refresh();
    done.setText("File successfully output at " + path);
    done.exec();
}

void Interface::rebinBounds() {
    QFile *file;
    QTextStream *input;

    QString path = QFileDialog::getOpenFileName(0, tr("Open File"), 0,
                   tr("Boundaries File (*.txt)"));

    if (path == 0) {
        return;
    }

    file = new QFile(path);

    QString line;
    QVector <QVector <double> > bounds;
    bounds.resize(3);

    if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        input = new QTextStream(file);

        for (int n = 0; n < 3; n++) {
            line = input->readLine();
            int i = 0, f = 0;
            bounds[n].clear();
            while (line.size() > f) {
                while (!(line.at(i).isDigit() ||
                         line.at(i) == QChar('-'))) {
                    i++;
                }

                f = i;

                while ((line.at(f).isDigit() ||
                        line.at(f) == QChar('.') ||
                        line.at(f) == QChar('-')) &&
                        f < line.size()) {
                    f++;
                }

                bounds[n] += line.mid(i, f-i).toDouble();
                i=f;

                if (f == line.size()) {
                    break;
                }
            }
        }

        delete input;
    }
    delete file;


    for (int i = 0; i < data->size(); i++)
        if (doseList->selectedItems().contains(doseList->item(i))) {
            QString bounds2 = QString("(Xi to Xf, Yi to Yf, Zi to Zf) ") + tr("bounds are:\n(")
                              +QString::number((*data)[i]->cx[0])+" to "+QString::number((*data)[i]->cx[(*data)[i]->x])+ QString(", ")
                              +QString::number((*data)[i]->cy[0])+" to "+QString::number((*data)[i]->cy[(*data)[i]->y])+ QString(", ")
                              +QString::number((*data)[i]->cz[0])+" to "+QString::number((*data)[i]->cz[(*data)[i]->z])+ QString(").");
            for (int j = 0; j < bounds[0].size(); j++) {
                if ((*data)[i]->getIndex("X",bounds[0][j]) < 0) {
                    badInput.setText(tr("Some boundaries in file are ") +
                                     tr("outside of selected ") +
                                     tr("distributions.  ") + bounds2.toLatin1());
                    badInput.exec();
                    return;
                }
                else if (j+1 < bounds[0].size()) {
                    if (bounds[0][j] > bounds[0][j+1]) {
                        badInput.setText(tr("Some boundaries in the file ")+
                                         tr("are not in ascending order."));
                        badInput.exec();
                        return;
                    }
                }
            }
            for (int j = 0; j < bounds[1].size(); j++) {
                if ((*data)[i]->getIndex("Y",bounds[1][j]) < 0) {
                    badInput.setText(tr("Some boundaries in file are ") +
                                     tr("outside of selected ") +
                                     tr("distributions.  ") + bounds2.toLatin1());
                    badInput.exec();
                    return;
                }
                else if (j+1 < bounds[1].size()) {
                    if (bounds[1][j] > bounds[1][j+1]) {
                        badInput.setText(tr("Some boundaries in file are ")+
                                         tr("not in ascending order."));
                        badInput.exec();
                        return;
                    }
                }
            }
            for (int j = 0; j < bounds[2].size(); j++) {
                if ((*data)[i]->getIndex("Z",bounds[2][j]) < 0) {
                    badInput.setText(tr("Some boundaries in file are ") +
                                     tr("outside of selected ") +
                                     tr("distributions.  ") + bounds2.toLatin1());
                    badInput.exec();
                    return;
                }
                else if (j+1 < bounds[2].size()) {
                    if (bounds[2][j] > bounds[2][j+1]) {
                        badInput.setText(tr("Some boundaries in file are ")+
                                         tr("not in ascending order."));
                        badInput.exec();
                        return;
                    }
                }
            }
        }

    for (int d = 0; d < data->size(); d++)
        if (doseList->selectedItems().contains(doseList->item(d))) {
            Dose *temp = new Dose();
            Dose *old = (*data)[d];
            double val, err;

            // Fill in all the new positions in the temp dose
            temp->cx.clear();
            for (int j = 0; j < bounds[0].size(); j++) {
                temp->cx += bounds[0][j];
            }
            temp->x = temp->cx.size()-1;

            temp->cy.clear();
            for (int j = 0; j < bounds[1].size(); j++) {
                temp->cy += bounds[1][j];
            }
            temp->y = temp->cy.size()-1;

            temp->cz.clear();
            for (int j = 0; j < bounds[2].size(); j++) {
                temp->cz += bounds[2][j];
            }
            temp->z = temp->cz.size()-1;

            // These three indeces will always be for the old dose
            int x=0, y=0, z=0;
            QVector <double> tempX, tempY, tempZ;


            // Go through and fill the file with empty doses and errors
            temp->val.resize(temp->x);
            temp->err.resize(temp->x);
            for (int i = 0; i < temp->x; i++) {
                temp->val[i].resize(temp->y);
                temp->err[i].resize(temp->y);
                for (int j = 0; j < temp->y; j++) {
                    temp->val[i][j].resize(temp->z);
                    temp->err[i][j].resize(temp->z);
                    for (int k = 0; k < temp->z; k++) {
                        temp->val[i][j][k] = 0;
                        temp->err[i][j][k] = 0;
                    }
                }
            }

            // Move the indices to point to the first indeces within the new
            // bounds
            while (old->cx[x] < temp->cx[0]) {
                x++;
            }

            while (old->cy[y] < temp->cy[0]) {
                y++;
            }

            while (old->cz[z] < temp->cz[0]) {
                z++;
            }

            for (int i = 0; i < temp->x; i++) {
                // All the x points of any boundaries within the voxel in temp
                // with index i, inclusive
                tempX.clear();
                tempX += temp->cx[i];
                while (old->cx[x] < temp->cx[i+1]) {
                    tempX += old->cx[x++];
                }
                tempX += temp->cx[i+1];

                for (int j = 0; j < temp->y; j++) {
                    // All the y points of any boundaries within the voxel in
                    // temp with index j, inclusive
                    tempY.clear();
                    tempY += temp->cy[j];
                    while (old->cy[y] < temp->cy[j+1]) {
                        tempY += old->cy[y++];
                    }
                    tempY += temp->cy[j+1];

                    for (int k = 0; k < temp->z; k++) {
                        // All the z points of any boundaries within the voxel
                        // in temp with index k, inclusive
                        tempZ.clear();
                        tempZ += temp->cz[k];
                        while (old->cz[z] < temp->cz[k+1]) {
                            tempZ += old->cz[z++];
                        }
                        tempZ += temp->cz[k+1];

                        // Add up all the doses and errors (in quadrature) of
                        // all the inbetween sections weighed by their volume
                        val = err = 0;
                        for (int i2 = 0; i2 < tempX.size()-1; i2++)
                            for (int j2 = 0; j2 < tempY.size()-1; j2++)
                                for (int k2 = 0; k2 < tempZ.size()-1; k2++) {
                                    val += (tempX[i2+1]-tempX[i2])*1.0*
                                           (tempY[j2+1]-tempY[j2])*1.0*
                                           (tempZ[k2+1]-tempZ[k2])*1.0*
                                           old->getDose(
                                               (tempX[i2+1]+tempX[i2])/2.0,
                                               (tempY[j2+1]+tempY[j2])/2.0,
                                               (tempZ[k2+1]+tempZ[k2])/2.0);
                                    err += (tempX[i2+1]-tempX[i2])*1.0*
                                           (tempY[j2+1]-tempY[j2])*1.0*
                                           (tempZ[k2+1]-tempZ[k2])*1.0*
                                           pow(old->getError(
                                                   (tempX[i2+1]+tempX[i2])/2.0,
                                                   (tempY[j2+1]+tempY[j2])/2.0,
                                                   (tempZ[k2+1]+tempZ[k2])/2.0),
                                               2);
                                }

                        // Divide by the total volume (and take the square root
                        // of the error first)
                        val /= (temp->cx[i+1]-temp->cx[i])*1.0*
                               (temp->cy[j+1]-temp->cy[j])*1.0*
                               (temp->cz[k+1]-temp->cz[k]);
                        err = sqrt(err/
                                   ((temp->cx[i+1]-temp->cx[i])*1.0*
                                    (temp->cy[j+1]-temp->cy[j])*1.0*
                                    (temp->cz[k+1]-temp->cz[k])));
                        temp->val[i][j][k] = val;
                        temp->err[i][j][k] = err;
                    }
                }
            }

            temp->filled = old->filled;
            temp->title = old->title;
            (*data)[d] = temp;
            delete old;
        }


    refresh();
    done.setText("Boundaries successfully altered.");
    done.exec();
}

void Interface::translate() {
    bool a = false, b = false, c = false;
    transx->getText().toDouble(&a);
    transy->getText().toDouble(&b);
    transz->getText().toDouble(&c);

    if (a && b && c)
        for (int i = 0; i < data->size(); i++) {
            if (doseList->selectedItems().contains(doseList->item(i))) {
                (*data)[i]->translate(transx->getText().toDouble(),
                                      transy->getText().toDouble(),
                                      transz->getText().toDouble());
            }
            else {
                badInput.setText(tr("The transformation vector input must be 3 ") +
                                 tr("real numbers."));
                badInput.exec();
            }
        }

    refresh();
    done.setText("File translated by (" + transx->getText() + ", "
                 + transy->getText() + ", "
                 + transz->getText() + ").");
    done.exec();
}

void Interface::strip() {
    for (int i = 0; i < data->size(); i++) {
        if (doseList->selectedItems().contains(doseList->item(i))) {
            if ((*data)[i]->x > 2 && (*data)[i]->y > 2 && (*data)[i]->z > 2) {
                (*data)[i]->strip();
            }
            else {
                badInput.setText(doseList->item(i)->text() +
                                 tr(" was not stripped, as it had less ") +
                                 tr("than 3 voxels in a dimension."));
                badInput.exec();
            }
        }
    }

    refresh();
    done.setText("Stripping operation complete.");
    done.exec();
}

void Interface::normalize() {
    // Check the appropriate normalization parameters
    bool flag = false;
    bool a, b, c, d, e, f;
    double x, y, z, xi, yi, zi, xf, yf, zf, norm;

    switch (normBox->currentIndex()+1) {
    case 1:
        a = false;
        factor->getText().toDouble(&a);
        if (!a) { // If the scaling factor is not a real number
            badInput.setText(tr("The scaling factor must be a ") +
                             tr("real number."));
            badInput.exec();
            flag = true;
        }
        break;

    case 2:
        for (int i = 0; i < data->size(); i++)
            if (doseList->selectedItems().contains(doseList->item(i))) {
                a = b = c = false;
                x = normx->getText().toDouble(&a);
                y = normy->getText().toDouble(&b);
                z = normz->getText().toDouble(&c);
                QString bounds = QString("(Xi to Xf, Yi to Yf, Zi to Zf) ") + tr("bounds are:\n(")
                                 +QString::number((*data)[i]->cx[0])+" to "+QString::number((*data)[i]->cx[(*data)[i]->x])+ QString(", ")
                                 +QString::number((*data)[i]->cy[0])+" to "+QString::number((*data)[i]->cy[(*data)[i]->y])+ QString(", ")
                                 +QString::number((*data)[i]->cz[0])+" to "+QString::number((*data)[i]->cz[(*data)[i]->z])+ QString(").");
                // If any of the parameters are not real numbers within
                // the defined dimensions
                if (!a || !b || !c ||
                        (*data)[i]->getIndex("X", x) < 0 ||
                        (*data)[i]->getIndex("Y", y) < 0 ||
                        (*data)[i]->getIndex("Z", z) < 0) {
                    badInput.setText(tr("The point at which to normalize") +
                                     tr(" must be a real point within ") +
                                     tr("all selected files.  ") + bounds.toLatin1());
                    badInput.exec();
                    flag = true;
                    i = data->size();
                }
            }
        break;

    case 3:
        for (int i = 0; i < data->size(); i++)
            if (doseList->selectedItems().contains(doseList->item(i))) {
                if ((*data)[i]->x < 0 || (*data)[i]->y < 0 || (*data)[i]->z < 0) {
                    badInput.setText(tr("The distributions must be non-zero."));
                    badInput.exec();
                    flag = true;
                    i = data->size();
                }
            }
        break;

    case 4:
        for (int i = 0; i < data->size(); i++)
            if (doseList->selectedItems().contains(doseList->item(i))) {
                a = b = c = d = e = f = false;
                xi = normxi->getText().toDouble(&a);
                yi = normyi->getText().toDouble(&b);
                zi = normzi->getText().toDouble(&c);
                xf = normxf->getText().toDouble(&d);
                yf = normyf->getText().toDouble(&e);
                zf = normzf->getText().toDouble(&f);
                // If any of the parameters are not real numbers within
                // the defined dimensions

                QString bounds = QString("(Xi to Xf, Yi to Yf, Zi to Zf) ") + tr("bounds are:\n(")
                                 +QString::number((*data)[i]->cx[0])+" to "+QString::number((*data)[i]->cx[(*data)[i]->x])+ QString(", ")
                                 +QString::number((*data)[i]->cy[0])+" to "+QString::number((*data)[i]->cy[(*data)[i]->y])+ QString(", ")
                                 +QString::number((*data)[i]->cz[0])+" to "+QString::number((*data)[i]->cz[(*data)[i]->z])+ QString(").");
                if (!a || !b || !c || !d || !e || !f ||
                        (*data)[i]->getIndex("X", xi) < 0 ||
                        (*data)[i]->getIndex("Y", yi) < 0 ||
                        (*data)[i]->getIndex("Z", zi) < 0 ||
                        (*data)[i]->getIndex("X", xf) < 0 ||
                        (*data)[i]->getIndex("Y", yf) < 0 ||
                        (*data)[i]->getIndex("Z", zf) < 0) {
                    badInput.setText(tr("The volume within which to ") +
                                     tr("normalize must be a real point ") +
                                     tr("within all selected files.  ") + bounds.toLatin1());
                    badInput.exec();
                    flag = true;
                    i = data->size();
                }
            }
        break;

    case 5:
        for (int i = 0; i < data->size(); i++)
            if (doseList->selectedItems().contains(doseList->item(i))) {
                if ((*data)[i]->x < 0 || (*data)[i]->y < 0 || (*data)[i]->z < 0) {
                    badInput.setText(tr("The distributions must be non-zero."));
                    badInput.exec();
                    flag = true;
                    i = data->size();
                }
            }
        break;

    default:
        flag = true;
        break;
    }

    if (flag) { // If there was an error
        return;    // quit
    }

    for (int i = 0; i < data->size(); i++)
        if (doseList->selectedItems().contains(doseList->item(i))) {
            switch (normBox->currentIndex()+1) {
            case 1:
                (*data)[i]->scale(factor->getText().toDouble());
                break;

            case 2:
                x = normx->getText().toDouble();
                y = normy->getText().toDouble();
                z = normz->getText().toDouble();
                if (tri->isChecked()) {
                    (*data)[i]->scaleAtPoint(x, y, z, 1, true);
                }
                else {                             // Normalize to 1
                    (*data)[i]->scaleAtPoint(x, y, z, 1, false);
                }
                break;

            case 3:
                (*data)[i]->scaleAverage();
                break;

            case 4:
                xi = normxi->getText().toDouble();
                yi = normyi->getText().toDouble();
                zi = normzi->getText().toDouble();
                xf = normxf->getText().toDouble();
                yf = normyf->getText().toDouble();
                zf = normzf->getText().toDouble();
                (*data)[i]->scaleArea(xi, yi, zi, xf, yf, zf);
                break;

            case 5:
                norm = 1.0/(*data)[i]->getMax()*100.0;
                (*data)[i]->scale(norm);
                break;

            default:
                break;
            }
        }

    refresh();
    done.setText("Scaling & Normalization operation complete.");
    done.exec();
}

void Interface::divide() {
    QStringList list;
    Dose *temp = 0;
    bool ok;

    for (int i = 0; i < data->size(); i++) {
        list << (*data)[i]->getTitle();
    }

    QString choice = QInputDialog::getItem((QWidget *)this,
                                           tr("Select Dose Distribution"),
                                           tr("Distribution List"),
                                           list, 0, false, &ok);
    if (!ok) {
        return;
    }

    for (int i = 0; i < data->size(); i++)
        if (!(*data)[i]->getTitle().compare(choice)) {
            temp = new Dose(*((*data)[i]));
            break;
        }

    for (int i = 0; i < data->size(); i++)
        if (doseList->selectedItems().contains(doseList->item(i)))
            if (!(*data)[i]->compareDimensions(temp)) {
                badInput.setText(tr("The dimensions of the phantoms ") +
                                 tr("being divided and the one dividing ") +
                                 tr("do not match."));
                badInput.exec();
                return;
            }

    long int count = 0;
    for (int i = 0; i < data->size(); i++)
        if (doseList->selectedItems().contains(doseList->item(i))) {
            count = (*data)[i]->divideDose(temp);
        }

    delete temp;

    refresh();
    if (count)
        done.setText("Dividing by " + choice + " complete.  " + count +
                     " voxels set to zero after division by zero.");
    else {
        done.setText("Dividing by " + choice + " complete.");
    }
    done.exec();
}

void Interface::subtract() {
    QStringList list;
    Dose *temp = 0;
    bool ok;

    for (int i = 0; i < data->size(); i++) {
        list << (*data)[i]->getTitle();
    }

    QString choice = QInputDialog::getItem((QWidget *)this,
                                           tr("Select Dose Distribution"),
                                           tr("Distribution List"),
                                           list, 0, false, &ok);
    if (!ok) {
        return;
    }

    for (int i = 0; i < data->size(); i++)
        if (!(*data)[i]->getTitle().compare(choice)) {
            temp = new Dose(*((*data)[i]));
            break;
        }

    for (int i = 0; i < data->size(); i++)
        if (doseList->selectedItems().contains(doseList->item(i)))
            if (!(*data)[i]->compareDimensions(temp)) {
                badInput.setText(tr("The dimensions of the phantoms ") +
                                 tr("being subtracted and the one ") +
                                 tr("subtracting do not match."));
                badInput.exec();
                return;
            }

    for (int i = 0; i < data->size(); i++)
        if (doseList->selectedItems().contains(doseList->item(i))) {
            (*data)[i]->subtractDose(temp);
        }

    delete temp;

    refresh();
    done.setText("Subtracting by " + choice + " complete.");
    done.exec();
}

void Interface::add() {
    QStringList list;
    Dose *temp = 0;
    bool ok;

    for (int i = 0; i < data->size(); i++) {
        list << (*data)[i]->getTitle();
    }

    QString choice = QInputDialog::getItem((QWidget *)this,
                                           tr("Select Dose Distribution"),
                                           tr("Distribution List"),
                                           list, 0, false, &ok);
    if (!ok) {
        return;
    }

    for (int i = 0; i < data->size(); i++)
        if (!(*data)[i]->getTitle().compare(choice)) {
            temp = new Dose(*((*data)[i]));
            break;
        }

    for (int i = 0; i < data->size(); i++)
        if (doseList->selectedItems().contains(doseList->item(i)))
            if (!(*data)[i]->compareDimensions(temp)) {
                badInput.setText(tr("The dimensions of the phantoms ") +
                                 tr("being subtracted and the one ") +
                                 tr("subtracting do not match."));
                badInput.exec();
                return;
            }

    for (int i = 0; i < data->size(); i++)
        if (doseList->selectedItems().contains(doseList->item(i))) {
            (*data)[i]->addDose(temp);
        }

    delete temp;

    refresh();
    done.setText("Adding " + choice + " complete.");
    done.exec();
}

void Interface::plotAxis() {
    // Check input
    bool d = false, e = false, f = false, g = false;
    double h = plota->getText().toDouble(&d);
    double u = plotb->getText().toDouble(&e);
    double j = plotci->getText().toDouble(&f);
    double k = plotcf->getText().toDouble(&g);
    QString axis = plotBox->currentText().toUpper().left(1);

    if (!d || !e || !f || !g) {
        badInput.setText(tr("The coordinates of the data must be real ") +
                         tr("numbers."));
        badInput.exec();
        return;
    }
    if (j >= k) {
        badInput.setText(tr("The min is larger than or equal to the max."));
        badInput.exec();
        return;
    }

    for (int i = 0; i < data->size(); i++)
        if (doseList->selectedItems().contains(doseList->item(i))) {
            QString bounds = QString("(Xi to Xf, Yi to Yf, Zi to Zf) ") + tr("bounds are:\n(")
                             +QString::number((*data)[i]->cx[0])+" to "+QString::number((*data)[i]->cx[(*data)[i]->x-1])+ QString(", ")
                             +QString::number((*data)[i]->cy[0])+" to "+QString::number((*data)[i]->cy[(*data)[i]->y-1])+ QString(", ")
                             +QString::number((*data)[i]->cz[0])+" to "+QString::number((*data)[i]->cz[(*data)[i]->z-1])+ QString(").");
            if (!axis.compare("X")) {
                if ((*data)[i]->getIndex("Y", h) < 0 || (*data)[i]->getIndex("Z", u) < 0) {
                    badInput.setText(tr("The parameters of the plot are ") +
                                     tr("outside of the area defined within ") +
                                     tr("the distribution.  ") + bounds.toLatin1());
                    badInput.exec();
                    return;
                }
            }
            else if (!axis.compare("Y")) {
                if ((*data)[i]->getIndex("X", h) < 0 || (*data)[i]->getIndex("Z", u) < 0) {
                    badInput.setText(tr("The parameters of the plot are ") +
                                     tr("outside of the area defined within ") +
                                     tr("the distribution.  ") + bounds.toLatin1());
                    badInput.exec();
                    return;
                }
            }
            else if (!axis.compare("Z")) {
                if ((*data)[i]->getIndex("X", h) < 0 || (*data)[i]->getIndex("Y", u) < 0) {
                    badInput.setText(tr("The parameters of the plot are ") +
                                     tr("outside of the area defined within ") +
                                     tr("the distribution.  ") + bounds.toLatin1());
                    badInput.exec();
                    return;
                }
            }
        }

    QString path = QFileDialog::getSaveFileName(0, tr("Save File "), 0,
                   tr("Comma Separated Values ") +
                   tr("(*.csv)"));
    if (path == "") {
        return;
    }

    if (!path.endsWith(tr(".csv"))) {
        path += tr(".csv");
    }

    QFile *file;
    QTextStream *input;
    file = new QFile(path);

    if (file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        input = new QTextStream(file);
        QString name = path.section("/",path.count("/"),path.count("/"));
        name = name.left(name.size() - (path.endsWith(tr(".csv"))?4:5));
        *input << name << "\n";
        int n = 0;
        for (int i = 0; i < data->size(); i++)
            if (doseList->selectedItems().contains(doseList->item(i))) {
                *input << doseList->item(i)->text() << "\n";
                *input << (*data)[i]->plot(axis, h, u, j, k);
                n++;
            }

        *input << "\n";
        delete input;
    }
    delete file;
}

void Interface::selectEGSFile() {
    QString path =
        QFileDialog::getOpenFileName(0, tr("Open File"), 0,
                                     tr("EGSphant File (*.egsphant *.begsphant)"));
    if (path == "") {
        return;
    }

    egsDVHFile->setText(path);

    // Set up the progress bar
    progress->reset();
    *remainder = 0;
    progWin->setWindowTitle("Importing EGSPhant Data");
    progWin->show();
    progWin->activateWindow();
    progWin->raise();

    connect(egsphant, SIGNAL(progressMade(double)),
            this, SLOT(updateProgress(double)));

    if (path.endsWith(".begsphant")) {
        egsphant->loadbEGSPhantFile(path);
    }
    else {
        egsphant->loadEGSPhantFile(path);
    }

    disconnect(egsphant, SIGNAL(progressMade(double)),
               this, SLOT(updateProgress(double)));

    progWin->hide();

    egsDVHMeds->clear();
    for (int i = 0; i < egsphant->media.size(); i++) {
        egsDVHMeds->insertItem(i, egsphant->media[i]);
    }

    refresh();
}


void Interface::selectStatEGSFile() {
    QString path =
        QFileDialog::getOpenFileName(0, tr("Open File"), 0,
                                     tr("EGSphant File (*.egsphant *.begsphant)"));
    if (path == "") {
        return;
    }

    statEgsFile->setText(path);

    // Set up the progress bar
    progress->reset();
    *remainder = 0;
    progWin->setWindowTitle("Importing EGSPhant Data");
    progWin->show();
    progWin->activateWindow();
    progWin->raise();

    connect(statEgsphant, SIGNAL(progressMade(double)),
            this, SLOT(updateProgress(double)));

    if (path.endsWith(".begsphant")) {
        statEgsphant->loadbEGSPhantFile(path);
    }
    else {
        statEgsphant->loadEGSPhantFile(path);
    }

    disconnect(statEgsphant, SIGNAL(progressMade(double)),
               this, SLOT(updateProgress(double)));

    progWin->hide();

    statEgsMeds->clear();
    for (int i = 0; i < statEgsphant->media.size(); i++) {
        statEgsMeds->insertItem(i, statEgsphant->media[i]);
    }

    refresh();
}

void Interface::plotLine() {
    // Check input
    bool d = false, e = false, f = false, g = false, h = false, i = false;
    double xi = plotxi->getText().toDouble(&d);
    double xf = plotxf->getText().toDouble(&e);
    double yi = plotyi->getText().toDouble(&f);
    double yf = plotyf->getText().toDouble(&g);
    double zi = plotzi->getText().toDouble(&h);
    double zf = plotzf->getText().toDouble(&i);
    int res = plotRes->getText().toInt();
    QString axis = plotBox->currentText().toUpper().left(1);

    if (!d || !e || !f || !g || !h || !i) {
        badInput.setText(tr("The coordinates of the line must be real ") +
                         tr("numbers."));
        badInput.exec();
        return;
    }
    else if (res <= 0) {
        badInput.setText(tr("The resolution must be a positive integer."));
        badInput.exec();
        return;
    }

    for (int i = 0; i < data->size(); i++)
        if (doseList->selectedItems().contains(doseList->item(i))) {
            QString bounds = QString("(Xi to Xf, Yi to Yf, Zi to Zf) ") + tr("bounds are:\n(")
                             +QString::number((*data)[i]->cx[0])+" to "+QString::number((*data)[i]->cx[(*data)[i]->x-1])+ QString(", ")
                             +QString::number((*data)[i]->cy[0])+" to "+QString::number((*data)[i]->cy[(*data)[i]->y-1])+ QString(", ")
                             +QString::number((*data)[i]->cz[0])+" to "+QString::number((*data)[i]->cz[(*data)[i]->z-1])+ QString(").");
            if ((*data)[i]->getIndex("X", xi) < 0 ||
                    (*data)[i]->getIndex("X", xf) < 0 ||
                    (*data)[i]->getIndex("Y", yi) < 0 ||
                    (*data)[i]->getIndex("Y", yf) < 0 ||
                    (*data)[i]->getIndex("Z", zi) < 0 ||
                    (*data)[i]->getIndex("Z", zf) < 0) {
                badInput.setText(tr("The parameters of the plot are ") +
                                 tr("outside of the area defined within ") +
                                 tr("the distribution.  ") + bounds.toLatin1());
                badInput.exec();
                return;
            }
        }

    QString path = QFileDialog::getSaveFileName(0, tr("Save File "), 0,
                   tr("Comma Separated Values ") +
                   tr("(*.csv)"));
    if (path == "") {
        return;
    }

    if (!path.endsWith(tr(".csv"))) {
        path += tr(".csv");
    }

    double xInc, yInc, zInc;
    QString delimiter = ",";
    xInc = (xf-xi)/double(res);
    yInc = (yf-yi)/double(res);
    zInc = (zf-zi)/double(res);

    QFile *file;
    QTextStream *input;
    file = new QFile(path);

    if (file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        input = new QTextStream(file);
        QString name = path.section("/",path.count("/"),path.count("/"));
        name = name.left(name.size() - (path.endsWith(tr(".csv"))?4:5));
        *input << name << "\n";

        int n = 0;
        double val = 0, err = 0;
        for (int i = 0; i < data->size(); i++)
            if (doseList->selectedItems().contains(doseList->item(i))) {

                *input << doseList->item(i)->text() << "\n";
                *input << "x" << delimiter << "y" << delimiter << "z" << delimiter << "dose" << delimiter << "error" << "\n";
                for (int j = 0; j < res; j++) {
                    if (tri->isChecked())
                        (*data)[i]->triInterpol(xi + (xInc * double(j)),
                                                yi + (yInc * double(j)),
                                                zi + (zInc * double(j)),
                                                &val, &err);
                    else {
                        val = (*data)[i]->getDose(xi + (xInc * double(j)),
                                                  yi + (yInc * double(j)),
                                                  zi + (zInc * double(j)));
                        err = (*data)[i]->getError(xi + (xInc * double(j)),
                                                   yi + (yInc * double(j)),
                                                   zi + (zInc * double(j)));
                    }

                    *input << QString::number(xi + (xInc * double(j)), 'E', 4);
                    *input << delimiter;
                    *input << QString::number(yi + (yInc * double(j)), 'E', 4);
                    *input << delimiter;
                    *input << QString::number(zi + (zInc * double(j)), 'E', 4);
                    *input << delimiter;
                    *input << QString::number(val, 'E', 6);
                    *input << delimiter;
                    *input << QString::number(val*err, 'E', 6);
                    *input << "\n";
                }
                n++;
            }

        *input << "\n";
        delete input;
    }
    delete file;
}

void Interface::plotDVH() {
    QString path = QFileDialog::getSaveFileName(0, tr("Save File "), 0,
                   tr("Comma Separated Values ") +
                   tr("(*.csv)"));
    if (path == "") {
        return;
    }

    if (!path.endsWith(tr(".csv"))) {
        path += tr(".csv");
    }

    QFile *file;
    QTextStream *input;

    // Aditional data variables
    QStringList extra;
    double min, eMin, max, eMax, avg, eAvg, err, maxErr, totVol, nVox;
    int dNum = dvhDose->text().count(',')+1,
        vNum = dvhVol->text().count(',')+1;
    if (dvhDose->text().toDouble() <= 0 && dvhDose->text().count(',') == 0) {
        dNum = 0;
    }
    if (dvhVol->text().toDouble() <= 0 && dvhVol->text().count(',') == 0) {
        vNum = 0;
    }
    QVector <double> oDx, oVx;
    QVector <double> Dx, Vx;
    for (int i = 0; i < dNum; i++) {
        oDx += dvhDose->text().section(',', i, i).toDouble();
    }
    for (int i = 0; i < vNum; i++) {
        oVx += dvhVol->text().section(',', i, i).toDouble();
    }
    extra << "File             |"
          << "                 |"
          << "Minimum Dose     |"
          << "Maximum Dose     |"
          << "Average Dose     |"
          << "Maximum Error    |"
          << "Average Error    |"
          << "Total Volume     |"
          << "Number of Voxels |";
    for (int i = 0; i < dNum; i++) {
        extra << QString("D") + QString::number(oDx[i]).leftJustified(16) + "|";
    }
    for (int i = 0; i < vNum; i++) {
        extra << QString("V") + QString::number(oVx[i]).leftJustified(16) + "|";
    }

    file = new QFile(path);

    if (file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        input = new QTextStream(file);
        QString name = path.section("/",path.count("/"),path.count("/"));
        name = name.left(name.size() - (path.endsWith(tr(".csv"))?4:5));
        *input << name << "\n";

        int n = 0;
        for (int i = 0; i < data->size(); i++)
            if (doseList->selectedItems().contains(doseList->item(i))) {
                *input << doseList->item(i)->text() << "\n";

                // Output data text file
                if (dvhExtra->isChecked()) {
                    extra[0] += (*data)[i]->getTitle().
                                leftJustified(29,' ',true) + " |";
                    extra[1] += "Value          Error          |";
                    Dx = oDx;
                    Vx = oVx;
                    *input << (*data)[i]->plot(0, 0, 0, 0, 0, 0, NULL, NULL,
                                               &min, &eMin, &max, &eMax, &avg,
                                               &eAvg, &err, &maxErr, &totVol,
                                               &nVox, &Dx, &Vx);
                    extra[2] += QString::number(min).leftJustified(14) + QString(" ") +
                                QString::number(eMin).leftJustified(14) + " |";
                    extra[3] += QString::number(max).leftJustified(14) + QString(" ") +
                                QString::number(eMax).leftJustified(14) + " |";
                    extra[4] += QString::number(avg).leftJustified(14) + QString(" ") +
                                QString::number(eAvg).leftJustified(14) + " |";
                    extra[5] += QString::number(maxErr).leftJustified(14) + QString(" ")
                                + "               |";
                    extra[6] += QString::number(err).leftJustified(14) + QString(" ")
                                + "               |";
                    extra[7] += QString::number(totVol).leftJustified(14) + QString(" ")
                                + "               |";
                    extra[8] += QString::number(nVox).leftJustified(14) + QString(" ")
                                + "               |";
                    int count = 9;
                    for (int j = 0; j < dNum; j++)
                        extra[count++] +=
                            QString::number(Dx[j]).leftJustified(19) +
                            "           |";
                    for (int j = 0; j < vNum; j++)
                        extra[count++] +=
                            QString::number(Vx[j]).leftJustified(19) +
                            "           |";
                }
                else
                    *input << (*data)[i]->plot(0, 0, 0, 0, 0, 0, NULL, NULL,
                                               &min, &eMin, &max, &eMax, &avg,
                                               &eAvg, &err, &maxErr, &totVol,
                                               &nVox, &Dx, &Vx);

                n++;
            }

        *input << "\n";
        delete input;
    }
    delete file;

    if (dvhExtra->isChecked()) {
        QString path2 = QFileDialog::getSaveFileName(0, tr("Save File"), 0,
                        tr("Additional Statistics (*.txt)"));
        if (path2 == 0) {
            return;
        }

        if (!path2.endsWith(".txt")) {
            path2 += ".txt";
        }

        QFile file2(path2);

        if (file2.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream input2(&file2);

            input2 << "Additional Statistics for DVH over the whole file";
            input2 << "\n\n";

            for (int i = 0; i < extra.size(); i++) {
                input2 << extra[i] << "\n";
            }

            file2.close();
        }
    }
}

void Interface::plotDVHReg() {
    bool a, b, c, d, e, f;
    double xi = 0, yi = 0, zi = 0, xf = 0, yf = 0, zf = 0;

    for (int i = 0; i < data->size(); i++)
        if (doseList->selectedItems().contains(doseList->item(i))) {
            a = b = c = d = e = f = false;
            xi = dvhxi->getText().toDouble(&a);
            yi = dvhyi->getText().toDouble(&b);
            zi = dvhzi->getText().toDouble(&c);
            xf = dvhxf->getText().toDouble(&d);
            yf = dvhyf->getText().toDouble(&e);
            zf = dvhzf->getText().toDouble(&f);

            QString bounds = QString("(Xi to Xf, Yi to Yf, Zi to Zf) ") + tr("bounds are:\n(")
                             +QString::number((*data)[i]->cx[0])+" to "+QString::number((*data)[i]->cx[(*data)[i]->x])+ QString(", ")
                             +QString::number((*data)[i]->cy[0])+" to "+QString::number((*data)[i]->cy[(*data)[i]->y])+ QString(", ")
                             +QString::number((*data)[i]->cz[0])+" to "+QString::number((*data)[i]->cz[(*data)[i]->z])+ QString(").");
            if (!a || !b || !c || !d || !e || !f ||
                    (*data)[i]->getIndex("X", xi) < 0 ||
                    (*data)[i]->getIndex("Y", yi) < 0 ||
                    (*data)[i]->getIndex("Z", zi) < 0 ||
                    (*data)[i]->getIndex("X", xf) < 0 ||
                    (*data)[i]->getIndex("Y", yf) < 0 ||
                    (*data)[i]->getIndex("Z", zf) < 0) {
                badInput.setText(tr("The region within which to create a") +
                                 tr(" DVH must exist within the selected") +
                                 tr(" distributions.  ") + bounds.toLatin1());
                badInput.exec();
                return;
            }
        }

    QString path = QFileDialog::getSaveFileName(0, tr("Save File "), 0,
                   tr("Comma Separated Values ") +
                   tr("(*.csv)"));
    if (path == "") {
        return;
    }

    if (!path.endsWith(tr(".csv"))) {
        path += tr(".csv");
    }

    QFile *file;
    QTextStream *input;
    QStringList extra;
    double min, eMin, max, eMax, avg, eAvg, err, maxErr, totVol, nVox;
    int dNum = dvhDose->text().count(',')+1,
        vNum = dvhVol->text().count(',')+1;
    if (dvhDose->text().toDouble() <= 0 && dvhDose->text().count(',') == 0) {
        dNum = 0;
    }
    if (dvhVol->text().toDouble() <= 0 && dvhVol->text().count(',') == 0) {
        vNum = 0;
    }
    QVector <double> oDx, oVx;
    QVector <double> Dx, Vx;
    for (int i = 0; i < dNum; i++) {
        oDx += dvhDose->text().section(',', i, i).toDouble();
    }
    for (int i = 0; i < vNum; i++) {
        oVx += dvhVol->text().section(',', i, i).toDouble();
    }
    extra << "File             |"
          << "                 |"
          << "Minimum Dose     |"
          << "Maximum Dose     |"
          << "Average Dose     |"
          << "Maximum Error    |"
          << "Average Error    |"
          << "Total Volume     |"
          << "Number of Voxels |";
    for (int i = 0; i < dNum; i++) {
        extra << QString("D") + QString::number(oDx[i]).leftJustified(16) + "|";
    }
    for (int i = 0; i < vNum; i++) {
        extra << QString("V") + QString::number(oVx[i]).leftJustified(16) + "|";
    }

    file = new QFile(path);

    if (file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        input = new QTextStream(file);
        QString name = path.section("/",path.count("/"),path.count("/"));
        name = name.left(name.size() - (path.endsWith(tr(".csv"))?4:5));
        *input << name << "\n";

        int n = 0;
        for (int i = 0; i < data->size(); i++)
            if (doseList->selectedItems().contains(doseList->item(i))) {
                xi = dvhxi->getText().toDouble();
                yi = dvhyi->getText().toDouble();
                zi = dvhzi->getText().toDouble();
                xf = dvhxf->getText().toDouble();
                yf = dvhyf->getText().toDouble();
                zf = dvhzf->getText().toDouble();


                *input << doseList->item(i)->text() << "\n";

                if (dvhExtra->isChecked()) {
                    extra[0] += (*data)[i]->getTitle().
                                leftJustified(29,' ',true) + " |";
                    extra[1] += "Value          Error          |";
                    Dx = oDx;
                    Vx = oVx;
                    *input << (*data)[i]->plot(xi, xf, yi, yf, zi, zf, NULL,
                                               NULL, &min, &eMin, &max, &eMax,
                                               &avg, &eAvg, &err, &maxErr,
                                               &totVol, &nVox, &Dx, &Vx);
                    extra[2] += QString::number(min).leftJustified(14) + QString(" ") +
                                QString::number(eMin).leftJustified(14) + " |";
                    extra[3] += QString::number(max).leftJustified(14) + QString(" ") +
                                QString::number(eMax).leftJustified(14) + " |";
                    extra[4] += QString::number(avg).leftJustified(14) + QString(" ") +
                                QString::number(eAvg).leftJustified(14) + " |";
                    extra[5] += QString::number(maxErr).leftJustified(14) + QString(" ")
                                + "               |";
                    extra[6] += QString::number(err).leftJustified(14) + QString(" ")
                                + "               |";
                    extra[7] += QString::number(totVol).leftJustified(14) + QString(" ")
                                + "               |";
                    extra[8] += QString::number(nVox).leftJustified(14) + QString(" ")
                                + "               |";
                    int count = 9;
                    for (int j = 0; j < dNum; j++)
                        extra[count++] +=
                            QString::number(Dx[j]).leftJustified(19) +
                            "           |";
                    for (int j = 0; j < vNum; j++)
                        extra[count++] +=
                            QString::number(Vx[j]).leftJustified(19) +
                            "           |";
                }
                else
                    *input << (*data)[i]->plot(xi, xf, yi, yf, zi, zf, NULL,
                                               NULL, &min, &eMin, &max, &eMax,
                                               &avg, &eAvg, &err, &maxErr,
                                               &totVol, &nVox, &Dx, &Vx);

                n++;
            }

        *input << "\n";
        delete input;
    }
    delete file;

    if (dvhExtra->isChecked()) {
        QString path2 = QFileDialog::getSaveFileName(0, tr("Save File"), 0,
                        tr("Additional Statistics (*.txt)"));
        if (path2 == 0) {
            return;
        }

        if (!path2.endsWith(".txt")) {
            path2 += ".txt";
        }

        QFile file2(path2);

        if (file2.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream input2(&file2);

            input2 << "Additional Statistics for DVH over the following area:";
            input2 << " (Xi, Yi, Zi) = (" << xi << ", " << yi << ", " << zi
                   << "),";
            input2 << " (Xf, Yf, Zf) = (" << xf << ", " << yf << ", " << zf
                   << ")";
            input2 << "\n\n";

            for (int i = 0; i < extra.size(); i++) {
                input2 << extra[i] << "\n";
            }

            file2.close();
        }
    }
}

void Interface::plotDVHMed() {
    QVector <char> temp;
    for (int i = 0; i < egsDVHMeds->count(); i++)
        if (egsDVHMeds->selectedItems().contains(egsDVHMeds->item(i))) {
            if (i < 9) {
                temp << char(i+49);
            }
            else {
                temp << char(i+56);
            }
        }

    QString path = QFileDialog::getSaveFileName(0, tr("Save File "), 0,
                   tr("Comma Separated Values ") +
                   tr("(*.csv)"));
    if (path == "") {
        return;
    }

    if (!path.endsWith(tr(".csv"))) {
        path += tr(".csv");
    }

    QFile *file;
    QTextStream *input;
    QStringList extra;
    double min, eMin, max, eMax, avg, eAvg, err, maxErr, totVol, nVox;
    int dNum = dvhDose->text().count(',')+1,
        vNum = dvhVol->text().count(',')+1;
    if (dvhDose->text().toDouble() <= 0 && dvhDose->text().count(',') == 0) {
        dNum = 0;
    }
    if (dvhVol->text().toDouble() <= 0 && dvhVol->text().count(',') == 0) {
        vNum = 0;
    }
    QVector <double> oDx, oVx;
    QVector <double> Dx, Vx;
    for (int i = 0; i < dNum; i++) {
        oDx += dvhDose->text().section(',', i, i).toDouble();
    }
    for (int i = 0; i < vNum; i++) {
        oVx += dvhVol->text().section(',', i, i).toDouble();
    }
    extra << "File             |"
          << "                 |"
          << "Minimum Dose     |"
          << "Maximum Dose     |"
          << "Average Dose     |"
          << "Maximum Error    |"
          << "Average Error    |"
          << "Total Volume     |"
          << "Number of Voxels |";
    for (int i = 0; i < dNum; i++) {
        extra << QString("D") + QString::number(oDx[i]).leftJustified(16) + "|";
    }
    for (int i = 0; i < vNum; i++) {
        extra << QString("V") + QString::number(oVx[i]).leftJustified(16) + "|";
    }

    file = new QFile(path);

    if (file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        input = new QTextStream(file);
        QString name = path.section("/",path.count("/"),path.count("/"));
        name = name.left(name.size() - (path.endsWith(tr(".csv"))?4:5));
        *input << name << "\n";

        int n = 0;
        for (int i = 0; i < data->size(); i++)
            if (doseList->selectedItems().contains(doseList->item(i))) {
                *input << doseList->item(i)->text() << "\n";
                if (dvhExtra->isChecked()) {
                    extra[0] += (*data)[i]->getTitle().leftJustified(29,' ',true) + " |";
                    extra[1] += "Value          Error          |";
                    Dx = oDx;
                    Vx = oVx;
                    *input << (*data)[i]->plot(0, 0, 0, 0, 0, 0, egsphant, &temp, &min, &eMin, &max, &eMax, &avg, &eAvg, &err, &maxErr,
                                               &totVol, &nVox, &Dx, &Vx);
                    extra[2] += QString::number(min).leftJustified(14) + QString(" ") + QString::number(eMin).leftJustified(14) + " |";
                    extra[3] += QString::number(max).leftJustified(14) + QString(" ") + QString::number(eMax).leftJustified(14) + " |";
                    extra[4] += QString::number(avg).leftJustified(14) + QString(" ") + QString::number(eAvg).leftJustified(14) + " |";
                    extra[5] += QString::number(maxErr).leftJustified(14) + QString(" ") + "               |";
                    extra[6] += QString::number(err).leftJustified(14) + QString(" ") + "               |";
                    extra[7] += QString::number(totVol).leftJustified(14) + QString(" ") + "               |";
                    extra[8] += QString::number(nVox).leftJustified(14) + QString(" ") + "               |";
                    int count = 9;
                    for (int j = 0; j < dNum; j++) {
                        extra[count++] += QString::number(Dx[j]).leftJustified(19) + "           |";
                    }
                    for (int j = 0; j < vNum; j++) {
                        extra[count++] += QString::number(Vx[j]).leftJustified(19) + "           |";
                    }
                }
                else
                    *input << (*data)[i]->plot(0, 0, 0, 0, 0, 0, egsphant,
                                               &temp, &min, &eMin, &max, &eMax,
                                               &avg, &eAvg, &err, &maxErr,
                                               &totVol, &nVox, &Dx, &Vx);

                n++;
            }

        *input << "\n";
        delete input;
    }
    delete file;

    if (dvhExtra->isChecked()) {
        QString path2 = QFileDialog::getSaveFileName(0, tr("Save File"), 0,
                        tr("Additional Statistics (*.txt)"));
        if (path2 == 0) {
            return;
        }

        if (!path2.endsWith(".txt")) {
            path2 += ".txt";
        }

        QFile file2(path2);

        if (file2.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream input2(&file2);

            input2 << "Additional Statistics for DVH over the following media:";
            for (int i = 0; i < egsDVHMeds->count(); i++)
                if (egsDVHMeds->selectedItems().contains(egsDVHMeds->item(i))) {
                    input2 << " " << egsDVHMeds->item(i)->text();
                }
            input2 << "\n\n";

            for (int i = 0; i < extra.size(); i++) {
                input2 << extra[i] << "\n";
            }

            file2.close();
        }
    }
}

void Interface::plot() {
    if (doseList->selectedItems().size() == 0) {
        return;
    }

    switch (plotBox->currentIndex()) {
    case 0:
    case 1:
    case 2:
        plotAxis();
        break;
    case 3:
        plotLine();;
        break;
    case 4:
        plotDVH();
        break;
    case 5:
        plotDVHReg();
        break;
    case 6:
        plotDVHMed();
        break;
    default:
        break;
    }

    refresh();
}

// The 3 stat functions for different region cases need to be combined,
// the current state is embarrassingly bad
void Interface::statH(Dose *comp) {
    QString path = QFileDialog::getSaveFileName(0, tr("Save File "), 0,
                   tr("Comma Separated Values ") +
                   tr("(*.csv)"));
    if (path == "") {
        return;
    }

    if (!path.endsWith(tr(".csv"))) {
        path += tr(".csv");
    }

    QFile *file;
    QTextStream *input;
    QStringList extra;
    double totVol, nVox, chi, rms, minFinal = 0, maxFinal = 0, min, max, avg,
                                   avg2, avg3, area1, area2;
    Dose orig(*comp);
    Dose temp;
    extra << "File             |"
          << "                 |"
          << "Total Volume     |"
          << "Number of Voxels |"
          << "Chi-Squared      |"
          << "RMS              |";
    if (statOptBox->currentIndex() == 3) // Sum Difference / Error
        extra << "Integral -1 to 1 |"
              << "Integral -2 to 2 |";
    else if (statOptBox->currentIndex() == 7) // Square Difference / Error
        extra << "Integral 0 to 1 |"
              << "Integral 0 to 2 |";

    file = new QFile(path);

    if (file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        input = new QTextStream(file);
        QString name = path.section("/",path.count("/"),path.count("/"));
        name = name.left(name.size() - (path.endsWith(tr(".csv"))?4:5));
        *input << name << "\n";
        switch (statOptBox->currentIndex()) {
        case 0: // Sum Difference
            *input << "difference";
            break;
        case 1: // Sum Difference / Average
            *input << "difference / average";
            break;
        case 2: // Sum Difference / Max
            *input << "difference /";
            break;
        case 3: // Sum Difference / Uncertainty
            *input << "difference / uncertainty";
            break;
        case 4: // Square of Difference
            *input << "square of difference";
            break;
        case 5: // Square of Difference / Average
            *input << "square of difference / average";
            break;
        case 6: // Square of Difference / Max
            *input << "square of difference / max";
            break;
        case 7: // Square of Difference / Variance
            *input << "square of difference / uncertainty";
            break;
        case 8: // Local difference
            *input << "difference / local dose (voxel by voxel)";
            break;
        default:
            break;
        }
        QString delimiter = ",";
        *input << delimiter << "frequency" << "\n";

        if (statAutoBounds->isChecked()) {
            // Determine the largest outer bounds, I end up making a deep copy
            // here, and then again on the next forloop.  Probably worth reimplementing.
            for (int i = 0; i < data->size(); i++)
                if (doseList->selectedItems().contains(doseList->item(i))) {
                    temp.copyDose((*data)[i]);
                    switch (statOptBox->currentIndex()) {
                    case 0: // Sum of Difference
                        temp.subtractDose(&orig);
                        temp.getMinMaxAvg(&min, &max, &avg);
                        break;
                    case 1: // Sum of Difference / Average
                        temp.subtractDose(&orig);
                        (*data)[i]->getMinMaxAvg(&min, &max, &avg2);
                        orig.getMinMaxAvg(&min, &max, &avg3);
                        temp.scale(2.0/(avg2+avg3));
                        temp.getMinMaxAvg(&min, &max, &avg);
                        break;
                    case 2: // Sum of Difference / Max
                        temp.subtractDose(&orig);
                        (*data)[i]->getMinMaxAvg(&min, &max, &avg2);
                        orig.getMinMaxAvg(&min, &max, &avg3);
                        temp.scale(1.0/max);
                        temp.getMinMaxAvg(&min, &max, &avg);
                        break;
                    case 3: // Sum of Difference / Uncertainty
                        temp.subtractDoseWithError(&orig);
                        temp.getMinMaxAvg(&min, &max, &avg);
                        break;
                    case 4: // Square of Difference
                        temp.subtractDose(&orig);
                        temp.square();
                        temp.getMinMaxAvg(&min, &max, &avg);
                        break;
                    case 5: // Square of Difference / Average
                        temp.subtractDose(&orig);
                        (*data)[i]->getMinMaxAvg(&min, &max, &avg2);
                        orig.getMinMaxAvg(&min, &max, &avg3);
                        temp.scale(2.0/(avg2+avg3));
                        temp.square();
                        temp.getMinMaxAvg(&min, &max, &avg);
                        break;
                    case 6: // Square of Difference / Max
                        temp.subtractDose(&orig);
                        (*data)[i]->getMinMaxAvg(&min, &max, &avg2);
                        orig.getMinMaxAvg(&min, &max, &avg3);
                        temp.scale(1.0/max);
                        temp.square();
                        temp.getMinMaxAvg(&min, &max, &avg);
                        break;
                    case 7: // Square of Difference / Uncertainty
                        temp.subtractDoseWithError(&orig);
                        temp.square();
                        temp.getMinMaxAvg(&min, &max, &avg);
                        break;
                    case 8: // Local Difference
                        temp.localDose(&orig);
                        temp.getMinMaxAvg(&min, &max, &avg);
                    default:
                        break;
                    }
                    if (minFinal == maxFinal) {
                        minFinal = min;
                        maxFinal = max;
                    }
                    else {
                        if (min < minFinal) {
                            minFinal = min;
                        }
                        if (max > maxFinal) {
                            maxFinal = max;
                        }
                    }
                }
        }
        else {
            minFinal = statMin->getText().toDouble();
            maxFinal = statMax->getText().toDouble();
        }

        int n = 0;
        for (int i = 0; i < data->size(); i++)
            if (doseList->selectedItems().contains(doseList->item(i))) {
                temp.copyDose((*data)[i]);
                switch (statOptBox->currentIndex()) {
                case 0: // Sum of Difference
                    temp.subtractDose(&orig);
                    break;
                case 1: // Sum of Difference / Average
                    temp.subtractDose(&orig);
                    (*data)[i]->getMinMaxAvg(&min, &max, &avg2);
                    orig.getMinMaxAvg(&min, &max, &avg3);
                    temp.scale(2.0/(avg2+avg3));
                    break;
                case 2: // Sum of Difference / Max
                    temp.subtractDose(&orig);
                    (*data)[i]->getMinMaxAvg(&min, &max, &avg2);
                    orig.getMinMaxAvg(&min, &max, &avg3);
                    temp.scale(1.0/max);
                    break;
                case 3: // Sum of Difference / Uncertainty
                    temp.subtractDoseWithError(&orig);
                    break;
                case 4: // Square of Difference
                    temp.subtractDose(&orig);
                    temp.square();
                    break;
                case 5: // Square of Difference / Average
                    temp.subtractDose(&orig);
                    (*data)[i]->getMinMaxAvg(&min, &max, &avg2);
                    orig.getMinMaxAvg(&min, &max, &avg3);
                    temp.scale(2.0/(avg2+avg3));
                    temp.square();
                    break;
                case 6: // Square of Difference / Max
                    temp.subtractDose(&orig);
                    (*data)[i]->getMinMaxAvg(&min, &max, &avg2);
                    orig.getMinMaxAvg(&min, &max, &avg3);
                    temp.scale(1.0/max);
                    temp.square();
                    break;
                case 7: // Square of Difference / Uncertainty
                    temp.subtractDoseWithError(&orig);
                    temp.square();
                    break;
                case 8: // Local difference
                    temp.localDose(&orig);
                    break;
                default:
                    break;
                }

                *input << (*data)[i]->getTitle() << "\n";
                *input << temp.stat(minFinal, maxFinal,
                                    statBin->getText().toInt(),
                                    0, 0, 0, 0, 0, 0,
                                    NULL, NULL,
                                    &totVol, &nVox, &chi, &rms,
                                    &area1, &area2);

                if (statExtra->isChecked()) {
                    extra[0] += (*data)[i]->getTitle().
                                leftJustified(29,' ',true) + " |";
                    extra[1] += "Value          Error          |";
                    extra[2] += QString::number(totVol).leftJustified(14) + QString(" ")
                                + "               |";
                    extra[3] += QString::number(nVox).leftJustified(14) + QString(" ")
                                + "               |";
                    extra[4] += QString::number(chi).leftJustified(14) + QString(" ")
                                + "               |";
                    extra[5] += QString::number(rms).leftJustified(14) + QString(" ")
                                + "               |";
                    // Difference / Error
                    if (statOptBox->currentIndex() == 3 ||
                            statOptBox->currentIndex() == 7) {
                        extra[6] += QString::number(area1).leftJustified(14)
                                    + QString(" ") + "               |";
                        extra[7] += QString::number(area2).leftJustified(14)
                                    + QString(" ") + "               |";
                    }
                }
                n++;
            }

        delete input;
    }
    delete file;

    if (statExtra->isChecked()) {
        QString path2 = QFileDialog::getSaveFileName(0, tr("Save File"), 0,
                        tr("Additional Statistics (*.txt)"));
        if (path2 == 0) {
            return;
        }

        if (!path2.endsWith(".txt")) {
            path2 += ".txt";
        }

        QFile file2(path2);

        if (file2.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream input2(&file2);

            input2 << "Additional Statistics for Statistical Comparison";
            input2 << "\n\n";

            for (int i = 0; i < extra.size(); i++) {
                input2 << extra[i] << "\n";
            }

            file2.close();
        }
    }
}

void Interface::statHReg(Dose *comp) {
    double xi, xf, yi, yf, zi, zf;
    xi = statxi->getText().toDouble();
    yi = statyi->getText().toDouble();
    zi = statzi->getText().toDouble();
    xf = statxf->getText().toDouble();
    yf = statyf->getText().toDouble();
    zf = statzf->getText().toDouble();

    QString path = QFileDialog::getSaveFileName(0, tr("Save File "), 0,
                   tr("Comma Separated Values ") +
                   tr("(*.csv)"));
    if (path == "") {
        return;
    }

    if (!path.endsWith(tr(".csv"))) {
        path += tr(".csv");
    }

    QFile *file;
    QTextStream *input;
    QStringList extra;
    double totVol, nVox, chi, rms, minFinal = 0, maxFinal = 0, min, max, avg,
                                   avg2, avg3, area1, area2;
    Dose orig(*comp);
    Dose temp;
    extra << "File             |"
          << "                 |"
          << "Total Volume     |"
          << "Number of Voxels |"
          << "Chi-Squared      |"
          << "RMS              |";
    if (statOptBox->currentIndex() == 3) // Sum Difference / Error
        extra << "Integral -1 to 1 |"
              << "Integral -2 to 2 |";
    else if (statOptBox->currentIndex() == 7) // Square Difference / Error
        extra << "Integral 0 to 1 |"
              << "Integral 0 to 2 |";

    file = new QFile(path);

    if (file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        input = new QTextStream(file);
        QString name = path.section("/",path.count("/"),path.count("/"));
        name = name.left(name.size() - (path.endsWith(tr(".csv"))?4:5));
        *input << name << "\n";
        switch (statOptBox->currentIndex()) {
        case 0: // Sum Difference
            *input << "difference";
            break;
        case 1: // Sum Difference / Average
            *input << "difference / average";
            break;
        case 2: // Sum Difference / Max
            *input << "difference /";
            break;
        case 3: // Sum Difference / Uncertainty
            *input << "difference / uncertainty";
            break;
        case 4: // Square of Difference
            *input << "square of difference";
            break;
        case 5: // Square of Difference / Average
            *input << "square of difference / average";
            break;
        case 6: // Square of Difference / Max
            *input << "square of difference / max";
            break;
        case 7: // Square of Difference / Variance
            *input << "square of difference / uncertainty";
            break;
        case 8: // Local difference
            *input << "difference / local dose (voxel by voxel)";
            break;
        default:
            break;
        }
        QString delimiter = ",";
        *input << delimiter << "frequency" << "\n";

        // Determine the largest outer bounds, I end up making a deep copy here,
        // and then again on the next forloop.  I don't know if it would be
        // better to do it all at once and save on processing but waste more
        // memory, or vice versa (I opted for vice-versa)
        for (int i = 0; i < data->size(); i++)
            if (doseList->selectedItems().contains(doseList->item(i))) {
                temp.copyDose((*data)[i]);
                switch (statOptBox->currentIndex()) {
                case 0: // Sum of Difference
                    temp.subtractDose(&orig);
                    temp.getMinMaxAvg(&min, &max, &avg);
                    break;
                case 1: // Sum of Difference / Average
                    temp.subtractDose(&orig);
                    (*data)[i]->getMinMaxAvg(&min, &max, &avg2);
                    orig.getMinMaxAvg(&min, &max, &avg3);
                    temp.scale(2.0/(avg2+avg3));
                    temp.getMinMaxAvg(&min, &max, &avg);
                    break;
                case 2: // Sum of Difference / Max
                    temp.subtractDose(&orig);
                    (*data)[i]->getMinMaxAvg(&min, &max, &avg2);
                    orig.getMinMaxAvg(&min, &max, &avg3);
                    temp.scale(1.0/max);
                    temp.getMinMaxAvg(&min, &max, &avg);
                    break;
                case 3: // Sum of Difference / Uncertainty
                    temp.subtractDoseWithError(&orig);
                    temp.getMinMaxAvg(&min, &max, &avg);
                    break;
                case 4: // Square of Difference
                    temp.subtractDose(&orig);
                    temp.square();
                    temp.getMinMaxAvg(&min, &max, &avg);
                    break;
                case 5: // Square of Difference / Average
                    temp.subtractDose(&orig);
                    (*data)[i]->getMinMaxAvg(&min, &max, &avg2);
                    orig.getMinMaxAvg(&min, &max, &avg3);
                    temp.scale(2.0/(avg2+avg3));
                    temp.square();
                    temp.getMinMaxAvg(&min, &max, &avg);
                    break;
                case 6: // Square of Difference / Max
                    temp.subtractDose(&orig);
                    (*data)[i]->getMinMaxAvg(&min, &max, &avg2);
                    orig.getMinMaxAvg(&min, &max, &avg3);
                    temp.scale(1.0/max);
                    temp.square();
                    temp.getMinMaxAvg(&min, &max, &avg);
                    break;
                case 7: // Square of Difference / Uncertainty
                    temp.subtractDoseWithError(&orig);
                    temp.square();
                    temp.getMinMaxAvg(&min, &max, &avg);
                    break;
                case 8: // Local difference
                    temp.localDose(&orig);
                    temp.getMinMaxAvg(&min, &max, &avg);
                    break;
                default:
                    break;
                }
                if (minFinal == maxFinal) {
                    minFinal = min;
                    maxFinal = max;
                }
                else {
                    if (min < minFinal) {
                        minFinal = min;
                    }
                    if (max > maxFinal) {
                        maxFinal = max;
                    }
                }
            }

        int n = 0;
        for (int i = 0; i < data->size(); i++)
            if (doseList->selectedItems().contains(doseList->item(i))) {
                temp.copyDose((*data)[i]);
                switch (statOptBox->currentIndex()) {
                case 0: // Sum of Difference
                    temp.subtractDose(&orig);
                    break;
                case 1: // Sum of Difference / Average
                    temp.subtractDose(&orig);
                    (*data)[i]->getMinMaxAvg(&min, &max, &avg2);
                    orig.getMinMaxAvg(&min, &max, &avg3);
                    temp.scale(2.0/(avg2+avg3));
                    break;
                case 2: // Sum of Difference / Max
                    temp.subtractDose(&orig);
                    (*data)[i]->getMinMaxAvg(&min, &max, &avg2);
                    orig.getMinMaxAvg(&min, &max, &avg3);
                    temp.scale(1.0/max);
                    break;
                case 3: // Sum of Difference / Uncertainty
                    temp.subtractDoseWithError(&orig);
                    break;
                case 4: // Square of Difference
                    temp.subtractDose(&orig);
                    temp.square();
                    break;
                case 5: // Square of Difference / Average
                    temp.subtractDose(&orig);
                    (*data)[i]->getMinMaxAvg(&min, &max, &avg2);
                    orig.getMinMaxAvg(&min, &max, &avg3);
                    temp.scale(2.0/(avg2+avg3));
                    temp.square();
                    break;
                case 6: // Square of Difference / Max
                    temp.subtractDose(&orig);
                    (*data)[i]->getMinMaxAvg(&min, &max, &avg2);
                    orig.getMinMaxAvg(&min, &max, &avg3);
                    temp.scale(1.0/max);
                    temp.square();
                    break;
                case 7: // Square of Difference / Uncertainty
                    temp.subtractDoseWithError(&orig);
                    temp.square();
                    break;
                case 8: // Local Difference
                    temp.localDose(&orig);
                    break;
                default:
                    break;
                }

                *input << (*data)[i]->getTitle() << "\n";
                *input << temp.stat(minFinal, maxFinal,
                                    statBin->getText().toInt(),
                                    xi, xf, yi, yf, zi, zf,
                                    NULL, NULL,
                                    &totVol, &nVox, &chi, &rms,
                                    &area1, &area2);

                if (statExtra->isChecked()) {
                    extra[0] += (*data)[i]->getTitle().
                                leftJustified(29,' ',true) + " |";
                    extra[1] += "Value          Error          |";
                    extra[2] += QString::number(totVol).leftJustified(14) + QString(" ")
                                + "               |";
                    extra[3] += QString::number(nVox).leftJustified(14) + QString(" ")
                                + "               |";
                    extra[4] += QString::number(chi).leftJustified(14) + QString(" ")
                                + "               |";
                    extra[5] += QString::number(rms).leftJustified(14) + QString(" ")
                                + "               |";
                    // Difference / Error
                    if (statOptBox->currentIndex() == 3 ||
                            statOptBox->currentIndex() == 7) {
                        extra[6] += QString::number(area1).leftJustified(14)
                                    + QString(" ") + "               |";
                        extra[7] += QString::number(area2).leftJustified(14)
                                    + QString(" ") + "               |";
                    }
                }
                n++;
            }

        delete input;
    }
    delete file;

    if (statExtra->isChecked()) {
        QString path2 = QFileDialog::getSaveFileName(0, tr("Save File"), 0,
                        tr("Additional Statistics (*.txt)"));
        if (path2 == 0) {
            return;
        }

        if (!path2.endsWith(".txt")) {
            path2 += ".txt";
        }

        QFile file2(path2);

        if (file2.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream input2(&file2);

            input2 << "Additional Statistics for Statistical Comparison over"
                   << " the following area:";
            input2 << " (Xi, Yi, Zi) = (" << xi << ", " << yi << ", " << zi
                   << "),";
            input2 << " (Xf, Yf, Zf) = (" << xf << ", " << yf << ", " << zf
                   << ")";
            input2 << "\n\n";

            for (int i = 0; i < extra.size(); i++) {
                input2 << extra[i] << "\n";
            }

            file2.close();
        }
    }
}

void Interface::statHMed(Dose *comp) {
    QVector <char> tempM;
    for (int i = 0; i < statEgsMeds->count(); i++)
        if (statEgsMeds->selectedItems().contains(statEgsMeds->item(i))) {
            if (i < 9) {
                tempM << char(i+49);
            }
            else {
                tempM << char(i+56);
            }
        }

    QString path = QFileDialog::getSaveFileName(0, tr("Save File "), 0,
                   tr("Comma Separated values ") +
                   tr("(*.csv)"));
    if (path == "") {
        return;
    }

    if (!path.endsWith(tr(".csv"))) {
        path += tr(".csv");
    }

    QFile *file;
    QTextStream *input;
    QStringList extra;
    double totVol, nVox, chi, rms, minFinal = 0, maxFinal = 0, min, max, avg,
                                   avg2, avg3, area1, area2;
    Dose orig(*comp);
    Dose temp;
    extra << "File             |"
          << "                 |"
          << "Total Volume     |"
          << "Number of Voxels |"
          << "Chi-Squared      |"
          << "RMS              |";
    if (statOptBox->currentIndex() == 3) // Sum Difference / Error
        extra << "Integral -1 to 1 |"
              << "Integral -2 to 2 |";
    else if (statOptBox->currentIndex() == 7) // Square Difference / Error
        extra << "Integral 0 to 1 |"
              << "Integral 0 to 2 |";

    file = new QFile(path);

    if (file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        input = new QTextStream(file);
        QString name = path.section("/",path.count("/"),path.count("/"));
        name = name.left(name.size() - (path.endsWith(tr(".csv"))?4:5));
        *input << name << "\n";
        switch (statOptBox->currentIndex()) {
        case 0: // Sum Difference
            *input << "difference";
            break;
        case 1: // Sum Difference / Average
            *input << "difference / average";
            break;
        case 2: // Sum Difference / Max
            *input << "difference /";
            break;
        case 3: // Sum Difference / Uncertainty
            *input << "difference / uncertainty";
            break;
        case 4: // Square of Difference
            *input << "square of difference";
            break;
        case 5: // Square of Difference / Average
            *input << "square of difference / average";
            break;
        case 6: // Square of Difference / Max
            *input << "square of difference / max";
            break;
        case 7: // Square of Difference / Variance
            *input << "square of difference / uncertainty";
            break;
        case 8: // Local difference
            *input << "difference / local dose (voxel by voxel)";
            break;
        default:
            break;
        }
        QString delimiter = ",";
        *input << delimiter << "frequency" << "\n";

        // Determine the largest outer bounds, I end up making a deep copy here,
        // and then again on the next forloop.  I don't know if it would be
        // better to do it all at once and save on processing but waste more
        // memory, or vice versa (I opted for vice-versa)
        for (int i = 0; i < data->size(); i++)
            if (doseList->selectedItems().contains(doseList->item(i))) {
                temp.copyDose((*data)[i]);
                switch (statOptBox->currentIndex()) {
                case 0: // Sum of Difference
                    temp.subtractDose(&orig);
                    temp.getMinMaxAvg(&min, &max, &avg);
                    break;
                case 1: // Sum of Difference / Average
                    temp.subtractDose(&orig);
                    (*data)[i]->getMinMaxAvg(&min, &max, &avg2);
                    orig.getMinMaxAvg(&min, &max, &avg3);
                    temp.scale(2.0/(avg2+avg3));
                    temp.getMinMaxAvg(&min, &max, &avg);
                    break;
                case 2: // Sum of Difference / Max
                    temp.subtractDose(&orig);
                    (*data)[i]->getMinMaxAvg(&min, &max, &avg2);
                    orig.getMinMaxAvg(&min, &max, &avg3);
                    temp.scale(1.0/max);
                    temp.getMinMaxAvg(&min, &max, &avg);
                    break;
                case 3: // Sum of Difference / Uncertainty
                    temp.subtractDoseWithError(&orig);
                    temp.getMinMaxAvg(&min, &max, &avg);
                    break;
                case 4: // Square of Difference
                    temp.subtractDose(&orig);
                    temp.square();
                    temp.getMinMaxAvg(&min, &max, &avg);
                    break;
                case 5: // Square of Difference / Average
                    temp.subtractDose(&orig);
                    (*data)[i]->getMinMaxAvg(&min, &max, &avg2);
                    orig.getMinMaxAvg(&min, &max, &avg3);
                    temp.scale(2.0/(avg2+avg3));
                    temp.square();
                    temp.getMinMaxAvg(&min, &max, &avg);
                    break;
                case 6: // Square of Difference / Max
                    temp.subtractDose(&orig);
                    (*data)[i]->getMinMaxAvg(&min, &max, &avg2);
                    orig.getMinMaxAvg(&min, &max, &avg3);
                    temp.scale(1.0/max);
                    temp.square();
                    temp.getMinMaxAvg(&min, &max, &avg);
                    break;
                case 7: // Square of Difference / Uncertainty
                    temp.subtractDoseWithError(&orig);
                    temp.square();
                    temp.getMinMaxAvg(&min, &max, &avg);
                    break;
                case 8: // Local difference
                    temp.localDose(&orig);
                    temp.getMinMaxAvg(&min, &max, &avg);
                    break;
                default:
                    break;
                }
                if (minFinal == maxFinal) {
                    minFinal = min;
                    maxFinal = max;
                }
                else {
                    if (min < minFinal) {
                        minFinal = min;
                    }
                    if (max > maxFinal) {
                        maxFinal = max;
                    }
                }
            }

        int n = 0;
        for (int i = 0; i < data->size(); i++)
            if (doseList->selectedItems().contains(doseList->item(i))) {
                temp.copyDose((*data)[i]);
                switch (statOptBox->currentIndex()) {
                case 0: // Sum of Difference
                    temp.subtractDose(&orig);
                    break;
                case 1: // Sum of Difference / Average
                    temp.subtractDose(&orig);
                    (*data)[i]->getMinMaxAvg(&min, &max, &avg2);
                    orig.getMinMaxAvg(&min, &max, &avg3);
                    temp.scale(2.0/(avg2+avg3));
                    break;
                case 2: // Sum of Difference / Max
                    temp.subtractDose(&orig);
                    (*data)[i]->getMinMaxAvg(&min, &max, &avg2);
                    orig.getMinMaxAvg(&min, &max, &avg3);
                    temp.scale(1.0/max);
                    break;
                case 3: // Sum of Difference / Uncertainty
                    temp.subtractDoseWithError(&orig);
                    break;
                case 4: // Square of Difference
                    temp.subtractDose(&orig);
                    temp.square();
                    break;
                case 5: // Square of Difference / Average
                    temp.subtractDose(&orig);
                    (*data)[i]->getMinMaxAvg(&min, &max, &avg2);
                    orig.getMinMaxAvg(&min, &max, &avg3);
                    temp.scale(2.0/(avg2+avg3));
                    temp.square();
                    break;
                case 6: // Square of Difference / Max
                    temp.subtractDose(&orig);
                    (*data)[i]->getMinMaxAvg(&min, &max, &avg2);
                    orig.getMinMaxAvg(&min, &max, &avg3);
                    temp.scale(1.0/max);
                    temp.square();
                    break;
                case 7: // Square of Difference / Uncertainty
                    temp.subtractDoseWithError(&orig);
                    temp.square();
                    break;
                case 8: // Local difference
                    temp.localDose(&orig);
                    break;
                default:
                    break;
                }

                *input << (*data)[i]->getTitle() << "\n";

                *input << temp.stat(minFinal, maxFinal,
                                    statBin->getText().toInt(),
                                    0, 0, 0, 0, 0, 0,
                                    statEgsphant, &tempM,
                                    &totVol, &nVox, &chi, &rms,
                                    &area1, &area2);

                if (statExtra->isChecked()) {
                    extra[0] += (*data)[i]->getTitle().
                                leftJustified(29,' ',true) + " |";
                    extra[1] += "Value          Error          |";
                    extra[2] += QString::number(totVol).leftJustified(14) + QString(" ")
                                + "               |";
                    extra[3] += QString::number(nVox).leftJustified(14) + QString(" ")
                                + "               |";
                    extra[4] += QString::number(chi).leftJustified(14) + QString(" ")
                                + "               |";
                    extra[5] += QString::number(rms).leftJustified(14) + QString(" ")
                                + "               |";
                    // Difference / Error
                    if (statOptBox->currentIndex() == 3 ||
                            statOptBox->currentIndex() == 7) {
                        extra[6] += QString::number(area1).leftJustified(14)
                                    + QString(" ") + "               |";
                        extra[7] += QString::number(area2).leftJustified(14)
                                    + QString(" ") + "               |";
                    }
                }
                n++;
            }

        delete input;
    }
    delete file;

    if (statExtra->isChecked()) {
        QString path2 = QFileDialog::getSaveFileName(0, tr("Save File"), 0,
                        tr("Additional Statistics (*.txt)"));
        if (path2 == 0) {
            return;
        }

        if (!path2.endsWith(".txt")) {
            path2 += ".txt";
        }

        QFile file2(path2);

        if (file2.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream input2(&file2);

            input2 << "Additional Statistics for Statistical Comparison over"
                   << " the following media:";
            for (int i = 0; i < statEgsMeds->count(); i++)
                if (statEgsMeds->selectedItems().contains(statEgsMeds->item(i))) {
                    input2 << " " << statEgsMeds->item(i)->text();
                }
            input2 << "\n\n";

            for (int i = 0; i < extra.size(); i++) {
                input2 << extra[i] << "\n";
            }

            file2.close();
        }
    }
}

void Interface::stat() {
    if (doseList->selectedItems().size() == 0) {
        return;
    }

    if (statBin->getText().toInt() <= 0) {
        badInput.setText(tr("The number of bins must be an integer ")+
                         tr("greater than zero."));
        badInput.exec();
        return;
    }

    QStringList list;
    Dose *temp = 0;
    bool ok;

    for (int i = 0; i < data->size(); i++) {
        list << (*data)[i]->getTitle();
    }

    QString choice = QInputDialog::getItem((QWidget *)this,
                                           tr("Select Dose Distribution"),
                                           tr("Distribution List"),
                                           list, 0, false, &ok);
    if (!ok) {
        return;
    }



    for (int i = 0; i < data->size(); i++)
        if (!(*data)[i]->getTitle().compare(choice)) {
            temp = new Dose(*((*data)[i]));
            break;
        }

    for (int i = 0; i < data->size(); i++)
        if (doseList->selectedItems().contains(doseList->item(i)))
            if (!(*data)[i]->compareDimensions(temp)) {
                badInput.setText(tr("The dimensions of the distributions ")+
                                 tr("being compared do not match."));
                badInput.exec();
                return;
            }

    switch (statBox->currentIndex()) {
    case 0:
        statH(temp);
        break;
    case 1:
        statHReg(temp);
        break;
    case 2:
        statHMed(temp);
        break;
    default:
        break;
    }

    refresh();
}

// Retired functionality
//void Interface::Grace(QString path) {
//    QProcess *grace = new QProcess();
//
//    connect(grace, SIGNAL(finished(int, QProcess::ExitStatus)),
//            this, SLOT(doneGrace()));
//
//    grace->start(tr("xmgrace ") + path);
//
//    refresh();
//}
//
//void Interface::doneGrace() {
//    QProcess *grace = (QProcess *)sender();
//
//    grace->terminate();
//    delete grace;
//
//    refresh();
//}

void Interface::updateProgress(double n) {
    // The flooring function rounds down a real number to the nearest integer
    // In this line, we remove the remainder from the total number
    *remainder += n - floor(n);

    // Next we increment the progress bar by all of our whole numbers
    progress->setValue(int(progress->value() + floor(n) + floor(*remainder)));

    // We redraw the bar
    progress->update();
    QApplication::processEvents();

    // And if our remainder makes a whole number, remove it
    *remainder -= floor(*remainder);
}

/*******************************************************************************
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Creating The Window~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*******************************************************************************/
void Interface::createLayout() {
    previewer = new Previewer(this, data);
    previewer->setDisabled(true);

    //QString style;
    //style  = "QWidget {";
    //style += "background-color: rgb(240, 240, 240);";
    //style += "}";
    //style += "QLineEdit";
    //style += "{";
    //style += "background-color: rgb(250, 250, 255)";
    //style += "}";
    //style += "QCheckBox::indicator:unchecked  {";
    //style += "width: 9px;";
    //style += "height: 9px;";
    //style += "background-color: rgb(250, 250, 255);";
    //style += "border-top: 1px solid black;";
    //style += "border-left: 1px solid black;";
    //style += "border-right: 1px solid rgb(160, 160, 160);";
    //style += "border-bottom: 1px solid rgb(160, 160, 160);";
    //style += "}";
    //style += "QCheckBox::indicator:checked  {";
    //style += "width: 11px;";
    //style += "height: 11px;";
    //style += "}";
    //style += "QListWidget";
    //style += "{";
    //style += "background-color: rgb(250, 250, 255);";
    //style += "}";
    //style += "QToolTip {";
    //style += "background-color: rgb(240, 240, 240);";
    //style += "color: rgb(0, 0, 0);";
    //style += "}";

    // Setup the Banner
    logo = new QLabel();
    QPixmap ddt_logo((const char **) ddt_logo_xpm);
    logo->setPixmap(ddt_logo);
    logo->setAlignment(Qt::AlignCenter);
    //logo = new QLabel ("3ddose\n  tools");
    //logo->setFont(QFont ("Serif", 36, QFont::Bold, false));
    //logo->setAlignment(Qt::AlignCenter);


    tri = new QCheckBox("Interpolation");
    tri->setToolTip(tr("When this option is checked, dose at points and\n") +
                    tr("scale to point will use trilinear interpolation\n") +
                    tr("to determine dose at a point."));
    tri->setChecked(true);
    doseList = new QListWidget();
    doseList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    doseLayout = new QGridLayout();
    doseLayout->addWidget(tri, 0, 0);
    doseLayout->addWidget(doseList, 1, 0);
    doseFrame = new QGroupBox();
    doseFrame->setLayout(doseLayout);
    doseFrame->setToolTip(tr("This panel lists all the distributions loaded\n")+
                          tr("into memory. Only selected files will be\n") +
                          tr("modified by the tools."));

    // 3ddose Widget Setup
    iButton = new QPushButton("Load");
    oButton = new QPushButton("Save");
    cButton = new QPushButton("Copy");
    rButton = new QPushButton("Rename");
    dButton = new QPushButton("Remove");
    ioLayout = new QGridLayout();
    ioLayout->addWidget(iButton, 0, 0, 1, 2);
    ioLayout->addWidget(oButton, 0, 2, 1, 2);
    ioLayout->addWidget(cButton, 1, 0, 1, 2);
    ioLayout->addWidget(rButton, 1, 2, 1, 2);
    ioLayout->addWidget(dButton, 2, 0, 1, 2);
    ioFrame = new QGroupBox("3ddose Distributions");
    ioFrame->setLayout(ioLayout);
    ioFrame->setToolTip(tr("This panel allows the user to load new 3ddose\n") +
                        tr("files, as well as save, copy, rename and delete\n")+
                        tr("3ddose files already loaded into memory."));

    // Normalization Widget Setup
    normItems = new QStringList();
    *normItems << tr("scale") << tr("to point") << tr("to average")
               << tr("to max within volume") << tr("to percent of max");
    normBox = new QComboBox();
    normBox->addItems(*normItems);
    normButton = new QPushButton("Scale");
    normStack = new QStackedLayout();
    normLayout = new QGridLayout();
    factor = new LineInput(0, "Factor", "1.0");
    scaleLayout = new QGridLayout();
    scaleLayout->addWidget(factor, 0, 0);
    scaleLayout->setRowStretch(1,20);
    normx = new LineInput(0, "x", "0.0");
    normy = new LineInput(0, "y", "0.0");
    normz = new LineInput(0, "z", "0.0");
    scaleWidget = new QWidget();
    scaleWidget->setLayout(scaleLayout);
    scaleWidget->setToolTip(tr("This mode scales all the dose by factor."));
    pointLayout = new QGridLayout();
    pointLayout->addWidget(normx, 0, 0);
    pointLayout->addWidget(normy, 1, 0);
    pointLayout->addWidget(normz, 2, 0);
    pointLayout->setRowStretch(1,20);
    pointWidget = new QWidget();
    pointWidget->setLayout(pointLayout);
    pointWidget->setToolTip(tr("This mode normalizes all the dose such\n") +
                            tr("that the dose at the point defined is 1") +
                            tr(" (can interpolate)."));
    normxi = new LineInput(0, "Initial x", "-1.0");
    normyi = new LineInput(0, "Initial y", "-1.0");
    normzi = new LineInput(0, "Initial z", "-1.0");
    normxf = new LineInput(0, "Final x", "1.0");
    normyf = new LineInput(0, "Final y", "1.0");
    normzf = new LineInput(0, "Final z", "1.0");
    lineLayout = new QGridLayout();
    lineLayout->addWidget(normxi, 0, 0);
    lineLayout->addWidget(normyi, 0, 1);
    lineLayout->addWidget(normzi, 0, 2);
    lineLayout->addWidget(normxf, 1, 0);
    lineLayout->addWidget(normyf, 1, 1);
    lineLayout->addWidget(normzf, 1, 2);
    lineWidget = new QWidget();
    lineWidget->setLayout(lineLayout);
    lineWidget->setToolTip(tr("This mode normalizes the distribution such\n") +
                           tr("that the highest dose in a voxel within the\n") +
                           tr("selected region is 1."));
    aveWidget = new QWidget();
    aveWidget->setToolTip(tr("This mode normalizes the distribution to\n") +
                          tr("the average dose of the distribution."));
    perWidget = new QWidget();
    perWidget->setToolTip(tr("This mode normalizes the distribution to\n") +
                          tr("a percentage of the max dose of the distribution."));
    normStack->addWidget(scaleWidget);
    normStack->addWidget(pointWidget);
    normStack->addWidget(aveWidget);
    normStack->addWidget(lineWidget);
    normStack->addWidget(perWidget);
    normLayout->addWidget(normButton, 0, 0);
    normLayout->addWidget(normBox, 0, 1);
    normLayout->addLayout(normStack, 1, 0, 1, 2);
    normFrame = new QGroupBox("Scaling && Normalization");
    normFrame->setLayout(normLayout);

    // Translation Widget Setup
    transButton = new QPushButton("Translate");
    transx = new LineInput(0, "x", "0.0");
    transy = new LineInput(0, "y", "0.0");
    transz = new LineInput(0, "z", "0.0");
    transLayout = new QGridLayout();
    transLayout->addWidget(transButton, 0, 0, 1, 3);
    transLayout->addWidget(transx, 1, 0);
    transLayout->addWidget(transy, 1, 1);
    transLayout->addWidget(transz, 1, 2);
    transFrame = new QGroupBox("Translate Coordinate System");
    transFrame->setLayout(transLayout);
    transFrame->setToolTip(tr("Translate your coordinate system by the\n") +
                           tr("parameters defined below."));

    // Statistical Comparison Widget Setup
    statButton = new QPushButton("Compare");
    statItems = new QStringList();
    *statItems << "Total Comparison" << "Regional Comparison"
               << "Media Comparison";
    statBox = new QComboBox();
    statBox->addItems(*statItems);
    statOptItems = new QStringList();
    *statOptItems << "Difference"
                  << "Difference / Average"
                  << "Difference / Max"
                  << "Difference / Uncertainty"
                  << "Square of Difference"
                  << "Square of Difference / Average"
                  << "Square of Difference / Max"
                  << "Square of Difference / Uncertainty"
                  << "Difference / (local) Dose";
    statBin = new LineInput(0, "Bins", "100");
    statMin = new LineInput(0, "Min", "-50");
    statMax = new LineInput(0, "Max", "50");
    statOptBox = new QComboBox();
    statOptBox->addItems(*statOptItems);
    statAutoBounds = new QCheckBox("Automatically Calculate Boundaries");
    statAutoBounds->setToolTip(tr("Set the boundaries equal to the minimum ") +
                               tr("and maximum values calculated."));
    statExtra = new QCheckBox("Output Additional Data");
    statExtra->setToolTip(tr("Output additional information."));
    statLayout = new QGridLayout();
    statLayout->addWidget(statButton, 0, 0);
    statLayout->addWidget(statBox, 0, 1, 1, 2);
    statLayout->addWidget(statOptBox, 1, 0, 1, 3);
    statTotWidget = new QWidget();
    statTotLayout = new QGridLayout();
    statTotWidget->setLayout(statTotLayout);
    statxi = new LineInput(0, "Initial x", "-1.0");
    statyi = new LineInput(0, "Initial y", "-1.0");
    statzi = new LineInput(0, "Initial z", "-1.0");
    statxf = new LineInput(0, "Final x", "1.0");
    statyf = new LineInput(0, "Final y", "1.0");
    statzf = new LineInput(0, "Final z", "1.0");
    statRegLayout = new QGridLayout();
    statRegLayout->addWidget(statxi, 0, 0, 1, 4);
    statRegLayout->addWidget(statyi, 0, 4, 1, 4);
    statRegLayout->addWidget(statzi, 0, 8, 1, 4);
    statRegLayout->addWidget(statxf, 1, 0, 1, 4);
    statRegLayout->addWidget(statyf, 1, 4, 1, 4);
    statRegLayout->addWidget(statzf, 1, 8, 1, 4);
    statRegLayout->setRowStretch(2,20);
    statRegWidget = new QWidget();
    statRegWidget->setLayout(statRegLayout);
    statEgsFile = new QLineEdit("EGSPhant File");
    statEgsFile->setReadOnly(true);
    statEgsBrowse = new QPushButton("Browse");
    statEgsMeds = new QListWidget();
    statEgsLayout = new QGridLayout();
    statEgsphant = new EGSPhant();
    statEgsMeds->setSelectionMode(QAbstractItemView::ExtendedSelection);
    statEgsLayout->addWidget(statEgsBrowse, 0, 0, 1, 2);
    statEgsLayout->addWidget(statEgsFile, 0, 2, 1, 4);
    statEgsLayout->addWidget(statEgsMeds, 1, 0, 1, 6);
    statEgsWidget = new QWidget();
    statEgsWidget->setLayout(statEgsLayout);
    statStack = new QStackedLayout();
    statStack->addWidget(statTotWidget);
    statStack->addWidget(statRegWidget);
    statStack->addWidget(statEgsWidget);
    statLayout->addLayout(statStack, 2, 0, 1, 3);
    statLayout->addWidget(statAutoBounds, 3, 0, 1, 3);
    statLayout->addWidget(statBin, 4, 0);
    statLayout->addWidget(statMin, 4, 1);
    statLayout->addWidget(statMax, 4, 2);
    statLayout->addWidget(statExtra, 5, 0, 1, 3);
    statFrame = new QGroupBox("Statistical Comparison");
    statFrame->setLayout(statLayout);
    statFrame->setToolTip(tr("Create a comma separated value histogram."));

    // Plotting Widget Setup
    plotButton = new QPushButton("Plot");
    plotItems = new QStringList();
    *plotItems << "x axis" << "y axis" << "z axis" << "Free" << "Total DVH"
               << "Regional DVH" << "Media DVH";
    plotBox = new QComboBox();
    plotBox->addItems(*plotItems);
    plota = new LineInput(0, "", "0.0");
    plotb = new LineInput(0, "", "0.0");
    plotci = new LineInput(0, "", "0.0");
    plotcf = new LineInput(0, "", "1.0");
    plotLayout = new QGridLayout();
    plotLayout->addWidget(plotButton, 0, 0, 1, 1);
    plotLayout->addWidget(plotBox, 0, 1, 1, 1);
    plotAxisLayout = new QGridLayout();
    plotAxisWidget = new QWidget();
    plotAxisLayout->addWidget(plota, 1, 0);
    plotAxisLayout->addWidget(plotb, 1, 1);
    plotAxisLayout->addWidget(plotci, 2, 0);
    plotAxisLayout->addWidget(plotcf, 2, 1);
    plotAxisLayout->setRowStretch(3,20);
    plotAxisWidget->setLayout(plotAxisLayout);
    plotAxisWidget->setToolTip(tr("This mode creates a plot defined along\n") +
                               tr("the 3 axes."));
    plotLineLayout = new QGridLayout();
    plotLineWidget = new QWidget();
    plotxi = new LineInput(0, "Initial x", "-1.0");
    plotyi = new LineInput(0, "Initial y", "-1.0");
    plotzi = new LineInput(0, "Initial z", "-1.0");
    plotxf = new LineInput(0, "Final x", "1.0");
    plotyf = new LineInput(0, "Final y", "1.0");
    plotzf = new LineInput(0, "Final z", "1.0");
    plotRes = new LineInput(0, "Resolution", "100");
    plotLineLayout->addWidget(plotxi, 0, 0);
    plotLineLayout->addWidget(plotyi, 0, 1);
    plotLineLayout->addWidget(plotzi, 0, 2);
    plotLineLayout->addWidget(plotxf, 1, 0);
    plotLineLayout->addWidget(plotyf, 1, 1);
    plotLineLayout->addWidget(plotzf, 1, 2);
    plotLineLayout->addWidget(plotRes, 2, 0, 1, 3);
    plotLineLayout->setRowStretch(2,20);
    plotLineWidget->setLayout(plotLineLayout);
    plotLineWidget->setToolTip(tr("This mode creates a plot defined along\n") +
                               tr("an arbitrary line with resolution number\n")+
                               tr("of points along it (can interpolate)."));
    plotDVHWidget = new QWidget();
    plotDVHWidget->setToolTip(tr("This mode creates Dose Volume Histograms."));
    plotDVHLayout = new QGridLayout();
    plotDVHWidget->setLayout(plotDVHLayout);
    dvhExtra = new QCheckBox("Output Additional Data");
    dvhExtra->setToolTip(tr("Output additional information."));
    dvhDoseLab = new QLabel("Dx");
    dvhDoseLab->setToolTip(tr("All the volume fractions (%) input, delimited with a ',', will output\n") +
                           tr("the minimum dose received by x percent of the volume."));
    dvhDose = new QLineEdit();
    dvhDose->setToolTip(tr("All the volume fractions (%) input, delimited with a ',', will output\n") +
                        tr("the minimum dose received by x percent of the volume."));
    dvhVolLab = new QLabel("Vx");
    dvhVolLab->setToolTip(tr("All the doses input, delimited with a ',', will output the\n") +
                          tr("corresponding absolute volume that received at least x dose."));
    dvhVol = new QLineEdit();
    dvhVol->setToolTip(tr("All the doses input, delimited with a ',', will output the\n") +
                       tr("corresponding absolute volume that received at least x dose."));
    dvhxi = new LineInput(0, "Initial x", "-1.0");
    dvhyi = new LineInput(0, "Initial y", "-1.0");
    dvhzi = new LineInput(0, "Initial z", "-1.0");
    dvhxf = new LineInput(0, "Final x", "1.0");
    dvhyf = new LineInput(0, "Final y", "1.0");
    dvhzf = new LineInput(0, "Final z", "1.0");
    dvhLayout = new QGridLayout();
    dvhLayout->addWidget(dvhxi, 0, 0, 1, 4);
    dvhLayout->addWidget(dvhyi, 0, 4, 1, 4);
    dvhLayout->addWidget(dvhzi, 0, 8, 1, 4);
    dvhLayout->addWidget(dvhxf, 1, 0, 1, 4);
    dvhLayout->addWidget(dvhyf, 1, 4, 1, 4);
    dvhLayout->addWidget(dvhzf, 1, 8, 1, 4);
    dvhLayout->setRowStretch(2,20);
    dvhWidget = new QWidget();
    dvhWidget->setLayout(dvhLayout);
    egsDVHFile = new QLineEdit("EGSPhant File");
    egsDVHFile->setReadOnly(true);
    egsDVHBrowse = new QPushButton("Browse");
    egsDVHMeds = new QListWidget();
    egsDVHLayout = new QGridLayout();
    egsphant = new EGSPhant();
    egsDVHMeds->setSelectionMode(QAbstractItemView::ExtendedSelection);
    egsDVHLayout->addWidget(egsDVHBrowse, 0, 0, 1, 2);
    egsDVHLayout->addWidget(egsDVHFile, 0, 2, 1, 4);
    egsDVHLayout->addWidget(egsDVHMeds, 1, 0, 1, 6);
    egsDVHWidget = new QWidget();
    egsDVHWidget->setLayout(egsDVHLayout);
    plotStack = new QStackedLayout();
    plotStack->addWidget(plotAxisWidget);
    plotStack->addWidget(plotLineWidget);
    plotStack->addWidget(plotDVHWidget);
    plotStack->addWidget(dvhWidget);
    plotStack->addWidget(egsDVHWidget);
    plotLayout->addLayout(plotStack, 2, 0, 1, 2);
    plotFrame = new QGroupBox("Create plot csv data");
    plotFrame->setLayout(plotLayout);
    plotFrame->setToolTip(tr("Create comma separated value profile plots \n") +
                          tr("or dose volume histograms."));

    // Additional Functions Widget Setup
    stripButton = new QPushButton("Strip outer voxels");
    stripButton->setToolTip(tr("Remove all voxels adjacent to the boundary\n") +
                            tr("of the selected dose files."));
    pointButton = new QPushButton("Output dose at points");
    pointButton->setToolTip(tr("Output the dose at points defined by an\n") +
                            tr("input file.  The input file format is a\n") +
                            tr("a text file with a coordinate (x, y, z)\n") +
                            tr("per line (can interpolate)."));
    rebinButton = new QPushButton("Change 3ddose file boundaries");
    rebinButton->setToolTip(tr("Change the 3ddose files' boundaries to\n") +
                            tr("match those of the input file.  The input\n") +
                            tr("file format is a text file with all the x\n") +
                            tr("coordinates defined on the first line, y on\n")+
                            tr("the second and z on the third.For example,\n") +
                            tr("-15,-5,-4,-3,-2,-1,0,1,2,3,4,5,15\n") +
                            tr("-15,-0.5,0.5,15\n") +
                            tr("-10,0,1,2,3,4,5,6,7,8,9,10,25\n"));

    // Arithmatic Widget Setup
    ratioButton = new QPushButton("Divide by distribution");
    ratioButton->setToolTip(tr("Divide the dose in each voxel of selected\n") +
                            tr("distributions by the dose of a seperately\n") +
                            tr("selected distribution."));
    diffButton = new QPushButton("Subtract a distribution");
    diffButton->setToolTip(tr("Subtract the dose in each voxel of selected\n") +
                           tr("distributions by the dose of a seperately\n") +
                           tr("selected distribution."));
    sumButton = new QPushButton("Add a distribution");
    sumButton->setToolTip(tr("Add to the dose in each voxel of selected\n") +
                          tr("distributions the dose of a seperately\n") +
                          tr("selected distribution."));

    mathLayout = new QGridLayout();
    mathLayout->addWidget(ratioButton, 0, 0);
    mathLayout->addWidget(sumButton, 1, 0);
    mathLayout->addWidget(diffButton, 2, 0);
    mathFrame = new QGroupBox("Combine Distributions");
    mathFrame->setLayout(mathLayout);

    newLayout = new QGridLayout();
    newLayout->addWidget(stripButton, 0, 0);
    newLayout->addWidget(pointButton, 1, 0);
    newLayout->addWidget(rebinButton, 2, 0);
    newFrame = new QGroupBox("Additional Tools");
    newFrame->setLayout(newLayout);

    close = new QPushButton("Close");
    preview = new QPushButton("Previewer");
    mainLayout = new QGridLayout();
    mainLayout->addWidget(logo, 0, 0, 1, 1);
    mainLayout->addWidget(ioFrame, 1, 0, 1, 1);
    mainLayout->addWidget(normFrame, 3, 1, 2, 2);
    //mainLayout->addWidget(transFrame, 1, 3, 2, 1); // not worth the clutter
    mainLayout->addWidget(statFrame, 0, 3, 2, 1);
    mainLayout->addWidget(doseFrame, 2, 0, 3, 1);
    mainLayout->addWidget(plotFrame, 0, 1, 2, 2);
    mainLayout->addWidget(newFrame, 3, 3, 1, 1);
    mainLayout->addWidget(mathFrame, 4, 3, 1, 1);
    mainLayout->addWidget(close, 5, 3, 1, 1);
    mainLayout->addWidget(preview, 5, 0, 1, 1);

    mainLayout->setColumnStretch(0, 2);
    mainLayout->setColumnStretch(1, 1);
    mainLayout->setColumnStretch(2, 1);
    mainLayout->setColumnStretch(3, 1);

    //setStyleSheet(style);
    setLayout(mainLayout);
    setWindowTitle(tr("3ddose tools v. 1.1"));
    badInput.setTextFormat(Qt::PlainText);

    // Progress Bar
    remainder = new double (0.0);
    progWin = new QWidget();
    progLayout = new QGridLayout();
    progress = new QProgressBar();
    progLayout->addWidget(progress, 0, 0);
    progWin->setLayout(progLayout);
    progWin->resize(300, 0);
    progress->setRange(0, Dose::MAX_PROGRESS);
    //style = "";
    //style += "QWidget {background-color: rgb(240, 240, 240)}";
    //style += "QProgressBar {";
    //style += "border: 1px solid black;";
    //style += "text-align: top;";
    //style += "padding: 1px;";
    //style += "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,";
    //style += "stop: 0.4 rgb(240, 240, 240), stop: 1.0 rgb(200, 200, 200),";
    //style += "stop: 0.4 rgb(240, 240, 240), stop: 1.0 rgb(200, 200, 200));";
    //style += "}";
    //style += "QProgressBar::chunk {";
    //style += "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,";
    //style += "stop: 0.4 rgb(176, 224, 230), stop: 1.0 rgb(65, 105, 225),";
    //style += "stop: 0.4 rgb(176, 224, 230), stop: 1.0 rgb(65, 105, 225));";
    //style += "}";
    progWin->setFont(QFont("Serif", 12, QFont::Normal, false));
    //progWin->setStyleSheet(style);
}

void Interface::connectLayout() {
    connect(normBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(refresh()));
    connect(plotBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(refresh()));
    connect(statBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(refresh()));
    connect(dvhExtra, SIGNAL(stateChanged(int)),
            this, SLOT(refresh()));
    connect(statAutoBounds, SIGNAL(stateChanged(int)),
            this, SLOT(refresh()));

    connect(iButton, SIGNAL(clicked()),
            this, SLOT(addDose()));
    connect(oButton, SIGNAL(clicked()),
            this, SLOT(outputDose()));
    connect(dButton, SIGNAL(clicked()),
            this, SLOT(removeDose()));
    connect(cButton, SIGNAL(clicked()),
            this, SLOT(copyDose()));
    connect(rButton, SIGNAL(clicked()),
            this, SLOT(renameDose()));
    connect(normButton, SIGNAL(clicked()),
            this, SLOT(normalize()));
    connect(stripButton, SIGNAL(clicked()),
            this, SLOT(strip()));
    connect(pointButton, SIGNAL(clicked()),
            this, SLOT(doseAtPoints()));
    connect(rebinButton, SIGNAL(clicked()),
            this, SLOT(rebinBounds()));
    connect(transButton, SIGNAL(clicked()),
            this, SLOT(translate()));
    connect(ratioButton, SIGNAL(clicked()),
            this, SLOT(divide()));
    connect(diffButton, SIGNAL(clicked()),
            this, SLOT(subtract()));
    connect(sumButton, SIGNAL(clicked()),
            this, SLOT(add()));
    connect(plotButton, SIGNAL(clicked()),
            this, SLOT(plot()));
    connect(statButton, SIGNAL(clicked()),
            this, SLOT(stat()));
    connect(egsDVHBrowse, SIGNAL(clicked()),
            this, SLOT(selectEGSFile()));
    connect(statEgsBrowse, SIGNAL(clicked()),
            this, SLOT(selectStatEGSFile()));
    connect(close, SIGNAL(clicked()),
            this, SLOT(close()));
    connect(close, SIGNAL(clicked()),
            qApp, SLOT(quit()));
    connect(preview, SIGNAL(clicked()),
            this, SLOT(showPreview()));
}

void Interface::refresh() {
    switch (plotBox->currentIndex()) {
    case 0:
    case 1:
    case 2:
        if (!plotBox->currentText().compare("x axis")) {
            plota->title->setText("y");
            plotb->title->setText("z");
            plotci->title->setText("Initial x");
            plotcf->title->setText("Final x");
        }
        else if (!plotBox->currentText().compare("y axis")) {
            plota->title->setText("x");
            plotb->title->setText("z");
            plotci->title->setText("Initial y");
            plotcf->title->setText("Final y");
        }
        else if (!plotBox->currentText().compare("z axis")) {
            plota->title->setText("x");
            plotb->title->setText("y");
            plotci->title->setText("Initial z");
            plotcf->title->setText("Final z");
        }
        plotStack->setCurrentIndex(0);
        break;
    case 3:
        plotStack->setCurrentIndex(1);
        break;
    case 4:
        if ((plotBox->currentIndex()-2) != plotStack->currentIndex()) {
            plotDVHLayout->addWidget(dvhExtra, 1, 0, 1, 6);
            plotDVHLayout->addWidget(dvhDoseLab, 2, 0);
            plotDVHLayout->addWidget(dvhDose, 2, 1, 1, 2);
            plotDVHLayout->addWidget(dvhVolLab, 2, 3);
            plotDVHLayout->addWidget(dvhVol, 2, 4, 1, 2);
            plotDVHLayout->setRowStretch(0, 10);
        }
        plotStack->setCurrentIndex(2);
        break;
    case 5:
        if ((plotBox->currentIndex()-2) != plotStack->currentIndex()) {
            dvhLayout->addWidget(dvhExtra, 2, 0, 1, 12);
            dvhLayout->addWidget(dvhDoseLab, 3, 0);
            dvhLayout->addWidget(dvhDose, 3, 1, 1, 5);
            dvhLayout->addWidget(dvhVolLab, 3, 6);
            dvhLayout->addWidget(dvhVol, 3, 7, 1, 5);
        }
        plotStack->setCurrentIndex(3);
        break;
    case 6:
        if ((plotBox->currentIndex()-2) != plotStack->currentIndex()) {
            egsDVHLayout->addWidget(dvhExtra, 2, 0, 1, 6);
            egsDVHLayout->addWidget(dvhDoseLab, 3, 0);
            egsDVHLayout->addWidget(dvhDose, 3, 1, 1, 2);
            egsDVHLayout->addWidget(dvhVolLab, 3, 3);
            egsDVHLayout->addWidget(dvhVol, 3, 4, 1, 2);
        }
        plotStack->setCurrentIndex(4);
        break;
    default:
        break;
    }

    switch (statBox->currentIndex()) {
    case 0:
        statStack->setCurrentIndex(0);
        break;
    case 1:
        statStack->setCurrentIndex(1);
        break;
    case 2:
        statStack->setCurrentIndex(2);
        break;
    default:
        break;
    }

    normStack->setCurrentIndex(normBox->currentIndex());
    if (normBox->currentIndex()) {
        normButton->setText("Normalize");
    }
    else {
        normButton->setText("Scale");
    }

    if (statAutoBounds->isChecked()) {
        statMin->setEnabled(false);
        statMax->setEnabled(false);
    }
    else {
        statMin->setEnabled(true);
        statMax->setEnabled(true);
    }

    if (dvhExtra->isChecked()) {
        dvhDoseLab->setEnabled(true);
        dvhDose->setEnabled(true);
        dvhVolLab->setEnabled(true);
        dvhVol->setEnabled(true);
    }
    else {
        dvhDoseLab->setEnabled(false);
        dvhDose->setEnabled(false);
        dvhVolLab->setEnabled(false);
        dvhVol->setEnabled(false);
    }
}

void Interface::showPreview() {
    this->setDisabled(true);
    previewer->window->show();
    QApplication::processEvents();
    previewer->setEnabled(true);
    previewer->updateDoses();
}
