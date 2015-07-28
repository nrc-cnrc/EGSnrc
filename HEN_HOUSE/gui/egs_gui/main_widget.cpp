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
#include "pegs_page.h"
#include "egs_config_reader.h"

#ifdef STATICGUI
#include <egsnrcmp_setup.h>
#endif

#include <qlistbox.h>
#include <qwidgetstack.h>
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

#include "my_pixmap.h"
#include "aboutform.h"


bool configLibExists(const QString& lib );

using namespace std;

EGS_MainWidget::EGS_MainWidget(QWidget *parent, const char *name,
   WFlags f) : QWidget(parent,name,f) {

    wizard_lib = 0;
    config_reader = new EGS_ConfigReader;

    setCaption("EGSnrcMP GUI, National Research Council of Canada");
    //my_icon = QPixmap::fromMimeSource( "egg_48_blue.png" );
    //my_icon = QPixmap::fromMimeSource( "rocket_egg_300.jpeg" );
    //my_icon.resize(32,32);
    //my_icon = QPixmap::fromMimeSource( "desktop_icon_2.png" );
    my_icon = QPixmap::fromMimeSource( "desktop_icon.png" );
    setIcon(my_icon);
    setIconText("egs_gui");

    about_gui = new aboutForm(this);
    //about_gui->setIcon(my_icon);

    // topl is the layout of the entire widget
    QVBoxLayout *topl = new QVBoxLayout(this);
    topl->setSpacing(6); topl->setMargin(11);

    // wl is the layout responsible for the area accupied by the
    // control area and work area
    QHBoxLayout *wl = new QHBoxLayout;
    wl->setSpacing(6); topl->setMargin(11);

    // wbl is the layout for the control area and the user code combo box
    QVBoxLayout *wbl = new QVBoxLayout;
    wbl->setSpacing(6); wbl->setMargin(11);

    // The control area is a list with clickable items that change the
    // page in the widget stack below
    control = new QListBox(this,"The control");
    control->setSelectionMode(QListBox::Single);
    double s = 1.4;
#ifdef IK_DEBUG
    qDebug("Creating Compile");
#endif
    EGS_MyPixmap *pix = new EGS_MyPixmap(
            QPixmap::fromMimeSource( "make_kdevelop.png" ), tr( "Compile" ),s );
    control->insertItem(pix);
#ifdef IK_DEBUG
    qDebug("Creating Execute");
#endif
    pix = new EGS_MyPixmap(
            QPixmap::fromMimeSource( "launch.png" ),tr( "Execute"),s );
    control->insertItem(pix);
#ifdef IK_DEBUG
    qDebug("Creating PEGS Data");
#endif
    pix = new EGS_MyPixmap(
            QPixmap::fromMimeSource( "file-manager.png" ),tr( "PEGS Data" ),s );
    control->insertItem(pix);
#ifdef IK_DEBUG
    qDebug("Creating Settings");
#endif
    pix = new EGS_MyPixmap(
            QPixmap::fromMimeSource( "configure.png" ),tr( "Settings" ),s );
    control->insertItem(pix);
#ifdef IK_DEBUG
    qDebug("Creating New config");
#endif
    pix = new EGS_MyPixmap(
            QPixmap::fromMimeSource( "wizard-32.png" ),tr( "New config" ),s );
    control->insertItem(pix);

    control->setSelected(0,true);
    connect(control,SIGNAL(selectionChanged(QListBoxItem *)),this,
             SLOT(changePage(QListBoxItem *)));

    wbl->addWidget(control);

    // here gb is the group box for the user code combo box
    QGroupBox *gb = new QGroupBox(this,"ucode box");
    gb->setColumnLayout(0, Qt::Vertical );
    gb->layout()->setSpacing( 6 ); gb->layout()->setMargin( 11 );
    gb->setTitle( tr("User code") );
    QHBoxLayout *h = new QHBoxLayout(gb->layout());
    h->setAlignment( Qt::AlignTop );
    user_code = new QComboBox(gb,"user code");
    h->addWidget(user_code);
    wbl->addWidget(gb);

    wl->addLayout(wbl,1);

    work_area = new QWidgetStack(this,"The working area");
    wl->addWidget(work_area,10);

    topl->addLayout(wl);

    QHBoxLayout *bl = new QHBoxLayout;
    bl->setSpacing(6); topl->setMargin(11);
    QButton *b = new QPushButton("&Help",this,"help_b");
    connect(b,SIGNAL(clicked()),this,SLOT(getHelp()));
    bl->addWidget(b);
    b = new QPushButton("&About",this,"about_b");
    connect(b,SIGNAL(clicked()),this,SLOT(aboutEGSGui()));
    bl->addWidget(b);
    b = new QPushButton("About Q&t",this,"about_qt");
    connect(b,SIGNAL(clicked()),this,SLOT(aboutQt()));
    bl->addWidget(b);
    b = new QPushButton("&Quit",this,"quit_b");
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

    libpath = QString::null;
//     getConfigLib(); // Sets wizard <- FAILS WHEN STATICALLY
//     work_area->addWidget(wizard);//     COMPILED !!!

//-----------------------------------------------------------------------------

    pegs_page = new EGS_PegsPage(this,"pegs page");
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

void EGS_MainWidget::aboutEGSGui() {
    about_gui->show();
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
    if( last_uc == user_code->text(j) ) {
      user_code->setCurrentItem(j); return;
    }
  }
  user_code->setCurrentItem(0);
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
          if( s == user_code->text(j) ) { is_there = true; break; }
        }
        if( !is_there ) user_code->insertItem(s);
      }
    }
  }
}

