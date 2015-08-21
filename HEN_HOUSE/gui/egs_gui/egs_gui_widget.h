/*
###############################################################################
#
#  EGSnrc gui widget headers
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


#ifndef EGS_GUI_WIDGET_
#define EGS_GUI_WIDGET_

#include <qwidget.h>
#include <qstring.h>

class QIODevice;
class EGS_ConfigReader;

class EGS_GUI_Widget : public QWidget {

  Q_OBJECT

public:

  EGS_GUI_Widget(QWidget *parent=0, const char *name=0, Qt::WFlags f=0);
  EGS_GUI_Widget(EGS_ConfigReader *, QWidget *parent=0,
          const char *name=0, Qt::WFlags f=0);

  virtual ~EGS_GUI_Widget(){}

public:

  QString henHouse();
  QString egsHome();
  QString egsConfiguration();
  QString myMachine();
  QString canonicalSystem();
  QString fFlags();
  QString cFlags();
  QString makeProgram();
  QString name(){return the_name;};

public slots:

  virtual void changeConfiguration(const QString &);
  virtual void setConfigReader(EGS_ConfigReader *r) { config_reader = r; };

signals:

  void egsHomeChanged(const QString &);
  void henHouseChanged(const QString &);

protected:

  EGS_ConfigReader *config_reader;

  bool killed;

  virtual void setHenHouse(const QString &);
  virtual void setEgsHome(const QString &);

  virtual bool checkVars();

private:

  QString the_name;
};

#endif
