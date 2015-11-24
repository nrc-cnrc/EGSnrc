/*
###############################################################################
#
#  EGSnrc egs_inprz tools
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
#  Contributors:    Blake Walters
#
###############################################################################
*/


#include "tools.h"
#include <qdir.h>
#include <qmessagebox.h>
//Added by qt3to4:
#include <QLabel>
#include <cstdlib>

using namespace std;

//---------------------------------------------------------------------------------
//Tools for cleaning names from multiplicities and bad separators.
//They guarantee that the proper separator is used in a specific OS
//---------------------------------------------------------------------------------
/*
  Very simple utility that ONLY replaces DOUBLE separators for
  single ones.
  */
QString simplifyFileNames( const QString& str ){
    QString tmp = str;
	    tmp.replace( "//", "/"  );
	    tmp.replace( "\\\\", "\\" );
   return tmp;
}

/*takes an arbitrary string str and simplifies any
   SUBSEQUENT multiplicity of expr.
*/
QString stripRepetitions( const QString& str , const QString& expr ){
    QString tmp = str;
    int pos = tmp.indexOf( expr, 0);
    while ( pos >= 0 && pos < tmp.length() ){
	pos += expr.length();// move past the string
	if ( pos == tmp.indexOf( expr, pos) ){                 // expression found
	    tmp = tmp.remove( pos, expr.length() );// remove it
	    pos -= expr.length();                                 // reset position back
	}
	else{ // expression not found get next expression ocurrence
	    pos = tmp.indexOf( expr, pos);  // move on to find next position
	}
    }
    return tmp;
}

/*takes a string (a file or directory name ) and simplifies any
   multiple ocurrence of the unix and windows separator.
   But if there is mixed entries, it does nothing about it.*/
/*QString simplifySeparators( const QString& str ){
 QString the_name = str;
 QString unix_sep = "/";
 QString winx_sep = "\\";

 the_name = stripRepetitions( stripRepetitions( the_name, winx_sep ), unix_sep);
qDebug("Read: %s",the_name.toLatin1().data());
 return the_name;
}*/
QString simplifySeparators( const QString& str ){
 QString the_name = (str.split(QDir::separator(),QString::SkipEmptyParts)).join(QDir::separator());
 if (str.startsWith(QDir::separator())) the_name.prepend(QDir::separator());
 if (str.endsWith(QDir::separator())) the_name.append(QDir::separator());
//qDebug("Read: %s",the_name.toLatin1().data());
 return the_name;
}

/*takes a string (a file or directory name ) and replaces
   any ocurrence of the unix/windows anti-separator for
   the proper separator. But it does not remove multiplicities.*/
QString strightenItUp( const QString& str ){
 QString the_name = str;
#ifdef WIN32
 QString anti_sep = "/";
#else
 QString anti_sep = "\\";
#endif
 the_name.replace( anti_sep, QString("%1").arg( QDir::separator() ) );
 return the_name;
}

/*takes a string (a file or directory name ) and
   replaces any ocurrence of \ for /. But it does
   not remove multiplicities.*/
QString forwardItUp( const QString& str ){
 QString the_name = str;
 QString anti_sep = "\\";
 QString the_sep  = "/";
 the_name.replace( anti_sep, the_sep );
 return the_name;
}

/*And last, but not least, if we want to get a proper
   name according the the OS, we can use this little
   tool to first get the proper separator and then
   remove multiplicities

  CAVEAT: On Windows, when manipulating files
                    between servers, they MUST HAVE
                    DOUBLE separators at the start of
                    their names.
*/
QString ironIt( const QString& s ){
//return simplifySeparators( forwardItUp( s ) );
return simplifySeparators( strightenItUp( s ) );
}


//!  Check that the file \em fname exists.
/*!
This simple  function returns \em true  if the file \em fname exists
and \em false if it does not exist.
\b Beware:  It also returns a \em true value if \em fname is a directory name.
*/

bool check_file( const QString & fname )
{
  QFile f( fname );
    if ( !f.open( QIODevice::ReadOnly ) ) {
        f.close();
        return false;
    }
    f.close();
  return true;
}

//!  Check that the file \em name.ext exists in $HOME/egsnrc/name or copy it from $HEN_HOUSE/name.
/*!
This simple  function returns \em true  if the file \em fname exists
and \em false if it does not exist.
\b Beware:  It also returns a \em true value if \em fname is a directory name.
*/
bool set_file( const QString& ext,  const QString& name )
{
    QString ENVVAR       = "HEN_HOUSE";
    QString HEN_HOUSE = getenv(ENVVAR.toLatin1().data());

            ENVVAR = "HOME";
    QString HOME      = getenv(ENVVAR.toLatin1().data());

    QString dir_home = HOME + "/egsnrc/" + name + "/";
    QString file_home =  dir_home + name + "." + ext;
    if (!check_file( file_home ) ) {
	   QString dir_hen = HEN_HOUSE + "/" + name + "/";
                 QString file_hen  =  dir_hen + name + "."  + ext;
	   if (check_file( file_hen ) ) {
	       return copy( file_hen, file_home);
	   }
	   else{
               //qt3to4 -- BW
	       //cout << "Couldn't find file " << file_hen << endl;
               cout << "Couldn't find file " << file_hen.toStdString() << endl;
	       return false;
	   }
    }

    return true;
}

