/*
###############################################################################
#
#  EGSnrc egs++ egs_cbct application utilities
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
#  Author:          Ernesto Mainegra-Hing, 2008
#
#  Contributors:
#
###############################################################################
*/


#include "egs_utils.h"
// Every C++ EGSnrc application needs this header file
#include "egs_interface2.h"
// We use the EGS_Input class
#include "egs_input.h"
// To get the maximum source energy
#include "egs_base_source.h"
// The random number generator
#include "egs_rndm.h"
// Transformations
#include "egs_transformations.h"
// Interpolators
#include "egs_interpolator.h"

#include <egs_run_control.h>

#include "egs_smoothing.h"

#ifdef DEBUG_WEIGHTS
inline void EGS_CBCT_Interactions::addInteraction(const EGS_Vector &x,
                                                  const EGS_Vector &u,
                                            EGS_Float E, EGS_Float wt,
                             int ir, int latch, int type,int N, int W)
{
   if( nnow >= n ) grow();
   i[nnow++] = Interaction(x,u,E,wt,ir,latch,type,N,W);
}

inline void EGS_CBCT_Interactions::addInteraction(const Interaction &I) {
   if( nnow >= n ) grow();
   i[nnow++] = I;
}

inline void EGS_CBCT_Interactions::grow() {
   int nnew = n > 0 ? 2*n : 10;
   Interaction *tmp = new Interaction[nnew];
   if( n > 0 ) {
       for(int j=0; j<nnow; j++) tmp[j] = i[j];
       delete [] i;
   }
   i = tmp; n = nnew;
}

void EGS_CBCT_Interactions::printInteractions() {
   egsInformation("Interactions:\n");
   for(int j=0; j<nnow; j++)
       egsInformation("%d: x=(%g,%g,%g) u=(%g,%g,%g) E=%g wt=%g"
           " ir=%d latch=%d type=%d nsplit=%d where=%d\n",j,
           i[j].x.x,i[j].x.y,i[j].x.z,i[j].u.x,i[j].u.y,i[j].u.z,
           i[j].E,i[j].wt,i[j].ir,i[j].latch,i[j].type,i[j].nsplit,
           i[j].where);
}
#endif

bool EGS_BaseFile::fileExist(const char *pName){
     fstream in; in.open(pName,ios::in | ios::binary);
     if(in.is_open()){in.close();return true;}
     return false;
}

bool EGS_BaseFile::fileExist(){return this->fileExist(name);};

EGS_I64 EGS_BaseFile::fileSize(){
     std::ifstream   in( name, ios::in | ios::binary );
     if ( !in ) {return 0;}
     EGS_I64 begin,end;
     begin = in.tellg();
     in.seekg (0, ios::end);
     end = in.tellg();
     in.seekg (0, ios::beg);
     return end - begin;
}

EGS_I64 EGS_BaseFile::fileSize(const char * n){
       name = const_cast<char *>(n);
       return this->fileSize();
}

EGS_BinaryFile::EGS_BinaryFile(const char *n):
 size(0), block(0)
{
   name = const_cast<char *> (n);
   size = fileSize();
}

EGS_BinaryFile::EGS_BinaryFile(const char *n, char *what):
 size(0), block(0)
{
   name = const_cast<char *> (n);
   size = fileSize();// actual file size
   block = new char[size]; block = what;
}

EGS_BinaryFile::EGS_BinaryFile(const char *n, const EGS_I64 &s, char *what):
 size(0), block(0)
{
   name = const_cast<char *> (n);
   size = s;// block size, not file size
   block = new char[size]; block = what;
}

void EGS_BinaryFile::fillWith(const EGS_I64 &s, const char *what){
      size =s*sizeof(float);block = new char[size];
      float item = atof(what);
      float *b = new float[s];
      for(EGS_I64 i=0;i<s;i++)
         b[i] = item;
      block = (char *)b;
      writeBinary(0);
}

