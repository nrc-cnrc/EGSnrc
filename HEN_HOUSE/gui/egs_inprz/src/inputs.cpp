/*
###############################################################################
#
#  EGSnrc egs_inprz inputs
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


#include "inputs.h"
//Added by qt3to4:
//#include <Q3TextStream>
//qt3to4 -- BW
#include <QTextStream>

MInputRZ::MInputRZ()
{
	mTitle = new MTitle;
	mIO    = new MIOInputs;
	mMC    = new MMCInputs;
	mPHD   = new MPHDInputs;
	mGEO   = new MGEOInputs;
	mCAV   = new MCAVInputs;
	mSRC   = new MSRCInputs;
	mMCP   = new MMCPInputs;
        mPGLS  = new PEGSLESSInputs;
	mVAR   = new MVARInputs;
	mPLOT  = new MPLOTInputs ;
}

MInputRZ::~MInputRZ()
{
	zap( mTitle );
	zap( mIO );
	zap( mMC );
        zap( mPGLS );
	zap( mPHD );
	zap( mGEO );
	zap( mCAV );
	zap( mSRC );
	zap( mMCP );
	zap( mVAR );
	zap( mPLOT );
}

void MInputRZ::SetUserCode( const UserCodeType& rUC )
{

	usercode = rUC; //local to object MInputRZ
	mIO->SetUserCode( rUC );
	mCAV->SetUserCode( rUC );
	mMC->SetUserCode( rUC );
	mPLOT->SetUserCode( rUC );
	mVAR->SetUserCode( rUC );
}

QString MInputRZ::getErrors()
{
   return errors;
}

QString MInputRZ::getGEOErrors()
{
   return mGEO->getErrors();
}

bool MInputRZ::gotErrors()
{

  if ( errors.isEmpty() )
       return false;
  else return true;
}

QString MInputRZ::GetIFULL() const
{
	QString ifulli = mMC->GetIFULL();
 	return ifulli;
}

QString MInputRZ::GetInputMethod() const
{
	QString inp_methodi = mGEO->GetInputMethod();
 	return inp_methodi;
}

void MInputRZ::SetInputMethod( const QString& rInpMeth )
{
	mCAV->SetInputMethod( rInpMeth );
}

bool print_phd( MInputRZ* rZ )
{
  QString iful = rZ->GetIFULL();
  UserCodeType usercode = rZ->GetUserCode();

   if (( iful.toLower() == "pulse height distribution"  ) &&
	    ( usercode == dosrznrc ) )
	      return true;
  return false;
}

bool cavity_inp( MInputRZ* rZ )
{
  UserCodeType usercode = rZ->GetUserCode();

	if ( usercode == cavrznrc)
	      return true;
  return false;
}

bool plot_on( MInputRZ* rZ )
{
  UserCodeType usercode = rZ->GetUserCode();

	if ( ( usercode == dosrznrc) ||
       ( usercode == flurznrc) )
	      return true;
  return false;
}


std::ifstream & operator >> ( std::ifstream & in, MInputRZ*  rZ )
{
	in >> rZ->mTitle; rZ->errors  = rZ->mTitle->errors;
	in >> rZ->mIO;    rZ->errors += rZ->mIO->errors;
	in >> rZ->mMC;    rZ->errors += rZ->mMC->errors;
              if ( print_phd( rZ ) ){
	   in >> rZ->mPHD;rZ->errors += rZ->mPHD->errors;
              }
	in >> rZ->mGEO;   rZ->errors += rZ->mGEO->errors;
              rZ->SetInputMethod( rZ->GetInputMethod() );
	if ( cavity_inp( rZ ) )
	   in >> rZ->mCAV;rZ->errors += rZ->mCAV->errors;
	in >> rZ->mSRC;   rZ->errors += rZ->mSRC->errors;
	in >> rZ->mMCP;   rZ->errors += rZ->mMCP->errors;
        in >> rZ->mPGLS;  rZ->errors += rZ->mPGLS->errors;
	in >> rZ->mVAR;   rZ->errors += rZ->mVAR->errors;
              if ( plot_on( rZ ) ){
	   in >> rZ->mPLOT;rZ->errors += rZ->mPLOT->errors;
              }
	return in;
}

/*
Q3TextStream & operator << ( Q3TextStream & t, MInputRZ * rZ )
{
	t << rZ->mTitle;
	t << rZ->mIO;
	t << rZ->mMC;
	if ( print_phd( rZ ) )
	   t << rZ->mPHD;
	t << rZ->mGEO; rZ->SetInputMethod( rZ->GetInputMethod() );
	if ( cavity_inp( rZ ) )
	   t << rZ->mCAV;
	t << rZ->mSRC;
	t << rZ->mMCP;
        t << rZ->mPGLS;
	t << rZ->mVAR;
	if ( plot_on( rZ ) )
	   t << rZ->mPLOT;
	return t;
}
*/

//qt3to4 -- BW
QTextStream & operator << ( QTextStream & t, MInputRZ * rZ )
{
        t << rZ->mTitle;
        t << rZ->mIO;
        t << rZ->mMC;
        if ( print_phd( rZ ) )
           t << rZ->mPHD;
        t << rZ->mGEO; rZ->SetInputMethod( rZ->GetInputMethod() );
        if ( cavity_inp( rZ ) )
           t << rZ->mCAV;
        t << rZ->mSRC;
        t << rZ->mMCP;
        t << rZ->mPGLS;
        t << rZ->mVAR;
        if ( plot_on( rZ ) )
           t << rZ->mPLOT;
        return t;
}
