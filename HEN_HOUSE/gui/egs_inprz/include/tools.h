/*
###############################################################################
#
#  EGSnrc egs_inprz tools headers
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


#ifndef TOOLS_H
#define TOOLS_H

#include <qfile.h>
#include <qlabel.h>
#include <qstringlist.h>
#include <fstream>
//qt3to4 -- BW
//#include <q3textstream.h>
#include <iostream>
#include <string>
#include <vector>

//qt3to4 -- BW
#include <QTextStream>

using namespace std;

#define zap(x) if(x){delete(x); x=0;}

typedef std::vector<char*>  v_ptr;
typedef std::vector<int>    v_int;
typedef std::vector<float>  v_float;
typedef std::vector<string> v_string;
/*
 ***********************************************************
 *
 *   U T I L I T Y               F U N C T I O N S
 *  (global functions, maybe I should create a general utility class)
 *
 ***********************************************************
 */

// defined in inputRZImplementation.cpp :
bool check_file( const QString & fname );
bool set_file( const QString& ext, const QString& name );
void chmod( const QString & attrib, const QString file );
bool copy(const QString& source, const QString& target);
bool getVariable(QIODevice *inp, const QString &key, QString &value);
QString simplifyFileNames( const QString& str );
QString stripRepetitions( const QString& str , const QString& expr );
QString simplifySeparators( const QString& str );
QString strightenItUp( const QString& str );
QString forwardItUp( const QString& str );
QString ironIt( const QString& str );
bool fileExists( const QString & fname );
bool dirExists( const QString & fname );
QStringList find_programs_in_system( const QStringList & progs, const char & sep );
QStringList strip_repetitions( const QStringList& str_list );
QStringList list_items( const QString & rDirName, const QString & rFilter );
QStringList strip_extension(const QStringList & rlist, const QString& ext);
QStringList strip_str(const QStringList & rlist, const QString& ext);
QStringList listExtensionLess( const QString & rDirName, const QString & ext );
#include <qwidget.h>
#include <qobject.h>
//void showAllWidgets( QWidget *w );
//void hideAllWidgets( QWidget *w );
//void enableAllWidgets( QWidget *w, bool enabled );
//void modifyAllWidgets( QWidget *w, bool show, bool enabled );
void showAllWidgets( QObject *o );
void hideAllWidgets( QObject *o );
void enableAllWidgets(QObject *o, bool enabled );
void modifyAllWidgets(QObject *o, bool show, bool enabled );
void changeTextColor( QLabel* l, const QString& color );

/*
 ***********************************************************
 *
 *                 O P E R A T O R S
 *
 ***********************************************************
 */
//Q3TextStream & operator << ( Q3TextStream & ts, v_int & v );
//Q3TextStream & operator << ( Q3TextStream & ts, v_float & v );
//Q3TextStream & operator << ( Q3TextStream & ts, v_string & v );
//Q3TextStream & operator << ( Q3TextStream & ts, string & str );
//Q3TextStream & operator >> ( Q3TextStream & ts, string & str );
//qt3to4 -- BW
QTextStream & operator << ( QTextStream & ts, v_int & v );
QTextStream & operator << ( QTextStream & ts, v_float & v );
QTextStream & operator << ( QTextStream & ts, v_string & v );
QTextStream & operator << ( QTextStream & ts, string & str );
QTextStream & operator >> ( QTextStream & ts, string & str );

#endif	// TOOLS_H
