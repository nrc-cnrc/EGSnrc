/*
###############################################################################
#
#  EGSnrc egs_inprz error dialog
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
#  Contributors:    Blake Walters
#
###############################################################################
*/


#include "errordlg.h"
#include "richtext.h"
//#include <q3textview.h>
//#include <q3hbox.h>
//#include <q3vbox.h>

MErrorDlg::MErrorDlg( QWidget *parent, const char *name, const QString& error )
          : QDialog( parent )
{
       MyRichText* tv = new MyRichText( this, error.toLatin1().data() );
       tv->resize( 450, 450 );
       tv->setWindowTitle( "Beware, error in input !!!	" );
       tv->show();

}
