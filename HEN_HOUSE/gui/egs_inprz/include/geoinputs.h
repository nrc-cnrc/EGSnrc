/*
###############################################################################
#
#  EGSnrc egs_inprz geometry input headers
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


#ifndef GEOINPUTS_H
#define GEOINPUTS_H

#include "inputblock.h"
//Added by qt3to4:
//qt3to4 -- BW
//#include <Q3TextStream>

//qt3to4 -- BW
#include <QTextStream>

class MGEOInputs : public MInputBlock
{
public:
	MGEOInputs();
	~MGEOInputs();
	QString   GetInputMethod() const { return inp_meth; }
              QString   getErrors();

	QString inp_meth;
 	QString zface;
 	QString description_by;

 	v_string media;

 	v_float slabs;
 	v_float radii;

 	v_int   nslab;
 	v_int mednum;
 	v_int start_reg_slab;
 	v_int stop_reg_slab;
 	v_int start_ring;
 	v_int stop_ring;
};
std::ifstream & operator >> ( std::ifstream & in, MGEOInputs * rGEO );
//Q3TextStream   & operator << ( Q3TextStream &    t, MGEOInputs * rGEO );
//qt3to4 -- BW
QTextStream   & operator << ( QTextStream &    t, MGEOInputs * rGEO );
#endif
