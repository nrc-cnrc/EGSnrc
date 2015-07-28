/*
###############################################################################
#
#  EGSnrc egs_inprz source inputs
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


#include "srcinputs.h"
//Added by qt3to4:
//#include <Q3TextStream>
//qt3to4 -- BW
#include <QTextStream>

MSRCInputs::MSRCInputs()
{
	iniparticle = "photon";
	isource     = "0";

	for ( int i = 0; i < 7; i++) srcopt.push_back( 0 );

	typenergy   = "monoenergetic";
	inienergy   = "1.25";
	modein      = "Local";
	spe_file    = "co60.spectrum";
	dist_file   = "dummy.file";
	phsp_file   = "dummy.file";
	speciout    = "include";
	rdistiout   = "Include";
	rdistf.push_back( 1. );
	rpdf.push_back( 1. );
	weight_win.push_back( -1.E30 );
	weight_win.push_back(  1.E30 );
        beam_code = "dummy";
        inp_file  = "dummy";
        pegs_file = "dummy";
}

MSRCInputs::~MSRCInputs()
{
	//free anything created in the constructor here
}

std::ifstream & operator >> ( std::ifstream & in, MSRCInputs*  rSRC )
{

	std::vector<string> codes;
	codes.push_back("INCIDENT PARTICLE");                //         #0
	codes.push_back("SOURCE NUMBER");                    //         #1
	codes.push_back("SOURCE OPTIONS");                   //         #2

	codes.push_back("MODEIN"); // only if ISOURC = 20               #3
	codes.push_back("NRDIST");        // ibdem                      #4
	codes.push_back("RDISTF");        // ibdem                      #5
	codes.push_back("RPDF");          // ibdem                      #6
	codes.push_back("RDIST IOUTSP");  // ibdem                      #7
	codes.push_back("RDIST FILENAME");// ibdem if MODEIN = external #8

	codes.push_back("FILSPC"); // only if ISOURC = 21 or 22         #9

	codes.push_back("INCIDENT ENERGY"); //monoenergetic or spectrum #10
	codes.push_back(
        "INCIDENT KINETIC ENERGY(MEV)");    //if monoenergetic #11
	codes.push_back("SPEC FILENAME");   // if spectrum     #12
	codes.push_back("SPEC IOUTSP");     // if spectrum     #13

	codes.push_back("BEAM CODE");     // if ISOURC = 23  #14
	codes.push_back("PEGS FILE");     // ibdem           #15
	codes.push_back("INPUT FILE");    // ibdem           #16
	codes.push_back("WEIGHT WINDOW"); // ibdem           #17

	DE_Parser *p = new DE_Parser(codes,0,"source inputs", in, false);

	rSRC->iniparticle = getIt( codes[0] , "photon", rSRC->errors, p );
	rSRC->isource     = getIt( codes[1] , "0"     , rSRC->errors, p );
	if ( rSRC->isource != "20" ) {
	   rSRC->srcopt = getThem( codes[2], -1000000.f, 1000000.f,
                                   rSRC->srcopt, rSRC->errors, p ) ;
	}

	if ( rSRC->isource == "20" ) {
    	     rSRC->modein = getIt( codes[3] , "Local" , rSRC->errors, p );

	     if ( rSRC->modein.toLower() == "external" ) {
       	   	  rSRC->dist_file = getIt( codes[8] , "dummy.file",
                                           rSRC->errors, p );
	     }
	     else {
	  	  rSRC->nrdist = getIt( codes[4], "0" , rSRC->errors, p );
	  	  rSRC->rdistf = getThem( codes[5], 0.f, 1000000.f,
                                          rSRC->rdistf, rSRC->errors, p ) ;
	  	  rSRC->rpdf   = getThem( codes[6], 0.f, 1000000.f,
                                          rSRC->rpdf, rSRC->errors, p ) ;
	     }
    	     rSRC->rdistiout = getIt( codes[7], "Include", rSRC->errors, p );
	}

        if ( ( rSRC->isource == "21" ) || ( rSRC->isource == "22" ) ) {
    		rSRC->phsp_file = getIt( codes[9] , "dummy.file",
                                         rSRC->errors, p );
	}
        else if ( rSRC->isource == "23" ) {
                  rSRC->beam_code = getIt( codes[14] , QString::null,
                                           rSRC->errors, p );
                  // no extension allowed in the input and pegs file names
                  rSRC->pegs_file = (getIt( codes[15] , QString::null,
                           rSRC->errors, p )).remove(".pegs4dat");
                  rSRC->inp_file = (getIt( codes[16] , QString::null,
                             rSRC->errors, p )).remove(".egsinp");
	          rSRC->weight_win = getThem( codes[17], -1.0e30f, 1.0e30f,
                                   rSRC->weight_win, rSRC->errors, p ) ;
        }
	else {
    		rSRC->typenergy = getIt( codes[10] , "monoenergetic" ,
                                         rSRC->errors, p );
    		if ( rSRC->typenergy.toLower() == "monoenergetic" ) {
             		rSRC->inienergy = getIt( codes[11] , "1.25",
                                                 rSRC->errors, p );
	  	}
		else {
             		rSRC->spe_file = getIt( codes[12] , "dummy.file",
                                                rSRC->errors, p );
             		rSRC->speciout = getIt( codes[13], "include",
                                                rSRC->errors, p );
	  	}

	}

	if ( ! rSRC->errors.isEmpty() ) {
	    rSRC->errors = "***  SOURCE Inputs block *** <br>" +
                           rSRC->errors + "<br>";
	}

	delete p;

	return in;
}

/*
Q3TextStream & operator << ( Q3TextStream & t, MSRCInputs * rSRC )
{

  	print_delimeter( "start" , "source inputs", t);

	t << "INCIDENT PARTICLE= " << rSRC->iniparticle << "\n";
	t << "SOURCE NUMBER= "     << rSRC->isource << "\n";
	if ( rSRC->isource != "20" ) {
	   t << "SOURCE OPTIONS= " << rSRC->srcopt << "\n";
	}

	if ( rSRC->isource == "20" ) {

     	   t << "MODEIN= " << rSRC->modein << "\n";

	   if ( rSRC->modein.toLower() == "external" ) {
                t << "RDIST FILENAME= " << rSRC->dist_file << "\n";
	   }
	   else {
	     t << "NRDIST= " << rSRC->nrdist << "\n";
	     t << "RDISTF= " << rSRC->rdistf << "\n";
	     t << "RPDF= "   <<  rSRC->rpdf << "\n";
	   }

    	   t << "RDIST IOUTSP= " << rSRC->rdistiout << "\n";

	}

	if ( ( rSRC->isource == "21" ) || ( rSRC->isource == "22" ) ) {
       	       t << "FILSPC= " << rSRC->phsp_file << "\n";
        }
        else if ( rSRC->isource == "23" ){
         // no extension allowed in the input and pegs file names
	 t << "BEAM CODE=  " << rSRC->beam_code << "\n";
       	 t << "INPUT FILE= " << (rSRC->inp_file).remove("egsinp") << "\n";
       	 t << "PEGS FILE=  " <<(rSRC->pegs_file).remove(".pegs4dat")<<"\n";
       	 t << "WEIGHT WINDOW= " << rSRC->weight_win << "\n";
        }
	else {
    	       t << "INCIDENT ENERGY= " << rSRC->typenergy << "\n";
    	       if ( rSRC->typenergy.toLower() == "monoenergetic" ) {
                    t << "INCIDENT KINETIC ENERGY(MEV)= " <<
                         rSRC->inienergy << "\n";
	       }
	       else {
                    t << "SPEC FILENAME= " << rSRC->spe_file << "\n";
                    t << "SPEC IOUTSP= " << rSRC->speciout << "\n";
	       }

	}

	print_delimeter( "stop" , "source inputs", t);

	return t;
}
*/

