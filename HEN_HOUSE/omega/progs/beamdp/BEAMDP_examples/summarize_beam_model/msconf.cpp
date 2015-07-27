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
#
###############################################################################
*/


#include "stdlib.h"
#include "unistd.h"
#include "msmodel.h"
#include "sources.h"

#include <iostream>
using namespace std;

#define NUM_PARS 7

static void
usage(const char *cmd) {
fprintf(stderr, "Usage: %s [-hvpe] source_file\n\n", cmd);
fputs("This program reads a multiple-source model and prints its parameters:\n",stderr);
fputs(" - source number\n",stderr);
fputs(" - sub-source type, charge and latch bit\n",stderr);
fputs(" - sub-source distance from the scoring plane\n",stderr);
fputs(" - average energy inside the field (Ei) and outside the field (Eo)\n",stderr);
fputs(" - relative intensity of each sub-source\n",stderr);
fputs("Options:\n",stderr);
fputs(" -v Verbose print\n",stderr);
fputs(" -p Print parameters\n",stderr);
fputs(" -e Print energy spectrum\n",stderr);
fputs("\nProgram created by Zdenko Sego, Carleton University, Canada, 2005\n\n",stderr);
}

void read_msconf(FILE *,ms_model *);
void process_str(char *);
static bool verbose = 0;

int
main(int argc,char **argv)
{
   int optch;
   static char optstring[]="hvpe";
   bool print_spectrum = 0, print_param = 0;
   FILE *fp;

   ms_model *my_model = new ms_model();;

   while( (optch = getopt(argc,argv,optstring)) != -1 ) {
      switch( optch ) {
	case 'v':
	   verbose = 1;	// Verbose print
	   break;
	case 'p':
	   print_param = 1;
	   break;
	case 'e':
	   print_spectrum = 1;
	   break;
	case 'h':
	default:
	   usage(argv[0]);
	   exit(1);
      }
   }

   if((argc == 1)||(argc > optind+1)) {
      usage(argv[0]);
      exit( -1 );
   } else if( argc = optind + 1 ) {
      if( fp = fopen( argv[optind], "r" ) ) {
	read_msconf(fp, my_model);
	fclose(fp);
      }
      else {
	cout << "Cannot open file '" << argv[optind] << "'\n";
	exit(-1);
      }
   }

   my_model->print();
   cout << " --- Total intensity: " << my_model->total_intensity() << endl;
   cout << endl;

   for( int i=0; i < my_model->get_num_sources(); i++ ) {
      if( print_spectrum ) {
	 my_model->print_energy_spectrum(i+1);
      }
      if( print_param ) {
	 my_model->print_parameters(i+1);
      }
   }
}

