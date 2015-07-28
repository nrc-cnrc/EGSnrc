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
#
###############################################################################
*/


#include "egs_run_page.h"

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
#include <qobjectlist.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qprocess.h>
#include <qmessagebox.h>
#include <qstringlist.h>

#include <cstdlib>

EGS_RunPage::EGS_RunPage(QWidget *parent, const char *name, WFlags f) :
  EGS_GUI_Widget(parent,name,f) { make(); }

EGS_RunPage::EGS_RunPage(EGS_ConfigReader *cr,
     QWidget *parent, const char *name, WFlags f) :
  EGS_GUI_Widget(cr,parent,name,f) { make(); }

void EGS_RunPage::make() {

  QVBoxLayout *topl = new QVBoxLayout(this);
  topl->setSpacing(6); topl->setMargin(11);

  QVBoxLayout *vl1 = new QVBoxLayout;

  QGroupBox *gb = new QGroupBox(this,"pegs file group");
  gb->setColumnLayout(0,Qt::Horizontal);
  gb->layout()->setSpacing( 6 ); gb->layout()->setMargin( 11 );
  gb->setTitle( tr("PEGS file") );
  QHBoxLayout *hbl = new QHBoxLayout(gb->layout());
  pegs_file = new QLineEdit(gb,"pegs file");
  hbl->addWidget(pegs_file);
  QPushButton *b = new QPushButton("...",gb);
  b->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
  connect(b,SIGNAL(clicked()),this,SLOT(selectPegsFile()));
  hbl->addWidget(b);

  hbl = new QHBoxLayout(gb->layout());
  QLabel *label = new QLabel("Look in:",gb);
  hbl->addWidget(label);
  look_for_pegs = new QComboBox(gb,"look for pegs file");
  look_for_pegs->insertItem("User pegs area");
  look_for_pegs->insertItem("HEN_HOUSE pegs area");
  look_for_pegs->insertItem("HOME");
  connect(look_for_pegs,SIGNAL(activated(const QString &)),this,
            SLOT(pegsAreaChanged(const QString &)));
  hbl->addWidget(look_for_pegs);

  vl1->addWidget(gb);

  gb = new QGroupBox(this,"input file group");
  gb->setColumnLayout(0,Qt::Horizontal);
  gb->layout()->setSpacing( 6 ); gb->layout()->setMargin( 11 );
  gb->setTitle( tr("Input file") );
  hbl = new QHBoxLayout(gb->layout());
  input_file = new QLineEdit(gb,"input file");
  hbl->addWidget(input_file);
  b = new QPushButton("...",gb);
  b->resize(b->minimumSize());
  b->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
  connect(b,SIGNAL(clicked()),this,SLOT(selectInputFile()));
  hbl->addWidget(b);

  vl1->addWidget(gb);

  gb = new QGroupBox(this,"extra options group");
  gb->setColumnLayout(0,Qt::Horizontal);
  gb->layout()->setSpacing( 6 ); gb->layout()->setMargin( 11 );
  gb->setTitle( tr("Extra arguments") );
  hbl = new QHBoxLayout(gb->layout());

  QGroupBox *gb1 = new QGroupBox(gb,"extra user code");
  gb1->setColumnLayout(0,Qt::Horizontal);
  gb1->layout()->setSpacing( 6 ); gb1->layout()->setMargin( 11 );
  gb1->setTitle( tr("User code") );
  QHBoxLayout *hbl1 = new QHBoxLayout(gb1->layout());
  extra_args = new QLineEdit(gb1,"extra args");
  hbl1->addWidget(extra_args);
  hbl->addWidget(gb1);

  gb1 = new QGroupBox(gb,"extra batch");
  gb1->setColumnLayout(0,Qt::Horizontal);
  gb1->layout()->setSpacing( 6 ); gb1->layout()->setMargin( 11 );
  gb1->setTitle( tr("Batch command") );
  hbl1 = new QHBoxLayout(gb1->layout());
  extra_batch_args = new QLineEdit(gb1,"extra batch args");
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
  run_options = new QButtonGroup("Run options",this,"run options group");
  run_options->setOrientation(Qt::Vertical);
  run_options->layout()->setSpacing(6); run_options->layout()->setMargin(11);
  QVBoxLayout *bgl = new QVBoxLayout( run_options->layout() );
  QRadioButton *rb = new QRadioButton("Interactive",run_options,"interactive");
  rb->setChecked(true); bgl->addWidget(rb); i_button = rb;
  rb = new QRadioButton("Batch",run_options,"batch"); bgl->addWidget(rb);
  b_button = rb;
#ifdef WIN32
  b_button->setEnabled(false);
#endif
  /*
  rb = new QRadioButton("Parallel",run_options,"parallel"); bgl->addWidget(rb);
  rb->setEnabled(false); p_button = rb;
  run_options->setRadioButtonExclusive(false);
  */
  connect(run_options,SIGNAL(clicked(int)),SLOT(checkRunOptions(int)));
  vl1->addWidget(run_options);

  gb = new QGroupBox(this,"n_parallel");
  gb->setColumnLayout(0, Qt::Vertical );
  gb->layout()->setSpacing( 6 ); gb->layout()->setMargin( 11 );
  gb->setTitle( tr("Number of jobs") );
  hbl = new QHBoxLayout(gb->layout());
  hbl->setAlignment( Qt::AlignTop );

  njob = new QSpinBox(1,100,1,gb,"njob");
  njob->setEnabled(false);
  hbl->addWidget(njob);
  vl1->addWidget(gb);

  gb = new QGroupBox(this,"queue_type");
  gb->setColumnLayout(0, Qt::Vertical );
  gb->layout()->setSpacing( 6 ); gb->layout()->setMargin( 11 );
  gb->setTitle( tr("Queueing system") );
  hbl = new QHBoxLayout(gb->layout());
  hbl->setAlignment( Qt::AlignTop );

  queue_system = new QComboBox(gb);
  queue_system->setEnabled(false);
  getBatchOptions();
  hbl->addWidget(queue_system);
  vl1->addWidget(gb);

  gb = new QGroupBox(this,"queue");
  gb->setColumnLayout(0, Qt::Vertical );
  gb->layout()->setSpacing( 6 ); gb->layout()->setMargin( 11 );
  gb->setTitle( tr("Queue") );
  hbl = new QHBoxLayout(gb->layout());
  hbl->setAlignment( Qt::AlignTop );
  queue = new QComboBox(gb);
  queue->insertItem("short");
  queue->insertItem("medium");
  queue->insertItem("long");
  queue->setCurrentItem(2);
  queue->setEnabled(false);
  hbl->addWidget(queue);
  vl1->addWidget(gb);

  hl1->addLayout(vl1);

  topl->addLayout(hl1);

  hl1 = new QHBoxLayout;
  gb = new QGroupBox(this,"buttons");
  gb->setColumnLayout(0, Qt::Horizontal);
  gb->layout()->setSpacing( 6 ); gb->layout()->setMargin( 11 );
  hbl = new QHBoxLayout(gb->layout());
  hbl->setAlignment( Qt::AlignTop );

  b = new QPushButton("&Start",gb,"start button");
  connect(b,SIGNAL(clicked()),this,SLOT(startExecution()));
  hbl->addWidget(b); start_b = b;
  QSpacerItem *spacer = new QSpacerItem(20,20,QSizePolicy::Expanding,
                                          QSizePolicy::Minimum);
  hbl->addItem(spacer);
  b = new QPushButton("Sto&p",gb,"stop button");
  connect(b,SIGNAL(clicked()),this,SLOT(stopExecution()));
  b->setEnabled(false);
  hbl->addWidget(b); stop_b = b;

  topl->addWidget(gb);

  r_text = new QTextEdit(this,"run output");
  QFont rtext_font(  r_text->font() );
  rtext_font.setFamily( "Courier [Adobe]" );
  rtext_font.setPointSize( 9 );
  r_text->setFont( rtext_font );

  r_text->setReadOnly(true);
  topl->addWidget(r_text);

  run = new QProcess;
  run_batch = new QProcess;
  connect(run,SIGNAL(processExited()),this,SLOT(processFinished()));
  connect(run,SIGNAL(readyReadStdout()),this,SLOT(readProcessOut()));
  connect(run,SIGNAL(readyReadStderr()),this,SLOT(readProcessErr()));
  connect(run_batch,SIGNAL(processExited()),this,SLOT(processFinished()));
  connect(run_batch,SIGNAL(readyReadStdout()),this,SLOT(readProcessOut()));
  connect(run_batch,SIGNAL(readyReadStderr()),this,SLOT(readProcessErr()));
  connect(run_batch,SIGNAL(wroteToStdin()),this,SLOT(closeStdin()));

}

