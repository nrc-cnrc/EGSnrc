/*
###############################################################################
#
#  EGSnrc configuration GUI utilities and tests
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
#  Contributors:    Reid Townson
#
###############################################################################
*/


#include "egs_install.h"
#include <QCoreApplication>
#include<QAbstractButton>
#define MY_VERSION "1.0"

#ifdef WIN32
 #define TEST_XML "tests_win.xml"
 #define TEST_C_XML "tests_unix.xml"
#else
 #define TEST_XML "tests_unix.xml"
#endif

void QInstallPage::run_tests(){

     printProgress(fc->version());

     QString xmlf = henHouse() + QString("pieces") + QDir::separator() + QString(TEST_XML);
     QString msg;
     if( ! QFile::exists( xmlf ) ){
        msg = QString("\n\n Test file ") + xmlf + QString(" does not exist !\n\n");
        printProgress( msg );
        //setFinishEnabled( currentPage(), true );
        //setBackEnabled( currentPage(), true );
        return;
     }
     else{
        msg = QString("\n Using xml test file ") + xmlf + QString("\n");
     }
     printProgress( msg );

    if( ft ) ft=0;
    ft    = new MTest( screen, xmlf, config_file );
    connect( ft, SIGNAL( testsFinished() ),
             this, SLOT( create_egs_c_utils() ) );
    connect( ft, SIGNAL( taskFinished() ),
             this, SLOT( updateProgress() ) );

    ft->setCompilers( fc,  cc );
    resetProgressBar( ft->getTotalTasks() + n_config_steps );
    ft->reset();

    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert("PWD",QDir::currentPath());
    ft->copyEnvironment( environment );
    ft->launch();
}

#define NO_EGS_UTILS_C "\nFailed to compile egs_c_utils.c. This may be due to a variety of reasons"\
"\n(e.g. non-standard include files, unsupported file lock operations, etc."\
"\nWill disable now the availability of a C compiler. "\
"\nIf you want to try to make the new parallel processing implementation in "\
"\ncavrznrc and dosrznrc work, "\
"\n  1. Go to $machine_mortran_dir "\
"\n  2. Edit egs_c_utils.h and egs_c_utils.c to make them compile with your"\
" \n    C compiler"\
" \n 3. Put "\
"\n     REPLACE {$HAVE_C_COMPILER(#);} WITH {{EMIT;{P1}define HAVE_C_COMPILER};}"\
" \n    in machine.macros"\
" \n 4. Put "\
" \n    $HAVE_C_COMPILER(#);"\
" \n    near the beginning of your user code."\
"\n  5. If your compiler requires some unusual options, include directives, etc., "\
"\n     change the variable C_FLAGS in the spec file $conf_file in the "\
"\n     $HEN_HOUSE/specs/ directory. \n"
#define NO_LOAD_BEAMLIB "\n **** Failed to compile load_beamlib.c!!! ****"\
"\n This is the tool for loading BEAM as a particle source for other user codes."
#define NO_LOAD_VCULIB "\n **** Failed to compile load_vculib.c!!! ****"\
"\n This is the tool for using sources 20 and 21 in BEAMnrc."

//--------------------------------------------------------------------
//  Updates template for egs_config1.h and copies to
//  $HEN_HOUSE/lib/my_machine the sources for egs_config1.h, egs_c_utils
//  and load_beamlib.
//
//   Try to build C tools.
//
//--------------------------------------------------------------------
void QInstallPage::create_egs_c_utils(){

     printProgress( "\n===> Creating C Utilities for EGSnrc...\n\n");

     QString define  = "#define F77_OBJ(fname,FNAME) ",
             define_ = "#define F77_OBJ_(fname,FNAME) ";

     MTestAnswer &a = ft->getAnswers();
     QString f77_function1,
             f77_function2;

     if( a["Decoration"] == "lower case, no underscores" ){
       f77_function1= define + (QString)"fname";
       f77_function2= define_ + (QString)"fname";
     }
     else if(  a["Decoration"] == "lower case, _ and _" ){
      f77_function1= define + (QString)"fname##_";
      f77_function2= define_ + (QString)"fname##_";
     }
     else if(  a["Decoration"] == "lower case, _ and __" ){
      f77_function1= define + (QString)"fname##_";
      f77_function2= define_ + (QString)"fname##__";
     }
     else if(  a["Decoration"] == "upper case, no underscores" ){
      f77_function1= define + (QString)"FNAME";
      f77_function2= define_ + (QString)"FNAME";
     }
     else if(  a["Decoration"] == "upper case, _ and _" ){
      f77_function1= define + (QString)"FNAME##_";
      f77_function2= define_ + (QString)"FNAME##_";
     }
     else if(  a["Decoration"] == "upper case, _ and __" ){
      f77_function1= define + (QString)"FNAME##_";
      f77_function2= define_ + (QString)"FNAME##__";
     }
     else{ // unknown mangling scheme, using a default
      f77_function1="FNAME";
      f77_function2="FNAME";
     }

     /************************************************************/
     /* C utils need egs_config1.h or else they won't be compiled*/
     /************************************************************/
     QString s = readFile2QString( henHouse() + QString("pieces/egs_config1.h"),
                                   QString(NO_EGS_UTILS_C) );
     if ( s.isEmpty() ){
        printProgress(tr("\nCould not read egs_config1.h, C utils not built!\n"));
        emit egsCUtilsEnded();
        return;
     }

     s.replace((QString)"__f77_function1__",  f77_function1 );
     s.replace((QString)"__f77_function2__",  f77_function2 );
     s.replace((QString)"__config_name__",  my_machine() );

     if ( ! writeQString2File( s, egsLibDir + QString("egs_config1.h") )){
        printProgress(tr("\nCould not write egs_config1.h needed for the") +
                      tr("\nC interface. You won't be able to build the\n")+
                      tr("\nC utilities nor to write C user-codes.\n"));
        // One needs to build egs_c_utils and load_beamlib and for that egs_config1.h is crucial !!!!!!
        printProgress(QString("\n\n") + QString(NO_EGS_UTILS_C));
        printProgress(QString("\n\n") + QString(NO_LOAD_BEAMLIB));
        emit egsCUtilsFailed();
        return;
     }
     /************************************************************/

     QStringList task_f, task_q;
     task_f.append("egs_c_utils.c");
     task_q.append("Could egs_c_utils.c be compiled? ");
     task_f.append("load_beamlib.c");
     task_q.append("Could load_beamlib.c be compiled? ");
     task_f.append("load_vculib.c");
     task_q.append("Could load_vculib.c be compiled? ");
     task_f.append("read_write_pardose.c");
     task_q.append("Could read_write_pardose.c be compiled? ");

     if(!cc->exists() ){
        egs_c_utils_ok = false;
        load_beamlib_ok = false;
        read_write_pardose_ok=false;
        printProgress(
         tr("WARNING:\n") + tr("========\n") +
         tr("No C compiler passed to the configuration process.\n") +
         tr("C utilities for parallel runs and BEAM source engine\n") +
         tr("will not be available for this configuration!!!\n\n")
        );
        emit egsCUtilsFailed();
        return;
     }

     //*************************************************
     // Compile all or missing C utilites, including
     // read_write_pardose.c so DOSXYZnrc can be build.
     //*********************************************
     if (!copy( henHouse() + (QString)"cutils/egs_c_utils.h",
                egsLibDir + (QString)"egs_c_utils.h")           ||
         !copy( henHouse() + (QString)"cutils/egs_c_utils.c",
                egsLibDir + (QString)"egs_c_utils.c")           ||
         !copy( henHouse() + (QString)"cutils/load_beamlib.c",
                egsLibDir + (QString)"load_beamlib.c")          ||
         !copy( henHouse() + (QString)"cutils/load_vculib.c",
                egsLibDir + (QString)"load_vculib.c")           ||
         !copy( henHouse() + (QString)"user_codes/dosxyznrc/read_write_pardose.c",
                egsLibDir + (QString)"read_write_pardose.c")
        )
     {
        printProgress(tr("\n\nThere were errors copying the C utilities sources")+
                      tr("\nto $HEN_HOUSE/lib/my_machine."));
        printProgress(tr("\nYou won't be able to: \n") +
                      tr("\n- Use parallel job submission in your C/C++ user-codes\n") +
                      tr("\n- Use a BEAM simulation as a source\n") +
                      tr("\n- Compile DOSXYZnrc\n") );
     }

     Tasks* task = new Tasks[task_f.size()];
     for (int itask=0; itask < task_f.size(); itask++){
         task[itask].setFName(egsLibDir + task_f[itask]);
         task[itask].setDeleteFlag( false );
         task[itask].setProgram( QString::null );
         task[itask].setTaskName( task_q[itask] );
         task[itask].setLanguage( "C" );
#ifdef WIN32
          task[itask].setOptions( cc->optimization() + " " + cc->options() + " -c -DWIN32" );
#else
          task[itask].setOptions( cc->optimization() + " " + cc->options() + " -c" );
#endif
 }

      if ( ct ) ct=0;
      ct = new MTest( screen, task_f.size() , config_file );
      connect( ct, SIGNAL( taskFinished() ),
               this, SLOT( updateProgress() ) );
      connect( ct, SIGNAL( testsFinished()    ),
               this, SLOT( check_egs_c_utils() ) );
      connect( ct, SIGNAL( criticalError() ),
               this, SLOT( check_egs_c_utils() ) );

      ct->setTitle( (QString)"\nBuilding the utilities object file ... " );
      ct->setEndStr( QString() );
      ct->setCompilers( fc,  cc );
      ct->reset();
      QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
      environment.insert("PWD",QDir::currentPath());
      ct->copyEnvironment( environment );
      ct->setTasks( task );
      ct->launch();
}

