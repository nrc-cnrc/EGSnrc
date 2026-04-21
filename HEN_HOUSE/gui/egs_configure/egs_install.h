/*
###############################################################################
#
#  EGSnrc configuration GUI headers
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


#ifndef EGS_INSTALL_H
#define EGS_INSTALL_H

#include <QWizardPage>
#include <QProgressBar>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QTextStream>
#include <QMessageBox>
#include <QDir>
#include<QCheckBox>

#include "egs_tools.h"
#include "egs_archive.h"

enum BuildSteps{
    corespec,
    mortran3,
    pegs4,
    density_corrections,
    egsppspec,
    iaea, egspp, cppDone,
    sysDone,
    UserCode,
    homeDone,
// BEAMnrc steps
    beamBuild,
    addPhSp,
    beamDP,
    ctCreate,
    readPhSp,
    statDose,
#ifndef WIN32
    DOSxyzShow,
#endif
    buildDOSXYZnrc,
    exampleModules,
    NUMBER_OF_STEPS
};

class QWizardPage;
class EGS_Archive;

class QInstallPage : public QWizardPage
{

  Q_OBJECT

public:

  QInstallPage(QWidget * parent, MCompiler *a_m, MCompiler *a_f,
                                  MCompiler *a_c, MCompiler *a_cpp);
  ~QInstallPage(){}

public slots:

  void initializePage();
  void cleanupPage();
  void initializeConnections();
  void updateProgress();
  void start();
  void abort();
  void resetPage();
  void switch2EGSnrc();
  /* Core System Configuration */
  void run_tests();
  void create_egs_c_utils();
  void check_egs_c_utils();
  void test_c_utils();
  void get_test_c_utils_result();
  void test_load_beamlib();
  void get_test_load_beamlib();
  void createSystemFiles();
  void create_dosxyz_spec(bool rw_pardose_ok);
  void append_vculib_dosxyz_spec(bool load_vculib_ok);
  void buildMortran3();
  void restore_mortran3();
  void buildPegs4();
  void copy_user_codes();

  /* C++ Library Configuration */
  void create_egspp_config();
  void buildIAEALib();
  void buildEGSPPLib();
  void finalize_cpp();
  /* Environment Configuration */
  void SaveAppSetting();
  void update_path( const QString& the_dir );
  bool cleanPath();
  void createEGSFolders();
  void createBeamFolders();
  void environmentSetUp();
  void setup_static_guis();
  void set_guis_dso();
  void update_unix_env();
  void createWinShortcuts( const QString& where,
                           const QString& from,
                               QStringList& link,
                               QStringList& dir,
                               QStringList& desc,
                               QStringList& icons,
                               QString& msg     );
  void createKDEShortcuts( const QString& where,
                           const QString& from,
                               QStringList& link,
                               QStringList& icons,
                               QStringList& scripts,
                               QStringList& dir,
                               QString& msg     );
  /* BEAMnrc Configuration */
  void beamInstall();
  void buildBEAMnrc( ushort code );
  //void build_prog(int i);
  //void make_beam_build_tool();
  //void copy_spec_modules();
  void copy_example_modules();
  void finalize_beam_setup();
  /* Auxiliar methods */
  void buildEGSCode( const QString& workDir );
  void buildEGSnrc( ushort code );
  void procStop();
  void procProgress();
  void procEnd(int exitCode, QProcess::ExitStatus exitStatus);
  QString replaceExit4Stop( const QString& code, const QString& stopFun );
  void cleanUp();
  void finalize_egs_config( const QString &the_readme );
  void timeStamp();
  /* Copies any directory structure inside srcFilePath into tgtFilePath*/
  bool copyRecursively(const QString &srcFilePath,  const QString &tgtFilePath);
  /* Copies files matching nameFilter from any directory inside srcFilePath into tgtFilePath
     which must already exist */
  bool copyFilesRecursively(const QString &srcFilePath,
                       const QString &tgtFilePath, const QString & nameFilter = QString());
  int howManyFilesInDir(const QString &dirPath);
  bool copy_files( const QString& src, const QString& trgt,  const QString& filter );
  /*Check itself to see whether it is a full installation*/
  bool itsAZip(){
     EGS_Archive* archi = new EGS_Archive(this); bool isAZippo = false;
     if ( archi->isZipFile(QCoreApplication::applicationFilePath().toLatin1().data()) ) isAZippo = true;
     delete archi;
     return isAZippo;
  };
  void extract( const char* archive, const char* dir );
  void customEvent(QEvent * event);
  void processExtaction();
