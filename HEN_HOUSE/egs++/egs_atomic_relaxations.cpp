/*
###############################################################################
#
#  EGSnrc egs++ atomic relaxation
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
#  Author:          Iwan Kawrakow, 2008
#
#  Contributors:    Reid Townson
#
###############################################################################
*/

/*! \file     egs_atomic_relaxations.cpp
 *  \brief    EGS_AtomicRelaxations implementation
 *  \IK
 ***************************************************************************/

#include "egs_atomic_relaxations.h"
#include "egs_rndm.h"
#include "egs_functions.h"
#include "egs_application.h"
#include "egs_alias_table.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

using namespace std;

/*
class EGS_LOCAL EGS_SimpleAliasTable {

public:

    EGS_SimpleAliasTable(int N, const EGS_Float *f);

    ~EGS_SimpleAliasTable();

    int sample(EGS_RandomGenerator *rndm) const {
        int bin = (int) (rndm->getUniform()*n);
        return rndm->getUniform() < wi[bin] ? bin : bins[bin];
    };

    int       n;          //!< number of subintervals
    EGS_Float *wi;        //!< array of bin branching probabilities
    unsigned short *bins; //!< bins

};

EGS_SimpleAliasTable::EGS_SimpleAliasTable(int N, const EGS_Float *f) : n(0) {
    if( N < 1 ) return;
    n = N;
    wi = new EGS_Float [n]; bins = new unsigned short [n]; int i;
    double sum = 0; double *fcum = new double [n];
    for(i=0; i<n; ++i) { fcum[i] = f[i]; sum += fcum[i]; bins[i] = n+1; }
    sum /= n; int jh, jl;
    for(i=0; i<n-1; ++i) {
        for(jh=0; jh<n-1; jh++) {
            if( bins[jh] > n && fcum[jh] > sum ) break;
        }
        for(jl=0; jl<n-1; jl++) {
            if( bins[jl] > n && fcum[jl] < sum ) break;
        }
        double aux = sum - fcum[jl];
        fcum[jh] -= aux;
        wi[jl] = fcum[jl]/sum; bins[jl] = jh;
    }
    for(i=0; i<n; ++i) {
        if( bins[i] > n ) { bins[i] = i; wi[i] = 1; }
        //egsInformation("alias table: %d %d %g\n",i,bins[i],wi[i]);
    }
    delete [] fcum;
}

EGS_SimpleAliasTable::~EGS_SimpleAliasTable() {
    if( n > 0 ) { delete [] wi; delete [] bins; }
}
*/

class EGS_LOCAL EGS_ShellData {

public:

    float           be;          //!< binding energy
    unsigned short  type;        //!< shell type according to EADL nomenclature
    unsigned short  ntrans;      //!< number of transitions
    unsigned short *ttypes;      //!< types of the ntrans transitions
    EGS_SimpleAliasTable *table; //!< for sampling transitions

    EGS_ShellData() : ntrans(0), table(0) {};
    ~EGS_ShellData() {
        deleteData();
    }

    void deleteData() {
        if (ntrans > 0) {
            delete [] ttypes;
            ntrans = 0;
        }
        if (table) {
            delete table;
            table = 0;
        }
    };

    bool loadData(istream &data);

    int sample(EGS_RandomGenerator *rndm) const {
        return ttypes[table->sample(rndm)];
    };

};

bool EGS_ShellData::loadData(istream &data) {
    deleteData();
    char t;
    data.read((char *)&t,sizeof(char));
    type = t;
    data.read((char *)&ntrans,sizeof(short));
    data.read((char *)&be,sizeof(float));
    //egsInformation("shell is of type %d, has BE=%g and %d transitions\n",type,be,ntrans);
    if (data.fail() || ntrans > 10000) {
        ntrans = 0;
        return false;
    }
    if (ntrans == 0) {
        return true;
    }
    ttypes = new unsigned short [ntrans];
    EGS_Float *tmp = new EGS_Float [ntrans];
    float aux;
    for (int i=0; i<ntrans; ++i) {
        data.read((char *)&ttypes[i],sizeof(unsigned short));
        data.read((char *)&aux,sizeof(float));
        tmp[i] = aux;
        //egsInformation("got transition %d with p=%g\n",ttypes[i],aux);
    }
    if (data.fail()) {
        delete [] tmp;
        deleteData();
        return false;
    }
    table = new EGS_SimpleAliasTable(ntrans,tmp);
    delete [] tmp;
    return true;
}

