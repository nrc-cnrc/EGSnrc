/*
###############################################################################
#
#  EGSnrc gui compile page headers
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
#
###############################################################################
*/


#ifndef EGS_GUI_COMPILE_PAGE_
#define EGS_GUI_COMPILE_PAGE_

#include "egs_gui_widget.h"

#include <qstring.h>
#include <qprocess.h>

class QComboBox;
class QLineEdit;
class QButtonGroup;
class QGroupBox;
class QPushButton;
class QIODevice;
class QTextEdit;
class QProcess;
class EGS_ConfigReader;

class EGS_CompilePage : public EGS_GUI_Widget {

  Q_OBJECT

public:

  EGS_CompilePage(QWidget *parent = 0, const char * name = 0, Qt::WindowFlags f = 0);
  EGS_CompilePage(EGS_ConfigReader *,
          QWidget *parent = 0, const char * name = 0, Qt::WindowFlags f = 0);
  ~EGS_CompilePage(){}

  void sendSignals();

signals:

  void targetChanged(const QString &);

public slots:

  void startCompilation();
  void stopCompilation();
  void compilationFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void readProcessOut();
  void readProcessErr();
  //void showHideDetails();
  void setUserCode(const QString &);
  void changeTarget(int);

private:

  bool checkExeDir();

private:

  void make();

  //QButtonGroup *c_option;
  QButtonGroup *bg_coption;
  QGroupBox *c_option;
  QLineEdit *extra_f_options;
  QLineEdit *extra_c_options;

  QPushButton *start_c;
  QPushButton *stop_c;

  QProcess *c_process;

  QTextEdit *c_text;

  QString the_user_code;

  int  the_target;
  bool is_running;
};

#endif
