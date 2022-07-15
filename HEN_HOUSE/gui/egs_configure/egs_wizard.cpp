/*
###############################################################################
#
#  EGSnrc configuration GUI wizard
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
#
###############################################################################
*/


#include <iostream>
#include <QIcon>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QPalette>
#include <QDate>

#include "egs_config_reader.h"
#include "egs_wizard.h"
#include "egs_location.h"
#include "egs_compilers.h"
#include "egs_licence.h"
#include "egs_install.h"

using namespace std;

EGS_Wizard::EGS_Wizard(QWidget *parent, Qt::WindowFlags f)
                                          : QWizard(parent,f),
                                          the_year("0000?"),version("0?"),
                                          custom(false)
{

   the_year = (QDate::currentDate()).toString("yyyy");
   version  = the_year; // Was QString(EGS_VERSION);
   setWindowTitle(tr("EGSnrc configuration, National Research Council of Canada ")+ the_year);
   setWindowIcon(QIcon(":/images/rocket_egg_tr_f.png"));
   setWindowIconText("egs_wizard");
#ifndef Q_OS_MAC
   setWizardStyle(QWizard::ModernStyle);
#endif
   setMinimumWidth(690); //setMaximumWidth(3000); setFixedHeight(420); //setMinimumHeight(420); setMaximumHeight(450);
   setPixmap(QWizard::LogoPixmap,
             QPixmap(":/images/nrc-badge.png").scaled(QSize(150,75),Qt::IgnoreAspectRatio,Qt::SmoothTransformation));

   config_reader = new EGS_ConfigReader;

   /* Initializing compilers (defaults to GNU compiler suite). Path to executables must be set. */
   fc = new MCompiler(F); cc = new MCompiler(C); cpp = new MCompiler(CPP);
   /* Initializing make utility (as a compiler). Path to executables must be set. */
   make = new MCompiler(GnuMake,"make");

   /* Welcome page definition */
   //addPage(createWelcomePage());
   setPage(Page_Intro, createWelcomePage());
   /* Location page definition */
   //addPage(createLocationPage());
   setPage(Page_Location, createLocationPage());
   setPage(Page_Compiler, createCompilerPage());
   /* Licence page definition */
   //addPage(createLicencePage());
   setPage(Page_Licence, createLicencePage());
   /* Installation page definition */
   //addPage(createInstallPage());
   setPage(Page_Install, createInstallPage());

   setButtonText(QWizard::CustomButton1, tr("&About"));
   setOption(QWizard::HaveCustomButton1, true);
   setButtonText(QWizard::CustomButton2, tr("About &Qt"));
   setOption(QWizard::HaveCustomButton2, true);
   connect(this, SIGNAL(customButtonClicked(int)),
           this, SLOT(processCustomButtonClick(int)));

   QList<QWizard::WizardButton> layout;
   layout << QWizard::CustomButton1 << QWizard::CustomButton2 << QWizard::Stretch <<
             QWizard::BackButton    << QWizard::NextButton    << QWizard::FinishButton;
   setButtonLayout(layout);

   setStartId(Page_Intro);

}

QWizardPage * EGS_Wizard::createWelcomePage(){
   QWizardPage *welcomePage = new QWizardPage(this);
   welcomePage->setTitle("Welcome to the EGSnrc configuration Wizard");
   welcomePage->setSubTitle("This wizard will guide you through the configuration of EGSnrc " + version);
   QLabel *guyLabel = new QLabel(welcomePage);
   guyLabel->setPixmap( QPixmap(":/images/the_guy_measures_flipped.png").scaled(QSize(200,200),
                        Qt::IgnoreAspectRatio,Qt::SmoothTransformation)
   );
   QHBoxLayout *hl = new QHBoxLayout(welcomePage); hl->setSpacing(30); hl->setMargin(11);
   QLabel *textLabel = new QLabel(welcomePage);
   textLabel->setFrameStyle(QFrame::StyledPanel || QFrame::Sunken);
   textLabel->setMargin(10); textLabel->setAutoFillBackground(true);
   QPalette palette; palette.setColor(QPalette::Window, Qt::white);
   textLabel->setPalette(palette);
   textLabel->setText("EGSnrc is an Open Source  Monte Carlo simulation toolkit for modelling the transport of "
                      "electrons, positrons and photons through matter. Users can benefit from a large "
                      "list of tutorials and custom user codes. Moreover the egs++ library provides "
                      "modules for arbitrary geometries and particle sources.<br><br>"
                      "<b>Compilers required:</b> FORTRAN (core system), C (parallel job submission), and C++ (egs++ and IAEA phase space files).");
   textLabel->setWordWrap(true);
   hl->addWidget(guyLabel); hl->addWidget(textLabel);
   return welcomePage;
}

QLocationPage * EGS_Wizard::createLocationPage(){
   locationPage = new QLocationPage(this,config_reader, make, fc, cc, cpp);
   return locationPage;
}

QCompilerPage * EGS_Wizard::createCompilerPage(){
   compilerPage = new QCompilerPage(this, make, fc, cc, cpp);
   return compilerPage;
}

QLicencePage * EGS_Wizard::createLicencePage(){
   licencePage = new QLicencePage(this,the_year,version);
   return licencePage;
}

QInstallPage * EGS_Wizard::createInstallPage(){
   installPage = new QInstallPage(this, make, fc, cc, cpp);
   return installPage;
}

void EGS_Wizard::processCustomButtonClick(int id){
  switch( id ){
    case CustomButton1:
      aboutEGSWizard();
      break;
    case CustomButton2:
      aboutQt();
      break;
  }
}
#define ABOUT_TEXT "Graphical Configuration Interface for EGSnrc"\
                   "\n            egs_configure, version 1.1"\
                   "\n\nAuthors: Ernesto Mainegra and Iwan Kawrakow"\
                   "\n\nThis program is free software. It is distributed "\
                   "\nunder the terms of the GNU Affero General Public License."\
                   "\n\nCopyright 2015 National Research Council of Canada"
void EGS_Wizard::aboutEGSWizard() {
    QMessageBox::about ( this, tr("About egs_configure"),
           QString("<p>Graphical Configuration Interface for EGSnrc</p>"\
                   "<p>Authors: Ernesto Mainegra and Iwan Kawrakow</p>"\
                   "<p>This program is free software. It is distributed under<br>"\
                   "the <a href=\"http://www.gnu.org/licenses/agpl.html\">GNU Affero General Public License</a>.</p>"\
                   "<p>Copyright (C) "+the_year+" National Research Council of Canada</p>") );
}

void EGS_Wizard::aboutQt() {
  QMessageBox::aboutQt(this);
}

void EGS_Wizard::getHelp() {
  QString info =
    "Sorry, this version of the EGSnrc GUI does not provide\n";
  info +=
    "online help. Perhaps the reports PIRS-701, PIRS-702 or\n";
  info +=
    "PIRS-877 (provided with the distribution) can answer \n";
  info += "your question ?";
  QMessageBox::information(this,"Help",info,QMessageBox::Ok);
}

void EGS_Wizard::exitGUI() {
}
