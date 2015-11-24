/*
###############################################################################
#
#  EGSnrc configuration GUI tools headers
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


#ifndef EGS_TOOLS_H
#define EGS_TOOLS_H

#include <QTextEdit>
#include <QTextStream>
#include <qprocess.h>
#include <qobject.h>
#include <qlibrary.h>
#include <qmap.h>
#include <qstring.h>

#include <QtXml>
#include <QFile>

#include <vector>

#if defined(QT_NO_THREAD)
#  error Thread support not enabled.
#endif

using namespace std;

typedef QMap<QString,QStringList> MSourceChunks;
typedef QMap<QByteArray,QString> MTestAnswer;
typedef QMap<QString,ushort> MTestID;
typedef std::vector<int> v_int;

// makefile language is similar to declarative programming
enum Language {F, C, CPP, GnuMake};

#define CRITICAL_ERROR "This is big trouble. Neither exit(status) nor stop "\
"\nstatus returns status as the exit value of a Fortran program."\
"\nThis means that a Make based build system is impossible "\
"\n(make may think that a mortran3 compilation has succeeded"\
"\nwhen it has actually failed or vice versa)."\
"\n "\
"\nReport this problem to Ernesto Mainegra-Hing (ernesto.mainegra-hing@nrc-cncr.gc.ca)"\
"\nPlease include "\
"\n  - Operating system (e.g. the output from 'Checking system type' above)"\
"\n  - Compiler name and version"\
"\n  - Compilation flags"\
"\nin your e-mail. You should also attach the file 'config.log' in the "\
"\nHEN_HOUSE/scripts directory."

class QTextEdit;
class QProcess;
class MTest;
class EGS_DSO;

#ifdef WIN32
static const char* objext[2]={
    "obj",
    "o"
};
/*static const char* exeext[1]={
    "exe"
};*/
#else
static const char* objext[1]={
    "o"
};
/*static const char* exeext[1]={
    "out"
};*/
#endif

bool        copy( const QString& source, const QString& target );
bool   move_file( const QString& source, const QString& target );
bool append2file( const char* source, const char* target);
void delete_file( const QString& target );
void delete_files( const QString& filter );
void delete_files( const QString& dir_str, const QString& filter );
bool  fileExists( const QString & fname );
bool   is_x86_64();
void chmod( const QString& attrib, const QString& file );
bool replaceUserEnvironmentVariable(  const QString& var, const QString& value, QString* msg);
bool prepend2UserEnvironmentVariable( const QString& var, const QString& value, QString* msg);
QString getUserEnvironmentVariable( const QString& var, QString* msg);
int createShortcut( const char* target, const char* link,
                    const char* desc, QString &return_message );
int createShortcut( const char* target, const char* link,
                    const char* desc, const char* icon,
                    int index, QString &return_message );
/*********************************************************************
 *
 *               **********        Tasks   ***********
 *
 *  Class Tasks: Defines a test to be done. So far one can check for
 *  successful compilation, execution, compare output from a program
 *  with a reference that is passed as an argument to the program. For
 *  each task one can define programing language, name, a tool function,
 *  libraries and objects to be linked to, etc.
 *
 **********************************************************************/
class Tasks{

    public:

    Tasks();
/*    void    setTask( const QString& fnam, const QString& fprog,
                     const QString& comp, const QString& opt,
		     const QString& lang, const QString& nam,
                     const char* arg_s, const QString& c_heck  );*/
    void    setResult( bool res ){ result = res;};
    void    setRunMode( bool run ){ run_mode = run;};
    void    setCriticalMode( bool critical ){ critical_mode = critical;};
    void    setDepMode( bool dep ){dependency = dep;};
    void    setToolFun( const QString& tFun ){ tool_func = tFun;};
    void    setAnswVar( const QString& a ){ answVar = a; };
    void    setAnswVal( const QString& a ){ answVal = a; };
    void    setProgram( const QString& prog ){ program = prog; };
    void    setLanguage( const QString& lang ){ language = lang; };
    void    setFName( const QString& fnam );
    void    setDepTask( const QString& dnam ){ depName.append( dnam ); };
    void    setOptions( const QString& opt ){ options = opt;};
    void    setTaskName(const QString& nam){ name = nam; };
    void    setChunks( MSourceChunks chks ){ chunks = chks; };
    void    setReplacementMap( MSourceChunks chks ){ replMap = chks; };
    void    setArgs( const QString& argmnts ){ args.append(argmnts); };
    void    setCheckMode( const QString& c ){ check = c;};
    void    setAnswKey( const QString& c ){ answ_key = c;};
    void    setReference(const QString& r){ reference = r; };
    void    setCurrentArg(const QString& r){ currentArg = r; };

