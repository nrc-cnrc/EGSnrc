/*
###############################################################################
#
#  EGSnrc configuration GUI tools
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
#  Author:          Ernesto Mainegra-Hing, 2003
#
#  Contributors:    Iwan Kawrkaow
#                   Cody Crewson
#                   Frederic Tessier
#                   Reid Townson
#
###############################################################################
*/


#include <QFile>
#include <QDir>

#include "egs_tools.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>  // needed for std::cout
#include <string>
#include <fstream>
#include <qmessagebox.h>
#include <qtimer.h>

#ifndef WIN32
#include <unistd.h>
#else
#include <windows.h>
#include <Winreg.h>
#include <windowsx.h>  //   needed by the shortcut creation function
#include <objbase.h>   //   needed by the shortcut creation function
#include <shlobj.h>    //   needed by the shortcut creation function
#include <shlguid.h>
#include <initguid.h>  //   needed by the shortcut creation function
#endif

#ifdef WIN32
static const char* objext[2]={
    "obj",
    "o"
};
#else
static const char* objext[1]={
    "o"
};
#endif

using namespace std;

typedef int (*SomeFunc)();
typedef char* (*SomeOtherFunc)( const char* );

void delete_file( const QString& target ){
  //qDebug("-> Deleting %s ...",target.toLatin1().data());
     QFile the_file( target );
     if ( the_file.exists() )
        the_file.remove();
/*  if ( the_file.exists() )
      qDebug("failed!");
  else
      qDebug("OK!");*/

}

bool append2file(const char* source, const char* target)
{
  std::ofstream out( target, ios::out | ios::binary | ios::app);
  if (!out) return false;
  char ch;
  while( *source ){
    ch  = *source;
    source++;
    out.put( ch );
  }
  out.close();
  return true;
}

bool copy(const QString& source, const QString& target)
{
     //if file exists, remove it
     delete_file(target);

     std::ifstream   in( source.toLatin1(), ios::in | ios::binary );
     std::ofstream out( target.toLatin1(), ios::out | ios::binary );
     if ( !in ) {
        QMessageBox::critical( 0, "File copy",  QString("Cannot open file: ") + source);
        return false;
     }
     char ch;
     while( 1 ){
        in.get(ch);
        if( in.eof() || !in.good() ) break;
        out.put( ch );
     }
     in.close(); out.close();
     return true;
}

bool fileExists( const QString & fname )
{
  QFileInfo fi( fname );
  if ( fi.exists() && fi.isFile() ) {
    return true;
  }
  return false;
}

bool move_file(const QString& source, const QString& target)
{
if ( copy( source, target) ){
    delete_file( source );
    return true;
}
return false;
}

/*!  Delete files on current directory defined by the given filters
      If you to delete all files ending with either ".cpp" or ".h", you would use "*.cpp;*.h"
*/
void delete_files( const QString& filter ){
  QDir dir( QDir::currentPath() );
  const QFileInfoList filist = dir.entryInfoList( filter.split(";"),  QDir::Files, QDir::DirsFirst|QDir::Name );
  for (int i = 0; i < filist.size(); ++i) {
    QFileInfo fi = filist.at(i);
    if ( fi.isFile() ) delete_file( fi.fileName() );
  }
}

void delete_files( const QString& dir_str, const QString& filter )
{
  QDir dir( dir_str );
  const QFileInfoList filist = dir.entryInfoList( filter.split(";"),  QDir::Files, QDir::DirsFirst|QDir::Name );
  for (int i = 0; i < filist.size(); ++i) {
    QFileInfo fi = filist.at(i);
    if ( fi.isFile() ) {
      //qDebug("-> Deleting %s \n", fi.absoluteFilePath().toLatin1().data());
      delete_file( fi.absoluteFilePath() );
    }
  }
}
/*  Determines Linux OS bitness or Windows OS or process bitness in case of WOW64 (32bit program on 64bit OS) */
bool is_x86_64(){
#ifndef WIN32
  QProcess arch;
  arch.start("getconf",QStringList() << "LONG_BIT");
  if (!arch.waitForStarted()) return false;
  arch.closeWriteChannel();
  if (!arch.waitForFinished()) return false;
  return QString(arch.readAll()).contains("64") ? true : false;
#else
  /* On Windows one can have 32 bit program (processes/software) on 64 bit OS (processor/hardware)*/
  return QString(getenv("PROCESSOR_ARCHITECTURE")).contains("64")? true : false;
#endif
}

/*
    changes the attributes of a file (Unix/Linux)
 */
void chmod( const QString& attrib, const QString& file )
{
QString p  = "chmod " + attrib;
p += " " + file + " &";
if ( system( p.toLatin1().data() ) )
    QMessageBox::warning ( 0, "chmod", "failed on " + file, 1, 0, 0 );
}
/*-------------------------------------------

     WINDOWS API TOOLS

---------------------------------------------*/

/***************************************************************
 * Replaces the value of the environment variable var with value
 ***************************************************************/
bool replaceUserEnvironmentVariable( const QString& var, const QString& value, QString* msg)
{
bool res = false;

#ifdef WIN32
LONG lRes;
HKEY hkSub = NULL;
HKEY hKey  = NULL;
PHKEY phkResult;
ulong neworused;// receives 1 if new key was created or 2 if an existing key was opened
QString subkey =    "Environment";

// Create or open the registry key
lRes = RegCreateKeyExA( HKEY_CURRENT_USER , subkey.toLocal8Bit(), 0, 0, REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS, NULL, &hKey, &neworused);
if( lRes == ERROR_SUCCESS ){
    res = true;
    *msg = (QString)"\n RegCreateKeyEx succeeded \n" ;
    if (neworused == 1){
        *msg +=  (QString)"A new Key was created \n";
    }
    else if (neworused == 2){
        *msg += (QString)"An existing Key was opened \n";
    }
    else {
        *msg +=  (QString)"neworused = " +  QString("%1").arg(neworused,0,10) + (QString)"\n";
    }

    // Write the string to the registry.
    lRes = RegSetValueExA(hKey, (const char*)var.toLocal8Bit(), 0, REG_EXPAND_SZ,
                                 (const uchar*) value.toLatin1().data(), value.length()+1 );
    if( lRes == ERROR_SUCCESS ){
        res = true && res;
        *msg += (QString)"Successful key creation \n";
        *msg += (QString)"Key Value: " +  value + (QString)"\n";
    }
    else{
        res = false;
        *msg += (QString)"Key creation failed! \n";
    }

    RegCloseKey( hKey );

    unsigned long long lpdwResult;
    if ( ! SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
                               (LPARAM) "Environment", SMTO_ABORTIFHUNG,
                               9000, &lpdwResult) )
        *msg += "Message broadcast failed! \n";
    else
        *msg += "Successful message broadcast ! \n";

}
#else
  /* Dummy statement to avoid warnings on Linux*/
  *msg = var + value;
#endif
return res;
}

/***************************************************************
 * PREPENDS value to the environment variable var if not set
 * already. Returns false if already set and warns about multiple
 * settings (>1). Using QString.lower() since Windows directories
 * are case insensitive.
 ***************************************************************/
bool prepend2UserEnvironmentVariable( const QString& var, const QString& value, QString* msg)
{
bool res = false;

#ifdef WIN32
LONG lRes;
HKEY hkSub = NULL;
HKEY hKey  = NULL;
PHKEY phkResult;
ulong neworused;// receives 1 if new key was created or 2 if an existing key was opened
QString subkey = "Environment";

unsigned char* lpData;
DWORD lpcbData = 2048;     // address of data buffer size
lpData = new unsigned char[lpcbData];

// Create or open the registry key
lRes = RegCreateKeyExA( HKEY_CURRENT_USER , subkey.toLocal8Bit(), 0, 0, REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS, NULL, &hKey, &neworused);
if( lRes == ERROR_SUCCESS ){
    res = true;
    *msg = (QString)"\n RegCreateKeyEx succeeded \n" ;
    if (neworused == 1){
        *msg +=  (QString)"A new Key was created \n";
    }
    else if (neworused == 2){
        LONG llres;
        //LPBYTE lpData;       // address of data buffer
        llres = RegQueryValueExA( hKey, var.toLocal8Bit(),0,0, lpData,&lpcbData);
        if (llres == ERROR_SUCCESS){
            *msg += (QString)"An existing Key was opened \n With value: ";
            *msg += (QString)((char*)lpData);
            *msg += (QString)"\n";
        }
        else{
            DWORD err = GetLastError();QString tmp;
            *msg += (QString)"Could not successfully query the registry key \n";
            *msg += (QString)"Error code (GetLastError): " + tmp.setNum(err,10) + (QString)"\n";
            *msg += (QString)"Return value: " + tmp.setNum(llres,10) + (QString)"\n";
            *msg += (QString)"Value of lpcbData: " + tmp.setNum(lpcbData,10) + (QString)"\n";
            *msg += QString("\n -> Perhaps variable does not exist, trying to create it ....\n\n");
            RegCloseKey( hKey );
            return replaceUserEnvironmentVariable( var, value, msg);
            //return false;
        }
    }
    else {
        *msg +=  (QString)"neworused = " +  QString("%1").arg(neworused,0,10) + (QString)"\n";
    }


    // Append the string to the registry key value.

    QString new_value = value + (QString)((char*)lpData);
    QString s;
    //int times = new_value.toLower().contains( value.toLower() );
    int times = new_value.toLower().count( value.toLower() );

    if ( times > 1 ){
        if ( times > 2)
            *msg += (QString)"***** Beware, multiple("
                    + s.setNum(times-1,10)
            + (QString)") entries found in your environment variable  " + var
                    +  (QString)"   *****\n";
        return false;
    }

    lRes = RegSetValueExA(hKey, (const char*)var.toLocal8Bit(), 0,
                          REG_EXPAND_SZ, (const unsigned char*) new_value.toLatin1().data(), new_value.length()+1 );
    if( lRes == ERROR_SUCCESS ){
        res = true && res;
        *msg += (QString)"Successful key creation \n";
        *msg += (QString)"Key Value: " +  new_value + (QString)"\n";
    }
    else{
        *msg += (QString)"Key creation failed! \n";
    }

    RegCloseKey( hKey );

    unsigned long long lpdwResult;
    if ( ! SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
                               (LPARAM) "Environment", SMTO_ABORTIFHUNG,
                               9000, &lpdwResult) )
        *msg += "Message broadcast failed! \n";
    else
        *msg += "Successful message broadcast ! \n";

}
#else
  /* Dummy statement to avoid warnings on Linux*/
  *msg = var + value;
