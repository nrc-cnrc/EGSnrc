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

#ifndef EGS_WIZARD_H
#define EGS_WIZARD_H

#include <QWidget>
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
         
  EGS_Wizard(QWidget * parent = 0, Qt::WFlags f = 0);
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