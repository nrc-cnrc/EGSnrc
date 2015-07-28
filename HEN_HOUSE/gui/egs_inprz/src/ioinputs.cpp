/*
###############################################################################
#
#  EGSnrc egs_inprz I/O inputs
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


#include "ioinputs.h"
//Added by qt3to4:
//#include <Q3TextStream>
//qt3to4 -- BW
#include <QTextStream>

MIOInputs::MIOInputs()
{
	iwatch     = "off";
	strnd      = "no";
	irestart   = "first";
	strdat     = "yes";
  outopt     = "short";
  etransport = "normal";
 	sproutopt= "regions";
  sproutreg.start_cyl.push_back(1);
  sproutreg.stop_slab.push_back(1);
  dosereg.minPln = 1;
  dosereg.maxPln = 61;
  dosereg.minCyl = 0;
  dosereg.maxCyl = 60;
  printFluSpec = "all";
  ListFluStart.push_back( 1 );
  ListFluStop.push_back( 1 );
  iprimary = "total fluence";
  slote    = "0.01";
  bintop.push_back( 0.0 );
}

MIOInputs::~MIOInputs()
{
	//free anything created in the constructor here
}

std::ifstream & operator >> ( std::ifstream & in, MIOInputs*  rIO )
{

	std::vector<string> codes;
	codes.push_back("IWATCH");
	codes.push_back("STORE INITIAL RANDOM NUMBERS");
	codes.push_back("IRESTART");
	codes.push_back("STORE DATA ARRAYS");
	codes.push_back("OUTPUT OPTIONS");
	codes.push_back("ELECTRON TRANSPORT");
	codes.push_back("DOSE ZBOUND MIN");
	codes.push_back("DOSE ZBOUND MAX");
	codes.push_back("DOSE RBOUND MIN");
	codes.push_back("DOSE RBOUND MAX");
	codes.push_back("SPR OUTPUT");
	codes.push_back("SPR IN CYLINDER IX");
	codes.push_back("SPR IN SLAB IZ");
	codes.push_back("SPR START REGION");
	codes.push_back("SPR STOP REGION");

	codes.push_back("PRINT FLUENCE SPECTRA");
	codes.push_back("LIST FLUENCE START REGION");
	codes.push_back("LIST FLUENCE STOP REGION");
	codes.push_back("IPRIMARY");
	codes.push_back("SLOTE");
	codes.push_back("TOPS OF ENERGY BINS");

	DE_Parser *p = new DE_Parser(codes,0,"I/O control", in, false);
	rIO->iwatch     = getIt( codes[0] , "off"   , rIO->errors, p ) ;
	rIO->strnd      = getIt( codes[1] , "no"    , rIO->errors, p ) ;
	rIO->irestart   = getIt( codes[2] , "first" , rIO->errors, p ) ;
	rIO->strdat     = getIt( codes[3] , "no"    , rIO->errors, p ) ;
  	if ( ( rIO->gusercode() != sprrznrc) && ( rIO->gusercode() != flurznrc) )
	   rIO->outopt     = getIt( codes[4] , "short", rIO->errors, p ) ;
	if ( rIO->gusercode() == dosrznrc) {
	   rIO->etransport = getIt( codes[5] , "no", rIO->errors, p ) ;
	   rIO->dosereg.minPln  = getItsafe( codes[6], 0, 1000, -111, rIO->errors, p  ) ;
	   rIO->dosereg.maxPln  = getItsafe( codes[7], 0, 1000, -111, rIO->errors, p  ) ;
	   rIO->dosereg.minCyl  = getItsafe( codes[8], 0, 1000, -111, rIO->errors, p  ) ;
	   rIO->dosereg.maxCyl  = getItsafe( codes[9], 0, 1000, -111, rIO->errors, p  ) ;
	}

	if ( rIO->gusercode() == sprrznrc) {
	   rIO->sproutopt  = getIt( codes[10], "regions", rIO->errors, p ) ;
	   if ( rIO->sproutopt.toLower() == "slabs/cylinders"){
	      rIO->sproutreg.start_cyl  = getThem( codes[11], 0, 1000, rIO->sproutreg.start_cyl, rIO->errors, p ) ;
	      rIO->sproutreg.stop_slab  = getThem( codes[12], 0, 1000, rIO->sproutreg.stop_slab, rIO->errors, p ) ;
	   }
	   else {
	      rIO->sproutreg.start_cyl  = getThem( codes[13], 0, 1000, rIO->sproutreg.start_cyl, rIO->errors, p ) ;
	      rIO->sproutreg.stop_slab  = getThem( codes[14], 0, 1000, rIO->sproutreg.stop_slab, rIO->errors, p ) ;
	   }
	}


	if ( rIO->gusercode() == flurznrc) {
	   rIO->printFluSpec = getIt( codes[15] , "all", rIO->errors, p ) ;
	   if ( rIO->printFluSpec.toLower() == "specified" ){
       		rIO->ListFluStart = getThem( codes[16], 0, 100000, rIO->ListFluStart, rIO->errors, p ) ;
		      rIO->ListFluStop  = getThem( codes[17], 0, 100000, rIO->ListFluStop, rIO->errors, p ) ;
	   }
	   rIO->iprimary = getIt( codes[18] , "total fluence", rIO->errors, p ) ;
	   rIO->slote    = getIt( codes[19] , "0.01", rIO->errors, p ) ;
	   if ( rIO->slote.toFloat( 0 ) == 0.0f ) {
      		rIO->bintop = getThem( codes[20], 0.f, 1000.f, rIO->bintop, rIO->errors, p ) ;
	   }
  	}

	if ( ! rIO->errors.isEmpty() ) {
	    rIO->errors = "*** I/O Control block *** <br>" +  rIO->errors + "<br>";
	}

	delete p;

	return in;
}

/*
Q3TextStream & operator << ( Q3TextStream & t, MIOInputs * rIO )
{

	print_delimeter( "start" , "I/O control", t);

	t << "IWATCH= "                       << rIO->iwatch   << "\n";
	t << "STORE INITIAL RANDOM NUMBERS= " << rIO->strnd    << "\n";
	t << "IRESTART= "                     << rIO->irestart << "\n";
	t << "STORE DATA ARRAYS= "            << rIO->strdat   << "\n";

	if ( ( rIO->gusercode() != sprrznrc ) && ( rIO->gusercode() != flurznrc ) ){
	 t << "OUTPUT OPTIONS= " << rIO->outopt << "\n";
	}
	if ( rIO->gusercode() == dosrznrc ) {
	   t << "ELECTRON TRANSPORT= " << rIO->etransport       << "\n";
	   t << "DOSE ZBOUND MIN= "    << rIO->dosereg.minPln   << "\n";
	   t << "DOSE ZBOUND MAX= "    << rIO->dosereg.maxPln   << "\n";
	   t << "DOSE RBOUND MIN= "    << rIO->dosereg.minCyl   << "\n";
	   t << "DOSE RBOUND MAX= "    << rIO->dosereg.maxCyl   << "\n";
	}

	if ( rIO->gusercode() == sprrznrc ) {
	   t << "SPR OUTPUT= " << rIO->sproutopt << "\n";
	   if ( rIO->sproutopt.toLower() == "slabs/cylinders"){
	      t << "SPR IN CYLINDER IX= " << rIO->sproutreg.start_cyl << "\n";
	      t << "SPR IN SLAB IZ= "     << rIO->sproutreg.stop_slab << "\n";
	   }
	   else {
	      t << "SPR START REGION= " << rIO->sproutreg.start_cyl << "\n";
	      t << "SPR STOP REGION= "  << rIO->sproutreg.stop_slab << "\n";
	   }
	}

	if ( rIO->gusercode() == flurznrc) {
	   	t << "PRINT FLUENCE SPECTRA= " << rIO->printFluSpec << "\n";

	   	if ( rIO->printFluSpec.toLower() == "specified" ){
          		t << "LIST FLUENCE START REGION= " << rIO->ListFluStart << "\n";
          		t << "LIST FLUENCE STOP REGION= " << rIO->ListFluStop << "\n";
		}

  		t << "IPRIMARY= " << rIO->iprimary << "\n";

		t << "SLOTE= " << rIO->slote << "\n";

		if ( rIO->slote.toFloat( 0 ) == 0.0f ) {
          		t << "TOPS OF ENERGY BINS= " << rIO->bintop << "\n";
		}
  	}

  	print_delimeter( "stop" , "I/O control", t);

	return t;
}
*/