void QInstallPage::check_egs_c_utils(){
     /**********************************************/
     /* Check outcome from compiling egs_c_utils.c */
     /**********************************************/
      bool all_ok=true;
      Tasks* task = ct->tasks(); int ntasks = ct->getTotalTasks();
      for (ushort itask=0; itask < ntasks; itask++){
          if ( ct->successful_test(itask) ) {
             QString obj_n = task[itask].objName();
             obj_n = obj_n.right(obj_n.length() - obj_n.lastIndexOf(QDir::separator())- 1);
             if ( fileExists( obj_n ) ){
                move_file( obj_n, task[itask].objName());
             }
             if (obj_n.contains("egs_c_utils"))
                egs_c_utils_ok = true;
             else if (obj_n.contains("load_beamlib"))
                load_beamlib_ok = true;
             else if (obj_n.contains("load_vculib"))
                load_vculib_ok = true;
             else if (obj_n.contains("read_write_pardose"))
                read_write_pardose_ok = true;
          }
          else all_ok = false;
      }

      if (!egs_c_utils_ok)
          printProgress((QString)NO_EGS_UTILS_C);
      if (!load_beamlib_ok)
          printProgress((QString)NO_LOAD_BEAMLIB);
      if (!load_vculib_ok)
          printProgress((QString)NO_LOAD_VCULIB);
      if (!read_write_pardose_ok)
          printProgress(QString("\nFailed compiling read_write_pardose.c!"
                                "\nThis is required to compile DOSXYZnrc!"));
      if (!all_ok)
          if(ct) ct->stop();

      updateProgress();
      emit egsCUtilsCreated();
}

/*******************************************************
 * Here we check whehter the egs_c_util object file can
 * be linked to a Fortran file.
 *******************************************************/
#define EGS_C_UTIL_VERSION "      program EGSCUTIL\n"\
"      integer i\n"\
"      call egs_create_control_file(\"test.lock\",i)\n"\
"      end\n"
void QInstallPage::test_c_utils(){

     if (!egs_c_utils_ok){
        printProgress("\n\nTesting egs_c_utils skipped due to previous errors...\n");
        updateProgress();
        emit egsCUtilsTested();
        return;
     }

    QString test_util( EGS_C_UTIL_VERSION );
    if ( ! writeQString2File( test_util, (QString)"test_c_utils.f" ) ) {
        printProgress( (QString)"\n Critical error: could not create file " +
                       (QString)"test_c_utils.f" );
        egs_c_utils_ok = false;
        printProgress(tr(NO_EGS_UTILS_C));
        updateProgress();
        emit egsCUtilsEnded();
        return;
    }

    if ( ct ) ct = 0;
    ct = new MTest( screen, 1, config_file );
    connect( ct, SIGNAL( taskFinished() ),
               this, SLOT( updateProgress() ) );
    connect( ct, SIGNAL( testsFinished() ),
             this, SLOT( get_test_c_utils_result()));
    connect( ct, SIGNAL( criticalError() ),
             this, SLOT( get_test_c_utils_result()));

    ct->setTitle( (QString)"\n\nTesting the C utilities object file ... " );
    ct->setEndStr( QString::null );
    ct->setCompilers( fc,  cc );
    ct->reset();
    Tasks* task = new Tasks();
    task->setFName( (QString)"test_c_utils.f" );
#ifdef WIN32
    task->setObjects( egsLibDir + tr("egs_c_utils.obj") );
#else
    task->setObjects( egsLibDir + tr("egs_c_utils.o") );
#endif
    task->setTaskName( "Could egs_c_utils be linked to a Fortran file ? " );
    task->setLanguage( "Fortran" );
    task->setDeleteFlag( true );
    ct->setTasks( task );
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert("PWD",QDir::currentPath());
    ct->copyEnvironment( environment );
    ct->launch();
}

void QInstallPage::get_test_c_utils_result(){
    if ( ct->successful_test((ushort)0) ) {
        egs_c_utils_ok = true;
        printProgress( tr("\n Yes, egs_c_utils can be successfully")+
                       tr(" linked to a Fortran file. \n")  );
    }
    else{
        egs_c_utils_ok = false;
        printProgress((QString)NO_EGS_UTILS_C);
    }
    updateProgress();
    emit egsCUtilsTested();
}

/****************************
 * Guessing link flag for DSO
 ****************************/
#define LOAD_BEAMLIB_VERSION "#include <dlfcn.h>\n"\
"int main() {\n"\
"    void *h, *sym;\n"\
"    h = dlopen(\"test.so\",RTLD_LAZY);\n"\
"    sym = dlsym(h,\"junk\");\n"\
"    return 0;\n"\
"}\n"
void QInstallPage::test_load_beamlib(){
     if (!load_beamlib_ok){
        dlopen_flags = QString();
        printProgress("\n\nTesting load_beamlib skipped due to previous errors...\n");
        updateProgress();
        emit LoadBeamLibTested();
        return;
     }
     QString test_util( LOAD_BEAMLIB_VERSION );
     if ( ! writeQString2File( test_util, (QString)"test_load_beamlib.c" ) ) {
        printProgress( (QString)"\n Critical error: could not create file " +
                       (QString)"test_load_beamlib.c" );
        load_beamlib_ok = false;
        dlopen_flags = QString();
        printProgress(tr("\nCouldn't guess link flag for an DSO object!!!")+
                      tr("\n ldopen_flag variable will be empty!\n"));
        updateProgress();
        emit LoadBeamLibTested();
        return;
     }

#ifdef WIN32
      load_beamlib_ok = true;
      dlopen_flags = QString();
      printProgress(tr("\nAssuming there is no need to pass a flag")+
                    tr("\nto the compiler for opening a DSO on Windows\n") +
                    tr("\nusing Win API function LoadLibrary().\n"));
      updateProgress();
      emit LoadBeamLibTested();
#else
     Tasks* task = new Tasks[3];
     task[0].setFName( (QString)"test_load_beamlib.c" );
     task[0].setTaskName( "Testing flag -ldl ..." );
     task[0].setLanguage( "C" );
     task[0].setDeleteFlag( false );
     task[0].setLibraries("-ldl");

     task[1].setFName( (QString)"test_load_beamlib.c" );
     task[1].setTaskName( "Testing flag -lc ..." );
     task[1].setLanguage( "C" );
     task[1].setDeleteFlag( false );
     task[1].setLibraries("-lc");

     task[2].setFName( (QString)"test_load_beamlib.c" );
     task[2].setTaskName( "Testing no flag ..." );
     task[2].setLanguage( "C" );
     task[2].setDeleteFlag( true );

     if ( ct ) ct = 0;
     ct = new MTest( screen, 3, config_file );
     connect( ct, SIGNAL( taskFinished() ),  this, SLOT( updateProgress() ) );
     connect( ct, SIGNAL( testsFinished() ), this, SLOT( get_test_load_beamlib()));
     connect( ct, SIGNAL( criticalError() ), this, SLOT( get_test_load_beamlib()));
    /**********************************************************************/

     ct->setTitle( (QString)"\n\nGuessing library needed for dlopen ... " );
     ct->setEndStr( QString::null );
     ct->setCompilers( fc,  cc );
     ct->reset();
     ct->setTasks( task );
     ct->launch();
#endif
}

