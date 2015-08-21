/*
###############################################################################
#
#  EGSnrc gui widget
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
#
###############################################################################
*/


#include "egs_gui_widget.h"
#include "egs_config_reader.h"

#include <qfile.h>
#include <qdir.h>
#include <qmessagebox.h>

#include <cstdlib>
#include <string>

using namespace std;
using namespace Qt;

EGS_GUI_Widget::EGS_GUI_Widget(QWidget *parent, const char *name,WFlags f) :
                               QWidget(parent,f), the_name(name)
{
    config_reader = 0; killed = false;
}

EGS_GUI_Widget::EGS_GUI_Widget(EGS_ConfigReader *cr, QWidget *parent, const char *name, WFlags f) :
                                                     QWidget(parent,f), the_name(name)
{
    config_reader = cr; killed = false;
}

void EGS_GUI_Widget::changeConfiguration(const QString &new_config) {
  if( new_config.isEmpty() ) {
#ifdef IK_DEBUG
    qDebug("EGS_GUI_Widget::changeConfiguration called with an empty string?");
#endif
    return;
  }
  if( !config_reader ) config_reader = new EGS_ConfigReader(new_config);
  else {
      int res = config_reader->checkConfigFile(new_config);
      bool use_it = ( res == 0 );
      if ( res ) {
          if( res == 1 ) QMessageBox::warning(this,"Error",
            QString("Failed to open %1 for reading").arg(new_config),
              QMessageBox::Ok,0,0);
          else if( res == 2 ) {
            int answer = QMessageBox::warning(this,"Error",
#ifdef WIN32
              "This appears to be a Unix/Linux config file\n"
              "Do you still want to use it ?",
#else
              "This appears to be a Windows config file\n"
              "Do you still want to use it ?",
#endif
              QMessageBox::Ok,QMessageBox::Cancel,0);
             if( answer == QMessageBox::Ok ) use_it = true;
          }
          else
              QMessageBox::warning(this,"Error",
                QString("Unknown error while reading %1").arg(new_config),
                QMessageBox::Ok,0,0);
      }
      if( use_it) config_reader->setConfig(new_config);
  }
  QString hh = config_reader->getVariable("HEN_HOUSE",true);
  emit henHouseChanged(hh);
}

void EGS_GUI_Widget::setHenHouse(const QString &new_hh) {
  QDir tmp(new_hh);
  if( tmp.exists() ) {
      QString hh = tmp.canonicalPath(); QChar junk = QDir::separator();
      if( !hh.endsWith("/") && !hh.endsWith(junk) )
          hh += QDir::separator();
      if( !config_reader ) config_reader = new EGS_ConfigReader;
      config_reader->setVariable("HEN_HOUSE",hh);
      emit henHouseChanged(hh);
  }
}

void EGS_GUI_Widget::setEgsHome(const QString &new_eh) {
  QDir tmp(new_eh);
  if( tmp.exists() ) {
      if( !config_reader ) config_reader = new EGS_ConfigReader;
      config_reader->setVariable("EGS_HOME",new_eh);
      emit egsHomeChanged(new_eh);
  }
}

QString EGS_GUI_Widget::egsConfiguration() {
    if( !config_reader ) config_reader = new EGS_ConfigReader;
    return config_reader->getConfig();
}

QString EGS_GUI_Widget::henHouse() {
    if( !config_reader ) config_reader = new EGS_ConfigReader;
    return config_reader->getVariable("HEN_HOUSE",true);
}

QString EGS_GUI_Widget::egsHome() {
    if( !config_reader ) config_reader = new EGS_ConfigReader;
    return config_reader->getVariable("EGS_HOME",true);
}

QString EGS_GUI_Widget::myMachine() {
    if( !config_reader ) config_reader = new EGS_ConfigReader;
    return config_reader->getVariable("my_machine",false);
}

QString EGS_GUI_Widget::canonicalSystem() {
    if( !config_reader ) config_reader = new EGS_ConfigReader;
    return config_reader->getVariable("canonical_system",false);
}

QString EGS_GUI_Widget::fFlags() {
    if( !config_reader ) config_reader = new EGS_ConfigReader;
    return config_reader->getVariable("FCFLAGS",false);
}

QString EGS_GUI_Widget::cFlags() {
    if( !config_reader ) config_reader = new EGS_ConfigReader;
    return config_reader->getVariable("C_FLAGS",false);
}

QString EGS_GUI_Widget::makeProgram() {
    if( !config_reader ) config_reader = new EGS_ConfigReader;
    return config_reader->getVariable("make_prog",true);
}

bool EGS_GUI_Widget::checkVars() {
#ifdef IK_DEBUG
  qDebug("In EGS_GUI_Widget::checkVars()");
#endif
  if( egsConfiguration().isEmpty() ) {
    QMessageBox::critical(this,"Error",
      "First define the configuration file on the configuration page",1,0,0);
    return false;
  }
  if( henHouse().isEmpty() ) {
    QMessageBox::critical(this,"Error",
      "First define HEN_HOUSE on the configuration page",1,0,0);
    return false;
  }
  if( egsHome().isEmpty() ) {
    QMessageBox::critical(this,"Error",
      "First define EGS_HOME on the configuration page",1,0,0);
    return false;
  }
  return true;
}