#endif
return res;
}

/*********************************************************************
 * Reads variable from user's environment. Useful for instance to get
 * the PATH just for the user without the system path to avoid a huge
 * PATH when updating the user's environment with EGS paths.
 ********************************************************************/
QString getUserEnvironmentVariable( const QString& var, QString* msg)
{
#ifdef WIN32
LONG lRes;
HKEY hkSub = NULL;
HKEY hKey  = NULL;
PHKEY phkResult;
ulong neworused;// receives 1 if new key was created or 2 if an existing key was opened
QString subkey = "Environment";

unsigned char* lpData;
DWORD lpcbData = 2048;     // address of data buffer size
lpData = new unsigned char[lpcbData];

// Create or open the registry key
lRes = RegCreateKeyExA( HKEY_CURRENT_USER , subkey.toLocal8Bit(), 0, 0, REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS, NULL, &hKey, &neworused);
if( lRes == ERROR_SUCCESS ){
    *msg = (QString)"\n RegCreateKeyEx succeeded \n" ;
    if (neworused == 1){
        *msg +=  (QString)"A new Key was created \n";
    }
    else if (neworused == 2){
        LONG llres;
        //LPBYTE lpData;       // address of data buffer
        llres = RegQueryValueExA( hKey, var.toLocal8Bit(),0,0, lpData,&lpcbData);
        if (llres == ERROR_SUCCESS){
            *msg += (QString)"An existing Key was opened \n With value: ";
            *msg += (QString)((char*)lpData);
            *msg += (QString)"\n";
        }
        else{
            DWORD err = GetLastError();QString tmp;
            *msg += (QString)"Could not successfully query the registry key \n";
            *msg += (QString)"Error code (GetLastError): " + tmp.setNum(err,10) + (QString)"\n";
            *msg += (QString)"Return value: " + tmp.setNum(llres,10) + (QString)"\n";
            *msg += (QString)"Value of lpcbData: " + tmp.setNum(lpcbData,10) + (QString)"\n";
            return QString::null;
        }
    }
    else {
        *msg +=  (QString)"neworused = " +  QString("%1").arg(neworused,0,10) + (QString)"\n";
    }
}
return (QString)((char*)lpData);
#else
  /* Dummy statement to avoid warnings on Linux*/
  *msg = var;
return QString::null;
#endif
}

/*-------------------------------------------------------------------------
   BEWARE : This function will fail when assuming UNICODE characters
   Took me one day to figure that one out. The Win Qt3.2 installation
   adds the -DUNICODE switch by default.
   I should find a generic way of handling any case, perhaps UNICODE ? :-)
----------------------------------------------------------------------------  */
int createShortcut( const char* target, const char* link,
        const char* desc, QString &return_message )
{
#ifdef WIN32
IShellLink *psl;
HRESULT hres;

return_message="";
hres = CoInitialize(NULL);
if (!SUCCEEDED(hres)){
    printf("Could not open the COM library\n");
    return 2;
}

hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                        IID_IShellLink, (LPVOID *)&psl);
if (!SUCCEEDED(hres)){
    printf("Error in CoCreateInstance API function\n");
    return 3;
}

IPersistFile *ppf;
hres = psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);
if (!SUCCEEDED(hres)){
    printf("Error in QueryInterface API function\n");
    return 4;
}

// Set the path to the shortcut target.
hres = psl->SetPath( target );
if (! SUCCEEDED (hres)){
    printf("SetPath failed!\n" );
    return_message = "SetPath failed!\nTarget : ";
    return_message += target;
    return 5;
}

// Set the description of the shortcut.
hres = psl->SetDescription( (LPCTSTR) desc );
if (! SUCCEEDED (hres)){
    printf("SetDescription failed!\n");
    return 6;
}

// Ensure that the string consists of ANSI characters.
WORD wsz[MAX_PATH];
MultiByteToWideChar(CP_ACP, 0, link, -1, wsz, MAX_PATH);

// Save the shortcut via the IPersistFile::Save member function.
hres = ppf->Save ( wsz, TRUE );
if (! SUCCEEDED (hres)){
    printf("Save failed!\n");
    return 7;
}

// Release the pointer to IPersistFile.
ppf->Release();

// Release the pointer to IShellLink.
psl->Release();
#else
  /* Dummy statement to avoid warnings on Linux*/
  return_message = QString(target) + QString("->") + QString(link) + QString(" => ") + QString(desc);
#endif

return 0; // SUCCESS

}

int createShortcut( const char* target, const char* link,
                    const char* desc, const char* icon,
                    int index, QString &return_message )
{
#ifdef WIN32
   IShellLink *psl;
   HRESULT hres;

   return_message="";
   hres = CoInitialize(NULL);
   if (!SUCCEEDED(hres)){
       printf("Could not open the COM library\n");
       return 2;
   }

   hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                           IID_IShellLink, (LPVOID *)&psl);
   if (!SUCCEEDED(hres)){
       printf("Error in CoCreateInstance API function\n");
       return 3;
   }

   IPersistFile *ppf;
   hres = psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);
   if (!SUCCEEDED(hres)){
       printf("Error in QueryInterface API function\n");
       return 4;
   }

   // Set the path to the shortcut target.
   hres = psl->SetPath( target );
   if (! SUCCEEDED (hres)){
       printf("SetPath failed!\n" );
       return_message = "SetPath failed!\nTarget : ";
       return_message += target;
       return 5;
   }

   // Set the description of the shortcut.
   hres = psl->SetDescription( (LPCTSTR) desc );
   if (! SUCCEEDED (hres)){
       printf("SetDescription failed!\n");
       return 6;
   }

   // Sets the location (path and index) of the icon for a Shell link object.
   if (icon){
    hres = psl->SetIconLocation( (LPCTSTR) icon, index );
    if (! SUCCEEDED (hres)){
       printf("SetIconLocation failed!\n");
       return 7;
    }
   }

   // Ensure that the string consists of ANSI characters.
   WORD wsz[MAX_PATH];
   MultiByteToWideChar(CP_ACP, 0, link, -1, wsz, MAX_PATH);

   // Save the shortcut via the IPersistFile::Save member function.
   hres = ppf->Save ( wsz, TRUE );
   if (! SUCCEEDED (hres)){
       printf("Save failed!\n");
       return 8;
   }

   // Release the pointer to IPersistFile.
   ppf->Release();

   // Release the pointer to IShellLink.
   psl->Release();
#else
  /* Dummy statement to avoid warnings on Linux*/
  return_message = QString(target) + QString(icon) + QString("->") +
                   QString(link) + QString(" => ") +
                   QString(desc);
  index = 0;
#endif

   return 0; // SUCCESS

}

/*---------------------------------------------------------------

     TASKS OBJECT IMPLEMENTATION

-----------------------------------------------------------------*/

//Tasks::Tasks( ushort rTasks )
Tasks::Tasks( )
{
    program  = QString();
    fname     = QString();
    name      = "Default test message";
    language = "fortran";
    compiler  = "gfortran";
    options    = QString();
    check      = "";
    tool_func  = QString();
    answVar   = QString();
    answVal   = QString();
    delete_source = true;
    //    replAnsw = -1;
    ename      = "default";
    iMissing    = 0;
    result        = false;
    run_mode  = false;  // false => runs as many steps from a test until successful
                                // true => runs all steps from a test
    critical_mode = false;
    dependency = false;
    check = "exitstatus";
    answ_key = "output";
#ifdef WIN32
    ename += ".exe";
#endif

}

void  Tasks::setFName( const QString& fnam )
{
    if ( !fnam.isEmpty() ){
	fname = fnam;
    }
    else{
	iMissing++;
	if ( language.toLower() == "c" )
	    fname = QString("dummy%1.c").arg(iMissing,0,10);
	else
	    fname = QString("dummy%1.f").arg(iMissing,0,10);
    }
}

