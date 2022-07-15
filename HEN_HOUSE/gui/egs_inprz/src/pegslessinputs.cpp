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
#  Contributors:    Frederic Tessier
#                   Ernesto Mainegra-Hing
#
###############################################################################
#
#  This file was originally derived from the file mcinputs.cpp written by
#  Ernesto Mainegra-Hing, starting in 2001.
#
###############################################################################
*/


#include "pegslessinputs.h"
#include <QTextStream>
#include <QMessageBox>

QStringList element_list;

int n_element=100;

Element element_data[] = {
    {1,"H",1.0079,19.2,8.3748e-05},
    {2,"He",4.0026,41.8,0.00016632},
    {3,"Li",6.941,40,0.534},
    {4,"Be",9.0122,63.7,1.848},
    {5,"B",10.812,76,2.37},
    {6,"C",12.011,78,2},
    {7,"N",14.0067,82,0.0011653},
    {8,"O",15.9994,95,0.0013315},
    {9,"F",18.9984,115,0.0015803},
    {10,"Ne",20.1797,137,0.0008385},
    {11,"Na",22.9898,149,0.971},
    {12,"Mg",24.305,156,1.74},
    {13,"Al",26.9815,166,2.702},
    {14,"Si",28.0855,173,2.33},
    {15,"P",30.9738,173,2.2},
    {16,"S",32.066,180,2},
    {17,"Cl",35.4527,174,0.0029947},
    {18,"Ar",39.948,188,0.001662},
    {19,"K",39.0983,190,0.862},
    {20,"Ca",40.078,191,1.55},
    {21,"Sc",44.9559,216,2.989},
    {22,"Ti",47.88,233,4.54},
    {23,"V",50.9415,245,6.11},
    {24,"Cr",51.9961,257,7.18},
    {25,"Mn",54.938,272,7.44},
    {26,"Fe",55.847,286,7.874},
    {27,"Co",58.9332,297,8.9},
    {28,"Ni",58.69,311,8.902},
    {29,"Cu",63.546,322,8.96},
    {30,"Zn",65.39,330,7.133},
    {31,"Ga",69.723,334,5.904},
    {32,"Ge",72.61,350,5.323},
    {33,"As",74.9216,347,5.73},
    {34,"Se",78.96,348,4.5},
    {35,"Br",79.904,357,0.0070722},
    {36,"Kr",83.8,352,0.0034783},
    {37,"Rb",85.4678,363,1.532},
    {38,"Sr",87.62,366,2.54},
    {39,"Y",88.9059,379,4.469},
    {40,"Zr",91.224,393,6.506},
    {41,"Nb",92.9064,417,8.57},
    {42,"Mo",95.94,424,10.22},
    {43,"Tc",97.9072,428,11.5},
    {44,"Ru",101.07,441,12.41},
    {45,"Rh",102.906,449,12.41},
    {46,"Pd",106.42,470,12.02},
    {47,"Ag",107.868,470,10.5},
    {48,"Cd",112.411,469,8.65},
    {49,"In",114.82,488,7.31},
    {50,"Sn",118.71,488,7.31},
    {51,"Sb",121.75,487,6.691},
    {52,"Te",127.6,485,6.24},
    {53,"I",126.904,491,4.93},
    {54,"Xe",131.29,482,0.0054854},
    {55,"Cs",132.905,488,1.873},
    {56,"Ba",137.327,491,3.5},
    {57,"La",138.905,501,6.154},
    {58,"Ce",140.115,523,6.657},
    {59,"Pr",140.908,535,6.71},
    {60,"Nd",144.24,546,6.9},
    {60,"Nd",144.24,546,6.9},
    {61,"Pm",144.913,560,7.22},
    {62,"Sm",150.36,574,7.46},
    {63,"Eu",151.965,580,5.243},
    {64,"Gd",157.25,591,7.9004},
    {65,"Tb",158.925,614,8.229},
    {66,"Dy",162.5,628,8.55},
    {67,"Ho",164.93,650,8.795},
    {68,"Er",167.26,658,9.066},
    {69,"Tm",168.934,674,9.321},
    {70,"Yb",173.04,684,6.73},
    {71,"Lu",174.967,694,9.84},
    {72,"Hf",178.49,705,13.31},
    {73,"Ta",180.948,718,16.654},
    {74,"W",183.85,727,19.3},
    {75,"Re",186.207,736,21.02},
    {76,"Os",190.2,746,22.57},
    {77,"Ir",192.22,757,22.42},
    {78,"Pt",195.08,790,21.45},
    {79,"Au",196.966,790,19.32},
    {80,"Hg",200.59,800,13.546},
    {81,"Tl",204.383,810,11.72},
    {82,"Pb",207.2,823,11.34},
    {83,"Bi",208.98,823,9.747},
    {84,"Po",208.982,830,9.32},
    {85,"At",209.987,825,9.32},
    {86,"Rn",222.018,794,0.0090662},
    {87,"Fr",223.02,827,1},
    {88,"Ra",226.025,826,5},
    {89,"Ac",227.028,841,10.07},
    {90,"Th",232.038,847,11.72},
    {91,"Pa",231.036,878,15.37},
    {92,"U",238.029,890,18.95},
    {93,"Np",237.048,902,20.25},
    {94,"Pu",239.052,921,19.84},
    {95,"Am",243.061,934,13.67},
    {96,"Cm",247.07,939,13.51},
    {97,"Bk",247.07,952,14},
    {98,"Cf",251.08,966,10},
    {99,"Es",252.083,980,10},
    {100,"Fm",257.095,994,10}
};

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
       rPEGSLESS->inpmedium[tempint]=QString::fromStdString(temp_start);

       //get a new block of the input file using new delimiters

       char temp_delim[120];//note this defines a max. length for medium name of 120 chars
       strcpy(temp_delim,temp_start.c_str());

       DE_Parser *p1 = new DE_Parser(codes1,0,temp_delim, in, false);

       //define defaults

       rPEGSLESS->elements[tempint].push_back("");
       rPEGSLESS->nelements[tempint]=0;
       rPEGSLESS->spec_by_pz[tempint]=true;
       rPEGSLESS->isgas[tempint]=false;
       rPEGSLESS->use_dcfile[tempint]=false;
       rPEGSLESS->rho_scale[tempint]=1.;

       rPEGSLESS->elements[tempint]=getThemAll( codes1[0] , rPEGSLESS->elements[tempint], rPEGSLESS->errors, p1 );

       if(rPEGSLESS->elements[tempint][0]!="") {
        //see if composition defined
         rPEGSLESS->nelements[tempint]=rPEGSLESS->elements[tempint].size();
         //initialize composition with blanks
         for(int i=0; i<rPEGSLESS->nelements[tempint]; i++) rPEGSLESS->pz_or_rhoz[tempint].push_back("");
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
       //will check if we can actually read from the DC file later
       //for now assume if a density correction file is specified it will
       //be used to determine comp
       if(!rPEGSLESS->dffile[tempint].isEmpty()) rPEGSLESS->use_dcfile[tempint]=true;
       rPEGSLESS->sterncid[tempint]=getIt( codes1[8] , "", rPEGSLESS->errors, p1 );

     }

  }

  rPEGSLESS->errors = QString::null;
  delete p;

  return in;
}

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
   if(rPEGSLESS->use_dcfile[i] && !rPEGSLESS->dffile[i].isEmpty()) {
     //medium composition and rho based on density correction file
     t << "density correction file= " << rPEGSLESS->dffile[i] << "\n";
   }
   else {
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
       QString qs_pz_or_rhoz=QString::fromStdString(rPEGSLESS->pz_or_rhoz[i][j]);
       float fl_pz_or_rhoz=qs_pz_or_rhoz.toFloat();
       int i_pz_or_rhoz=(int)fl_pz_or_rhoz;
       if(rPEGSLESS->spec_by_pz[i]){
           if (fl_pz_or_rhoz-i_pz_or_rhoz>0) {
             QString error = "Composition of med " + QString::number(i+1) +
                             " specified by no. of atoms but" +
                             " non-integer no. input.  Number will" +
                             " be truncated in .egsinp file.";
             QMessageBox::warning(0,"Warning",error,1,0,0);
          }
          t << i_pz_or_rhoz;
       }
       else t << fl_pz_or_rhoz;
       if(j<rPEGSLESS->nelements[i]-1) t << ",";
       else t << "\n";
     }
     if(rPEGSLESS->rho[i]!="") t << "rho= " << rPEGSLESS->rho_scale[i]*rPEGSLESS->rho[i].toFloat() << "\n";
     if(rPEGSLESS->gasp[i]!="" && rPEGSLESS->gasp[i].toFloat()>0.0 && rPEGSLESS->isgas[i]) t << "gas pressure= " << rPEGSLESS->gasp[i] << "\n";
   }
   //NB: Certain parameters below are either redundant/unnecessary
   //will be commented out but coding kept just in case
   //  if(rPEGSLESS->spr[i]!="") t << "stopping powers= " << rPEGSLESS->spr[i] << "\n";

     if(rPEGSLESS->bc[i]!="") t << "bremsstrahlung correction= " << rPEGSLESS->bc[i] << "\n";

     //if(rPEGSLESS->sterncid[i]!="") t << "sterncid= " << rPEGSLESS->sterncid[i] << "\n";
     t << ":stop " << rPEGSLESS->inpmedium[i] << ":\n\n";
   }

  print_delimeter( "stop" , "media definition", t);

  return t;
}
