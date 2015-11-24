/*
###############################################################################
#
#  EGSnrc configuration GUI location
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
#  Contributors:
#
###############################################################################
*/


#ifndef EGS_LOCATION_H
#define EGS_LOCATION_H

#include <QCoreApplication>
#include <QWizardPage>
#include <QFormLayout>
#include<QLineEdit>
#include <QGroupBox>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QButtonGroup>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>

#include <iostream>

#include "egs_config_reader.h"
#include "egs_tools.h"
#include "egs_wizard.h"

#if defined(Q_OS_MAC) || defined(Q_OS_DARWIN)
 #define CANONICAL "darwin"
#elif defined(Q_OS_LINUX)
 #define CANONICAL "linux"
#elif defined(Q_OS_UNIX)
 #define CANONICAL "unix"
#elif defined(Q_OS_WIN32) || defined(WIN32)
 #define CANONICAL "win"
#else
 #define CANONICAL "otherOS"
#endif

class QWizardPage;
class EGS_Wizard;

class QLocationPage : public QWizardPage
{

  Q_OBJECT

  Q_PROPERTY(QString confName READ getConfName WRITE setConfName)
  Q_PROPERTY(QString Canonical READ getCanonical WRITE setCanonical)
  Q_PROPERTY(bool copyUCs READ needsUCs WRITE setNeedsUCs)

public:

  QLocationPage(QWidget * parent, EGS_ConfigReader *cr,
                MCompiler *a_m, MCompiler *a_f, MCompiler *a_c, MCompiler *a_cpp)
                : QWizardPage(parent), fc(a_f), cc(a_c), cpp(a_cpp), make(a_m),
                  copyUCs(true), all_defaults_exist(true)
  {
       //fc = a_f; cc = a_c; cpp = a_cpp; make = a_m;
       setTitle("Configuration Page");
       setSubTitle("System location and configuration name");

       config_reader = cr;

       // the page layout
       QVBoxLayout *topl = new QVBoxLayout(this);
       //topl->setSpacing(6); topl->setMargin(11);

       QString arch = getArch(); canonical = QString(CANONICAL) + arch;

       // configuration file: if exists, defines config and HEN_HOUSE
       QGroupBox *gb = new QGroupBox("configuration group box",this);
       QHBoxLayout *gbl = new QHBoxLayout(gb);gbl->setSpacing(6); //gbl->setMargin(11);
       gb->setTitle( tr("Configuration name") );
       confLineEdit = new QLineEdit(gb); QString conf_temp = egsConfiguration();
       if (!conf_temp.isEmpty()){// configuration defines canonical
          setConfigField(conf_temp);
          //canonical = confLineEdit->text(); //canonical.truncate(canonical.lastIndexOf("."));
       }
       else // defaults to current directory, use a guess for canonical
          confLineEdit->setText( canonical );
          //confLineEdit->setText( canonical + tr(".conf"));

       /* Either canonical defines conf_name in pristine environment, or a configuration exists which defines canonical */
       conf_name = canonical;

       gbl->addWidget(confLineEdit);
       //QPushButton *b = new QPushButton("...",gb);
       //b->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
       //connect(b,SIGNAL(clicked()),this,SLOT(selectConfigurationFile()));
       //gbl->addWidget(b);

       topl->addWidget(gb);

       // System location (HEN_HOUSE):
       gb = new QGroupBox("HEN_HOUSE group box",this);
       gbl = new QHBoxLayout(gb);gbl->setSpacing(6); //gbl->setMargin(11);
       gb->setTitle( tr("System location (a.k.a. HEN_HOUSE)") );
       henLineEdit = new QLineEdit("HEN_HOUSE line edit",gb); QString hen_temp = henHouse();
       henLineEdit->setReadOnly(true);
       /* If empty, means either EGS_CONFIG not set, or not found in EGS_CONFIG. */
       henLineEdit->setText( hen_temp );
       //henLineEdit->setText( hen_temp.isEmpty() ? QDir::toNativeSeparators(QCoreApplication::applicationDirPath()) :
       //                                           hen_temp);
       //henLineEdit->setText( hen_temp.isEmpty() ? QDir::currentPath() : hen_temp);
       gbl->addWidget(henLineEdit);
       QPushButton *b = new QPushButton("...",gb);
       b->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
       connect(b,SIGNAL(clicked()),this,SLOT(selectHenHouse()));
       gbl->addWidget(b);

       topl->addWidget(gb);

       // Work Area (EGS_HOME) taken from environment or defaults to $HOME/egs_home
       gb = new QGroupBox("EGS_HOME group box",this);
       gbl = new QHBoxLayout(gb);gbl->setSpacing(6); //gbl->setMargin(11);
       gb->setTitle( tr("Working area (a.k.a. EGS_HOME)") );
       homeLineEdit = new QLineEdit("EGS_HOME line edit",gb);
/*       QString home_root = henLineEdit->text();
       if (home_root.endsWith(QDir::separator())) home_root.chop(1);
       home_root.truncate(home_root.lastIndexOf(QDir::separator())+1);
       homeLineEdit->setText(home_root + tr("egs_home"));*/
       homeLineEdit->setText( egsHome() );

       gbl->addWidget(homeLineEdit);
       b = new QPushButton("...",gb);
       b->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
       connect(b,SIGNAL(clicked()),this,SLOT(selectEgsHome()));
       gbl->addWidget(b);

       topl->addWidget(gb);

       connect(this,SIGNAL(henHouseChanged(const QString &)),this,
                    SLOT(setHenHouseField(const QString &)));

       // Define installation type: typical or custom
       //gb = new QGroupBox("Installation type group box",this);
       gbl = new QHBoxLayout();//gbl->setSpacing(6); gbl->setMargin(11);
       //gb->setTitle( tr("Installation type") );
       typical = new QRadioButton("&Typical"); custom = new QRadioButton("&Custom");
       gbl->addWidget(typical); gbl->addWidget(custom);
       if ( defaultMakeExists() && defaultFortranExists() &&
            defaultCExists()    && defaultCPPExists() )
            typical->setChecked(true);

       topl->addLayout(gbl);

       registerField("hen_house",henLineEdit);
       registerField("egs_conf", confLineEdit);
       registerField("egs_home", homeLineEdit);
       registerField("conf_name", this, "confName");
       registerField("Canonical", this, "Canonical");
       registerField("copyUCs", this,"copyUCs");

       if (!defaultMakeExists() || !defaultFortranExists() ||
           !defaultCExists()    || !defaultCPPExists()     )
           all_defaults_exist = false;

  }
  ~QLocationPage(){}

