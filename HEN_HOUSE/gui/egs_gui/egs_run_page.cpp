/*
###############################################################################
#
#  EGSnrc gui run page
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
#                   Cody Crewson
#
###############################################################################
*/


#include "egs_run_page.h"

#include <QCheckBox>

#include <qlayout.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qtextedit.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qprocess.h>
#include <qmessagebox.h>
#include <qstringlist.h>

#include <cstdlib>

// #define RP_DEBUG

using namespace Qt;

EGS_RunPage::EGS_RunPage(QWidget *parent, const char *name, WindowFlags f) :
  EGS_GUI_Widget(parent,name,f) { make(); }

EGS_RunPage::EGS_RunPage(EGS_ConfigReader *cr,
     QWidget *parent, const char *name, WindowFlags f) :
  EGS_GUI_Widget(cr,parent,name,f) { make(); }

void EGS_RunPage::make() {

  QVBoxLayout *topl = new QVBoxLayout(this);
  topl->setSpacing(6); topl->setMargin(11);

  QVBoxLayout *vl1 = new QVBoxLayout;

  QGroupBox *gb = new QGroupBox(this);
  gb->setObjectName(QString::fromUtf8("pegs_file_group"));
  QVBoxLayout *gbl = new QVBoxLayout(gb);gbl->setSpacing(6); gbl->setMargin(11);
  gb->setTitle( tr("PEGS file") );

  QHBoxLayout *hbl = new QHBoxLayout();
  pegs_file = new QLineEdit(gb);
  hbl->addWidget(pegs_file);
  pegsFileButton = new QPushButton("...",gb);
  pegsFileButton->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
  connect(pegsFileButton,SIGNAL(clicked()),this,SLOT(selectPegsFile()));
  hbl->addWidget(pegsFileButton);

  gbl->addLayout(hbl);

  hbl = new QHBoxLayout();
  QLabel *label = new QLabel("Look in:",gb);
  hbl->addWidget(label);
  look_for_pegs = new QComboBox(gb);
  look_for_pegs->addItem("User pegs area");
  look_for_pegs->addItem("HEN_HOUSE pegs area");
  look_for_pegs->addItem("HOME");
  connect(look_for_pegs,SIGNAL(activated(const QString &)),this,
            SLOT(pegsAreaChanged(const QString &)));
  hbl->addWidget(look_for_pegs);
  QSpacerItem *spacer = new QSpacerItem(20,20,QSizePolicy::Expanding,
                                          QSizePolicy::Minimum);
  hbl->addItem(spacer);
  pegsless = new QCheckBox("&pegsless",gb);
  connect(pegsless,SIGNAL(toggled(bool)),this,SLOT(go_pegsless(bool)));
  hbl->addWidget(pegsless);

  gbl->addLayout(hbl);

  vl1->addWidget(gb);

  gb = new QGroupBox(this); gb->setTitle( tr("Input file") );
  gb->setObjectName(QString::fromUtf8("input_file_group"));

  hbl = new QHBoxLayout(gb);
  input_file = new QLineEdit(gb);
  hbl->addWidget(input_file);
  QPushButton *b = new QPushButton("...",gb);
  b->resize(b->minimumSize());
  connect(b,SIGNAL(clicked()),this,SLOT(selectInputFile()));
  hbl->addWidget(b);

  vl1->addWidget(gb);

  gb = new QGroupBox(this);
  gb->setObjectName(QString::fromUtf8("extra_options_group"));
  hbl = new QHBoxLayout(gb); hbl->setSpacing(6); hbl->setMargin(11);
  gb->setTitle( tr("Extra arguments") );

  QGroupBox *gb1 = new QGroupBox(gb); gb1->setTitle( tr("User code") );
  gb1->setObjectName(QString::fromUtf8("extra_user_code"));
  QHBoxLayout *hbl1 = new QHBoxLayout(gb1);
  extra_args = new QLineEdit(gb1);
  extra_args->setObjectName(QString::fromUtf8("extra args"));
  hbl1->addWidget(extra_args);

  hbl->addWidget(gb1);

  gb1 = new QGroupBox("extra batch",gb); gb1->setTitle( tr("Batch command") );
  hbl1 = new QHBoxLayout(gb1);
  extra_batch_args = new QLineEdit(gb1);
  extra_batch_args->setObjectName(QString::fromUtf8("extra batch args"));
  hbl1->addWidget(extra_batch_args);

  hbl->addWidget(gb1);
#ifdef WIN32
  gb1->setEnabled(false);
#endif

  vl1->addWidget(gb);

  QHBoxLayout *hl1 = new QHBoxLayout;
  hl1->setSpacing(6); hl1->setMargin(11);
  hl1->addLayout(vl1);

  vl1 = new QVBoxLayout;

  bg_run_options = new QButtonGroup(this);
  run_options    = new QGroupBox("run options group",this);run_options->setTitle( tr("Run options") );
  QVBoxLayout *bgl = new QVBoxLayout(run_options);bgl->setSpacing(6); bgl->setMargin(11);
  QRadioButton *rb = new QRadioButton("Interactive",run_options);
  rb->setChecked(true); i_button = rb;       bgl->addWidget(rb);bg_run_options->addButton(rb,0);
  rb = new QRadioButton("Batch",run_options); b_button = rb;
  bgl->addWidget(rb);bg_run_options->addButton(rb,1);

#ifdef WIN32
  b_button->setEnabled(false);
#endif
  connect(bg_run_options,SIGNAL(buttonClicked(int)),
                         SLOT(checkRunOptions(int)));

  vl1->addWidget(run_options);

  gb   = new QGroupBox("n_parallel",this);gb->setTitle( tr("Number of jobs") );
  gbl  = new QVBoxLayout(gb);gbl->setSpacing(6); gbl->setMargin(11);
  njob = new QSpinBox(gb);njob->setRange(1,10000);njob->setValue(1);
  njob->setEnabled(false);
  gbl->addWidget(njob);
  vl1->addWidget(gb);

  gb = new QGroupBox("queue_type",this);gb->setTitle( tr("Queueing system") );
  gbl = new QVBoxLayout(gb);gbl->setSpacing(6); gbl->setMargin(11);

  queue_system = new QComboBox(gb);
  queue_system->setEnabled(false);
  getBatchOptions();
  gbl->addWidget(queue_system);

  vl1->addWidget(gb);

  gb = new QGroupBox("queue",this); gb->setTitle( tr("Queue") );
  gbl = new QVBoxLayout(gb);gbl->setSpacing(6); gbl->setMargin(11);
  queue = new QComboBox(gb);
  queue->addItem("short"); queue->addItem("medium"); queue->addItem("long");
  queue->setCurrentIndex(2);
  queue->setEnabled(false);
  gbl->addWidget(queue);

  vl1->addWidget(gb);

  hl1->addLayout(vl1);

  topl->addLayout(hl1);

  hl1 = new QHBoxLayout;
  gb = new QGroupBox("buttons",this);
  hbl = new QHBoxLayout(gb);

  b = new QPushButton("&Start",gb);
  connect(b,SIGNAL(clicked()),this,SLOT(startExecution()));
  hbl->addWidget(b); start_b = b;
  spacer = new QSpacerItem(20,20,QSizePolicy::Expanding,
                                          QSizePolicy::Minimum);
  hbl->addItem(spacer);

  b = new QPushButton("Sto&p",gb);
  connect(b,SIGNAL(clicked()),this,SLOT(stopExecution()));
  b->setEnabled(false);
  hbl->addWidget(b); stop_b = b;

  topl->addWidget(gb);

  r_text = new QTextEdit(this);
  QFont rtext_font(  r_text->font() );
  rtext_font.setFamily( "Courier [Adobe]" );
  rtext_font.setPointSize( 9 );
  r_text->setFont( rtext_font );

  r_text->setReadOnly(true);
  topl->addWidget(r_text);

  run = new QProcess; run_batch = new QProcess;
  connect(run,SIGNAL(finished(int , QProcess::ExitStatus )),this,
              SLOT(processFinished(int , QProcess::ExitStatus )));
  connect(run,SIGNAL(readyReadStandardOutput()),this,SLOT(readProcessOut()));
  connect(run,SIGNAL(readyReadStandardError()),this,SLOT(readProcessErr()));
  connect(run_batch,SIGNAL(finished(int , QProcess::ExitStatus )),this,
              SLOT(processFinished(int , QProcess::ExitStatus )));
  connect(run_batch,SIGNAL(readyReadStandardOutput()),this,SLOT(readProcessOut()));
  connect(run_batch,SIGNAL(readyReadStandardError()),this,SLOT(readProcessErr()));
  // Qt3 to Qt4 -- EMH
  //Only needed if writing to stdin after QProcess::start which is not the case anywhere here!
  //connect(run_batch,SIGNAL(wroteToStdin()),this,SLOT(closeStdin()));

}