void EGS_RunPage::getBatchOptions(const QString &) {
    getBatchOptions();
}

void EGS_RunPage::getBatchOptions() {
  QString script_dir = henHouse() + "scripts";
  QDir dir(script_dir);
  dir.setNameFilter("batch_options.*");
  QStringList list = dir.entryList();
#ifdef IK_DEBUG
  qDebug("batch_options in %s:",script_dir.latin1());
#endif
  queue_system->clear();
  for(QStringList::Iterator it=list.begin(); it != list.end(); ++it) {
      QString aux = *it;
      if( !aux.endsWith("~") && !aux.endsWith(".bak") ) {
          aux.replace("batch_options.","");
#ifdef IK_DEBUG
          qDebug("  %s",aux.latin1());
#endif
          queue_system->insertItem(aux);
      }
  }
  char *ebs = getenv("EGS_BATCH_SYSTEM");
  if( ebs ) {
#ifdef IK_DEBUG
      qDebug("EGS_BATCH_SYSTEM: %s",ebs);
#endif
      queue_system->setCurrentText(ebs);
  } //else qDebug("EGS_BATCH_SYSTEM not set");
}

bool EGS_RunPage::addCommandArguments(QString &s, QProcess *process) {
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
        pfile = QDir::homeDirPath();
        if( !pfile.endsWith(ss) ) pfile += ss;
        pfile += pegs_file->text() + ".pegs4dat";
    }
    else
      pfile = look_in + ss + pegs_file->text() + ".pegs4dat";
    if( process ) {
        process->addArgument("-p"); process->addArgument(pfile);
    }
    else {
        s += " -p "; s += pfile;
    }
    if( !input_file->text().isEmpty() ) {
        if( process ) {
            process->addArgument("-i");
            process->addArgument(input_file->text());
        }
        else {
            s += " -i "; s += input_file->text();
        }
    }
    return true;
}