void
read_msconf(FILE *fp, ms_model *my_model)
{
   source *my_source;
   char buffer[120], *pch;
   int type, charge, latch;
   int i, j, numss = 0, snum;
   int Ebins;
   double E, Emin, Emax;
   int FieldType;
   int Fbins;
   double F, IFdim, OFdim; // Inside/Outside Field dimension
   double relint, parameters[NUM_PARS];
   energy_spectrum *my_energy_spectrum;

   // Title
   fgets(buffer,sizeof(buffer), fp );
   my_model->put_title(buffer);
   if( verbose )
      printf( "Title: %s\n", buffer );

   // Number of sources
   fgets(buffer,sizeof(buffer), fp );
   sscanf( buffer, "%d", &numss );
   if( verbose )
      printf( "Number of sources: %d\n", numss );
   for( i=0; i < numss; i++ ) {
      // Type, charge, latch
      fgets( buffer,sizeof(buffer), fp );
      pch = strtok( &buffer[0], " " );
      sscanf(pch, "%d", &type);
      pch = strtok( NULL, " " );
      sscanf(pch, "%d", &charge);
      pch = strtok( NULL, " " );
      sscanf(pch, "%d", &latch);
      my_source = my_model->add_source(type,charge,latch);
      if( verbose )
	 printf( "Type:%d,charge:%d,latch:%d\n",type,charge,latch);

      // Add parameters
      fgets(buffer,sizeof(buffer), fp );
      for( j = 0; j < NUM_PARS; j++ )
	 parameters[j] = 0.0;
      j = 0;
      pch = strtok( &buffer[0], " " );
      do {
	 sscanf(pch, "%lf", &parameters[j]);
	 j++;
	 pch = strtok( NULL, " " );
      } while( pch && (j < NUM_PARS) );
      my_source->put_pars(parameters);
   }

   // Number of energy bins, Emin, Emax
   fgets(buffer,sizeof(buffer), fp );
   pch = strtok( &buffer[0], " " );
   sscanf(pch, "%d", &Ebins);
   pch = strtok( NULL, " " );
   sscanf(pch, "%lf", &Emin);
   pch = strtok( NULL, " " );
   sscanf(pch, "%lf", &Emax);

   // Field type
   fgets( buffer, sizeof(buffer), fp );
   sscanf( buffer, "%d", &FieldType );
   if( verbose )
      printf( "Field type: %d\n", FieldType );

   // Number of fluence bins
   fgets(buffer,sizeof(buffer), fp );
   pch = strtok( &buffer[0], " " );
   sscanf(pch, "%d", &Fbins);
   pch = strtok( NULL, " " );
   sscanf(pch, "%lf", &IFdim);
   if( FieldType < 3 ) {
      pch = strtok( NULL, " " );
      sscanf(pch, "%lf", &OFdim);
   } else OFdim = 0;
   if( verbose )
      printf( "Planar bins: %d,IFdim=%lf,OFdim=%lf\n", Fbins, IFdim, OFdim );

   // Phase-space file
   fgets(buffer,sizeof(buffer), fp );
   my_model->put_phsp(buffer);

   for( i=0; i < numss; i++ ) {
      // Source #
      fgets( buffer, sizeof(buffer), fp );
      sscanf( buffer, "%d", &snum );

      // Type, charge, latch
      fgets( buffer,sizeof(buffer), fp );
      pch = strtok( &buffer[0], " " );
      sscanf(pch, "%d", &type);
      pch = strtok( NULL, " " );
      sscanf(pch, "%d", &charge);
      pch = strtok( NULL, " " );
      sscanf(pch, "%d", &latch);
      my_source = my_model->get_source(i);
      if( my_source->verify_identity(type,charge,latch) ) {
	 fprintf( stderr, "Verification failed! Type:%d,charge:%d,latch:%d\n",type,charge,latch);
	 my_source->print();
	 exit(1);
      }
      my_source->add_energy_spectrum(Ebins,Emin,Emax);
      my_source->add_fluence_field( (fluence_field_type)FieldType, Fbins, IFdim, OFdim );

      // Parameters. Check them.
      fgets(buffer,sizeof(buffer), fp );
      for( j = 0; j < NUM_PARS; j++ )
	 parameters[j] = 0.0;
      j = 0;
      pch = strtok( &buffer[0], " " );
      do {
	 sscanf(pch, "%lf", &parameters[j]);
	 j++;
	 pch = strtok( NULL, " " );
      } while( pch && (j < NUM_PARS) );
      if( my_source->check_pars(parameters) ) {
	 fprintf( stderr, "Check failed! Parameters not the same\n");
	 my_source->print();
	 exit(1);
      }

      // Relative intensity
      fgets(buffer,sizeof(buffer), fp );
      pch = strtok( &buffer[0], " " );
      sscanf(pch, "%lf", &relint );
      my_source->put_relint(relint);

      // Number of energy bins, Emin, Emax
      fgets(buffer,sizeof(buffer), fp );
      pch = strtok( &buffer[0], " " );
      sscanf(pch, "%d", &Ebins);
      pch = strtok( NULL, " " );
      sscanf(pch, "%lf", &Emin);
      pch = strtok( NULL, " " );
      sscanf(pch, "%lf", &Emax);

      if( my_source->verify_energy_spectrum(Ebins,Emin,Emax) ) {
	 fprintf( stderr, "Verification failed! Ebins:%d,Emin:%lf,Emax:%lf\n",Ebins,Emin,Emax);
	 exit(1);
      }

      // Energy spectrum inside the treatment field
      my_energy_spectrum = my_source->get_energy_spectrum();
      for( j = 0; j < Ebins; ) {
	fgets(buffer,sizeof(buffer), fp );
	pch = strtok( &buffer[0], " " );
	while( pch != NULL ) {
	   sscanf(pch, "%lf", &E);
	   my_energy_spectrum->put_value( E_IN, E, j );
	   j++;
	   pch = strtok( NULL, " " );
	}
      }

      // Energy spectrum outside the treatment field
      for( j = 0; j < Ebins; ) {
        fgets(buffer,sizeof(buffer), fp );
        pch = strtok( &buffer[0], " " );
        while( pch != NULL ) {
           sscanf(pch, "%lf", &E);
	   my_energy_spectrum->put_value( E_OUT, E, j );
           j++;
           pch = strtok( NULL, " " );
        }
      }

      // Field type
      fgets( buffer, sizeof(buffer), fp );
      sscanf( buffer, "%d", &FieldType );

      // Number of fluence bins
      fgets(buffer,sizeof(buffer), fp );
      pch = strtok( &buffer[0], " " );
      sscanf(pch, "%d", &Fbins);
      if( FieldType < 3 ) {
	pch = strtok( NULL, " " );
        sscanf(pch, "%lf", &OFdim);
      } else OFdim = 0;
      if( my_source->verify_fluence_field((fluence_field_type)FieldType, Fbins, OFdim) ) {
	 fprintf( stderr, "Verification failed! FieldType:%d,Fbins:%d,OFdim:%lf\n", FieldType, Fbins, OFdim );
	 exit(1);
      }

      // Planar Fluence distribution for the sub-source
      for( j = 0; j < Fbins; ) {
        fgets(buffer,sizeof(buffer), fp );
        pch = strtok( &buffer[0], " " );
        while( pch != NULL ) {
           sscanf(pch, "%lf", &F);
/////	   my_source->add_binValue( F_IN, F );
           j++;
           pch = strtok( NULL, " " );
        }
      }

      if( FieldType == 1 ) {
	// Parameters required to correct the variation of planar fluence
	for( j = 0; j < Fbins; ) {
	   fgets(buffer,sizeof(buffer), fp );
	   pch = strtok( &buffer[0], " " );
	   while( pch != NULL ) {
	      sscanf(pch, "%lf", &F);
	      j++;
////	      my_source->add_binValue( F_OUT, F );
	      pch = strtok( NULL, " " );
	   }
	}
        // Parameters required to correct the variation of planar fluence
        for( j = 0; j < Fbins; ) {
           fgets(buffer,sizeof(buffer), fp );
           pch = strtok( &buffer[0], " " );
           while( pch != NULL ) {
              sscanf(pch, "%lf", &F);
/////	      my_source->add_binValue( ANG, F );
	      j++;
              pch = strtok( NULL, " " );
           }
        }

      }
   }
}

