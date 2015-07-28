/*
###############################################################################
#
#  EGSnrc egs_inprz title
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


#include "title.h"
//Added by qt3to4:
//#include <Q3TextStream>
//qt3to4 -- BW
#include <QTextStream>

MTitle::MTitle()
{
	str = "EGSnrc input template file: Please modify this !!!";
}

MTitle::~MTitle()
{
}


//QString parseStr( std::ifstream & in, const QString & id, const QString & defvalue )
QString parseStr( std::ifstream & in, const QString & id )
{
	const int bufsize = 1024;
	char buf[1024];
	QString strLine;
	QString t;
	do { // while blank or comment line
		in.getline( buf, bufsize );
		strLine = buf;
		t = strLine.simplified();
	}while ( in && ( t.left(1)=="#" || t.isEmpty() || t.isNull() ) );

	int i = t.indexOf( id,0,Qt::CaseInsensitive );
	if ( i == -1 ){// id not found...!
		return "*** error ***" ;
	}
	else { // found id, cut it and check for a comment
		QString s = t;
		s.remove(i,id.length());
		int indx = s.indexOf("#",0,Qt::CaseInsensitive );
		if (indx != -1) {
		    s.remove(indx,s.length()-indx+1);
		    s = s.simplified();
		}
		return s;
	}
}


std::ifstream & operator >> ( std::ifstream & in, MTitle* t )
{
  t->str = parseStr(in, "TITLE= ");
  if ( t->str == "*** error ***" ){
       t->errors = "*** Title not found *** <br>";
       t->str       = "*** Title not found ***";
   }
  return in;
}
/*
Q3TextStream & operator << ( Q3TextStream & ts, MTitle* t )
{
	ts << "TITLE= " << t->str << "\n" << "\n";
	return ts;
}
*/

//qt3to4 -- BW
QTextStream & operator << ( QTextStream & ts, MTitle* t )
{
        ts << "TITLE= " << t->str << "\n" << "\n";
        return ts;
}