void EGS_RunPage::getBatchOptions(const QString &) {
    getBatchOptions();
}

void EGS_RunPage::getBatchOptions() {
  QString script_dir = henHouse() + "scripts";
  QDir dir(script_dir);
  QStringList list = dir.entryList( QStringList("batch_options.*") );
#ifdef RP_DEBUG
  qDebug("batch_options in %s:",script_dir.toLatin1().data());
#endif
  queue_system->clear();
  for(QStringList::Iterator it=list.begin(); it != list.end(); ++it) {
      QString aux = *it;
      if( !aux.endsWith("~") && !aux.endsWith(".bak") ) {
          aux.replace("batch_options.","");
#ifdef RP_DEBUG
          qDebug("  %s",aux.toLatin1().data());
#endif
          queue_system->addItem(aux);
      }
  }
  char *ebs = getenv("EGS_BATCH_SYSTEM");
  if( ebs ) {
#ifdef RP_DEBUG
      qDebug("EGS_BATCH_SYSTEM: %s",ebs);
#endif
      queue_system->setEditText(ebs);
  } //else qDebug("EGS_BATCH_SYSTEM not set");
}

bool EGS_RunPage::addCommandArguments(QStringList &s) {
    if (!pegsless->isChecked()){
       if( pegs_file->text().isEmpty() ) {
           QMessageBox::critical(this,"Error",
                   "You must select a PEGS file first",1,0);
           return false;
       }
       QString pfile; QString look_in = look_for_pegs->currentText();
       QChar ss = QDir::separator();
       if( look_in == "User pegs area" ||
           look_in == "HEN_HOUSE pegs area" ) pfile = pegs_file->text();
       else if ( look_in == "HOME" ) {
           pfile = QDir::homePath();
           if( !pfile.endsWith(ss) ) pfile += ss;
           pfile += pegs_file->text() + ".pegs4dat";
       }
       else
         pfile = look_in + ss + pegs_file->text() + ".pegs4dat";
       s << "-p" << pfile;
    }

    if( !input_file->text().isEmpty() ) {
      s << "-i" << input_file->text();
    }
    return true;
}


