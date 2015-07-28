/*
###############################################################################
#
#  EGSnrc user interface headers for egs_configure
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
#  Author:          Ernesto Mainegra-Hing, 2004
#
#  Contributors:    Iwan Kawrakow
#
###############################################################################
*/


/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
#include <fstream>
#include <iostream>
#include <string>
#include <cstdio>
#include <stdlib.h>

#include <qfiledialog.h>
#include <qtooltip.h>
#include <qiconset.h>
#include <qpixmap.h>
#include <qmessagebox.h>
#include <qstringlist.h>
#include <qprocess.h>
#include <qwhatsthis.h>
#include <qapplication.h>
#include <qfileinfo.h>
#include <qtimer.h>

#include <qurloperator.h>
#include <qnetwork.h>
#include <qhttp.h>
#include <qlocalfs.h>
#include <qregexp.h>
#include <qsettings.h>
#include <qthread.h>
#include <egs_tools.h>
#include <egs_tests.h>
#include <egs_config_reader.h>

#ifndef WIN32
#include <unistd.h>
#else
#include <windows.h>
#include <Winreg.h>
#endif
#include <sstream>


using namespace std;

#ifdef WIN32
#define X_EXPORT __declspec(dllexport)
#else
#define X_EXPORT
#endif

extern "C" {
X_EXPORT QWizard* create(){return new EGSnrcMP_setup();}
X_EXPORT QWizard* create1( QWidget* p ){return new EGSnrcMP_setup(p,0,false,0);}
X_EXPORT QWizard* create2( QWidget* p, const QString& n ){return new EGSnrcMP_setup(p,n,false,0);}
}

#ifdef WIN32
#define EGS_CONFIG_LOG_NAME "egs_config_win_machine.log"
#else
#define EGS_CONFIG_LOG_NAME "egs_config_unix_machine.log"
#endif

/* last page visited */
short int last_page;

// This is the bin directory of the current config,
// i.e., the config passed to the egs_config plugin.
QString gui_bin_dir;

void EGSnrcMP_setup::init()
{

   //If config file passed as argument, use it as EGS_CONFIG,
   //else set to null
   EGS_CONFIG = (fileExists(this->name()))? (QString)this->name():QString::null;

   QFont courierFont("Courier",10);
   configMonitor->setCurrentFont( courierFont );
   configCPPMonitor->setCurrentFont( courierFont );
   configMonitor->setPaper(this->backgroundBrush());
   configCPPMonitor->setPaper(this->backgroundBrush());
   the_monitor = configMonitor;
   configProgressBar->setTotalSteps( 10 );
   configProgressBar->setPercentageVisible(false);
   configProgressBar->setProgress( -1 );
   configCPPProgressBar->setTotalSteps( 50 );
   configCPPProgressBar->setProgress( -1 );

    tc          = 0;
    ft          = 0;
    ftt         = 0;
    egs         = 0;
    proc        = 0;
    tobj        = 0;
    procInstall = 0;
    procMake    = 0;
    config_file = 0;
    procConf    = 0;
    buildFlag   = 0;
    last_page   = 0;

    configureOnly      = false;
    downloadComponents = false;
    isGNUMake          = false;
    buildingMortran3   = false;
    cancelPressed      = false;

#ifdef WIN32
    environment = get_environment();
#endif

    fc   = new MCompiler();
    cc   = new MCompiler();
    cpp     = new MCompiler();
    /* initialize DSO object */
    dso = new EGS_DSO();
    dso->is_osx=false;
    dso->libpre="lib";
    dso->libext=".so";
    dso->shared="-shared";
    dso->fpic="";
    dso->obje="o";
    dso->eout="-o ";
    dso->extra=tr("-o $@");
    dso->extra_link="";
    dso->lib_link1="-L$(abs_dso) -Wl,-rpath,$(abs_dso)";
    dso->defines="";
    dso->link2_prefix="-l";
    dso->link2_suffix="";
    dso->shared_bundle="";
    dso->libext_bundle="";
    dso->flibs = "";
    cpp_page = 2;

    outflag  = "-o ";  // default   BEWARE OF SPACE AT END
    coutflag = "-o ";  // default  BEWARE OF SPACE AT END
    cppoutflag= "-o ";  // default   BEWARE OF SPACE AT END

    EGSBinDir  = QString::null;
    EGSBinDir0 = QString::null;

    nbuild     = 8;
    buildOK  = new bool[nbuild];
    for ( ushort i = 0; i < nbuild; i++){
        buildOK[i] = true;
   }

#ifndef WIN32
    SPECcheckBox->hide();
#endif

    setupConfigPage();

    // This is the bin directory of the current config,
    // i.e., the config passed to the egs_config plugin.
    // We will use it to copy binaries from there to the
    // new configuration's bin directory.
    gui_bin_dir = hen_house() + tr("bin") + QDir::separator() +
                  ConfNameEdit->text()    + QDir::separator();

    DSOSwitchesgroupBox->setEnabled( false );

    connect ( customCPPcheckBox, SIGNAL(clicked()),
              this, SLOT(show_cpp_switches()));

    connect( backButton(), SIGNAL( clicked() ),
             this, SLOT( clearTheMonitor() ) );
    connect( this, SIGNAL( uncompressingComplete() ),
             this, SLOT( get_win_objects() ) );
    connect( this, SIGNAL( doFortranTests() ),
             this, SLOT( run_tests() ) );
    connect( this, SIGNAL( egsCUtilsCreated() ),
             this, SLOT( test_c_utils() ) );
    connect( this, SIGNAL( egsCUtilsTested() ),
             this, SLOT( test_load_beamlib() ) );
    connect( this, SIGNAL( egsCUtilsEnded() ),
             this, SLOT( createSystemFiles() ) );
    connect( this, SIGNAL( LoadBeamLibTested() ),
             this, SLOT( createSystemFiles() ) );

    connect( this, SIGNAL( gotCompilerOptions() ),
             this, SLOT( guessConfig() ) );
    connect( this, SIGNAL( systemCreated( ushort ) ),
             this, SLOT( buildEGSnrc( ushort ) ) );
    connect( this, SIGNAL(nextBuildStep( ushort )),
             this, SLOT(buildEGSnrc( ushort )) );
    connect( this, SIGNAL( cppSystemCreated( ushort ) ),
             this, SLOT( buildEGSnrc( ushort ) ) );

   if ( ! procInstall ) {
       procInstall = new QProcess( this );
   }
   connect( procInstall, SIGNAL(readyReadStdout()),
            this, SLOT(readBuildingProgress()) );
   connect( procInstall, SIGNAL(readyReadStderr()),
            this, SLOT(readBuildingProgress()) );
   connect( procInstall, SIGNAL(processExited()),
            this, SLOT(getBuildingStatus() ) );

   QString nam = (QString) this->parent()->name();
   if ( nam.lower().contains("gui",true) > 0  &&
        nam.lower().contains("egs",true) > 0 ){
       is_the_egs_gui = true;
       finishButton()->hide();
       cancelButton()->hide();
   }
   else{
       is_the_egs_gui = false;
   }
}

void EGSnrcMP_setup::clearTheMonitor(){
    QWidget* page = currentPage();
    short index = indexOf( page ) + 1;
    if ( index == pageCount()-1){
	//InstallationMonitor->clear();
    }
}

void EGSnrcMP_setup::show_cpp_switches()
{
  if ( customCPPcheckBox->isChecked() ){
     DSOSwitchesgroupBox->setEnabled( true );
  }
  else{
      DSOSwitchesgroupBox->setEnabled( false );
  }
}

