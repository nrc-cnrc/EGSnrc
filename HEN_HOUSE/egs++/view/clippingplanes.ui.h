/*
###############################################################################
#
#  EGSnrc egs++ geometry viewer clipping planes extensions
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

#include <qstring.h>

void ClippingPlanesWidget::applyClicked() {
     emit clippingPlanesChanged();
}


void ClippingPlanesWidget::helpClicked() {
#ifdef VIEW_DEBUG
    egsWarning("In ClippingPlanesWidget::helpClicked()\n");
#endif
}


int ClippingPlanesWidget::numPlanes() {
    return planeTable->numRows();
}


bool ClippingPlanesWidget::getPlane( int j, EGS_Vector &a, EGS_Float &d ) {
    if( !planeTable->isRowSelected(j,true) ) return false;
    QString tmp = planeTable->text(j,0);
    if( tmp.isEmpty() ) return false;
    bool ok; double aux = tmp.toDouble(&ok);
    if( !ok ) return false; a.x = aux;
    tmp = planeTable->text(j,1); if( tmp.isEmpty() ) return false;
    aux = tmp.toDouble(&ok); if( !ok ) return false; a.y = aux;
    tmp = planeTable->text(j,2); if( tmp.isEmpty() ) return false;
    aux = tmp.toDouble(&ok); if( !ok ) return false; a.z = aux;
    tmp = planeTable->text(j,3); if( tmp.isEmpty() ) return false;
    aux = tmp.toDouble(&ok); if( !ok ) return false; d = aux;
    return true;
}