QString Tasks::exeName(){
    QString exe_name = fname; exe_name.truncate(exe_name.lastIndexOf("."));
#ifdef WIN32
    exe_name += ".exe";
#endif
    ename = exe_name;
    return ename;
}

QString Tasks::objName(){
    QString obj_name = fname; obj_name.truncate(obj_name.lastIndexOf("."));
#ifdef WIN32
    obj_name += ".obj";
#else
    obj_name += ".o";
#endif
    return obj_name;
}

//void Tasks::createFfile( ushort index )
void Tasks::createFfile( unsigned int iEle )
{
    QString the_code = program;
    if ( program.isEmpty() ) return;
    QString the_key;
    MSourceChunks::Iterator itsrc;
    for ( itsrc = chunks.begin();  itsrc != chunks.end();  ++itsrc ) {
	the_key = itsrc.key();
	unsigned int counter = 0;
	for (QStringList::Iterator it = chunks[ the_key ].begin(); it != chunks[ the_key ].end(); it++ ){
	    if (counter == iEle ){
		the_code.replace( the_key, *it );
		break;
	    }
	    else{ counter++; }
	}
    }


    QFile f_file( fname );
    if (!f_file.open( QIODevice::WriteOnly  ) ){
	cout << "Error creating file" << fname.toLatin1().data() << endl;
	return;
    }
    QTextStream to_disk( &f_file );
    to_disk << the_code;
    f_file.flush();
    f_file.close();

}

QString Tasks::compilerOptions(){
    return options;
}

QString libPath(){
    QString the_path = QDir::currentPath();
    the_path += "/lib/";
    QString OS;
#ifdef WIN32
    OS = getenv("OS");
#else
    OS = "Linux";
#endif
    the_path += OS;
    return the_path;
}

bool Tasks::hasReplacementKey(){
//    if ( replAnsw.size() > 0 ) return true;
    if ( ! replMap.empty() ) return true;
    else                     return false;
}

/*---------------------------------------------------------------
     COMPILER OBJECT IMPLEMENTATION
     (Defaults to GNU Fortran Compiler)
-----------------------------------------------------------------*/

// Initialize DSO with defaults

void EGS_DSO::init(){
    is_osx=false;
    libpre="lib";
    libext=".so";
    shared="-shared";
    fpic=is_x86_64() ? "-fPIC" : QString();
    obje="o";
    eout="-o ";
    extra="-o $@";
    extra_link="";
    lib_link1="-L$(abs_dso) -Wl,-rpath,$(abs_dso)";
    defines="";
    link2_prefix="-l";
    link2_suffix="";
    shared_bundle="";
    libext_bundle="";
    flibs = "";
}

EGS_DSO::EGS_DSO(){
  init();
}

EGS_DSO::EGS_DSO(const QString &cpp_name){

   init();

#if defined(WIN32) || defined(Q_OS_WIN32)
   libpre=QString(); libext=".dll";
   defines="-DWIN32";
   if (cpp_name.contains("g++")){
      defines="-DWIN32";
      lib_link1="-L$(abs_dso)";
      extra="-o $@ -Wl,--out-implib,$(@:.dll=.lib)";
   }
   else if ( cpp_name.contains("cl") ){
      defines="-DWIN32 -DMSVC"; obje="obj";
      extra="-link -DLL -implib:$(@:.dll=.lib) -out:$@";
      eout="-Fe"; lib_link1=QString();
      link2_prefix="$(ABS_DSO)"; link2_suffix=".lib";
   }
#elif defined(Q_OS_CYGWIN)
   libpre=QString(); libext=".dll";
   if (cpp_name.contains("g++")){
      defines="-DCYGWIN";
      extra="-o $@ -Wl,--out-implib,$(@:.dll=.lib)";
   }
   else if ( cpp_name.contains("cl") ){
      defines="-DWIN32 -DMSVC"; obje="obj";
      extra="-link -DLL -implib:$(@:.dll=.lib) -out:$@";
      eout="-Fe"; lib_link1=QString();
      link2_prefix="$(ABS_DSO)"; link2_suffix=".lib";
   }
#elif defined(Q_OS_MAC) || defined(Q_OS_DARWIN)
   is_osx=true;
   libext=".dylib"; libext_bundle=".so";
   defines="-DOSX"; lib_link1="-L$(abs_dso)";
   shared="-dynamiclib";
   if ( cpp_name == "g++") shared_bundle="-bundle" ;
   else                    shared_bundle="-qmkshrobj";
#elif defined(Q_OS_SOLARIS)
   lib_link1="-L$(abs_dso) -Wl,-R,$(abs_dso)";
   if ( cpp_name == "g++") fpic="-fPIC";
   else                    fpic="-KPIC"; shared="-G";
#elif defined(Q_OS_AIX)
   shared="-G -Wl,-brtl";
   lib_link1="-L$(abs_dso) -Wl,-brtl,-bexpall";
#elif defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
   fpic = is_x86_64() ? "-fPIC" : QString();
#endif
   flibs = cpp_name.contains("g++") ? "-lgfortran -lquadmath" : QString();
}

// Initialize default GCC compiler as gfortran
MCompiler::MCompiler(){
  init();
}

// Initialize GCC compiler for language l
MCompiler::MCompiler(Language l){
    init();
    setLanguage(l);
}

// Initialize default GCC compiler as gfortran
void MCompiler::init(){
    dso = 0;
    the_name = "gfortran"; vopt = "--version";
    opt = is_x86_64() ? "-fPIC" : QString(); // Only for 64-bit GNU compilers
    optimiz = "-O2 -mtune=native -mcmodel=medium";
    deb = "-g";
    eext = QString();
    oflag = "-o ";
#ifdef WIN32
    oext = "obj";
#else
    oext = "o";
#endif
    lflag = QString(); libs = QString();
    _exists = true;// determined below in method getVersion

    the_hen = QString(); // nothing known about this initially
                         // set later on the location page
}

// Set options for GCC compilers
void MCompiler::setLanguage(Language l){
    QString majorVersion;
    switch(l){
      case F:
        the_name = "gfortran";
        optimiz = "-O2 -mtune=native -mcmodel=medium";
        break;
      case C:
        the_name = "gcc";
        optimiz = "-O2";
        break;
      case CPP:
        the_name = "g++";

        // Use c++14 for gcc 5 or higher
        vopt = "-dumpversion";
        majorVersion = getVersion();
        majorVersion = majorVersion.split("\n").takeFirst().split(".").takeFirst();
        vopt = "--version";
        if(majorVersion.toInt() >= 5) {
          optimiz  = "-O2 -mtune=native -mcmodel=medium -std=c++14";
        } else {
          optimiz  = "-O2 -mtune=native -mcmodel=medium -std=c++11";
        }

        dso = new EGS_DSO(name());// Creates dso, sets flibs to -lgfortran literally
        dso->flibs = getFlibs2LinkCPP("gfortran",path());
        break;
      case GnuMake:
        the_name = "make";
        break;
    }
    _version = getVersion(); _version = _version.split("\n").takeFirst();
    _path = getPathOf(the_name);
}

MCompiler::~MCompiler(){}

// Create compiler "a_name" for language "l" (location should be set in PATH)
MCompiler::MCompiler(Language l, const QString &a_name)
          : dso(0)
{
     _path = getPathOf(a_name);
     setUpCompiler( l, a_name );
}

// Create C++ compiler "cpp_name" (location should be set in PATH)
MCompiler::MCompiler(const QString &cpp_name,
                     const QString &f_name,
                     const QString &le_hen)
          : dso(0)
{
     _path = getPathOf(cpp_name);
     setTheHen(le_hen);
     setUpCompiler( CPP, cpp_name, f_name );
}

// Create compiler "a_name" for language "l" located in a_path
// (not necessarily in the PATH) as obtained from the user using a file dialog.
// a_path should be added to the PATH at the end of the installation
MCompiler::MCompiler(Language l, const QString &a_name, const QString &a_path)
          : dso(0), _path(a_path)
{
     setUpCompiler( l, a_name );
}

// Create Fortran compiler "a_name" (location should be set in PATH)
MCompiler::MCompiler(const QString &a_name)
          : dso(0)
{
     _path = getPathOf(a_name);
    setUpCompiler(F,a_name);
}

// Sets up Fortran compiler "a_name"
void MCompiler::setUpCompiler( const QString& a_name ){
    setUpCompiler(F, a_name);
}

// Sets up compiler "n" for language "l"
void MCompiler::setUpCompiler( Language l, const QString& a_name,
                                           const QString& link_to_name )
{
     the_name = a_name;
    switch(l){
      case F:
        setUpFortranCompiler();
        break;
      case C:
        setUpCCompiler();
        break;
      case CPP:
        setUpCPPCompiler(link_to_name);
        break;
      case GnuMake:
        setUpGnuMake();
        break;
    }
}