signals:

  void doFortranTests();
  void egsCUtilsCreated();
  void egsCUtilsTested();
  void egsCUtilsEnded();
  void egsCUtilsFailed();
  void LoadBeamLibTested();
  void gotCompilerOptions();
  void systemCreated( ushort );
  void cppSystemCreated( ushort );
  void cppBuildFinalized();
  void nextBuildStep( ushort );
  void egsSysBuilt();
  void environmentSet();
  void userCodesCopied();
  void AllDone();
  void threadRunning();
//BEAMnrc signals
  void progsBuilt();
  void dosxyzBuilt();
  void exampleModulesCopied();
  void beamDone();
  void skipBeam();

private:

  QString henHouse();
  QString egsHome();
  QString confFile(){QString le_conf = field("egs_conf").toString(); return le_conf+".conf";}
  QString canonical(){return field("Canonical").toString();}
  QString my_machine(){return field("conf_name").toString();}
  bool needsUCs(){return field("copyUCs").toBool();}
  QString readFile2QString( const QString& fname, const QString& err );
  bool writeQString2File( const QString& the_string, const QString& fname );
  bool appendQString2File( const QString& the_string, const QString& fname );
  void resetProgressBar(const int &nsteps);
  //void printProgress( const QString& message );
  void printProgress( const QString& message, bool new_line = true );

//************************************************************************

/* Creates a log file to store useful information about
 * the installation process.
   This log file is created in $HEN_HOUSE/install_status
*/
void createLogFile(const QString & dir);

void createDirs();

void createDir( QString &dir, bool critical, const QString & def);
void createDir( QString dir, const QString & def){ createDir( dir, false, def );}
void createDir( QString dir, bool critical){ createDir( dir, critical, QString() );}
void createDir( QString dir){ createDir( dir, false, QString() );}

  MTest        *ft, *ct; // Two needed since results from tests needed to create system
  MCompiler    *fc, *cc, *cpp, *make; // Compilers
  QProgressBar *progressBar;
  QTextEdit    *screen;
  QPushButton  *installButton;
  QCheckBox    *envCheckBox, *shortcutCheckBox, *ucCheckBox, *beamCheckBox;
  QTextStream   config_log;
  QFile        *config_file;
  QProcess     *procInstall;
  EGSThread    *t;
  QTime         the_time;
  QString       installationDir,
                egsBinDir,
                homeBinDir,
                egsLibDir,
                dsoDir,
                specFile,
                specFileCPP,
                pegsDir,
                dlopen_flags; // Must be added to $extra in egspp*.conf file
  int           ntasks, istep, n_config_steps, i_config_steps, n_beam_steps;
  BuildSteps    buildFlag;
  bool          egs_c_utils_ok,
                load_beamlib_ok, load_vculib_ok,
                read_write_pardose_ok, installing_beam,
               *buildOK, user_aborted, skip_config;
};

#define HAS_C_COMPILER "# We have a C compiler that succesfully compiled egs_c_utils.c\n"\
"# For simplicity in the Makefiles, we always include egs_c_utils.o\n"\
"# in the link step, even if the functions are not used. This is not\n"\
"# too wasteful as egs_c_utils.o is quite small.                    \n"

#define HAS_NO_C_COMPILER "# No working C compiler.  \n"\
"# If you ever manage to compile egs_c_utils.c, set the following variable\n"\
"# to "

#define MACHINE_HAS_C_COMPILER "\" mortran3 gets confused by the # char => we need to pass it as an \"\n"\
"\" argument to the macro. \"\n"\
"REPLACE {$HAVE_C_COMPILER(#);} WITH {{EMIT;{P1}define HAVE_C_COMPILER};}\n"

#define MACHINE_HAS_NO_C_COMPILER "\" You either have no C compiler installed or it failed to compile \"\n"\
"\" egs_c_utils.c in this directory. If you install a C compiler that can \"\n"\
"\" compile egs_c_utils.c, uncomment the next line and remove the null \"\n"\
"\" replacement \"\n"\
"\"REPLACE {$HAVE_C_COMPILER(#);} WITH {{EMIT;{P1}define HAVE_C_COMPILER};}\"\n"\
"REPLACE {$HAVE_C_COMPILER(#);} WITH {;}\n"