void QInstallPage::get_test_load_beamlib(){
 if ( ct->successful_test((ushort)0) ) {
   load_beamlib_ok = true;
   dlopen_flags = "-ldl";
 }
 else if ( ct->successful_test((ushort)1) ) {
   load_beamlib_ok = true;
   dlopen_flags = "-lc";
 }
 else if ( ct->successful_test((ushort)2) ) {
   load_beamlib_ok = true;
   dlopen_flags = QString();
 }
 else{
   load_beamlib_ok = false;
   dlopen_flags = QString();
   printProgress(tr("\nFailed guessing flag for using dlopen,\n")+
                 tr("make sure you find the proper flag!!!"));
   emit LoadBeamLibTested();
   return;
 }
 printProgress( tr("\n ===> Library for dlopen is ")+ dlopen_flags +
                tr(" !!!\n")  );
 emit LoadBeamLibTested();
}

//**************************************************************
//
//       SPECfile   <===   configuration file name
//       CONFIGname <===   configuration name (taken form guessConfig)
//
//**************************************************************
void QInstallPage::createSystemFiles(){

  QString the_name = QFileInfo( QCoreApplication::applicationFilePath() ).fileName();
  MTestAnswer &a = ft->getAnswers();  // map relating test name to test result
  MTestID id = ft->getIDs();          // map relating test name to task number

  printProgress( "\n===> Creating configuration file ...\n\n");
  specFile = henHouse() + tr("specs") + QDir::separator() + confFile();
  QDate date = QDate::currentDate();
  QString today = date.toString ( Qt::TextDate );
  QDateTime dateTime = QDateTime::currentDateTimeUtc();
  QString config_time = dateTime.toString ( Qt::TextDate ) + " UTC";
  QString OS; QString stdfile;
#ifdef  WIN32
  OS = "Windows";
  stdfile = "windows.spec";
#else
  OS = "Unix";
  stdfile = "unix.spec";
#endif
  /*************************************************/
  // checking result from egs_c_utils.c compilation
  /*************************************************/
  QString hasCComp, egs_c_obj = QString();
  QString wouldbe_c_util = "$(HEN_HOUSE)lib$(DSEP)$(my_machine)$(DSEP)" +
    (QString)"egs_c_utils.o";
  QString machine_c_compiler = QString(MACHINE_HAS_C_COMPILER);
  if ( egs_c_utils_ok ) {
     hasCComp  = QString(HAS_C_COMPILER); wouldbe_c_util = QString();
     if ( fileExists( egsLibDir + QString("egs_c_utils.o") ) )
         egs_c_obj = QString("$(HEN_HOUSE)lib$(DSEP)$(my_machine)$(DSEP)") +
                   QString("egs_c_utils.o");
     else if ( fileExists( egsLibDir + (QString)"egs_c_utils.obj" ) )
         egs_c_obj = QString("$(HEN_HOUSE)lib$(DSEP)$(my_machine)$(DSEP)") +
                     QString("egs_c_utils.obj");
     else egs_c_obj = QString();
  }
  else{
     hasCComp = QString(HAS_NO_C_COMPILER); egs_c_obj = QString();
     machine_c_compiler = QString(MACHINE_HAS_NO_C_COMPILER);
  }
  /*************************************************/
  // checking result from load_beamlib.c compilation
  /*************************************************/
  QString beamlib_header(BEAMLIB_OBJECTS_HEADER1),
          beamlib_extra_header(BEAMLIB_EXTRA_LIBS_HEADER1),
          load_beamlib_obj("$(HEN_HOUSE)lib$(DSEP)$(my_machine)"
                           "$(DSEP)load_beamlib.o");
  QString load_beamlib_macro(HAVE_LOAD_DSO);
  if(!load_beamlib_ok){
    beamlib_header       = QString(BEAMLIB_OBJECTS_HEADER2);
    beamlib_extra_header = QString(BEAMLIB_EXTRA_LIBS_HEADER2);
    load_beamlib_obj     = QString();
    load_beamlib_macro   = QString(HAVE_NOT_LOAD_DSO);
  }
#ifdef WIN32
  else if (!fileExists(egsLibDir+(QString)"/load_beamlib.o")&&
            fileExists(egsLibDir+(QString)"/load_beamlib.obj")){
    load_beamlib_obj = QString("$(HEN_HOUSE)lib$(DSEP)$(my_machine)") +
                       QString("$(DSEP)load_beamlib.obj");
  }
  else{
    load_beamlib_obj = QString();
  }
#endif
  /***********************************************************/
  // check whether exit and stop failed and add egs_exit to the
  // EGS_EXTRA_OBJECTS variable
  /***********************************************************/
  QString egs_exit;
  if (! ft->successful_test( id["exitFun"] ) &&
      ! ft->successful_test( id["stopFun"] ) &&
        ft->successful_test( id["egsExitFun"] )  ){
#ifndef WIN32
    egs_exit = QString(" $(HEN_HOUSE)lib$(DSEP)$(my_machine)$(DSEP)") +
               QString("egs_exit.o ");
    if ( fileExists( "egs_exit.o" ) ){
         move_file("egs_exit.o", egsLibDir + QString("egs_exit.o"));
    }
    else{
      egs_exit = QString();
    }
#else
    egs_exit = (QString)" $(HEN_HOUSE)lib$(DSEP)$(my_machine)$(DSEP)" +
      (QString)"egs_exit.obj ";
#endif
  }

  QString specfile(spec_file);
  specfile.replace(QString("$my_name"),  the_name );
  specfile.replace(QString("$my_version"),  QString(MY_VERSION) );

  QString the_extra_flag = QString();
  QString the_hen = henHouse();
#ifdef WIN32
  if ( the_hen.endsWith( QDir::separator() ) )
    the_hen.chop(1);
  the_hen.append("$(DSEP)");
  the_hen.replace( 0, 1, QString(the_hen[0]).toUpper());

  //QString the_sep = (QString)" := $(shell echo \\)";
  QString the_sep = QString(" := $(subst /,\\,/)");
#else
  QString the_sep = QString(" = /");
  if ( fc->name().toLower() == "pgf77" || fc->version().contains("PGI") )
    the_extra_flag = "-Mnomain";

#endif
  specfile.replace(QString("$DSEP"),  the_sep ) ;
  specfile.replace(QString("$HEN_HOUSE"), the_hen );
  specfile.replace(QString("$conf_name"),  my_machine() );
  specfile.replace(QString("$canonical_system"),  canonical() );
  specfile.replace(QString("$current_date"),  today );
  specfile.replace(QString("$make_prog"),  make->name() );
  specfile.replace(QString("$OS"),  OS );
  specfile.replace(QString("$std.spec"),  stdfile );
  specfile.replace(QString("$F77"),  fc->name() );

  /* ************** IK: changes needed for new config version
                        required to accommodate Darwin
                        once I'm at making changes, let's also
                        try to find libg2c.a and put it in
                        SHLIB_LIBS
  */
  bool is_generic = true;
  if( fc->name() == "g77" && cc->name() == "gcc" ) {
      if( canonical().contains("darwin") ) {
          is_generic = false;
          specfile.replace((QString)"$EGS_F77_LINK",(QString)"gcc");
          specfile.replace((QString)"$EGS_SHLIB_FLAGS",(QString)"-bundle");
          specfile.replace((QString)"$EGS_SHLIB_LIBS",(QString)"-lg2c");
      }
      else if( canonical().contains("linux") ||
               canonical().contains("unix")) {
          is_generic = false;
          QStringList plist;
          plist += "/lib64/";
          plist += "/usr/lib64/";
          plist += "/usr/local/lib64/";
          plist += "/lib/";
          plist += "/usr/lib/";
          plist += "/usr/local/lib/";
          QString shlib_libs = " ";
          for ( QStringList::Iterator it=plist.begin(); it!=plist.end();++it) {
              QString file = *it + "libg2c.a";
              QFileInfo fi(file);
              if( fi.exists() ) { shlib_libs = file; break; }
          }
          specfile.replace((QString)"$EGS_F77_LINK","$(F77)");
          specfile.replace((QString)"$EGS_SHLIB_FLAGS",
              (QString)"-shared -Wl,-Bsymbolic");
          specfile.replace((QString)"$EGS_SHLIB_LIBS",shlib_libs);
      }
  }
  if( is_generic ) {
      specfile.replace(QString("$EGS_F77_LINK"),QString("$(F77)"));
      specfile.replace(QString("$EGS_SHLIB_FLAGS"),
              QString("-shared -Wl,-Bsymbolic"));
      specfile.replace(QString("$EGS_SHLIB_LIBS"),QString(" "));
  }
#ifdef WIN32
  specfile.replace(QString("$EGS_FOBJE"),QString("obj"));
#else
  specfile.replace(QString("$EGS_FOBJE"),QString("o"));
#endif

  //
  //  ********************** End of changes needed for Darwin
  //

  specfile.replace(QString("$FFLAGS"),  fc->options() );
  specfile.replace(QString("$FOPT"),  fc->optimization() );
  specfile.replace(QString("$FDEBUG"),  fc->debug() );
  specfile.replace(QString("$FLIBS"),  egs_exit + fc->libraries() );
  specfile.replace(QString("$FOUT"),  fc->outflag() );
  specfile.replace(QString("$FEXTRA"),  the_extra_flag );
  specfile.replace(QString("$CC"),  cc->name() );
  specfile.replace((QString)"$C_FLAGS",  cc->optimization() + QString(" ") + cc->options() );
  specfile.replace((QString)"$COUT",  cc->outflag() );
  specfile.replace((QString)"$C_COMPILER_AVAILABILITY",  hasCComp );
  specfile.replace((QString)"$EGS_C_UTILS_OBJ",  egs_c_obj );
  specfile.replace((QString)"$WOULD_BE_EGS_C_UTIL",  wouldbe_c_util );
  specfile.replace((QString)"$BEAMLIB_OBJECTS_HEADER",beamlib_header);
  specfile.replace((QString)"$BEAMLIB_EXTRA_LIBS_HEADER",beamlib_extra_header);
  specfile.replace((QString)"$LOAD_BEAMLIB", load_beamlib_obj );
  specfile.replace((QString)"$dlopen_flags", dlopen_flags );

  if ( ! writeQString2File( specfile, specFile ) ) {
      printProgress( tr("\n Critical error: could not create ")+
                     tr("configuration file ") + specFile );
      QString direrror = QString("\n Could not create configuration file ") +
        specFile;
      direrror+= QString("\n Check your permissions on this area or");
      direrror+= QString("\n consult your system/EGSnrc administrator!");
      QMessageBox::critical(this,"ERROR", direrror,"&OK",0,0,0,-1);
      return;
  }

  printProgress( "\n===> Creating machine.macros ...\n\n");

  QString comp_def = fc->name();
  QString sep;
  QString copy;
  QString move;
  QString remove_dir;
#ifdef WIN32
  comp_def.remove(".exe");
  sep = "char(92)";
  copy = "copy ";
  move = "move /Y ";
  remove_dir = "rmdir /S /Q ";
  the_hen.replace("$(DSEP)","/");
#else
  sep = "'/'";
  copy = "cp ";
  move = "mv -f ";
  remove_dir = "rm -rf ";
#endif
  QString no_rec_len = "\" Record Length for unformatted I/O \"\n";
  QString flush_func  = (QString)HAS_FLUSH_FUN;
  QString the_exit = QString(HAS_EXIT);
  QString the_longest_int ="\"$MAX_INT SET TO 2^63-1 in egsnrc.macros\"";

  if ( ! ft->successful_test( id["LongInteger"] ) ){
    a["LongInteger"] = "integer*4";
    the_longest_int = "REPLACE {$MAX_INT} WITH {2147483647};\"2^31-1\"";
  }
  if ( ! ft->successful_test( id["ShortInteger"] ) )
    a["ShortInteger"] = "integer*4";
  if ( ! ft->successful_test( id["flushFun"] ) ){
    flush_func = (QString)HAS_NO_FLUSH_FUN;
    a["flushFun"] = ";";
  }
  if ( ! ft->successful_test( id["recLen"] ) ){
    a["recLen"] = "fixme!";
    no_rec_len = (QString)UNKNOWN_REC_LEN;
  }
  if ( ! ft->successful_test( id["exitFun"] ) ){
    if ( ft->successful_test( id["stopFun"] ) )
      the_exit =(QString)HAS_STOP;
    else
      the_exit =(QString)USE_EGS_EXIT;
  }
  if (a["Endianess"] == "little endian"){
    a["Endianess"] = "1234";
  }
  else if (a["Endianess"] == "big endian"){
    a["Endianess"] = "4321";
  }
  else if (a["Endianess"] == "pdp endian"){
    a["Endianess"] = "3412";
  }
  else{// Unknown endianess
    a["Endianess"] = "xxxx";
  }

  QString configfile = specFile;
  QString Machine_Macros  =  egsLibDir + QString("machine.macros");
  QString machinemacros(machine_macros);
  machinemacros.replace(QString("$my_name"), the_name );
  machinemacros.replace(QString("$my_version"), QString(MY_VERSION) );
  machinemacros.replace(QString("$current_date"),today );
  machinemacros.replace(QString("$the_config"),my_machine() );
  machinemacros.replace(QString("$$HEN_HOUSE"),the_hen.replace("\\","/") );
  machinemacros.replace(QString("$spec_file"), configfile.replace("\\","/") );
  machinemacros.replace(QString("$canonical_system"),  canonical() );
  machinemacros.replace(QString("$conf_name"),  my_machine() );
  machinemacros.replace(QString("$config_time"),  config_time );
  machinemacros.replace(QString("$sep"),  sep );
  machinemacros.replace(QString("$copyf"),  copy );
  machinemacros.replace(QString("$movef"),  move );
  machinemacros.replace(QString("$rmdir"),  remove_dir );
  machinemacros.replace(QString("LongInteger"),  a["LongInteger"] );
  machinemacros.replace(QString("ShortInteger"),  a["ShortInteger"] );
  machinemacros.replace(QString("THE_LONGEST_INT"), the_longest_int);
  machinemacros.replace(QString("Endianess"),  a["Endianess"] );
  machinemacros.replace(QString("$NO_REC_LEN"),  no_rec_len );
  machinemacros.replace(QString("recLen"),  a["recLen"] );
  machinemacros.replace(QString("$FLUSH_FUN"),  flush_func );
  machinemacros.replace(QString("flushFun"),  a["flushFun"] );
  machinemacros.replace(QString("$EXIT_FUN"),the_exit );
  machinemacros.replace(QString("exitFun"), a["exitFun"] );   // Only one of the three keys below
  machinemacros.replace(QString("stopFun"), a["stopFun"] );   // will get replaced depending on
  machinemacros.replace(QString("egsExitFun"),  a["egsExitFun"] );
  machinemacros.replace(QString("$MACHINE_C_COMPILER"),  machine_c_compiler );
  machinemacros.replace(QString("$MACHINE_HAVE_LOAD_DSO"), load_beamlib_macro );

  if ( ! writeQString2File( machinemacros, Machine_Macros ) ) {
    printProgress( tr("\n Critical error: could not create system file ")
        + Machine_Macros );
    return;
  }
  printProgress( (QString)"\n *** System file " + Machine_Macros +
      (QString)" successfully created ***\n\n");

  printProgress( "\n===> Creating machine.f ...\n\n");
  QString Machine_F  =  egsLibDir + QString("machine.f");
  QString machinef(machine_f);
  machinef.replace(QString("$my_name"),  the_name );
  machinef.replace(QString("$my_version"), QString(MY_VERSION) );
  QString fun_chunk;
  if ( ft->successful_test( id["SystemFun"] ) ){
    fun_chunk = readFile2QString( henHouse() +
        QString("pieces/egs_system_v1.f"),
        QString("Error copying egs_system_v1.f)") );
  }
  else if( ft->successful_test( id["SystemSub"] ) ){
    fun_chunk = readFile2QString( henHouse() +
        QString("pieces/egs_system_v2.f"),
        QString("Error copying egs_system_v2.f") );
  }
  else{
    fun_chunk = readFile2QString( henHouse() +
        QString("pieces/egs_system_v3.f"),
        QString("Error copying egs_system_v3.f") );
  }

  machinef.append( fun_chunk );

  if ( ft->successful_test( id["lstatFun"] ) ){
    fun_chunk = readFile2QString( henHouse() + QString("pieces/egs_isdir_v1.f"),
                                  QString("Error copying egs_isdir_v1.f") );
    QString s = a[ "lstatFun" ], direle=s, arraysize=s;
    direle.chop(direle.length() - direle.indexOf(":"));
    arraysize.remove(0,arraysize.indexOf(":")+1);
    fun_chunk.replace( "___dir_element___", direle );
    fun_chunk.replace( "___array_size___", arraysize );

  }
  else if ( ft->successful_test( id["InquireFun"] ) ){
    fun_chunk = readFile2QString( henHouse() + QString("pieces/egs_isdir_v2.f"),
                                  QString("Error copying egs_isdir_v2.f") );
  }
  else{
    fun_chunk = readFile2QString( henHouse() + QString("pieces/egs_isdir_v3.f"),
                                  QString("Error copying egs_isdir_v3.f") );
  }
  machinef.append( fun_chunk );

  if ( ft->successful_test( id["fdateFun"] ) ){
     fun_chunk = readFile2QString( henHouse() + QString("pieces/egs_fdate_v1.f"),
        QString("Error copying egs_fdate_v1.f") );
     fun_chunk.replace( "__fdate__", a[ "fdateFun" ]  );
  }
  else if( ft->successful_test( id["DateAndTime"] ) ){
    fun_chunk = readFile2QString( henHouse() +
        (QString)"pieces/egs_fdate_v2.f",
        (QString)"Error copying egs_fdate_v2.f" );
  }
  else{
    fun_chunk = readFile2QString( henHouse() +
        (QString)"pieces/egs_fdate_v3.f",
        (QString)"Error copying egs_fdate_v3.f" );
  }
  machinef.append( fun_chunk );

  if ( ft->successful_test( id["DateAndTime"] ) ){
    fun_chunk = readFile2QString( henHouse() +
        (QString)"pieces/egs_date_and_time_v1.f",
        (QString)"Error copying egs_date_and_time_v1.f" );
  }
  else if( ft->successful_test( id[ "fdateFun" ] ) ){
    fun_chunk = readFile2QString( henHouse() +
        (QString)"pieces/egs_date_and_time_v2.f",
        (QString)"Error copying egs_date_and_time_v1.f" );
    fun_chunk.replace( "__fdate__", a[ "fdateFun" ]  );
  }
  else{
    fun_chunk = readFile2QString( henHouse() +
        (QString)"pieces/egs_date_and_time_stub.f",
        (QString)"Error copying egs_date_and_time_stub.f" );
  }
  machinef.append( fun_chunk );

  if ( ft->successful_test( id["fdateFun"] ) ){
    fun_chunk = readFile2QString( henHouse() +
        (QString)"pieces/egs_date_v1.f",
        (QString)"Error copying egs_date_v1.f" );
    fun_chunk.replace( "__fdate__", a[ "fdateFun" ]  );
  }
  else if( ft->successful_test( id["DateAndTime"] ) ){
    fun_chunk = readFile2QString( henHouse() +
        (QString)"pieces/egs_date_v2.f",
        (QString)"Error copying egs_date_v2.f" );
  }
  else if( ft->successful_test( id["dateFun"] ) ){
    fun_chunk = readFile2QString( henHouse() +
        (QString)"pieces/egs_date_v3.f",
        (QString)"Error copying egs_date_v3.f" );
    fun_chunk.replace( "__date__", a[ "dateFun" ]  );
  }
  else{
    fun_chunk = readFile2QString( henHouse() +
        (QString)"pieces/egs_date_stub.f",
        (QString)"Error copying egs_date_stub.f" );
  }
  machinef.append( fun_chunk );

  if ( ft->successful_test( id["fdateFun"] ) ){
    fun_chunk = readFile2QString( henHouse() +
        (QString)"pieces/egs_time_v1.f",
        (QString)"Error copying egs_time_v1.f" );
    fun_chunk.replace( "__fdate__", a[ "fdateFun" ]  );
  }
  else if( ft->successful_test( id["DateAndTime"] ) ){
    fun_chunk = readFile2QString( henHouse() +
        (QString)"pieces/egs_time_v2.f",
        (QString)"Error copying egs_time_v2.f" );
  }
  else if( ft->successful_test( id["timeFun"] ) ){
    fun_chunk = readFile2QString( henHouse() +
        (QString)"pieces/egs_time_v3.f",
        (QString)"Error copying egs_time_v3.f" );
    fun_chunk.replace( "__time__", a[ "timeFun" ]  );
  }
  else{
    fun_chunk = readFile2QString( henHouse() +
        (QString)"pieces/egs_date_stub.f",
        (QString)"Error copying egs_date_stub.f" );
  }
  machinef.append( fun_chunk );

  if ( ft->successful_test( id["DateAndTime"] ) ){
    fun_chunk = readFile2QString( henHouse() +
        (QString)"pieces/egs_secnds_v1.f",
        (QString)"Error copying egs_secnds_v1.f" );
  }
  else if( ft->successful_test( id[ "fdateFun" ] ) ){
    fun_chunk = readFile2QString( henHouse() +
        (QString)"pieces/egs_secnds_v2.f",
        (QString)"Error copying egs_secnds_v2.f" );
    fun_chunk.replace( "__fdate__", a[ "fdateFun" ]  );
  }
  else if( ft->successful_test( id[ "secsFun" ] ) ){
    fun_chunk = readFile2QString( henHouse() +
        (QString)"pieces/egs_secnds_v3.f",
        (QString)"Error copying egs_secnds_v3.f" );
    fun_chunk.replace( "__secnds__", a[ "secsFun" ]  );
  }
  else if( ft->successful_test( id[ "timeFun" ] ) ){
    fun_chunk = readFile2QString( henHouse() +
        (QString)"pieces/egs_secnds_v4.f",
        (QString)"Error copying egs_secnds_v4.f" );
    fun_chunk.replace( "__time__", a[ "timeFun" ]  );
  }
  else{
    fun_chunk = readFile2QString( henHouse() +
        (QString)"pieces/egs_secnds_stub.f",
        (QString)"Error copying egs_secnds_stub.f" );
  }
  machinef.append( fun_chunk );

  if ( ft->successful_test( id["DateAndTime"] ) ||
      ft->successful_test( id[ "fdateFun" ] )  ){
    fun_chunk = readFile2QString( henHouse() +
        (QString)"pieces/time_stuff.f",
        (QString)"Error copying time_stuff.f" );
  }
  machinef.append( fun_chunk );


  if ( ft->successful_test( id["etimeFun"] ) ){
    fun_chunk = readFile2QString( henHouse() +
        (QString)"pieces/egs_etime.f",
        (QString)"Error copying egs_etime.f" );
    fun_chunk.replace( "__etime__", a[ "etimeFun" ]  );
  }
  else{
    fun_chunk = readFile2QString( henHouse() +
        (QString)"pieces/egs_etime_stub.f",
        (QString)"Error copying egs_etime_stub.f" );
  }
  machinef.append( fun_chunk );

  fun_chunk = readFile2QString( henHouse() +
      (QString)"pieces/egs_canonical_system.f",
      (QString)"Error copying egs_canonical_system.f" );
  fun_chunk.replace( "__canonical_system__", canonical()  );
  machinef.append( fun_chunk );

  fun_chunk = readFile2QString( henHouse() +
      (QString)"pieces/egs_configuration_name.f",
      (QString)"Error copying egs_configuration_name.f" );
  fun_chunk.replace( "__configuration_name__", my_machine()  );
  machinef.append( fun_chunk );

  if ( ft->successful_test( id["hostFun"] ) ){
    fun_chunk = readFile2QString( henHouse() +
        (QString)"pieces/egs_hostnm_v1.f",
        (QString)"Error copying egs_hostnm_v1.f" );
    fun_chunk.replace( "__hostnm__", a[ "hostFun" ]  );
  }
  else{
    fun_chunk = readFile2QString( henHouse() +
        (QString)"pieces/egs_hostnm_v2.f",
        (QString)"Error copying egs_hostnm_v1.f" );
  }
  machinef.append( fun_chunk );

  if ( ! ft->successful_test( id["LnblnkFun"] ) ){
    machinef.append( (QString)LNBLNK );
  }

  if ( ! writeQString2File( machinef, Machine_F ) ) {
   printProgress( tr("\n Critical error: could not create system file ") +
                  Machine_F );
    return;
  }

  /*----------------------------------------------------------------------------
    MACHINE.MORTRAN
    machine.mortran is what will get used in mortran compilations
    We generate it just as a wrapper around machine.f
    -----------------------------------------------------------------------------*/
  printProgress( "\n===> Creating machine.mortran ...\n\n");
  QString Machine_M  =  egsLibDir + QString("machine.mortran");
  QString machinem("%F\n");
  fun_chunk = readFile2QString( Machine_F, QString("Error reading ") +
      Machine_F );
  machinem.append( fun_chunk );
  machinem.append( "%M\n" );
  if ( ! writeQString2File( machinem, Machine_M ) ) {
    printProgress( tr("\n Critical error: could not create system file ")+
        Machine_M                    +
        (QString)"\n ***************" +
        (QString)"\n Stopping installation...");
    return;
  }
  printProgress( (QString)"\n *** System file " + Machine_M +
      (QString)" successfully created ***\n\n");

  create_dosxyz_spec(read_write_pardose_ok);
  append_vculib_dosxyz_spec(load_vculib_ok);

  updateProgress();
  emit systemCreated( mortran3 );
}