//qt3to4 -- BW
QTextStream & operator << ( QTextStream & t, MSRCInputs * rSRC )
{

        print_delimeter( "start" , "source inputs", t);

        t << "INCIDENT PARTICLE= " << rSRC->iniparticle << "\n";
        t << "SOURCE NUMBER= "     << rSRC->isource << "\n";
        if ( rSRC->isource != "20" ) {
           t << "SOURCE OPTIONS= " << rSRC->srcopt << "\n";
        }

        if ( rSRC->isource == "20" ) {

           t << "MODEIN= " << rSRC->modein << "\n";

           if ( rSRC->modein.toLower() == "external" ) {
                t << "RDIST FILENAME= " << rSRC->dist_file << "\n";
           }
           else {
             t << "NRDIST= " << rSRC->nrdist << "\n";
             t << "RDISTF= " << rSRC->rdistf << "\n";
             t << "RPDF= "   <<  rSRC->rpdf << "\n";
           }

           t << "RDIST IOUTSP= " << rSRC->rdistiout << "\n";

        }

        if ( ( rSRC->isource == "21" ) || ( rSRC->isource == "22" ) ) {
               t << "FILSPC= " << rSRC->phsp_file << "\n";
        }
        else if ( rSRC->isource == "23" ){
         // no extension allowed in the input and pegs file names
         t << "BEAM CODE=  " << rSRC->beam_code << "\n";
         t << "INPUT FILE= " << (rSRC->inp_file).remove("egsinp") << "\n";
         t << "PEGS FILE=  " <<(rSRC->pegs_file).remove(".pegs4dat")<<"\n";
         t << "WEIGHT WINDOW= " << rSRC->weight_win << "\n";
        }
        else {
               t << "INCIDENT ENERGY= " << rSRC->typenergy << "\n";
               if ( rSRC->typenergy.toLower() == "monoenergetic" ) {
                    t << "INCIDENT KINETIC ENERGY(MEV)= " <<
                         rSRC->inienergy << "\n";
               }
               else {
                    t << "SPEC FILENAME= " << rSRC->spe_file << "\n";
                    t << "SPEC IOUTSP= " << rSRC->speciout << "\n";
               }

        }

        print_delimeter( "stop" , "source inputs", t);

        return t;
}





