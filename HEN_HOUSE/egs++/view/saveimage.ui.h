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
#  Contributors:
#
###############################################################################
#
#  ui.h extension file, included from the uic-generated form implementation.
#
#  If you want to add, delete, or rename functions or slots, use Qt Designer
#  to update this file, preserving your code.
#
#  You should not define a constructor or destructor in this file. Instead,
#  write your code in functions called init() and destroy(). These will
#  automatically be called by the form's constructor and destructor.
#
###############################################################################
*/


#include "egs_libconfig.h"

#include <qimage.h>
#include <qstringlist.h>
#include <qstring.h>
#include <q3filedialog.h>
#include <qimagewriter.h>

#ifdef VIEW_DEBUG
extern void (* egsWarning)(const char*, ...);
#endif

void SaveImage::saveImage() {

}

void SaveImage::getImageSize(int *nx, int *ny) {
    *nx = xsizeSB->value(); *ny = ysizeSB->value();
}

QString SaveImage::getImageFormat() {
    return formatCB->currentText();
}

QString SaveImage::getImageFileName() {
    QString fname = fileName->text(), format = ".";
    format += formatCB->currentText();
    if( !fname.endsWith(format,false) ) fname += format.lower();
    return fname;
}


void SaveImage::selectFileName() {
    QString filter = "Images(";
    for(int j=0; j<formatCB->count(); j++) {
        filter += "*."; filter += formatCB->text(j).lower(); filter += " ";
    }
    filter += ")";
    QString s = Q3FileDialog::getSaveFileName(QString::null,filter,
                    this,
                    "save file dialog",
                    "Select a filename" );
    if( !s.isEmpty() ) {
        fileName->setText(s);
        for(int j=0; j<formatCB->count(); j++) {
            if( s.endsWith(formatCB->text(j),false) )
                formatCB->setCurrentItem(j);
        }
    }
}


void SaveImage::init() {
    QList<QByteArray> blist = QImageWriter::supportedImageFormats();
    // hacky code, please fix
    QStringList list;
    for (int i=0;i<blist.size();i++) {
        list << QString(blist.at(i));
    }
    formatCB->insertStringList(list);
    int ind = list.findIndex("PNG");
    if( ind >= 0 ) formatCB->setCurrentItem(ind);
}

void SaveImage::enableOkButton() {
#ifdef VIEW_DEBUG
    egsWarning("In SaveImage::enableOkButton()\n");
#endif
    if( !fileName->text().isEmpty() ) okButton->setEnabled(true);
    else okButton->setEnabled(false);
}

void SaveImage::fnameTextChanged(const QString &text) {
#ifdef VIEW_DEBUG
    egsWarning("SaveImage::fnameTextChanged(%s)\n",text.latin1());
#endif
    if( text.isEmpty() ) okButton->setEnabled(false);
    else okButton->setEnabled(true);
}