void EGSnrcMP_setup::SaveAppSetting()
{
cleanUp();
#ifndef WIN32

#else
 QString  SPECfile   = ironIt( getSpecDir() + SPECcomboBox->currentText() );
 QString smsg;
 if ( SPECcheckBox->isChecked()){
    cleanPath();// cleans PATH from previous EGSnrcMP HEN_HOUSE entries
    config_log <<
    (QString)"\nUpdating environment variable EGS_CONFIGURATON to " +
    SPECfile + (QString)"\n";
    replaceUserEnvironmentVariable( "EGS_CONFIG", SPECfile, &smsg);

    // Update the user path variable with the EGS bin directory
    config_log << "\nSetting EGSnrc bin directory in your path ....\n";
    update_path( EGSBinDir );

    // Update the user path variable with the EGS_HOME bin directory
    config_log << "\nSetting EGS_HOME bin directory in your path ....\n";
    QString egshome  = ironIt( EGS_HOME.simplifyWhiteSpace() + QDir::separator());
    QString HomeBinDir  = ironIt( egshome + (QString)"bin"   +
                              QDir::separator() + ConfNameEdit->text() );
    update_path( HomeBinDir );

    // Update the user path variable with the previewRZ directory
    config_log << "\nSetting previewRZ directory in your path ....\n";
    QString previewRZDir  = ironIt( hen_house() + (QString)"previewRZ" );
    update_path( previewRZDir );

    // Update the user path variable with the dso/win2k-cl directory
    config_log << "\nSetting win2k-cl DSO directory in your path ....\n";
    QString dsoDir  =  ironIt(
                     hen_house()      + QDir::separator()+
                     (QString)"egs++" + QDir::separator()+
                     (QString)"dso"   + QDir::separator()+
                     (QString)"win2k-cl"
                     );
    update_path( dsoDir);

    // Update the user path variable with the dso/$my_machine directory
    config_log << "\nSetting DSO directory in your path ....\n";
            dsoDir  = ironIt(
                      hen_house()     +
                     (QString)"egs++" + QDir::separator()+
                     (QString)"dso"   + QDir::separator()+
                     ConfNameEdit->text()
                     );
    update_path( dsoDir);

    /* If MinGW GNU installed, probably wipped it out from PATH with cleanPath() */
    QString the_gnu = ironIt(hen_house()    + QDir::separator() +
                      QString("gnu") + QDir::separator() );
    if (dirExists( the_gnu )){// set GNU dirs in PATH
        update_path(the_gnu+ironIt(QString("bin")));
        update_path(the_gnu+ironIt(QString("libexec/gcc/mingw32/3.4.2")));
    }

  }
#endif

}

void EGSnrcMP_setup::update_path( const QString& the_dir ){
  QString path = the_dir + (QString)";";
 QString smsg;
  if ( ! prepend2UserEnvironmentVariable( "PATH", path, &smsg) ){
       config_log << "\n Directory " + the_dir +
                     " was already set in the user's PATH. \n";
   }
  else{
       config_log << "\n Directory " + the_dir +
               " successfully set in the user's PATH. \n";
  }
}

/*---------------------------------------------------------------
 *
 * Checks for entries related to the previous HEN_HOUSE or EGS_HOME
 * and removes them. Prevents the PATH variable from cluttering with
 * tons of entries when testing/experimenting with EGSnrcMP on Windows
 * For a regular user, installing EGSnrcMP on one location, this
 * is not necessary, but useful for the developer testing different
 * installations options.
 *
 *---------------------------------------------------------------*/
void EGSnrcMP_setup::cleanPath(){
#ifdef WIN32
  //QString path = get_env( "PATH" );
  QString user_msg;
  QString path = getUserEnvironmentVariable( "PATH", &user_msg);
  config_log << "Cleaning USER PATH = " << path << endl;
  QStringList pathl = QStringList::split( ";", path),
              clean_path,
              henhouses;

  for ( QStringList::Iterator it0 = pathl.begin(); it0 != pathl.end(); ++it0 ) {
        if ( (*it0).contains( "previewRZ",true ) ){
           QString a_hen_house = (*it0).remove(QString("previewRZ"));
           //QString a_hen_house = (*it0).remove("previewRZ",true);
           //QString a_hen_house = (*it0).replace("previewRZ","",true);
           henhouses.append(a_hen_house);
           config_log << tr("Found HEN_HOUSE : ") + a_hen_house << "\n";
        }
  }
  if (henhouses.count()==0){config_log << "\nNo need to clean PATH variable!!!\n";return;}
  for ( QStringList::Iterator it = pathl.begin(); it != pathl.end(); ++it ) {
    for ( QStringList::Iterator it1 = henhouses.begin(); it1 != henhouses.end(); ++it1 ){
        if (!(*it).contains( *it1,false ))
            clean_path.append(*it);
    }
  }
  path = clean_path.join(";");
  QString smsg;
  if ( ! replaceUserEnvironmentVariable( "PATH", path, &smsg) ){
    config_log << "\n Failed cleaning/replacing the PATH\n";
  }
  else{
    config_log << "\n Sucessfully cleaned PATH variable!!!\n";
  }
#else
  config_log << "\n EGSnrcMP_setup::cleanPath() is Windows only\n";
#endif
}

//---------------------------------------------------------------------------------------------
//                        C O N F I G U R A T I O N     P A G E
//                   ------------------------------------------------
//---------------------------------------------------------------------------------------------
void EGSnrcMP_setup::setupConfigPage()
{

    QStringList makeprog;
    QStringList fcompiler;
    QStringList ccompiler;
    QStringList cppcompiler;
    QString coption, cppoption;

#ifdef WIN32
    makeprog.append( "make.exe" );  // GNU make
    makeprog +=     "gmake.exe";    // GNU make
    makeprog += "mingw32-make.exe"; // MinGW make

    fcompiler.append( "mingw32-g77.exe" );    // GNU Fortran
    fcompiler  += "g77.exe";          // GNU Fortran
    fcompiler  += "f77.exe";          // LAHEY Fortran 77
    fcompiler  += "lf95.exe";         // LAHEY Fortran 95
    fcompiler  += "df.exe";           // DEC Compaq Fortran 90/95
    fcompiler  += "fl32.exe";         // MS Fortran 77
    fcompiler  += "ifl.exe";          // Intel Fortran < 8.0
    fcompiler  += "ifort.exe";        // Intel Fortran 8.0
    fcompiler  += "pgf77.exe";        // Portland Group Fortran 77 Compiler

    ccompiler.append( "mingw32-gcc.exe" );    // GNU C
    ccompiler += "gcc.exe";           // GNU C
    ccompiler += "cl.exe";            // MS C/C++ compiler
    ccompiler += "pgcc.exe";          // Portland Group C compiler
    ccompiler += "fcc.exe";           // Fujitsu C compiler

    coption = "-O2 -DWIN32";

    cppcompiler.append( "g++.exe" );
    cppcompiler += "g++4.exe";
    cppcompiler += "mingw32-g++.exe";
    cppcompiler += "cl.exe";

    cppoption = "-O3";

    const char* sep = ";";
#else
    makeprog.append( "make" );
    makeprog +=     "gmake";

    fcompiler.append( "g77" );    // GNU Fortran 77 compiler
    fcompiler  += "g95";          // GNU Fortran 95 Compiler
    fcompiler  += "gfortran";     // GNU Fortran 95 Compiler
    fcompiler  += "f77";          // LAHEY Fortran 77 Compiler
    fcompiler  += "lf95";         // LAHEY Fortran 95 Compiler
    fcompiler  += "pgf77";        // Portland Group Fortran 77 Compiler
    fcompiler  += "pgf90";        // Portland Group Fortran 90 Compiler
    fcompiler  += "ifort";        // Intel Fortran 8.0
    fcompiler  += "xlf";
    fcompiler  << "cf77" << "cft77" << "frt" << "af77"
               << "fort77" << "f90" << "xlf90" << "epcf90"
               << "f95" << "fort" << "xlf95" << "asf77";

    ccompiler.append( "gcc" );             // GNU C
    ccompiler += "cc ";
    ccompiler += "xlc ";
    ccompiler += "cl";
    ccompiler += "c89";

    coption = "-O2";

    cppcompiler.append( "g++" );
    cppcompiler += "g++4";
    cppcompiler += "icc";
    cppcompiler += "icpc";
    cppcompiler += "aCC";
    cppcompiler += "CC";
    cppcompiler += "cxx";
    cppcompiler += "cc++";
    cppcompiler += "FCC";
    cppcompiler += "KCC";
    cppcompiler += "RCC";
    cppcompiler += "xlC_r";
    cppcompiler += "xlC";
    cppcompiler += "gpp";
    cppcompiler += "cl";

    cppoption = "-O3";

    const char* sep = ":";
#endif

    QStringList dummyList;

    makeprog = find_programs_in_system( makeprog , *sep );
    if (makeprog.isEmpty()){
	hasMake = false;
#ifdef WIN32
	makeprog.append("make.exe");
#endif
	MAKEcomboBox->insertStringList( makeprog );
    }
    else{
        dummyList = strip_repetitions( makeprog );
	MAKEcomboBox->insertStringList( dummyList );
	hasMake = true;
    }

    fcompiler = find_programs_in_system( fcompiler , *sep );
    if (fcompiler.isEmpty()){
	//QMessageBox::critical(this,"Fortran compiler not found",NO_FORTRAN_COMPILER,"&OK",0,0,0,-1);
	hasFCompiler = false;
    }
    else{
        dummyList = strip_repetitions( fcompiler );
	FCcomboBox->insertStringList( dummyList );
	getCompilerOptions();
	hasFCompiler = true;
    }



    ccompiler = find_programs_in_system( ccompiler , *sep );
    if (ccompiler.isEmpty()){
	//QMessageBox::critical(this,"C compiler not found",NO_C_COMPILER,"&OK",0,0,0,-1);
	hasCCompiler = false;

    }
    else{
        dummyList = strip_repetitions( ccompiler );
	CComboBox->insertStringList( dummyList );
	CCOlineEdit->setText(coption);
        COUTEdit->setText(coutflag);
	hasCCompiler = true;
    }

    cppcompiler = find_programs_in_system( cppcompiler , *sep );
    if (cppcompiler.isEmpty()){
	hasCPPCompiler = false;
    }
    else{
	if ( CPPComboBox->count() > 0)
	    CPPComboBox->clear();
	CPPComboBox->insertStringList( cppcompiler );
	CPPCOEdit->setText(cppoption);
        CPPOUTEdit->setText(cppoutflag);
	hasCPPCompiler = true;
    }

    guessConfig();

    QString dir = getSpecDir();
    update_files( dir, SPECcomboBox, "*.conf" );

    connect( FCcomboBox, SIGNAL( textChanged(const QString&) ), this,
             SLOT( FCompilerChanged(const QString&) ) );
    connect( CComboBox, SIGNAL( textChanged(const QString&) ), this,
             SLOT( CCompilerChanged(const QString&) ) );
    connect( CPPComboBox, SIGNAL( textChanged(const QString&) ), this,
             SLOT( CPPCompilerChanged(const QString&) ) );

    QToolTip::add( MAKEgroupBox , MAKE_HELP );
    QToolTip::add( FCOlineEdit, FFLAGS );
    QToolTip::add( FCOLabel, FFLAGS );
    QWhatsThis::add ( FCOlineEdit, FFLAGS );
    QWhatsThis::add ( FCOLabel, FFLAGS );
    QToolTip::add( FLIBSEdit, FLIBS );
    QToolTip::add( FLIBSLabel, FLIBS );
    QWhatsThis::add( FLIBSEdit, FLIBS );
    QWhatsThis::add( FLIBSLabel, FLIBS );
    QToolTip::add( SPECgroupBox, SPEC_NAME );
    QWhatsThis::add( SPECgroupBox, SPEC_NAME );

#ifdef WIN32
    QToolTip::add ( CCompilergroupBox, C_COMPILER_WIN );
    QWhatsThis::add ( CCompilergroupBox, C_COMPILER_WIN );
#else
    QToolTip::add ( CCompilergroupBox, C_COMPILER_UNIX );
    QWhatsThis::add ( CCompilergroupBox, C_COMPILER_UNIX );
#endif

}

