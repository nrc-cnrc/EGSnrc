/*
###############################################################################
#
#  EGSnrc gui pixmap headers
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
#  Author:          Iwan Kawrakow, 2003
#
#  Contributors:
#
###############################################################################
*/


#ifndef MY_PIXMAP__
#define MY_PIXMAP__

#include <qlistbox.h>

class EGS_MyPixmap : public QListBoxPixmap {

public:

    EGS_MyPixmap(const QPixmap &pix, const QString &text,
        double Stretch);


    int height(const QListBox * lb ) const;

private:

    double stretch;

};

#endif
