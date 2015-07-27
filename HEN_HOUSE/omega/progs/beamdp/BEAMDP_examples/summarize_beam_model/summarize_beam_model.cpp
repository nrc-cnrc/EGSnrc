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
#
#  This program reads a .source file produced by beamdp (i.e., a multiple
#  source model) and outputs a readable summary of the model.
#
###############################################################################
*/


#include <iostream>
#include <fstream>
#include "unistd.h"
#include "msmodel.h"
#include "sources.h"

using namespace std;

#define NUM_PARS 8
#define V2_0	 2

typedef struct {
   short int version;	// version: 2-new model, 0-old model
   char title[120];
   char phspFname[120];
   int numss;
   struct {
     int type;
     int bins,nradii;
     double Emin, Emax, eRadii[BUFSIZE];
     double EnTrField, EnScoField;
   } EnergySpectrum;
   struct {
     int type;
     int bins, TrFieldbins, ScoFieldbins;
     double FlTrField, FlScoField;
   } Field;
   struct {
     int type, charge, latch;
     double parameters[NUM_PARS];
     double relint,vSSD;
     double Energy[BUFSIZE][BUFSIZE];
     double Fluence[BUFSIZE][3];
   } subsource[MAX_NUM_SOURCES];
   double Angular[BUFSIZE];
} model_struct;

static void
usage(const char *cmd) {
fprintf(stderr, "Usage: %s [-hvdi] source_file\n", cmd);
fputs("\t-h\tgives this help display\n",stderr);
fputs("\t-v\tverbose print\n",stderr);
fputs("\t-p\tprint source parameters\n",stderr);
fputs("\t-f\tprint fluence distribution\n",stderr);
fputs("\t-e\tprint energy distribution\n",stderr);
fputs("\t-m\tprint mean energies\n",stderr);
fputs("\t-o\topen OLD beamdp file\n",stderr);
}

int read_stream_newModel(char *, model_struct *);
int read_stream_oldModel(char *, model_struct *);
void load_model(model_struct *, ms_model *);
void plot(void);
int verbose;

int
main(int argc,char **argv)
{
   int optch, ret;
   bool print_parameters = false, print_energy_spectrum = false, print_fluence_dist = false, print_mean_energies = false;
   bool old_source_file = false;
   static char optstring[]="hvpefmo";

   model_struct model;
   ms_model *my_model = new ms_model();;

   while( (optch = getopt(argc,argv,optstring)) != -1 ) {
      switch( optch ) {
	case 'v':
	   verbose = 1;	// Verbose print
	   break;
	case 'p':
	   print_parameters = true;
	   break;
	case 'f':
	   print_fluence_dist = true;
	   break;
	case 'e':
	   print_energy_spectrum = true;
	   break;
	case 'm':
	   print_mean_energies = true;
	   break;
	case 'o':
	   old_source_file = true;
	   break;
	case 'h':
	default:
	   usage(argv[0]);
	   exit(1);
      }
   }

   if( argc > optind+1 ) {
      usage(argv[0]);
      exit( -1 );
   } else if( argc = optind + 1 ) {
      if ( !old_source_file )
         ret = read_stream_newModel(argv[optind], &model);
      else
	 ret = read_stream_oldModel(argv[optind], &model);
      if( ret == 0 ) {
	 cout << "\n'" << argv[optind] << "' is " <<
	 (model.version == 2 ? "a NEW" : "an OLD") <<
	 " source model\n";
	 load_model(&model, my_model);
	 my_model->print();
	 cout << " --- Total intensity: " << my_model->total_intensity() << endl;
	 cout << " --- (electrons: " << my_model->total_Particleintensity(-1);
	 cout << " photons: " << my_model->total_Particleintensity(0);
	 cout << " positrons: " << my_model->total_Particleintensity(1);
	 cout << " )" << endl;

	 if( print_parameters )
	    my_model->print_parameters();
	 if( print_fluence_dist )
	    my_model->print_fluence_dist();
	 if( print_energy_spectrum )
	    my_model->print_energy_spectrum();
	 if( print_mean_energies )
	    my_model->print_mean_energies();
      }
      else if( ret == -1 ) {
	cout << "Cannot open file '" << argv[optind] << "'\n";
	exit(-1);
      }
   }
}