void EGSnrcMP_setup::FCompilerChanged( const QString & fname )
{
     if ( is_program_in_system(fname) ||
          fileExists( fname )         ||
          itemExists(fname.latin1(),FCcomboBox) ){
         switch_compiler();
     }
}

/*******************************************************************
 * When C compiler changes, it selects the proper flags.
 * ================
 * A word on flags:
 *=================
 * FOUT: is the executable output flag of the Fortran compiler
 *
 * COUT: is the object output flag of the C compiler
 *
 * When compiling an user-code, no matter in what language written,
 * the linking is done using the Fortran compiler after producing the
 * proper extra object files.
 *
 *********************************************************************/
void EGSnrcMP_setup::CCompilerChanged( const QString & cname)
{
  /*
   *  All C compilers seem to accept the "-o " output flag
   *  and some don't even need an output flag.
   **/
#ifdef WIN32
  QString c = cname;
          c.remove(0, 1+c.findRev(QDir::separator()));
  QString coption;
  if (c.lower()== "cl.exe"){             //-Ox max. optimizations
    coption  = "-Ox -DWIN32 -MD -nologo";//-MD links with MSVCRT.LIB
    coutflag = "-Fo";                    //-Fo<name> obj file name
  }
  else{
    coption  = "-O3 -DWIN32";
    coutflag = "-o ";
  }
  cc->setoutflag(coutflag);
  COUTEdit->setText(coutflag);
  CCOlineEdit->setText(coption);
#endif
}


void EGSnrcMP_setup::CPPCompilerChanged( const QString & cname)
{

QString coption;
#ifdef WIN32
  if (cname.lower()== "cl.exe"){         //-Ox max. optimizations
    coption  = "-Ox -MD -nologo";//-MD link with MSVCRT.LIB
    cppoutflag = "-Fo";                    //-Fo<name> obj file name
  }
  else{
    coption  = "-O2";
    cppoutflag = "-o ";
  }
#else
  cppoutflag = "-o ";
  if (cname.lower()== "cl"){
    coption  = "-Ox -Ob2 -GX -GR -DWIN32 -MD -nologo";
  }
  else if (cname.contains("g++",true)>0 ){
    coption  = "-O3 -ffast-math";
  }
  else if (cname.lower()== "icpc"){
    coption  = "-O3 -no-prec-div -fp-model fast=2";
  }
  else if (cname.lower()== "icc"){
    coption  = "-O3 -no-prec-div -fp-model fast=2";
  }
  else{
    coption  = "-O2";
  }
#endif
  CPPOUTEdit->setText(cppoutflag);
  CPPCOEdit->setText(coption);
  cpp->setName(cname);
  cpp->setoutflag(cppoutflag);
  cpp->setOptimization(coption);

  get_dso_switches();//updates dso object

  CPPDEFCEdit->setText(dso->defines);
  CPPPICEdit->setText(dso->fpic);
  CPPSharedCEdit->setText(dso->shared);
  CPPdlopenCEdit->setText(dso->extra);
  CPPdsopathCEdit->setText(dso->lib_link1);
  CPPlink2CEdit->setText(dso->link2_prefix  +
                       QString("some_lib")+
                       dso->link2_suffix);
#ifdef WIN32
  CPPflibsCEdit->setText(dso->flibs);
#else
  //CPPflibsCEdit->setText(dso->flibs); updated by getFlibs process
#endif
  CPPbundleCEdit->setText(dso->shared_bundle);
}

