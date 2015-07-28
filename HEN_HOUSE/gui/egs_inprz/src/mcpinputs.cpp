/*
###############################################################################
#
#  EGSnrc egs_inprz Monte Carlo parameters inputs
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


#include "mcpinputs.h"
//Added by qt3to4:
//#include <Q3TextStream>
//qt3to4 -- BW
#include <QTextStream>

MMCPInputs::MMCPInputs()
{
  SetPCUTbyRegion = false;
  SetECUTbyRegion = false;
  SetSMAXbyRegion = false;


  ECUT.push_back( 0.0 );
  PCUT.push_back( 0.0 );
  SMAX.push_back( 0.0 );

  PCUTstart.push_back( 1 );
  PCUTstop.push_back( 1 );

  ECUTstart.push_back( 1 );
  ECUTstop.push_back( 1 );

  SMAXstart.push_back( 1 );
  SMAXstop.push_back( 1 );


  GECUT = "0.0";// force EGSnrc to use AP
  GPCUT = "0.0";// force EGSnrc to use AE
  GSMAX = "0.5";

  ESTEPE        = "0.25";
  XImax         = "0.50";
  SkinD         = "3.0";
  PairSampling  = "Simple";
  BremsSampling = "KM";
  BremsXSection = "BH";
  ESTEPAlg      = "PRESTA-II";
  BoundXAlg     = "EXACT";
  EIIXSection   = "Off";
  PhotXSection  = "si"; //defaults to Storm&Israel
  PhotXSectionOut = "Off";

  Spin  = "On";
  BC    = "On";
  PE    = "On";
  RAY   = "Off";
  RELAX = "On";

  BCstart.push_back( 1 );
  BCstop.push_back( 1 );
  PEstart.push_back( 1 );
  PEstop.push_back( 1 );
  RAYstart.push_back( 1 );
  RAYstop.push_back( 1 );
  RELAXstart.push_back( 1 );
  RELAXstop.push_back( 1 );

  ff_media.push_back("");
  ff_files.push_back("");

}

MMCPInputs::~MMCPInputs()
{
	//free anything created in the constructor here
}


std::ifstream & operator >> ( std::ifstream & in, MMCPInputs*  rMCP )
{

  std::vector<string> codes;
  codes.push_back("Global ECUT");
  codes.push_back("Global PCUT");
  codes.push_back("Global SMAX");
  codes.push_back("ESTEPE");
  codes.push_back("XImax");
  codes.push_back("Skin depth for BCA");
  codes.push_back("Boundary crossing algorithm");
  codes.push_back("Electron-step algorithm");
  codes.push_back("Spin effects");
  codes.push_back("Brems angular sampling");
  codes.push_back("Brems cross sections");

  codes.push_back("Bound Compton scattering");
  codes.push_back("Bound Compton start region");
  codes.push_back("Bound Compton stop region");

  codes.push_back("Pair angular sampling");

  codes.push_back("Photoelectron angular sampling");
  codes.push_back("PE sampling start region");
  codes.push_back("PE sampling stop region");

  codes.push_back("Rayleigh scattering");
  codes.push_back("Rayleigh start region");
  codes.push_back("Rayleigh stop region");

  codes.push_back("Atomic relaxations");
  codes.push_back("Relaxations start region");
  codes.push_back("Relaxations stop region");

  codes.push_back("Set PCUT");
  codes.push_back("Set PCUT start region");
  codes.push_back("Set PCUT stop region");

  codes.push_back("Set ECUT");
  codes.push_back("Set ECUT start region");
  codes.push_back("Set ECUT stop region");

  codes.push_back("Set SMAX");
  codes.push_back("Set SMAX start region");
  codes.push_back("Set SMAX stop region");

  codes.push_back("Photon cross sections");
  codes.push_back("Electron Impact Ionization");
  codes.push_back("Photon cross-sections output");
  codes.push_back("ff media names");
  codes.push_back("ff file names");

  DE_Parser *p = new DE_Parser(codes,0,"mc transport parameter", in, false);

  rMCP->GECUT         = getIt( codes[0] , "0.521", rMCP->errors, p );
  rMCP->GPCUT         = getIt( codes[1] , "0.001", rMCP->errors, p );
  rMCP->GSMAX         = getIt( codes[2] ,"0.5", rMCP->errors, p );
  rMCP->ESTEPE        = getIt( codes[3] ,"0.25", rMCP->errors, p );
  rMCP->XImax         = getIt( codes[4] ,"0.5", rMCP->errors, p );
  rMCP->SkinD         = getIt( codes[5] ,"3.0", rMCP->errors, p );
  rMCP->BoundXAlg     = getIt( codes[6] ,"EXACT", rMCP->errors, p );
  rMCP->ESTEPAlg      = getIt( codes[7] ,"PRESTA-II", rMCP->errors, p );
  rMCP->Spin          = getIt( codes[8] ,"On", rMCP->errors, p );
  rMCP->BremsSampling = getIt( codes[9] ,"KM", rMCP->errors, p );
  rMCP->BremsXSection = getIt( codes[10] ,"BH", rMCP->errors, p );
  rMCP->PhotXSection  = getIt( codes[33] ,"si", rMCP->errors, p );
  rMCP->EIIXSection   = getIt( codes[34] ,"off", rMCP->errors, p );
  rMCP->PhotXSectionOut= getIt( codes[35] ,"Off", rMCP->errors, p );

  rMCP->BC = getIt( codes[11] ,"On", rMCP->errors, p );
  if ( ( rMCP->BC.toLower() == "on in regions"  ) ||
       ( rMCP->BC.toLower() == "off in regions" ) ){
     rMCP->BCstart = getThem( codes[12], 0, 1000000,
                     rMCP->BCstart, rMCP->errors, p ) ;
     rMCP->BCstop  = getThem( codes[13], 0, 1000000,
                     rMCP->BCstop, rMCP->errors, p ) ;
  }

  rMCP->PairSampling  = getIt( codes[14] ,"Simple", rMCP->errors, p );

  rMCP->PE = getIt( codes[15] ,"On", rMCP->errors, p );
  if ( ( rMCP->PE.toLower() == "on in regions"  ) ||
       ( rMCP->PE.toLower() == "off in regions" ) ) {
     rMCP->PEstart = getThem( codes[16], 0, 1000000,
                     rMCP->PEstart, rMCP->errors, p ) ;
     rMCP->PEstop  = getThem( codes[17], 0, 1000000,
                     rMCP->PEstop, rMCP->errors, p ) ;
  }

  /* Rayleigh scattering input */
  rMCP->RAY = getIt( codes[18] ,"Off", rMCP->errors, p );
  if ( ( rMCP->RAY.toLower() == "on in regions"  ) ||
       ( rMCP->RAY.toLower() == "off in regions" ) ) {
     rMCP->RAYstart  = getThem( codes[19], 0, 1000000,
                       rMCP->RAYstart, rMCP->errors, p ) ;
     rMCP->RAYstop  = getThem( codes[20], 0, 1000000,
                      rMCP->RAYstop, rMCP->errors, p ) ;
  }
  else if (rMCP->RAY.toLower() == "custom"){
     rMCP->ff_media = getThemAll( codes[36], rMCP->ff_media,
                                  rMCP->errors, p );
     rMCP->ff_files = getThemAll( codes[37], rMCP->ff_files,
                                  rMCP->errors, p );
  }

  /* Atomic relaxations input */
  rMCP->RELAX = getIt( codes[21] ,"On", rMCP->errors, p );
  if ( ( rMCP->RELAX.toLower() == "on in regions"  ) ||
       ( rMCP->RELAX.toLower() == "off in regions" ) ) {
     rMCP->RELAXstart = getThem( codes[22], 0, 1000000,
                        rMCP->RELAXstart, rMCP->errors, p ) ;
     rMCP->RELAXstop  = getThem( codes[23], 0, 1000000,
                        rMCP->RELAXstop, rMCP->errors, p ) ;
  }

  v_float tmpFloat;
  rMCP->SetPCUTbyRegion = !p->get_input( codes[24], tmpFloat );
  if ( rMCP->SetPCUTbyRegion ) {
      rMCP->PCUT      = tmpFloat;
      rMCP->PCUTstart = getThem( codes[25], 0, 1000000,
                        rMCP->PCUTstart, rMCP->errors, p );
      rMCP->PCUTstop  = getThem( codes[26], 0, 1000000,
                        rMCP->PCUTstop, rMCP->errors, p );
      if ( rMCP->PCUTstart[0] < 2 &&
           rMCP->PCUTstop[0]  < 2 &&
           rMCP->PCUTstart.size() == 1)
	  rMCP->SetPCUTbyRegion = false;
  }

  tmpFloat.clear();
  rMCP->SetECUTbyRegion = !p->get_input( codes[27], tmpFloat );
  if ( rMCP->SetECUTbyRegion ) {
      rMCP->ECUT      = tmpFloat;
      rMCP->ECUTstart = getThem( codes[28], 0, 1000000,
                        rMCP->ECUTstart, rMCP->errors, p );
      rMCP->ECUTstop  = getThem( codes[29], 0, 1000000,
                        rMCP->ECUTstop, rMCP->errors, p );
      if ( rMCP->ECUTstart[0] < 2 &&
           rMCP->ECUTstop[0]  < 2 &&
           rMCP->ECUTstart.size() == 1)
	  rMCP->SetECUTbyRegion = false;
  }

  tmpFloat.clear();
  rMCP->SetSMAXbyRegion = !p->get_input( codes[30], tmpFloat );
  if ( rMCP->SetSMAXbyRegion ) {
      rMCP->SMAX      = tmpFloat;
      rMCP->SMAXstart = getThem( codes[31], 0, 1000000,
                        rMCP->SMAXstart, rMCP->errors, p );
      rMCP->SMAXstop  = getThem( codes[32], 0, 1000000,
                        rMCP->SMAXstop, rMCP->errors, p );
      if ( rMCP->SMAXstart[0] < 2 &&
           rMCP->SMAXstop[0]  < 2 &&
           rMCP->SMAXstart.size() == 1)
	  rMCP->SetSMAXbyRegion = false;
    }
  /*
  // If any of these inputs are missing, defaults are used
  // Disabling error reporting...
  if ( ! rMCP->errors.isEmpty() ) {
    rMCP->errors = "***  MC transport parameters block *** <br>" +
                   rMCP->errors + "<br>";
  }
  */
  rMCP->errors = QString::null;
  delete p;

  return in;
}

