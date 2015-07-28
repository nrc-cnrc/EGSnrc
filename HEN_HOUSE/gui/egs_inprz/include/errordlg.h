/*
###############################################################################
#
#  EGSnrc egs_inprz error dialog headers
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
#  Author:          Ernesto Mainegra-Hing, 2001
#
#  Contributors:
#
###############################################################################
*/


#ifndef ERRORDLG_H
#define ERRORDLG_H

#include <qdialog.h>

class MErrorDlg : public QDialog
{
    Q_OBJECT

public:
    MErrorDlg( QWidget *parent = 0, const char *name = 0, const QString& error = "ERROR..." );

};

#endif