/* option switches and flags taken from configure_c++ script */
void EGSnrcMP_setup::get_dso_switches()
{

QString name = cpp->name();

QString errors = QString::null;

QRegExp rx_linux64( "x86_64-*-linux*",true,true );
QRegExp rx_linux32( "i686-*-linux*",true,true );
QRegExp rx_linux( "*linux*",true,true );
QRegExp rx_darwin( "*-*-darwin**",true,true );
QRegExp rx_win( "*win*",true,true );
QRegExp rx_gnu( "*gnu*",true,true );
QRegExp rx_mingw( "*mingw*",true,true );
QRegExp rx_cygwin( "*cygwin*",true,true );
QRegExp rx_solaris( "*solaris*",true,true );
QRegExp rx_aix( "*ibm-aix*",true,true );

#ifndef WIN32
if (canonicalSys.contains(rx_linux64)){
   dso->fpic="-fPIC";
}
else if(canonicalSys.contains(rx_linux32)||
        canonicalSys.contains(rx_linux)){
   dso->fpic="";// this is the default
}
else if(canonicalSys.contains(rx_darwin)){
   dso->is_osx=false;
   dso->libext=".dylib"; dso->libext_bundle=".so";
   dso->defines="-DOSX"; dso->lib_link1="-L$(abs_dso)";
   dso->shared="-dynamiclib";
   if (name=="g++") dso->shared_bundle="-bundle" ;
   else             dso->shared_bundle="-qmkshrobj";
}
else if(canonicalSys.contains(rx_solaris)){
   dso->lib_link1="-L$(abs_dso) -Wl,-R,$(abs_dso)";
   if (name=="g++") dso->fpic="-fPIC";
   else             dso->fpic="-KPIC";dso->shared="-G";
}
else if(canonicalSys.contains(rx_aix)){
   dso->shared="-G -Wl,-brtl";
   dso->lib_link1="-L$(abs_dso) -Wl,-brtl,-bexpall";
}
else{
 errors += tr("Unsupported architecture : ") +
             canonicalSys +
             tr("\n using flag -fPIC\n");
 dso->fpic="-fPIC";
}
#else
if(canonicalSys.contains(rx_cygwin)){
   dso->libpre=QString::null; dso->libext=".dll";
   if (name.contains("g++")){
      dso->defines="-DCYGWIN";
      dso->extra="-o $@ -Wl,--out-implib,$(@:.dll=.lib)";
   }
   else if (name=="cl.exe"){
      dso->defines="-DWIN32 -DMSVC"; dso->obje="obj";
      dso->extra="-link -DLL -implib:$(@:.dll=.lib) -out:$@";
      dso->eout="-Fe"; dso->lib_link1=QString::null;
      dso->link2_prefix="$(ABS_DSO)"; dso->link2_suffix=".lib";
   }
   else{
    errors += tr("Unknown compiler ")+ name +
              tr(" on cygwin\n. Using defaults.\n");
   }
}
else if(canonicalSys.contains(rx_mingw)||
        canonicalSys.contains(rx_win)  ||
        canonicalSys.contains(rx_gnu)  ){
   dso->libpre=QString::null; dso->libext=".dll";
   dso->defines="-DWIN32";
   if (name.contains("g++")){
      dso->defines="-DWIN32";
      dso->lib_link1="-L$(abs_dso)";
      dso->extra="-o $@ -Wl,--out-implib,$(@:.dll=.lib)";
   }
   else if (name=="cl.exe"){
      dso->defines="-DWIN32 -DMSVC"; dso->obje="obj";
      dso->extra="-link -DLL -implib:$(@:.dll=.lib) -out:$@";
      dso->eout="-Fe"; dso->lib_link1=QString::null;
      dso->link2_prefix="$(ABS_DSO)"; dso->link2_suffix=".lib";
   }
   else{
    errors += tr("Unknown compiler ")+ name +
              tr(" on Windows\n. Using defaults.\n");
   }
}
else{
 errors += tr("Unsupported architecture : ") +
             canonicalSys;
 dso->fpic="";
}
#endif

/******************************************************
 * finding Fortran libraries when linking done in C++ */
/******************************************************
cout << "Fortran compler is " << fc->name() << endl;
QString str = where_is_program(fc->name());
cout << "Located at " << str.latin1() << endl;
str = str.replace( str.find("bin",0,false), 3, "lib" );
cout << "Libraries located at " << str.latin1() << endl;
*/
#ifdef WIN32
QString path = ironIt( hen_house() +
               QDir::separator()+(QString)"gnu"                   +
               QDir::separator()+(QString)"lib"                   +
               QDir::separator() );
if ( name.contains("g++",false) > 0 ){// user's GNU compiler version
 dso->flibs= QString("-lg2c");
}
else{// using any other C++/Fortran compiler combinaton
 path = where_is_program(fc->name());
 if (!path.isEmpty()) path = path.replace( path.find("bin",0,false), 3, "lib" );
 if (name=="cl.exe"){// MS C++ compiler
    if (fc->name()=="gfortran") dso->flibs=path + QString("libgfortran.a");
    else                        dso->flibs=path + QString("libg2c.a");
 }
 else{// any other C++ compiler here, using libg2c.a
    dso->flibs=path + QString("libg2c.a");
    errors += tr("\n No GNU nor MS C++ compiler in use.") +
              tr("\n Using libg2c.a as Fortran library to link with C++")+
              tr("\n If you know this to be wrong, modify the egspp config");
 }
}
#else
  if (fc->name()=="gfortran") dso->flibs="-lgfortran";
  else                        dso->flibs="-lg2c";

  /* runs get_f77_libs: updates gui and dso, cpp_page defined in init */
  if (indexOf(currentPage()) == cpp_page) getFlibs();
#endif
}

/********************************************************************************
 *
 *  Tries to obtain configuration information from environment. If it fails, it
 *  then tries to guess the system's canonical name, OS and processor and combines
 *  these with the selected compiler to suggest a name for the configuration.
 *
********************************************************************************/
void EGSnrcMP_setup::guessConfig()
{
//  Get system environment variables
 if (EGS_CONFIG.isEmpty())EGS_CONFIG = getenv( "EGS_CONFIG" );
 QString my_machine;
// If there is an active configuration get HEN_HOUSE from it
 if ( !EGS_CONFIG.isEmpty() ){
  EGS_ConfigReader* egs = new EGS_ConfigReader(EGS_CONFIG);
  int res = egs->checkConfigFile(EGS_CONFIG);
  if (!res){
   my_machine = egs->getVariable("my_machine",false);
   update_conf_info(EGS_CONFIG);
   if ( ! my_machine.isEmpty() ){//<=== EGS_CONFIG seems OK, don't guess
    return;
   }

  }
 }

/********************************************************/
/* If we get here, we'll try to guess the configuration */
/********************************************************/
QString comp_def = ironIt( FCcomboBox->currentText() );
               //comp_def .remove(0, 1+ comp_def .findRev( SEP ) );
        comp_def .remove(0, 1+comp_def.findRev(QDir::separator()));

#ifdef WIN32
    comp_def.remove(".exe");
    QString confguess;
    QString Processor_Level = "PROCESSOR_LEVEL";
    QString lev = getenv(Processor_Level.latin1());
    if (!lev.isEmpty()) confguess = "i" + lev + "86_pc_";
    else                  confguess = "ix86_pc_";
    QString Operating_System = "OS";
    QString OS = getenv(Operating_System.latin1());
    canonicalSys = confguess + OS;

    QString CONFIGname = canonicalSys + (QString)"-" + comp_def;
         // ^
         // |
         // --- configuration name

    ConfNameEdit->setText( CONFIGname );
    if (!OS.isEmpty()) confguess = confguess + OS + (QString)"-" +
                       comp_def + (QString)".conf";
    else               confguess = confguess+ (QString)"-" +
                       comp_def + "win32.conf";
    SPECcomboBox->setCurrentText ( confguess ) ;
#else
    procConf = new QProcess( this );
    procConf->addArgument( "/bin/sh" );
    connect( procConf, SIGNAL(readyReadStdout()),
             this, SLOT(readConfigName()) );
    if ( !procConf->start() ) {
	// error handling
	cout << "Error executing "
             << (procConf->arguments()).join(" ") << endl;
    }
    else{
	procConf->writeToStdin( config ); //<=== config defined
                                          //     in egs_tools.h
    }
#endif
}

void EGSnrcMP_setup::readConfigName()
{
#ifndef WIN32
    QString s  = procConf->readLineStdout();
    canonicalSys = s;
    QString comp_def = FCcomboBox->currentText();
    QString CONFIGname = s + (QString)"-" + comp_def;//config name
    ConfNameEdit->setText( CONFIGname );
    s += (QString)"-"+comp_def+(QString)".conf";//config file name
    SPECcomboBox->setCurrentText ( s ) ;
    CPPCompilerChanged( CPPComboBox->currentText());//architecture dependent
#endif
}

void EGSnrcMP_setup::switch_compiler(){
/*
    if ( proc ) {
cout << "proc exists ..." << endl;
	while( proc->isRunning() ){
cout << "proc still running ..." << endl;
	    proc->tryTerminate();
	    QTimer::singleShot( 100, proc, SLOT( kill() ) );
	}
        //proc = 0;
    }
    proc = 0;//This must be outside the if-block
             //in case several signals call this
*/
    //if ( proc ) delete proc;
    //proc->clearArguments();
    getCompilerOptions();
}

