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


#if !defined(SOURCES_H)
#define SOURCES_H

#include <iostream>
#include <cstdlib>
#include <iomanip>
#include "abuf.h"
using namespace std;

#define W_SNAME		11
#define W_STYPE		3
#define W_CHARGE	7
#define W_LATCH		7
#define W_D		9
#define W_EN		9
#define W_RELINT	9

#define P_D		2
#define P_EN		3
#define P_RELINT	4

typedef enum source_type {APPLICATOR=1,COLLIMATOR=2,RING=3,RECTPLANE=4,CIRCPLANE=5};

class source
{
protected:
   source_type s_type;
   int charge;
   int latch;
   double d;
   double relint;
   double particle_relint;

   energy_spectrum *my_energy_spectrum;
   fluence_field *my_fluence_field;

public:
   source() {		// Constructor
     d = 0;
     relint = 0;
     my_energy_spectrum = NULL;
     my_fluence_field = NULL;
   }

   // Charge
   int get_charge( void ) {
      return charge;
   }

   // Relative intensity
   void put_relint( double r ) {
      relint = r;
   }
   double get_relint( void ) {
      return relint;
   }

   // Particle relative intensity
   void put_particle_relint( double r ) {
      particle_relint = r;
   }
   double get_particle_relint( void ) {
      return particle_relint;
   }

   // Add/get field
   fluence_field* add_fluence_field(fluence_field_type ft, int trn, int scon, double rin,double rout) {
      my_fluence_field = new fluence_field( (fluence_field_type)ft, trn, scon, rin, rout );
      return my_fluence_field;
   }
   fluence_field* get_fluence_field(void) {
      return my_fluence_field;
   }

   energy_spectrum* add_energy_spectrum(int type,int nbins,int nradii, double emin,double emax) {
      my_energy_spectrum = new energy_spectrum( type, nradii, nbins, emin, emax );
      return my_energy_spectrum;
   }

   energy_spectrum* get_energy_spectrum(void) {
      return my_energy_spectrum;
   }

   virtual void put_pars(double *) {
     cout << "Not implemented\n";
   }

   void print_fluence_dist( void ) {
      double total = 0.0, d;
      cout << setiosflags(ios::fixed);
      for( int i=0, j=0; i < this->my_fluence_field->get_bins(); i++ ) {
	 d = my_fluence_field->get_Fvalue(i,0);
	 total += d;
	 cout << setw(W_EN) << setprecision(P_EN) << d << " ";
	 if( ++j == 8 ) {
	   cout << endl;
	   j = 0;
	 }
      }
      cout << "\nTotal number of particles: " << total << endl;

      if( this->my_fluence_field->get_type() == SQUARE ) {
	 cout << "\nParameters for spatial distribution (1):\n";
	 for( int i=0, j=0; i < this->my_fluence_field->get_bins(); i++ ) {
	    d = my_fluence_field->get_Fvalue(i,1);
	    cout << setw(W_EN) << setprecision(P_EN) << d << " ";
	    if( ++j == 8 ) {
		cout << endl;
		j = 0;
	    }
	 }
	 cout << "\nParameters for spatial distribution (2):\n";
	 for( int i=0, j=0; i < this->my_fluence_field->get_bins(); i++ ) {
	    d = my_fluence_field->get_Fvalue(i,2);
	    cout << setw(W_EN) << setprecision(P_EN) << d << " ";
	    if( ++j == 8 ) {
		cout << endl;
		j = 0;
	    }
	 }
      }
   }

   void print_energy_spectrum( void ) {
      double init_radius = 0.0;
      cout << setiosflags(ios::fixed);
      for( int i=0; i < this->my_energy_spectrum->get_nradii(); i++ ) {
	cout << "Energy spectrum for region " << init_radius << "-" <<
	         this->my_energy_spectrum->get_radius(i) << "cm\n";
	init_radius = this->my_energy_spectrum->get_radius(i);
	for( int j=0, k=0; j < this->my_energy_spectrum->get_bins(); j++ ) {
	   cout << setw(W_EN) << setprecision(P_EN) <<
	   my_energy_spectrum->get_Evalue(j,i) << " ";
	   if( ++k == 8 ) {
	      cout << endl;
	      k = 0;
	   }
	}
	cout << "\nAverage energy: " << this->my_energy_spectrum->get_average(i) << endl;
     }
  }

   void print_mean_energy( void ) {
      double init_radius = 0.0;
      cout << setiosflags(ios::fixed);
      for( int i=0; i < this->my_energy_spectrum->get_nradii(); i++ ) {
	cout << "Mean energy for region " << init_radius << "-" <<
	         this->my_energy_spectrum->get_radius(i) << "cm : " <<
		 this->my_energy_spectrum->get_average(i) << " MeV\n";
	init_radius = this->my_energy_spectrum->get_radius(i);
     }
  }

   virtual void print(void) const {
     cout << "Not implemented\n";
   }

   virtual void print_parameters(void) const {
     cout << "Not implemented\n";
   }
};

class applicator:public source
{
   private:
     double Xmin;
     double Xmax;
     double Ymin;
     double Ymax;
     double absXmax;
     double absYmax;
   public:
      applicator(int,int,int);
      void put_pars( double* );
      virtual void print(void) const;
      virtual void print_parameters(void) const;
};

class collimator:public source
{
   private:
     double Xmin;
     double Xmax;
     double Ymin;
     double Ymax;
     double absXmax;
     double absYmax;
     int orientation;
   public:
      collimator(int,int,int);
      void put_pars( double* );
      virtual void print(void) const;
      virtual void print_parameters(void) const;
};

class ring:public source
{
   private:
     double IR;
     double OR;
     double virtualOR;
   public:
      ring(int,int,int);
      void put_pars( double* );
      virtual void print(void) const;
      virtual void print_parameters(void) const;
};

class rectplane:public source
{
   private:
     double Xmin;
     double Xmax;
     double Ymin;
     double Ymax;
   public:
      rectplane(int,int,int);
      void put_pars( double* );
      virtual void print(void) const;
      virtual void print_parameters(void) const;
};


class circplane:public source
{
   private:
     double R;
   public:
      circplane(int,int,int);
      void put_pars( double* );
      virtual void print(void) const;
      virtual void print_parameters(void) const;
};

#endif
