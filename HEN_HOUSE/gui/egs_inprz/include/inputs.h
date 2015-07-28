/*
###############################################################################
#
#  EGSnrc egs_inprz inputs headers
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


#ifndef INPUTS_H
#define INPUTS_H

#include "inputblock.h"
#include "cavinputs.h"
#include "geoinputs.h"
#include "ioinputs.h"
#include "mcinputs.h"
#include "mcpinputs.h"
#include "pegslessinputs.h"
#include "phdinputs.h"
#include "plotinputs.h"
#include "srcinputs.h"
#include "title.h"
#include "varinputs.h"
//Added by qt3to4:
//qt3to4 -- BW
//#include <Q3TextStream>

//qt3to4 -- BW
#include <QTextStream>

class MInputRZ
{
public:
	MInputRZ();
	~MInputRZ();
	QString GetInputMethod() const;
	QString GetIFULL() const;
	void    SetInputMethod( const QString& rInpMeth );
    void    SetUserCode( const UserCodeType& rUC );
	UserCodeType GetUserCode() const { return usercode; }
    bool    gotErrors();
    QString getErrors();
    QString getGEOErrors();

     friend  std::ifstream & operator >> ( std::ifstream & in, MInputRZ * r );
     //friend  Q3TextStream   & operator << ( Q3TextStream &    t, MInputRZ * r );
     //qt3to4 -- BW
     friend  QTextStream   & operator << ( QTextStream &    t, MInputRZ * r );

	MTitle*      mTitle;
	MIOInputs*   mIO;
	MMCInputs*   mMC;
	MPHDInputs*  mPHD;
	MGEOInputs*  mGEO;
	MCAVInputs*  mCAV;
	MSRCInputs*  mSRC;
	MMCPInputs*  mMCP;
        PEGSLESSInputs* mPGLS;
	MVARInputs*  mVAR;
	MPLOTInputs* mPLOT;
    QString errors;

private:
	bool phdON;
	bool cavON;
	bool plotON;
	UserCodeType usercode;
};

std::ifstream & operator >> ( std::ifstream & in, MInputRZ*  rZ );
//Q3TextStream & operator << ( Q3TextStream & t, MInputRZ * rZ );
//qt3to4 -- BW
QTextStream & operator << ( QTextStream & t, MInputRZ * rZ );

#endif