/*
Q3TextStream & operator << ( Q3TextStream & t, MMCPInputs * rMCP )
{

  print_delimeter( "start" , "MC transport parameter", t);

  t << "Global ECUT= " << rMCP->GECUT << "\n";
  t << "Global PCUT= " << rMCP->GPCUT << "\n";
  t << "Global SMAX= " << rMCP->GSMAX << "\n";
  t << "ESTEPE= " << rMCP->ESTEPE << "\n" ;
  t << "XImax= " << rMCP->XImax << "\n";
  t << "Skin depth for BCA= " << rMCP->SkinD << "\n";
  t << "Boundary crossing algorithm= " << rMCP->BoundXAlg << "\n";
  t << "Electron-step algorithm= " << rMCP->ESTEPAlg << "\n";
  t << "Spin effects= " << rMCP->Spin << "\n";
  t << "Brems angular sampling= " << rMCP->BremsSampling << "\n";
  t << "Brems cross sections= " << rMCP->BremsXSection << "\n";

  //if (rMCP->PhotXSection!="PEGS4"){
      t << "Photon cross sections= " << rMCP->PhotXSection << "\n";
  //}
  t << "Electron Impact Ionization= " << rMCP->EIIXSection << "\n";
  t << "Photon cross-sections output= " << rMCP->PhotXSectionOut << "\n";

  t << "Bound Compton scattering= " << rMCP->BC << "\n";
  if ( ( rMCP->BC.toLower() == "on in regions" ) ||
       ( rMCP->BC.toLower() == "off in regions" ) )
  {
     t << "Bound Compton start region= " << rMCP->BCstart << "\n";
     t << "Bound Compton stop region= " << rMCP->BCstop << "\n";
  }

  t << "Pair angular sampling= " << rMCP->PairSampling << "\n";

  t << "Photoelectron angular sampling= " << rMCP->PE << "\n";
  if ( ( rMCP->PE.toLower() == "on in regions" ) ||
       ( rMCP->PE.toLower() == "off in regions" ) )
  {
     t << "PE sampling start region= " << rMCP->PEstart << "\n";
     t << "PE sampling stop region= "  << rMCP->PEstop  << "\n";
  }

  // Rayleigh scattering output
  t << "Rayleigh scattering= " << rMCP->RAY << "\n";
  if ( ( rMCP->RAY.toLower() == "on in regions" ) ||
       ( rMCP->RAY.toLower() == "off in regions" ) )
  {
     t << "Rayleigh start region= " << rMCP->RAYstart << "\n";
     t << "Rayleigh stop region= "  << rMCP->RAYstop  << "\n";
  }
  else if (rMCP->RAY.toLower() == "custom"){
     t << "ff media names= " << rMCP->ff_media << "\n";
     t << "ff file names= " << rMCP->ff_files << "\n";
  }

  t << "Atomic relaxations= " << rMCP->RELAX << "\n";
  if ( ( rMCP->RELAX.toLower() == "on in regions" ) ||
       ( rMCP->RELAX.toLower() == "off in regions" ) )
  {
     t << "Relaxations start region= " << rMCP->RELAXstart << "\n";
     t << "Relaxations stop region= "  << rMCP->RELAXstop  << "\n";
  }

//  if ( rMCP->SetPCUTbyRegion ) {
     t << "Set PCUT= " << rMCP->PCUT << "\n" ;
     t << "Set PCUT start region= " << rMCP->PCUTstart << "\n";
     t << "Set PCUT stop region= " << rMCP->PCUTstop << "\n";
//  }

//  if ( rMCP->SetECUTbyRegion ) {
     t << "Set ECUT= " << rMCP->ECUT << "\n" ;
     t << "Set ECUT start region= " << rMCP->ECUTstart << "\n";
     t << "Set ECUT stop region= " << rMCP->ECUTstop << "\n";
//  }

//  if ( rMCP->SetSMAXbyRegion ) {
     t << "Set SMAX= " << rMCP->SMAX << "\n" ;
     t << "Set SMAX start region= " << rMCP->SMAXstart << "\n";
     t << "Set SMAX stop region= " << rMCP->SMAXstop << "\n";
//  }

  print_delimeter( "stop" , "MC transport parameter", t);

  return t;
}
*/

