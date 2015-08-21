/*
###############################################################################
#
#  EGSnrc gui run page headers
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


#ifndef EGS_GUI_RUN_PAGE_
#define EGS_GUI_RUN_PAGE_

#include <QAbstractButton>

#include "egs_gui_widget.h"
#include <qprocess.h>

class QLineEdit;
class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QRadioButton;
class QPushButton;
class QSpinBox;
class QTextEdit;
class QFileDialog;
class QComboBox;
class QProcess;
class EGS_ConfigReader;

class EGS_RunPage : public EGS_GUI_Widget {

  Q_OBJECT

public:

  EGS_RunPage(QWidget *parent=0, const char *name=0, Qt::WFlags f=0);
  EGS_RunPage(EGS_ConfigReader *cr,
          QWidget *parent=0, const char *name=0, Qt::WFlags f=0);
  ~EGS_RunPage(){}
public slots:

  void selectPegsFile();
  void pegsAreaChanged(const QString &);
  void go_pegsless(bool checked);
  void setUserCode(const QString &);
  void setTarget(const QString &);
  void selectInputFile();
  void checkRunOptions(int);
  void startExecution();
  void startBatchExecution();
  void stopExecution();
  void readProcessOut();
  void readProcessErr();
  void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void getBatchOptions();
  void getBatchOptions(const QString &);
  void closeStdin();

private:

  void make();
  void checkExecutable();
  bool addCommandArguments(QStringList &);
  bool parseBatchOptions();
  bool getVariable(const QString &from, const QString &what, QString &var);
  QString getExecutable();

  QCheckBox *pegsless;
  QPushButton *pegsFileButton;
  QLineEdit *pegs_file;
  QLineEdit *input_file;
  QLineEdit *extra_args;
  QLineEdit *extra_batch_args;

  QGroupBox    *run_options;
  QButtonGroup *bg_run_options;
  QRadioButton *i_button;
  QRadioButton *b_button;
  QSpinBox     *njob;
  QComboBox    *queue_system;
  QComboBox    *queue;

  QPushButton  *start_b;
  QPushButton  *stop_b;

  QTextEdit    *r_text;

  QComboBox    *look_for_pegs;

  QString      user_code;
  QString      target;

  QString      batch_command;
  QString      generic_bo;
  QString      output_bo;
  QString      rname_bo;
  QString      batch_sleep_time;
  QString      short_queue;
  QString      medium_queue;
  QString      long_queue;

  QProcess     *run;
  QProcess     *run_batch;
  QProcess     *run_current;
  int          n_start;
};

#endif
