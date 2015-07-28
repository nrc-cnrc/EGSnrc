/*
###############################################################################
#
#  EGSnrc egs_inprz geometry input
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


#include "geoinputs.h"
#include <qmessagebox.h>
//Added by qt3to4:
//#include <Q3TextStream>
#include <exception>

//qt3to4 -- BW
#include <QTextStream>

MGEOInputs::MGEOInputs()
{

	inp_meth  = "groups";

	zface = "0";

	nslab.push_back( 1 );
	slabs.push_back( 1 );

	radii.push_back( 1 );
	media.push_back( "VACUUM" ); //<=== zero element of media list
	media.push_back( "H2O521ICRU" );
	description_by = "regions";
	mednum.push_back( 1 );

	start_reg_slab.push_back( 2 );
	stop_reg_slab.push_back( 2 );

	start_ring.push_back( 1 );
	stop_ring.push_back( 1 );

}

MGEOInputs::~MGEOInputs()
{
	//free anything created in the constructor here
}

QString MGEOInputs::getErrors()
{
        return errors;
}


std::ifstream & operator >> ( std::ifstream & in, MGEOInputs*  rGEO )
{

 std::vector<string> codes;
 codes.push_back("METHOD OF INPUT");
 codes.push_back("Z OF FRONT FACE");
 codes.push_back("NSLAB");
 codes.push_back("SLAB THICKNESS");
 codes.push_back("DEPTH BOUNDARIES");
 codes.push_back("RADII");
 codes.push_back("MEDIA");
 codes.push_back("DESCRIPTION BY");
 codes.push_back("MEDNUM");
 codes.push_back("START REGION");
 codes.push_back("STOP REGION");
 codes.push_back("START ZSLAB");
 codes.push_back("STOP ZSLAB");
 codes.push_back("START RING");
 codes.push_back("STOP RING");

 DE_Parser *p = new DE_Parser(codes,0,"geometrical inputs", in, false);


 rGEO->inp_meth = getIt( codes[0] , "individual" , rGEO->errors, p ) ;

 if (rGEO->inp_meth.toLower() != "cavity information") {
   rGEO->zface = getIt( codes[1] , "0.0"     , rGEO->errors, p ) ;

   if ( rGEO->inp_meth.toLower() == "groups") {
      rGEO->nslab = getThem( codes[2] ,0  ,1000   ,rGEO->nslab  ,rGEO->errors, p ) ;
      rGEO->slabs = getThem( codes[3] , 0.0f, 100000.f, rGEO->slabs, rGEO->errors, p ) ;
   }
   else {
      rGEO->slabs = getThem( codes[4] , -100000.0f, 100000.f, rGEO->slabs, rGEO->errors, p ) ;
      rGEO->nslab.clear();
      for (unsigned int i = 0; i < rGEO->slabs.size(); i++) {
          rGEO->nslab.push_back( 1 );
      }
   }

   rGEO->radii = getThem( codes[5] , 0.0f, 100000.f, rGEO->radii, rGEO->errors, p ) ;

   rGEO->media = getThemAll( codes[6], rGEO->media, rGEO->errors, p ) ;
   rGEO->media.insert( rGEO->media.begin(), "VACUUM");//<=== zero element in media list
   rGEO->description_by = getIt( codes[7] , "regions", rGEO->errors, p ) ;

   //this is to catch the exception of no medium set for any region
   rGEO->start_reg_slab.clear();rGEO->start_reg_slab.push_back( 0 );
   rGEO->stop_reg_slab.clear();rGEO->stop_reg_slab.push_back( 0 );

   if ( rGEO->description_by.toLower() == "regions"){
      rGEO->mednum         = getThem( codes[ 8], 0, 1000, rGEO->mednum, rGEO->errors, p ) ;
      rGEO->start_reg_slab = getThem( codes[ 9], 0, 1000, rGEO->start_reg_slab, rGEO->errors, p ) ;
      rGEO->stop_reg_slab  = getThem( codes[10], 0, 1000, rGEO->stop_reg_slab, rGEO->errors, p ) ;
   }
  else {
      rGEO->mednum         = getThem( codes[ 8], 0, 1000, rGEO->mednum, rGEO->errors, p ) ;
      rGEO->start_reg_slab = getThem( codes[11], 0, 1000, rGEO->start_reg_slab, rGEO->errors, p ) ;
      rGEO->stop_reg_slab  = getThem( codes[12], 0, 1000, rGEO->stop_reg_slab, rGEO->errors, p ) ;
      rGEO->start_ring     = getThem( codes[13], 0, 1000, rGEO->start_ring, rGEO->errors, p ) ;
      rGEO->stop_ring      = getThem( codes[14], 0, 1000, rGEO->stop_ring, rGEO->errors, p ) ;
      if ( rGEO->start_ring.size() != rGEO->stop_ring.size() )
           rGEO->errors += "Wrong region assignment, start-stop ring region mismatch. <br>";
      if (  rGEO->start_reg_slab.size() != rGEO->start_ring.size() )
             rGEO->errors += "Same number of inputs for planes and rings needed. <br>";
   }
 }

 if (rGEO->start_reg_slab.size() != rGEO->stop_reg_slab.size() )
    rGEO->errors += "Wrong region assignment, start-stop region mismatch. <br>";

 if (rGEO->start_reg_slab.size() > rGEO->mednum.size() )
    rGEO->errors += "Wrong medium-region assignment. <br>";

// ***********************************************************************************
//              CHECKING WHETHER USER WANTS ALL REGIONS TO BE OF MEDIUM 1
//	OR IF THERE IS ANY ZERO REGION, WHICH SHOULDN'T HAPPEN
// ***********************************************************************************
bool ZeroRegionPresent = false;

for (unsigned int j = 0; j < rGEO->start_reg_slab.size(); j++) {
  if ( rGEO->start_reg_slab[j] == 0 || rGEO->stop_reg_slab[j] == 0)
     {ZeroRegionPresent = true; break;}
}

if ( ZeroRegionPresent ) {// there shouldn't be a zero region
     int n_slab = 0;              // set all regions to the first region
     for (unsigned int j = 0; j < rGEO->nslab.size(); j++) {
          n_slab += rGEO->nslab[j];
      }

      int n_regions = n_slab * rGEO->radii.size() + 1;

      rGEO->start_reg_slab.clear(); rGEO->start_reg_slab.push_back( 2 );
      rGEO->stop_reg_slab.clear(); rGEO->stop_reg_slab.push_back( n_regions );
      rGEO->mednum.clear(); rGEO->mednum.push_back( 1 );
}

// ***********************************************************************************


 if ( !rGEO->errors.isEmpty() ) {
   rGEO->errors = "***  GEOMETRY Inputs block *** <br>" +  rGEO->errors + "<br>";
 }

 delete p;

 return in;
}

/*
Q3TextStream & operator << ( Q3TextStream & t, MGEOInputs * rGEO )
{

  print_delimeter( "start" , "geometrical inputs", t);

	t << "METHOD OF INPUT= " << rGEO->inp_meth << "\n";
  if (rGEO->inp_meth.toLower() != "cavity information") {
	t << "Z OF FRONT FACE= " << rGEO->zface    << "\n";

	if ( rGEO->inp_meth.toLower() == "groups" ) {
	   t << "NSLAB= "          << rGEO->nslab    << "\n";
	   t << "SLAB THICKNESS= " << rGEO->slabs    << "\n";
	}
	else if( rGEO->inp_meth.toLower() == "individual" ) {
	   t << "DEPTH BOUNDARIES= " << rGEO->slabs    << "\n";
	}

	t << "RADII= "          << rGEO->radii << "\n";

	int vacuumPos = delete_element( &(rGEO->media), string("VACUUM") );
	     vacuumPos++;

	t << "MEDIA= "          << rGEO->media << "\n";

	t << "DESCRIPTION BY= " << rGEO->description_by << "\n";

	std::vector<int>::iterator  iternum( rGEO->mednum.begin() );
	while( iternum <  rGEO->mednum.end() ) {
             (*iternum)++;
             if ( *iternum == vacuumPos ) // set medium number to zero
 		  *iternum = 0;
	     else if( *iternum > vacuumPos ) // decrease medium number by one
		  *iternum -= 1 ;
	     iternum++;
	}

	t << "MEDNUM= "         << rGEO->mednum << "\n";

	if ( rGEO->description_by.toLower() == "regions" ) {
	   t << "START REGION= " << rGEO->start_reg_slab << "\n";
	   t << "STOP REGION= "  << rGEO->stop_reg_slab  << "\n";

	}
	else {
	   t << "START ZSLAB= " << rGEO->start_reg_slab << "\n";
	   t << "STOP ZSLAB= "  << rGEO->stop_reg_slab  << "\n";
	   t << "START RING= "  << rGEO->start_ring << "\n";
	   t << "STOP RING= "   << rGEO->stop_ring  << "\n";

	}
   }
  print_delimeter( "stop" , "geometrical inputs", t);

  return t;

}
*/

