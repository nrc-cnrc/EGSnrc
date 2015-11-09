/*
###############################################################################
#
#  EGSnrc configuration GUI
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


#include "egs_install.h"
#include <fstream>
#include <QPushButton>

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
#define SET_ENV "Appends environment variables to shell resource files (.bashrc, .cshrc, etc)"
#elif defined(Q_OS_WIN32) || defined(WIN32)
#define SET_ENV "Updates user environment variables in the registry"
#else
#define SET_ENV
#endif

QInstallPage::QInstallPage(QWidget * parent, MCompiler *a_m, MCompiler *a_f,
                                              MCompiler *a_c, MCompiler *a_cpp)
             : QWizardPage(parent), ft(0),ct(0), fc(a_f), cc(a_c), cpp(a_cpp), make(a_m),
               config_file(0), procInstall(0), ntasks(0), n_config_steps(0), i_config_steps(0), n_beam_steps(0),
               egs_c_utils_ok(false), installing_beam(false), user_aborted(false), skip_config(false)
{

    setTitle("EGSnrc Installation Page");
    setSubTitle("Configuring EGSnrc core system");

     // the page layout
     QVBoxLayout *topl = new QVBoxLayout(this);
     progressBar = new QProgressBar(this);
     topl->addWidget(progressBar);
     screen = new QTextEdit();screen->setReadOnly(true);screen->ensureCursorVisible();
     topl->addWidget(screen);
     installButton    = new QPushButton("&Install", this);

     QHBoxLayout *hl = new QHBoxLayout;

     QGridLayout *checksLayout = new QGridLayout;

     envCheckBox      = new QCheckBox("&Set environment",this);
     envCheckBox->setToolTip(QString(SET_ENV));
     shortcutCheckBox = new QCheckBox("&Desktop shortcuts",this);
/*     ucCheckBox   = new QCheckBox("&Copy user-codes",this); ucCheckBox->setChecked(true);
     ucCheckBox->setToolTip("Copies tutorials, RZ, and C++ user codes as well as BEAMnrc examples to working area.\n"
                            "BEWARE of overwriting previously modified user codes !!!");*/
/*     beamCheckBox = new QCheckBox("&BEAMnrc install",this); beamCheckBox->setChecked(true);
     beamCheckBox->setToolTip("Configures and builds BEAMnrc system.");*/

     checksLayout->addWidget( envCheckBox, 0, 0); checksLayout->addWidget( shortcutCheckBox, 0, 1);
     //checksLayout->addWidget( ucCheckBox,  0, 2); //checksLayout->addWidget( beamCheckBox,     1, 1);

     hl->addLayout(checksLayout);

     hl->addStretch(5);
     hl->addWidget(installButton);

     topl->addLayout(hl);


     connect(installButton,SIGNAL(clicked()),this,SLOT(start()));

    initializeConnections();

    procInstall = new QProcess(this);
    procInstall->setProcessChannelMode(QProcess::MergedChannels);
    connect( procInstall, SIGNAL(readyReadStandardOutput()),
             this, SLOT(procProgress()) );
    connect( procInstall, SIGNAL(finished(int , QProcess::ExitStatus)),
             this, SLOT(procEnd(int , QProcess::ExitStatus)) );

    //----------------------------------------------------------------------
    // Flags for the building process of the system
    // NUMBER_OF_STEPS is last element of the enum BuildSteps in egs_install.h
    //----------------------------------------------------------------------
    buildOK = new bool[NUMBER_OF_STEPS ];
    for ( ushort i2 = 0; i2 < NUMBER_OF_STEPS; i2++){
        buildOK[i2] = true;
    }

    //skip_config = !needsTests(); Not working right now. mortran3 compilation uses test results. :-(

}

