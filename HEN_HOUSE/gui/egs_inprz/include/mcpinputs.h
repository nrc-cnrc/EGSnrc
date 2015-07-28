/*
###############################################################################
#
#  EGSnrc egs_inprz Monte Carlo parameters inputs headers
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


#ifndef MCPINPUTS_H
#define MCPINPUTS_H

#include "inputblock.h"
//Added by qt3to4:
//qt3to4 -- BW
//#include <Q3TextStream>

//qt3to4 -- BW

class MMCPInputs : public MInputBlock
{
  public:
   MMCPInputs();
  ~MMCPInputs();

        v_string ff_media;
        v_string ff_files;

	v_float ECUT;
 	v_int   ECUTstart;
 	v_int   ECUTstop;

 	v_float PCUT;
 	v_int   PCUTstart;
 	v_int   PCUTstop;

 	v_float SMAX;
 	v_int   SMAXstart;
 	v_int   SMAXstop;

 	QString GECUT;
 	QString GPCUT;
 	QString GSMAX;

 	bool SetPCUTbyRegion;
 	bool SetECUTbyRegion;
 	bool SetSMAXbyRegion;

 	QString ESTEPE;
 	QString XImax;
 	QString SkinD;
 	QString PairSampling;
 	QString BremsSampling;
 	QString BremsXSection;
 	QString ESTEPAlg;
 	QString BoundXAlg;
 	QString PhotXSection;
 	QString PhotXSectionOut;
 	QString EIIXSection;

 	QString Spin;
 	QString BC;
 	QString PE;
 	QString RAY;
 	QString RELAX;

 	v_int BCstart;
 	v_int BCstop;
 	v_int PEstart;
 	v_int PEstop;
 	v_int RAYstart;
 	v_int RAYstop;
 	v_int RELAXstart;
 	v_int RELAXstop;
};
std::ifstream & operator >> ( std::ifstream & in, MMCPInputs * rMCP );
//Q3TextStream   & operator << ( Q3TextStream &    t, MMCPInputs * rMCP );
//qt3to4 -- BW
QTextStream   & operator << ( QTextStream &    t, MMCPInputs * rMCP );
#endif