void EGS_RunPage::startBatchExecution() {
#ifdef RP_DEBUG
    qDebug("In EGS_RunPage::startBatchExecution()");
#endif
    QString exe = getExecutable(); QStringList args;
    if( exe.isEmpty() ) {
        QMessageBox::critical(this,"Error",
                "You must first compile the executable",1,0);
        return;
    }
    //run_batch->clearArguments();
    QString exb = henHouse() + "scripts/run_user_code_batch";
    //args << exb; //run_batch->addArgument(exb);
    args << user_code; //run_batch->addArgument(user_code);
    args << input_file->text(); //run_batch->addArgument(input_file->text());
    if (!pegsless->isChecked()){
       QString pfile; QString look_in = look_for_pegs->currentText();
       QChar ss = QDir::separator();
       if( look_in == "User pegs area" ) pfile = pegs_file->text();
       else if ( look_in == "HEN_HOUSE pegs area" )
           pfile = henHouse() + "pegs4" + ss + "data" + ss +
               pegs_file->text() + ".pegs4dat";
       else if ( look_in == "HOME" )
           pfile = QDir::homePath() + ss + pegs_file->text() +
               ".pegs4dat";
       else
           pfile = look_in + ss + pegs_file->text() + ".pegs4dat";
       args << pfile;//run_batch->addArgument(pfile);
    }
    else{
       args << "pegsless";
    }
    args << queue->currentText();//run_batch->addArgument(queue->currentText());
    QString the_qs = "batch="; the_qs += queue_system->currentText();
    args << the_qs;//run_batch->addArgument(the_qs);
    if( njob->value() > 1 ) {
        QString p = "p="; p += njob->text();
        args << p;//run_batch->addArgument(p);
    }
    if ( !henHouse().isEmpty() ) {
        QString aux = "hh="; aux += henHouse();
        args << aux;//run_batch->addArgument(aux);
    }
    if ( !egsHome().isEmpty() ) {
        QString aux = "eh="; aux += egsHome();
        args << aux;//run_batch->addArgument(aux);
    }
    if ( !egsConfiguration().isEmpty() ) {
        QString aux = "config="; aux += egsConfiguration();
        qDebug("adding %s to list of arguments",aux.toLatin1().data());
        args << aux;//run_batch->addArgument(aux);
    }
    QString extra = extra_batch_args->text();
    if( !extra.isEmpty() ) {
        //QStringList list = QStringList::split(" ",extra);
        QStringList list = extra.split(" ");
        for(QStringList::iterator it=list.begin(); it != list.end(); it++)
            args << *it;//run_batch->addArgument(*it);
    }

    run_batch->start(exb,args);
    //if( !run_batch->start() )
    if(run_batch->error()==QProcess::FailedToStart)
        QMessageBox::critical(this,"Error","Failed to start exb",1,0);
    else killed = false;
    run_current = run_batch;
    start_b->setEnabled(false); queue->setEnabled(false);
    njob->setEnabled(false); queue_system->setEnabled(false);
    run_options->setEnabled(false);
}