    void    setObjects( const QString& ob ){objs = ob;};
    void    setLibraries( const QString& li ){libs = li;};

    void    replaceKey( const QString& k ){ replKey.append( k );   };
    void    replaceAnsw( const QString& k ){ replAnsw.append( k );   };

    void    setDeleteFlag( bool del ){ delete_source = del;};
    bool    removeSource(){ return delete_source;};

    bool     get_result(){return result;};
    bool     RunAllSteps(){return run_mode;};
    bool     isCriticalTest(){return critical_mode;};
    bool     isDependent(){return dependency;};
    bool     hasReplacementKey();

    QStringList    getReplacementKey(){ return replKey;};
    QStringList    getReplacementAnsw(){ return replAnsw;};

    QString get_program(){return program;};
    QStringList getArg(){ return args; };
    MSourceChunks getChunks(){return chunks;};// map of KEYS and CODE
                                              //to be replaced during a test
    MSourceChunks replacementMap(){return replMap;};// map of KEYS and CODE to
                                                    //be replaced during a test
    QString getCompiler(){ return compiler;  };
    QString getLanguage(){ return language; };
    QString getAnswVar(){ return answVar; };
    QString getAnswVal(){ return answVal; };
    QString fortranName(){ return fname;  };
    QString exeName();
    QString objName();
    QString taskName(){ return name; };
    QString compilerOptions();
    QString getToolFun(){ return tool_func;};
    QString objects(){return objs;};
    QString libraries(){return libs;};
    QString checkMode(){ return check;};
    QString getAnswKey(){ return answ_key;};
    QString getReference(){ return reference;};
    QString getCurrentArg(){ return currentArg;};

    QStringList getDepName(){return depName;};

    void     createFfile( uint iEle );

    private:

    QString compiler; // compiler to be used
    QString options;  // compiler options
    QString objs;     // object files
    QString libs;     // libraries
    QString language; // programing language (Fortran or C)
    QString fname;    // fortran file name
    QString ename;    // executable name
    QString program;  // fortran source code
    QString name;     // task description
    QStringList args; // arguments to be passed to the tests,
                      // check whether output equals the arguments
    QString tool_func;// Name of a function. Gives extra flexibility in test.
                      // Must return a value.to be compared with the
                      // exit status of the test.
    QString check;    // exitstatus ==> use exit status of test to
                      // compare against a reference
                      // output  ==> use output from test to compare
                      // against a reference
    QString answ_key; // argument ==> use argument passed to executable
                      //              as the answer.
                      // output   ==> use output from test as the answer.
    QString reference;// Used as reference to compare test result or
                      // exit status. Can be any string.
    QString answVar;
    QString answVal;

    QString currentArg;// to be used if setting answer from argument
                       // set to argument of last SUCCESSFUL execution
                       // step.

    QStringList replAnsw;
    QStringList replKey;

    MSourceChunks replMap;

    QStringList depName; // program name this tasks depends on

    bool  result;      // test result

    bool  run_mode;    // false => runs as many steps from a test
                       //          until successful
                       // true => runs all steps from a test

    bool  critical_mode; // false => failure does not stop configuration
                         // true  => failure stops configuration

    bool dependency;     // true => perform test only if previous failed
                         // false=> perform test independently

    bool delete_source;  // true => delete source file after test finished
                         //         (DEFAULT)
                         // false => keep source file after test finished

    MSourceChunks chunks;// map of KEYS and CODE to be replaced during a test

    uint iMissing;
};

