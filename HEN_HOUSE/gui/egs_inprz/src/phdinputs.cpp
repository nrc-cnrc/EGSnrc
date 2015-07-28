/*
###############################################################################
#
#  EGSnrc egs_inprz pulse height distribution inputs
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


#include "phdinputs.h"
//Added by qt3to4:
//#include <Q3TextStream>
//qt3to4 -- BW
#include <QTextStream>

MPHDInputs::MPHDInputs()
{
  sreg.push_back( 1 ) ;
  bintop.push_back( 0.0 ) ;
  slote  = "0.01";
  deltae = "0.005";
}

MPHDInputs::~MPHDInputs()
{
	//free anything created in the constructor here
}

std::ifstream & operator >> ( std::ifstream & in, MPHDInputs*  rPHD )
{
	std::vector<string> codes;
	codes.push_back("REGION OF SENSITIVE VOLUME");
	codes.push_back("SLOTE");
	codes.push_back("DELTAE");
	codes.push_back("TOPS OF ENERGY BINS");

	DE_Parser *p = new DE_Parser(codes,0,"pulse height distribution input", in, false);


	rPHD->sreg   = getThem( codes[0], 0, 10000, rPHD->sreg, rPHD->errors, p ) ;
	rPHD->slote  = getIt(   codes[1], "0.01", rPHD->errors, p ) ;
	rPHD->deltae = getIt(   codes[2], "0.01", rPHD->errors, p ) ;
	if ( rPHD->slote.toFloat(0) < 0.0f )
	   rPHD->bintop = getThem( codes[3], 0.f, 1000000.f, rPHD->bintop, rPHD->errors, p ) ;

	if ( ! rPHD->errors.isEmpty() ) {
	    rPHD->errors = "***  PULSE HEIGHT Inputs block *** <br>" +  rPHD->errors + "<br>";
	}

	delete p;

	return in;
}

/*
Q3TextStream & operator << ( Q3TextStream & t, MPHDInputs * rPHD )
{

  print_delimeter( "start" , "pulse height distribution input", t);

	t << "REGION OF SENSITIVE VOLUME= " << rPHD->sreg << "\n";
	t << "SLOTE= "                      << rPHD->slote << "\n";
	t << "DELTAE= "                     << rPHD->deltae << "\n";
	if ( rPHD->slote.toFloat(0) < 0.0f )
	 t << "TOPS OF ENERGY BINS= " << rPHD->bintop << "\n";

  print_delimeter( "stop"  , "pulse height distribution input", t);

  return t;

}
*/

//qt3to4 -- BW
QTextStream & operator << ( QTextStream & t, MPHDInputs * rPHD )
{

  print_delimeter( "start" , "pulse height distribution input", t);

        t << "REGION OF SENSITIVE VOLUME= " << rPHD->sreg << "\n";
        t << "SLOTE= "                      << rPHD->slote << "\n";
        t << "DELTAE= "                     << rPHD->deltae << "\n";
        if ( rPHD->slote.toFloat(0) < 0.0f )
         t << "TOPS OF ENERGY BINS= " << rPHD->bintop << "\n";

  print_delimeter( "stop"  , "pulse height distribution input", t);

  return t;

}