/*
*****************************************************************
 Create dosxyznrc_my_machine.spec file in $HEN_HOUSE/specs
*****************************************************************
*/
void QInstallPage::create_dosxyz_spec(bool rw_pardose_ok)
{
  printProgress( "\n===> Creating dosxyznrc_" + my_machine() + ".spec ...", false);
  if (rw_pardose_ok){
     QString dosxyz_spec = henHouse() + tr("specs") + QDir::separator() +
                           tr("dosxyznrc_") + my_machine() + tr(".spec");
     QString str("PARDOSE_OBJECTS = ");
     if ( fileExists( egsLibDir + QDir::separator() + tr("read_write_pardose.o")) ){
        str += tr("$(EGS_LIBDIR)")+tr("read_write_pardose.o");
     }
     else if ( fileExists( egsLibDir + QDir::separator() + tr("read_write_pardose.obj")))
     {
        str += tr("$(EGS_LIBDIR)")+tr("read_write_pardose.obj");
     }
     else{
        printProgress(tr(" failed \n\n") +
                      tr("Could not find read_write_pardose object file!")+
                      tr("You will not be able to use ")+
                      tr("\nDOSXYZnrc for in-phantom dose calculations.\n"));
        return;
     }

     bool wrote_pardose = writeQString2File( str, dosxyz_spec );
     if (!wrote_pardose){
      printProgress(tr(" failed \n\n") +
                    tr("Failed creating dosxyz_")+ my_machine() + tr(".spec") +
                    tr("\nfile. You will not be able to use ")+
                    tr("\nDOSXYZnrc for in-phantom dose calculations.\n"));
      return;
    }
    printProgress(tr(" done \n\n"));
  }
  else{
      printProgress(tr(" failed \n\n") +
                    tr("Failed creating dosxyz_")+ my_machine() + tr(".spec") +
                    tr("\nfile. You will not be able to use ")+
                    tr("\nDOSXYZnrc for in-phantom dose calculations.\n"));

  }
}