void EGS_BinaryFile::writeBinary(const EGS_I64 &pos){

char *cptr = new char[size*sizeof(char)]; cptr = block;

/*  Had to open stream in in/out mode for this to work */
    ofstream o;
    if (fileExist(name))
     o.open( name, ios::in | ios::out | ios::binary);
    else
     o.open( name, ios::out | ios::binary);
    if ( !o ) {
      egsFatal("\n\n*** EGS_BinaryFile::writeBinary: \n"
                 "    Cannot open BINARY file: %s", name );
    }
    if (pos)
     o.seekp(pos,ios::beg);
    o.write(cptr, size);
    o.close();
}

void EGS_BinaryFile::writeBinary(char          *what,
                                 const EGS_I64 &s,
                                 const EGS_I64 &pos)
{

char *cptr = new char[s*sizeof(char)]; cptr = what;

/*  Had to open stream in in/out mode for this to work */
    ofstream o;
    if (fileExist(name))
     o.open( name, ios::in | ios::out | ios::binary);
    else
     o.open( name, ios::out | ios::binary);
    if ( !o ) {
      egsFatal("\n\n*** EGS_BinaryFile::writeBinary: \n"
                 "    Cannot open BINARY file: %s", name );
    }
    if (pos)
     o.seekp(pos,ios::beg);
    o.write(cptr, s);
    o.close();
}

void EGS_BinaryFile::readBinary(){

    std::ifstream   in( name, ios::in | ios::binary );
    if ( !in ) {
      cout << " EGS_BinaryFile::readBinary: " <<
              "Cannot open file: " << name << endl;
      return;
    }

  size = fileSize();

  char* b = new char[size];
  in.read(b, size);
  in.close();
  if (block) delete [] block;
  block = new char[size];block = b;
}

float * EGS_BinaryFile::readValues(){
  readBinary();
  EGS_I64 block_size = fileSize()/sizeof(float);
  float *b = new float[block_size];
  for(int i = 0; i < block_size; i++){
    b[i] = *((float*) &block[i*sizeof(float)]);
  }

  return b;
}

void EGS_BinaryFile::readValues(float *s, const int &_size){
  readBinary();
  EGS_I64 block_size = fileSize()/sizeof(float);
  if (block_size > _size){
     egsFatal("\n\n*** Fatal error!!!\n"
              " EGS_BinaryFile::readValues: not enough space allocated\n");
  }
  for(int i = 0; i < block_size; i++){
    s[i] = *((float*) &block[i*sizeof(float)]);
  }
}

/* print out a section of each block */
void EGS_BinaryFile::writeASCII(const int &chunk){

  readBinary();

  string the_name = name; the_name += ".txt";
  std::ofstream   out( the_name.c_str(), ios::out );
  if ( !out ) {
      cout << " EGS_BinaryFile::writeASCII: " <<
              "Cannot open file: " << name << endl;
      return;
  }


  int line = chunk > 80 ? 80: chunk;

  for(int i = 0; i < size/chunk; i++){
   for(int j = 0; j < line; j++){
      out << *((int*) &block[(j*sizeof(int)+i*chunk)]);
      cout << *((int*) &block[(j*sizeof(int)+i*chunk)]);

   }
   out << "\n\n"; cout << "\n";
  }

  out.close();

}

/* Define profile */
EGS_XYProfile::EGS_XYProfile(EGS_Input * inp,
                             EGS_Float rAx, EGS_Float rAy,
                             int rNx, int rNy):
    Nx(0), Ny(0), ax(0), ay(0), defined(false)
{
    if (inp) {

      /* scoring plane info */
      Nx = rNx; Ny = rNy; ax = rAx; ay = rAy;

      vector<string> output;
      output.push_back("off");
      output.push_back("regions");output.push_back("locations");
      icoord = inp->getInput("output by",output,1);
      if (!icoord) return;  // status set to false

      vector<string> scan;
      scan.push_back("x-scan");scan.push_back("y-scan");
      iscan = inp->getInput("scan type",scan,0);

      int err = 0;
      vector<EGS_Float> pos;
      err = inp->getInput("scan regions",pos);
      if (err) return; // status set to false
      position = pos;

      switch(icoord){
        case 1:
          for (int l = 0; l < pos.size(); l++){
              region.push_back((int)pos[l]);
          }
          break;
        case 2:
          for (int l = 0; l < pos.size(); l++){
            region.push_back(iscan ? get_j(pos[l]):get_i(pos[l]));
          }
          break;
        default:
          return;
      }
      defined = true;

    }

}