void EGSnrcMP_setup::getCompilerOptions()
{
    compiler_response = QString::null; // reset this
    QString fc = FCcomboBox->currentText();
    if (!proc) {
       proc = new QProcess( this );
       connect( proc, SIGNAL(processExited()),
                this, SLOT(readCompilerResponse() ) );
    }
    else proc->clearArguments();

    proc->addArgument( fc );

    if ( fc.lower().contains("ifort") ){
	proc->addArgument( "-V" );
    }
    else if ( fc.lower() != "df.exe"){
        proc->addArgument( "-v" );
    }
    if ( !proc->start() ) {
	FCOlineEdit->setText("");
	FOPTEdit->setText("");
	FDEBUGEdit->setText("");
	// error handling
	//cout << "Error executing "
        //     << (proc->arguments()).join(" ") << endl;
    }
}

void EGSnrcMP_setup::gotCompilerResponse(){
    //emit gotCompilerOptions();
}


void EGSnrcMP_setup::readStdOut()
{
   compiler_response += QString( proc->readStdout() );

}


void EGSnrcMP_setup::readStdErr()
{
   compiler_response += QString( proc->readStdout() );
}

void EGSnrcMP_setup::readCompilerResponse()
{
    QString fco = QString( proc->readStdout() );
    QString fce = QString( proc->readStderr() );
    QString fc_out  = fco + fce;
    //QString fc_out  = compiler_response;
    compiler_response = fc_out;
//cout << "\n" << compiler_response << endl;

#ifdef WIN32
    if ( fc_out.find("GNU F77 version",0,true) > -1 ||
         fc_out.find("mingw",0,true)                > -1 ||
         fc_out.find("gcc version",0,true)        > -1  ){
	FCOlineEdit->setText("");
	FOPTEdit->setText("-O3 -ffast-math");
	FDEBUGEdit->setText("-g");
	Version = "GNU";
	fc->setLinkFlag( "-l" );
	fc->setObjExt( "o" );
	outflag = "-o ";               // BEWARE OF SPACE AT END
    }
    else if (fc_out.find("lahey",0,false) > -1 ){
	FCOlineEdit->setText("-nco -nw");
	FOPTEdit->setText("-tp");
	FDEBUGEdit->setText("-g");
	Version = "LAHEY";
	outflag = "-out ";            // BEWARE OF SPACE AT END
	fc->setLinkFlag( "-lib" );
	fc->setObjExt( "obj" );
    }
    else if (fc_out.find("Compaq Visual Fortran",0,false) > -1 ){
	FCOlineEdit->setText("-fpp");
	FOPTEdit->setText("-fast");
	FDEBUGEdit->setText("-g");
	Version = "COMPAQ";
	outflag = "-exe:";           // MUST HAVE NO SPACE AT END
	fc->setLinkFlag( "-libs" );
	fc->setObjExt( "obj" );
    }
    else if (fc_out.find("Power Station",0,true) > -1 ||
             fc_out.find("PowerStation",0,true) > -1  ||
             fc_out.find("Microsoft (R)",0,true) > -1  ){
	FCOlineEdit->setText("/W0");
	FOPTEdit->setText("/Ox /4Ya");// /G5 increases performance
                                      // for Pentium and compatible
	FDEBUGEdit->setText("/debug");
	Version = "MSPS";
	outflag = "Fe"; // <=== MUST HAVE NO SPACE AT END
	fc->setObjExt( "obj" );
    }
    else if (fc_out.find("Intel(R)",0,true) > -1          &&
             (FCcomboBox->currentText()).contains("ifl")){
	//FCOlineEdit->setText("/W0 /4Na /4Yportlib /Qfpp");
	FCOlineEdit->setText("/W0 /4Yportlib /Qfpp");
	FOPTEdit->setText("/Ox");
	FDEBUGEdit->setText("/debug");
	Version = "INTEL < 8.0";
	outflag = "-o ";           // BEWARE OF SPACE AT END
	fc->setObjExt( "obj" );
    }
    else if (fc_out.find("Intel(R)",0,true) > -1          &&
             (FCcomboBox->currentText()).contains("ifort")){
	FCOlineEdit->setText("-fpp");
	FOPTEdit->setText("-O3");      // -QxK -Qip (P-III and Athlons)
	FDEBUGEdit->setText("-debug"); // -Qx[W,P, etc] P-IV
	Version = "INTEL 8.x";
	outflag = "-o ";           // BEWARE OF SPACE AT END
	fc->setObjExt( "obj" );
    }
    else if (fc_out.find("PGI Fortran compiler",0,true) > -1 ||
              fc_out.find("pgf77",0,true) > -1  ||
              fc_out.find("PGFTN",0,true) > -1  ){
	FCOlineEdit->setText("");
	FOPTEdit->setText("-fast");
	FDEBUGEdit->setText("-g");
	outflag = "-o ";           // BEWARE OF SPACE AT END
	Version = "PGI";
    }
    else{
	FCOlineEdit->setText("");
	FOPTEdit->setText("");
	FDEBUGEdit->setText("");
	Version = "DUNO !!!";
    }
#else
    fc->setLinkFlag( "-l" );
    fc->setObjExt( "o" );
    outflag = "-o ";              // BEWARE OF SPACE AT END
    if ( fc_out.find("GNU F77 version",0,true) > -1  ||
         fc_out.find("gcc version",0,true)        > -1  ){
	FCOlineEdit->setText("");
	FOPTEdit->setText("-O3 -ffast-math");
	FDEBUGEdit->setText("-g");
	Version = "GNU";
    }
    else if (fc_out.find("DEC Fortran compiler",0,true) > -1 ){
	//FCOlineEdit->setText("-noautomatic");
	FOPTEdit->setText("-fast");
	FDEBUGEdit->setText("-g -C");
	Version = "DEC";
    }
    else if (fc_out.find("Intel(R)",0,true) > -1 ){
	FCOlineEdit->setText("");
	FOPTEdit->setText("-O3");
	FDEBUGEdit->setText("-g");
	Version = "INTEL";
    }
    else if (fc_out.find("SGI Fortran compiler",0,true) > -1 ){
	//FCOlineEdit->setText("-static");
	FOPTEdit->setText("-O3");
	FDEBUGEdit->setText("-g -C");
	Version = "SGI";
    }
    else if (fc_out.find("IBM XL Fortran compiler",0,true) > -1 ){
	//FCOlineEdit->setText("-qsave");
	FOPTEdit->setText("-O4");
	FDEBUGEdit->setText("-g -C");
	Version = "IBM";
    }
    else if (fc_out.find("Sun Fortran compiler",0,true) > -1 ){
	FCOlineEdit->setText("");
	FOPTEdit->setText("-fast");
	FDEBUGEdit->setText("-g -C");
	Version = "SUN";
    }
    else if (fc_out.find("HP Fortran compiler",0,true) > -1 ){
	FCOlineEdit->setText("+E1 +E4 +U77 -K");
	FOPTEdit->setText("+O4");
	FDEBUGEdit->setText("-g -C");
	Version = "HP";
    }
    else if (fc_out.find("PGI Fortran compiler",0,true) > -1 ||
	     fc_out.find("pgf77",0,true) > -1  ||
	     fc_out.find("PGFTN",0,true) > -1  ){
	FCOlineEdit->setText("");
	FOPTEdit->setText("-fast");
	FDEBUGEdit->setText("-g");
	Version = "PGI";
    }
    else{
	FCOlineEdit->setText("");
	FOPTEdit->setText("");
	FDEBUGEdit->setText("");
	Version = "DUNO !!!";
    }
#endif

    FOUTEdit->setText(outflag);
    fc->setoutflag(outflag);
    fc->setVersion(Version);
//cout << "Version is:" << Version << endl;

    //    emit gotCompilerOptions();
}
void EGSnrcMP_setup::getCompiler()
{
    QString s = QFileDialog::getOpenFileName(QString::null,
            "all (*)",this,"open file dialog","Choose a compiler" );
    if (!s.isEmpty()){
        FCcomboBox->setEditText(s);
	FCcomboBox->insertItem (  s, 0 ) ;
	hasFCompiler = true;
        switch_compiler();
    }
}