/*********************************************************************
 *
 *               **********        XMLTextReader     ***********
 *
 *  Class XMLTestReader: Reads an XML file looking for defined tasks.
 **********************************************************************/
class XMLTestReader{
    public:
    XMLTestReader( const QString& xmlf );
    QString       getProgram( const QDomNode &n );
    MSourceChunks getMap( const QDomElement &e );
    MTest*        getTests( const QString& xml_name );
    int countTags( const QString& xml_file);
    int testsNumber(){return nTests;};

    private:
    QString xmldoc;
    int nTests;
};

//--------------------------------------------------------------
//   Dynamic Shared Object (DSO)
//--------------------------------------------------------------
class EGS_DSO{
  friend class MCompiler;
  public:
       EGS_DSO();
       EGS_DSO(const QString &cpp_name);
       void init();
       QString getDefinitions(){return defines;};
  private:
       QString libpre,
               libext,
               shared,
               fpic,
               obje,
               eout,
               extra,
               extra_link,
               lib_link1,
               defines,flibs,
               link2_prefix,
               link2_suffix,
               shared_bundle,libext_bundle;
       bool    is_osx;
};

/*********************************************************************
 *
 *               **********        MCompiler   ***********
 *
 *  Class MCompiler: Provides information about a compiler such as
 *  output flag, optimization flags, libraries, link flag, its name,
 *  object and executable extensions, etc.
 **********************************************************************/
class MCompiler{

    public:
    MCompiler();
   ~MCompiler();
    //MCompiler(const MCompiler &mc);
    MCompiler(Language l);
    MCompiler(const QString &a_name);
    MCompiler(Language l, const QString &a_name);
    MCompiler(Language l, const QString &a_name, const QString &a_path);

    void init();

    QString name() const { return the_name;};
    QString path() const { return _path;};
    QString options() const { return opt;};
    QString optimization() const { return optimiz;};
    QString debug() const { return deb;};
    QString exeext() const { return eext;};
    QString objext() const { return oext;};
    QString linkflag() const { return lflag;};
    QString libraries() const { return libs;};
    QString outflag() const { return oflag;};
    QString version() const {return _version;};
    QString getVersionFlag() const {return vopt;};
    bool    exists() const {return _exists;};

    QString defines(){
      if (dso) return dso->defines;
      else     return QString();
    };
    QString libpre(){
      if (dso) return dso->libpre;
      else     return QString();
    };
    QString libext(){
      if (dso) return dso->libext;
      else     return QString();
    };
    QString obje(){
      if (dso) return dso->obje;
      else     return oext;
    };
    QString shared(){
      if (dso) return dso->shared;
      else     return QString();
    }
    QString fpic(){
      if (dso) return dso->fpic;
      else     return QString();
    }
    QString flibs(){
      if (dso) return dso->flibs;
      else     return QString();
    }
    QString extra_link(){
      if (dso) return dso->extra_link;
      else     return QString();
    }
    QString extra(){
      if (dso) return dso->extra;
      else     return QString();
    }
    QString dsoPath(){
      if (dso) return dso->lib_link1;
      else     return QString();
    }
    QString LinkPrefix(){
      if (dso) return dso->link2_prefix;
      else     return QString();
    }
    QString LinkSuffix(){
      if (dso) return dso->link2_suffix;
      else     return QString();
    }
    QString bundleOSX(){
      if (dso) return dso->shared_bundle;
      else     return QString();
    }
    QString libextBundleOSX(){
      if (dso) return dso->libext_bundle;
      else     return QString();
    }

    void setLanguage(Language l);
    void setUpCompiler( const QString& a_name );
    void setUpCompiler( Language l, const QString& a_name );
    void setUpFortranCompiler();
    void setUpCCompiler();
    void setUpCPPCompiler();
    void setUpGnuMake();
    void setPath( const QString& p ){ _path = p;};
    void setVersion( const QString& n ){ _version = n;};
    void setOptions( const QString& n ){ opt = n;};
    void setOptimization( const QString& n ){ optimiz = n;};
    void setDebug( const QString& n ){ deb = n;};
    void setExeExt( const QString& n ){ eext = n;};
    void setObjExt( const QString& n ){ oext = n;};
    void setLinkFlag( const QString& n ){ lflag = n;};
    void setLibs( const QString& n ){ libs = n;};
    void setoutflag( const QString& n ){ oflag = n;};