/* Describe profile */
void EGS_XYProfile::describeIt(){
   if (!defined){return;}
   string scanstr = "X-scan", coord = "y = ";
   if (iscan){ scanstr = "Y-scan";coord = "x = ";}
   string outby = "by regions";
   if (icoord == 2) outby = "by locations";
   egsInformation("\n\n==> %s %s requested at :\n\n",
                 scanstr.c_str(),outby.c_str());
   for (int l = 0; l < position.size(); l++){
       if (icoord == 2)
         egsInformation("%s %7.3f cm",coord.c_str(),position[l]);
       else
         egsInformation("%s %d",coord.c_str(),(int)position[l]);
   }
   egsInformation("\n\n");
}

/* Save profile */
void EGS_XYProfile::saveProfile(const string pName,
                                const vector<string> prof){
   if (!defined){
     egsInformation("No profile defined, nothing to be done!");
     return;
   }
   ofstream out(pName.c_str());
   int N; EGS_Float a;string header;string xscan = iscan ? "  y ":"  x ";
   if (iscan){N = Ny;a = ay;header="          y-scan at x = ";}
   else      {N = Nx;a = ax;header="          x-scan at y = ";}
   string **map = getProfile(prof);
   vector<EGS_Float> coord = getPositions(N, a);
   char buf[8192];
   for(int i = 0; i < region.size(); i++){
     sprintf(buf,"%7.3f",position[i]);
     out << header.c_str() << buf <<  "\n\n\n";
     out << xscan << "    reg#     Ktot                       Katt" <<
            "                       Ascat " << "\n\n";
     for(int j = 0; j < N; j++){
       sprintf(buf,"  %3d",(int)coord[j]);
       if (icoord == 2) sprintf(buf,"%7.3f",coord[j]);
       out << buf <<" " << map[i][j];
     }
   }
}

/* Save projection */
void EGS_XYProfile::saveProjection(const string pName,
                                const EGS_Float *proj){
    std::ofstream o( pName.c_str(), ios::out|ios::binary);
    if ( !o ) {
      cout << " EGS_XYProfile::saveProjection: " <<
              "Cannot open BINARY file: " << pName.c_str() << endl;
      return;
    }
    char *block = new char[Nx*Ny*sizeof(float)];
          block = (char *) proj;
    o.write(block, Nx*Ny*sizeof(float));
    o.close();
}


vector<EGS_Float> EGS_XYProfile::getPositions(int N, EGS_Float a){
   EGS_Float bwidth = a/N, min = -a/2.0, max = a/2.0;
   vector<EGS_Float> pos;
   for (int i = 0; i < N; i++){
     if (icoord == 1 ) pos.push_back(i);
     else pos.push_back( min + ((float)i+0.5)*bwidth );
   }
   return pos;
}

string** EGS_XYProfile::getProfile(const vector<string> prof){

   string **map = new string*[region.size()];
   int N = iscan ? Ny:Nx;
   for(int j = 0; j < region.size(); j++){
      map[j] = new string[N];
   }
   for(int i = 0; i < region.size(); i++){
         for(int j = 0; j < N; j++){
            int k = iscan ? index(region[i],j):index(j,region[i]);
            map[i][j] = prof[k];
         }
   }
   return map;

}

EGS_PlanePointSelector::EGS_PlanePointSelector(const EGS_Vector &midpoint,
                               const EGS_Vector &Ux, const EGS_Vector &Uy,
                             EGS_Float Ax, EGS_Float Ay, int Nx, int Ny) :
xo(midpoint), ux(Ux), uy(Uy), ax(Ax), ay(Ay), nx(Nx), ny(Ny), at(0)
{
        nreg = nx*ny; list = new int [nreg]; nlist = 0;
        dx = ax/nx; dy = ay/ny;
        xmin = -ax/2; ymin = -ay/2;
        for(int j=0; j<nreg; j++) list[j] = j;
        nlist = nreg;
}

