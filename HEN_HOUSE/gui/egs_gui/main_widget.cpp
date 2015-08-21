/*
###############################################################################
#
#  EGSnrc gui main widget
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


#include <iostream>

#include "main_widget.h"
#include "egs_compile_page.h"
#include "egs_run_page.h"
#include "egs_configuration_page.h"
#include "egs_config_reader.h"

#ifdef STATICGUI
#include <egsnrcmp_setup.h>
#endif

#include <QListWidget>
#include <QAbstractItemView>
#include <QStackedWidget>
#include <QTextStream>
#include <QStyle>

#include <qlayout.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qmessagebox.h>
#include <qdir.h>
#include <qfile.h>
#include <qstringlist.h>
#include <qwizard.h>
#include <qlibrary.h>

bool configLibExists(const QString& lib );

// #define MW_DEBUG

using namespace std;

EGS_MainWidget::EGS_MainWidget(QWidget *parent, Qt::WFlags f)
                                          : QWidget(parent,f)
{

   setWindowTitle("EGSnrcMP GUI, National Research Council of Canada");
   setWindowIcon(QIcon(":/images/desktop_icon.png"));
   //setWindowIcon(QIcon("images/desktop_icon.png"));
   setWindowIconText("egs_gui");

    wizard_lib = 0; wizard = 0;
    config_reader = new EGS_ConfigReader;

    // topl is the layout of the entire widget
    QVBoxLayout *topl = new QVBoxLayout(this);
    topl->setSpacing(6); topl->setMargin(11);

    // wl is the layout responsible for the area occupied by the
    // control area and work area
    QHBoxLayout *wl = new QHBoxLayout;
    wl->setSpacing(6); wl->setMargin(11);

    // wbl is the layout for the control area and the user code combo box
    QVBoxLayout *wbl = new QVBoxLayout;
    wbl->setSpacing(6); wbl->setMargin(11);

    // The control area is a list with clickable items that change the
    // page in the widget stack below
    control = new QListWidget(this);
    control->setSelectionMode(QAbstractItemView::SingleSelection);
    control->setIconSize(QSize(48,48));
    int height = 50;// width = 100; control->setMaximumWidth(width+300); control->setMinimumWidth(width);
#ifdef IK_DEBUG
    qDebug("Creating Compile");
#endif
    QListWidgetItem *it = new QListWidgetItem(QIcon(":/images/make_kdevelop.png"), tr( "Compile" ), control);
    //QListWidgetItem *it = new QListWidgetItem(QIcon("images/make_kdevelop.png"), tr( "Compile" ), control);
    it->setSizeHint(QSize(it->sizeHint().width(), height));
#ifdef IK_DEBUG
    qDebug("Creating Execute");
#endif
    it = new QListWidgetItem(QIcon(":/images/launch.png"), tr( "Execute" ), control);
    //it = new QListWidgetItem(QIcon("images/launch.png"), tr( "Execute" ), control);
    it->setSizeHint(QSize(it->sizeHint().width(), height));
#ifdef IK_DEBUG
    qDebug("Creating PEGS Data");
#endif
    it = new QListWidgetItem(QIcon(":/images/file-manager.png"), tr( "PEGS Data" ), control);
    //it = new QListWidgetItem(QIcon("images/file-manager.png"), tr( "PEGS Data" ), control);
    it->setSizeHint(QSize(it->sizeHint().width(), height));
#ifdef IK_DEBUG
    qDebug("Creating Settings");
#endif
    it = new QListWidgetItem(QIcon(":/images/configure.png"), tr( "Settings" ), control);
    //it = new QListWidgetItem(QIcon("images/configure.png"), tr( "Settings" ), control);
    it->setSizeHint(QSize(it->sizeHint().width(), height));
#ifdef IK_DEBUG
    qDebug("Creating New config");
#endif
    it = new QListWidgetItem(QIcon(":/images/wizard-32.png"), tr( "New config" ), control);
    //it = new QListWidgetItem(QIcon("images/wizard-32.png"), tr( "New config" ), control);
    it->setSizeHint(QSize(it->sizeHint().width(), height));

    control->setCurrentItem(control->item(0));
    connect(control,SIGNAL(currentItemChanged(QListWidgetItem *,QListWidgetItem *)),this,
             SLOT(changePage(QListWidgetItem *,QListWidgetItem *)));

    wbl->addWidget(control);

    // here gb is the group box for the user code combo box
    QGroupBox *gb = new QGroupBox(this);
    gb->setTitle( tr("User code") );
    QHBoxLayout *h = new QHBoxLayout(gb); h->setAlignment(Qt::AlignCenter);
    user_code = new QComboBox(gb); user_code->addItem("user code");
    h->addWidget(user_code);

    wbl->addWidget(gb);

    wl->addLayout(wbl,1);

    work_area = new QStackedWidget(this);
    wl->addWidget(work_area,10);

    topl->addLayout(wl);

    QHBoxLayout *bl = new QHBoxLayout;
    bl->setSpacing(6); topl->setMargin(11);
    QAbstractButton *b = new QPushButton("&Help",this);
    connect(b,SIGNAL(clicked()),this,SLOT(getHelp()));
    bl->addWidget(b);
    b = new QPushButton("&About",this);
    connect(b,SIGNAL(clicked()),this,SLOT(aboutEGSGui()));
    bl->addWidget(b);
    b = new QPushButton("About Q&t",this);
    connect(b,SIGNAL(clicked()),this,SLOT(aboutQt()));
    bl->addWidget(b);
    b = new QPushButton("&Quit",this);
    QSpacerItem *spacer = new QSpacerItem(20,20,QSizePolicy::Expanding,
                                          QSizePolicy::Minimum);
    bl->addItem(spacer);
    bl->addWidget(b);
    connect(b,SIGNAL(clicked()),this,SLOT(exitGUI()));
    topl->addLayout(bl);

    compile_page = new EGS_CompilePage(config_reader,this,"compile page");
    work_area->addWidget(compile_page);

    run_page = new EGS_RunPage(config_reader,this,"run page");
    work_area->addWidget(run_page);

    conf_page = new EGS_ConfigurationPage(config_reader,
            this,"configuration page");
    work_area->addWidget(conf_page);

//-----------------------------------------------------------------------------
//  ADDED EGS_CONFIGURE WIZARD, A REDUCED VERSION OF
// EGS_INSTALL.
//                                                 EMH
// MOVED to slot changePage to avoid failure when statically built.
//-----------------------------------------------------------------------------

    libpath = QString();
//     getConfigLib(); // Sets wizard <- FAILS WHEN STATICALLY
//     work_area->addWidget(wizard);//     COMPILED !!!

//-----------------------------------------------------------------------------

    pegs_page = new EGS_PegsPage(this);
    pegs_page->setConfigReader(config_reader);
    work_area->addWidget(pegs_page);

    connect(conf_page,SIGNAL(egsHomeChanged(const QString &)),this,
             SLOT(changeEgsHome(const QString &)));
    connect(conf_page,SIGNAL(henHouseChanged(const QString &)),this,
             SLOT(changeHenHouse(const QString &)));
    connect( conf_page,SIGNAL(henHouseChanged(const QString &)),this,
             SLOT( changeConfigLib() ) );
    connect(user_code,SIGNAL(activated(const QString &)),this,
             SLOT(changeUserCode(const QString &)));
    connect(this,SIGNAL(userCodeChanged(const QString &)),compile_page,
             SLOT(setUserCode(const QString &)));
    connect(this,SIGNAL(userCodeChanged(const QString &)),run_page,
             SLOT(setUserCode(const QString &)));
    connect(compile_page,SIGNAL(targetChanged(const QString &)),run_page,
             SLOT(setTarget(const QString &)));
    connect(conf_page,SIGNAL(henHouseChanged(const QString &)),
             run_page,SLOT(getBatchOptions(const QString &)) );

    egs_home = conf_page->egsHome();
    changeHenHouse(conf_page->henHouse());
    if( !user_code->currentText().isEmpty() )
      emit userCodeChanged(user_code->currentText());
    compile_page->sendSignals();
}

#define ABOUT_TEXT "Graphical User Interface for EGSnrcMP"\
                   "\n            egs_gui, version 2.0"\
                   "\n\nAuthors: Iwan Kawrakow and Ernesto Mainegra"\
                   "\nCopyright National Research Council of Canada"\
                   "\n\nThis program is free software. It is distributed "\
                   "\nunder the terms of the GNU Public License, version 2."\
                   "\n See the file GPL_License in the licenses folder of the "\
                   "\nEGSnrcMP distribution for details."\
                   "\n\nThis program uses the Qt toolkit by Trolltech"
void EGS_MainWidget::aboutEGSGui() {
    //about_gui->show();
    QMessageBox::about ( this, tr("About egs_gui"), QString(ABOUT_TEXT) );

}

void EGS_MainWidget::aboutQt() {
  QMessageBox::aboutQt(this);
}

void EGS_MainWidget::getHelp() {
  QString info =
    "Sorry, this version of the EGSnrc GUI does not provide\n";
  info +=
    "online help. Perhaps the reports PIRS-701, PIRS-702 or\n";
  info +=
    "PIRS-877 (provided with the distribution) can answer \n";
  info += "your question ?";
  QMessageBox::information(this,"Help",info,QMessageBox::Ok);
}

void EGS_MainWidget::changeUserCode(const QString &uc) {
  emit userCodeChanged(uc);
}

void EGS_MainWidget::changeEgsHome(const QString &new_egs_home) {
  if( new_egs_home == egs_home ) return;
  egs_home = new_egs_home;
  updateUserCodeList();
}

void EGS_MainWidget::changeHenHouse(const QString &new_hen_house) {
  if( new_hen_house == hen_house ) return;
  hen_house = new_hen_house;
  updateUserCodeList();
}

void EGS_MainWidget::updateUserCodeList() {
  QString last_uc = user_code->currentText();
  int j;
  for(j=user_code->count()-1; j>=0; j--) user_code->removeItem(j);
  addUserCodes(egs_home);
  if( !hen_house.isEmpty() )
    addUserCodes(hen_house + QDir::separator() + "user_codes");
  for(j=0; j<user_code->count(); j++) {
    if( last_uc == user_code->itemText(j) ) {
      user_code->setCurrentIndex(j); return;
    }
  }
  user_code->setCurrentIndex(0);
  emit userCodeChanged(user_code->currentText());
}


void EGS_MainWidget::addUserCodes(const QString &from_dir) {
  if( from_dir.isEmpty() ) return;
  QDir d1(from_dir);
  QStringList list = d1.entryList(QDir::Dirs);
  for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
    QString s = *it;
    if( s != "." && s != ".." && s != "bin" && s != "pegs4" ) {
      QString aux = from_dir + QDir::separator() + s + QDir::separator() +
                    "Makefile";
      QFile f(aux);
      if( f.exists() ) {
        bool is_there = false;
        for(int j=0; j<user_code->count(); j++) {
          if( s == user_code->itemText(j) ) { is_there = true; break; }
        }
        if( !is_there ) user_code->addItem(s);
      }
    }
  }
}

void EGS_MainWidget::exitGUI() {
  emit quit();
}

void EGS_MainWidget::changePage(QListWidgetItem *item,QListWidgetItem * previous) {
#ifdef IK_DEBUG
  qDebug("Selection changed to: %s",item->text().toLatin1().data());
#endif
  if( item->text() == "Compile" ) work_area->setCurrentWidget(compile_page);
  else if( item->text() == "Execute" ) work_area->setCurrentWidget(run_page);
  else if( item->text() == "PEGS Data" ) work_area->setCurrentWidget(pegs_page);
  else if( item->text() == "Settings" ) work_area->setCurrentWidget(conf_page);
  else if( item->text() == "New config" ) {changeConfigLib();if (wizard)work_area->setCurrentWidget(wizard);}
#ifdef IK_DEBUG
  else qDebug("Unknow page %s",item->text().toLatin1());
#endif
}

//-----------------------------------------------------------------------------
//  FUNCTIONS RELATED TO CONFIGURATION WIZARD A REDUCED
//  VERSION OF EGS_INSTALL.
//                                                 EMH
//-----------------------------------------------------------------------------
/*! Loads configuration library from new location.
      If library path changes, this functions attempts to load the configuration
      wizard from the new location. In case of any failure, an empty wizard is
      used instead.
 */
