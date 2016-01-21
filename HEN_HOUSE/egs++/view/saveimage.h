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

#ifndef SAVEIMAGE_H
#define SAVEIMAGE_H

#include <QDialog>

#include "ui_saveimage.h" // for Ui::SaveImage

class SaveImage : public QDialog, public Ui::SaveImage {
    Q_OBJECT

public:

    SaveImage(QWidget *parent=0, const char *name=0);
    virtual ~SaveImage();

    virtual void getImageSize(int *, int *);
    virtual QString getImageFormat();
    virtual QString getImageFileName();

public slots:

    virtual void selectFileName();
    virtual void enableOkButton();
    virtual void fnameTextChanged(const QString &);

signals:

    void saveFileSelected(const QString &fname, const QString &format);

protected slots:

    virtual void languageChange() {
        retranslateUi(this);
    }
};

#endif // SAVEIMAGE_H