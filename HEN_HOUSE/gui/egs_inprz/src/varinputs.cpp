/*
###############################################################################
#
#  EGSnrc egs_inprz variable inputs
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


#include "varinputs.h"
//Added by qt3to4:
//#include <Q3TextStream>
//qt3to4 -- BW
#include <QTextStream>

MVARInputs::MVARInputs()
{
 	BremsSplitting = "Off";
 	nBrems = 0;
	chargedPartRR = "Off";
 	eRangeRej = "Off";
 	ESAVEIN = "10000.";//<=== initially always on
 	RRDepth = "0.0";
 	RRFraction = "0.0";
 	ExpoTrafoC = "0.0";
 	PhotonForcing = "Off";
 	startForcing = 0;
 	stopForcing = 0;
 	CSEnhancement = 1;
 	CSEnhanStart.push_back( 1 );
 	CSEnhanStop.push_back( 1 );
 	nsplit = 1;
}

MVARInputs::~MVARInputs()
{
	//free anything created in the constructor here
}

std::ifstream & operator >> ( std::ifstream & in, MVARInputs*  rVAR )
{

  std::vector<string> codes;
  codes.push_back("BREM SPLITTING");
  codes.push_back("NUMBER OF BREMS PER EVENT");
  codes.push_back("CHARGED PARTICLE RUSSIAN ROULETTE");
  codes.push_back("ELECTRON RANGE REJECTION");
  codes.push_back("ESAVEIN");
  codes.push_back("RUSSIAN ROULETTE DEPTH");
  codes.push_back("RUSSIAN ROULETTE FRACTION");
  codes.push_back("EXPONENTIAL TRANSFORM C");
  codes.push_back("PHOTON FORCING");
  codes.push_back("START FORCING");
  codes.push_back("STOP FORCING AFTER");
  codes.push_back("CS ENHANCEMENT FACTOR");
  codes.push_back("CS ENHANCEMENT START REGION");
  codes.push_back("CS ENHANCEMENT STOP REGION");
  codes.push_back("PHOTON SPLITTING");


  DE_Parser *p = new DE_Parser(codes,0,"variance reduction", in, false);

  if ( ( rVAR->gusercode() == dosrznrc ) ||
       ( rVAR->gusercode() == flurznrc ) ){
     rVAR->BremsSplitting = getIt( codes[0] ,"Off", rVAR->errors, p );
     rVAR->nBrems         = p->get_input( codes[1], 0, 1000, 0 );
     //rVAR->nBrems         = getItsafe( codes[1], 0, 1000, -111, rVAR->errors, p );
     rVAR->chargedPartRR  = getIt( codes[2] ,"Off", rVAR->errors, p );
  }

  rVAR->eRangeRej      = getIt( codes[3] ,"Off", rVAR->errors, p );
  rVAR->ESAVEIN        = getIt( codes[4] ,"10000.0", rVAR->errors, p );

  if ( rVAR->gusercode() != flurznrc ) {
     rVAR->RRDepth        = getIt( codes[5] ,"0.0", rVAR->errors, p );
     rVAR->RRFraction     = getIt( codes[6] ,"0.0", rVAR->errors, p );
     rVAR->ExpoTrafoC     = getIt( codes[7] ,"0.0", rVAR->errors, p );
  }

  rVAR->PhotonForcing  = getIt( codes[8] ,"Off", rVAR->errors, p );
  rVAR->startForcing   = p->get_input(  codes[9], 0, 1000, 0 );
  //rVAR->startForcing   = getItsafe(  codes[9], 0, 1000, -111, rVAR->errors, p );
  rVAR->stopForcing    = p->get_input( codes[10], 0, 1000, 0 );
  //rVAR->stopForcing    = getItsafe( codes[10], 0, 1000, -111, rVAR->errors, p );

  if ( ( rVAR->gusercode() == dosrznrc ) ||
       ( rVAR->gusercode() == cavrznrc ) ) { // only implemented in these user codes
     rVAR->CSEnhancement  = p->get_input( codes[11], 1, 10000, 1);
     //rVAR->CSEnhancement  = getItsafe( codes[11], 1, 10000, -1, rVAR->errors, p );
     if ( rVAR->gusercode() == dosrznrc ){
         rVAR->CSEnhanStart   = getThem( codes[12], 1, 1000000, rVAR->CSEnhanStart, rVAR->errors, p );
         rVAR->CSEnhanStop    = getThem( codes[13], 1, 1000000, rVAR->CSEnhanStop, rVAR->errors, p );
     }
  }

  if ( rVAR->gusercode() == cavrznrc )
     rVAR->nsplit = p->get_input( codes[14], 1, 1000000, 1 );
     //rVAR->nsplit = getItsafe( codes[14], 1, 1000000, -111, rVAR->errors, p );

  if ( ! rVAR->errors.isEmpty() ) {
    rVAR->errors = "***  VARIANCE REDUCTION block *** <br>" +  rVAR->errors + "<br>";
  }

  delete p;

  return in;
}

/*
Q3TextStream & operator << ( Q3TextStream & t, MVARInputs * rVAR )
{

  print_delimeter( "start" , "variance reduction", t);

  if ( ( rVAR->gusercode() == dosrznrc ) ||
       ( rVAR->gusercode() == flurznrc ) ){
     t << "BREM SPLITTING= " << rVAR->BremsSplitting << "\n";
     t << "NUMBER OF BREMS PER EVENT= " << rVAR->nBrems << "\n";
     t << "CHARGED PARTICLE RUSSIAN ROULETTE= " << rVAR->chargedPartRR << "\n";
  }

  t << "ELECTRON RANGE REJECTION= " << rVAR->eRangeRej << "\n";
  t << "ESAVEIN= " << rVAR->ESAVEIN << "\n";

  if ( rVAR->gusercode() != flurznrc ) {
     t << "RUSSIAN ROULETTE DEPTH= " << rVAR->RRDepth << "\n";       // flurznrc wont use this
     t << "RUSSIAN ROULETTE FRACTION= " << rVAR->RRFraction << "\n"; // flurznrc wont use this
     t << "EXPONENTIAL TRANSFORM C= " << rVAR->ExpoTrafoC << "\n";   // flurznrc wont use this
  }

  t << "PHOTON FORCING= " << rVAR->PhotonForcing << "\n";
  t << "START FORCING= " << rVAR->startForcing << "\n";
  t << "STOP FORCING AFTER= " << rVAR->stopForcing << "\n";

  if ( ( rVAR->gusercode() == dosrznrc ) ||
       ( rVAR->gusercode() == cavrznrc ) ) { // only implemented in these user codes
     t << "CS ENHANCEMENT FACTOR= " << rVAR->CSEnhancement << "\n";
     if ( rVAR->gusercode() == dosrznrc ){
        t << "CS ENHANCEMENT START REGION= " << rVAR->CSEnhanStart << "\n";
        t << "CS ENHANCEMENT STOP REGION= " << rVAR->CSEnhanStop << "\n";
     }
  }

  if ( rVAR->gusercode() == cavrznrc )
     t << "PHOTON SPLITTING= " << rVAR->nsplit << "\n";

  print_delimeter( "stop" , "variance reduction", t);


  return t;
}
*/

