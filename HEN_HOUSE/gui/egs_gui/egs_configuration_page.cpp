/*
###############################################################################
#
#  EGSnrc gui configuration page
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


#include "egs_configuration_page.h"

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qfiledialog.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qmessagebox.h>

using namespace Qt;

EGS_ConfigurationPage::EGS_ConfigurationPage(QWidget *parent,
       const char *name, WFlags f) : EGS_GUI_Widget(parent,name,f) {
    make();
}

EGS_ConfigurationPage::EGS_ConfigurationPage(EGS_ConfigReader *cr,
        QWidget *parent, const char *name, WFlags f) :
    EGS_GUI_Widget(cr,parent,name,f) { make(); }

void EGS_ConfigurationPage::make() {
  // the page layout
  QVBoxLayout *topl = new QVBoxLayout(this);
  topl->setSpacing(6); topl->setMargin(11);

  QLabel *rocket_egss = new QLabel(this);
  rocket_egss->setPixmap(QPixmap(":/images/rocket_egg_tr_f1_300.png"));
  //rocket_egss->setPixmap(QPixmap("images/rocket_egg_tr_f1_300.png"));
  topl->addWidget(rocket_egss,0,Qt::AlignHCenter);

  QSpacerItem *spacer = new QSpacerItem(20,20,QSizePolicy::Fixed,
                              QSizePolicy::Expanding);
  topl->addItem(spacer);

  // configuration
  QGroupBox *gb = new QGroupBox("configuration group box",this);
  QHBoxLayout *gbl = new QHBoxLayout(gb);gbl->setSpacing(6); gbl->setMargin(11);
  gb->setTitle( tr("Configuration file") );
  le_configuration = new QLineEdit("configuration line edit",gb);
  le_configuration->setText(egsConfiguration());
  le_configuration->setReadOnly(true);
  gbl->addWidget(le_configuration);
  QPushButton *b = new QPushButton("...",gb);
  b->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
  connect(b,SIGNAL(clicked()),this,SLOT(selectConfigurationFile()));
  gbl->addWidget(b);

  topl->addWidget(gb);

  gb = new QGroupBox("HEN_HOUSE group box",this);
  gbl = new QHBoxLayout(gb);gbl->setSpacing(6); gbl->setMargin(11);
  gb->setTitle( tr("HEN_HOUSE directory") );
  le_henhouse = new QLineEdit("HEN_HOUSE line edit",gb);
  le_henhouse->setText(henHouse());
  le_henhouse->setReadOnly(true);
  gbl->addWidget(le_henhouse);
  b = new QPushButton("...",gb);
  b->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
  connect(b,SIGNAL(clicked()),this,SLOT(selectHenHouse()));
  gbl->addWidget(b);

  topl->addWidget(gb);

  gb = new QGroupBox("EGS_HOME group box",this);
  gbl = new QHBoxLayout(gb);gbl->setSpacing(6); gbl->setMargin(11);
  gb->setTitle( tr("EGS_HOME directory") );
  le_egshome = new QLineEdit("EGS_HOME line edit",gb);
  le_egshome->setReadOnly(true);
  le_egshome->setText(egsHome());
  gbl->addWidget(le_egshome);
  b = new QPushButton("...",gb);
  b->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
  connect(b,SIGNAL(clicked()),this,SLOT(selectEgsHome()));
  gbl->addWidget(b);

  topl->addWidget(gb);

  spacer = new QSpacerItem(20,20,QSizePolicy::Fixed,
                              QSizePolicy::Expanding);
  topl->addItem(spacer);

  connect(this,SIGNAL(henHouseChanged(const QString &)),this,
          SLOT(setHenHouseField(const QString &)));

}

void EGS_ConfigurationPage::queueTypeChanged(int id) {
  /*
  qDebug("EGS_ConfigurationPage::queueTypeChanged: %d",id);
  if( queue_type != id ) {
    if( queue_type == 0 ) {
      bc_last_nqs = le_queue_command->text();
      batch_last_nqs = le_batch->text();
      options_last_nqs = le_batch_options->text();
    }
    else if ( queue_type == 1 ) {
      bc_last_other = le_queue_command->text();
      batch_last_other = le_batch->text();
      options_last_other = le_batch_options->text();
    }
    if( id == 0 ) {
      b_commands->setEnabled(true);
      le_queue_command->setText(bc_last_nqs);
      le_batch->setText(batch_last_nqs);
      le_batch_options->setText(options_last_nqs);
    }
    else if( id == 1 ) {
      b_commands->setEnabled(true);
      le_queue_command->setText(bc_last_other);
      le_batch->setText(batch_last_other);
      le_batch_options->setText(options_last_other);
    }
    else {
      b_commands->setEnabled(false);
    }
    queue_type = id;
  }
  */
}

void EGS_ConfigurationPage::setHenHouseField(const QString &hh) {
  le_henhouse->setText(hh);
}

void EGS_ConfigurationPage::selectConfigurationFile() {
#ifdef IK_DEBUG
  qDebug("In EGS_ConfiguirationPage::selectConfigurationFile()");
#endif
  QString start_dir;
  if( le_configuration->text().isEmpty() ) {
    if( le_henhouse->text().isEmpty() )
      start_dir = QDir::homePath();
    else
      start_dir = le_henhouse->text() + QDir::separator() + "specs";
  }
  else {
    QFileInfo fi(le_configuration->text());
    start_dir = fi.absolutePath();
  }
  QString new_conf = QFileDialog::getOpenFileName(this,tr("Select a configuration file"),
                                                  start_dir,
                                 tr("EGS configuration files (*.conf);; All files (*)"));
  QChar ss = QDir::separator(); QString sss = ss;
  new_conf.replace('/',sss); new_conf.replace('\\',sss);
  if( !new_conf.isEmpty() ) {
      changeConfiguration(new_conf);
      le_configuration->setText(egsConfiguration());
      if( le_egshome->text().isEmpty() )
          le_egshome->setText(egsHome());
  }
}

void EGS_ConfigurationPage::selectHenHouse() {
#ifdef IK_DEBUG
  qDebug("In EGS_ConfiguirationPage::selectHenHouse()");
#endif
  QString s = QFileDialog::getExistingDirectory(this,tr("Select HEN_HOUSE directory"),
                                                QDir::homePath(),QFileDialog::ShowDirsOnly);
  QChar ss = QDir::separator(); QString sss = ss;
  s.replace('/',sss); s.replace('\\',sss);
  if( !s.isEmpty() ) {
      if( !s.endsWith(sss) ) s += sss;
      setHenHouse(s); le_henhouse->setText(henHouse());
  }
}

void EGS_ConfigurationPage::selectEgsHome() {
#ifdef IK_DEBUG
  qDebug("In EGS_ConfiguirationPage::selectEgsHome()");
#endif
  QString s = QFileDialog::getExistingDirectory(this,tr("Select EGS_HOME directory"),
                                                QDir::homePath(),QFileDialog::ShowDirsOnly);
  QChar ss = QDir::separator(); QString sss = ss;
  s.replace('/',sss); s.replace('\\',sss);
  if( !s.isEmpty() ) {
    if( !s.endsWith(sss) ) s += sss;
    setEgsHome(s); le_egshome->setText(egsHome());
  }
}