int
read_stream_newModel(char *fname, model_struct *model)
{
   int numpars, error = 0;
   static model_struct temp;

   ifstream ins;

   ins.open( fname );
   if( !ins )
      return -1;

   try {
      // Title
      ins.getline(model->title, sizeof(model->title) );
      if( verbose )
	 cout << "Title: " << model->title << endl;

      // Number of sources
      ins >> model->numss;
      if( verbose )
         cout << "Number of sources: " << model->numss << endl;

      for( int i=0; i < model->numss; i++ ) {
	 // Type, charge, latch
	 ins >> model->subsource[i].type;
	 ins >> model->subsource[i].charge,
	 ins >> model->subsource[i].latch;

	 if( verbose ) {
	    cout << "Type:" << model->subsource[i].type;
	    cout << ",charge:" << model->subsource[i].charge;
	    cout << ",latch:" << model->subsource[i].latch << endl;
	 }

	 switch( model->subsource[i].type ) {
	   case APPLICATOR:
	       numpars = 7;
	       break;
	   case COLLIMATOR:
	       numpars = 8;
	       break;
	   case RING:
	       numpars = 4;
	       break;
	   case RECTPLANE:
	       numpars = 5;
	       break;
	   case CIRCPLANE:
	       numpars = 2;
	       break;
	   default:
	       numpars = 0;
	       cout << "\nError: wrong sub-source type" << endl;
	       return -2;
	 }
	 for( int j = 0; j < numpars; j++ )
	   ins >> model->subsource[i].parameters[j];
	 if( verbose ) {
	    cout << "Parameters: ";
	    for( int j = 0; j < numpars; j++ )
		cout << model->subsource[i].parameters[j] << " ";
	    cout << endl;
	 }
      }

      // Number of energy bins, Emin, Emax
      ins >> model->EnergySpectrum.bins;
      ins >> model->EnergySpectrum.Emin;
      ins >> model->EnergySpectrum.Emax;

      // Energy spectrum type
      ins >> model->EnergySpectrum.type;

      if( model->EnergySpectrum.type == 0 ) {
	 // EnTrField, EnScoField
	 ins >> model->EnergySpectrum.EnTrField;
	 ins >> model->EnergySpectrum.EnScoField;
	 model->EnergySpectrum.nradii = 2;
	 model->EnergySpectrum.eRadii[0] = model->EnergySpectrum.EnTrField;
	 model->EnergySpectrum.eRadii[1] = model->EnergySpectrum.EnScoField;
      }
      else {
	 // Number of energy radii
	 ins >> model->EnergySpectrum.nradii;
	 ins >> model->EnergySpectrum.EnScoField;
	 for( int j = 0; j < model->EnergySpectrum.nradii; j++ ) {
	    ins >> model->EnergySpectrum.eRadii[j];
	 }
      }
      if( verbose ) {
	 cout << "\nEnergy type: " << model->EnergySpectrum.type << endl;
	 cout << " Emin: " << model->EnergySpectrum.Emin << endl;
	 cout << " Emax: " << model->EnergySpectrum.Emax << endl;
	 cout << "\nEnergy treatment field: " << model->EnergySpectrum.EnTrField << endl;
	 cout << " Energy scoring field: " << model->EnergySpectrum.EnScoField << endl;
	 if( model->EnergySpectrum.type == 1 ) {
	    cout << " Number of energy radii: " << model->EnergySpectrum.nradii << endl;
	    for( int j = 0; j < model->EnergySpectrum.nradii; j++ ) {
	       cout << " R[" << j+1 << "]=" << model->EnergySpectrum.eRadii[j] <<
	       (j != model->EnergySpectrum.nradii-1 ? ",":"\n");
	    }
	 }
      }

      // Field type
      ins >> model->Field.type;
      ins >> model->Field.TrFieldbins;
      ins >> model->Field.FlTrField;
      ins >> model->Field.ScoFieldbins;
      ins >> model->Field.FlScoField;
      model->Field.bins = model->Field.TrFieldbins + model->Field.ScoFieldbins;
      if( verbose ) {
	 cout << "\nField type: " << model->Field.type << endl;
	 cout << " Tr field bins: " << model->Field.TrFieldbins << endl;
	 cout << " FlTrField: " << model->Field.FlTrField << endl;
	 cout << " Sco field bins: " << model->Field.ScoFieldbins << endl;
	 cout << " FlScoField: " << model->Field.FlScoField << endl;
      }

      // Phase-space file
      ins >> model->phspFname;
      if( verbose )
	 cout << "\nPhase-space file: " << model->phspFname << endl;

      for( int i=0; i < model->numss; i++ ) {
	 // Source number
	 ins >> temp.numss;
	 if( temp.numss != i+1 ) {
	    cout << "\nError: wrong sub-source number: " << temp.numss << endl;
	    return -2;
	 }
	 // Type, charge, latch
	 ins >> temp.subsource[i].type >> temp.subsource[i].charge >> temp.subsource[i].latch;
	 if(( temp.subsource[i].type != model->subsource[i].type ) ||
	    ( temp.subsource[i].charge != model->subsource[i].charge) ||
	    ( temp.subsource[i].latch != model->subsource[i].latch )) {
	    cout << "\nError: wrong sub-source information" << endl;
	    return -2;
	 }

	 switch( temp.subsource[i].type ) {
	   case APPLICATOR:
	       numpars = 7;
	       break;
	   case COLLIMATOR:
	       numpars = 8;
	       break;
	   case RING:
	       numpars = 4;
	       break;
	   case RECTPLANE:
	       numpars = 5;
	       break;
	   case CIRCPLANE:
	       numpars = 2;
	       break;
	   default:
	       numpars = 0;
	       cout << "\nError: wrong sub-source type" << endl;
	       return -2;
	 }
	 for( int j=0; j < numpars; j++ )
	   ins >> temp.subsource[i].parameters[j];
	 for( int j = 0; j < numpars; j++ ) {
	   if( temp.subsource[i].parameters[j] != model->subsource[i].parameters[j] ) {
	      cout << "\nError: wrong parameters for sub-source " << i+1 << endl;
	      cout << " Parameters: " << temp.subsource[i].parameters[j] << ", " \
	           << model->subsource[i].parameters[j]<< endl;
	      return -2;
	   }
	 }

	 // Relative intensity and virtual SSD
	 ins >> model->subsource[i].relint >> model->subsource[i].vSSD;
	 if( verbose ) {
	    cout << "Relative intensity for source " << i+1 <<": " << model->subsource[i].relint << endl;
	    cout << "Virtual SSD for source " << i+1 <<": " << model->subsource[i].vSSD << endl;
	 }

	 // Number of energy bins, Emin, Emax
	 ins >> temp.EnergySpectrum.bins;
	 ins >> temp.EnergySpectrum.Emin;
	 ins >> temp.EnergySpectrum.Emax;
	 if(( temp.EnergySpectrum.bins != model->EnergySpectrum.bins ) ||
	    ( temp.EnergySpectrum.Emin != model->EnergySpectrum.Emin) ||
	    ( temp.EnergySpectrum.Emax != model->EnergySpectrum.Emax )) {
	    cout << "\nError: wrong energy spectar information for subsource " << i+1 << endl;
	    return -2;
	 }
	 for( int j=0; j < model->EnergySpectrum.nradii; j++ ) {
	   for( int k=0; k < temp.EnergySpectrum.bins; k++ )
	      ins >> model->subsource[i].Energy[k][j];
	 }

	 // Field type
	 ins >> temp.Field.type;
	 ins >> temp.Field.TrFieldbins;
	 ins >> temp.Field.FlTrField;
	 ins >> temp.Field.ScoFieldbins;
	 ins >> temp.Field.FlScoField;
	 temp.Field.bins = temp.Field.TrFieldbins + temp.Field.ScoFieldbins;

	 if(( temp.Field.type != model->Field.type ) ||
	    ( temp.Field.TrFieldbins != model->Field.TrFieldbins ) ||
	    ( temp.Field.FlTrField != model->Field.FlTrField ) ||
	    ( temp.Field.ScoFieldbins != model->Field.ScoFieldbins ) ||
	    ( temp.Field.FlScoField != model->Field.FlScoField )) {
	    cout << "\nError: wrong field information for subsource " << i+1 << endl;
	    return -2;
	 }

	 // Planar Fluence distribution for the sub-source
	 switch( model->Field.type ) {
	    case CIRCULAR:
	    case RECTANGULAR:
	       for( int j=0; j < temp.Field.bins; j++)
		  ins >> model->subsource[i].Fluence[j][0];
	       break;
	    case SQUARE:
	       for( int j=0; j < temp.Field.bins; j++)
		  ins >> model->subsource[i].Fluence[j][0];
	       for( int j=0; j < temp.Field.bins; j++)
		  ins >> model->subsource[i].Fluence[j][1];
	       for( int j=0; j < temp.Field.bins; j++)
		  ins >> model->subsource[i].Fluence[j][2];
	       break;
	 }
      }
      for( int i = 0; i < temp.Field.bins; i++ )
	 ins >> model->Angular[i];

      model->version = V2_0;
   } catch( ... ) {
	cout << "Error in the source file";
	error = -2;
   }

   ins.close();
   return error;
}

