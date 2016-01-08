/*
###############################################################################
#
#  EGSnrc egs++ geometry viewer geometry view extensions
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

void GeometryView::updateView() {
#ifdef VIEW_DEBUG
    egsWarning("In GeometryView::updateView()\n");
#endif
}


void GeometryView::xRotation(int rot) {
#ifdef VIEW_DEBUG
    egsWarning("In GeometryView::xRotation(%d)\n",rot);
#endif
}


void GeometryView::yRotation(int rot) {
#ifdef VIEW_DEBUG
    egsWarning("In GeometryView::yRotation(%d)\n",rot);
#endif
}


void GeometryView::zRotation(int rot) {
#ifdef VIEW_DEBUG
    egsWarning("In GeometryView::zRotation(%d)\n",rot);
#endif
}



void GeometryView::startRotation() {
#ifdef VIEW_DEBUG
    egsWarning("In GeometryView::startRotation()\n");
#endif
}


void GeometryView::endRotation() {
#ifdef VIEW_DEBUG
    egsWarning("In GeometryView::endRotation()\n");
#endif
}