    void setDefines(const QString &s){
      if (dso) dso->defines = s;
    };
    void setShared(const QString &s){
      if (dso) dso->shared = s;
    };
    void setfPIC(const QString &s){
      if (dso) dso->fpic = s;
    };
    void setFlibs(const QString &s){
      if (dso) dso->flibs = s;
    };
    void setExtra(const QString &s){
      if (dso) dso->extra = s;
    };
    void setDSOPath(const QString &s){
      if (dso) dso->lib_link1 = s;
    };
    void setLinkPrefixSuffix(const QString &s){
      QStringList fix = s.split("some_lib");
      if (dso){
        dso->link2_prefix = fix[0];
        dso->link2_suffix = fix[1];
      }
    };

    bool OS_x86_64();
    bool is_x86_64();

    QString getFlibs2LinkCPP( const QString &f_name, const QString &a_path );
    QString getFlibs2LinkCPPFromScript( const QString &f_name, const QString &the_script );

    private:

    EGS_DSO *dso;
    QString getVersion(){
      QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
      QString the_path = environment.value("PATH");
#ifdef WIN32
      environment.insert("PATH",the_path + ";"+ _path);
#else
      environment.insert("PATH",the_path + ":" + _path);
#endif
      QProcess vp; vp.setProcessEnvironment(environment);
      vp.start(the_name,QStringList() << vopt);
      if (!vp.waitForStarted()){ _exists = false; return QString();}
      vp.closeWriteChannel();
      if (!vp.waitForFinished(-1)){ _exists = false; return QString();}
      //QString answer = QString(vp.readAll()); _exists = true;
      _exists = true;
      QString answer = QString(vp.readAllStandardOutput());
              answer += QString(vp.readAllStandardError());
      return answer;
    }

    /* returns location of prog if in path variable */
    QString getPathOf( const QString &prog ){
       QString the_prog = prog;
#ifdef WIN32
       const char* sep = ";";
       if ( !the_prog.endsWith(".exe") ) the_prog += ".exe";
#else
       const char* sep = ":";
#endif
       QString pathval = getenv(QString("PATH").toLatin1());
       if ( !pathval.endsWith(*sep) ) pathval.append(*sep);

       //Extracting paths from environment variable PATH into a list
       QStringList dirs_in_path = pathval.split(*sep);
       //Finding programs from list progs in the directory list
       QStringList progs_found;
       for ( QStringList::Iterator it = dirs_in_path.begin();
                                it != dirs_in_path.end();
                                ++it ) {
           if ( fileExists(*it+QDir::separator()+the_prog) ) return *it;
       }
       return QString();
    }
    bool fileExists( const QString & fname ){
      QFileInfo fi( fname );
      if ( fi.exists() && fi.isFile() ) return true;
      return false;
    }
    QString the_name, _path;
    QString vopt;
    QString opt;    // things like automatic and static variables
    QString optimiz;
    QString deb;
    QString eext; //                                Windows:  exe
    QString oext; //Unix:   o                       Windows:  o, obj
    QString lflag;//Unix:-lsomelib1 -lsomelib2 ...  Windows: -Lib lib1 lib2 ....
    QString libs;
    QString oflag;
    QString _version;
    bool    _exists;
};

/* To use it in a property definition */
//Q_DECLARE_METATYPE(MCompiler)

/*********************************************************************
 *
 *               **********        MTest     ***********
 *
 *  Class MTest: Performs a number of tasks which can be defined in an
 *  xml file or programatically. It compiles the code for a given task
 *  and executes it if needed and keeps the answers to the test.
 **********************************************************************/

class MTest : public QObject{
    Q_OBJECT

