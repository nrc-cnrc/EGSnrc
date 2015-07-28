/*
###############################################################################
#
#  EGSnrc egs_inprz cavity inputs
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


#include "cavinputs.h"
//Added by qt3to4:
//#include <Q3TextStream>

//qt3to4 -- BW
#include <QTextStream>

MCAVInputs::MCAVInputs()
{
	ncavreg = "1";
 	cav_reg.push_back( 2 );
 	wall_thick = "0.1";
	cav_rad    = "1.5";
	cav_len    = "0.2";
	electr_rad = "0.15";
	wall_mat   = "GRAPHITE";
	electr_mat = "GRAPHITE";
}

MCAVInputs::~MCAVInputs()
{
	//free anything created in the constructor here
}

void MCAVInputs::SetInputMethod( const QString& rInpMeth )
{
	inp_meth = rInpMeth;
}

std::ifstream & operator >> ( std::ifstream & in, MCAVInputs*  rCAV )
{

	std::vector<string> codes;
	codes.push_back("NUMBER OF CAVITY REGIONS");
	codes.push_back("REGION NUMBERS OF THE CAVITY");

	codes.push_back("WALL THICKNESS");
	codes.push_back("CAVITY RADIUS");
	codes.push_back("CAVITY LENGTH");
	codes.push_back("ELECTRODE RADIUS");
	codes.push_back("WALL MATERIAL");
	codes.push_back("ELECTRODE MATERIAL");

	DE_Parser *p = new DE_Parser(codes,0,"cavity inputs", in, false);

	if ( rCAV->inp_meth.toLower() != "cavity information" ) {
	   rCAV->ncavreg = getIt( codes[0] , "1", rCAV->errors, p ) ;
	   rCAV->cav_reg = getThem( codes[1], 0, 1000, rCAV->cav_reg, rCAV->errors, p ) ;

	}
	else {
	   rCAV->wall_thick = getIt( codes[2] , "0.10", rCAV->errors, p );
	   rCAV->cav_rad    = getIt( codes[3] , "1.50", rCAV->errors, p );
	   rCAV->cav_len    = getIt( codes[4] , "0.10", rCAV->errors, p );
	   rCAV->electr_rad = getIt( codes[5] , "0.15", rCAV->errors, p );
	   rCAV->wall_mat   = getIt( codes[6] , "GRAPHITE", rCAV->errors, p );
	   rCAV->electr_mat = getIt( codes[7] , "GRAPHITE", rCAV->errors, p );

	}

	if ( ! rCAV->errors.isEmpty() ) {
	    rCAV->errors = "***  CAVITY Inputs block *** <br>" +  rCAV->errors + "<br>";
	}

	delete p;

	return in;
}

/*
Q3TextStream & operator << ( Q3TextStream & t, MCAVInputs * rCAV )
{

  print_delimeter( "start" , "cavity inputs", t);
	if ( rCAV->inp_meth != "cavity information" ) {
	   t << "NUMBER OF CAVITY REGIONS= " << rCAV->ncavreg << "\n";
	   t << "REGION NUMBERS OF THE CAVITY= " << rCAV->cav_reg << "\n";
	}
	else {
	   t << "WALL THICKNESS= "     << rCAV->wall_thick << "\n";
	   t << "CAVITY RADIUS= "      << rCAV->cav_rad << "\n";
	   t << "CAVITY LENGTH= "      << rCAV->cav_len << "\n";
	   t << "ELECTRODE RADIUS= "   << rCAV->electr_rad << "\n";
	   t << "WALL MATERIAL= "      << rCAV->wall_mat << "\n" ;
	   if ( rCAV->electr_rad.toFloat(0) > 0.0f )
	      t << "ELECTRODE MATERIAL= " << rCAV->electr_mat << "\n";
	}

  print_delimeter( "stop" , "cavity inputs", t);

  return t;

}
*/

//qt3to4 -- BW
QTextStream & operator << ( QTextStream & t, MCAVInputs * rCAV )
{

  print_delimeter( "start" , "cavity inputs", t);
        if ( rCAV->inp_meth != "cavity information" ) {
           t << "NUMBER OF CAVITY REGIONS= " << rCAV->ncavreg << "\n";
           t << "REGION NUMBERS OF THE CAVITY= " << rCAV->cav_reg << "\n";
        }
        else {
           t << "WALL THICKNESS= "     << rCAV->wall_thick << "\n";
           t << "CAVITY RADIUS= "      << rCAV->cav_rad << "\n";
           t << "CAVITY LENGTH= "      << rCAV->cav_len << "\n";
           t << "ELECTRODE RADIUS= "   << rCAV->electr_rad << "\n";
           t << "WALL MATERIAL= "      << rCAV->wall_mat << "\n" ;
           if ( rCAV->electr_rad.toFloat(0) > 0.0f )
              t << "ELECTRODE MATERIAL= " << rCAV->electr_mat << "\n";
        }

  print_delimeter( "stop" , "cavity inputs", t);

  return t;

}