void EGS_RunPage::closeStdin() {
#ifdef RP_DEBUG
    qDebug("In EGS_RunPage::closeStdin()");
#endif
    //run_current->closeStdin();
    //Should be closeWriteChannel(), however not needed
    //since there is no writting to the process after the
    //call to start.
    //Qt3 to Qt4 -- EMH
}

void EGS_RunPage::startExecution() {
  r_text->setText("");
  if( b_button->isChecked() ) {
      startBatchExecution(); return;
  }
#ifdef RP_DEBUG
  qDebug("In EGS_RunPage::startExecution()");
#endif
  QString exe = getExecutable();
  if( exe.isEmpty() ) return;
  //run->clearArguments();
  QStringList args;
  //run->addArgument(exe);
  //QString dum;
  //if( !addCommandArguments(dum,run) ) return;
  if( !addCommandArguments(args) ) return;
  if( !egsHome().isEmpty() ) {
    args << "-e";//run->addArgument("-e");
    args << egsHome();//run->addArgument(egsHome());
  }
  if( !henHouse().isEmpty() ) {
    args << "-H";//run->addArgument("-H");
    args << henHouse();//run->addArgument(henHouse());
  }
  //QStringList list = run->arguments();
  qWarning("Executing: <%s>",args.join(" ").toLatin1().data());

  //if( !run->start() )
  run->start(exe,args);
  if(run->error()==QProcess::FailedToStart)
    QMessageBox::critical(this,"Error","Failed to start user code",1,0);
  run_current = run;
  start_b->setEnabled(false); stop_b->setEnabled(true);
  run_options->setEnabled(false);
}

void EGS_RunPage::stopExecution() {
#ifdef RP_DEBUG
  qDebug("In EGS_RunPage::stopExecution()");
#endif
  run_current->kill(); killed = true;
}

void EGS_RunPage::readProcessOut() {
  QByteArray a = run_current->readAllStandardOutput();
  r_text->insertPlainText(a);
}

void EGS_RunPage::readProcessErr() {
  QByteArray a = run_current->readAllStandardError();
  r_text->insertPlainText(a);
}

void EGS_RunPage::processFinished(int exitCode, QProcess::ExitStatus exitStatus) {
#ifdef RP_DEBUG
  qDebug("In EGS_RunPage::processFinished()");
#endif
  start_b->setEnabled(true); stop_b->setEnabled(false);
  run_options->setEnabled(true);
  if( run_current == run_batch ) {
      njob->setEnabled(true); queue->setEnabled(true);
      queue_system->setEnabled(true);
  }
  if (killed)
       r_text->append(
       "\n\n************************* killed *****************************\n\n");
  else
      if( ! exitStatus )
        r_text->append("\n\n**************** finished *************\n");
      else
        r_text->append("\n\n*********** execution failed *********\n");

}

void EGS_RunPage::checkRunOptions(int id) {
  QAbstractButton *b = bg_run_options->button(id);
  if( !b ) qFatal("Clicked button (%d) is a null widget?",id);
#ifdef RP_DEBUG
  qDebug("Button %d clicked, is on = %d",id,b->isChecked());
#endif
  if( id == 0 ) {
      queue_system->setEnabled(false);
      queue->setEnabled(false);
      njob->setEnabled(false);
  }
  else {
      queue_system->setEnabled(true);
      queue->setEnabled(true);
      njob->setEnabled(true);
  }

}

void EGS_RunPage::go_pegsless(bool checked){

  if (checked){
    pegsFileButton->setEnabled(false);
    pegs_file->setEnabled(false);
    look_for_pegs->setEnabled(false);
  }
  else{
    pegsFileButton->setEnabled(true);
    pegs_file->setEnabled(true);
    look_for_pegs->setEnabled(true);
  }
}

