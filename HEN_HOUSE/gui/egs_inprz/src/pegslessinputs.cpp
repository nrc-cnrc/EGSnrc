/*
###############################################################################
#
#  EGSnrc egs_inprz pegsless input
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
#  Author:          Blake Walters, 2013
#
#  Contributors:
#
###############################################################################
#
#  This file was originally derived from the file mcinputs.cpp written by
#  Ernesto Mainegra-Hing, starting in 2001.
#
###############################################################################
*/


#include "pegslessinputs.h"
//Added by qt3to4:
//#include <Q3TextStream>
//qt3to4 -- BW
#include <QTextStream>

PEGSLESSInputs::PEGSLESSInputs()
{


  AE="";
  UE="";
  AP="";
  UP="";

  matdatafile="";

  ninpmedia=0;

  inpmedind=-1;

}

PEGSLESSInputs::~PEGSLESSInputs()
{
	//free anything created in the constructor here
}


std::ifstream & operator >> ( std::ifstream & in, PEGSLESSInputs*  rPEGSLESS )
{
  std::vector<string> codes,med_delims;
  codes.push_back("AE");
  codes.push_back("UE");
  codes.push_back("AP");
  codes.push_back("UP");
  codes.push_back("material data file");
  med_delims.push_back(":start");
  med_delims.push_back(":stop");

  //possible inputs for media in .egsinp file
  std::vector<string> codes1;
  codes1.push_back("elements");
  codes1.push_back("no. of atoms");
  codes1.push_back("mass fractions");
  codes1.push_back("rho");
  codes1.push_back("stopping powers");
  codes1.push_back("bremsstrahlung correction");
  codes1.push_back("gas pressure");
  codes1.push_back("density correction file");
  codes1.push_back("sterncid");

  DE_Parser *p = new DE_Parser(codes,0,"media definition", in, false);

  rPEGSLESS->AE         = getIt( codes[0] , "", rPEGSLESS->errors, p );
  rPEGSLESS->UE         = getIt( codes[1] , "", rPEGSLESS->errors, p );
  rPEGSLESS->AP         = getIt( codes[2] ,"", rPEGSLESS->errors, p );
  rPEGSLESS->UP        = getIt( codes[3] ,"", rPEGSLESS->errors, p );
  rPEGSLESS->matdatafile         = getIt( codes[4] ,"", rPEGSLESS->errors, p );

  //now search for media defined in the .egsinp file
  //set multiple entries to true to allow for multiple media defined
  DE_Parser *p2 = new DE_Parser(med_delims,0,"media definition", in, true);

  bool loop=true;
  while (loop) {

     //qt3to4 -- BW
     //string temp_start = getIt( codes[5] ,"", rPEGSLESS->errors, p );
     //string temp_stop = getIt( codes[6] ,"", rPEGSLESS->errors, p );
      string temp_start = getIt( med_delims[0] ,"", rPEGSLESS->errors, p2 ).toStdString();
     string temp_stop = getIt( med_delims[1] ,"", rPEGSLESS->errors, p2 ).toStdString();
  //now strip trailing : off the strings and compare

     if(temp_start=="") break;

     for(int i=temp_start.size(); i>-1; i--) {
       if (temp_start[i]==':'){
         temp_start.erase(i,1);
         break;
       }
     }
     for(int i=temp_stop.size(); i>-1; i--) {
       if (temp_stop[i]==':'){
         temp_stop.erase(i,1);
         break;
       }
     }

     if(temp_start==temp_stop) {
    //look for inputs for this medium

       rPEGSLESS->ninpmedia++;
       int tempint = rPEGSLESS->ninpmedia-1;
       //qt3to4 -- BW
       //rPEGSLESS->inpmedium[tempint]=temp_start;
       rPEGSLESS->inpmedium[tempint]=QString::fromStdString(temp_start);

       //get a new block of the input file using new delimiters

       char temp_delim[120];//note this defines a max. length for medium name of 120 chars
       strcpy(temp_delim,temp_start.c_str());

       DE_Parser *p1 = new DE_Parser(codes1,0,temp_delim, in, false);

       //define defaults

       rPEGSLESS->elements[tempint].push_back("");
       rPEGSLESS->pz_or_rhoz[tempint].push_back("");
       rPEGSLESS->spec_by_pz[tempint]=true;
       rPEGSLESS->isgas[tempint]=false;

       rPEGSLESS->elements[tempint]=getThemAll( codes1[0] , rPEGSLESS->elements[tempint], rPEGSLESS->errors, p1 );

       if(rPEGSLESS->elements[tempint][0]!="") {
        //see if composition defined
         rPEGSLESS->nelements[tempint]=rPEGSLESS->elements[tempint].size();
         rPEGSLESS->pz_or_rhoz[tempint]=getThemAll( codes1[1] , rPEGSLESS->pz_or_rhoz[tempint], rPEGSLESS->errors, p1 );
         if(rPEGSLESS->pz_or_rhoz[tempint][0]=="") {
           rPEGSLESS->pz_or_rhoz[tempint]=getThemAll( codes1[2] , rPEGSLESS->pz_or_rhoz[tempint], rPEGSLESS->errors, p1 );
           if(rPEGSLESS->pz_or_rhoz[tempint][0]!="") rPEGSLESS->spec_by_pz[tempint]=false;
         }
       }

       rPEGSLESS->rho[tempint]=getIt( codes1[3] , "", rPEGSLESS->errors, p1 );
       rPEGSLESS->spr[tempint]=getIt( codes1[4] , "restricted total", rPEGSLESS->errors, p1 );
       rPEGSLESS->bc[tempint]=getIt( codes1[5] , "KM", rPEGSLESS->errors, p1 );
       rPEGSLESS->gasp[tempint]=getIt( codes1[6] , "", rPEGSLESS->errors, p1 );
       if(rPEGSLESS->gasp[tempint]!="" && rPEGSLESS->gasp[tempint].toFloat()>0.0) rPEGSLESS->isgas[tempint]=true;
       rPEGSLESS->dffile[tempint]=getIt( codes1[7] , "", rPEGSLESS->errors, p1 );
       rPEGSLESS->sterncid[tempint]=getIt( codes1[8] , "", rPEGSLESS->errors, p1 );

     }

  }

  rPEGSLESS->errors = QString::null;
  delete p;

  return in;
}