void EGSnrcMP_setup::getCCompiler()
{
    QString s = QFileDialog::getOpenFileName(QString::null,"all (*)",this,
                                  "open file dialog","Choose a compiler" );
    if (!s.isEmpty()){
	CComboBox->insertItem (  s, 0 ) ;
	hasCCompiler = true;
    }
}

void EGSnrcMP_setup::getMake()
{
    QString s = QFileDialog::getOpenFileName(QString::null,"all (*)",this,
                "open file dialog","Choose make version" );
    if (!s.isEmpty()){
	MAKEcomboBox->insertItem (  s, 0 ) ;
	hasMake = true;
    }
}

QString EGSnrcMP_setup::getSpecDir()
{
    QString dir = HEN_HOUSE;
    if (!dir.isEmpty())  dir += QDir::separator() + (QString)"specs" +
                                QDir::separator();
    else                 dir = QString::null;
    return ironIt( dir );
}

void EGSnrcMP_setup::getSpec()
{
    QString dir = getSpecDir();
    QString s = QFileDialog::getSaveFileName(dir,"configuration file (*.conf)",
                             this,"open file dialog","Choose a cofiguration" );
    if (!s.isEmpty()){
        //s.remove(0, 1+ s.findRev( QDir::separator()) );
        update_conf_info( ironIt( s ) );
        //SPECcomboBox->insertItem (  s, 0 ) ;
    }

}

// This is called from two different places:
//  -    After selecting a config file from the SPEC Open File Box
// or
//  -   when selecting a config file from the SPEC Combo Box.
void EGSnrcMP_setup::update_conf_info( const QString & conf )
{
 QString confi = conf;
 confi.remove(0, 1+ confi.findRev( QDir::separator()) );
 SPECcomboBox->setCurrentText(confi);

 QString f=(conf.findRev(QDir::separator())<0)?ironIt(getSpecDir()+conf):conf;

 QString my_machine; QString make_prog;
 QString the_make; QString the_fortran; QString the_c;
 QString the_fopt; QString the_fcflags; QString the_fdebug;
 QString the_flibs;
 QString the_cflags; QString the_cout; QString the_fout;



 if ( !f.isEmpty() ){
  EGS_ConfigReader* egs = new EGS_ConfigReader(f);
  int res = egs->checkConfigFile(f);
  if (!res){
   HEN_HOUSE    = egs->getVariable("HEN_HOUSE",true);
   EGS_HOME    = egs->getVariable("EGS_HOME",true);
   my_machine   = egs->getVariable("my_machine",false);
   canonicalSys = egs->getVariable("canonical_system",false);
   make_prog    = egs->getVariable("make_prog",true);
   the_fortran  = egs->getVariable("F77",true);
   the_fopt      = egs->getVariable("FOPT",true);
   the_fcflags   = egs->getVariable("FCFLAGS",true);
   the_fdebug    = egs->getVariable("FDEBUG",true);
   the_flibs     = egs->getVariable("FLIBS",true);
   outflag       = egs->getVariable("FOUT",false);
   the_c         = egs->getVariable("CC",true);
   the_cflags    = egs->getVariable("C_FLAGS",true);
   the_cout      = egs->getVariable("COUT",true);
  }
  else if(res == 1){
   qDebug("Could not find or read config file: %s" ,f.latin1());
   QString s = conf;
   s.remove( ".conf" );s.remove(".spec");
   ConfNameEdit->setText( s );
  }
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
    if( answer != QMessageBox::Ok ){
        qDebug("Wrong config file. Configuration update aborted!");
        return;
    }
  }
 }
 else{
  qDebug("Empty config file pased to update_conf_info!");
  return;
 }

 if ( my_machine.isEmpty() ){// didn't find my_machine, something is wrong
    qDebug("variable %s not found in configuration file %s","my_machine",
           conf.latin1());
    my_machine = QString::null;
 }
 else{
    ConfNameEdit->setText( my_machine );
 }

 if (canonicalSys.isEmpty())
#ifdef WIN32
    canonicalSys = "i86-pc-win32";
#else
    canonicalSys = "i86-pc-linux";
#endif
 if ( !the_make.isEmpty() )
    MAKEcomboBox->setCurrentText( the_make);

 if ( !the_fortran.isEmpty() ){
   if (the_fortran.lower() != FCcomboBox->currentText().lower()){
       disconnect( FCcomboBox, SIGNAL( textChanged(const QString&) ),
       this, SLOT( FCompilerChanged(const QString&) ) );
       FCcomboBox->setCurrentText( the_fortran);
       connect( FCcomboBox, SIGNAL( textChanged(const QString&) ),
       this, SLOT( FCompilerChanged(const QString&) ) );
       FOPTEdit->setText(the_fopt);
       FCOlineEdit->setText(the_fcflags);
       FOUTEdit->setText(outflag);
       FDEBUGEdit->setText(the_fdebug);
       FLIBSEdit->setText(the_flibs);
    }
 }

 if ( !the_c.isEmpty() ){
   disconnect( CComboBox, SIGNAL( textChanged(const QString&) ),
   this, SLOT( CCompilerChanged(const QString&) ) );
   CComboBox->setCurrentText( the_c);
   connect( CComboBox, SIGNAL( textChanged(const QString&) ),
   this, SLOT( CCompilerChanged(const QString&) ) );
   COUTEdit->setText(the_cout);
   CCOlineEdit->setText(the_cflags);
 }

 if ( outflag.isEmpty() ){
  outflag = "-o ";
 }

 CPPCompilerChanged( CPPComboBox->currentText());//architecture dependent

 henhouseLabel->setText(tr("HEN_HOUSE: ") + HEN_HOUSE);
 egshomeLabel->setText(tr("EGS_HOME: ") + EGS_HOME);

 update_files( getSpecDir(), SPECcomboBox, "*.conf" );

}

/* use script to determine Fortran libraries when linking with C++ */
void EGSnrcMP_setup::getFlibs()
{
#ifndef WIN32
 QString for_c = ironIt( FCcomboBox->currentText() );
 QString cpp_c = ironIt( CPPComboBox->currentText() );
 if (FCcomboBox->currentText().isEmpty()||
     CPPComboBox->currentText().isEmpty())
 {
     if (config_file){
      config_log << "\n EGSnrcMP_setup::getFlibs: No Fortran or C++ compiler!\n";
      config_log << "Fortran : " << for_c.latin1()<< endl;
      config_log << "C++     : " << cpp_c.latin1()<< endl;
     }
     else{
      cout << "\n EGSnrcMP_setup::getFlibs: No Fortran or C++ compiler!\n";
      cout << "Fortran : " << for_c.latin1()<< endl;
      cout << "C++     : " << cpp_c.latin1()<< endl;
     }
     dso->flibs = "";
     CPPflibsCEdit->setText(dso->flibs);
     return;
 }
 if (!procConf) {
    procConf = new QProcess( this );
    QDir d(EGSStatDir);
    procConf->setWorkingDirectory( d );
 }
 else {
    procConf->clearArguments();
    procConf->disconnect( SIGNAL( readyReadStdout() ) );
    procConf->disconnect( SIGNAL( readyReadStderr() ) );
 }
 QString the_script = hen_house() + QString("scripts/get_f77_libs1");
 procConf->addArgument( the_script.latin1() );
 procConf->addArgument( for_c.latin1() );
 procConf->addArgument( cpp_c.latin1() );
 connect( procConf, SIGNAL(readyReadStdout()), this, SLOT(readFLIB()) );
 connect( procConf, SIGNAL(readyReadStderr()), this, SLOT(readErrFLIB()) );
 if ( !procConf->start() ) {
    cout << "Error executing " << (procConf->arguments()).join(" ") << endl;
    CPPflibsCEdit->setText(dso->flibs);
 }
#endif
}

void EGSnrcMP_setup::readErrFLIB(){
    QString err = procConf->readLineStderr();
    CPPflibsCEdit->setText(dso->flibs);
}
void EGSnrcMP_setup::readFLIB(){
    if (!procConf->canReadLineStdout()) return;
    QString s  = procConf->readLineStdout();
    dso->flibs = s.remove("fortran_libs = ");
    CPPflibsCEdit->setText(dso->flibs);
}