int
read_stream_oldModel(char *fname, model_struct *model)
{
   int numpars, error = 0;
   static model_struct temp;

   ifstream ins;

   ins.open( fname );
   if( !ins )
      return -1;

   try {
      // Title
      ins.getline(model->title, sizeof(model->title) );
      if( verbose )
	 cout << "Title: " << model->title << endl;

      // Number of sources
      ins >> model->numss;
      if( verbose )
         cout << "Number of sources: " << model->numss << endl;

      for( int i=0; i < model->numss; i++ ) {
	 // Type, charge, latch
	 ins >> model->subsource[i].type;
	 ins >> model->subsource[i].charge,
	 ins >> model->subsource[i].latch;

	 if( verbose ) {
	    cout << "Type:" << model->subsource[i].type;
	    cout << ",charge:" << model->subsource[i].charge;
	    cout << ",latch:" << model->subsource[i].latch << endl;
	 }

	 switch( model->subsource[i].type ) {
	   case APPLICATOR:
	       numpars = 7;
	       break;
	   case COLLIMATOR:
	       numpars = 8;
	       break;
	   case RING:
	       numpars = 4;
	       break;
	   case RECTPLANE:
	       numpars = 5;
	       break;
	   case CIRCPLANE:
	       numpars = 2;
	       break;
	   default:
	       numpars = 0;
	       cout << "\nError: wrong sub-source type" << endl;
	       return -2;
	 }
	 for( int j = 0; j < numpars; j++ )
	   ins >> model->subsource[i].parameters[j];
	 if( verbose ) {
	    cout << "Parameters: ";
	    for( int j = 0; j < numpars; j++ )
		cout << model->subsource[i].parameters[j] << " ";
	    cout << endl;
	 }
      }

      // Number of energy bins, Emin, Emax
      ins >> model->EnergySpectrum.bins;
      ins >> model->EnergySpectrum.Emin;
      ins >> model->EnergySpectrum.Emax;
      if( verbose ) {
	 cout << "\nEnergy type: " << model->EnergySpectrum.type << endl;
	 cout << " Emin: " << model->EnergySpectrum.Emin << endl;
	 cout << " Emax: " << model->EnergySpectrum.Emax << endl;
      }

      // Field type
      ins >> model->Field.type;
      ins >> model->Field.bins;
      ins >> model->Field.FlTrField;
      ins >> model->Field.FlScoField;
      model->Field.TrFieldbins = model->Field.bins;
      model->Field.ScoFieldbins = 0;

      if( verbose ) {
	 cout << "\nField type: " << model->Field.type << endl;
	 cout << " Field bins: " << model->Field.bins << endl;
	 cout << " FlTrField: " << model->Field.FlTrField << endl;
	 cout << " FlScoField: " << model->Field.FlScoField << endl;
      }

      // Energy spectrum type is always o for the OLD source file
      model->EnergySpectrum.type = 0;
      model->EnergySpectrum.nradii = 2;
      model->EnergySpectrum.eRadii[0] = model->Field.FlTrField;
      model->EnergySpectrum.eRadii[1] = model->Field.FlScoField;

      // Phase-space file
      ins >> model->phspFname;
      if( verbose )
	 cout << "\nPhase-space file: " << model->phspFname << endl;

      for( int i=0; i < model->numss; i++ ) {
	 // Source number
	 ins >> temp.numss;
	 if( temp.numss != i+1 ) {
	    cout << "\nError: wrong sub-source number: " << temp.numss << endl;
	    return -2;
	 }
	 // Type, charge, latch
	 ins >> temp.subsource[i].type >> temp.subsource[i].charge >> temp.subsource[i].latch;
	 if(( temp.subsource[i].type != model->subsource[i].type ) ||
	    ( temp.subsource[i].charge != model->subsource[i].charge) ||
	    ( temp.subsource[i].latch != model->subsource[i].latch )) {
	    cout << "\nError: wrong sub-source information" << endl;
	    return -2;
	 }

	 switch( temp.subsource[i].type ) {
	   case APPLICATOR:
	       numpars = 7;
	       break;
	   case COLLIMATOR:
	       numpars = 8;
	       break;
	   case RING:
	       numpars = 4;
	       break;
	   case RECTPLANE:
	       numpars = 5;
	       break;
	   case CIRCPLANE:
	       numpars = 2;
	       break;
	   default:
	       numpars = 0;
	       cout << "\nError: wrong sub-source type" << endl;
	       return -2;
	 }
	 for( int j=0; j < numpars; j++ )
	   ins >> temp.subsource[i].parameters[j];
	 for( int j = 0; j < numpars; j++ ) {
	   if( temp.subsource[i].parameters[j] != model->subsource[i].parameters[j] ) {
	      cout << "\nError: wrong parameters for sub-source " << i+1 << endl;
	      cout << " Parameters: " << temp.subsource[i].parameters[j] << ", " \
	           << model->subsource[i].parameters[j]<< endl;
	      return -2;
	   }
	 }

	 // Relative intensity and virtual SSD
	 ins >> model->subsource[i].relint >> model->subsource[i].vSSD;
	 if( verbose ) {
	    cout << "Relative intensity for source " << i+1 <<": " << model->subsource[i].relint << endl;
	    cout << "Virtual SSD for source " << i+1 <<": " << model->subsource[i].vSSD << endl;
	 }

	 // Number of energy bins, Emin, Emax
	 ins >> temp.EnergySpectrum.bins;
	 ins >> temp.EnergySpectrum.Emin;
	 ins >> temp.EnergySpectrum.Emax;
	 if(( temp.EnergySpectrum.bins != model->EnergySpectrum.bins ) ||
	    ( temp.EnergySpectrum.Emin != model->EnergySpectrum.Emin) ||
	    ( temp.EnergySpectrum.Emax != model->EnergySpectrum.Emax )) {
	    cout << "\nError: wrong energy spectar information for subsource " << i+1 << endl;
	    return -2;
	 }
	 for( int j=0; j < model->EnergySpectrum.nradii; j++ ) {
	   for( int k=0; k < temp.EnergySpectrum.bins; k++ )
	      ins >> model->subsource[i].Energy[k][j];
	 }

	 // Field type
	 ins >> temp.Field.type;
	 ins >> temp.Field.bins;
	 ins >> temp.Field.FlScoField;

	 if(( temp.Field.type != model->Field.type ) ||
	    ( temp.Field.bins != model->Field.bins ) ||
	    ( temp.Field.FlScoField != model->Field.FlScoField )) {
	    cout << "\nError: wrong field information for subsource " << i+1 << endl;
	    return -2;
	 }

	 // Planar Fluence distribution for the sub-source
	 switch( model->Field.type ) {
	    case CIRCULAR:
	    case RECTANGULAR:
	       for( int j=0; j < temp.Field.bins; j++)
		  ins >> model->subsource[i].Fluence[j][0];
	       break;
	    case SQUARE:
	       for( int j=0; j < temp.Field.bins; j++)
		  ins >> model->subsource[i].Fluence[j][0];
	       for( int j=0; j < temp.Field.bins; j++)
		  ins >> model->subsource[i].Fluence[j][1];
	       for( int j=0; j < temp.Field.bins; j++)
		  ins >> model->subsource[i].Fluence[j][2];
	       break;
	 }
      }
      for( int i = 0; i < temp.Field.bins; i++ )
	 ins >> model->Angular[i];
   } catch( ... ) {
	cout << "Error in the source file";
	error = -2;
   }

   ins.close();
   return error;
}

