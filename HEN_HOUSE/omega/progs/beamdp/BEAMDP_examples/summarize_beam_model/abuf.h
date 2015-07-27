/*
###############################################################################
#
#  EGSnrc summarize beam model header file
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
#  Author:          Zdenko Sego, 2005
#
#  Contributors:    Iwan Kawrakow
#                   Blake Walters
#
###############################################################################
*/


#if !defined(ABUF_H)
#define ABUF_H

#include <iostream>
#include <cstdlib>

using namespace std;

#define BUFSIZE 200

typedef enum fluence_field_type {CIRCULAR=0,SQUARE=1,RECTANGULAR=2};
typedef enum buffer_type {F_IN=3,F_OUT=4,ANG=5};

class fluence_field
{
protected:
   fluence_field_type type;
   int trFbins;
   int scoFbins;
   int totalFbins;
   double trFieldSize;
   double scoFieldSize;
   double F[BUFSIZE][3];
public:
   fluence_field( fluence_field_type t, int trn, int scon, double trf, double scof) {
      type = t;
      trFbins = trn;
      scoFbins = scon;
      totalFbins = trn + scon;
      trFieldSize = trf;
      scoFieldSize = scof;
   }
   int get_type( void ) {
      return type;
   }
   int get_bins( void ) {
      return totalFbins;
   }
   int get_trFbins( void ) {
      return trFbins;
   }
   int get_scoFbins( void ) {
      return scoFbins;
   }
   double get_trFieldSize( void ) {
      return trFieldSize;
   }
   double get_scoFieldSize( void ) {
      return scoFieldSize;
   }
   void put_Fvalue( double value, int i, int j ) {
     if(((i >= 0) && (i < BUFSIZE)) && ((j>=0) && (j < 2)))
	F[i][j] = value;
     else
	cout << "Error writing fluence value!\n";
   }
   double get_Fvalue( int i, int j ) {
     if(((i >= 0) && (i < BUFSIZE)) && ((j>=0) && (j < 3)))
	return F[i][j];
     else {
	cout << "Error reading fluence value!\n";
	return 0.0;
      }
   }
};

class energy_spectrum
{
protected:
   int type;	// Inside/outside or different radii
   int nradii;	// Number of radii
   int nbins;	// Number of energy bins
   double Emin;	// Minimum kinetic energy
   double Emax;	// Maximum kinetic energy

   double radii[BUFSIZE];
   double E[BUFSIZE][BUFSIZE];

public:
   energy_spectrum(int t,int nr,int nb, double emin,double emax) {
      type = t;
      nradii = nr;
      nbins = nb;
      Emin = emin;
      Emax = emax;
   }
   int get_type( void ) {
      return type;
   }
   int get_bins( void ) {
      return nbins;
   }
   int get_nradii( void ) {
      return nradii;
   }
   double get_Emin( void ) {
      return Emin;
   }
   double get_Emax( void ) {
      return Emax;
   }
   double get_Evalue( int i, int j ) {
      if(((i >= 0) && (i < BUFSIZE)) && ((j>=0) && (j < BUFSIZE)))
	return( E[i][j] );
      else {
	cout << "Error reading energy value!\n";
	return 0.0;
      }
   }
   void put_Evalue( double value, int i, int j ) {
     if(((i >= 0) && (i < BUFSIZE)) && ((j>=0) && (j < BUFSIZE)))
	E[i][j] = value;
     else
	cout << "Error writing energy value!\n";
   }
   void put_radius( double value, int i ) {
      if( i >= 0 && i < nradii )
	radii[i] = value;
      else
	cout << "Error writing energy radius!\n";
   }
   double get_radius( int i ) {
      if( i >= 0 && i < nradii )
	 return( radii[i] );
      else {
	cout << "Error reading energy radius!\n";
	 return 0.0;
      }
   }
   double get_average(int i) {
      int j;
      double sum = 0.0, average = 0.0;

      for( j = 0; j < nbins; j++ ) {
	sum += E[j][i];
	average += (((Emax-Emin)*j)/nbins)*E[j][i];
      }

      return sum == 0.0 ? sum : average/sum;
   }
};

#endif
