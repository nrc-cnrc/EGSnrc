/*
###############################################################################
#
#  EGSnrc egs_inprz I/O inputs headers
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


#ifndef IOINPUTS_H
#define IOINPUTS_H

#include "inputblock.h"
//Added by qt3to4:
//qt3to4 -- BW
//#include <Q3TextStream>
//qt3to4 -- BW
#include <QTextStream>

typedef struct
{
 int minPln;
 int maxPln;
 int minCyl;
 int maxCyl;
} REGIONS ;

typedef struct
{
 v_int start_cyl;
 v_int stop_slab;
} SPROUT;

class MIOInputs : public MInputBlock
{
public:
	MIOInputs();
	~MIOInputs();

 	QString iwatch;
 	QString strnd;
 	QString irestart;
 	QString strdat;
 	QString outopt;
 	QString etransport;
 	QString sproutopt;
 	SPROUT  sproutreg;
 	REGIONS dosereg;

 	QString printFluSpec;
 	v_int   ListFluStart;
 	v_int   ListFluStop;
 	QString iprimary;
 	QString slote;
 	v_float bintop;
};
std::ifstream & operator >> ( std::ifstream & in, MIOInputs * rIO );
//Q3TextStream   & operator << ( Q3TextStream &    t, MIOInputs * rIO );
//qt3to4 -- BW
QTextStream   & operator << ( QTextStream &    t, MIOInputs * rIO );
#endif