void EGS_RunPage::startBatchExecution() {
#ifdef IK_DEBUG
    qDebug("In EGS_RunPage::startBatchExecution()");
#endif
    /*
       QString exe = getExecutable();
       if( exe.isEmpty() ) return;
       run_batch->clearArguments();
       run_batch->addArgument("a.out");
       QString command_args = exe;
       if( !addCommandArguments(command_args,0) ) return;
       if( !parseBatchOptions() ) {
       QMessageBox::critical(this,"Error",
       "Parse error in batch_options file",1,0);
       return;
       }
       command_args += " -b";
       run_batch->addArgument("./a.out");
       run_current = run_batch;
       run_batch->start();
       run_batch->writeToStdin(command_args);
     */
    QString exe = getExecutable();
    if( exe.isEmpty() ) {
        QMessageBox::critical(this,"Error",
                "You must first compile the executable",1,0);
        return;
    }
    run_batch->clearArguments();
    QString exb = henHouse() + "scripts/run_user_code_batch";
    run_batch->addArgument(exb);
    run_batch->addArgument(user_code);
    run_batch->addArgument(input_file->text());
    /*
    if( input_file->text().isEmpty() )
        run_batch->addArgument("\"\"");
    else
        run_batch->addArgument(input_file->text());
    */
    QString pfile; QString look_in = look_for_pegs->currentText();
    QChar ss = QDir::separator();
    if( look_in == "User pegs area" ) pfile = pegs_file->text();
    else if ( look_in == "HEN_HOUSE pegs area" )
        pfile = henHouse() + "pegs4" + ss + "data" + ss +
            pegs_file->text() + ".pegs4dat";
    else if ( look_in == "HOME" )
        pfile = QDir::homeDirPath() + ss + pegs_file->text() +
            ".pegs4dat";
    else
        pfile = look_in + ss + pegs_file->text() + ".pegs4dat";
    run_batch->addArgument(pfile);
    run_batch->addArgument(queue->currentText());
    //run_batch->addArgument("testing=yes");
    QString the_qs = "batch="; the_qs += queue_system->currentText();
    run_batch->addArgument(the_qs);
    if( njob->value() > 1 ) {
        QString p = "p="; p += njob->text();
        run_batch->addArgument(p);
    }
    if ( !henHouse().isEmpty() ) {
        QString aux = "hh="; aux += henHouse();
        run_batch->addArgument(aux);
    }
    if ( !egsHome().isEmpty() ) {
        QString aux = "eh="; aux += egsHome();
        run_batch->addArgument(aux);
    }
    if ( !egsConfiguration().isEmpty() ) {
        QString aux = "config="; aux += egsConfiguration();
        qDebug("adding %s to list of arguments",aux.latin1());
        run_batch->addArgument(aux);
    }
    QString extra = extra_batch_args->text();
    if( !extra.isEmpty() ) {
        QStringList list = QStringList::split(" ",extra);
        for(QStringList::iterator it=list.begin(); it != list.end(); it++)
            run_batch->addArgument(*it);
    }

    if( !run_batch->start() )
        QMessageBox::critical(this,"Error","Failed to start exb",1,0);
    run_current = run_batch;
    start_b->setEnabled(false); queue->setEnabled(false);
    njob->setEnabled(false); queue_system->setEnabled(false);
    run_options->setEnabled(false);
}