//qt3to4 -- BW

QTextStream & operator << ( QTextStream & t, MGEOInputs * rGEO )
{

  print_delimeter( "start" , "geometrical inputs", t);

        t << "METHOD OF INPUT= " << rGEO->inp_meth << "\n";
  if (rGEO->inp_meth.toLower() != "cavity information") {
        t << "Z OF FRONT FACE= " << rGEO->zface    << "\n";

        if ( rGEO->inp_meth.toLower() == "groups" ) {
           t << "NSLAB= "          << rGEO->nslab    << "\n";
           t << "SLAB THICKNESS= " << rGEO->slabs    << "\n";
        }
        else if( rGEO->inp_meth.toLower() == "individual" ) {
           t << "DEPTH BOUNDARIES= " << rGEO->slabs    << "\n";
        }

        t << "RADII= "          << rGEO->radii << "\n";

        int vacuumPos = delete_element( &(rGEO->media), string("VACUUM") );
             vacuumPos++;

        t << "MEDIA= "          << rGEO->media << "\n";

        t << "DESCRIPTION BY= " << rGEO->description_by << "\n";

        std::vector<int>::iterator  iternum( rGEO->mednum.begin() );
        while( iternum <  rGEO->mednum.end() ) {
             (*iternum)++;
             if ( *iternum == vacuumPos ) // set medium number to zero
                  *iternum = 0;
             else if( *iternum > vacuumPos ) // decrease medium number by one
                  *iternum -= 1 ;
             iternum++;
        }

        t << "MEDNUM= "         << rGEO->mednum << "\n";

        if ( rGEO->description_by.toLower() == "regions" ) {
           t << "START REGION= " << rGEO->start_reg_slab << "\n";
           t << "STOP REGION= "  << rGEO->stop_reg_slab  << "\n";

        }
        else {
           t << "START ZSLAB= " << rGEO->start_reg_slab << "\n";
           t << "STOP ZSLAB= "  << rGEO->stop_reg_slab  << "\n";
           t << "START RING= "  << rGEO->start_ring << "\n";
           t << "STOP RING= "   << rGEO->stop_ring  << "\n";

        }
   }
  print_delimeter( "stop" , "geometrical inputs", t);

  return t;

}