void
load_model(model_struct *m, ms_model *my_model)
{
   source *s;
   fluence_field *f;
   energy_spectrum *e;

   my_model->put_title(m->title);
   my_model->put_phsp(m->phspFname);

   for( int i = 0; i < m->numss; i++ ) {
      s = my_model->add_source(m->subsource[i].type,
			   m->subsource[i].charge,
			   m->subsource[i].latch);

      s->put_relint(m->subsource[i].relint);
      s->put_pars(m->subsource[i].parameters);

      // Fluence
      f = s->add_fluence_field((fluence_field_type)m->Field.type,
			m->Field.TrFieldbins,
			m->Field.ScoFieldbins,
			m->Field.FlTrField,
			m->Field.FlScoField);
      for( int j = 0; j < f->get_bins(); j++ ) {
	 f->put_Fvalue(m->subsource[i].Fluence[j][0],j,0);
	 f->put_Fvalue(m->subsource[i].Fluence[j][1],j,1);
      }

      // Energy spectrum
      e = s->add_energy_spectrum(m->EnergySpectrum.type,
			m->EnergySpectrum.bins,
			m->EnergySpectrum.nradii,
			m->EnergySpectrum.Emin,
			m->EnergySpectrum.Emax);
      for( int j = 0; j < e->get_nradii(); j++ ) {
	 e->put_radius(m->EnergySpectrum.eRadii[j],j);
      }
      for( int j = 0; j < e->get_bins(); j++ ) {
	 for( int k = 0; k < e->get_nradii(); k++ ) {
	    e->put_Evalue(m->subsource[i].Energy[j][k],j,k);
	 }
      }
   }

   // All subsources are created, calculate relative intensity
   // for each particle type
   my_model->CalculateIntensity();
}

void
plot(void)
{
   #define PS_PATH = "/home/zsego/prog/C++/msconf"
   extern char **environ;	// Our environment array

   static char *argv[] = {"xmgrace", NULL};

   cout << "Ploting a function ...\n";
//   execve(PS_PATH,argv,environ);
}

