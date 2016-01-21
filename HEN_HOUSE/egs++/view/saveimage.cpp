/*
###############################################################################
#
#  EGSnrc egs++ geometry viewer save image extensions
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
#
###############################################################################
*/

#include "egs_libconfig.h"
#include "saveimage.h"

#include <qimage.h>
#include <qstringlist.h>
#include <qstring.h>
#include <qfiledialog.h>
#include <qimagewriter.h>


#ifdef VIEW_DEBUG
    extern void (* egsWarning)(const char *, ...);
#endif

SaveImage::SaveImage(QWidget *parent, const char *name)
    : QDialog(parent) {
    setObjectName(name);
    setModal(false);
    setupUi(this);

    QList<QByteArray> blist = QImageWriter::supportedImageFormats();
    int ind = -1;
    for (int i=0; i<blist.size(); i++) {
        formatCB->addItem(blist[i]);
        if (QString(blist[i]).toUpper() == "PNG") {
            ind = i;
        }
    }
    if (ind >= 0) {
        formatCB->setCurrentIndex(ind);
    }
}

SaveImage::~SaveImage() {
    // Qt handles child _widget_ deletion
}

void SaveImage::getImageSize(int *nx, int *ny) {
    *nx = xsizeSB->value();
    *ny = ysizeSB->value();
}

QString SaveImage::getImageFormat() {
    return formatCB->currentText();
}

QString SaveImage::getImageFileName() {
    QString fname = fileName->text(), format = ".";
    format += formatCB->currentText();
    if (!fname.endsWith(format,Qt::CaseInsensitive)) {
        fname += format.toLower();
    }
    return fname;
}


void SaveImage::selectFileName() {
    QString filter = "Images(";
    for (int j=0; j<formatCB->count(); j++) {
        filter += "*.";
        filter += formatCB->itemText(j).toLower();
        filter += " ";
    }
    filter += ")";
    QString s = QFileDialog::getSaveFileName(this, "Select a filename", QString(), filter);
    if (!s.isEmpty()) {
        fileName->setText(s);
        for (int j=0; j<formatCB->count(); j++) {
            if (s.endsWith(formatCB->itemText(j),Qt::CaseInsensitive)) {
                formatCB->setCurrentIndex(j);
            }
        }
    }
}

void SaveImage::enableOkButton() {
#ifdef VIEW_DEBUG
    egsWarning("In SaveImage::enableOkButton()\n");
#endif
    if (!fileName->text().isEmpty()) {
        okButton->setEnabled(true);
    }
    else {
        okButton->setEnabled(false);
    }
}

void SaveImage::fnameTextChanged(const QString &text) {
#ifdef VIEW_DEBUG
    egsWarning("SaveImage::fnameTextChanged(%s)\n",text.toUtf8().constData());
#endif
    if (text.isEmpty()) {
        okButton->setEnabled(false);
    }
    else {
        okButton->setEnabled(true);
    }
}