/*
Q3TextStream & operator << ( Q3TextStream & t, PEGSLESSInputs * rPEGSLESS )
{

  print_delimeter( "start" , "media definition", t);


  if(rPEGSLESS->AE!="") t << "AE= " << rPEGSLESS->AE << "\n";
  if(rPEGSLESS->UE!="") t << "UE= " << rPEGSLESS->UE << "\n";
  if(rPEGSLESS->AP!="") t << "AP= " << rPEGSLESS->AP << "\n";
  if(rPEGSLESS->UP!="") t << "UP= " << rPEGSLESS->UP << "\n" ;
  if(rPEGSLESS->matdatafile !="") t << "material data file= " << rPEGSLESS->matdatafile << "\n";

  t << "\n";

  for (int i=0; i<rPEGSLESS->ninpmedia; i++) {
     t << ":start " << rPEGSLESS->inpmedium[i] << ":\n";
     for(int j=0; j<rPEGSLESS->nelements[i]; j++){
       if(j==0) t << "elements= ";
       t << rPEGSLESS->elements[i][j];
       if(j<rPEGSLESS->nelements[i]-1) t << ",";
       else t << "\n";
     }
     for(int j=0; j<rPEGSLESS->nelements[i]; j++){
       if(j==0) {
        if(!rPEGSLESS->spec_by_pz[i]) t << "mass fractions= ";
        else t << "no. of atoms= ";
       }
       t << rPEGSLESS->pz_or_rhoz[i][j];
       if(j<rPEGSLESS->nelements[i]-1) t << ",";
       else t << "\n";
     }
     if(rPEGSLESS->rho[i]!="") t << "rho= " << rPEGSLESS->rho[i] << "\n";
     if(rPEGSLESS->spr[i]!="") t << "stopping powers= " << rPEGSLESS->spr[i] << "\n";
     if(rPEGSLESS->bc[i]!="") t << "bremsstrahlung correction= " << rPEGSLESS->bc[i] << "\n";
     if(rPEGSLESS->gasp[i]!="") t << "gas pressure= " << rPEGSLESS->gasp[i] << "\n";
     if(rPEGSLESS->dffile[i]!="") t << "density correction file= " << rPEGSLESS->dffile[i] << "\n";
     if(rPEGSLESS->sterncid[i]!="") t << "sterncid= " << rPEGSLESS->sterncid[i] << "\n";
     t << ":stop " << rPEGSLESS->inpmedium[i] << ":\n\n";
   }

  print_delimeter( "stop" , "media definition", t);

  return t;
}
*/

//qt3to4 -- BW
QTextStream & operator << ( QTextStream & t, PEGSLESSInputs * rPEGSLESS )
{

  print_delimeter( "start" , "media definition", t);


  if(rPEGSLESS->AE!="") t << "AE= " << rPEGSLESS->AE << "\n";
  if(rPEGSLESS->UE!="") t << "UE= " << rPEGSLESS->UE << "\n";
  if(rPEGSLESS->AP!="") t << "AP= " << rPEGSLESS->AP << "\n";
  if(rPEGSLESS->UP!="") t << "UP= " << rPEGSLESS->UP << "\n" ;
  if(rPEGSLESS->matdatafile !="") t << "material data file= " << rPEGSLESS->matdatafile << "\n";

  t << "\n";

  for (int i=0; i<rPEGSLESS->ninpmedia; i++) {
     t << ":start " << rPEGSLESS->inpmedium[i] << ":\n";
     for(int j=0; j<rPEGSLESS->nelements[i]; j++){
       if(j==0) t << "elements= ";
       t << rPEGSLESS->elements[i][j];
       if(j<rPEGSLESS->nelements[i]-1) t << ",";
       else t << "\n";
     }
     for(int j=0; j<rPEGSLESS->nelements[i]; j++){
       if(j==0) {
        if(!rPEGSLESS->spec_by_pz[i]) t << "mass fractions= ";
        else t << "no. of atoms= ";
       }
       t << rPEGSLESS->pz_or_rhoz[i][j];
       if(j<rPEGSLESS->nelements[i]-1) t << ",";
       else t << "\n";
     }
     if(rPEGSLESS->rho[i]!="") t << "rho= " << rPEGSLESS->rho[i] << "\n";
     if(rPEGSLESS->spr[i]!="") t << "stopping powers= " << rPEGSLESS->spr[i] << "\n";
     if(rPEGSLESS->bc[i]!="") t << "bremsstrahlung correction= " << rPEGSLESS->bc[i] << "\n";
     if(rPEGSLESS->gasp[i]!="" && rPEGSLESS->gasp[i].toFloat()>0.0 && rPEGSLESS->isgas[i]) t << "gas pressure= " << rPEGSLESS->gasp[i] << "\n";
     if(rPEGSLESS->dffile[i]!="") t << "density correction file= " << rPEGSLESS->dffile[i] << "\n";
     if(rPEGSLESS->sterncid[i]!="") t << "sterncid= " << rPEGSLESS->sterncid[i] << "\n";
     t << ":stop " << rPEGSLESS->inpmedium[i] << ":\n\n";
   }

  print_delimeter( "stop" , "media definition", t);

  return t;
}
