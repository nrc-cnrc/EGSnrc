/*
###############################################################################
#
#  EGSnrc egs++ egs_cbct application utilities headers
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


#ifndef EGS_UTILS_
#define EGS_UTILS_

#include "egs_functions.h"
#include "egs_vector.h"
// We use scoring objects provided by egspp => need the header file.
#include "egs_scoring.h"
// We use the EGS_Input class
#include "egs_input.h"
// The random number generator
#include "egs_rndm.h"

#include "egs_alias_table.h"

#include "array_sizes.h"
#include <fstream>
#include <iostream>
#include <cstdarg>
#include <cstdlib>

#define Pi 3.1415926535897932385128

#ifdef DEBUG_WEIGHTS
struct Interaction {
    EGS_Vector x,u;
    EGS_Float  E,wt;
    int        ir,latch,type,nsplit,where;
    Interaction() {};
    Interaction(const EGS_Vector &X, const EGS_Vector &U,
                      EGS_Float e, EGS_Float Wt,
                      int Ir, int Latch, int Type, int N, int W){};
};

class EGS_CBCT_Interactions {
public:

    EGS_CBCT_Interactions() : n(0), nnow(0){};

    ~EGS_CBCT_Interactions() { if(nnow>0) delete i; };

    void addInteraction(const EGS_Vector &x, const EGS_Vector &u,
                              EGS_Float E, EGS_Float wt,
                              int ir, int latch, int type,int N, int W);

    void addInteraction(const Interaction &I);

    void grow();

    void printInteractions();

    Interaction *i;
    int         n, nnow;
};
#endif

struct EGS_CBCT_Photon {
    int        latch;  //!< particle latch, counts number of interactions
    int        isc;    //!< detector index (-1 if not moving towards screen)
    int        ir;     //!< geometry region
    int        imed;   //!< current medium
    int        phat;   //!< "phatness"
    EGS_Float  E;      //!< particle energy in MeV
    EGS_Float  gle;    //!< ln(E)
    EGS_Float  wt;     //!< statistical weight
    EGS_Vector x;      //!< position
    EGS_Vector u;      //!< direction
};

class EGS_CorrelatedScoring {

public:

    EGS_CorrelatedScoring(EGS_ScoringArray *a1, EGS_ScoringArray *a2);

    ~EGS_CorrelatedScoring();

    inline void startNewCase(EGS_I64 this_case) {
       if( this_case != last_case ) {
           last_case = this_case;
           for(int j=0; j<nlist; j++) {
               int ibin = list[j];
               corr[ibin] += s1->thisHistoryScore(ibin)*
                             s2->thisHistoryScore(ibin);
           }
           nlist = 0;
           s1->setHistory(last_case);
           s2->setHistory(last_case);
       }
    };

    inline void score(int ibin) {
      if( !s1->thisHistoryScore(ibin) && !s2->thisHistoryScore(ibin) )
          list[nlist++] = ibin;
    };

    inline void score(int ibin, int which, double x)
    {
      if( x>1e-35 && !s1->thisHistoryScore(ibin) &&
                     !s2->thisHistoryScore(ibin) ) {
          if( nlist < nreg ) list[nlist++] = ibin;
          else {
              egsWarning("List overflow in score(int,int,double)?\n");
              for(int j=0; j<nreg; j++) {
                  egsWarning("%d %g %g\n",j,s1->thisHistoryScore(j),
                          s2->thisHistoryScore(j));
              }
              egsFatal("Quitting now\n");
          }
      }
      EGS_ScoringArray *s = which ? s2 : s1;
      s->score(ibin,x);
    };

    bool storeState(ostream &data);

    bool setState(istream &data);

    void reset();

    EGS_CorrelatedScoring &operator+=(const EGS_CorrelatedScoring &x);

    double getCorrelation(int j) const { return corr[j]; };


protected:

    EGS_ScoringArray *s1;
    EGS_ScoringArray *s2;
    double           *corr;
    int              *list;
    EGS_I64          last_case;
    int              nlist;
    int              nreg;

};

class EGS_BaseFile{

public:

  EGS_BaseFile(){};
  EGS_BaseFile(const char *n){name = const_cast<char *>(n);};
  ~EGS_BaseFile(){};
  bool fileExist(const char *pName);
  bool fileExist();
  EGS_I64 fileSize();
  EGS_I64 fileSize(const char * n);
  char *Name(){return name;};

protected:

char * name;

};

class EGS_BinaryFile: public EGS_BaseFile{

public:

 EGS_BinaryFile(const char *n, const EGS_I64 &s, char *what);
 EGS_BinaryFile(const char *n, char *what);
 EGS_BinaryFile(const char *n);
 ~EGS_BinaryFile(){if(size) delete [] block;};
 void writeBinary( const EGS_I64 &pos );
 void writeBinary(char          *what,
                  const EGS_I64 &s,
                  const EGS_I64 &pos);
 void readBinary();
 float * readValues();
 void    readValues(float *s, const int &_size);
 void writeASCII( const int &chunk );
 void readASCII();
 void setBlock(const EGS_I64 &s, char *what){
      size =s;block = new char[s];block=what;
 };
 void fillWith(const EGS_I64 &s, const char *what);
 EGS_I64 blockSize(){return fileSize()/sizeof(float);};
protected:

private:

 EGS_I64  size;
 char *block;

};

class EGS_XYProfile {

public:
    EGS_XYProfile(EGS_Input * inp,
                  EGS_Float rAx, EGS_Float rAy,
                  int rNx, int rNy);
    ~EGS_XYProfile();

    void saveProfile(const string pName,
                     const vector<string> prof);
    void saveProjection(const string pName,
                     const EGS_Float *proj);
    void describeIt();

    bool isDefined(){return defined;};

protected:
    string** getProfile(const vector<string> prof);
    vector<EGS_Float> getPositions(int N, EGS_Float a);
    int index(int i, int j){return i + j*Nx;};
    int get_i(EGS_Float x){return int(Nx*(x+ax/2.0)/ax);};
    int get_j(EGS_Float y){return int(Ny*(y+ay/2.0)/ay);};
private:

bool              defined;// true if defined, false else
int               iscan; // 0: X-scan, non-zero: Y-scan
int               icoord;// 0: off, 1: by regions, 2: by location
int               Nx, Ny;
EGS_Float         ax, ay;
vector<EGS_Float> position;// either geometrical or region index
vector<int> region;
};

/************************************/
/* EGS_CBCT_ParticleContainer class */
/************************************/
class EGS_CBCT_ParticleContainer {

inline void grow()
{
        int n = ntot ? 2*ntot : 32;
        EGS_CBCT_Photon *tmp = new EGS_CBCT_Photon [n];
        for(int j=0; j<nnow; j++) tmp[j] = particles[j];
        if( ntot > 0 ) delete [] particles;
        ntot = n; particles = tmp;
}

public:
    int nnow, ntot;
    EGS_CBCT_Photon *particles;
    EGS_CBCT_ParticleContainer() : nnow(0), ntot(0) {};
    ~EGS_CBCT_ParticleContainer(){if( ntot > 0 ) delete [] particles;};