void chmod( const QString & attrib, const QString file )
{
     QString p  = "chmod " + attrib;
      p += " " + file + " &";
      if ( system( p.toLatin1().data() ) )
          //qt3to4 -- BW
	  //cout << "chmod failed on " << file << endl;
          cout << "chmod failed on " << file.toStdString() << endl;
}

bool copy(const QString& source, const QString& target)
{
    std::ifstream   in( source.toLatin1().data(), ios::in | ios::binary );
    std::ofstream out( target.toLatin1().data(), ios::out | ios::binary );
    if ( !in ) {
     //qt3to4 -- BW
     //cout << "File copy : Cannot open file: " << source << endl;
      cout << "File copy : Cannot open file: " << source.toStdString() << endl;
      return false;
    }
    char ch;
    while( 1 ){
	in.get(ch);
	if( in.eof() || !in.good() ) break;
	out.put( ch );
    }
    in.close();
    out.close();
   return true;
}

bool getVariable(QIODevice *inp, const QString &key, QString &value)
{
  inp->reset(); bool res = false;
  char buf[1024];
  while( !inp->atEnd() ) {
    inp->readLine(buf,1023L); QString aux = buf;
    int pos = aux.indexOf(key);
    if( pos >= 0 ) {
        value = aux.mid(pos+key.length()+1);
         value  =    value.trimmed().replace( (QString)"$(DSEP)" ,
	              QString("%1").arg( QDir::separator() ) );
        res = true; break;
    }
  }
  return res;
}

bool fileExists( const QString & fname )
{
QFileInfo fi( fname );
if ( fi.exists() && fi.isFile() ) {
    return true;
}
return false;
}

bool dirExists( const QString & fname )
{
QFileInfo fi( fname );
if ( fi.exists() && fi.isDir() ) {
    return true;
}
return false;
}

/******************************************************
   Although the name suggests otherwise, it removes
   EVERY occurrence of ext in thes trings on the list.
*******************************************************/
QStringList strip_extension(const QStringList & rlist, const QString& ext)
{
  QStringList l = rlist;
  for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it ) {
        (*it).remove(ext);
  }

  return l;
}

QStringList strip_str(const QStringList & rlist, const QString& ext)
{
  return strip_extension(rlist, ext);
}

QStringList listExtensionLess( const QString & rDirName, const QString & ext )
{
  QString extension = ext;
          extension = extension.remove(0,ext.lastIndexOf("."));
  return strip_extension( list_items(rDirName,ext ), extension );
}

QStringList list_items( const QString & rDirName, const QString & rFilter )
{
  // check whether rDirName exists
  QString dirName = rDirName;
  QDir d( dirName );
  if ( !d.exists() ) {
    //qt3to4 -- BW
    //return 0;
    return QStringList("");
  }

  d.setNameFilters( QStringList(rFilter) );
  d.setSorting( QDir::Name | QDir::IgnoreCase );
  //qt3to4 -- BW
  //const QFileInfoList *list = d.entryInfoList();
  //QFileInfoListIterator it( *list );      // create list iterator
  const QFileInfoList list = d.entryInfoList();
  QFileInfoList::const_iterator it;
  //QFileInfo *fi;
  QFileInfo fi;

  QStringList lst;

 //qt3to4 -- BW
 // while ( (fi=it.current()) ) {           // for each file...
  for(it = list.constBegin(); it !=list.constEnd(); ++it ) {
    //qt3to4 -- BW
    //++it;                               // go to next list element
    fi = *it;
    //qt3to4 -- BW
    //if ( fi->fileName() == ".." || fi->fileName() == "." )
    if ( fi.fileName() == ".." || fi.fileName() == "." )
       continue;
    //qt3to4 -- BW
    //lst += fi->fileName();
    lst += fi.fileName();
  }

  return lst;

}