/***************************************************************/
/* Find out if load_vculib is in $HEN_HOUSE/lib/$my_machine */
/***************************************************************/
void QInstallPage::append_vculib_dosxyz_spec(bool load_vculib_ok){

  printProgress( "\n===> Appending VCU library to dosxyznrc_" +
                 my_machine() + ".spec ...", false);
  if (load_vculib_ok){
     QString vcuobject = QString::null;
     if ( fileExists( egsLibDir + QDir::separator() + tr("load_vculib.o")) ){
        vcuobject = tr("$(EGS_LIBDIR)")+tr("load_vculib.o");
     }
     else if ( fileExists( egsLibDir + QDir::separator() + tr("load_vculib.obj")) ){
        vcuobject = tr("$(EGS_LIBDIR)")+tr("load_vculib.obj");
     }
     else{
      printProgress(tr(" failed \n\n") +
                    tr("Failed finding load_vculib ")+
                    tr("You will not be able to use ")+
                    tr("\nVCU source 20.\n"));
      return;
  }
     QString dosxyz_spec = henHouse() + tr("specs") + QDir::separator() +
                           tr("dosxyznrc_") + my_machine() + tr(".spec");
     QString str = tr("\nVCULIB_OBJECTS = ") + vcuobject;
     bool wrote_vculib = appendQString2File( str, dosxyz_spec );
     if (!wrote_vculib){
      printProgress(tr(" failed \n\n") +
                    tr("Failed adding VCULIB_OBJECTS to the ")+
                    dosxyz_spec +
                    tr("\nfile. You will not be able to use ")+
                    tr("\nVCU source 20.\n"));
      return;
     }

     printProgress(tr(" done \n\n"));
  }
}