class EGS_LOCAL EGS_ElementRelaxData {

public:

    EGS_ElementRelaxData() : Z(0), nshell(0), shells(0) {};

    ~EGS_ElementRelaxData() {
        deleteData();
    };

    int loadData(int iZ, istream &data);


    unsigned short       Z;          //!< atomic number
    unsigned short       nshell;     //!< number of shells
    EGS_ShellData       *shells;     //!< shell data

    void deleteData() {
        if (nshell > 0) {
            delete [] shells;
        }
        Z = 0;
        nshell = 0;
        shells = 0;
    };

};

int EGS_ElementRelaxData::loadData(int iZ, istream &data) {
    if (iZ == Z) {
        return 0;
    }
    deleteData();
    Z = iZ;
    int pos = 2 + (Z-1)*sizeof(int);
    data.seekg(pos,ios::beg);
    data.read((char *)&pos,sizeof(int));
    if (data.fail()) {
        egsWarning("Failed reading position for element %d\n",Z);
        Z = 0;
        return 5;
    }
    data.seekg(pos,ios::beg);
    data.read((char *)&nshell,sizeof(nshell));
    if (data.fail() || nshell < 1 || nshell > 63) {
        egsWarning("Failed reading number of shells for Z=%d (got %d shells and fail()=%d\n",
                   Z,nshell,data.fail());
        nshell = 0;
        return 6;
    }
    //egsInformation("element %d has %d shells\n",Z,nshell);
    shells = new EGS_ShellData [nshell];
    for (int j=0; j<nshell; ++j) {
        if (!shells[j].loadData(data)) {
            egsWarning("Failed loading data for shell %d of element %d\n",j+1,Z);
            deleteData();
            return 7;
        }
    }
    return 0;
}

class EGS_LOCAL EGS_RelaxImplementation {

public:

    EGS_RelaxImplementation(const char *data_path) : elements(0), nz(0) {
        data_file = egsJoinPath(data_path,"relax_onebyte.data");
    };

    ~EGS_RelaxImplementation() {
        if (nz > 0) {
            for (int j=0; j<nz; ++j) {
                if (elements[j]) {
                    delete elements[j];
                }
            }
            delete [] elements;
        }
    };

    int loadData(int Z);
    int loadData(int Z, istream &data);
    int loadData(int Nz, const int *Zarray);
    int loadAllData();

    int openDataFile(istream **data);

    void checkData(int Z, int shell) {
        static const char *func_name = "checkData";
        if (!elements)
            if (loadData(Z)) egsFatal("%s: failed to load data for Z=%d\n",
                                          func_name,Z);
        if (Z < 1 || Z > nz)
            egsFatal("%s: Z=%d is outside of initialized range 1...%d\n",
                     func_name,Z,nz);
        if (shell < 0 || shell >= elements[Z-1]->nshell)
            egsFatal("%s: element %d has %d shells but you are asking for "
                     "shell %d\n",func_name,Z,elements[Z-1]->nshell,shell);
    };

    void relax(int Z, int sh, EGS_Float ecut, EGS_Float pcut,
               EGS_RandomGenerator *rndm, double &edep,
               EGS_SimpleContainer<EGS_RelaxationParticle> &particles) {
        checkData(Z,sh);
        EGS_Float minE = pcut < ecut ? pcut : ecut;
        relax(Z,sh,minE,ecut,pcut,rndm,edep,particles);
    };

    EGS_Float bindingEnergy(int Z, int shell) {
        checkData(Z,shell);
        return elements[Z-1]->shells[shell].be;
    };

    int getNShell(int Z) {
        return elements[Z-1]->nshell;
    };

    void setBindingEnergy(int Z, int shell, EGS_Float new_be) {
        checkData(Z,shell);
        elements[Z-1]->shells[shell].be = new_be;
    };

