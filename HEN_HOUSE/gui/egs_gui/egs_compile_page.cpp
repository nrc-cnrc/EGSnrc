/*
###############################################################################
#
#  EGSnrc gui compile page
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
#  Contributors:
#
###############################################################################
*/


#include "egs_compile_page.h"

#include <qlayout.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qfile.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qtextedit.h>
#include <qprocess.h>
#include <qmessagebox.h>
#include <qcstring.h>

#include <cstdlib>

const QSizePolicy ignored(QSizePolicy::Ignored,QSizePolicy::Ignored);
const QSizePolicy preferred(QSizePolicy::Preferred,QSizePolicy::Preferred);

EGS_CompilePage::EGS_CompilePage(QWidget *parent, const char *name,
    WFlags f) : EGS_GUI_Widget(parent,name,f) { make(); }

EGS_CompilePage::EGS_CompilePage(EGS_ConfigReader *cr, QWidget *parent,
    const char *name, WFlags f) : EGS_GUI_Widget(cr,parent,name,f) { make(); }

void EGS_CompilePage::make() {

   // The layout of the compile widget
   QVBoxLayout *topl = new QVBoxLayout(this);
   topl->setSpacing(6); topl->setMargin(11);

   c_option = new QButtonGroup("Target",this,"target group");
   c_option->setOrientation(Qt::Vertical);
   c_option->layout()->setSpacing(6); c_option->layout()->setMargin(11);
   QVBoxLayout *bgl = new QVBoxLayout( c_option->layout() );

   QRadioButton *rb = new QRadioButton("opt",c_option,"opt");
   rb->setChecked(true); bgl->addWidget(rb); the_target = 0;
   rb = new QRadioButton("noopt",c_option,"noopt"); bgl->addWidget(rb);
   rb = new QRadioButton("debug",c_option,"debug"); bgl->addWidget(rb);
   rb = new QRadioButton("clean",c_option,"clean"); bgl->addWidget(rb);
   connect(c_option,SIGNAL(clicked(int)),this,SLOT(changeTarget(int)));

   QHBoxLayout *hl1 = new QHBoxLayout;
   hl1->addWidget(c_option);

   QVBoxLayout *vl1 = new QVBoxLayout;

   // Fortran options
   QGroupBox *gb = new QGroupBox(this,"Extra F options");
   gb->setColumnLayout(0, Qt::Vertical );
   gb->layout()->setSpacing( 6 ); gb->layout()->setMargin( 11 );
   gb->setTitle( tr("Extra Fortran options") );
   QHBoxLayout *hbl = new QHBoxLayout(gb->layout());
   hbl->setAlignment( Qt::AlignTop );
   extra_f_options = new QLineEdit(gb,"extra fortran options");
   hbl->addWidget(extra_f_options);
   vl1->addWidget(gb);
   // C Options
   gb = new QGroupBox(this,"Extra C options");
   gb->setColumnLayout(0, Qt::Vertical );
   gb->layout()->setSpacing( 6 ); gb->layout()->setMargin( 11 );
   gb->setTitle( tr("Extra C options") );
   hbl = new QHBoxLayout(gb->layout());
   hbl->setAlignment( Qt::AlignTop );
   extra_c_options = new QLineEdit(gb,"extra C options");
   hbl->addWidget(extra_c_options);
   vl1->addWidget(gb);

   hl1->addLayout(vl1);
   topl->addLayout(hl1,1);

   hl1 = new QHBoxLayout;
   QPushButton *b = new QPushButton("&Go",this,"start compilation");
   connect(b,SIGNAL(clicked()),this,SLOT(startCompilation()));
   hl1->addWidget(b); is_running = false; start_c = b;

   QSpacerItem *spacer = new QSpacerItem(20,20,QSizePolicy::Expanding,
                                          QSizePolicy::Minimum);
   hl1->addItem(spacer);
   b = new QPushButton("&Cancel",this,"cancel button");
   connect(b,SIGNAL(clicked()),this,SLOT(stopCompilation()));
   b->setEnabled(false); hl1->addWidget(b); stop_c = b;
   /*
   spacer = new QSpacerItem(20,20,QSizePolicy::Expanding,QSizePolicy::Minimum);
   hl1->addItem(spacer);
   b = new QPushButton("&Details >>",this,"details button");
   connect(b,SIGNAL(clicked()),this,SLOT(showHideDetails()));
   hl1->addWidget(b); details_c = b;
   */

   topl->addLayout(hl1,1);

   c_text = new QTextEdit(this,"compilation output");
   QFont ctext_font(  c_text->font() );
   ctext_font.setFamily( "Courier [Adobe]" );
   ctext_font.setPointSize( 9 );
   c_text->setFont( ctext_font );

   c_text->setReadOnly(true);
   //c_text->hide(); d_hidden = true;
   topl->addWidget(c_text,100);

   c_process = new QProcess;
   connect(c_process,SIGNAL(processExited()),this,SLOT(compilationFinished()));
   connect(c_process,SIGNAL(readyReadStdout()),this,SLOT(readProcessOut()));
   connect(c_process,SIGNAL(readyReadStderr()),this,SLOT(readProcessErr()));
}