//**************************************************************
// Creates a C++ config file (egspp_$(my_machine).conf)
// and updates the config file $(EGS_CONFIG) with
// Some info is already in the dso object but the user can
// change some of these in the GUI. Hence, we take these from
// the GUI directly.
//**************************************************************
void QInstallPage::create_egspp_config(){

  QString the_name = QFileInfo( QCoreApplication::applicationFilePath() ).fileName();
  QDate date = QDate::currentDate();
  QString today = date.toString ( Qt::TextDate ),
        version = (QDate::currentDate()).toString("yyyy");

  printProgress( "\n===> Creating C++ configuration file ...\n\n");

  specFileCPP = henHouse() + tr("specs") + QDir::separator() + QString("egspp_") + confFile();

  QString egsppspecfile(egspp_spec_file);
  egsppspecfile.replace(QString("$my_name"),  the_name );
  egsppspecfile.replace(QString("$my_version"),  version ); // Was QString(EGS_VERSION)
  egsppspecfile.replace(QString("$current_date"),  today );
  egsppspecfile.replace(QString("$CXX"),  cpp->name() );
  egsppspecfile.replace(QString("$opt"), cpp->optimization());
  egsppspecfile.replace(QString("$shared"), cpp->shared());
  egsppspecfile.replace(QString("$libpre"), cpp->libpre());
  egsppspecfile.replace(QString("$libext"), cpp->libext());
  egsppspecfile.replace(QString("$obje"), cpp->obje() );
  egsppspecfile.replace(QString("$defines"), cpp->defines() );
  egsppspecfile.replace(QString("$fpic"), cpp->fpic() );
  egsppspecfile.replace(QString("$extra_link"), cpp->extra_link());  //order is
  egsppspecfile.replace(QString("$extra"), cpp->extra() + " " + dlopen_flags);//important here
  egsppspecfile.replace(QString("$eout"), cpp->outflag() );
  egsppspecfile.replace(QString("$lib_link1"), cpp->dsoPath());
  egsppspecfile.replace(QString("$f_libs"), cpp->flibs());
  egsppspecfile.replace(QString("$link2_prefix"), cpp->LinkPrefix());
  egsppspecfile.replace(QString("$link2_suffix"), cpp->LinkSuffix());

  if ( ! writeQString2File( egsppspecfile, specFileCPP ) ) {
    QString direrror = QString("\n Could not create configuration file ") +
      specFileCPP;
    printProgress( direrror );
    direrror+= (QString)"\n Check your permissions on this area or";
    direrror+= (QString)"\n consult your system/EGSnrc administrator!";
    QMessageBox::critical(this,"ERROR", direrror,"&OK",0,0,0,-1);
    return;
  }

  /* append MACOS variables to C++ config file */
  if (canonical().contains("darwin")){
     QString mac_bundles = QString(MACOS_BUNDLES);
     mac_bundles.replace(QString("$shared_bundle"),cpp->bundleOSX());
     mac_bundles.replace(QString("$libext_bundle"),cpp->libextBundleOSX());
     append2file(mac_bundles.toLatin1(),specFileCPP.toLatin1());
  }

  emit cppSystemCreated( iaea );

}

/*---------------------------------------------------------------------------
                               B U I L D I N G     E G S n r c
-----------------------------------------------------------------------------*/
int build_steps = 0;
void QInstallPage::buildEGSnrc( ushort code )
{

  QString USER, usercodemsg;

#ifdef WIN32
  QString system_installed(WIN_EGS_INSTALLED);
  USER = getenv("USERNAME");
#else
  QString system_installed(UNIX_EGS_INSTALLED);
  USER = getenv( "USER" );
#endif

  switch( code ){
    case corespec:
         if (skip_config) emit systemCreated( mortran3 );
         else run_tests();
         break;
    case mortran3:
         buildFlag = pegs4;
         buildMortran3();
         break;
    case pegs4:
         qApp->processEvents();
         //buildFlag = density_corrections; density correction files are now already uncompressed
         buildFlag = sysDone;
         buildPegs4();
         break;
    case density_corrections:
         updateProgress();
         qApp->processEvents();
         printProgress(
         "\n===> Creating density correction files for elements and compounds ...\n\n");
         buildFlag = sysDone;
         buildEGSCode( henHouse() + (QString)"pegs4" + QDir::separator() +
                                   (QString)"density_corrections"  );
         break;
    case sysDone:
#ifndef WIN32
         // Since file attributes maintained in github, this is not needed anymore!
         //create_additions();
         //make_scripts_executable();
#endif
         if (!buildOK[mortran3]){
         QString no_mortran ="\n****************************************************\n";
                 no_mortran+=  "* Failed building mortran3, the string preprocessor*\n";
                 no_mortran+=  "* utility for generating Fortran code. You won't be*\n";
                 no_mortran+=  "* able to compile any Mortran source code therefore*\n";
                 no_mortran+=  "* your system will not be functional.              *\n";
                 no_mortran+=  "****************************************************\n\n";
            printProgress(no_mortran);
            printProgress(
            (QString)"\n\n***** EGSnrc installation FAILED,"
                     " see log file for details  *****\n\n");
            resetPage();
            return;
          }
         if (!buildOK[pegs4]){
          QString no_pegs ="\n****************************************************\n";
                  no_pegs+=  "* Failed building PEGS4, the data preprocessor for *\n";
                  no_pegs+=  "* EGSnrc, you won't be able to create your own data*\n";
                  no_pegs+=  "* sets. However, some standard PEGS4 data sets are *\n";
                  no_pegs+=  "* available in your $HEN_HOUSE/pegs4/data.         *\n";
                  no_pegs+=  "****************************************************\n\n";
                  printProgress(no_pegs);
         }
         if (!buildOK[density_corrections]){
          QString no_dens ="\n****************************************************\n";
                  no_dens+=  "* Failed creating density correction files !!!     *\n";
                  no_dens+=  "* Check the config log file for more details!!!    *\n";
                  no_dens+=  "* You can still use the EGSnrc system, but won't   *\n";
                  no_dens+=  "* be able to produce PEGS4 data sets using ICRU-37 *\n";
                  no_dens+=  "* density corrections !!!                          *\n";
                  no_dens+=  "****************************************************\n\n";
                  printProgress(no_dens);
         }
         system_installed.replace(  "$HEN_HOUSE", henHouse() );
         system_installed.replace(  "$conf_file", specFile );
         printProgress( system_installed );
         finalize_egs_config( system_installed );
         break;
    case egsppspec:
         if (skip_config) emit cppSystemCreated( iaea );
         else create_egspp_config();
         break;
    case iaea:
         qApp->processEvents();
         buildFlag = egspp;
         buildIAEALib();
         break;
    case egspp:
         qApp->processEvents();
         buildFlag = cppDone;
         buildEGSPPLib();
         break;
    case cppDone:
         finalize_cpp();
         break;
    default:
          emit AllDone();
          break;
  }
}