void MCompiler::setUpGnuMake(){
    vopt = "-v";
    _exists = true;// determined below in method getVersion
    _version = getVersion(); _version = _version.split("\n").takeFirst();
}
void MCompiler::setUpCCompiler(){
    optimiz  = "-O2"; oflag = "-o "; vopt = "--version";
    opt = is_x86_64() ? "-fPIC" : QString(); // Only for 64-bit GNU compilers

#ifdef WIN32
  if (the_name.toLower()== "icc"){
    optimiz  = "-O2 -no-prec-div -fp-model fast=2 -DWIN32";
  }
  else if ( the_name.contains("gcc") ){
    optimiz  = "-O2 -DWIN32";
  }
  else if ( the_name.toLower() == "cl.exe" ){//-Ox max. optimizations
    optimiz = "-Ox -DWIN32 -MD -nologo";//-MD link with MSVCRT.LIB
    oflag   = "-Fo";                    //-Fo<name> obj file name
    vopt = QString();
  }
  else{
    optimiz  = "-O2 -DWIN32";
  }
#else
  if (the_name.toLower()== "icc"){
    optimiz  = "-O2 -no-prec-div -fp-model fast=2";
  }
  else if ( the_name.contains("gcc") ){
    optimiz  = "-O2 -mtune=native -mcmodel=medium";
  }
  else{
    optimiz  = "-O2";
  }
#endif
  _version = getVersion(); _version = _version.split("\n").takeFirst();
}

void MCompiler::setUpCPPCompiler(const QString& link_to_name){
    optimiz  = "-O2"; oflag = "-o "; vopt = "--version";
#ifdef WIN32
  if ( the_name.toLower() == "cl.exe" ){//-Ox max. optimizations
    optimiz = "-Ox -DWIN32 -MD -nologo";//-MD link with MSVCRT.LIB
    oflag   = "-Fo";                    //-Fo<name> obj file name
    vopt = QString();
  }
  else if ( the_name.contains("g++") ){
    // Use c++14 for gcc 5 or higher
    vopt = "-dumpversion";
    QString majorVersion = getVersion();
    majorVersion = majorVersion.split("\n").takeFirst().split(".").takeFirst();
    vopt = "--version";
    if(majorVersion.toInt() >= 5) {
      optimiz  = "-O2 -mtune=native -mcmodel=medium -std=c++14 -DWIN32";
    } else {
      optimiz  = "-O2 -mtune=native -mcmodel=medium -std=c++11 -DWIN32";
    }
  }
  else if (the_name.toLower()== "icpc"){
    optimiz  = "-O2 -no-prec-div -fp-model fast=2 -DWIN32";
  }
#else
  if ( the_name.contains("g++") ){
    // Use c++14 for gcc 5 or higher
    vopt = "-dumpversion";
    QString majorVersion = getVersion();
    majorVersion = majorVersion.split("\n").takeFirst().split(".").takeFirst();
    vopt = "--version";
    if(majorVersion.toInt() >= 5) {
      optimiz  = "-O2 -mtune=native -mcmodel=medium -std=c++14";
    } else {
      optimiz  = "-O2 -mtune=native -mcmodel=medium -std=c++11";
    }
  }
  else if (the_name.toLower()== "icpc"){
    optimiz  = "-O2 -no-prec-div -fp-model fast=2";
  }
  else{
    optimiz  = "-O2";
  }
#endif

  _version = getVersion(); _version = _version.split("\n").takeFirst();

  //get_dso();
  dso = new EGS_DSO(name());// Creates dso, sets flibs to -lgfortran literally
  /* Initially set for gfortran, on Windows set to "libgfortran.a" */
  dso->flibs = getFlibs2LinkCPP(link_to_name,path());

}

/******************************************************
 * finding Fortran libraries when linking done in C++ */
/******************************************************/
QString MCompiler::getFlibs2LinkCPP( const QString &f_name, const QString &a_path )
{
  QString flibs = QString(),
          path = a_path;
#ifdef WIN32
    if ( f_name.contains("g77") && name().contains("g++"))
        flibs= QString("-lg2c");
    else if ( f_name.contains("gfortran") && name().contains("g++"))
        flibs= QString("-lgfortran -lquadmath");
    else if ( f_name.contains("ifort") && name().contains("icpc"))
        flibs= QString("-lifport -lifcore");
    else{// using any other C++/Fortran compiler combination
        if (!path.isEmpty()) path = path.replace( "bin", "lib" );
        if ( f_name.contains("gfortran") )// MS C++ compiler
           flibs = path + QString("libgfortran.a");
        else
           flibs = path + QString("libg2c.a");
    }
#else
  // Setting an initial guess
  if      ( f_name.contains("gfortran") && name().contains("g++") )
          flibs="-lgfortran -lquadmath";
  else if ( f_name.contains("g77") )
          flibs="-lg2c";
  else if ( f_name.contains("ifort") && name().contains("icpc"))
          flibs= QString("-lifport -lifcore");
  else if (!getTheHen().isEmpty()){
          flibs= getFlibs2LinkCPPFromScript(f_name,
                 getTheHen() + QString("scripts/get_f77_libs1"));
  }

#endif
  return flibs;
}

/******************************************************
 * finding Fortran libraries when linking done in C++ *
 * This version is meant to use script get_f77_libs1  *
 * in the HEN_HOUSE/scripts folder.                   *
 ******************************************************/
QString MCompiler::getFlibs2LinkCPPFromScript( const QString &f_name, const QString &the_script )
{
      QString answer = QString();
#ifndef WIN32
      QProcess vp;
      vp.start(the_script, QStringList() << f_name << name() );
      if (!vp.waitForStarted()) return QString("FailedToStart!");
      vp.closeWriteChannel();
      if (!vp.waitForFinished(-1)){ _exists = false; return QString("FailedToFinish");}
      answer = QString(vp.readAll());
#endif
      return answer.remove("fortran_libs = ");
}
void MCompiler::setUpFortranCompiler(){
    // Setting some defaults
    oflag = "-o "; opt = QString();
    oext = "o"; lflag = "-l"; vopt = QString(); _version = "DUNO!";
    eext = QString(); libs = QString(); deb = "-g"; optimiz = "-O2";
    _exists = true;// determined below in method getVersion

#if defined(WIN32) || defined(Q_OS_WIN32)
    oext = "obj";
    if ( the_name.contains("gfortran") || // GNU Fortran
         the_name.contains("g77")      ){
        vopt = "--version";
        opt = is_x86_64() ? "-fPIC" : QString(); // Only for 64-bit GNU compilers
        optimiz = "-O2 -mtune=native -mcmodel=medium";
    }
    else if (the_name == "ifl"){
        vopt = "-V";
        opt = "/W0 /4Yportlib /Qfpp";
        optimiz = "/Ox";
        deb = "/debug";
    }
    else if (the_name == "ifort"){
        vopt = "/logo";
        opt = "-fpp";
        optimiz = "-O2";
        deb = "-debug";
    }
    else if (the_name == "lf95" || the_name == "f77"){
        opt = "-nco -nw";
        optimiz = "-tp";
        deb = "-g";
        oflag = "-out "; // BEWARE OF SPACE AT END
        lflag = "-lib";
    }
    else if (the_name == "df"){
        vopt = "-v";
        opt = "-fpp";
        optimiz = "-fast";
        deb = "-g";
        oflag = "-exe:"; // MUST HAVE NO SPACE AT END
        lflag = "-libs";
    }
    else if (the_name == "fl32"){
        opt = "/W0";
        optimiz = "/Ox /4Ya";
        deb = "/debug";
        oflag = "/Fe";    // <=== MUST HAVE NO SPACE AT END
        lflag = QString();// no need to pass a flag: fl32 [fortran files] [obj files] [library files]
    }
    else if (the_name == "pgf77" || the_name == "pgf90" || the_name == "pgfortran"){
        opt = QString();
        optimiz = "-fast";
        deb = "-g";
    }
    _version = getVersion(); _version = _version.split("\r\n").takeFirst();
#elif defined(Q_OS_MAC) || defined(Q_OS_DARWIN)
    if ( the_name.contains("gfortran") || the_name == "g95" || the_name.contains("g77") ){ // GNU Fortran
        vopt = "-v --version";
        opt = is_x86_64() ? "-fPIC" : QString(); // Only for 64-bit GNU compilers
        optimiz = "-O2 -mtune=native -mcmodel=medium";
        deb = "-g";
    }
    _version = getVersion(); _version = _version.split("\n").takeFirst();
#elif defined(Q_OS_SGI)
    // Rest uses defaults
    vopt = "-version";
    deb = "-g -C";
    _version = getVersion(); _version = _version.split("\n").takeFirst();
#elif defined(Q_OS_HPUX)
    opt = "+E1 +E4 +U77 -K";
    optimiz = "+O4";
    deb = "-g -C";
    _version = "HP-UX Fortran";
#elif defined(Q_OS_SOLARIS)
    // Rest uses defaults
    vopt = "-V";
    deb = "-g -C";
    _version = getVersion(); _version = _version.split("\n").takeFirst();
#elif defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    if ( the_name.contains("gfortran") || the_name == "g95" || the_name.contains("g77")){ // GNU Fortran
        vopt = "--version"; optimiz = "-O2 -mtune=native -mcmodel=medium"; deb = "-g";
        opt = is_x86_64() ? "-fPIC" : QString(); // Only for 64-bit GNU compilers
    }
    else if (the_name == "ifort"){
        vopt = "-logo"; optimiz = "-O2"; deb = "-g -CB";
        opt = is_x86_64() ? "-fPIC" : QString(); // Only for 64-bit GNU compilers;
    }
    else if ( the_name.contains("pgf") ){
        optimiz = "-fast";
        deb = "-g -C";
    }
    else{ // Use some typical values
        vopt = "-v -V -version";// Trick trying to get one of the options to produce version info
        opt  = is_x86_64() ? "-fPIC" : QString(); // Only for 64-bit GNU compilers
        optimiz = "-O2"; deb = "-g";
    }
    _version = getVersion(); _version = _version.split("\n").takeFirst();