//--------------------------------------------------------------
//             I N S T A L L A T I O N     P A G E
//         --------------------------------------------
//--------------------------------------------------------------

/*------------------------------------------------------------------
This function is called after pressing the Finish button.
But the installation might have to be done before that !!!
------------------------------------------------------------------ */
void EGSnrcMP_setup::stopProc( QProcess* p ){
      if ( p ){
           if ( p->isRunning() ){
                p->tryTerminate();
                QTimer::singleShot( 100, p, SLOT( kill() ) );
           }
           printProgress(  (QString) "\n User stopped execution of "
			   + (p->arguments()).join(" ")
			   + (QString)"\n");
           p->clearArguments();
      }
}

void EGSnrcMP_setup::setGUIReady( bool enabled ){
    setButtons(enabled);
}

void EGSnrcMP_setup::setButtons( bool enabled ){
  if ( is_the_egs_gui )
    finishButton()->hide();
  else
    setFinishEnabled( currentPage(), enabled );
  setBackEnabled( currentPage(), enabled );
  cancelButton()->setEnabled(enabled);
}

void EGSnrcMP_setup::install_egs()
{
 configProgressBar->updateGeometry();
 setBackEnabled( currentPage(), false );
 setNextEnabled( currentPage(), false );

    QString fcompiler  = FCcomboBox->currentText();
    QString fcoptions  = FCOlineEdit->text();
    fc->setName( FCcomboBox->currentText() );
    fc->setOptions( FCOlineEdit->text() );
    fc->setOptimization( FOPTEdit->text() );
    fc->setDebug( FDEBUGEdit->text() );
    fc->setLibs( FLIBSEdit->text() );
    cc->setName( CComboBox->currentText() );
    cc->setOptions( CCOlineEdit->text() );
    cc->setOptimization( "-O2" );
    cc->setLibs( CLIBSEdit->text() );

    outflag = FOUTEdit->text();
    fc->setoutflag(FOUTEdit->text());
    cc->setoutflag(COUTEdit->text());

    printProgress(
            tr("\n===>  Creating an EGSnrc configuration  \n\n") );
    copy_binaries();
    emit uncompressingComplete( );

}

// On Linux/Unix:
// =============
// If one is running any of the guis and
// creating a new configuration, it basically means
// the guis can be also run with the new configuration,
// therefore we are making a copy from the original bin
// directory to the new one if they are different.
//
// On Windows:
// ==========
// The executables for the guis and the configuration
// plugin are copied from pieces/windows with the method
// get_win_objects which is common for both egs_install
// and egs_configure (in egs_sys_install.cpp).
void EGSnrcMP_setup::copy_binaries(){

#ifndef WIN32
  QString new_bin_dir = hen_house() + tr("bin") + QDir::separator() +
                        ConfNameEdit->text()    + QDir::separator();

  if (new_bin_dir.lower() == gui_bin_dir.lower()){
     return;
  }

  printProgress("\n\n===> Copying executables for new configuration ...\n");

  printProgress("     + Copying egs_gui               ... ");
  if ( copy_files( gui_bin_dir, new_bin_dir,  "egs_gui*" ) )
   printProgress("OK \n");
  else
   printProgress("failed \n");

  printProgress("     + Copying egs_inprz             ... ");
  if ( copy_files( gui_bin_dir, new_bin_dir,  "egs_inprz*" ) )
   printProgress("OK \n");
  else
   printProgress("failed \n");

  printProgress("     + Copying configuration plugin  ... ");
  if ( copy_files( gui_bin_dir, new_bin_dir,  "libegsconfig.*" ) )
   printProgress("OK \n");
  else
   printProgress("failed \n");

  printProgress("     + Copying egs_view  ... ");
  if ( copy_files( gui_bin_dir, new_bin_dir,  "egs_view.*" ) )
   printProgress("OK \n");
  else
   printProgress("failed \n");

  printProgress("\n\n===> Making gui's executable ...\n");
  chmodv( "a+x",new_bin_dir + tr("*") );
#endif

}

void EGSnrcMP_setup::chmodv( const QString& attrib, const QString& file )
{
 QString p  = "chmod " + attrib;
         p += " " + file + " &";
 if ( system( p ) )
   QMessageBox::warning ( 0, "chmod", "failed on " + file, 1, 0, 0 );
 else
   printProgress( (QString)"       -> Changing attribute of " + file + (QString)" to " +
                  attrib + (QString)" ...\n");
}

/*--------------------------------------------------------------------------
                       B U I L D I N G     E G S n r c
---------------------------------------------------------------------------- */

void EGSnrcMP_setup::buildEGSnrc( ushort code  ){
QString the_home  = ironIt( EGS_HOME.simplifyWhiteSpace() + QDir::separator());
QString the_house = ironIt( HEN_HOUSE.simplifyWhiteSpace() + QDir::separator());
QString conffile = SPECcomboBox->currentText();
bool OK = true;
ushort i; QString OS;
#ifdef WIN32
    QString system_installed(WIN_EGS_INSTALLED);
    OS = "Windows";
#else
    QString system_installed(UNIX_EGS_INSTALLED);
    OS = "Unix/Linux";
#endif

    switch( code ){
    case mortran3:
	buildFlag = pegs4;
        buildMortran3();
	break;
    case pegs4:
        configProgressBar->setProgress( ++config_step );
        qApp->processEvents();
	buildFlag = sysDone;
        buildPegs4();
	break;
    case iaea:
        config_step+=5;
        configCPPProgressBar->setProgress( config_step );
        qApp->processEvents();
        buildFlag = egspp;
        buildIAEALib();
        break;
    case egspp:
        config_step+=15;
        configCPPProgressBar->setProgress( config_step );
        qApp->processEvents();
        buildFlag = cppDone;
        buildEGSPPLib();
        break;
    case sysDone:
#ifndef WIN32
          create_additions();
#endif
          OK = true;
          OK = OK && buildOK[mortran3];
          OK = OK && buildOK[pegs4];

          if ( OK ){
             printProgress(QString("\n\n===> EGSnrc succesfully configured for ")+
                       OS + QString(".\n     Please proceed to the next page to configure")
                          + QString("\n     the C++ portion of EGSnrc.\n")
             );
          }
          else{
             printProgress((QString)"\n\n***** EGSnrc system configuration FAILED, " +
                           (QString)"see log file for details  *****\n\n");
             setBackEnabled( currentPage(), true );
             setNextEnabled( currentPage(), false );
             return;
          }
          finalize_egs_setup(); // WE ARE STOPPING RIGHT HERE
          break;
    case cppDone:
         finalize_cpp();
         break;
    default:
          break;
    }
}

void EGSnrcMP_setup::finalize_cpp(){
    QString s = tr("\n\n"
         "*********************************************\n"
         " Finished creating/modifying configuration machine !\n"
         " To create/modify another configuration go back\n"
         " to the first page.\n"
         "*********************************************\n");
    s.replace(QString("machine"), ConfNameEdit->text() );
    printProgress(s);
    configCPPMonitor->find ("Finished",true,true,false,0,0);
    configCPPProgressBar->setProgress( configCPPProgressBar->totalSteps() );
    SaveAppSetting();//updates path and puts gui icons&folders if requested
    setButtons(true);
}

void EGSnrcMP_setup::finalize_egs_setup(){
 SaveAppSetting();
 setBackEnabled( currentPage(), true );
 setNextEnabled( currentPage(), true );
 configMonitor->find ("===>",true,true,false,0,0);
 configProgressBar->setProgress( configProgressBar->totalSteps() );
}

//Now create cshrc and bashrc addition files
void EGSnrcMP_setup::create_additions(){

}

void EGSnrcMP_setup::buildEGSCode( const QString& workDir ){
    QString egshome = ironIt( EGS_HOME.simplifyWhiteSpace() + QDir::separator() );
            egshome += QDir::separator();

   QDir d( ironIt( workDir ) );
   if ( ! d.exists() ){
             printProgress((QString)"\n There is no " + workDir +
		           (QString)"directory ! \n" +
                           (QString)"\n Installation stopped ! \n" );
	  return;
   }
   procInstall->setWorkingDirectory ( workDir );
   procInstall->clearArguments();
   procInstall->addArgument( MAKEcomboBox->currentText() );
   procInstall->addArgument( (QString)"EGS_CONFIG="
			     + ironIt( getSpecDir() + SPECcomboBox->currentText() ) );
   procInstall->addArgument( (QString)"EGS_HOME=" + egshome );
    if ( ! procInstall->start() ) {
       QString err = "Error executing " + ( procInstall->arguments()).join(" ") + "\n";
       printProgress( err );
      setButtons( true );
    }
}