void EGS_RunPage::closeStdin() {
#ifdef IK_DEBUG
    qDebug("In EGS_RunPage::closeStdin()");
#endif
    run_current->closeStdin();
}

void EGS_RunPage::startExecution() {
  r_text->setText("");
  if( b_button->isChecked() ) {
      startBatchExecution(); return;
  }
#ifdef IK_DEBUG
  qDebug("In EGS_RunPage::startExecution()");
#endif
  QString exe = getExecutable();
  if( exe.isEmpty() ) return;
  run->clearArguments();
  run->addArgument(exe);
  QString dum;
  if( !addCommandArguments(dum,run) ) return;
  if( !egsHome().isEmpty() ) {
    run->addArgument("-e");
    run->addArgument(egsHome());
  }
  if( !henHouse().isEmpty() ) {
    run->addArgument("-H");
    run->addArgument(henHouse());
  }
  QStringList list = run->arguments();
  qWarning("Executing: <%s>",list.join(" ").latin1());

  if( !run->start() )
    QMessageBox::critical(this,"Error","Failed to start user code",1,0);
  run_current = run;
  start_b->setEnabled(false); stop_b->setEnabled(true);
  run_options->setEnabled(false);
}

void EGS_RunPage::stopExecution() {
#ifdef IK_DEBUG
  qDebug("In EGS_RunPage::stopExecution()");
#endif
  run_current->tryTerminate();
  //start_b->setEnabled(true); stop_b->setEnabled(false);
}

void EGS_RunPage::readProcessOut() {
  QByteArray a = run_current->readStdout();
  r_text->insert(a);
}

void EGS_RunPage::readProcessErr() {
  QByteArray a = run_current->readStderr();
  r_text->insert(a);
}