//qt3to4 -- BW
QTextStream & operator << ( QTextStream & t, MMCPInputs * rMCP )
{

  print_delimeter( "start" , "MC transport parameter", t);

  t << "Global ECUT= " << rMCP->GECUT << "\n";
  t << "Global PCUT= " << rMCP->GPCUT << "\n";
  t << "Global SMAX= " << rMCP->GSMAX << "\n";
  t << "ESTEPE= " << rMCP->ESTEPE << "\n" ;
  t << "XImax= " << rMCP->XImax << "\n";
  t << "Skin depth for BCA= " << rMCP->SkinD << "\n";
  t << "Boundary crossing algorithm= " << rMCP->BoundXAlg << "\n";
  t << "Electron-step algorithm= " << rMCP->ESTEPAlg << "\n";
  t << "Spin effects= " << rMCP->Spin << "\n";
  t << "Brems angular sampling= " << rMCP->BremsSampling << "\n";
  t << "Brems cross sections= " << rMCP->BremsXSection << "\n";

  //if (rMCP->PhotXSection!="PEGS4"){
      t << "Photon cross sections= " << rMCP->PhotXSection << "\n";
  //}
  t << "Electron Impact Ionization= " << rMCP->EIIXSection << "\n";
  t << "Photon cross-sections output= " << rMCP->PhotXSectionOut << "\n";

  t << "Bound Compton scattering= " << rMCP->BC << "\n";
  if ( ( rMCP->BC.toLower() == "on in regions" ) ||
       ( rMCP->BC.toLower() == "off in regions" ) )
  {
     t << "Bound Compton start region= " << rMCP->BCstart << "\n";
     t << "Bound Compton stop region= " << rMCP->BCstop << "\n";
  }

  t << "Pair angular sampling= " << rMCP->PairSampling << "\n";

  t << "Photoelectron angular sampling= " << rMCP->PE << "\n";
  if ( ( rMCP->PE.toLower() == "on in regions" ) ||
       ( rMCP->PE.toLower() == "off in regions" ) )
  {
     t << "PE sampling start region= " << rMCP->PEstart << "\n";
     t << "PE sampling stop region= "  << rMCP->PEstop  << "\n";
  }

  /* Rayleigh scattering output */
  t << "Rayleigh scattering= " << rMCP->RAY << "\n";
  if ( ( rMCP->RAY.toLower() == "on in regions" ) ||
       ( rMCP->RAY.toLower() == "off in regions" ) )
  {
     t << "Rayleigh start region= " << rMCP->RAYstart << "\n";
     t << "Rayleigh stop region= "  << rMCP->RAYstop  << "\n";
  }
  else if (rMCP->RAY.toLower() == "custom"){
     t << "ff media names= " << rMCP->ff_media << "\n";
     t << "ff file names= " << rMCP->ff_files << "\n";
  }

  t << "Atomic relaxations= " << rMCP->RELAX << "\n";
  if ( ( rMCP->RELAX.toLower() == "on in regions" ) ||
       ( rMCP->RELAX.toLower() == "off in regions" ) )
  {
     t << "Relaxations start region= " << rMCP->RELAXstart << "\n";
     t << "Relaxations stop region= "  << rMCP->RELAXstop  << "\n";
  }

//  if ( rMCP->SetPCUTbyRegion ) {
     t << "Set PCUT= " << rMCP->PCUT << "\n" ;
     t << "Set PCUT start region= " << rMCP->PCUTstart << "\n";
     t << "Set PCUT stop region= " << rMCP->PCUTstop << "\n";
//  }

//  if ( rMCP->SetECUTbyRegion ) {
     t << "Set ECUT= " << rMCP->ECUT << "\n" ;
     t << "Set ECUT start region= " << rMCP->ECUTstart << "\n";
     t << "Set ECUT stop region= " << rMCP->ECUTstop << "\n";
//  }

//  if ( rMCP->SetSMAXbyRegion ) {
     t << "Set SMAX= " << rMCP->SMAX << "\n" ;
     t << "Set SMAX start region= " << rMCP->SMAXstart << "\n";
     t << "Set SMAX stop region= " << rMCP->SMAXstop << "\n";
//  }

  print_delimeter( "stop" , "MC transport parameter", t);

  return t;
}