void EGSnrcMP_setup::printBuildResults(){

    if ( ! buildOK[ mortran3 ] ){
     printProgress(tr("\n\n\n       -       Failed building MORTRAN3.")+
                   tr(" You wont be able to run the EGSnrc system.\n") );
    }
    if ( ! buildOK[ pegs4 ] ){
     printProgress("\n       -       Failed building the data pre-processor PEGS4.\n"  );
    }
}

/**********************************************************
 Every time a page is selected, this slot will be invoked.
 Used to validate information in the current and sometimes
 the previous page.
***********************************************************/
void EGSnrcMP_setup::processPage( const QString & the_title )
{
    QWidget* pagina = currentPage();
    short index = indexOf( pagina );

    /***************************************************/
    /* Fortran and C compiler setup + config file name */
    /***************************************************/
    if ( index == 1 && last_page < 1) {
#ifndef WIN32
      getFlibs();// runs script get_f77_libs: updates gui and dso
#endif
       configMonitor->clear();
       configProgressBar->setProgress( -1 );
       config_step = 0;
       the_monitor = configMonitor;
       qApp->processEvents();

       /* Everytime HEN_HOUSE changes, so does the install_status location */
       EGSStatDir =  ironIt( HEN_HOUSE + QDir::separator() +
             QString("install_status") + QDir::separator() );
       if ( ! checkDir( EGSStatDir ) ){
          back(); return;
       }
       QDir::setCurrent(EGSStatDir); //SETTING CURRENT DIR ON WRITEABLE AREA !!!!
       QString the_conf_file = EGSStatDir + QString(EGS_CONFIG_LOG_NAME);
       the_conf_file.replace(QString("machine"),ConfNameEdit->text());
       if (!config_file ){
          config_file = new QFile( the_conf_file );
       }
       else{//already exists
        config_log.unsetDevice();
        config_file->close();
        config_file->setName(the_conf_file);
       }

       if (!config_file->open( IO_WriteOnly  ) ){
          perror("Error was ");
          qFatal("Error creating configuration log file ");
          QMessageBox::critical(this,"Error creating configuration log file ",
	                             "Check whether you have write permissions "
                                     "in : \n" + QDir::currentDirPath () +
            	                     "\n and start the installation again!",
       	                             "&OK", 0,0,0,-1);
       }
       config_log.setDevice ( config_file );

       EGSLibDir =   ironIt( HEN_HOUSE + QDir::separator() +
                        (QString)"lib" + QDir::separator() +
                        ConfNameEdit->text() );
       EGSBinDir =   ironIt( HEN_HOUSE + QDir::separator() +
                        (QString)"bin" + QDir::separator() +
                        ConfNameEdit->text() );
       QString HomeBinDir = ironIt( EGS_HOME + QDir::separator() +
                        (QString)"bin" + QDir::separator() +
                        ConfNameEdit->text() );

       QString  SPECfile   = ironIt( getSpecDir() +
                       SPECcomboBox->currentText() );
       QString  SpecDOSXYZ = ironIt( getSpecDir() + QString("dosxyznrc_") +
                       ConfNameEdit->text() );

       if ( ! dirExists( EGSLibDir ) ){
           if ( ! make_dir( EGSLibDir ) ) {
	       back(); return;
           }
       }
       if ( ! dirExists( EGSBinDir ) ){
           if ( ! make_dir( EGSBinDir ) ) {
	       back(); return;
           }
       }
       if ( ! dirExists( HomeBinDir ) ){
           if ( ! make_dir( HomeBinDir ) ) {
               back(); return;
           }
       }

       if ( dirExists( EGSLibDir ) &&
            dirExists( EGSBinDir ) &&
            fileExists( SPECfile ) ){
   	  switch( QMessageBox::warning( this, "EGSnrc setup",
	      "Configuration " + ConfNameEdit->text() +
              " already exists.\n"  "Do you want to replace it ?",
              "&Yes", "&No",0,
	      0,      // Enter == button 0
	      1 ) ) { // Escape == button 2
	  case 0: //  OK clicked or Alt+O pressed or Enter pressed.
	           // save
	    break;
	  case 1: // Cancel clicked or Alt+C pressed or Escape pressed
	           // don't save but exit
            //setButtons( true );
	    back(); return;
	    break;
	  }
       }

       bool no_go = false;
       QString msg = QString::null;
       if ( FCcomboBox->currentText().isEmpty() ){
          no_go = true;
          msg = "No Fortran compiler to use with EGSnrc !\n";
       }
       if ( MAKEcomboBox->currentText().isEmpty() ){
	   no_go = true;
           msg += "No GNU make utility to use with EGSnrc !\n";
       }
       if ( SPECcomboBox->currentText().isEmpty() ){
	   no_go = true;
           msg += "You need to specify a name for the configuration file !\n";
       }
       if ( ConfNameEdit->text().isEmpty() ){
	   no_go = true;
           msg += "You need to specify a name for the configuration !";
       }
       if ( no_go ){
           QMessageBox::critical(this,"ERROR",msg, "&OK",0,0,0,-1);
            back(); return;
       }

      install_egs();

    }

    /*********************/
    /* C++ configuration */
    /*********************/
    if ( index == 2 ) {

      if (canonicalSys.contains("darwin",false)){
         CPPbundleCEdit->setEnabled(true);
         bundletextLabel->setEnabled(true);
      }
      else{
         CPPbundleCEdit->setEnabled(false);
         bundletextLabel->setEnabled(false);
      }

    }

    /**********************/
    /* C++ compiler setup */
    /**********************/
    if ( index == pageCount()-1 ) {
       configCPPMonitor->clear();
       configCPPProgressBar->setProgress( -1 );
       config_step = 0;
       the_monitor = configCPPMonitor;
       qApp->processEvents();
       QString EGSPPfile=ironIt(getSpecDir() + QString("egspp_") +
                       ConfNameEdit->text() + QString(".conf"));

       if ( fileExists( EGSPPfile ) ){
         switch( QMessageBox::warning( this, "EGSnrc setup",
               "C++ Configuration " + ConfNameEdit->text() +" already exists.\n"
               "Do you want to replace it ?","&Yes", "&No",0,
               0,      // Enter == button 0
               1 ) ) { // Escape == button 2
           case 0: //  OK clicked or Alt+O pressed or Enter pressed.
             // save
             break;
           case 1: // Cancel clicked or Alt+C pressed or Escape pressed
             // don't save but exit
             setBackEnabled( currentPage(), true );
             back(); return;
             break;
         }
       }
       QString DSODir = hen_house() + QString("/egs++/dso/") +
                        ConfNameEdit->text();
       if( !checkDir( DSODir.latin1() ) ){back(); return;}

       bool no_go = false;
       QString msg = QString::null;
       if ( CPPComboBox->currentText().isEmpty() ){
          msg = "No C++ compiler to use with EGSnrc !\n";
          QMessageBox::critical(this,"ERROR",msg, "&OK",0,0,0,-1);
          back(); return;
       }

       setBackEnabled( currentPage(), false );
       setNextEnabled( currentPage(), false );
       setFinishEnabled( currentPage(), false );
       create_egspp_config();
    }

    setHelpEnabled( pagina, FALSE );
    last_page = index;

}

//----------------------------------------------------------------
//
// EGS_CONFIGURE   S P E C I F I C         T O O L S
//
//----------------------------------------------------------------

QString EGSnrcMP_setup::hen_house(){
    return ironIt(HEN_HOUSE+QDir::separator());
}

QString EGSnrcMP_setup::win_dir(){
return ironIt( hen_house()      + QDir::separator()+
              (QString)"pieces" + QDir::separator() +
              (QString)"windows"+ QDir::separator());
}