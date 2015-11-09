/***************************************************************************
    $Id$
    begin                : August 2015
    copyright            : (C) 2015 by Ernesto Mainegra-Hing and NRC
    email                : ernesto.mainegra-hing@nrc-cnrc.gc.ca
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