    EGS_Float getMaxGammaEnergy(int Z, int shell) {
        checkData(Z,shell);
        int ntrans = elements[Z-1]->shells[shell].ntrans;
        if (ntrans < 1) {
            return 0;
        }
        EGS_Float E = elements[Z-1]->shells[shell].be;
        EGS_Float emax = 0;
        for (int j=0; j<ntrans; ++j) {
            int transition = elements[Z-1]->shells[shell].ttypes[j];
            if (transition < 64) {
                EGS_Float Egamma = E - elements[Z-1]->shells[transition].be;
                if (Egamma > emax) {
                    emax = Egamma;
                }
            }
        }
        return emax;
    };

    EGS_Float getMaxElectronEnergy(int Z, int shell) {
        checkData(Z,shell);
        int ntrans = elements[Z-1]->shells[shell].ntrans;
        if (ntrans < 1) {
            return 0;
        }
        EGS_Float E = elements[Z-1]->shells[shell].be;
        EGS_Float emax = 0;
        for (int j=0; j<ntrans; ++j) {
            int transition = elements[Z-1]->shells[shell].ttypes[j];
            if (transition >= 64) {
                int sh1 = (transition >> 6);
                int sh2 = transition - (sh1 << 6);
                EGS_Float Eelec = E - (elements[Z-1]->shells[sh1].be +
                                       elements[Z-1]->shells[sh2].be);
                if (Eelec > emax) {
                    emax = Eelec;
                }
            }
        }
        return emax;
    };

    void relax(int Z, int sh, EGS_Float minE, EGS_Float ecut, EGS_Float pcut,
               EGS_RandomGenerator *rndm, double &edep,
               EGS_SimpleContainer<EGS_RelaxationParticle> &particles) {
        EGS_Float E = elements[Z-1]->shells[sh].be;
        //egsInformation("relax(%d,%d): E=%g minE=%g\n",Z,sh,E,minE);
        if (E <= minE || elements[Z-1]->shells[sh].ntrans < 1) {
            //egsInformation("relax: terminating\n");
            edep += E;
            return;
        }
        int transition = elements[Z-1]->shells[sh].sample(rndm);
        //egsInformation("relax: got transition %d\n",transition);
        if (transition < 64) {
            EGS_Float Egamma = E - elements[Z-1]->shells[transition].be;
            //egsInformation("->flourescence, Egamma=%g\n",Egamma);
            if (Egamma > pcut) {
                EGS_RelaxationParticle p(0,Egamma);
                particles.add(p);
            }
            else {
                edep += Egamma;
            }
            relax(Z,transition,minE,ecut,pcut,rndm,edep,particles);
        }
        else {
            int sh1 = (transition >> 6);
            int sh2 = transition - (sh1 << 6);
            EGS_Float Eelec = E - elements[Z-1]->shells[sh1].be - elements[Z-1]->shells[sh2].be;
            //egsInformation("->Auger, sh1=%d sh2=%d E=%g\n",sh1,sh2,Eelec);
            if (Eelec > ecut) {
                EGS_RelaxationParticle p(-1,Eelec);
                particles.add(p);
            }
            else {
                edep += Eelec;
            }
            relax(Z,sh1,minE,ecut,pcut,rndm,edep,particles);
            relax(Z,sh2,minE,ecut,pcut,rndm,edep,particles);
        }
    };

    EGS_ElementRelaxData **elements;
    string               data_file;
    int                  nz;

};

int EGS_RelaxImplementation::openDataFile(istream **the_data) {
    static const char *func_name = "EGS_RelaxImplementation::openDataFile()";
    *the_data = 0;
    ifstream *data = new ifstream(data_file.c_str(),ios::binary);
    if (!(*data)) {
        egsWarning("%s: failed to open data file %s\n",
                   func_name,data_file.c_str());
        return 1;
    }
    short Nz;
    data->read((char *)&Nz,sizeof(short));
    if (data->fail() || Nz < 1 || Nz > 200) {
        egsWarning("%s: failed reading first record from %s\n",
                   func_name,data_file.c_str());
        return 2;
    }
    if (Nz > nz) {
        EGS_ElementRelaxData **new_elements = new EGS_ElementRelaxData* [Nz];
        if (nz > 0) {
            for (int j=0; j<nz; ++j) {
                new_elements[j] = elements[j];
            }
            delete [] elements;
        }
        else {
            for (int j=0; j<Nz; ++j) {
                new_elements[j] = 0;
            }
        }
        elements = new_elements;
        nz = Nz;
    }
    *the_data = data;
    return 0;
}