/*
find_programs_in_system( const QStringList & progs, const char & sep )
Attempts to find programs provided in the list "progs" in any of the locations defined in the
PATH environment variable. The separator used in the PATH variable must be also supplied.
It returns a list of found programs.
*/
QStringList find_programs_in_system( const QStringList & progs, const char & sep )
{
QString pathvar = "PATH";
QString pathval = getenv(pathvar.toLatin1().data());
if ( pathval.at( pathval.length()-1 ) != sep ) pathval.append(sep);

//Extracting paths from environment variable PATH into a list
QStringList dirs_in_path;
short int sep_pos;
while ( ! pathval.isEmpty() ){
    sep_pos = pathval.indexOf ( sep, 0 );
    dirs_in_path += pathval.left( sep_pos );
    pathval.remove(0,sep_pos+1);
}

//Finding programs from list progs in the directory list
QStringList progs_found;
for ( QStringList::ConstIterator it1 = progs.begin(); it1 != progs.end(); ++it1 ) {
    for ( QStringList::Iterator it2 = dirs_in_path.begin(); it2 != dirs_in_path.end(); ++it2 ) {
	if ( fileExists(*it2 + "/" + *it1) ) progs_found += *it1;
    }
}

return strip_repetitions( progs_found );

}

QStringList strip_repetitions( const QStringList& str_list )
{
QStringList the_list = str_list;
the_list.sort();
QString  the_string;
QString the_dummy = the_list.join( ";" ) + (QString)";";
for ( QStringList::Iterator it = the_list.begin(); it != the_list.end(); ++it ) {
    the_string = stripRepetitions( the_dummy, *it + (QString)";" );
    the_dummy = the_string;
}
return the_string.split( ";");
}

void showAllWidgets( QObject *o )
{
  //qt3to4 -- BW
  //QObjectList *l = o->queryList( "QWidget",0,false,true );
  QList<QWidget *> l = o->findChildren<QWidget *>();

  //qt3to4 -- BW
  //if(!l->isEmpty()) {
    if(!l.isEmpty()) {
    QObject *oc;
  //qt3to4 -- BW
  //for( uint i=0; i < l->count(); i++ ) {
    for( uint i=0; i < l.count(); i++ ) {
        //o = l->at(i);
        oc = l.at(i);
        if (oc->isWidgetType())
           ((QWidget*)oc)->show();
    }
    //qt3to4 -- BW
    //delete l;
  }
  else{
   if (o->isWidgetType())
      ((QWidget*)o)->show();
  }
}

void hideAllWidgets( QObject *o )
{

  //qt3to4 -- BW
  //QObjectList *l = o->queryList( "QWidget",0,false,true );
  QList<QWidget *> l = o->findChildren<QWidget *>();

  //qt3to4 -- BW
  //if(!l->isEmpty()) {
    if(!l.isEmpty()) {
    QObject *oc;
    //qt3to4 -- BW
    //for( uint i=0; i < l->count(); i++ ) {
    for( uint i=0; i < l.count(); i++ ) {
        //oc = l->at(i);
        oc = l.at(i);
        if (oc->isWidgetType())
           ((QWidget*)oc)->hide();
    }
    //qt3to4 -- BW
    //delete l;
  }
  else{
   if (o->isWidgetType())
      ((QWidget*)o)->hide();
  }
}

void enableAllWidgets( QObject *o, bool enabled )
{

  //qt3to4 -- BW
  //QObjectList *l = o->queryList( "QWidget",0,false,true );
  QList<QWidget *> l = o->findChildren<QWidget *>();

  //qt3to4 -- BW
  //if(!l->isEmpty()) {
    if(!l.isEmpty()) {
    QObject *oc;
    //qt3to4 -- BW
    //for( uint i=0; i < l->count(); i++ ) {
    for( uint i=0; i < l.count(); i++ ) {
        //oc = l->at(i);
        oc = l.at(i);
        if (oc->isWidgetType())
           ((QWidget*)oc)->setEnabled(enabled);
    }
    //qt3to4 -- BW
    //delete l;
  }
  else{
   if (o->isWidgetType())
      ((QWidget*)o)->setEnabled(enabled);
  }
}

void modifyAllWidgets( QObject *o, bool show, bool enabled )
{

  //qt3to4 -- BW
  //QObjectList *l = o->queryList( "QWidget",0,false,false );
  QList<QWidget *> l = o->findChildren<QWidget *>();

  //qt3to4 -- BW
  //if(!l->isEmpty()) {
  if(!l.isEmpty()) {
    QObject *oc;
  //qt3to4 -- BW
  //  for( uint i=0; i < l->count(); i++ ) {
    for( uint i=0; i < l.count(); i++ ) {
      // oc = l->at(i);
       oc = l.at(i);
       if (oc->isWidgetType()){
        if (show) {((QWidget*)oc)->show();}
        else      {((QWidget*)oc)->hide();}
        ((QWidget*)oc)->setEnabled(enabled);
       }
    }
    //qt3to4 -- BW
    //delete l;
  }
  else{
    if (o->isWidgetType()){
       if (show) {((QWidget*)o)->show();}
       else      {((QWidget*)o)->hide();}
       ((QWidget*)o)->setEnabled(enabled);
    }
  }
}

void changeTextColor( QLabel* l, const QString& color )
{
  QString s = l->text();
  l->setText( "<p style=\"color:" + color + "\">"+ s + "</p>");
}