#define BEAMLIB_OBJECTS_HEADER1 "# Use the following variable in EGSnrc user codes that \n"\
"# use a BEAMnrc user code compiled and linked as a DSO\n"\
"# (dynamic shared object, a.k.a DLL in the Windows world)\n"\
"# as a particle source.\n"

#define BEAMLIB_OBJECTS_HEADER2 "# If you ever manage to compile ${HEN_HOUSE}cutils/load_beamlib.c,\n"\
"# set the following variable to\n"\
"# $HEN_HOUSE/lib/$my_machine/load_beamlib.$test_objext\n"\
"#\n"

#define BEAMLIB_EXTRA_LIBS_HEADER1 "# load_beamlib uses system calls to load a DSO and to resolve the \n"\
"# addresses  of the various functions in the DSO (dlopen and dlsym\n"\
"# on Unix, LoadLibrary and GetProcAddress on Windows).\n"\
"# On Linux, Windows with GCC and various other Unixes the system\n"\
"# library that must be linked to get access to these system calls\n"\
"# is dl. If this is not the case for you, adjust accordingly.\n"

#define BEAMLIB_EXTRA_LIBS_HEADER2 "# System libraries that provide access to the dynamic linking \n"\
"# facilities. Left empty as load_beamlib.c could not be compiled, \n"\
"# has to be changed to something like -ldl if you manage to compile\n"\
"# load_beamlib.c \n"\
"#\n"

#define UNKNOWN_REC_LEN "\" !!!!!!!!!!!!!! Unknow record length for unformatted I/O !!!!!!!!!!!!!\"\n"\
"\" !!!!!!!!!!!!!! Please replace !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\"\n"

#define HAS_FLUSH_FUN "REPLACE {$FLUSH_UNIT(#);} WITH {call flushFun({P1});}\n"
#define HAS_NO_FLUSH_FUN "REPLACE {$FLUSH_UNIT(#);} WITH {;}\n"

#define HAS_EXIT   "REPLACE {$CALL_EXIT(#);} WITH {call exitFun({P1});}\n"
#define HAS_STOP "REPLACE {$CALL_EXIT(#);} WITH {stopFun {P1};}\n"
#define USE_EGS_EXIT "REPLACE {$CALL_EXIT(#);} WITH {call egsExitFun({P1});}\n"

#define HAVE_LOAD_DSO "REPLACE {$HAVE_LOAD_DSO(#);} WITH {{EMIT;{P1}define HAVE_LOAD_DSO};}\n"

#define HAVE_NOT_LOAD_DSO "\" Same as above: if you manage to compile load_beamlib.c, uncomment the next \"\n" \
"\" line and remove the null replacement \"\n" \
"\"REPLACE {$HAVE_LOAD_DSO(#);} WITH {{EMIT;{P1}define HAVE_LOAD_DSO};}\"\n" \
"REPLACE {$HAVE_LOAD_DSO(#);} WITH {;}\n" \
";\n"

#define UNIX_EGS_INSTALLED "\n\nCongratulations! You successfully configured the EGSnrc build system\n"\
"for Unix/Linux. You can use this configuration for compiling EGSnrc user codes\n"\
"by the following 4 methods:\n"\
"\n"\
"  1. Set the environment variable EGS_CONFIG to point to the file\n"\
"     $HEN_HOUSEspecs/$conf_file, e.g.\n"\
"\n"\
"     for the Bourne (again) shell or the Korn shell use \n"\
"       export EGS_CONFIG=$HEN_HOUSEspecs/$conf_file\n"\
"     for the C-shell or tcsh use\n"\
"       setenv EGS_CONFIG $HEN_HOUSEspecs/$conf_file\n"\
"\n"\
"     and then use either the compilation script compile_user_code (aliased to mf)\n"\
"     or just go to a user code directory and type 'make'.\n"\
"\n"\
"  2. By running the compile script with an argument specifying to use this \n"\
"     configuration, e.g.\n"\
"       $HEN_HOUSEscripts/compile_user_code tutor1 config=$conf_file\n"\
"\n"\
"  3. By invoking make with an argument specifying to use this configuration,e.g\n"\
"       make EGS_CONFIG=$conf_file\n"\
"\n"\
"  4. By using one of the GUI's egs_gui or egs_inprz (RZ codes only)\n\n"