EGS_PlanePointSelector::~EGS_PlanePointSelector() {
        delete [] list;
        if( at ) delete at;
}

//void EGS_PlanePointSelector::resetList() { nlist = 0; };

void EGS_PlanePointSelector::setProbabilities(const EGS_Float *prob)
{
        if( at ) delete at;
        EGS_Float *dum = new EGS_Float [nreg];
        for(int j=0; j<nreg; j++) dum[j] = j;
        at = new EGS_AliasTable(nreg,dum,prob,0);
        delete [] dum;
}

// inline void EGS_CorrelatedScoring::startNewCase(EGS_I64 this_case) {
//    if( this_case != last_case ) {
//        last_case = this_case;
//        for(int j=0; j<nlist; j++) {
//            int ibin = list[j];
//            corr[ibin] += s1->thisHistoryScore(ibin)*
//                          s2->thisHistoryScore(ibin);
//        }
//        nlist = 0;
//        s1->setHistory(last_case);
//        s2->setHistory(last_case);
//    }
// }
//
// inline void EGS_CorrelatedScoring::score(int ibin) {
//   if( !s1->thisHistoryScore(ibin) && !s2->thisHistoryScore(ibin) )
//       list[nlist++] = ibin;
// }

// inline void EGS_CorrelatedScoring::score(int ibin, int which, double x)
// {
//   if( x>1e-35 && !s1->thisHistoryScore(ibin) &&
//                  !s2->thisHistoryScore(ibin) ) {
//       if( nlist < nreg ) list[nlist++] = ibin;
//       else {
//           egsWarning("List overflow in score(int,int,double)?\n");
//           for(int j=0; j<nreg; j++) {
//               egsWarning("%d %g %g\n",j,s1->thisHistoryScore(j),
//                       s2->thisHistoryScore(j));
//           }
//           egsFatal("Quitting now\n");
//       }
//   }
//   EGS_ScoringArray *s = which ? s2 : s1;
//   s->score(ibin,x);
// }

bool EGS_CorrelatedScoring::storeState(ostream &data) {
        data << nreg << "  ";
        if( !egsStoreI64(data,last_case) ) return false;
        data << endl;
        for(int j=0; j<nreg; j++) data << corr[j] << "  ";
        data << endl;
        return true;
}

bool EGS_CorrelatedScoring::setState(istream &data) {
        int nreg1; data >> nreg1;
        if( nreg1 != nreg ) return false;
        if( !egsGetI64(data,last_case) ) return false;
        for(int j=0; j<nreg; j++) data >> corr[j];
        return true;
}

void EGS_CorrelatedScoring::reset() {
        last_case = 0; nlist = 0;
        for(int j=0; j<nreg; j++) corr[j] = 0;
}

EGS_CorrelatedScoring & EGS_CorrelatedScoring::operator+=(
                  const EGS_CorrelatedScoring &x)
{
        last_case += x.last_case;
        for(int j=0; j<nreg; j++) corr[j] += x.corr[j];
        return *this;
}

EGS_CorrelatedScoring::EGS_CorrelatedScoring(EGS_ScoringArray *a1,
        EGS_ScoringArray *a2) : s1(a1), s2(a2), last_case(0), nreg(0) {
    if( !s1 || !s2 ) egsFatal("Attempt to construct an EGS_CorrelatedScoring"
           " object with null scorring arrays\n");
    nreg = s1->bins();
    if( nreg != s2->bins() ) egsFatal("Attempt to construct an "
       "EGS_CorrelatedScoring object\n  with scorring arrays of "
       "different size (%d vs %d)\n",nreg,s2->bins());
    list = new int [nreg]; nlist = 0;
    corr = new double [nreg];
    for(int j=0; j<nreg; j++) corr[j] = 0;
}

EGS_CorrelatedScoring::~EGS_CorrelatedScoring() {
    if( nreg > 0 ) { delete [] list; delete [] corr; }
}
