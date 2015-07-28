/*
###############################################################################
#
#  EGSnrc egs_inprz about form headers
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
#  Author:          Blake Walters, 2015
#
#  Contributors:
#
###############################################################################
*/


//qt3to4 -- BW
//implementation of AboutForm container class for AboutForm form
#ifndef ABOUTFORM_H
#define ABOUTFORM_H

#include <qvariant.h>
#include "ui_aboutform_rz.h"

class AboutForm : public QDialog, public Ui::AboutForm
{
    Q_OBJECT

public:
    AboutForm(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~AboutForm();

protected slots:
    virtual void languageChange();

};

#endif // ABOUTFORM_H
