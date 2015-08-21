/*
###############################################################################
#
#  EGSnrc gui configuration page headers
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


#ifndef EGS_CONFIG_PAGE_
#define EGS_CONFIG_PAGE_

#include "egs_gui_widget.h"

#include <qstring.h>

class QLineEdit;
class QButtonGroup;
class QGroupBox;
class EGS_ConfigReader;

class EGS_ConfigurationPage : public EGS_GUI_Widget {

  Q_OBJECT

public:

  EGS_ConfigurationPage(QWidget *parent=0, const char *name=0, Qt::WFlags f=0);
  EGS_ConfigurationPage(EGS_ConfigReader *cr,
          QWidget *parent=0, const char *name=0, Qt::WFlags f=0);
  ~EGS_ConfigurationPage(){}
public slots:

  void selectConfigurationFile();
  void selectHenHouse();
  void selectEgsHome();
  void setHenHouseField(const QString &);
  void queueTypeChanged(int);

private:

  void make();

  QLineEdit  *le_egshome;
  QLineEdit  *le_henhouse;
  QLineEdit  *le_configuration;

};

#endif