#define WIN_EGS_INSTALLED "\n\nCongratulations! You successfully configured the EGSnrc build system\n"\
"for Windows. You can use this configuration for compiling EGSnrc user codes\n"\
"by the following 3 methods:\n"\
"\n"\
"  1. Set the environment variable EGS_CONFIG to point to the file\n"\
"     $HEN_HOUSEspecs/$conf_file by typing on your command prompt\n"\
"           set EGS_CONFIG=$HEN_HOUSEspecs/$conf_file\n"\
"     for a temporary setting or by adding/updating EGS_CONFIG permanently\n"\
"     on your system properties:\n"\
"\n"\
"     Start->[Settings]->Control panel->System->Advanced->Environment Variables\n"\
"\n"\
"     and then just go to a user code directory and type 'make'.\n"\
"\n"\
"\n  Note: EGS_CONFIG might have been already set by the installation program\n"\
"\n"\
"  2. By invoking make with an argument specifying to use this configuration,e.g\n"\
"       make EGS_CONFIG=$conf_file\n"\
"\n"\
"  3. By using one of the GUI's egs_gui or egs_inprz (RZ codes only)\n\n"

#define READ_ME "\n\n This is EGSnrcMP $my_version\n"\
"------------------------------\n"\
"\n"

#define UPDATE_UNIX_ENVIRONMENT "\n\n***************************\n"\
" IMPORTANT NOTE : \n"\
"***************************\n"\
"To start using the EGSnrc system, activate your current configuration\n"\
"by adding the following lines to your favorite shell resource file: \n"\
"\n"\
"if your default shell is a C-shell or derivative:\n"\
"\n"\
"setenv EGS_HOME $EGS_HOME\n"\
"setenv EGS_CONFIG $EGS_CONFIG\n"\
"source $HEN_HOUSEscripts/egsnrc_cshrc_additions\n"\
"\n"\
"if your default shell is a Bourne shell or derivative:\n"\
"\n"\
"EGS_HOME=$EGS_HOME\n"\
"EGS_CONFIG=$EGS_CONFIG\n"\
"export EGS_HOME EGS_CONFIG\n"\
". $HEN_HOUSEscripts/egsnrc_bashrc_additions\n"

#define LNBLNK "C*****************************************************************************\n"\
"C\n"\
"C Our coleague Blake Walters loves using lnblnk instead of the function \n"\
"C lnblnk1 provided by EGSnrc, even though he knows that lnblnk is not \n"\
"C available for all compilers. This one does not support it and so, here \n"\
"C is a replacement.\n"\
"C\n"\
"C*****************************************************************************\n"\
"      integer function lnblnk(string)\n"\
"      character*(*) string\n"\
"      integer i,j\n"\
"     do i=len(string),1,-1\n"\
"       j = ichar(string(i:i))\n"\
"        if( j.ne.9.and.j.ne.10.and.j.ne.11.and.j.ne.12.and.j.ne.13.\n"\
"     &      and.j.ne.32 ) then\n"\
"          lnblnk = i\n"\
"          return\n"\
"        end if\n"\
"      end do\n"\
"      lnblnk = 0\n"\
"      return\n"\
"      end\n"

