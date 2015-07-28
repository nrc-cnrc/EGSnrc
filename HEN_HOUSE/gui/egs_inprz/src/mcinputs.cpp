/*
###############################################################################
#
#  EGSnrc egs_inprz Monte Carlo inputs
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


#include "mcinputs.h"
//Added by qt3to4:
//#include <Q3TextStream>
//qt3to4
#include <QTextStream>

MMCInputs::MMCInputs()
{
  ncase   = "20000";
	rnd.push_back( 1 );
	rnd.push_back( 33 );
	maxcpu  = "60";
	ifull   = "dose and stoppers";
	stat    = "0.1";
	kerma   = "no";
	photreg = "no";
}

MMCInputs::~MMCInputs()
{
	//free anything created in the constructor here
}

std::ifstream & operator >> ( std::ifstream & in, MMCInputs*  rMC )
{
	std::vector<string> codes;
	codes.push_back("NUMBER OF HISTORIES");
	codes.push_back("MAX CPU HOURS ALLOWED");
	codes.push_back("IFULL");
	codes.push_back("STATISTICAL ACCURACY SOUGHT");
	codes.push_back("SCORE KERMA");
	codes.push_back("PHOTON REGENERATION");
	codes.push_back("INITIAL RANDOM NO. SEEDS");

	DE_Parser *p = new DE_Parser(codes,0,"Monte Carlo inputs", in, false);

	rMC->ncase   = getIt( codes[0] , "20000", rMC->errors, p ) ;
	rMC->maxcpu  = getIt( codes[1] , "60", rMC->errors, p ) ;
  	if ( ( rMC->gusercode() != sprrznrc ) && ( rMC->gusercode() != flurznrc ) ){
		rMC->ifull   = getIt( codes[2] , "dose and stoppers", rMC->errors, p ) ;
		rMC->stat    = getIt( codes[3] , "0.1", rMC->errors, p ) ;
	}
	if ( rMC->gusercode() == dosrznrc) {
		rMC->kerma   = getIt( codes[4] , "no", rMC->errors, p ) ;
	}
	else if ( rMC->gusercode() != flurznrc ) {
        	rMC->photreg = getIt( codes[5] , "no", rMC->errors, p ) ;
	}

	rMC->rnd   = getThem( codes[6], 1, 1073741824, rMC->rnd, rMC->errors, p ) ;
//	if ( rMC->rnd[0] > 4 ) {
//		rMC->rnd[0] = 1;
//		rMC->errors += "wrong random luxury level, default to 1 ! <br>";
//	}
	if ( !rMC->errors.isEmpty() ) {
	   rMC->errors = "***  MC Inputs block *** <br>" + rMC->errors + "<br>";
	}

	delete p;

	return in;
}

/*
Q3TextStream & operator << ( Q3TextStream & t, MMCInputs * rMC )
{
	print_delimeter( "start" , "Monte Carlo inputs", t);

	t << "NUMBER OF HISTORIES= "      << rMC->ncase  << "\n";
	t << "INITIAL RANDOM NO. SEEDS= " << rMC->rnd    << "\n";
	t << "MAX CPU HOURS ALLOWED= "    << rMC->maxcpu << "\n";
	if ( ( rMC->gusercode() != sprrznrc) ||
	     ( rMC->gusercode() != flurznrc) ){
	 t << "IFULL= "                      << rMC->ifull << "\n";
	 t << "STATISTICAL ACCURACY SOUGHT= " << rMC->stat << "\n";
	}
	if ( rMC->gusercode() == dosrznrc ) {
	   t << "SCORE KERMA= " << rMC->kerma << "\n";
	}
	else if ( rMC->gusercode() != flurznrc ) {
	   t << "PHOTON REGENERATION= " << rMC->photreg << "\n";
	}

	print_delimeter( "stop" , "Monte Carlo inputs", t);

	return t;
}
*/

//qt3to4 -- BW
QTextStream & operator << ( QTextStream & t, MMCInputs * rMC )
{
        print_delimeter( "start" , "Monte Carlo inputs", t);

        t << "NUMBER OF HISTORIES= "      << rMC->ncase  << "\n";
        t << "INITIAL RANDOM NO. SEEDS= " << rMC->rnd    << "\n";
        t << "MAX CPU HOURS ALLOWED= "    << rMC->maxcpu << "\n";
        if ( ( rMC->gusercode() != sprrznrc) ||
             ( rMC->gusercode() != flurznrc) ){
         t << "IFULL= "                      << rMC->ifull << "\n";
         t << "STATISTICAL ACCURACY SOUGHT= " << rMC->stat << "\n";
        }
        if ( rMC->gusercode() == dosrznrc ) {
           t << "SCORE KERMA= " << rMC->kerma << "\n";
        }
        else if ( rMC->gusercode() != flurznrc ) {
           t << "PHOTON REGENERATION= " << rMC->photreg << "\n";
        }

        print_delimeter( "stop" , "Monte Carlo inputs", t);

        return t;
}





