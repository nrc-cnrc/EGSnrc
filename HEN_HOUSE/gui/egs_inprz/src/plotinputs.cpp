/*
###############################################################################
#
#  EGSnrc egs_inprz plot inputs
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


#include "plotinputs.h"
//Added by qt3to4:
//#include <Q3TextStream>
//qt3to4 -- BW
#include <QTextStream>

MPLOTInputs::MPLOTInputs()
{
  Plotting    = "off";   // both
  LinePrnOut  = "off";   // dosrz
  ExtPlotOut  = "off";   // dosrz
  ExtPlotType = "Point"; // both

  PlotIX.push_back( 1 ); // both
  PlotIZ.push_back( 1 ); // both

  DrawFluPlot = "none"; // flurz
  eminusPlot  = "off"; // flurz
  eplusPlot   = "off"; // flurz
  ePlot       = "off"; // flurz
  gPlot       = "off"; // flurz

  SpecPlotStart.push_back( 1 ); // flurz
  SpecPlotStop.push_back( 1 );  // flurz
}

MPLOTInputs::~MPLOTInputs()
{
	//free anything created in the constructor here
}

/*
PLOT_CONTROL MPLOTInputs::Get()
{
	return PLOTInp;
}

void MPLOTInputs::Set( const PLOT_CONTROL& rPLOT )
{
	PLOTInp = rPLOT;
}
*/

std::ifstream & operator >> ( std::ifstream & in, MPLOTInputs*  rPLOT )
{

  std::vector<string> codes;
  codes.push_back("PLOTTING");
  codes.push_back("LINE PRINTER OUTPUT");
  codes.push_back("EXTERNAL PLOTTER OUTPUT");
  codes.push_back("EXTERNAL PLOT TYPE");
  codes.push_back("PLOT RADIAL REGION IX");
  codes.push_back("PLOT PLANAR REGION IZ");
  codes.push_back("DRAW FLUENCE PLOTS");
  codes.push_back("PLOTS FOR ELECTRONS");
  codes.push_back("PLOTS FOR POSITRONS");
  codes.push_back("PLOTS FOR PHOTONS");
  codes.push_back("PLOTS FOR E- AND E+");
  codes.push_back("START SPECTRAL PLOT IN REGION");
  codes.push_back("STOP SPECTRAL PLOT IN REGION");

  DE_Parser *p = new DE_Parser(codes,0,"plot control", in, false);

  rPLOT->Plotting = getIt( codes[0] ,"off", rPLOT->errors, p );
  if ( rPLOT->Plotting.toLower() == "on" ) {

       if ( rPLOT->gusercode() == dosrznrc ) {
            rPLOT->LinePrnOut = getIt( codes[1] ,"off", rPLOT->errors, p );
            rPLOT->ExtPlotOut = getIt( codes[2] ,"off", rPLOT->errors, p );
       }

       if ( ( ( rPLOT->ExtPlotOut.toLower() == "on" ) &&
              ( rPLOT->gusercode() == dosrznrc ) ) ||
              ( rPLOT->gusercode() == flurznrc )   ){
     	      rPLOT->ExtPlotType = getIt( codes[3] ,"Point", rPLOT->errors, p );
       }

       rPLOT->PlotIX = getThem( codes[4], 0, 10000, rPLOT->PlotIX, rPLOT->errors, p );
       rPLOT->PlotIZ = getThem( codes[5], 0, 10000, rPLOT->PlotIZ, rPLOT->errors, p );

       if ( rPLOT->gusercode() == flurznrc ) {
   	      rPLOT->DrawFluPlot = getIt( codes[6] ,"none", rPLOT->errors, p );

          rPLOT->eminusPlot =  getIt( codes[7] ,"off", rPLOT->errors, p );
          rPLOT->eplusPlot  =  getIt( codes[8] ,"off", rPLOT->errors, p );
          rPLOT->gPlot      =  getIt( codes[9] ,"off", rPLOT->errors, p );
          rPLOT->ePlot      =  getIt( codes[10] ,"off", rPLOT->errors, p );

          rPLOT->SpecPlotStart = getThem( codes[11], 0, 10000, rPLOT->SpecPlotStart, rPLOT->errors, p );
          rPLOT->SpecPlotStop  = getThem( codes[12], 0, 10000, rPLOT->SpecPlotStop, rPLOT->errors, p );
       }
  }

  if ( ! rPLOT->errors.isEmpty() ) {
    rPLOT->errors = "***  PLOT CONTROL block *** <br>" +  rPLOT->errors + "<br>";
  }

  delete p;

  return in;
}