#endif

}

/*  Determines OS bitness. Accounts for 32-bit process on 64-bit OS on Windows (WOW64) */
bool MCompiler::OS_x86_64(){
#ifndef WIN32
  QProcess arch;
  arch.start("getconf",QStringList() << "LONG_BIT");
  if (!arch.waitForStarted()) return false;
  arch.closeWriteChannel();
  if (!arch.waitForFinished()) return false;

  return QString(arch.readAll()).contains("64") ? true : false;
#else
  /* On Windows one can have 32 bit program (processes/software) on 64 bit OS (processor/hardware)
   *
                             32bit Native  64bit Native    WOW64
    PROCESSOR_ARCHITECTURE     x86          AMD64         x86
    PROCESSOR_ARCHITEW6432  undefined     undefined      AMD64

  */
  bool archiSoft = QString(getenv("PROCESSOR_ARCHITECTURE")).contains("64")? true : false;// software
  bool archiOS   = QString(getenv("PROCESSOR_ARCHITEW6432")).contains("64")? true : false;// OS/hardware
  return (archiOS || archiSoft);
#endif
}

/*  Determines Linux OS bitness or Windows OS or process bitness in case of WOW64 (32bit program on 64bit OS) */
bool MCompiler::is_x86_64(){
#ifndef WIN32
  QProcess arch;
  arch.start("getconf",QStringList() << "LONG_BIT");
  if (!arch.waitForStarted()) return false;
  arch.closeWriteChannel();
  if (!arch.waitForFinished()) return false;
  return QString(arch.readAll()).contains("64") ? true : false;
#else
  /* On Windows one can have 32 bit program (processes/software) on 64 bit OS (processor/hardware)*/
  return QString(getenv("PROCESSOR_ARCHITECTURE")).contains("64")? true : false;
#endif
}

/*---------------------------------------------------------------

     FORTRAN TEST OBJECT IMPLEMENTATION

-----------------------------------------------------------------*/

MTest::MTest( QTextEdit*  io_dev, const QString& xmlf, QIODevice* file ){
    config_log_name = QString();
    config_log.setDevice( file );
    ini();
    ioDevice  = io_dev;

    if  ( ! QFile::exists(xmlf) ){
        QString msg = "\n Test file " + xmlf + " does not exist !";
        ioDevice->insertPlainText( msg );
        config_log << "\n Test file " << xmlf << " does not exist ! \n";
        return;
    }

    XMLTestReader* xmlr = new XMLTestReader( xmlf );
    MTest* test                = xmlr->getTests( xmlf );
    t                               = test->tasks();
    title                           = test->getTitle();
    testLib                       = test->getTestLib();
    totalTasks                  = test->getTotalTasks();

    if ( totalTasks > 0 ) setIDs();

    if ( !testLib.isEmpty() ) lib = new QLibrary(testLib);

}

void MTest::setIDs(){
    QString answ;
    for (ushort iTask = 0; iTask < totalTasks; iTask++){
        answ = t[iTask].getAnswVar();
        if ( ! answ.isEmpty() ){
           answer[ answ.toLatin1().data() ] = "**********    UNKNOWN    **********";
           id[ answ ]         = iTask;// QMap relating name to task number
        }                             // use Answer key
       else{
          if ( t[iTask].getLanguage().toLower() == "c"){               // QMap relating name to task number
            id[t[iTask].fortranName().remove((QString)".c")] = iTask;// Use test program name without extension
          }
          else{
            id[t[iTask].fortranName().remove((QString)".f")] = iTask;
          }
       }
// cout << "Task name = " << t[iTask].taskName().toLatin1().data() << " has " <<  t[iTask].getArg().count() << " arguments ..." << endl; EMH_DEBUG
     }

}

MTest::MTest( QTextEdit*  io_dev, const QString& xmlf, const QString& log ){
    config_log_name = log;
    ini();
    ioDevice                    = io_dev;

    XMLTestReader* xmlr = new XMLTestReader( xmlf );
    MTest* test                = xmlr->getTests( xmlf );
    t                               = test->tasks();
    title                           = test->getTitle();
    testLib                       = test->getTestLib();
    totalTasks                  = test->getTotalTasks();
    if ( totalTasks > 0 ) setIDs();

    if ( !testLib.isEmpty() ) lib = new QLibrary(testLib);
}

MTest::MTest( QTextEdit*  io_dev, const QString& xmlf , const QString& tit, const QString& log  ){
    config_log_name = log;
    ini();
    ioDevice                    = io_dev;

    title = tit;
    XMLTestReader* xmlr = new XMLTestReader( xmlf );

    MTest* test                = xmlr->getTests( xmlf );
    t                               = test->tasks();
    testLib                       = test->getTestLib();
    totalTasks                  = test->getTotalTasks();
    if ( totalTasks > 0 ) setIDs();
    if ( !testLib.isEmpty() ) lib = new QLibrary(testLib);

}

/*
To be used in conjunction with setTest
*/

MTest::MTest( QTextEdit*  io_dev, ushort nTests, QIODevice* file ){
 config_log_name = QString();
 config_log.setDevice( file );
 ini();
 t = new Tasks[nTests];// <=== create space for nTests tasks, needs to be set
 totalTasks = nTests;
 ioDevice  = io_dev;
 if ( totalTasks > 0 ) setIDs();
}

MTest::MTest( QTextEdit*  io_dev, Tasks* rtask ){
    ini();
    t = rtask;
    ioDevice       = io_dev;
}

void MTest::ini(){

    totalTasks    = 0;
    currentTask   = 0;
    succComp      = false;
    succExe       = false;
    canCreateObj  = false;
    ObjDir        = QString();
    linkerOptions = QString();
    o_flag        = QString();
    needs_minus_o_flag = false;

    NEle = 0;
    compileTimes = 0;
    m_exit = false;

    fcompiler = new MCompiler(F);
    ccompiler = new MCompiler(C);

    title = " ===> Performing tests .....";
    endStr = (QString) "\n\n************              Tests concluded !          *******\n";
    testLib = QString(); // defaults to nothing

   if (!config_log_name.isEmpty()){
       config_file = new QFile( config_log_name );
       if (!config_file->open( QIODevice::WriteOnly  ) ){
            perror("Error was ");
            qFatal("Error creating file config.log");
        }
        config_log.setDevice ( config_file );
    }

    //connect( this, SIGNAL(taskFinished()), this, SLOT(compile()) );
    connect( this, SIGNAL(taskFinished()), this, SLOT(launch()) );
    connect( this, SIGNAL(compilationFinished()), this, SLOT(compile()) );
    connect( this, SIGNAL(stepFinished()), this, SLOT(compile()) );
    connect( this, SIGNAL(readyToExecute()), this, SLOT(execute()) );
    connect( this, SIGNAL(executionFinished()), this, SLOT(execute()) );

    cProc     = new QProcess(this);
/*    cProc->setCommunication(QProcess::Stdout | QProcess::Stderr |
                            QProcess::DupStderr);*/
    cProc->setProcessChannelMode(QProcess::MergedChannels);
    connect( cProc, SIGNAL(readyReadStandardOutput()),
             this, SLOT(compilationProgress()) );
    connect( cProc, SIGNAL(finished(int , QProcess::ExitStatus)),
             this, SLOT(errDetect(int , QProcess::ExitStatus)) );

    exeProc     = new QProcess(this);
/*    exeProc->setCommunication(QProcess::Stdout | QProcess::Stderr |
                              QProcess::DupStderr);*/
    exeProc->setProcessChannelMode(QProcess::MergedChannels);
    connect( exeProc, SIGNAL(readyReadStandardOutput()),
             this, SLOT(executionProgress()) );
    connect( exeProc, SIGNAL(finished(int , QProcess::ExitStatus)),
             this, SLOT(executionStatus(int , QProcess::ExitStatus)) );
}

MTest::~MTest(){
    if ( cProc ) delete cProc;
    if ( exeProc ) delete exeProc;
    if ( t ) delete t;
    if ( lib ) delete lib;
}

void MTest::setCompilers ( const QString& fc, const QString& fopt,
                           const QString& cc, const QString& copt ){

    //fcompiler->setName( fc );
    fcompiler->setUpCompiler( F, fc );
    fcompiler->setOptions( fopt );

    //ccompiler->setName( cc );
    ccompiler->setUpCompiler( C, cc );
    ccompiler->setOptions( copt );
}

void MTest::setCompilers ( MCompiler* fcomp, MCompiler* ccomp ){
    fcompiler = fcomp;
    ccompiler = ccomp;
}

bool  MTest::needs2Bmade( QStringList nam  ){
    bool succ = false;

    for (QStringList::Iterator it = nam.begin(); it != nam.end(); ++it) {
	succ = succ || t[ id[*it]  ].get_result();
    }

    return ! succ;
}