//qt3to4 -- BW
QTextStream & operator << ( QTextStream & t, MVARInputs * rVAR )
{

  print_delimeter( "start" , "variance reduction", t);

  if ( ( rVAR->gusercode() == dosrznrc ) ||
       ( rVAR->gusercode() == flurznrc ) ){
     t << "BREM SPLITTING= " << rVAR->BremsSplitting << "\n";
     t << "NUMBER OF BREMS PER EVENT= " << rVAR->nBrems << "\n";
     t << "CHARGED PARTICLE RUSSIAN ROULETTE= " << rVAR->chargedPartRR << "\n";
  }

  t << "ELECTRON RANGE REJECTION= " << rVAR->eRangeRej << "\n";
  t << "ESAVEIN= " << rVAR->ESAVEIN << "\n";

  if ( rVAR->gusercode() != flurznrc ) {
     t << "RUSSIAN ROULETTE DEPTH= " << rVAR->RRDepth << "\n";       // flurznrc wont use this
     t << "RUSSIAN ROULETTE FRACTION= " << rVAR->RRFraction << "\n"; // flurznrc wont use this
     t << "EXPONENTIAL TRANSFORM C= " << rVAR->ExpoTrafoC << "\n";   // flurznrc wont use this
  }

  t << "PHOTON FORCING= " << rVAR->PhotonForcing << "\n";
  t << "START FORCING= " << rVAR->startForcing << "\n";
  t << "STOP FORCING AFTER= " << rVAR->stopForcing << "\n";

  if ( ( rVAR->gusercode() == dosrznrc ) ||
       ( rVAR->gusercode() == cavrznrc ) ) { // only implemented in these user codes
     t << "CS ENHANCEMENT FACTOR= " << rVAR->CSEnhancement << "\n";
     if ( rVAR->gusercode() == dosrznrc ){
        t << "CS ENHANCEMENT START REGION= " << rVAR->CSEnhanStart << "\n";
        t << "CS ENHANCEMENT STOP REGION= " << rVAR->CSEnhanStop << "\n";
     }
  }

  if ( rVAR->gusercode() == cavrznrc )
     t << "PHOTON SPLITTING= " << rVAR->nsplit << "\n";

  print_delimeter( "stop" , "variance reduction", t);


  return t;
}