void QInstallPage::initializeConnections()
{
    /* MTest processes: Tests to find system specific functions */

    connect( this, SIGNAL( egsCUtilsCreated() ),
             this, SLOT( test_c_utils() ) ); n_config_steps += 25;
    connect( this, SIGNAL( egsCUtilsTested() ),
             this, SLOT( test_load_beamlib() ) );
    connect( this, SIGNAL( egsCUtilsEnded() ),
             this, SLOT( createSystemFiles() ) );
    connect( this, SIGNAL( egsCUtilsFailed() ),
             this, SLOT( createSystemFiles() ) );
    connect( this, SIGNAL( LoadBeamLibTested() ),
             this, SLOT( createSystemFiles() ) );

    /* Make processes: Only need to go to folder and execute 'make' */

    connect( this, SIGNAL( systemCreated( ushort ) ),
             this, SLOT( buildEGSnrc( ushort ) ) );
    connect( this, SIGNAL( nextBuildStep( ushort )),
             this, SLOT( buildEGSnrc( ushort )) );
    connect( this, SIGNAL( cppSystemCreated( ushort ) ),
             this, SLOT( buildEGSnrc( ushort ) ) );

  /* Environment Configuration */
    connect( this, SIGNAL( cppBuildFinalized() ),
             this, SLOT( copy_user_codes() ) );
    connect( this, SIGNAL( userCodesCopied() ),
             this, SLOT( beamInstall() ) );
    connect( this, SIGNAL( beamDone() ),
             this, SLOT( environmentSetUp() ) );
    connect( this, SIGNAL( skipBeam() ),
             this, SLOT( environmentSetUp() ) );

    connect( this, SIGNAL( environmentSet() ),
             this, SIGNAL( AllDone() ) );

    connect( this, SIGNAL( AllDone() ),
             this, SLOT( resetPage() ) );

}

void QInstallPage::initializePage()
{

    wizard()->button(QWizard::FinishButton)->setEnabled(false);

    screen->clear(); progressBar->reset(); progressBar->hide();

    createDirs(); installButton->setEnabled(true);

    createLogFile(installationDir);
}

void QInstallPage::cleanupPage(){
     switch2EGSnrc();
     procStop();
     if (config_file){
       config_file->close();
       delete config_file; config_file = 0;
    }
}

void QInstallPage::start(){
  setSubTitle("Configuring EGSnrc core system");
  progressBar->show(); progressBar->reset(); screen->clear();
  wizard()->button(QWizard::BackButton)->setEnabled(false);
  wizard()->button(QWizard::FinishButton)->setEnabled(false);
  installButton->setEnabled(false);
  the_time.start();
  buildEGSnrc(corespec);
}

void QInstallPage::abort(){
     user_aborted = true;
     progressBar->hide();
     if (ft) ft->stop();
     if (ct) ct->stop();
     procStop();
     disconnect(installButton,SIGNAL(clicked()),this,SLOT(abort()));
     wizard()->button(QWizard::BackButton)->setEnabled(true);
     wizard()->button(QWizard::FinishButton)->setEnabled(true);
     installButton->setText("&Install");
     connect(installButton,SIGNAL(clicked()),this,SLOT(start()));
     if (installing_beam){
         installing_beam = false;
         switch2EGSnrc();
     }
}

void QInstallPage::switch2EGSnrc(){
     disconnect( this, SIGNAL( nextBuildStep( ushort )),
                 this, SLOT( buildBEAMnrc( ushort )) );
        connect( this, SIGNAL( nextBuildStep( ushort )),
                 this, SLOT( buildEGSnrc( ushort )) );
}

void QInstallPage::procStop(){
     if ( procInstall && procInstall->state() == QProcess::Running ){
        procInstall->waitForFinished(-1);
        //procInstall->terminate();
        //QTimer::singleShot( 20, procInstall, SLOT( kill() ) );
        //procInstall->kill();
     }
}
void QInstallPage::resetPage(){
     wizard()->button(QWizard::BackButton)->setEnabled(true);
     wizard()->button(QWizard::FinishButton)->setEnabled(true);
     //installButton->setText("&Install");
     installButton->setEnabled(true);
     progressBar->setValue( progressBar->maximum() );
     //connect(installButton,SIGNAL(clicked()),this,SLOT(start()));
     //qDebug("Total steps = %d",i_config_steps);
     timeStamp();
}

void QInstallPage::resetProgressBar(const int &nsteps){
     progressBar->setRange( 0, nsteps );
     istep = 0; progressBar->setValue( istep );
}

void QInstallPage::updateProgress(){progressBar->setValue( ++istep);i_config_steps++;}

QString QInstallPage::henHouse(){
  QString the_hen = field("hen_house").toString();
#ifndef WIN32
    if (!the_hen.startsWith(QDir::separator())) the_hen.prepend(QDir::separator());
#endif
    if (!the_hen.endsWith(QDir::separator()))   the_hen.append(QDir::separator());
  return the_hen;
}

QString QInstallPage::egsHome(){
  QString the_home = field("egs_home").toString();
#ifndef WIN32
    if (!the_home.startsWith(QDir::separator())) the_home.prepend(QDir::separator());
#endif
    if (!the_home.endsWith(QDir::separator()))   the_home.append(QDir::separator());
  return the_home;
}

