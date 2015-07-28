/*
###############################################################################
#
#  EGSnrc egs_inprz plot inputs headers
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


#ifndef PLOTINPUTS_H
#define PLOTINPUTS_H

#include "inputblock.h"
//Added by qt3to4:
//qt3to4 -- BW
//#include <Q3TextStream>
//qt3to4 -- BW
#include <QTextStream>

class MPLOTInputs : public MInputBlock
{
public:
	MPLOTInputs();
	~MPLOTInputs();

	  QString Plotting; //both
  	QString LinePrnOut; // dosrz
  	QString ExtPlotOut; // dosrz
  	QString ExtPlotType;// both

  	v_int   PlotIX;     // both
  	v_int   PlotIZ;     // both

  	QString DrawFluPlot; // flurz
  	QString eminusPlot;  // flurz
  	QString eplusPlot;   // flurz
  	QString ePlot;       // flurz
  	QString gPlot;       // flurz

  	v_int SpecPlotStart; // flurz
  	v_int SpecPlotStop;  // flurz
};
std::ifstream & operator >> ( std::ifstream & in, MPLOTInputs * rPLOT );
//Q3TextStream   & operator << ( Q3TextStream &    t, MPLOTInputs * rPLOT );
//qt3to4 -- BW
QTextStream   & operator << ( QTextStream &    t, MPLOTInputs * rPLOT );
#endif