static const char spec_file[]={
"#*************************************************************************\n"\
"#\n"\
"# EGSnrc configuration file \n"\
"#\n"\
"# Created by $my_name version $my_version on $current_date \n"\
"#\n"\
"# Attention: all changes you make to this file may be lost next time\n"\
"# you run $my_name.\n"\
"#\n"\
"#*************************************************************************\n"\
"\n"\
"DSEP$DSEP\n"\
"\n"\
"my_machine = $conf_name\n"\
"canonical_system = $canonical_system\n"\
"make_prog = $make_prog\n"\
"\n"\
"HEN_HOUSE = $HEN_HOUSE\n"\
"SPEC_DIR = $(HEN_HOUSE)specs$(DSEP)\n"\
"\n"\
"# Include the standard $OS spec file\n"\
"#\n"\
"include $(SPEC_DIR)$std.spec\n"\
"\n"\
"# Include definitions common for all systems.\n"\
"# These are mainly directory names, executable names, etc.,\n"\
"# which are constructed from the previous definitions.\n"\
"#\n"\
"include $(SPEC_DIR)all_common.spec\n"\
"\n"\
"# Fortran compiler name and options\n"\
"#\n"\
"F77 = $F77\n"\
"F77_LINK = $EGS_F77_LINK\n"
"FCFLAGS = $FFLAGS\n"\
"FOPT = $FOPT\n"\
"FDEBUG = $FDEBUG\n"\
"FLIBS = $FLIBS\n"\
"FOUT = $FOUT\n"\
"FOBJE = $EGS_FOBJE\n"
"\n"\
"# C compiler name and options\n"\
"#\n"\
"CC = $CC\n"\
"C_FLAGS = $C_FLAGS\n"\
"COUT = $COUT\n"\
"\n"\
"# The following is for creating a DSO (Dynamic Shared Object)  \n"\
"# also known as shared library (DLL) in the Windows world.     \n"\
"# At this point we don't have a procedure in place to determine\n"\
"# the compile/link flags necessary to create a DSO.            \n"\
"# We therefore put generic values here that are known to work  \n"\
"# on Linux with the GNU, PGI and Intel compilers, on Windows   \n"\
"# with the GNU compiler and on IRIX with the SGI compiler.     \n"\
"# It is up to the users to adjust this if it does not work for \n"\
"# them. Note that the -Wl,-Bsymbolic option is needed on the   \n"\
"# tested systems to make fortran common blocks local to the DSO\n"\
"#\n"\
"#SHLIB_FLAGS = -shared -Wl,-Bsymbolic                           \n"\
"SHLIB_FLAGS = $EGS_SHLIB_FLAGS                                 \n"\
"\n"\
"# When using the GNU compiler, one must link against the static\n"\
"# g2c library, otherwise I/O is shared between the             \n"\
"# main process and the library => the library process fails    \n"\
"# because units that it wants to open are already opened       \n"\
"# by the main process.                                         \n"\
"# The following variable is set to be empty, but if you use    \n"\
"# the GNU compiler set it to /usr/lib/libg2c.a (or wherever    \n"\
"# your static g2c library is installed).                       \n"\
"#\n"\
"SHLIB_LIBS = $EGS_SHLIB_LIBS\n"\
"\n"\
"\n"\
"$C_COMPILER_AVAILABILITY $WOULD_BE_EGS_C_UTIL\n"\
"#\n"\
"CUTIL_OBJECTS = $EGS_C_UTILS_OBJ\n"\
"\n"\
"#\n"\
"$BEAMLIB_OBJECTS_HEADER\n"\
"#\n"\
"BEAMLIB_OBJECTS = $LOAD_BEAMLIB\n"\
"\n"\
"$BEAMLIB_EXTRA_LIBS_HEADER\n"\
"#\n"\
"BEAMLIB_EXTRA_LIBS = $dlopen_flags $(IAEA_LIB)\n"\
"\n"\
"# FC_FLAGS gets used for compiling the EGSnrc fortran routines and for \n"\
"# linking for EGSnrc user codes written in C. We set FC_FLAGS by \n"\
"# default to be given by $(FCFLAGS) $(FOPT). This is OK for most \n"\
"# compilers. Unfortunately, some Fortran compilers insert a default \n"\
"# main function and then complain about multiply defined main \n"\
"# (theirs and the main of the user code written in C), unless a special \n"\
"# flag is passed. For instance, the PGI compiler needs -Mnomain. \n"\
"# As I don't know how to test for this feature, it is left up to you \n"\
"# to read the documentation of your compiler and adjust FC_FLAGS in \n"\
"# case it does not work.\n"\
"#\n"\
"FC_FLAGS = $(FOPT) $(FCFLAGS) $FEXTRA \n"\
"\n"\
"# The following variables are needed to define the IAEA phase space library \n"\
"# and the way EGSnrc user codes link against it. \n"\
"# \n"\
"dso = dso$(DSEP)$(my_machine) \n"\
"abs_dso = $(HEN_HOUSE)egs++$(DSEP)$(dso) \n"\
"ABS_DSO = $(abs_dso)$(DSEP) \n"\
};

