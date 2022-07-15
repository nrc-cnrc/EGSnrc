/*
###############################################################################
#
#  EGSnrc configuration GUI wizard headers
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
#  Contributors:    Cody Crewson
#                   Reid Townson
#
###############################################################################
*/


#ifndef EGS_WIZARD_H
#define EGS_WIZARD_H

#include <QtGlobal>
#if QT_VERSION >= 0x050000
    #include <QtWidgets>
#else
    #include <QWidget>
#endif
#include <QString>
#include <QPixmap>
#include <QWizard>
#include <QWizardPage>

class QWizard;
class EGS_ConfigReader;
class QLicencePage;
class QLocationPage;
class QCompilerPage;
class QInstallPage;
class MCompiler;

class EGS_Wizard : public QWizard
{

  Q_OBJECT

public:

  enum { Page_Intro, Page_Location, Page_Compiler,
         Page_Licence, Page_Install, Page_Conclusion };

  EGS_Wizard(QWidget * parent = 0, Qt::WindowFlags f = 0);
  ~EGS_Wizard(){}

public slots:

void exitGUI();
void aboutQt();
void aboutEGSWizard();
void getHelp();
void processCustomButtonClick(int id);
signals:

private:

  QWizardPage   * createWelcomePage();
  QCompilerPage * createCompilerPage();
  QLocationPage * createLocationPage();
  QLicencePage  * createLicencePage();
  QInstallPage  * createInstallPage();

  MCompiler     *fc, *cc, *cpp, *make; // Compilers + make utility

  QString the_year, version;
  QString  egs_home;
  QString  hen_house;
  QString  libpath;

  QLicencePage     *licencePage;
  QLocationPage    *locationPage;
  QCompilerPage    *compilerPage;
  QInstallPage     *installPage;
  EGS_ConfigReader *config_reader;

  QString canonicalSys;

  bool custom;

};

#endif