void EGS_RunPage::selectPegsFile() {
#ifdef RP_DEBUG
  qDebug("In EGS_RunPage::selectPegsFile()");
#endif
  QString start_dir; QString look_in = look_for_pegs->currentText();
  QChar ss = QDir::separator();
  if( look_in == "User pegs area" )
    start_dir = egsHome() + "pegs4" + ss + "data";
  else if ( look_in == "HEN_HOUSE pegs area" )
    start_dir = henHouse() + "pegs4" + ss + "data";
  else if( look_in == "HOME" )
    start_dir = QDir::homePath();
  else
    start_dir = look_in;
  QString s = QFileDialog::getOpenFileName(this,tr("Select a PEGS4 data file"),
                                           start_dir, tr("PEGS4 files (*.pegs4dat)"));
#ifdef RP_DEBUG
  qDebug("file: %s",s.toLatin1().data());
#endif
  if ( !s.isEmpty() ) {
    QFileInfo fi(s);
#ifdef RP_DEBUG
    qDebug("is link: %d readLink: %s\n",(int)fi.isSymLink(),
      fi.readLink().toLatin1().data());
#endif

    pegs_file->setText(fi.baseName());
    QDir from_dir = fi.dir(); from_dir.makeAbsolute();
#ifdef RP_DEBUG
    qDebug("from_dir: %s",from_dir.absolutePath().toLatin1().data());
#endif
    QDir d1(egsHome() + "pegs4" + ss + "data");
    d1.makeAbsolute();
#ifdef RP_DEBUG
    qDebug("d1: %s",d1.absolutePath().toLatin1().data());
#endif
    if( from_dir == d1 ) {
      look_for_pegs->setCurrentIndex(0); return;
    }
    QDir d2(henHouse() + "pegs4" + ss + "data");
    d2.makeAbsolute();
#ifdef RP_DEBUG
    qDebug("d2: %s",d2.absolutePath().toLatin1().data());
#endif
    if( from_dir == d2 ) {
      look_for_pegs->setCurrentIndex(1); return;
    }
    QDir d3 = QDir::home(); d3.makeAbsolute();
    QString home = d3.canonicalPath();
    QDir d4(home);
#ifdef RP_DEBUG
    qDebug("d4: %s",d4.absolutePath().toLatin1().data());
#endif
    if( from_dir == d4 ) {
      look_for_pegs->setCurrentIndex(2); return;
    }
    bool is_there = false;
    for(int j=3; j<look_for_pegs->count(); j++) {
      QDir d(look_for_pegs->itemText(j));
      if( from_dir == d ) {
        look_for_pegs->setCurrentIndex(j);
        is_there = true; break;
      }
    }
    if( !is_there ) {
      look_for_pegs->addItem(from_dir.absolutePath());
      look_for_pegs->setCurrentIndex(look_for_pegs->count()-1);
    }
  }
}

void EGS_RunPage::pegsAreaChanged(const QString &new_area) {
  if( pegs_file->text().isEmpty() ) return;
  QDir d; QChar ss = QDir::separator();
  if( new_area == "User pegs area" )
    d.setPath(egsHome() + "pegs4" + ss + "data");
  else if( new_area == "HEN_HOUSE pegs area" )
    d.setPath(henHouse() + "pegs4" + ss + "data");
  else if( new_area == "HOME" )
    d.setPath(QDir::homePath());
  else
    d.setPath(new_area);
  QString pfile = pegs_file->text() + ".pegs4dat";
  if( !d.exists(pfile) ) pegs_file->clear();

}

void EGS_RunPage::setUserCode(const QString &new_uc) {
  user_code = new_uc;
#ifdef RP_DEBUG
  qDebug("In EGS_RunPage::setUserCode: %s",user_code.toLatin1().data());
#endif
  checkExecutable();
}

void EGS_RunPage::selectInputFile() {
#ifdef RP_DEBUG
  qDebug("In EGS_RunPage::selectInputFile()");
#endif
  QString start_dir = egsHome() + user_code;
  QString s = QFileDialog::getOpenFileName(this,tr("Select input file"),
                                           start_dir, tr("EGS input files (*.egsinp)"));
#ifdef RP_DEBUG
  qDebug("file: %s",s.toLatin1().data());
#endif
  if( !s.isEmpty() ) {
    QFileInfo fi(s); input_file->setText(fi.baseName());
  }
}

void EGS_RunPage::setTarget(const QString &new_target) {
#ifdef RP_DEBUG
  qDebug("EGS_RunPage::setTarget: target = %s new_target = %s",
    target.toLatin1().data(),new_target.toLatin1().data());
#endif
  if( new_target != "clean" ) {
    target = new_target;
    checkExecutable();
  }
}