    public:
    MTest(){ini(); };
    MTest( QTextEdit*  io_dev, ushort nTests, QIODevice* file   );
    MTest( QTextEdit*  io_dev, const QString& xmlf, const QString& log  );
    MTest( QTextEdit*  io_dev, const QString& xmlf , const QString& tit,
                               const QString& log  );
    MTest( QTextEdit*  io_dev, const QString& xmlf, QIODevice* file );
    MTest( QTextEdit*  io_dev, Tasks* t );
    ~MTest();
    void ini();
/*    void setTest( const ushort& i, const char* f_name,
                  const char* f_prog, const char* f_c,
		  const char* c_options, const char* c_lang,
                  const char* t_ask, const char* a_rgs,
		  const char* c_heck  );*/
    bool successfulCompilation(){return succComp;};
    bool successful_test( ushort i ){ return t[i].get_result();};

    void setCurrentTask();
    void setCompilers ( const QString& fc, const QString& fopt,
                        const QString& cc, const QString& copt );
    void setCompilers ( MCompiler* fcomp, MCompiler* ccomp );

    ushort  getIDepIndex( const QString& nam  ){return id[nam];};
    uint    totalCompilation( Tasks* tT );
    ushort  getCurrentTask(){return currentTask;};
    ushort  getTotalTasks(){return totalTasks;};
    void    setTotalTasks( ushort n ){ totalTasks = n; };
    Tasks*  tasks(){return t;};
    void    setTasks(Tasks* tsks ){t = tsks;};

    void    setTitle( const QString& tit ){title = tit;};
    QString getTitle(){return title;};

    void    setTestLibName( const QString& nam ){ testLib = nam; };
    QString getTestLib(){ return testLib; };

    MTestID getIDs(){return id;};
    void    setIDs() ;

    void setObjDir( const QString& od );

    MTestAnswer &getAnswers(){ return answer;};

    void reset();
    bool needs2Bmade( QStringList nam  );
    void stop();

    void setEndStr( const QString& str ){ endStr = str;};

    void setLinkerOptions( const QString& str ){ linkerOptions = str; };
    //------------------------------------
   // env in the form key=VALUE
    //------------------------------------
    void setEnvironment( const QProcessEnvironment& env ){ copyEnvironment( env ); };
    //void setEnvironment( const QString& env ){ environment.insert( env ); }; Qt4.8 and up only :-(
    //void copyEnvironment( const QStringList& env ){ environment = env; };
    void copyEnvironment( const QProcessEnvironment& env){environment = env;}
    //------------------------------------
    //void set_o_flag( bool need_it ){ needs_minus_o_flag = need_it; };
    void set_o_flag( const QString& oflag ){ o_flag = oflag; };

    public slots:
    void compile();
    void compilationProgress();
    void errDetect(int , QProcess::ExitStatus);
    void execute();
    void executionStatus(int , QProcess::ExitStatus);
    void executionProgress();
    void launch();
    signals:
    void compilationFinished();
    void executionFinished();
    void stepFinished();
    void taskFinished();
    void testsFinished();
    void readyToExecute();
    void criticalError();
    private:
    QTextEdit*  ioDevice;
    QProcess*   cProc;
    QProcess*   exeProc;
    QStringList args;

    QString title;
    QString endStr;
    QString testLib;
    QString exeOut;

    MTestAnswer answer;
    MTestID id;
    MCompiler* fcompiler;
    MCompiler* ccompiler;

    QLibrary* lib;

    QTextStream config_log;
    QFile* config_file;
    QString config_log_name ;
    QString ObjDir;
    QString linkerOptions;
    QString o_flag;

    //QStringList environment;
    QProcessEnvironment environment;

    ushort        totalTasks;
    ushort        currentTask;
    ushort        compileTimes;
    uint           tcompileTimes;
    uint           NEle;
    uint           NKeys;
    ushort        exeTimes;
    Tasks*       t;
    bool          succComp;
    bool          succExe;

    bool  canCreateObj;
    bool  canCreateExe;
    bool  hasExit;
    bool  has_Exit;
    bool  needs_minus_o_flag;

//     void setArgument( QProcess* p, const char* arg);
    QStringList getCompilerArguments( Tasks* tT );
    bool fileExists( const char* rfile, const char** ext_name );
    bool m_exit;

};
#endif

