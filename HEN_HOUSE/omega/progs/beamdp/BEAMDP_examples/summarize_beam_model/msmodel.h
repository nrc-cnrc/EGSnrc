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


#if !defined(MSMODEL_H)
#define MSMODEL_H

#include <iostream>
using namespace std;

#include "sources.h"

// Maximum number of sub-sources
#define MAX_NUM_SOURCES 22

class ms_model
{
   private:
     char title[100];		// This is more than max size of title
     char phsp[160];		// Path could be very long
     int num_sources;		// Number of sub-sources
     source *my_sources[MAX_NUM_SOURCES];

   public:
      ms_model(void);

      // Write/read title
      void put_title(char *);
      char* get_title(void);

      // Phase-space file name
      void put_phsp(char *);
      char* get_phsp(void);

      // Add/get sub-source
      int get_num_sources(void);	// Get number of sub-sources
      source* add_source(int,int,int);	// Add sub-source
      source* get_source(int);		// Get sub-source

      // Misc
      double total_intensity();		// Total intensity
      void CalculateIntensity(void); // Total charge particle intensity
      double total_Particleintensity(int);// Total charge particle intensity

      // Print
      void print_fluence_dist(void) const;
      void print_energy_spectrum(void) const;
      void print_mean_energies(void) const;
      void print_parameters(void) const;
      void print(void) const;
};

#endif