//*********************************************************************
// Gets name of user-code executable. On Windows one needs to append
// extension ".exe" so that exists() works (exe1). However, this name
// is used later on to figure out the user-code directory and hence should
// not have the ".exe" extension (exe).
//*********************************************************************
QString EGS_RunPage::getExecutable() {
  QString exe_null;
  if( !checkVars() ) return exe_null;
  QChar ss = QDir::separator();
  QString exe, exe1 = egsHome() + "bin" + ss + myMachine() +
       ss + user_code;
  if( target != "opt" ) exe1 += "_" + target; exe = exe1;
#ifdef WIN32
  exe1 += ".exe";// So that exists() can find the file
#endif
  QFileInfo fi(exe1);
#ifdef WIN32
  if( !fi.exists() ) {  // isExecutable() appears to not work on Vista
#else
  if( !fi.isExecutable() ) {
#endif
    QMessageBox::critical(this,"Error","You must compile the target first",1,0);
    return exe_null;
  }
  return exe;
}

void EGS_RunPage::checkExecutable() {
#ifdef RP_DEBUG
  qDebug("In EGS_RunPage::checkExecutable()");
#endif
}

bool EGS_RunPage::parseBatchOptions() {
    QString bf = henHouse() + "scripts/batch_options.";
    bf += queue_system->currentText();
#ifdef RP_DEBUG
    qDebug("EGS_RunPage::parseBatchOptions(): reading %s",bf.toLatin1().data());
#endif
    QFile bfile(bf);
    if( !bfile.open(QIODevice::ReadOnly) ) {
        QMessageBox::critical(this,"Error",
                "Could not open the batch specification file",1,0);
        return false;
    }
    bool have_bc=false, have_gbo=false, have_obo=false, have_rbo=false,
         have_bst=false, have_sq=false, have_mq=false, have_lq=false;
    while( !bfile.atEnd() ) {
        char buf[100000];
        if ( bfile.readLine(buf, sizeof(buf)) < 0 ) break;
        QString aux(buf);
        //if( bfile.readLine(aux,100000L) < 0 ) break;
        if( aux.startsWith("#") ) continue;
        int ind;
        if( (ind=aux.indexOf("#")) >= 0 ) aux = aux.left(ind);
        if( aux.isEmpty() ) continue;
        QString what="batch_command=";
        if( getVariable(aux,what,batch_command) ) {
            have_bc = true; continue;
        }
        what="generic_bo="; if( getVariable(aux,what,generic_bo) ) {
            have_gbo=true; continue;
        }
        what="output_bo="; if( getVariable(aux,what,output_bo) ) {
            have_obo=true; continue;
        }
        what="rname_bo="; if( getVariable(aux,what,rname_bo) ) {
            have_rbo=true; continue;
        }
        what="batch_sleep_time="; if( getVariable(aux,what,batch_sleep_time) ) {
            have_bst=true; continue;
        }
        what="short_queue="; if( getVariable(aux,what,short_queue) ) {
            have_sq=true; continue;
        }
        what="medium_queue="; if( getVariable(aux,what,medium_queue) ) {
            have_mq=true; continue;
        }
        what="long_queue="; if( getVariable(aux,what,long_queue) ) {
            have_lq=true; continue;
        }
    }
#ifdef RP_DEBUG
    qDebug("batch_command = <%s>",batch_command.toLatin1().data());
    qDebug("generic_bo = <%s>",generic_bo.toLatin1().data());
    qDebug("rname_bo = <%s>",rname_bo.toLatin1().data());
    qDebug("batch_sleep_time = <%s>",batch_sleep_time.toLatin1().data());
    qDebug("short_queue = <%s>",short_queue.toLatin1().data());
    qDebug("medium_queue = <%s>",medium_queue.toLatin1().data());
    qDebug("long_queue = <%s>",long_queue.toLatin1().data());
#endif
    if( !have_bc || !have_gbo || !have_obo || !have_rbo || !have_bst ||
        !have_sq || !have_mq || !have_lq ) return false;
    return true;
}

bool EGS_RunPage::getVariable(const QString &from, const QString &what,
        QString &var) {
    int ind;
    if( (ind=from.indexOf(what)) >= 0 ) {
        var = from.mid(ind+what.length());
        var = var.simplified();
        if( var.startsWith("\"") ) var = var.mid(1,var.length()-2);
        return true;
    }
    return false;
}