  int nextId() const
  {
    if ( typical->isChecked() )
         return EGS_Wizard::Page_Licence; // Page_Install
    else return EGS_Wizard::Page_Compiler;
  }

  QString getConfName(){
    conf_name = confLineEdit->text();
    //conf_name.truncate(conf_name.lastIndexOf("."));
    return conf_name;
  }
  void    setConfName(const QString & cn){conf_name = cn;}
  QString getCanonical(){return canonical;}
  void    setCanonical(const QString & can){canonical = can;}
  void setNeedsUCs(bool need){copyUCs=need;}
  bool needsUCs(){return copyUCs;}

public slots:
  /* In case one of the compilers is missing, there is no other choice than custom */
  void initializePage()
  {
      if (!all_defaults_exist){
          custom->setChecked(true); custom->hide(); typical->hide();
      }
  }

  bool defaultMakeExists() {
    if ( make->exists() ) return true;
#ifndef WIN32
    else if (MCompiler(GnuMake,"gmake").exists() ){
             MCompiler m(GnuMake,"gmake");
             *make = m;
             return true;
    }
#else
    else if ( MCompiler(GnuMake,"mingw32-make").exists() ){
              MCompiler m(GnuMake,"mingw32-make");
             *make = m;
              return true;
    }
#endif
    else return false;
  }

  bool defaultFortranExists() {
    if (fc->exists()) return true;
#ifdef WIN32
    else if ( MCompiler(F,"mingw32-gfortran").exists() ){
              MCompiler f(F,"mingw32-gfortran");
             *fc = f;
              return true;

    }
    else if ( MCompiler(F,"mingw32-g77" ).exists() ){
              MCompiler f(F,"mingw32-g77");
             *fc = f;
              return true;

    }
#else
    else if ( MCompiler(F,"g77").exists() ){
              MCompiler f(F,"g77");
             *fc = f;
              return true;

    }
#endif
    else return false;
  }

  bool defaultCExists() {
    if ( cc->exists() ) return true;
#ifdef WIN32
    else if ( MCompiler(C,"mingw32-gcc").exists() ){
              MCompiler c(C,"mingw32-gcc");
             *cc = c;
              return true;
    }
    else if ( MCompiler(C,"x86_64-w64-mingw32-gcc").exists() ){
              MCompiler c(C,"x86_64-w64-mingw32-gcc");
             *cc = c;
              return true;

    }
    else if ( MCompiler(C,"i686-w64-mingw32-gcc").exists() ){
              MCompiler c(C,"i686-w64-mingw32-gcc");
             *cc = c;
              return true;

    }
#else
    else if (MCompiler(C,"c89").exists()) { MCompiler c(C,"c89");*cc = c; return true; }
    else if (MCompiler(C,"c90").exists()) { MCompiler c(C,"c90");*cc = c; return true; }
    else if (MCompiler(C,"c99").exists()) { MCompiler c(C,"c99");*cc = c; return true; }
#endif
    else return false;
  }

