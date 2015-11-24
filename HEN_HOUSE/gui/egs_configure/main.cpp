/*
###############################################################################
#
#  EGSnrc configuration GUI main program
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
#  Author:          Ernesto Mainegra-Hing, 2015
#
#  Contributors:
#
###############################################################################
*/


#include "egs_wizard.h"
#include "egs_tools.h"
#include <QtGui>
#include <QMainWindow>

#include <qapplication.h>

int main(int argc, char **argv) {

    QApplication::setStyle("windows");
   QApplication a(argc,argv);
   EGS_Wizard ew(0);
   //a.connect(&ew,SIGNAL(quit()),SLOT(quit()));
   ew.show();
   return a.exec();

}