    inline void addParticle(const EGS_CBCT_Photon &p)
    {
        if( nnow >= ntot ) grow();
        particles[nnow++] = p;
    };

    inline EGS_CBCT_Photon takeParticle()
    {
      return nnow > 0 ? particles[--nnow] : EGS_CBCT_Photon();
    };

};

class EGS_PlanePointSelector {

public:

    EGS_PlanePointSelector(const EGS_Vector &midpoint, const EGS_Vector &Ux,
            const EGS_Vector &Uy, EGS_Float Ax, EGS_Float Ay, int Nx, int Ny);

    ~EGS_PlanePointSelector();

    void refreshList(EGS_RandomGenerator *rndm){nlist = nreg;};

    void resetList(){};

    inline int  getPoint(EGS_RandomGenerator *rndm,
                                      EGS_Vector &v)
    {
        int ireg;
        if( at ) ireg = at->sampleBin(rndm);
        else {
            if( !nlist ) refreshList(rndm);
            int j = (int) (rndm->getUniform()*nlist);
            ireg = list[j];
            list[j] = list[--nlist]; list[nlist] = ireg;
        }
        int ix = ireg%nx, iy = ireg/nx;
        //EGS_Float rx = xmin + dx*(ix + rrx);
        //EGS_Float ry = ymin + dy*(iy + rry);
        EGS_Float rx = xmin + dx*(ix + rndm->getUniform());
        EGS_Float ry = ymin + dy*(iy + rndm->getUniform());
        v = xo + ux*rx + uy*ry;
        return ireg;
    };


    void setProbabilities(const EGS_Float *prob);

private:

    EGS_Vector xo;  //!<  rectangle midpoint
    EGS_Vector ux;  //!<  first unit vector on the plane
    EGS_Vector uy;  //!<  second unit vector on the plane
    EGS_Float  ax;  //!<  rectangle size along ux
    EGS_Float  ay;  //!<  rectangle size along uy
    EGS_Float  dx;  //!<  voxel size along ux
    EGS_Float  dy;  //!<  voxel size along uy
    EGS_Float xmin; //!<  min. along ux
    EGS_Float ymin; //!<  min. along uy
    EGS_Float rrx;  //!<  a random number
    EGS_Float rry;  //!<  a random number
    int        nx;  //!<  number of voxels along ux
    int        ny;  //!<  number of voxels along uy
    int      nreg;  //!<  total number of voxels
    int     nlist;  //!<  number of voxels not yet selected
    int     *list;  //!<  list of voxels not yet selected
    EGS_AliasTable *at;
};

#endif