/********************
 *
 * STEP #
 *
 * Building mortran3
 *
 ********************/
void QInstallPage::buildMortran3(){

 MTestAnswer &a = ft->getAnswers();//map relating test name to test result
 MTestID id = ft->getIDs();        // map relating test name to task number

//*********************   Creatng mortran3.f   ******************
 printProgress( "\n===> Compiling Mortran3 ...\n\n");

 QDir d( henHouse() + "mortran3" + QDir::separator() );
 if ( !d.exists() ){
    printProgress(
           tr("\n There is no mortran3 subdirectory in your EGSnrc directory ")
           + henHouse() + (QString)"\n Installation stopped ! \n" );
    return;
 }

 QString Mortran3_F = henHouse() + "mortran3/mortran3.f";
 delete_file( egsBinDir + "/mortran3.exe" );
 delete_file( egsBinDir + "/mortran3.dat" );
 copy( Mortran3_F, henHouse() + "/mortran3/mortran3.f.orig" );
 QString mortran3f = readFile2QString( Mortran3_F, (QString)"Error copying " +
                     Mortran3_F );

 if ( ft->successful_test( id["exitFun"] ) ){
   mortran3f.replace("call exit" , (QString)"call " + a[ "exitFun" ]  );
 }
 else if( ft->successful_test( id["stopFun"] ) ){
  mortran3f = replaceExit4Stop( mortran3f, a[ "stopFun" ] );
 }
 else{
  QString flush_it = QString();
  if ( ft->successful_test( id["flushFun"] ) ){
    flush_it = (QString)"call " + a["flushFun"] + (QString)"(7)\n";
    flush_it += (QString)"call " + a["flushFun"] + (QString)"(8)\n";
  }
  mortran3f.replace("call exit" , flush_it + (QString)"      call " +
                   a[ "egsExitFun" ]  );
 }

 if ( ! writeQString2File( mortran3f, Mortran3_F ) ) {
   printProgress( (QString)"\n Critical error: could not create mortran3.f "
                   + Mortran3_F );
   return;
 }

//*********************   Compiling Mortran3   ******************
  procStop();
  procInstall->setWorkingDirectory ( d.absolutePath() );
  procInstall->start( make->name(), QStringList() << "EGS_CONFIG="+specFile);
  if(procInstall->error()==QProcess::FailedToStart){
     printProgress( "Error executing " + make->name() + "EGS_CONFIG="+specFile );
     restore_mortran3();
     buildOK[ buildFlag - 1 ] = false;
  }
}

/********************
 *
 * STEP #
 *
 * Building pegs4
 *
 ********************/
void QInstallPage::buildPegs4(){

 //*********************   Compiling Pegs4   ******************
 printProgress( "\n===> Compiling Pegs4 ...\n\n");
 QDir d( henHouse() + "/pegs4/" );

 if ( ! d.exists() ){
    printProgress( tr("\n There is no pegs4 subdirectory ")+
                   tr("in your EGSnrc directory ")         +
                   henHouse()                             +
                   (QString)"\n Installation stopped ! \n" );
    return;
 }

 delete_file( egsBinDir + "/pegs4.exe" );
 delete_file( d.absolutePath() + "/pegs4_" + my_machine() + ".f" );
 procStop();
 procInstall->setWorkingDirectory ( d.absolutePath() );
 procInstall->start( make->name(), QStringList() << "EGS_CONFIG="+specFile);
 if(procInstall->error()==QProcess::FailedToStart){
     printProgress( "Error executing " + make->name() + "EGS_CONFIG="+specFile );
     buildOK[ buildFlag - 1 ] = false;
 }
}

int iaea_steps = 0;
void QInstallPage::buildIAEALib(){
 setSubTitle("Compiling IAEA Library");
 //*****************   Compiling IAEA library ******************
 printProgress( "\n===> Compiling IAEA library ...\n\n");
 QDir d( henHouse() + "iaea_phsp/" );

 if ( ! d.exists() ){
    printProgress( tr("\n There is no iaea_phsp subdirectory ")+
                   tr("in your EGSnrc directory ")         +
                   henHouse() + tr("\n\n"));
    buildOK[ buildFlag - 1 ] = false;
    emit nextBuildStep( buildFlag );
    return;
 }
 /* delete existing binaries */
 delete_files( dsoDir + my_machine(), QString("*iaea_*"));

 resetProgressBar(5); iaea_steps = 0;
 procStop();
 procInstall->setWorkingDirectory ( d.absolutePath() );
  procInstall->start( make->name(), QStringList() << "-j" << "EGS_CONFIG="+specFile);
 if(procInstall->error()==QProcess::FailedToStart){
     printProgress( "Error executing " + make->name() + "-j" + "EGS_CONFIG="+specFile );
     buildOK[ buildFlag - 1 ] = false;
    emit nextBuildStep( buildFlag );
 }
}

int egspp_step;

void QInstallPage::buildEGSPPLib(){
 setSubTitle("Compiling EGSnrc C++ Library");
 //*****************   Compiling C++ library ******************
 printProgress( "\n===> Compiling C++ library ...\n\n");
 QDir d( henHouse() + "egs++/" );

 if ( ! d.exists() ){
    printProgress( tr("\n There is no egs++ subdirectory ")+
                   tr("in your EGSnrc directory ")         +
                   henHouse() + tr("\n\n"));
    buildOK[ buildFlag - 1 ] = false;
    emit nextBuildStep( buildFlag );
    return;
 }
 /* delete existing binaries */
 delete_files( dsoDir + my_machine(), QString("*egs*"));

 resetProgressBar(196); egspp_step = 0;
 procStop();
 procInstall->setWorkingDirectory ( d.absolutePath() );
 procInstall->start( make->name(), QStringList() << "-j" << "EGS_CONFIG="+specFile);
 if(procInstall->error()==QProcess::FailedToStart){
     printProgress( "Error executing " + make->name() + " -j" + " EGS_CONFIG="+specFile );
     buildOK[ buildFlag - 1 ] = false;
    emit nextBuildStep( buildFlag );
 }
}

//********************
// Auxiliary methods
//********************
void QInstallPage::buildEGSCode( const QString& workDir ){
  QDir d( workDir );
  if ( ! d.exists() ){
    printProgress( "\n There is no " + workDir +
                   "directory ! \n" +
                   "\n Installation stopped ! \n" );
    return;
  }
  QStringList args; args << "EGS_CONFIG=" + specFile << "EGS_HOME="   + egsHome();
  procStop();
  procInstall->setWorkingDirectory ( workDir );
  procInstall->start( make->name(), args );
  if(procInstall->error()==QProcess::FailedToStart){
     QString err = "Error executing " + make->name() + args.join(" ") + "\n";
     err += "Check that compiler and make can be found through your PATH environment variable!\n";
     printProgress( err );
     buildOK[ buildFlag - 1 ] = false;
     emit nextBuildStep( buildFlag );
  }
}