#define IAEA_VARS "IAEA_LIB = $lib_link1 $link2_prefix_iaea_phsp$link2_suffix \n"\
"IAEA_PHSP_MACROS = $iaea_phsp_macros\n"

#define NO_IAEA_VARS "IAEA_LIB = \n"\
"IAEA_PHSP_MACROS = \n"

static const char egspp_spec_file[]={
"#*************************************************************************\n"\
"#\n"\
"# EGSnrc C++ configuration file \n"\
"#\n"\
"# Created by \n"\
"# $my_name version $my_version\n"\
"# on $current_date \n"\
"#\n"\
"# Attention: all changes you make to this file may be lost next time\n"\
"# you run $my_name.\n"\
"#\n"\
"#*************************************************************************\n"\
"\n"\

"# The C++ compiler\n"\
"#\n"\
"CXX = $CXX\n"\
"\n"\
"#\n"\
"# The compilation options\n"\
"#\n"\
"opt = $opt\n"\
"\n"\
"#\n"\
"# The switch to create a shared library\n"\
"#\n"\
"shared = $shared\n"\
"#\n"\
"#\n"\
"# Library prefix and extension\n"\
"#\n"\
"libpre = $libpre\n"\
"libext = $libext\n"\
"#\n"\
"#\n"\
"# The object file extension\n"\
"#\n"\
"obje = $obje\n"\
"#\n"\
"#\n"\
"# Configuration specific definitions for the preprocessor\n"\
"#\n"\
"DEF1 = $defines $fpic\n"\
"#\n"\
"#\n"\
"# Extra arguments passed to the linker\n"\
"#\n"\
"extra = $extra\n"\
"#\n"\
"#\n"\
"# Extra step after building the DSO (may be needed for Windows when\n"\
"# using g++ to create the .lib and .exp files using the lib tool\n"\
"#\n"\
"extra_link = $extra_link\n"\
"#\n"\
"#\n"\
"# How to name the executable\n"\
"#\n"\
"EOUT = $eout\n"\
"#\n"\
"#\n"\
"# How to encode the library path into the executable.\n"\
"# If this is not available for your system, you have to add the directory\n"\
"#    ${hen_house}egs++/dso/$my_machine\n"\
"# to your library search path. On many (but not all) systems, this is\n"\
"# achieved by defining the environment variable LD_LIBRARY_PATH to contain\n"\
"# the above path.\n"\
"#\n"\
"lib_link1 = $lib_link1\n"\
"#\n"\
"#\n"\
"# Switches for linking against a shared library\n"\
"#\n"\
"link2_prefix = $link2_prefix\n"\
"link2_suffix = $link2_suffix\n"\
"#\n"\
"#\n"\
"# Libraries needed when linking together C++ and Fortran code and the linking \n"\
"# is done by the C++ compiler.\n"\
"#\n"\
"fortran_libs = $f_libs\n"\
"#\n"
};

#define MACOS_BUNDLES "# Unlike other systems, on OSX a dynamically loadable"\
" library is a     \n"\
"# different thing from a shared library (it is called a bundle).          \n"\
"# Thus, we need extra targets and switches to build bundles.              \n"\
"#                                                                         \n"\
"MACOSX = yes                                                              \n"\
"shared_bundle = $shared_bundle                                            \n"\
"libext_bundle = $libext_bundle                                            \n"

#define DSO_TOOLTIP "Using following compiler/linker switches for creating/linking\n"\
"against dynamic shared objects (DSO, a.k.a. shared library or DLL), where\n"\
"$(abs_dso) or $(ABS_DSO) below will be replaced with the absolute path to the\n"\
"EGSnrc DSO directory at compile/link time."