void EGS_MainWidget::getConfigLib()
{
    QWizard* wiz;
    QString mhen_house = conf_page->henHouse();
    QChar s = QDir::separator();
    QString old_libpath   = libpath;
    libpath = mhen_house + s + (QString)"bin" + s + conf_page->myMachine() + s;

    if ( libpath == old_libpath ) return;
#ifdef STATICGUI
    wiz = new EGSnrcMP_setup(this,config_reader->getConfig());
#else
    if ( configLibExists( libpath ) ){
       typedef  QWizard* (*CreateEGS)( QWidget*, const QString& );
       QLibrary* lib = new QLibrary(  libpath + (QString)"egsconfig"  );
#ifdef MW_DEBUG
  qDebug(" About to load %s", (libpath + tr("egsconfig")).toLatin1().data());
#endif
       if (!lib){
#ifdef MW_DEBUG
  qDebug(" Failed to create library %s",(libpath + tr("egsconfig")).toLatin1().data());
#endif
          QMessageBox::critical( 0,
             tr("Error"),
             tr("Failed to create library: \n") + libpath + tr("egsconfig"),
             tr("Quit")
          );
          //wiz = new QWizard(this,"The wizard");
          wiz = new QWizard(this);
       }
       else if( !lib->load() ) {
#ifdef MW_DEBUG
          qDebug(" Failed to load library %s",lib->fileName().toLatin1().data());
#endif
          QMessageBox::critical( 0,
             tr("Error"),
             tr("Failed to load library: \n") + lib->fileName(),
             tr("Quit")
          );
          //wiz = new QWizard(this,"The wizard");
          wiz = new QWizard(this);
       }
       else{
#ifdef MW_DEBUG
          qDebug(" Loaded library %s",lib->fileName().toLatin1().data());
#endif
          CreateEGS create2 =(CreateEGS)  lib->resolve("create2");
          if ( create2 ) {
             wiz = create2( this, config_reader->getConfig() );
             wizard_lib = lib;
          }
          else {
             //wiz = new QWizard(this,"The wizard");
             wiz = new QWizard((QWidget*)this);
          }
       }
    }
    else{
        cout << " Couldn't find library " << libpath.toLatin1().data() << "egsconfig" << endl;
        wiz = new QWizard((QWidget*)this);
    }
#endif

    //cout << "Found and loaded library" << wizard_lib->fileName().toLatin1().data() << endl;
    wizard = wiz;
}

/*!
    Calls function for loading configuration library form new location
    and sets wizard on Configuration page.
*/
void EGS_MainWidget::changeConfigLib()
{
    getConfigLib(); // Sets wizard
    if (wizard)
     work_area->addWidget(wizard);
}

/*! Checks that a file containing "egsconfig" in the library location exists.
*/
bool configLibExists(const QString& lib ){

    QDir dir( lib );
    if ( ! dir.exists() ) return false;
    QStringList lst = dir.entryList( QStringList("*.*") );
    for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
            if ( ( *it ).contains( "egsconfig" ) )
                return true;
    }
    return false;
}
//-----------------------------------------------------------------------------