void EGS_RunPage::processFinished() {
#ifdef IK_DEBUG
  qDebug("In EGS_RunPage::processFinished()");
#endif
  start_b->setEnabled(true); stop_b->setEnabled(false);
  run_options->setEnabled(true);
  if( run_current == run_batch ) {
      njob->setEnabled(true); queue->setEnabled(true);
      queue_system->setEnabled(true);
  }
  if( run_current->normalExit() ) {
      r_text->append("\n\n**************** success *************\n");
  }
  else {
      r_text->append("\n\n*********** execution failed *********\n");
  }

}

void EGS_RunPage::checkRunOptions(int id) {
  QButton *b = run_options->find(id);
  if( !b ) qFatal("Clicked button (%d) is a null widget?",id);
#ifdef IK_DEBUG
  qDebug("Button %d clicked, is on = %d",id,b->isOn());
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

  /*
  if( id == 0 ) {
    if( b->isOn() ) {
      b_button->setChecked(false); p_button->setEnabled(false);
    }
    else {
      b_button->setChecked(true); p_button->setEnabled(true);
    }
  }
  else if( id == 1 ) {
    if( b->isOn() ) {
      //bo = run_options->find(0); bo->setDown(false);
      //bo = run_options->find(2); bo->setEnabled(true);
      i_button->setChecked(false); p_button->setEnabled(true);
    }
    else {
      //bo = run_options->find(0); bo->setDown(true);
      //bo = run_options->find(2); bo->setEnabled(false);
      i_button->setChecked(true); p_button->setEnabled(false);
    }
  }
  else if( id == 2 ) {
    njob->setEnabled(b->isOn());
  }
  */
}

