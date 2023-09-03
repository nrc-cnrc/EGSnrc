/*
###############################################################################
#
#  EGSnrc gui main program
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
#  Contributors:    Ernesto Mainegra-Hing
#                   Cody Crewson
#                   Reid Townson
#
###############################################################################
*/


#include "main_widget.h"
#include <QtGlobal>
#if QT_VERSION >= 0x050000
    #include <QtWidgets>
#else
    #include <QWidget>
#endif
#include <QMainWindow>

#include <qapplication.h>

int main(int argc, char **argv) {
   QApplication::setStyle("windows");
   QApplication a(argc,argv);
   EGS_MainWidget mw(0);
   a.connect(&mw,SIGNAL(quit()),SLOT(quit()));
   mw.show();
   return a.exec();

}