int EGS_RelaxImplementation::loadData(int Z) {
    static const char *func_name = "EGS_RelaxImplementation::loadData()";
    if (Z < 1) {
        egsWarning("%s: called with Z=%d?\n",func_name,Z);
        return 3;
    }
    istream *the_data;
    int res = openDataFile(&the_data);
    if (res || !the_data) {
        return res;
    }
    istream &data = *the_data;
    if (Z > nz) {
        egsWarning("%s: called with Z=%d, but I only have data for Z<=%d.\n",
                   func_name,Z,nz);
        return 4;
    }
    res = loadData(Z,data);
    delete the_data;
    return res;
}

int EGS_RelaxImplementation::loadData(int Z, istream &data) {
    if (elements[Z-1]) {
        return 0;
    }
    elements[Z-1] = new EGS_ElementRelaxData;
    return elements[Z-1]->loadData(Z,data);
}

int EGS_RelaxImplementation::loadData(int Nz, const int *Zarray) {
    istream *the_data;
    int res = openDataFile(&the_data);
    if (res || !the_data) {
        return res;
    }
    istream &data = *the_data;
    for (int j=0; j<Nz; ++j) {
        res = loadData(Zarray[j],data);
        if (res) {
            break;
        }
    }
    delete the_data;
    return res;
}

int EGS_RelaxImplementation::loadAllData() {
    istream *the_data;
    int res = openDataFile(&the_data);
    if (res || !the_data) {
        return res;
    }
    istream &data = *the_data;
    for (int Z=1; Z<=nz; ++Z) {
        res = loadData(Z,data);
        if (res) {
            break;
        }
    }
    delete the_data;
    return res;
}

EGS_AtomicRelaxations::EGS_AtomicRelaxations(const char *data_path) {
    string path;
    if (!data_path) {
        EGS_Application *app = EGS_Application::activeApplication();
        if (app) {
            path = egsJoinPath(app->getHenHouse(),"data");
        }
        else {
            char *hen_house = getenv("HEN_HOUSE");
            if (!hen_house) {
                egsWarning("EGS_AtomicRelaxations::EGS_AtomicRelaxations: "
                           "no active application and HEN_HOUSE not defined.\n"
                           "  assuming local directory for relax data\n");
                path = "./";
            }
            else {
                path = egsJoinPath(hen_house,"data");
            }
        }
    }
    else {
        path = data_path;
    }
    p = new EGS_RelaxImplementation(path.c_str());
}

EGS_AtomicRelaxations::~EGS_AtomicRelaxations() {
    delete p;
}

int EGS_AtomicRelaxations::loadData(int Z) {
    return p->loadData(Z);
}

int EGS_AtomicRelaxations::loadData(int nz, const int *Zarray) {
    return p->loadData(nz,Zarray);
}

int EGS_AtomicRelaxations::loadAllData() {
    return p->loadAllData();
}

void EGS_AtomicRelaxations::relax(int Z, int sh, EGS_Float ecut, EGS_Float pcut,
                                  EGS_RandomGenerator *rndm, double &edep,
                                  EGS_SimpleContainer<EGS_RelaxationParticle> &particles) {
    p->relax(Z,sh,ecut,pcut,rndm,edep,particles);
}

EGS_Float EGS_AtomicRelaxations::getBindingEnergy(int Z, int shell) {
    return p->bindingEnergy(Z,shell);
}

int EGS_AtomicRelaxations::getNShell(int Z) {
    return p->getNShell(Z);
}

void EGS_AtomicRelaxations::setBindingEnergy(int Z, int shell,
        EGS_Float new_be) {
    p->setBindingEnergy(Z,shell,new_be);
}

EGS_Float EGS_AtomicRelaxations::getMaxGammaEnergy(int Z, int shell) {
    return p->getMaxGammaEnergy(Z,shell);
}

EGS_Float EGS_AtomicRelaxations::getMaxElectronEnergy(int Z, int shell) {
    return p->getMaxElectronEnergy(Z,shell);
}