void MTest::launch(){

 if ( m_exit ){
  ioDevice->insertPlainText( "\n\n**** TESTS CANCELLED BY USER ****\n\n");
   config_log << "\n\n**** TESTS CANCELLED BY USER ****\n\n" << endl;
   config_log.flush();
   return;
 }

 QString lineStr = tr("\n--------------------------------------------")+
                   tr("--------------------------------------------------\n");
 QString lineBrk =(QString)"\n";
 if ( currentTask == 0 ){
    ioDevice->insertPlainText( title );
    ioDevice->insertPlainText( lineBrk );
    config_log << title << endl;
    config_log.flush();
 }

 if ( currentTask < totalTasks )  {
   compileTimes = 0;
   tcompileTimes = totalCompilation( &t[currentTask] );
   exeOut = QString();
   if ( tcompileTimes == 0 ) tcompileTimes = 1;
   if ( t[currentTask].isDependent() && ! id.empty()){
    if ( ! needs2Bmade( t[currentTask].getDepName() ) ){
       setCurrentTask(); // equivalent to currentTask++;
       emit taskFinished();
       return;
    }
   }
   compile();
 }
 else{
  if ( ! endStr.isEmpty() ){
    ioDevice->insertPlainText( endStr );
    config_log << endStr << endl;;
    config_log.flush();
   }

   emit testsFinished();
 }
}

unsigned int MTest::totalCompilation( Tasks* tT ){
    MSourceChunks s = tT->getChunks();
    QString the_key;
    unsigned int n_ele   =  0;
    NKeys = 0;
    NEle   =0;
    for ( MSourceChunks::Iterator it = s.begin();  it != s.end();  ++it ) {
        NKeys++;
        the_key = it.key();
        n_ele = s[the_key].count();
        if ( n_ele > NEle ) NEle = n_ele; // take maximum number of elements
    }

    return NEle;
}

void MTest::setCurrentTask(){
    currentTask++;
}

// void MTest::setArgument( QProcess* p, const char* arg){
//     if ( ! ((QString) arg).isEmpty() )
// 	p->addArgument( arg );
// }

void MTest::setObjDir( const QString& od ){
     ObjDir = ( od.endsWith(QString("%1").arg(QDir::separator()))) ?
                od:od+QDir::separator();
};


QStringList setObjectFileExtensions( const QString& objnames,
                                     const QString& objdir ){

    QStringList the_objects = objnames.split(" ", QString::SkipEmptyParts);
    QStringList the_real_objs;


    for ( QStringList::Iterator ito = the_objects.begin();
          ito != the_objects.end(); ++ito ) {

// Object file names passed with extension
	if ( QFile::exists( objdir + *ito) &&
           ( (*ito).endsWith(QString(".o"),Qt::CaseSensitive)||
             (*ito).endsWith(QString(".obj"), Qt::CaseSensitive) ) ){
	       the_real_objs << objdir + *ito;
	}
// Object file names passed without extension
#ifdef WIN32
	else if ( QFile::exists( objdir + *ito + QString(".obj") ) ){
	    the_real_objs <<  objdir + *ito + QString(".obj") ;
	}
#endif
	else if ( QFile::exists( objdir + *ito + QString(".o") ) ){
	    the_real_objs <<  objdir + *ito + QString(".o") ;
	}
    }

    return the_real_objs;

}

QStringList MTest::getCompilerArguments( Tasks* tT ){
    QStringList the_args;
    QString GUIoptions;
    MCompiler* compiler = new MCompiler();
    QString option;
#ifdef WIN32
    bool isGNU = false;
#endif
    if ( tT->getLanguage().toLower() == "c"){
	the_args.append( ccompiler->name() );
	GUIoptions = ccompiler->options();
	compiler = ccompiler;
    }
    else{                              // assume by default Fortran tests
	the_args.append( fcompiler->name() );
	GUIoptions = fcompiler->options();
	compiler = fcompiler;
    }

    if (compiler->name().isEmpty()){//no compiler, skip test
     config_log << "\nBEWARE:\n-------\n -> empty " <<
                  tT->getLanguage().toUpper() << " compiler ..." <<
                  "\n Compilation process will fail!!!"<< endl;
    }

    QString task_option = tT->compilerOptions();
    if ( !task_option.isEmpty() )
	option = task_option;
    else
	option = GUIoptions;

#ifdef WIN32
    o_flag = compiler->outflag();//taken from compiler

    if ( o_flag == "-o " || o_flag == "-o" ){ //up till now: GNU and PGI
     QString the_o_flag = QString(" -o ");
     if ( option.indexOf( "-c") < 0 ){
         option += the_o_flag + tT->exeName();
     }
     else{
         option += the_o_flag + tT->objName();
     }
    }
#else
    if ( option.indexOf( "-c") < 0 ){
       option += QString(" -o ") + tT->exeName();
    }
#endif

    QStringList the_options = option.split(" ", QString::SkipEmptyParts );
    for ( QStringList::Iterator it = the_options.begin();
                                it != the_options.end(); ++it ) {
        the_args << *it;
    }

    the_args.append( tT->fortranName() );

    QStringList the_objects = setObjectFileExtensions( tT->objects(), ObjDir );
    for ( QStringList::Iterator ito = the_objects.begin();
                                ito != the_objects.end(); ++ito ){
	          the_args << *ito;
    }


    QStringList the_libs = tT->libraries().split( " ", QString::SkipEmptyParts);
    for ( QStringList::Iterator itl = the_libs.begin();
                                itl != the_libs.end(); ++itl ){
#ifdef WIN32
          if ( isGNU ){
             the_args << compiler->linkflag() + *itl;
          }
          else{
             the_args << compiler->linkflag();
	     if ( (*itl).indexOf(".lib", true) < 0 )
	          the_args << *itl + QString(".lib");
	     else
	          the_args << *itl;
          }
#else
          the_args << compiler->linkflag() + *itl;
#endif
    }

    if ( ! linkerOptions.isEmpty() ){
       QStringList linker_options = linkerOptions.split( " ", QString::SkipEmptyParts);
       for ( QStringList::Iterator lo = linker_options.begin();
                                   lo != linker_options.end(); ++lo )
            the_args << *lo;
    }

    return the_args;
}

void MTest::compile(){

 if ( compileTimes < tcompileTimes ){

  //****************************************************************************
  //   Checking whether KEYS in the test code are to be replaced by the results
  //   from another test.
  //   The connection is established through the QMap answer key and the QMap
  //   code chunks key to avoid dependence on the position of the test.
  //   Originally, answer[key] is set to a dummy argument NADA.
  //****************************************************************************
  if ( t[ currentTask ].hasReplacementKey() ){
    MSourceChunks key_map( t[ currentTask ].getChunks() );
    MSourceChunks repl_map( t[ currentTask ].replacementMap() );
    for ( MSourceChunks::Iterator itsrc = repl_map.begin();
          itsrc != repl_map.end(); ++itsrc){
      QStringList strList = itsrc.value();
      QStringList replList;
      unsigned int index = 0;
      for ( QStringList::Iterator strit = strList.begin();
            strit != strList.end(); ++strit ){
       replList.append( answer[ strList[ index ].toLatin1().data() ] );
       index++ ;
      }
      key_map[ itsrc.key() ] = replList;
    }
    t[currentTask].setChunks( key_map );
  }



 //****************************************************************************

  t[ currentTask ].createFfile( compileTimes );
  QStringList the_args = getCompilerArguments( &t[ currentTask ]  );
  QString command = the_args.takeFirst();
// cout << "Executing " << command.toLatin1().data() << " with " << the_args.count() << " args: " << the_args.join(" : ").toLatin1().data() << endl;//EMH_DEBUG
  succExe = true;
  if ( environment.isEmpty() ){
     cProc->start(command,the_args);
     if(cProc->error()==QProcess::FailedToStart){
        QString errorExe = QString("\n Could not start ") +
                           command + " " + the_args.join(" ");
        cout       << errorExe.toLatin1().data() << endl;
        config_log << errorExe.toLatin1().data() << endl;
        succExe = false;
        compileTimes++;
        emit compilationFinished();
        return;
     }
  }
  else{
     cProc->setProcessEnvironment(environment);
     //cProc->setEnvironment(environment);
     cProc->start(command,the_args);
     if(cProc->error()==QProcess::FailedToStart){
         QString errorExe = QString("\n Could not start ") +
                            command + " " + the_args.join(" ");
                 errorExe += QString("\nEnvironment : ") + environment.toStringList().join("\n");
         cout       << errorExe.toLatin1().data() << endl;
         config_log << errorExe << endl;
         succExe = false;
         compileTimes++;
         emit compilationFinished();
         return;
     }
  }

  QString rExe = QString("\n") + command + " " + the_args.join(" ");
  config_log << rExe << "\n";
  config_log.flush();

  QString strObj = t[ currentTask ].fortranName();
   strObj.truncate(strObj.lastIndexOf("."));

   if ( fileExists( strObj.toLatin1().data(), objext ) )
      canCreateObj = true;
 }
 else{
  QString msg = (QString) "\n" + t[ currentTask ].taskName() ;
  msg = msg.leftJustified(60,'.');
  if ( ! succExe ){
    msg += QString(" no ");
  }
  else{
    msg += QString(" yes");
    if ( ! exeOut.isEmpty() ){
       if ( exeOut.endsWith("\n") ) exeOut.chop(1);

       if (t[ currentTask ].getAnswKey() == "argument")
          t[ currentTask ].setAnswVal( t[currentTask].getCurrentArg() );
       else // default is output
          t[ currentTask ].setAnswVal( exeOut );

       answer[(t[currentTask].getAnswVar()).toLatin1().data()]=t[currentTask].getAnswVal() ;
       if ( ! t[ currentTask ].getAnswVar().isEmpty() )
          msg += QString(" [") + t[currentTask].getAnswVal() + QString("]");
    }
  }

  t[ currentTask ].setResult( succExe );

  ioDevice->insertPlainText( msg ); ioDevice->ensureCursorVisible();

   //removing fortran file
   if ( t[ currentTask ].removeSource() ){
      QFile f_file( t[ currentTask ].fortranName() );
      if ( f_file.open( QIODevice::ReadWrite ) )
           f_file.remove();
   }
   //removing exe file
   QFile exe_file( t[ currentTask ].exeName() );
   if ( exe_file.open( QIODevice::ReadWrite ) )
        exe_file.remove();
   setCurrentTask(); // equivalent to currentTask++;
   emit taskFinished();
 }

}