/*
Q3TextStream & operator << ( Q3TextStream & t, MPLOTInputs * rPLOT )
{

  print_delimeter( "start" , "plot control", t);

  t << "PLOTTING= " << rPLOT->Plotting << "\n";
  if ( rPLOT->Plotting.toLower() == "on" ) {

       if ( rPLOT->gusercode() == dosrznrc ) {
            t << "LINE PRINTER OUTPUT= " << rPLOT->LinePrnOut << "\n";
            t << "EXTERNAL PLOTTER OUTPUT= " << rPLOT->ExtPlotOut << "\n";
       }

       if ( ( ( rPLOT->ExtPlotOut.toLower() == "on" ) &&
              ( rPLOT->gusercode() == dosrznrc ) ) ||
              ( rPLOT->gusercode() == flurznrc )   ){
     	      t << "EXTERNAL PLOT TYPE= " << rPLOT->ExtPlotType << "\n";
       }

       if ( (rPLOT->gusercode() == dosrznrc)      ||
            (rPLOT->gusercode() == flurznrc         &&
	rPLOT->DrawFluPlot.toLower() != "none")  ) {
              t << "PLOT RADIAL REGION IX= " << rPLOT->PlotIX << "\n";
              t << "PLOT PLANAR REGION IZ= " << rPLOT->PlotIZ << "\n";
       }

       if ( rPLOT->gusercode() == flurznrc ) {
          t << "DRAW FLUENCE PLOTS= " << rPLOT->DrawFluPlot << "\n";

          t << "PLOTS FOR ELECTRONS= " << rPLOT->eminusPlot << "\n";
          t << "PLOTS FOR POSITRONS= "   << rPLOT->eplusPlot  << "\n";
          t << "PLOTS FOR PHOTONS= "   << rPLOT->gPlot      << "\n";
          t << "PLOTS FOR E- AND E+= " << rPLOT->ePlot      << "\n";

          t << "START SPECTRAL PLOT IN REGION= " << rPLOT->SpecPlotStart << "\n";
          t << "STOP SPECTRAL PLOT IN REGION= "  << rPLOT->SpecPlotStop  << "\n";

       }
  }

  print_delimeter( "stop" , "plot control", t);

  return t;
}
*/

//qt3to4 -- BW
QTextStream & operator << ( QTextStream & t, MPLOTInputs * rPLOT )
{

  print_delimeter( "start" , "plot control", t);

  t << "PLOTTING= " << rPLOT->Plotting << "\n";
  if ( rPLOT->Plotting.toLower() == "on" ) {

       if ( rPLOT->gusercode() == dosrznrc ) {
            t << "LINE PRINTER OUTPUT= " << rPLOT->LinePrnOut << "\n";
            t << "EXTERNAL PLOTTER OUTPUT= " << rPLOT->ExtPlotOut << "\n";
       }

       if ( ( ( rPLOT->ExtPlotOut.toLower() == "on" ) &&
              ( rPLOT->gusercode() == dosrznrc ) ) ||
              ( rPLOT->gusercode() == flurznrc )   ){
              t << "EXTERNAL PLOT TYPE= " << rPLOT->ExtPlotType << "\n";
       }

       if ( (rPLOT->gusercode() == dosrznrc)      ||
            (rPLOT->gusercode() == flurznrc         &&
        rPLOT->DrawFluPlot.toLower() != "none")  ) {
              t << "PLOT RADIAL REGION IX= " << rPLOT->PlotIX << "\n";
              t << "PLOT PLANAR REGION IZ= " << rPLOT->PlotIZ << "\n";
       }

       if ( rPLOT->gusercode() == flurznrc ) {
          t << "DRAW FLUENCE PLOTS= " << rPLOT->DrawFluPlot << "\n";

          t << "PLOTS FOR ELECTRONS= " << rPLOT->eminusPlot << "\n";
          t << "PLOTS FOR POSITRONS= "   << rPLOT->eplusPlot  << "\n";
          t << "PLOTS FOR PHOTONS= "   << rPLOT->gPlot      << "\n";
          t << "PLOTS FOR E- AND E+= " << rPLOT->ePlot      << "\n";

          t << "START SPECTRAL PLOT IN REGION= " << rPLOT->SpecPlotStart << "\n";
          t << "STOP SPECTRAL PLOT IN REGION= "  << rPLOT->SpecPlotStop  << "\n";

       }
  }

  print_delimeter( "stop" , "plot control", t);

  return t;
}