void EGS_RunPage::selectPegsFile() {
#ifdef IK_DEBUG
  qDebug("In EGS_RunPage::selectPegsFile()");
#endif
  QString start_dir; QString look_in = look_for_pegs->currentText();
  QChar ss = QDir::separator();
  if( look_in == "User pegs area" )
    start_dir = egsHome() + "pegs4" + ss + "data";
  else if ( look_in == "HEN_HOUSE pegs area" )
    start_dir = henHouse() + "pegs4" + ss + "data";
  else if( look_in == "HOME" )
    start_dir = QDir::homeDirPath();
  else
    start_dir = look_in;
  QString s = QFileDialog::getOpenFileName(start_dir,
     "PEGS4 files (*.pegs4dat)",this,"pegs file dialog",
     "Select a PEGS4 data file");
#ifdef IK_DEBUG
  qDebug("file: %s",s.latin1());
#endif
  if ( !s.isEmpty() ) {
    QFileInfo fi(s);
#ifdef IK_DEBUG
    qDebug("is link: %d readLink: %s\n",(int)fi.isSymLink(),
      fi.readLink().latin1());
#endif

    pegs_file->setText(fi.baseName());
    QDir from_dir = fi.dir(); from_dir.convertToAbs();
#ifdef IK_DEBUG
    qDebug("from_dir: %s",from_dir.absPath().latin1());
#endif
    QDir d1(egsHome() + "pegs4" + ss + "data");
    d1.convertToAbs();
#ifdef IK_DEBUG
    qDebug("d1: %s",d1.absPath().latin1());
#endif
    if( from_dir == d1 ) {
      look_for_pegs->setCurrentItem(0); return;
    }
    QDir d2(henHouse() + "pegs4" + ss + "data");
    d2.convertToAbs();
#ifdef IK_DEBUG
    qDebug("d2: %s",d2.absPath().latin1());
#endif
    if( from_dir == d2 ) {
      look_for_pegs->setCurrentItem(1); return;
    }
    QDir d3 = QDir::home(); d3.convertToAbs();
    QString home = d3.canonicalPath();
    QDir d4(home);
#ifdef IK_DEBUG
    qDebug("d4: %s",d4.absPath().latin1());
#endif
    if( from_dir == d4 ) {
      look_for_pegs->setCurrentItem(2); return;
    }
    bool is_there = false;
    for(int j=3; j<look_for_pegs->count(); j++) {
      QDir d(look_for_pegs->text(j));
      if( from_dir == d ) {
        look_for_pegs->setCurrentItem(j);
        is_there = true; break;
      }
    }
    if( !is_there ) {
      look_for_pegs->insertItem(from_dir.absPath());
      look_for_pegs->setCurrentItem(look_for_pegs->count()-1);
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
    d.setPath(QDir::homeDirPath());
  else
    d.setPath(new_area);
  QString pfile = pegs_file->text() + ".pegs4dat";
  if( !d.exists(pfile) ) pegs_file->clear();

}

void EGS_RunPage::setUserCode(const QString &new_uc) {
  user_code = new_uc;
#ifdef IK_DEBUG
  qDebug("In EGS_RunPage::setUserCode: %s",user_code.latin1());
#endif
  checkExecutable();
}

void EGS_RunPage::selectInputFile() {
#ifdef IK_DEBUG
  qDebug("In EGS_RunPage::selectInputFile()");
#endif
  QString start_dir = egsHome() + user_code;
  QString s = QFileDialog::getOpenFileName(start_dir,
     "EGS input files (*.egsinp)",this,"input file dialog",
     "Select input file");
#ifdef IK_DEBUG
  qDebug("file: %s",s.latin1());
#endif
  if( !s.isEmpty() ) {
    QFileInfo fi(s); input_file->setText(fi.baseName());
  }
}

void EGS_RunPage::setTarget(const QString &new_target) {
#ifdef IK_DEBUG
  qDebug("EGS_RunPage::setTarget: target = %s new_target = %s",
    target.latin1(),new_target.latin1());
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
#ifdef IK_DEBUG
  qDebug("In EGS_RunPage::checkExecutable()");
#endif
}

bool EGS_RunPage::parseBatchOptions() {
    QString bf = henHouse() + "scripts/batch_options.";
    bf += queue_system->currentText();
#ifdef IK_DEBUG
    qDebug("EGS_RunPage::parseBatchOptions(): reading %s",bf.latin1());
#endif
    QFile bfile(bf);
    if( !bfile.open(IO_ReadOnly) ) {
        QMessageBox::critical(this,"Error",
                "Could not open the batch specification file",1,0);
        return false;
    }
    bool have_bc=false, have_gbo=false, have_obo=false, have_rbo=false,
         have_bst=false, have_sq=false, have_mq=false, have_lq=false;
    while( !bfile.atEnd() ) {
        QString aux;
        if( bfile.readLine(aux,100000L) < 0 ) break;
        if( aux.startsWith("#") ) continue;
        int ind;
        if( (ind=aux.findRev("#")) >= 0 ) aux = aux.left(ind);
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
#ifdef IK_DEBUG
    qDebug("batch_command = <%s>",batch_command.latin1());
    qDebug("generic_bo = <%s>",generic_bo.latin1());
    qDebug("rname_bo = <%s>",rname_bo.latin1());
    qDebug("batch_sleep_time = <%s>",batch_sleep_time.latin1());
    qDebug("short_queue = <%s>",short_queue.latin1());
    qDebug("medium_queue = <%s>",medium_queue.latin1());
    qDebug("long_queue = <%s>",long_queue.latin1());
#endif
    if( !have_bc || !have_gbo || !have_obo || !have_rbo || !have_bst ||
        !have_sq || !have_mq || !have_lq ) return false;
    return true;
}

bool EGS_RunPage::getVariable(const QString &from, const QString &what,
        QString &var) {
    int ind;
    if( (ind=from.find(what)) >= 0 ) {
        var = from.mid(ind+what.length());
        var = var.simplifyWhiteSpace();
        if( var.startsWith("\"") ) var = var.mid(1,var.length()-2);
        return true;
    }
    return false;
}