bool EGS_CompilePage::checkExeDir() {
  QString exe_dir = egsHome() + "bin" + QDir::separator();
  QDir dexe(exe_dir);
  if( !dexe.exists() ) {
#ifdef IK_DEBUG
    qDebug("Creating directory %s",exe_dir.latin1());
#endif
    if( !dexe.mkdir(exe_dir) ) return false;
  }
  exe_dir += myMachine();
  dexe.setPath(exe_dir);
  if( !dexe.exists() ) {
#ifdef IK_DEBUG
    qDebug("Creating directory %s",exe_dir.latin1());
#endif
    if( !dexe.mkdir(exe_dir) ) return false;
  }
  return true;
}

void EGS_CompilePage::startCompilation() {
#ifdef IK_DEBUG
  qDebug("In EGS_CompilePage::startCompilation()\n");
#endif
  if ( !checkVars() ) return;
  start_c->setEnabled(false); stop_c->setEnabled(true);
  c_option->setEnabled(false);

  if( !checkExeDir() )
    qFatal("Failed to create executable directory for %s",myMachine().latin1());

  c_process->clearArguments();
  c_process->addArgument(makeProgram());
#ifdef IK_DEBUG
  qDebug("user code: %s",the_user_code.latin1());
#endif
  QString ucode = the_user_code;
  QString udir = egsHome() + ucode;
  QDir::setCurrent(udir);
#ifdef IK_DEBUG
  qDebug("udir: %s",udir.latin1());
#endif
  QButton *b = c_option->selected();
  QString bname = b->name();
#ifdef IK_DEBUG
  qDebug("selected button: %s",bname.latin1());
#endif
  if( bname != "opt" ) c_process->addArgument(bname);
  QString conf = "EGS_CONFIG="; conf += egsConfiguration();
  c_process->addArgument(conf);
  QString ehome = "EGS_HOME="; ehome += egsHome();
  c_process->addArgument(ehome);
  if( bname != "clean" ) {
    QString fopt = extra_f_options->text();
    if( !fopt.isEmpty() ) {
      //QString aux="FCFLAGS=\"";
      //aux += fFlags() + " " + fopt + "\"";
      QString aux="FCFLAGS=";
      aux += fFlags() + " " + fopt;
      c_process->addArgument(aux);
    }
    QString copt = extra_c_options->text();
    if( !copt.isEmpty() ) {
      QString aux="C_FLAGS=\"";
      aux += cFlags() + " " + copt + "\"";
      c_process->addArgument(aux);
    }
  }
#ifdef IK_DEBUG
  qDebug("Current directory: %s",QDir::currentDirPath().latin1());
#endif
  QStringList list = c_process->arguments();
  qWarning("Executing: <%s>",list.join(" ").latin1());
  c_text->setText("");

  c_process->start();

  is_running = true;
}

void EGS_CompilePage::stopCompilation() {
#ifdef IK_DEBUG
  qDebug("In EGS_CompilePage::stopCompilation()\n");
#endif
  /*
  start_c->setEnabled(true); stop_c->setEnabled(false);
  c_option->setEnabled(true);
  is_running = false;
  */
  c_process->tryTerminate();
}

void EGS_CompilePage::readProcessOut() {
  QByteArray a = c_process->readStdout();
  c_text->insert(a); //c_text->append(a);
}

void EGS_CompilePage::readProcessErr() {
  QByteArray a = c_process->readStderr();
  c_text->insert(a); //c_text->append(a);
}

void EGS_CompilePage::setUserCode(const QString &new_user_code) {
#ifdef IK_DEBUG
  qDebug("In EGS_CompilePage::setUserCode: %s",new_user_code.latin1());
#endif
  the_user_code = new_user_code;
}

void EGS_CompilePage::sendSignals() {
  QButton *b = c_option->selected();
  QString bname = b->name();
  emit targetChanged(bname);
}

void EGS_CompilePage::changeTarget(int id) {
  if( id == the_target ) return;
  the_target = id;
  QButton *b = c_option->selected(); QString bname = b->name();
  emit targetChanged(bname);
}

void EGS_CompilePage::compilationFinished() {
#ifdef IK_DEBUG
  qDebug("In EGS_CompilePage::compilationFinished()\n");
#endif
  start_c->setEnabled(true); stop_c->setEnabled(false);
  c_option->setEnabled(true);
  is_running = false;

  QString bname = c_option->selected()->name();
  if( bname != "clean" ) {
  if( !c_process->normalExit() || c_process->exitStatus() )
    //QMessageBox::critical(this,"Compilation Error","Compilation failed",
    //   QMessageBox::Ok,QMessageBox::NoButton);
    c_text->append(
  "\n\n************************* failed *****************************\n\n");
  else c_text->append(
  "\n\n************************* success *****************************\n\n");
  }
}