static const char machine_macros[]={
"%E\n"\
"%I4\n"\
"!INDENT F2;\n"\
"%C80\n"\
"%Q1\n"\
"\"*************************************************************************\n"\
"\"\n"\
"\" EGSnrc machine dependent macro file for \n"\
"\" $the_config\n"\
"\"\n"\
"\" Created by :\n"\
"\" $my_name version $my_version on $current_date\n"\
"\"\n"\
"\" You may add your own machine dependent macros to this file,\n"\
"\" but be carefull to not overwrite it if you re-run\n"\
"\" $my_name\n"\
"\"\n"\
"\"*************************************************************************\n"\
"\n"\
"REPLACE {$MACHINE} WITH \n"\
"  {,'$the_config',};\n"\
"REPLACE {$HEN_HOUSE} WITH \n"\
"  {'$$HEN_HOUSE'};\n"\
"REPLACE {$CANONICAL_SYSTEM} WITH \n"\
"  {'$canonical_system'};\n"\
"REPLACE {$CONFIGURATION_NAME} WITH \n"\
"  {'$conf_name'};\n"\
"REPLACE {$EGS_CONFIG} WITH \n"\
"  {'$spec_file'};\n"\
"\n"\
"REPLACE {$GIT_HASH} WITH \n"\
"  {'$git_hash'}; \n"\
"REPLACE {$CONFIG_TIME} WITH \n"\
"  {'$config_time'}; \n"\
"\n"\
"\" System dependent stuff \"\n"\
"\"========================================\"\n"\
"\" Unfortunately, there appears to be no reliable way of copying files \"\n"\
"\" under Fortran => we use a system call for this\" \n"\
"REPLACE {$copy_file} WITH {'$copyf '};\n"\
"\n"\
"\" Although one can move files using Fortran's intrinsic rename, \"\n"\
"\" we don't know whether the user has not created additional files in \"\n"\
"\" the temporary working directory => it is easiest to use a system call \"\n"\
"\" to move all files from the temporary working directory to the user code \"\n"\
"\" directory. \"\n"\
"REPLACE {$move_file} WITH {'$movef'};\n"\
"\n"\
"\" There appears to be no way of removing a directory from Fortran \"\n"\
"\" => we use a system call for this. \"\n"\
"REPLACE {$remove_directory} WITH {'$rmdir'};\n"\
"\n"\
"\" The directory separator \"\n"\
"REPLACE {$file_sep} WITH {$sep};\n"\
"\n"\
"REPLACE {$LONG_INT} WITH { LongInteger };\n"\
"REPLACE {$SHORT_INT} WITH { ShortInteger };\n"\
"THE_LONGEST_INT\n"\
"\"The machine byte order\"\n"\
"REPLACE {$BYTE_ORDER} WITH {'Endianess'};\n"\
";\n"\
"$NO_REC_LEN"\
"REPLACE {$RECL-FACTOR} WITH {recLen};\n"\
";\n"\
"$FLUSH_FUN"\
";\n"\
"\" If you want your user code to return an exit status, use the \n"\
"\" following macro to terminate execution\n"\
"$EXIT_FUN"\
";\n"\
"$MACHINE_C_COMPILER"\
";\n"\
"$MACHINE_HAVE_LOAD_DSO"
";\n"\
};

static const char machine_f[]={
"C***************************************************************************\n"\
"C\n"\
"C   This file was automatically generated by:\n"\
"C   $my_name version $my_version\n"\
"C   It contains various subroutines and functions for date, time, \n"\
"C   CPU time, host name, etc.\n"\
"C\n"\
"C   Attention: all changes will be lost the next time you run\n"\
"C   $my_name. \n"\
"C\n"\
"C***************************************************************************\n"
};

static const char guilnk[]={
"[Desktop Entry]\n"
"Exec=gui_script\n"
"Name[en_US]=gui_name\n"
"Icon=gui_icon\n"
"Terminal=false\n"
"Type=Application"
};

static const char gui_script[]={
"#! /bin/sh\n"
"export EGS_HOME=the_egshome\n"
"export HEN_HOUSE=the_henhouse\n"
"export EGS_CONFIG=the_egsconfig\n"
"#exportVIEW LD_LIBRARY_PATH=the_dso:$LD_LIBRARY_PATH # Only for egs_view\n"
"#exportBEAM PATH=ehBinDir:$PATH # Only for Tcl/Tk GUIs\n"
"gui_exe"
};

static const char egsnrc_bashrc_additions[]={
"\n"\
"###############################################################################\n"
"#                                                                              \n"
"# Set dynamic library path for the pre-compiled egs_view GUI                   \n"
"#                                                                              \n"
"###############################################################################\n"
"                                                                               \n"
"export LD_LIBRARY_PATH=${HEN_HOUSE}egs++/dso/static_machine:$LD_LIBRARY_PATH     "
};

#endif