  bool defaultCPPExists() {
    if (cpp->exists()) return true;
#ifdef WIN32
    else if ( MCompiler(CPP,"mingw32-g++").exists() ){
              MCompiler c(CPP,"mingw32-g++");
             *cpp = c;
              return true;
    }
    else if ( MCompiler(CPP,"x86_64-w64-mingw32-g++").exists() ){
              MCompiler c(CPP,"x86_64-w64-mingw32-g++");
             *cpp = c;
              return true;
    }
    else if ( MCompiler(CPP,"i686-w64-mingw32-g++").exists() ){
              MCompiler c(CPP,"i686-w64-mingw32-g++");
             *cpp = c;
              return true;

    }
#endif
    else return false;
  }

  bool validatePage()
  {
    /* Configuration file exists and seems to be the proper one for the current OS
       checkConfigFile returns 0 if no unusual errors in config file detected.
     */
    if (!config_reader->checkConfigFile(henLineEdit->text() + "/specs/" + confLineEdit->text() + ".conf")){
        QMessageBox msgBox;
        msgBox.addButton(QMessageBox::Yes);
        QPushButton *noButton = msgBox.addButton(QMessageBox::No);
        msgBox.setDefaultButton(noButton);
        msgBox.setText("Configuration " + confLineEdit->text() + " exists!");
        msgBox.setInformativeText("Do you want to overwrite it?");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        if (msgBox.clickedButton() == noButton)
           return false;
    }

    if (henLineEdit->text().isEmpty()){
        QMessageBox::critical(this, tr("Error!!!"),
                               tr("Please select a directory with an existing EGSnrc system!"));
        return false;
    }

    // Allowing empty EGS_HOME in which case no user code is copied. Must check in environment settings
    // for an empty EGS_HOME!!!
    if (homeLineEdit->text().isEmpty()){
        QMessageBox::critical(this, tr("Working area error!"),
                              tr("Please select the working area directory \n") +
                              tr("needed to create configuration related\n") +
                              tr("directories and copy user codes!"));
        return false;
    }

    return dirOK(henLineEdit->text(),false) && dirOK(homeLineEdit->text(),true);
  }

  bool dirOK(const QString & dir, const bool &create){
    if ( !QDir(dir).exists() ){
       if ( create ){ // directory can be created
          QMessageBox msgBox(QMessageBox::Warning, tr("Warning!"),
                       dir + tr(" does not exist! Do you want to create it?"), 0, this);
                   msgBox.addButton(tr("&OK"), QMessageBox::AcceptRole);
                   msgBox.addButton(tr("&Cancel"), QMessageBox::RejectRole);
           if (msgBox.exec() == QMessageBox::AcceptRole){
              if (! QDir().mkpath(dir) ){
                  QMessageBox::warning(this, tr("Error creating ") + dir,
                                             tr("Check whether you have proper permissions!\n")+
                                             tr("Fix this issue before proceeding with install!"));
                  return false;
              }
            }
            else
                return false;
       }
       else{// directory must exist
          QMessageBox::critical(this, tr("System location error!"),
                               dir + tr(" does not exist!\n") +
                               tr("Make sure to select a directory with an existing EGSnrc system!"));

      }
    }
    else{
        if (!create) // Assuming this is a HEN_HOUSE directory from github
            return is_A_HenHouse(dir);
        else{
          QMessageBox msgBox(QMessageBox::Warning, tr("Warning!"),
                       tr("Working area ") + dir + tr(" exists!\n") +
                       tr(" Do you want to overwrite it?\n") +
                       tr("Choose No to configure an existing system!"), 0, this);
                   msgBox.addButton(tr("&Yes"), QMessageBox::AcceptRole);
                   QPushButton *noButton = msgBox.addButton(tr("&No"), QMessageBox::RejectRole);
                   msgBox.setDefaultButton(noButton);
           if ( msgBox.exec() == QMessageBox::RejectRole )
              copyUCs = false;
           else
              copyUCs = true; // Although set to true by default, this is needed in case Back button pressed.
        }
    }

    return true;

  }

  bool is_A_HenHouse(const QString & h){
    if (!QFile( h + QDir::separator() + "src/egsnrc.mortran").exists()){
       QMessageBox::critical(this, tr("Beware!"),
                             h + tr(" does not seem to contain a valid EGSnrc system!\n") +
                             tr("Make sure to select a directory with an existing EGSnrc system!"));
      return false;
    }
    else return true;
  }

  QString egsConfiguration() {
      return config_reader ? config_reader->getConfig() : QString();
  }