void QInstallPage::procProgress(){
    printProgress(QString(procInstall->readAllStandardOutput()));
    if      ( buildFlag - 1 == egspp ) egspp_step++;
    else if ( buildFlag - 1 == iaea )  iaea_steps++;
    else                               build_steps++;
    if (!installing_beam) updateProgress();
}

void QInstallPage::procEnd(int exitCode, QProcess::ExitStatus exitStatus){

    if ( buildFlag - 1 == mortran3 ) //move mortran3.f.orig back to mortran3.f
         restore_mortran3();


    if ( exitStatus != QProcess::NormalExit || exitCode != 0 ){
        if (user_aborted){
            printProgress( "Compilation aborted!" );
            buildOK[ buildFlag - 1 ] = false;
            user_aborted = false;
            emit AllDone();
            return;
        }
        else{
            printProgress( "Compilation failed!" );
            buildOK[ buildFlag - 1 ] = false;
            emit nextBuildStep( buildFlag );
            return;
        }
    }

    printProgress( "\nCompilation succeeded !\n" );
    buildOK[ buildFlag - 1 ] = true;

    if ( buildFlag - 1 == egspp || buildFlag - 1 == iaea) {
       //qDebug("egs++ steps = %d iaea steps = %d",egspp_step, iaea_steps);
       progressBar->setValue( progressBar->maximum() );
       timeStamp();
    }

    emit nextBuildStep( buildFlag );
}

void QInstallPage::restore_mortran3()
{
    move_file( henHouse() + "/mortran3/mortran3.f",
               henHouse() + "/mortran3/mortran3_" + my_machine() + ".f" );
    move_file( henHouse() + "/mortran3/mortran3.f.orig",
               henHouse() + "/mortran3/mortran3.f" );
}

QString QInstallPage::replaceExit4Stop( const QString& code, const QString& stopFun )  {
  QString tmp = code, found; int pos = 0;
  QRegExp rx( "call(\\s+)exit(\\s*)\\((\\s*\\d+\\s*)\\)" );
  while ( pos >= 0 ) {
    pos = rx.indexIn( tmp, pos );
    if ( pos > -1 ) {
        found = rx.cap( 0 );
        found = found.simplified();
        pos  += rx.matchedLength();
        found.remove( QRegExp("call\\s+exit\\s*\\(\\s*") );
        found.remove( QRegExp("\\s*\\)") );
        //cout << "Found " << rx.cap(0) << " got " << found << endl;
        tmp.replace( QRegExp("call(\\s+)exit(\\s*)\\((\\s*" + found + "\\s*)\\)" ), stopFun + (QString)" " + found );
        pos = 0;
    }
  }
  return tmp;
}

void QInstallPage::finalize_egs_config( const QString &the_readme ){
   QString dummyStr = the_readme;
   updateProgress();
   //qDebug("System config steps = %d",build_steps);
   timeStamp();
   emit systemCreated(egsppspec);
}

void QInstallPage::timeStamp(){
   double t_elapsed = the_time.elapsed()/1000.;// elapsed time in seconds
   printProgress("*********************************");
   if (t_elapsed < 60)
      printProgress(QString("*     Elapsed time: %1 s *").arg(t_elapsed));
   else
      printProgress(QString("*     Elapsed time: %1 min *").arg(t_elapsed/60.));
   printProgress("*********************************");
#ifdef CONF_DEBUG
   qDebug("Time elapsed: %g s", t_elapsed);
#endif
   //the_time.restart();
}

void QInstallPage::finalize_cpp(){

  /* append IAEA variables to EGS config file */
     QString the_iaea;
     // if succesful compilation of IAEA library for space files
     if (buildOK[iaea]){
        the_iaea = QString(IAEA_VARS);
        the_iaea.replace(QString("$lib_link1"), cpp->dsoPath());
        the_iaea.replace((QString)"$link2_prefix_", cpp->LinkPrefix() );
        the_iaea.replace((QString)"$link2_suffix", cpp->LinkSuffix() );
        the_iaea.replace(QString("$iaea_phsp_macros"),
                         QString("$(EGS_UTILS)iaea_phsp_macros.mortran") );
     }
     else{
        the_iaea = QString(NO_IAEA_VARS);
     }

     append2file(the_iaea.toLatin1(),specFile.toLatin1());

    //configCPPProgressBar->setProgress( configCPPProgressBar->totalSteps() );
    emit cppBuildFinalized();
}

int QInstallPage::howManyFilesInDir(const QString &dirPath){
    int count = 0;
    QDir d(dirPath);
    QStringList qsl = d.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
    foreach (QString file, qsl) {
        QFileInfo finfo(dirPath + QDir::separator() + file);
        if (finfo.isDir()) count += howManyFilesInDir(finfo.filePath());
        else               count++;
    }
    return count;
}

/* Copies any directory structure inside srcFilePath into tgtFilePath*/
bool QInstallPage::copyRecursively(const QString &srcFilePath, const QString &tgtFilePath)
{
    QFileInfo srcFileInfo(srcFilePath);
    if (srcFileInfo.isDir()) {
        QDir targetDir(tgtFilePath);
        QDir sourceDir(srcFilePath);
        if (!QDir().mkpath(tgtFilePath)) {
          printProgress("***Failed creating " + tgtFilePath);
          return false;
        }
        QStringList fileNames = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        foreach (const QString &fileName, fileNames) {
            const QString newSrcFilePath
                    = srcFilePath + QDir::separator() + fileName;
            const QString newTgtFilePath
                    = tgtFilePath + QDir::separator() + fileName;
            if (!copyRecursively(newSrcFilePath, newTgtFilePath))
                return false;
        }
    } else {
        qApp->processEvents();
        if (QFile(tgtFilePath).exists()) QFile::remove(tgtFilePath);
        printProgress(tr("Copying ") + srcFilePath + " to " + tgtFilePath);
        if (!QFile::copy(srcFilePath, tgtFilePath)){
            printProgress("***Failed!");
            return false;
        }
        updateProgress();
    }
    return true;
}

/* Copies files matching nameFilter from any directory inside srcFilePath
 * into tgtFilePath which must already exist */
bool QInstallPage::copyFilesRecursively(const QString &srcFilePath,
                                   const QString &tgtFilePath, const QString & nameFilter)
{
    QFileInfo srcFileInfo(srcFilePath);
    if (srcFileInfo.isDir()) {
        QDir targetDir(tgtFilePath);
        QDir sourceDir(srcFilePath);
        if (!targetDir.exists()) {
           printProgress("*** Directory " + tgtFilePath + " does not exist!");
           return false;
        }
        QStringList fileNames = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);// | QDir::System );
        foreach (const QString &fileName, fileNames) {
            const QString newSrcFilePath
                    = srcFilePath + QDir::separator() + fileName;
            if (!copyFilesRecursively(newSrcFilePath, tgtFilePath, nameFilter))
                return false;
        }
    } else {
        if (!nameFilter.isEmpty()){
          QRegExp rx(nameFilter); rx.setPatternSyntax(QRegExp::Wildcard);
          if (!rx.exactMatch(srcFilePath)) return true;
        }
        qApp->processEvents();
        QString tgtFile = tgtFilePath + QDir::separator() + QFileInfo(srcFilePath).fileName();
        if (QFile(tgtFile).exists()) QFile::remove(tgtFile);
        printProgress(tr("Copying ") + srcFilePath + " to " + tgtFile);
        if (!QFile::copy( srcFilePath, tgtFile )){
            printProgress("***Failed!");
            return false;
        }
        updateProgress();
    }
    return true;
}

void QInstallPage::copy_user_codes(){

    //if ( !ucCheckBox->isChecked()){
    if ( !needsUCs()){
       emit userCodesCopied();
       return;
    }

     resetProgressBar( howManyFilesInDir(henHouse() + "user_codes") );
     setSubTitle("Copying user codes");

     printProgress((QString)"\nCopying user codes to user area .... \n\n");
     QString eh = egsHome(); eh.chop(1);
     if ( ! copyRecursively( henHouse() + "user_codes", eh) ){
        printProgress( (QString)"\nError copying user codes from " + henHouse()
               + (QString)"user_codes/ to " + egsHome() + (QString)"\n");
        printProgress( "Aborting user area setup early .... \n" );
        printProgress( "Check that the user codes exist in the $HEN_HOUSE\n" );
        printProgress( "and your write permissions in the $EGS_HOME area.\n" );
        printProgress( tr("In any case you can copy the user codes manually ")+
                       tr("and build them by runing make") );
        return;
     }
     emit userCodesCopied();
}