//qt3to4 -- BW
QTextStream & operator << ( QTextStream & t, MIOInputs * rIO )
{

        print_delimeter( "start" , "I/O control", t);

        t << "IWATCH= "                       << rIO->iwatch   << "\n";
        t << "STORE INITIAL RANDOM NUMBERS= " << rIO->strnd    << "\n";
        t << "IRESTART= "                     << rIO->irestart << "\n";
        t << "STORE DATA ARRAYS= "            << rIO->strdat   << "\n";

        if ( ( rIO->gusercode() != sprrznrc ) && ( rIO->gusercode() != flurznrc ) ){
         t << "OUTPUT OPTIONS= " << rIO->outopt << "\n";
        }
        if ( rIO->gusercode() == dosrznrc ) {
           t << "ELECTRON TRANSPORT= " << rIO->etransport       << "\n";
           t << "DOSE ZBOUND MIN= "    << rIO->dosereg.minPln   << "\n";
           t << "DOSE ZBOUND MAX= "    << rIO->dosereg.maxPln   << "\n";
           t << "DOSE RBOUND MIN= "    << rIO->dosereg.minCyl   << "\n";
           t << "DOSE RBOUND MAX= "    << rIO->dosereg.maxCyl   << "\n";
        }

        if ( rIO->gusercode() == sprrznrc ) {
           t << "SPR OUTPUT= " << rIO->sproutopt << "\n";
           if ( rIO->sproutopt.toLower() == "slabs/cylinders"){
              t << "SPR IN CYLINDER IX= " << rIO->sproutreg.start_cyl << "\n";
              t << "SPR IN SLAB IZ= "     << rIO->sproutreg.stop_slab << "\n";
           }
           else {
              t << "SPR START REGION= " << rIO->sproutreg.start_cyl << "\n";
              t << "SPR STOP REGION= "  << rIO->sproutreg.stop_slab << "\n";
           }
        }

        if ( rIO->gusercode() == flurznrc) {
                t << "PRINT FLUENCE SPECTRA= " << rIO->printFluSpec << "\n";

                if ( rIO->printFluSpec.toLower() == "specified" ){
                        t << "LIST FLUENCE START REGION= " << rIO->ListFluStart << "\n";
                        t << "LIST FLUENCE STOP REGION= " << rIO->ListFluStop << "\n";
                }

                t << "IPRIMARY= " << rIO->iprimary << "\n";

                t << "SLOTE= " << rIO->slote << "\n";

                if ( rIO->slote.toFloat( 0 ) == 0.0f ) {
                        t << "TOPS OF ENERGY BINS= " << rIO->bintop << "\n";
                }
        }

        print_delimeter( "stop" , "I/O control", t);

        return t;
}