/*
 * Prints message to the log file and APPENDS message to the
 * QTextEdit object InstallationMonitor
*/
void QInstallPage::printProgress( const QString& message, bool new_line )
{
  if ( config_file->isOpen() ){
    if (new_line)
       config_log << message << endl;
    else
       config_log << message;
    config_file->flush();
  }
  // The code below does not work. A new line is always added! :-(
  if (new_line)
     screen->append( message );
  else{
     screen->insertPlainText(message);
  }

  qApp->processEvents();

  screen->ensureCursorVisible();
}
//************************************************************************

/* Creates a log file to store useful information about
 * the installation process.
   This log file is created in $HEN_HOUSE/install_status
*/
void QInstallPage::createLogFile(const QString & dir){
  if (!config_file){
    config_file = new QFile( dir + tr("config_") + my_machine() + tr(".log"));
    if (!config_file->open( QIODevice::WriteOnly | QIODevice::Text ) ){
       QMessageBox::critical(this, tr("Error creating configuration log file"),
                               tr("Check whether you have write permissions in :\n")+
                               dir + tr("\n and start the installation again!"));
       return;
    }
    config_log.setDevice ( config_file );//connects a QTextStream to log file
 }
}

void QInstallPage::createDirs(){
    QChar s = QDir::separator();
    /* Set installation status directory if not there already */
    //installationDir = henHouse() + tr("install_status") + s;
    installationDir = henHouse() + tr("log") + s;
    createDir(installationDir, false, henHouse());
    QDir::setCurrent(installationDir); //SETTING CURRENT DIR ON WRITEABLE AREA !!!!
    /* Create work area directory if not there already */
    createDir(egsHome(),true);
    /* Create work area bin directory if not there already */
    homeBinDir = egsHome() + tr("bin") + s + my_machine() + s;
    createDir(homeBinDir,true);
    /* Create pegs4 directory in work area if not there already */
    createDir(egsHome() + tr("pegs4") + s);
    createDir(egsHome() + tr("pegs4") + s + tr("inputs") + s);
    createDir(egsHome() + tr("pegs4") + s + tr("data")   + s);
    createDir(egsHome() + tr("pegs4") + s + tr("density")+ s);
    /* Create system bin directory if not there already */
    egsBinDir = henHouse() + tr("bin") + s + my_machine() + s;
    createDir(egsBinDir,true);
    /* Create system lib directory if not there already */
    egsLibDir = henHouse() + tr("lib") + s + my_machine() + s;
    createDir(egsLibDir,true);
    /* Create system dso directory if not there already */
    dsoDir = henHouse() + "egs++" + s + "dso" + s + my_machine() + s;
    createDir(dsoDir,true);
}

void QInstallPage::createDir( QString &dir, bool critical, const QString & def){
    if (! QDir().mkpath(dir) ){
       if (critical)
          QMessageBox::critical(this, tr("Error creating ") + dir,
                                      tr("Check whether you have proper permissions!\n")+
                                      tr("Fix this issue before proceeding with install!"));
       else{
          QMessageBox::warning(this, tr("Error creating ") + dir,
                                      tr("Check whether you have proper permissions!\n")+
                                      tr("Fix this issue before proceeding with install!"));
          if (!def.isEmpty()) dir = def;
       }

    }
}

QString QInstallPage::readFile2QString( const QString& fname,
                                          const QString& err ){
    QFile* the_file = new QFile( fname  );
    if ( ! the_file->open( QIODevice::ReadOnly  ) ){
        printProgress( err );
        return QString::null;
    }
    QTextStream the_stream;
    the_stream.setDevice( the_file );
    QString the_string = the_stream.readAll();
//     the_stream.unsetDevice();
    the_file->close();
    return the_string;
}

bool QInstallPage::writeQString2File( const QString& the_string,
                                        const QString& fname ){
    QFile* the_file = new QFile( fname  );
    if ( ! the_file->open( QIODevice::WriteOnly ) ){
        printProgress( (QString)"Error opening file " + fname + " for writing \n" );
        printProgress( (QString)"Check that it exists or could be created ..." );
        return false;
    }
    QTextStream the_stream;
    the_stream.setDevice( the_file );
    the_stream << the_string;
    the_file->close();
    return true;
}

bool QInstallPage::appendQString2File( const QString& the_string, const QString& fname ){
    QFile* the_file = new QFile( fname  );
    if ( ! the_file->open(QIODevice::WriteOnly | QIODevice::Append ))
       return false;
    QTextStream the_stream;
    the_stream.setDevice( the_file );
    the_stream << the_string << endl;
    the_file->close();
    return true;
}