void EGS_MainWidget::exitGUI() {
  emit quit();
}

void EGS_MainWidget::changePage(QListBoxItem *item) {
#ifdef IK_DEBUG
  qDebug("Selection changet to: %s",item->text().latin1());
#endif
  if( item->text() == "Compile" ) work_area->raiseWidget(compile_page);
  else if( item->text() == "Execute" ) work_area->raiseWidget(run_page);
  else if( item->text() == "PEGS Data" ) work_area->raiseWidget(pegs_page);
  else if( item->text() == "Settings" ) work_area->raiseWidget(conf_page);
  else if( item->text() == "New config" ) {changeConfigLib();work_area->raiseWidget(wizard);}
#ifdef IK_DEBUG
  else qDebug("Unknow page %s",item->text().latin1());
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
    char s = QDir::separator();
    QString old_libpath   = libpath;
    libpath = mhen_house + s + (QString)"bin" + s + conf_page->myMachine() + s;

    if ( libpath == old_libpath ) return;

#ifdef STATICGUI
    wiz = new EGSnrcMP_setup(this,config_reader->getConfig());
#else
    if ( configLibExists( libpath ) ){
       typedef  QWizard* (*CreateEGS)( QWidget*, const QString& );
       QLibrary* lib = new QLibrary(  libpath + (QString)"egsconfig"  );
       if( !lib->load() ) {
          cout << " Failed to load library" << lib->library().latin1() << endl;
          QMessageBox::critical( 0,  tr("Error"),
	     tr("Failed to load library: \n"
	     + lib->library()), tr("Quit") );
              wiz = new QWizard(this,"The wizard");
       }
       else{
          CreateEGS create2 =(CreateEGS)  lib->resolve("create2");
          if ( create2 ) {
             wiz = create2( this, config_reader->getConfig() );
             wizard_lib = lib;
          }
          else {
             wiz = new QWizard(this,"The wizard");
          }
       }
    }
    else{
        QString junk = "wizard lib = ";
        QTextStream mist(&junk,IO_WriteOnly | IO_Append); mist << wizard_lib;
        wiz = new QWizard(this,"The wizard");
    }
#endif

    wizard = wiz;
}

/*!
    Calls function for loading configuration library form new location
    and sets wizard on Configuration page.
*/
void EGS_MainWidget::changeConfigLib()
{
    getConfigLib(); // Sets wizard
    work_area->addWidget(wizard);
}

/*! Checks that a file containing "egsconfig" in the library location exists.
*/
bool configLibExists(const QString& lib ){

    QDir dir( lib );
    if ( ! dir.exists() ) return false;
    QStringList lst = dir.entryList( "*.*" );
    for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
            if ( ( *it ).contains( "egsconfig" ) )
                return true;
    }
    return false;
}
//-----------------------------------------------------------------------------
