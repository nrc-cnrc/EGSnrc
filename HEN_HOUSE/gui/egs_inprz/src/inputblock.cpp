/*
###############################################################################
#
#  EGSnrc egs_inprz input block
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
#  Author:          Ernesto Mainegra-Hing, 2001
#
#  Contributors:    Blake Walters
#
###############################################################################
*/


# include "inputblock.h"
//Added by qt3to4:
//#include <Q3TextStream>

//qt3to4 -- BW
#include <QTextStream>
# define MAXENTRIES 30

MInputBlock::MInputBlock()
{
	usercode = cavrznrc;
}


MInputBlock::~MInputBlock()
{
}


void MInputBlock::SetUserCode( const UserCodeType& uc )
{
			usercode = uc;
}

//qt3to4 -- BW
//void print_delimeter( const QString& boundary , const QString& section, Q3TextStream &t )
void print_delimeter( const QString& boundary , const QString& section, QTextStream &t )
{
 if ( boundary == "start" ) {
	t << "##########################"; t << "\n";
   	t << ":start " << section << ":"; t << "\n"; t << "\n";
 }
 else if ( boundary == "stop") {
   	t << "\n";
   	t << ":stop " << section <<  ":"; t << "\n";
	t << "#########################"; t << "\n";
	t << "\n";
 }
 else {
	t << "*************************************"; t << "\n";
	t << " BEWARE !!!!!  ERROR IN DELIMETER FOR " << section << ":";
	t << "\n"; t << "\n";
	t << "*************************************"; t << "\n";
 }
}

QString getIt( string &code, QString def, QString & error, DE_Parser *p )
{
 	std::vector<string> s;
	QString str ="";

 	if ( !p->get_input( code, s ) ) {
	   std::vector<string>::iterator iter(s.begin());
	   while (iter != s.end()){
	   	str += (*iter).c_str();
		str += " ";
		iter++;
	   }
	   str = str.simplified();
	   return str;
	}
	else {
	   error += "value sought not found for " ;
	   error += code.c_str(); error += "<br>";
	   return def;
	}
}

//**********************************************
// *********           OPERATORS     ***********
//**********************************************
/*
Q3TextStream & operator << ( Q3TextStream & ts, v_int & v )
{
  std::vector<int>::iterator iter(v.begin());
  int   j = 0;
  int max = MAXENTRIES; //arbitrary number per line!!!
  while ( iter != v.end()) {
    j++;
    ts << *iter++;
    if ( iter != v.end() ) ts << ", ";
    if ( j == max ) {ts << "\n"; j = 0;}//put end-of-line and
  }                                     //reset counter

  return ts;
}

Q3TextStream & operator << ( Q3TextStream & ts, v_float & v )
{
  std::vector<float>::iterator iter(v.begin());
  int   j = 0;
  int max = MAXENTRIES; //arbitrary number per line!!!
  while ( iter != v.end()) {
    j++;
    ts << *iter++;
    if ( iter != v.end() ) ts << ", ";
    if ( j == max ) {ts << "\n";j = 0;}//end-of-line, reset counter
  }

  return ts;
}

Q3TextStream & operator << ( Q3TextStream & ts, v_string & v )
{
  std::vector<string>::iterator iter(v.begin());
  int   j = 0;
  while ( iter != v.end()) {
    j++;
    ts << (*iter++).c_str();
    if ( iter != v.end() ) {
  		ts << ",\n ";
    }
    else {
	  	ts << ";\n ";
    }
  }

  return ts;
}

Q3TextStream & operator << ( Q3TextStream & ts, string & str )
{
	ts << str.c_str();
	return ts;
}

Q3TextStream & operator >> ( Q3TextStream & ts, string & str )
{
	QString s;
	ts >> s;
	str = s.toLatin1();
	return ts;
}
*/

//qt3to4 -- BW

QTextStream & operator << ( QTextStream & ts, v_int & v )
{
  std::vector<int>::iterator iter(v.begin());
  int   j = 0;
  int max = MAXENTRIES; //arbitrary number per line!!!
  while ( iter != v.end()) {
    j++;
    ts << *iter++;
    if ( iter != v.end() ) ts << ", ";
    if ( j == max ) {ts << "\n"; j = 0;}//put end-of-line and
  }                                     //reset counter

  return ts;
}

QTextStream & operator << ( QTextStream & ts, v_float & v )
{
  std::vector<float>::iterator iter(v.begin());
  int   j = 0;
  int max = MAXENTRIES; //arbitrary number per line!!!
  while ( iter != v.end()) {
    j++;
    ts << *iter++;
    if ( iter != v.end() ) ts << ", ";
    if ( j == max ) {ts << "\n";j = 0;}//end-of-line, reset counter
  }

  return ts;
}

QTextStream & operator << ( QTextStream & ts, v_string & v )
{
  std::vector<string>::iterator iter(v.begin());
  int   j = 0;
  while ( iter != v.end()) {
    j++;
    ts << (*iter++).c_str();
    if ( iter != v.end() ) {
                ts << ",\n ";
    }
    else {
                ts << ";\n ";
    }
  }

  return ts;
}

QTextStream & operator << ( QTextStream & ts, string & str )
{
        ts << str.c_str();
        return ts;
}

QTextStream & operator >> ( QTextStream & ts, string & str )
{
        QString s;
        ts >> s;
        str = s.toStdString();
        return ts;
}