  /* EGS_CONFIG is the only supplier of an existing HEN_HOUSE through config_reader. */
  QString henHouse() {
      return config_reader ? config_reader->getVariable("HEN_HOUSE",true) : QString();
  }

  /* EGS_HOME can only be taken from environment. */
  QString egsHome() {
     return config_reader ? config_reader->getVariable("EGS_HOME",true) : QString(getenv("EGS_HOME"));
  }

/* Determines system and processor architecture. On Windows we are checking if
   we are dealing with a 32 bit process on a 64 bit CPU.
 */
QString getArch(){
#ifndef WIN32
  QProcess arch;
  arch.start("getconf",QStringList() << "LONG_BIT");
  if (!arch.waitForStarted()) return QString();
  arch.closeWriteChannel();
  if (!arch.waitForFinished()) return QString();
  QString answer = QString(arch.readAll());
  if (answer.endsWith("\n")) answer.chop(1);
  return answer;
#else
  /* On Windows one can have 32 bit processes on 64 bit processor */
  QString archi = (config_reader->getVariable("PROCESSOR_ARCHITECTURE",false)).contains("64")? "64":"32";
  QString archi6432 = (config_reader->getVariable("PROCESSOR_ARCHITEW6432",false)).contains("64")? "64":"32";
  return archi6432+archi;
#endif
}

void selectConfigurationFile() {
#ifdef LP_DEBUG
  qDebug("In QLocationPage::::selectConfigurationFile()");
#endif
  QString start_dir;
  if( confLineEdit->text().isEmpty() ) {
    if( henLineEdit->text().isEmpty() )
      start_dir = QDir::homePath();
    else
      start_dir = henLineEdit->text() + QDir::separator() + "specs";
  }
  else {
    QFileInfo fi(confLineEdit->text());
    start_dir = fi.absolutePath();
  }

  QString new_conf = QFileDialog::getOpenFileName(this,tr("Select a configuration file"),
                                                  start_dir,
                                 tr("EGS configuration files (*.conf);; All files (*)"));
  QChar ss = QDir::separator(); QString sss = ss;
  new_conf.replace('/',sss); new_conf.replace('\\',sss);
  if( !new_conf.isEmpty() ) {
      changeConfiguration(new_conf);
      setConfigField(egsConfiguration());
  }
}

void selectHenHouse() {
#ifdef LP_DEBUG
  qDebug("In QLocationPage::selectHenHouse()");
#endif
  QString s = QFileDialog::getExistingDirectory(this,tr("Select HEN_HOUSE directory"),
                                                QDir::currentPath(),QFileDialog::ShowDirsOnly);// homePath()
  if( !s.isEmpty() ) {
      if( !s.endsWith(QDir::separator()) ) s += QDir::separator();
      setHenHouse(s); henLineEdit->setText(henHouse());
  }
}

void selectEgsHome() {
#ifdef LP_DEBUG
  qDebug("In QLocationPage::selectEgsHome()");
#endif
  QString s = QFileDialog::getExistingDirectory(this,tr("Select EGS_HOME directory"),
                                                QDir::currentPath(),QFileDialog::ShowDirsOnly);// homePath()
  if( !s.isEmpty() ) {
    if( !s.endsWith(QDir::separator()) ) s += QDir::separator();
    setEgsHome(s); homeLineEdit->setText(egsHome());
  }
}

void changeConfiguration(const QString &new_config) {
  if( new_config.isEmpty() ) {
#ifdef LP_DEBUG
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
  setHenHouseField(henHouse());
}

void setHenHouseField(const QString &hh) {
  henLineEdit->setText(hh);
}

/* Sets config file name without path information */
void setConfigField(const QString &conf) {
  QString the_conf = conf; the_conf.truncate(the_conf.lastIndexOf("."));
  confLineEdit->setText( the_conf.right( the_conf.length() - the_conf.lastIndexOf(QDir::separator()) - 1 ));
}

void setHenHouse(const QString &new_hh) {
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

void setEgsHome(const QString &new_eh) {
  QDir tmp(new_eh);
  if( tmp.exists() ) {
      if( !config_reader ) config_reader = new EGS_ConfigReader;
      config_reader->setVariable("EGS_HOME",new_eh);
      emit egsHomeChanged(new_eh);
  }
}

signals:

  void egsHomeChanged(const QString &);
  void henHouseChanged(const QString &);

private:
  EGS_ConfigReader *config_reader;
  MCompiler        *fc, *cc, *cpp, *make; // Compilers + make utility
  QLineEdit        *henLineEdit,
                   *homeLineEdit,
                   *confLineEdit;
  QRadioButton     *typical, *custom;
  QString           canonical, conf_name;
  bool              copyUCs, all_defaults_exist;
};

#endif