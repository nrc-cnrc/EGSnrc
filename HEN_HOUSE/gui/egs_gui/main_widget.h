/*
###############################################################################
#
#  EGSnrc gui main widget headers
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


#ifndef EGS_GUI_MAIN_WIDGET_
#define EGS_GUI_MAIN_WIDGET_

#include <qwidget.h>
#include <qstring.h>
#include <qpixmap.h>

#include "pegs_page.h"
class QListWidget;
class QListWidgetItem;
class QStackedWidget;

class QComboBox;
class QLineEdit;
class EGS_CompilePage;
class EGS_RunPage;
class EGS_ConfigurationPage;
class EGS_PegsPage;
class QWizard;
class EGS_ConfigReader;
class QLibrary;

class EGS_MainWidget : public QWidget
{

  Q_OBJECT

public:

  EGS_MainWidget(QWidget * parent = 0, Qt::WFlags f = 0);
  ~EGS_MainWidget(){}
public slots:

  void exitGUI();

  void changePage(QListWidgetItem *,QListWidgetItem *);

  void changeEgsHome(const QString &);
  void changeHenHouse(const QString &);
  void changeUserCode(const QString &);
  void getConfigLib();
  void changeConfigLib();
  void aboutEGSGui();
  void aboutQt();
  void getHelp();

signals:

  void quit();
  void userCodeChanged(const QString &);

private:

  void addUserCodes(const QString &);
  void updateUserCodeList();

  QListWidget             *control;
  QStackedWidget          *work_area;

  EGS_CompilePage         *compile_page;
  EGS_PegsPage            *pegs_page;
  EGS_RunPage             *run_page;
  EGS_ConfigurationPage   *conf_page;
  QWizard                 *wizard;
  QComboBox               *user_code;
  EGS_ConfigReader        *config_reader;
  QLibrary                *wizard_lib;

  QString  egs_home;
  QString  hen_house;
  QString  libpath;

};

#endif