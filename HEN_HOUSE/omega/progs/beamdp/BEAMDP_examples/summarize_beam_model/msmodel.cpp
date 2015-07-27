/*
###############################################################################
#
#  EGSnrc summarize beam model source code
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


#include <iomanip>
#include "msmodel.h"

ms_model::ms_model(void) {
   memset( title, '\0', sizeof(title) );
   memset( phsp, '\0', sizeof(phsp) );

   // Set number of sub-sources to zero
   num_sources = 0;

   for( int i = 0;i < MAX_NUM_SOURCES; i++ )
      my_sources[i] = NULL;
}

// Title
void ms_model::put_title(char *t) {
   memcpy(title,t,strlen(t));
}
char* ms_model::get_title(void) {
   return title;
}

// Phase-space file information
void ms_model::put_phsp(char *p) {
   memcpy(phsp,p,strlen(p));
}
char* ms_model::get_phsp(void) {
   return phsp;
}

// Get number of sources
int ms_model::get_num_sources(void) {
   return num_sources;
}

// Add new source
source* ms_model::add_source(int src, int ch, int lat) {

   switch(src) {
      case APPLICATOR:
	my_sources[num_sources] = new applicator(src,ch,lat);
	break;
      case COLLIMATOR:
	my_sources[num_sources] = new collimator(src,ch,lat);
	break;
      case RING:
	my_sources[num_sources] = new ring(src,ch,lat);
	break;
      case RECTPLANE:
	my_sources[num_sources] = new rectplane(src,ch,lat);
	break;
      case CIRCPLANE:
	my_sources[num_sources] = new circplane(src,ch,lat);
	break;
      default:
	cout << "Unknown sub-source number: " << src << "\n";
	break;
   }
   ++num_sources;
   return my_sources[num_sources-1];
}

// Get source
source* ms_model::get_source(int n) {
   return my_sources[n];
}

// Calculate total intensity
double ms_model::total_intensity(void)
{
   int i;
   double total;

   for( i=0, total=0.0; i < num_sources; i++ ) {
	total += my_sources[i]->get_relint();
   }
   return total;
}

// Calculate total charge particle intensity
void ms_model::CalculateIntensity(void)
{
   int i,c;
   double relint, parint;

   for( i=0; i < num_sources; i++ ) {
      c = my_sources[i]->get_charge();
      relint = my_sources[i]->get_relint();
      parint = total_Particleintensity(c);
      my_sources[i]->put_particle_relint( relint/parint );
   }
}

// Calculate total charge particle intensity
double ms_model::total_Particleintensity(int c)
{
   int i;
   double total=0.0;

   if((c >= -1) && (c <= 1)) {
      for( i=0; i < num_sources; i++ ) {
	if( my_sources[i]->get_charge() == c )
	   total += my_sources[i]->get_relint();
      }
   }
   return total;
}

// Print fluence distribution for all sources
void ms_model::print_fluence_dist( void ) const {
   for( int i = 0; i < num_sources; i++ ) {
      cout << "\nFluence distribution for source number " << i+1 << endl;
      cout << "---------------------------------------------------------------------\n";
      my_sources[i]->print_fluence_dist();
   }
   cout << endl;
}

// Print energy spectra for all sources
void ms_model::print_energy_spectrum( void ) const {
   for( int i = 0; i < num_sources; i++ ) {
      cout << "\nEnergy spectrum (Ein) for source number " << i+1 << endl;
      cout << "---------------------------------------------------------------------\n";
      my_sources[i]->print_energy_spectrum();
   }
   cout << endl;
}

// Print mean energies
void ms_model::print_mean_energies( void ) const {
   for( int i = 0; i < num_sources; i++ ) {
      cout << "\nMean energies for source number " << i+1 << endl;
      cout << "--------------------------------------------------\n";
      my_sources[i]->print_mean_energy();
   }
   cout << endl;
}

void ms_model::print_parameters( void ) const {
   for( int i = 0; i < num_sources; i++ ) {
      cout << "\nParameters for source number " << i+1 << endl;
      cout << "-----------------------------------------------\n";
      my_sources[i]->print_parameters();
   }
}

void ms_model::print(void) const
{
   cout << "\nTitle: " << title << endl;
   cout << "Number of sub-sources: " << num_sources << "\n";
   cout << "Phsp file: " << phsp << endl;
   cout << "-----------------------------------------------------------\n";
   cout << "Src# SourceType Type# Charge Latch d(cm)   RelInte ParRelIn\n";
   cout << "-----------------------------------------------------------\n";
   for( int i=0; i < num_sources; i++ ) {
      cout << setw(3) << i+1;
      my_sources[i]->print();
   }
   cout << "\n\n";
}