void MTest::compilationProgress(){
    config_log << (QString)cProc->readAllStandardOutput();
    config_log.flush();
}

void MTest::errDetect(int exitCode, QProcess::ExitStatus exitStatus){

    if ( exitStatus != QProcess::NormalExit ) {
        if ( m_exit  ){
            ioDevice->insertPlainText( QString("\n Task stopped by user. \n") );
            config_log << "\n Task stopped by user with exit code: " << exitCode << endl;
        }
        else{
            ioDevice->insertPlainText(QString("\n Task failed. \n") );
            config_log << "\n Task failed with exit code: " << exitCode << endl;
        }
        config_log.flush();
        succComp = false;
        return;
    }
    if (exitCode != 0)
       succComp = false;
    else
       succComp = true;

    //clearing process arguments
    //cProc->clearArguments();

    // Prepare stuff for execution
    exeTimes = 0;
    QStringList tArgs = t[ currentTask ].getArg();
    if ( ! tArgs.isEmpty() ){
	if ( compileTimes < tArgs.count() )
	    args = tArgs[ compileTimes ].split(" ", QString::SkipEmptyParts);
	else
	    args = tArgs[ tArgs.count()-1 ].split(" ", QString::SkipEmptyParts);
    }
    else{
	args.clear();
    }
    compileTimes++;
    if ( ! args.isEmpty() && succComp ){ // execution required after compilation succeeded
//         cout << "Ready to execute with arguments: " << args.join(" : ").toLatin1().data() << endl;//EMH_DEBUG
	emit readyToExecute();
    }
    else if ( succComp ){  // no execution required after compilation succeeded, conclude task
	compileTimes = tcompileTimes;
	succExe = succExe && succComp;
//         cout << "Compilation finished successfully!" << endl;//EMH_DEBUG
	emit compilationFinished();
    }
    else{                        // compilation failed, see if there is another step or report failure
	succExe = succExe && succComp;
//         cout << "Compilation failed!" << endl;//EMH_DEBUG
	emit compilationFinished();
    }

}

void MTest::execute(){

    if ( exeTimes < args.count() ){
        QStringList the_arg; the_arg << QString( args[exeTimes] );
        QString command = QString("./") + t[currentTask].exeName();

        QString criterion = t[currentTask].checkMode();
        if ( criterion.toLower() == "exitstatus"){
            exeProc->setProcessChannelMode(QProcess::MergedChannels);
        }
        else if (  criterion.toLower() == "output"){
            exeProc->setProcessChannelMode(QProcess::SeparateChannels);
		}

        exeProc->start(command,the_arg);
        if(exeProc->error()==QProcess::FailedToStart){
	//if ( ! exeProc->start() ) {
	    // error handling
	    QString errorExe = QString("\n Could not execute ") +
                               command + QString(" ") + the_arg.join(" ");
	    cout << errorExe.toLatin1().data() << endl;
	    ioDevice->insertPlainText( errorExe );
            emit criticalError();
	}
	QString Exe = QString("\n") + command + QString(" ") + the_arg.join(" ");
             config_log << Exe << "\n";
               config_log.flush();
    }
    else{

	if (  ( t[ currentTask ].getArg().count() <= 1 ||
               !t[ currentTask ].RunAllSteps() )       &&
	       succExe && compileTimes < tcompileTimes  ){
	    compileTimes = tcompileTimes;
	}
	emit stepFinished();
    }
}

// We'll catch here output from running tests
void MTest::executionProgress(){
    exeOut = QString(exeProc->readAllStandardOutput()).trimmed();
    config_log << exeOut;
    config_log.flush();
}

// We'll catch here exit status from running tests
void MTest::executionStatus(int exitCode, QProcess::ExitStatus exitStatus){

  if ( exitStatus != QProcess::NormalExit ) {
    if ( m_exit  ){
      ioDevice->insertPlainText( QString("\n Task stopped by user. \n") );
      config_log << "\n Task stopped by user. \n";
    }
    else{
      ioDevice->insertPlainText( QString("\n Task failed. \n") );
      config_log << "\n Task failed. \n";
    }
    config_log.flush();
    succExe = succExe && false;
    return;
  }

  QString reference = t[ currentTask ].getReference();//QString() if not set
  QString a_tool_fun = t[ currentTask ].getToolFun();
  if ( !a_tool_fun.isEmpty() && lib ){
    SomeOtherFunc myfunc = (SomeOtherFunc) lib->resolve(a_tool_fun.toLatin1().data());
    if( myfunc ) {
      reference = myfunc( args[ exeTimes ].toLatin1().data() );
    }
  }
  else if(reference.isEmpty()){ // use argument as reference
    reference = args[ exeTimes ];
  }

  QString Exe = QString("\n Exit code is ") +
                 QString("%1").arg(exitCode,0,10);
  Exe+= QString("\n Reference is ") + reference;
  config_log <<  Exe << endl;
  config_log.flush();

  QString criterion = t[currentTask].checkMode();
  if ( criterion.toLower() == "exitstatus"){
    if ( QString("%1").arg( exitCode,0,10) != reference ){
      succExe = succExe && false;
    }
    else{
      succExe  = succExe && true;
    }
  }
  else if (  criterion.toLower() == "output"){

    config_log << " output:" << exeOut << endl;
    config_log << " reference:" << reference << endl;
    config_log.flush();

    if ( exeOut != reference ){
      succExe = succExe && false;
    }
    else{
      succExe  = succExe && true;
    }
  }

  //clearing process arguments
  if (succExe)
   t[currentTask].setCurrentArg(args[ exeTimes ]);
  exeTimes++;
  emit executionFinished();

}


bool MTest::fileExists( const char* rfile, const char** ext_name )
{
    QString the_name;
    for ( ushort i = 0; i < sizeof(ext_name)/sizeof(*ext_name); i++){
	the_name = (QString) rfile + "." + (QString) ext_name[i];
	QFileInfo fi( the_name );
	if ( fi.exists() && fi.isFile() ) {
	    return true;
	}

	i++;
    }
    return false;
}

void MTest::stop(){
       m_exit = true;
       if ( cProc->state() == QProcess::Running ){
         cProc->terminate();
         QTimer::singleShot( 20, cProc, SLOT( kill() ) );
         cProc->kill();
       }

       if ( exeProc->state() == QProcess::Running ){
         exeProc->terminate();
         QTimer::singleShot( 20, exeProc, SLOT( kill() ) );
         exeProc->kill();
       }
}

void MTest::reset(){
    currentTask = 0;
    m_exit = false;
    ObjDir = QString();
    linkerOptions  = QString();
    environment.clear();
};



/*---------------------------------------------------------------

     XMLTestReader IMPLEMENTATION

-----------------------------------------------------------------*/


XMLTestReader::XMLTestReader( const QString& xml_file ){
    xmldoc = xml_file;
    nTests = countTags(xmldoc);
    //nTests = countTags(xmldoc, "test");
    //cout << "Running " << nTests << " tests ..." << endl;//EMH_DEBUG
}

QString XMLTestReader::getProgram( const QDomNode &n ){
    QString prog = QString("");
    QDomText textChild = ((QDomNode)n).firstChild().toText();
    if ( !textChild.isNull()  ) {
       prog = textChild.nodeValue();
       if ( prog.startsWith("\n") ) prog.remove(0,1);
       if ( prog.endsWith("\n")   ) prog.chop(1);
    }
    return prog;
}

