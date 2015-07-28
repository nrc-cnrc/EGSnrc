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
#include <qradiobutton.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qfiledialog.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qmessagebox.h>

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
  //QPixmap *egg = new QPixmap(QPixmap::fromMimeSource("rocket_egg_tr_f1.jpg"));
  //egg->resize(300,300);
  //rocket_egss->setPixmap(*egg);
  rocket_egss->setPixmap(QPixmap::fromMimeSource("rocket_egg_tr_f1_300.png"));
  topl->addWidget(rocket_egss,0,Qt::AlignHCenter);

  QSpacerItem *spacer = new QSpacerItem(20,20,QSizePolicy::Fixed,
                              QSizePolicy::Expanding);
  topl->addItem(spacer);

  // configuration
  QGroupBox *gb = new QGroupBox(this,"configuration group box");
  gb->setColumnLayout(0,Qt::Horizontal);
  gb->layout()->setSpacing( 6 ); gb->layout()->setMargin( 11 );
  gb->setTitle( tr("Configuration file") );
  QHBoxLayout *hbl = new QHBoxLayout(gb->layout());
  le_configuration = new QLineEdit(gb,"configuration line edit");
  le_configuration->setText(egsConfiguration());
  le_configuration->setReadOnly(true);
  hbl->addWidget(le_configuration);
  QPushButton *b = new QPushButton("...",gb);
  b->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
  connect(b,SIGNAL(clicked()),this,SLOT(selectConfigurationFile()));
  hbl->addWidget(b);

  topl->addWidget(gb);

  gb = new QGroupBox(this,"HEN_HOUSE group box");
  gb->setColumnLayout(0,Qt::Horizontal);
  gb->layout()->setSpacing( 6 ); gb->layout()->setMargin( 11 );
  gb->setTitle( tr("HEN_HOUSE directory") );
  hbl = new QHBoxLayout(gb->layout());
  le_henhouse = new QLineEdit(gb,"HEN_HOUSE line edit");
  le_henhouse->setText(henHouse());
  le_henhouse->setReadOnly(true);
  hbl->addWidget(le_henhouse);
  b = new QPushButton("...",gb);
  b->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
  connect(b,SIGNAL(clicked()),this,SLOT(selectHenHouse()));
  hbl->addWidget(b);

  topl->addWidget(gb);

  gb = new QGroupBox(this,"EGS_HOME group box");
  gb->setColumnLayout(0,Qt::Horizontal);
  gb->layout()->setSpacing( 6 ); gb->layout()->setMargin( 11 );
  gb->setTitle( tr("EGS_HOME directory") );
  hbl = new QHBoxLayout(gb->layout());
  le_egshome = new QLineEdit(gb,"EGS_HOME line edit");
  le_egshome->setReadOnly(true);
  le_egshome->setText(egsHome());
  hbl->addWidget(le_egshome);
  b = new QPushButton("...",gb);
  b->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
  connect(b,SIGNAL(clicked()),this,SLOT(selectEgsHome()));
  hbl->addWidget(b);

  topl->addWidget(gb);

  spacer = new QSpacerItem(20,20,QSizePolicy::Fixed,
                              QSizePolicy::Expanding);
  topl->addItem(spacer);

  /*
  gb = new QGroupBox(this,"Batch job submission");
  gb->setColumnLayout(0,Qt::Horizontal);
  gb->layout()->setSpacing( 6 ); gb->layout()->setMargin( 11 );
  gb->setTitle( tr("Batch job submission") );
  hbl = new QHBoxLayout(gb->layout());

  QButtonGroup *bg = new QButtonGroup("Queue type",gb);
  bg->setColumnLayout(0,Qt::Vertical);
  bg->setExclusive(true);
  QVBoxLayout *vbl = new QVBoxLayout(bg->layout());
  QRadioButton *rb = new QRadioButton("NQS",bg,"nqs");
  rb->setChecked(true); vbl->addWidget(rb);
  queue_type = 0;
  rb = new QRadioButton("Other",bg,"other");
  vbl->addWidget(rb);
  rb = new QRadioButton("None",bg,"none");
  vbl->addWidget(rb);
  hbl->addWidget(bg);
  connect(bg,SIGNAL(clicked(int)),this,SLOT(queueTypeChanged(int)));

  QGroupBox *gb1 = new QGroupBox(gb,"batch options");
  gb1 ->setColumnLayout(0,Qt::Horizontal);
  gb1->setTitle( tr("Batch commands") );
  b_commands = gb1;
  QHBoxLayout *hbl1 = new QHBoxLayout(gb1->layout());

  QVBoxLayout *vbl1 = new QVBoxLayout;
  QLabel *l = new QLabel("batch command",gb1);
  vbl1->addWidget(l);
  l = new QLabel("$batch",gb1); vbl1->addWidget(l);
  l = new QLabel("$options",gb1); vbl1->addWidget(l);
  hbl1->addLayout(vbl1);

  vbl1 = new QVBoxLayout;
  le_queue_command = new QLineEdit(gb1,"queue command");
  le_queue_command->setText("$egs_command | $batch $options");
  vbl1->addWidget(le_queue_command);
  le_batch = new QLineEdit(gb1,"batch");
  le_batch->setText("qsub");
  vbl1->addWidget(le_batch);
  le_batch_options = new QLineEdit(gb1,"batch options");
  le_batch_options->setText("-me -eo -o ${input}.eo -r ${code}_${input} -x");
  vbl1->addWidget(le_batch_options);
  hbl1->addLayout(vbl1);

  hbl->addWidget(gb1);

  topl->addWidget(gb);
  */

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
      start_dir = QDir::homeDirPath();
    else
      start_dir = le_henhouse->text() + QDir::separator() + "specs";
  }
  else {
    QFileInfo fi(le_configuration->text());
    start_dir = fi.dirPath(true);
  }
  QString new_conf = QFileDialog::getOpenFileName(start_dir,
    "EGS configuration files (*.conf);; All files (*)",
    this,"configuration file dialog", "Select a configuration file");
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
  QString s = QFileDialog::getExistingDirectory(QDir::homeDirPath(),
    this,"hen house dialog","Select HEN_HOUSE directory");
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
  QString s = QFileDialog::getExistingDirectory(QDir::homeDirPath(),
    this,"egs home dialog","Select EGS_HOME directory");
  QChar ss = QDir::separator(); QString sss = ss;
  s.replace('/',sss); s.replace('\\',sss);
  if( !s.isEmpty() ) {
    if( !s.endsWith(sss) ) s += sss;
    setEgsHome(s); le_egshome->setText(egsHome());
  }
}