MSourceChunks XMLTestReader::getMap( const QDomElement &e ){
    MSourceChunks src;
    QString key;
    QDomNode node = e.firstChild();
    while ( !node.isNull() ) {
	if ( node.isElement() ) {
	    // case for the different header entries
	    QDomText textChild = node.firstChild().toText();
	    if ( !textChild.isNull()  ) {
		if ( node.nodeName() == "key" ) {
		    key = textChild.nodeValue();
		}
		else if( node.nodeName() == "body" ){
		    src[ key ].append( textChild.nodeValue() );
		}
	    }
	}
	node = node.nextSibling();
    }
    return src;
}
int XMLTestReader::countTags( const QString& xml_file){
    QDomDocument doc( "the_doc" );
    QFile file( xml_file );
    if ( !file.open( QIODevice::ReadOnly ) ){
	cout << "Could not open file " << xml_file.toLatin1().data() << endl;
	return 0;
    }
    if ( !doc.setContent( &file ) ) {
	file.close();
	cout << "Error in " << xml_file.toLatin1().data() << endl;
	return 0;
    }
    file.close();

    int counter = 0;

    QDomElement root = doc.documentElement();
    // get number of tests to run from root elemnt's attribute
    QDomAttr a = root.attributeNode( "number" );
    if ( !a.value().isEmpty() ){
	counter = a.value().toUInt(0,10);
    }
    else{
	QDomNode node = root.firstChild();
	while( !node.isNull() ) {
	    if ( node.isElement() && node.nodeName() == "test" )
		counter++;
	    node = node.nextSibling();
	}
    }
    //    cout << "Number of tasks = " << counter << endl;
    return counter;
}

MTest* XMLTestReader::getTests( const QString& xml_name ){

  Tasks* the_tasks;
  MTest* the_test = new MTest();
  the_test->setTotalTasks( nTests );

  //**********************************
  // PLAYING AROUND WITH DOM DOCS
  //**********************************

  QDomDocument doc( "testdoc" );
  //    QFile file( "tests.xml" );
  QFile file( xml_name );
  if ( !file.open( QIODevice::ReadOnly ) ){
    cout << "Could not open file " << xml_name.toLatin1().data() << endl;
    return 0;
  }
  if ( !doc.setContent( &file ) ) {
    file.close();
    cout << "Error in " << xml_name.toLatin1().data() << endl;
    return 0;
  }
  file.close();

  MSourceChunks chunks;
  QString the_program;
  QString OS;
#ifdef WIN32
  OS = "win32";
#else
  OS = "unix";
#endif


  // print out the element names of all elements that are direct children
  // of the outermost element.
  QDomElement root = doc.documentElement();


  the_tasks = new Tasks[nTests];   // <=== CREATE TASKS TO RUN
  ushort cTask      = 0;                          // current Task

  QDomNode node = root.firstChild();
  QDomElement e;
  //QDomNode prog;
  QDomNode parameter;
  while( !node.isNull() && cTask < nTests ) {
    if ( node.isElement() && node.nodeName() == "test" ) {
      //prog = node.firstChild();
      parameter = node.firstChild();
      while( !parameter.isNull() ){
        if (parameter.isElement() ){
          e = parameter.toElement(); // try to convert the node to an element.

          if (parameter.nodeName() == "program" ){   //main program to compile
            QDomAttr a = e.attributeNode( "name" );//name of program: if empty,
            the_tasks[cTask].setFName( a.value() );//a default name is used
            the_tasks[cTask].setProgram( getProgram(parameter) );
          }
          else if ( parameter.nodeName() == "map" ){// map of keys and source code for each test step
            if ( parameter.hasAttributes() ){
              QDomAttr aRep = e.attributeNode( "replace" );
              if ( !aRep.isNull() && aRep.value().toLower() == "yes")
                the_tasks[cTask].setReplacementMap(getMap(e) );
            }
            else{
              the_tasks[cTask].setChunks(getMap(e) );
            }
          }
          else if ( parameter.nodeName() == "replace" ){// any key in the test source will be replaced
            QDomAttr aRepl = e.attributeNode( "keyname" );//with answer from test given in this field
            the_tasks[cTask].replaceKey( aRepl.value() );
            QDomText textRepl= parameter.firstChild().toText();
            the_tasks[cTask].replaceAnsw( textRepl.nodeValue() );
          }
          else if ( parameter.nodeName() == "arg" ){            //execution arguments:if nArgs<nCompile
            QDomText textArg= parameter.firstChild().toText();//uses last available args
            the_tasks[cTask].setArgs( textArg.nodeValue() );
          }
          else if ( parameter.nodeName() == "description" ){       // Task description
            QDomText textDescription = parameter.firstChild().toText();
            the_tasks[cTask].setTaskName( textDescription.nodeValue() );
          }
          else if ( parameter.nodeName() == "language" ){          // fortran or C
            QDomText textLang = parameter.firstChild().toText();
            the_tasks[cTask].setLanguage( textLang.nodeValue() );
          }
          else if ( parameter.nodeName() == "opt" ){                    // compiler options
            QDomText textOpt = parameter.firstChild().toText();
            the_tasks[cTask].setOptions( textOpt.nodeValue() );
          }
          else if ( parameter.nodeName() == "mode" ){
            QDomText textMode = parameter.firstChild().toText();
            if ( textMode.nodeValue().toLower() == "all" ){// run all steps of a test
              the_tasks[cTask].setRunMode( true );
            }
            else{                                // runs as many steps until success or end of steps
              the_tasks[cTask].setRunMode( false );
            }
          }
          else if ( parameter.nodeName() == "critical" ){
            QDomText textCritical = parameter.firstChild().toText();
            if ( textCritical.nodeValue().toLower() == "yes" ){// stops configuration on an error
              the_tasks[cTask].setCriticalMode( true );
            }
          }
          else if ( parameter.nodeName() == "delete" ){
            QDomText textDelete = parameter.firstChild().toText();
            if ( textDelete.nodeValue().toLower() == "no" ){// keeps the source file
              the_tasks[cTask].setDeleteFlag( false );
            }
          }
          else if ( parameter.nodeName() == "depend" ){
            QDomText textDep = parameter.firstChild().toText();
            QString dName = textDep.nodeValue();
            //if ( textDep.nodeValue().toLower() == "yes" ){// do test only if previous fails
            if ( ! dName.isEmpty() ){                    // do test only if previous fails
              the_tasks[cTask].setDepTask( dName );
              the_tasks[cTask].setDepMode( true );
            }
          }
            else if ( parameter.nodeName() == "function" ){      //Function used after execution
              QDomText textFun = parameter.firstChild().toText();//of a test's step. Useful for
              the_tasks[cTask].setToolFun( textFun.nodeValue() );//checking 'things' .
            }
            else if ( parameter.nodeName() == "answer" ){         //Name of variable receiving
              QDomText textAnsw = parameter.firstChild().toText();//the result of a test
              the_tasks[cTask].setAnswVar( textAnsw.nodeValue() );
            }
            else if ( parameter.nodeName() == "check" ){
              QDomText textCheck = parameter.firstChild().toText();
              if ( textCheck.nodeValue().toLower() == "output" )// by default this is set to exit status
                the_tasks[cTask].setCheckMode( textCheck.nodeValue().toLower() );
            }
            else if ( parameter.nodeName() == "answ_key" ){
              QDomText textCheck = parameter.firstChild().toText();
              the_tasks[cTask].setAnswKey( textCheck.nodeValue().toLower() );
            }
            else if ( parameter.nodeName() == "reference" ){
              QDomText textCheck = parameter.firstChild().toText();
              the_tasks[cTask].setReference( textCheck.nodeValue() );
            }
            else if ( parameter.nodeName() == "obj" ){             //space separated object files
              QDomText textObj = parameter.firstChild().toText();//to be linked(with or without extens.)
              if ( parameter.hasAttributes() ){
                QDomAttr aObj = e.attributeNode( "OS" );
                if ( !aObj.isNull() ){
                  if ( ( aObj.value().toLower() == "win32" && OS.toLower() == "win32") ||
                      ( aObj.value().toLower() == "unix" && OS.toLower() == "unix")   )
                    the_tasks[cTask].setObjects( textObj.nodeValue() );
                }
              }
              else{
                the_tasks[cTask].setObjects( textObj.nodeValue() );
              }
            }
            else if ( parameter.nodeName() == "lib" ){             //space separated library names
              QDomText textLib = parameter.firstChild().toText();//  to be linked
              if ( parameter.hasAttributes() ){
                QDomAttr aLib = e.attributeNode( "OS" );
                if ( !aLib.isNull() ){
                  if ( ( aLib.value().toLower() == "win32" && OS.toLower() == "win32") ||
                      ( aLib.value().toLower() == "unix" && OS.toLower() == "unix")   )
                    the_tasks[cTask].setLibraries( textLib.nodeValue() );//with or without extens.on<=Win
                }                                                        //without extension on     <= Unix
              }
              else{
                the_tasks[cTask].setLibraries( textLib.nodeValue() );
              }
            }
          }
          parameter = parameter.nextSibling();
        }

        cTask++;
      }
      else if(  node.isElement() && node.nodeName() == "title" ){
        QDomText textTitle= node.firstChild().toText();
        the_test->setTitle( (QString)"\n" + textTitle.nodeValue()  + (QString)"\n" );
      }
      else if(  node.isElement() && node.nodeName() == "testlib" ){
        QDomText textLib= node.firstChild().toText();
        the_test->setTestLibName( textLib.nodeValue() );
      }

      node = node.nextSibling();
    }

    the_test->setTasks( the_tasks );

    return the_test;
}
