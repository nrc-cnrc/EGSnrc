/*
###############################################################################
#
#  EGSnrc egs++ nd geometry
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
#  Author:          Iwan Kawrakow, 2005
#
#  Contributors:    Frederic Tessier
#
###############################################################################
*/


/*! \file egs_nd_geometry.cpp
 *  \brief N-dimensional geometries: implementation
 *  \IK
 */

#include "egs_nd_geometry.h"
#include "egs_input.h"
#include "egs_functions.h"
#include "egs_transformations.h"

#include <vector>
#include <fstream>

using namespace std;

EGS_NDGeometry::EGS_NDGeometry(int ng, EGS_BaseGeometry **G,
                               const string &Name, bool O) : EGS_BaseGeometry(Name), N(ng), ortho(O) {
    nreg = 1;
    setup();
    for (int j=0; j<N; j++) {
        g[j] = G[j];
        n[j] = nreg;
        nreg *= g[j]->regions();
        if (!g[j]->isConvex()) {
            is_convex = false;
        }
    }
    n[N] = nreg;
}

EGS_NDGeometry::EGS_NDGeometry(vector<EGS_BaseGeometry *> &G,
                               const string &Name, bool O) : EGS_BaseGeometry(Name), N(G.size()), ortho(O) {
    nreg = 1;
    setup();
    for (int j=0; j<N; j++) {
        g[j] = G[j];
        n[j] = nreg;
        nreg *= g[j]->regions();
        if (!g[j]->isConvex()) {
            is_convex = false;
        }
    }
    n[N] = nreg;
}

EGS_NDGeometry::~EGS_NDGeometry() {
    for (int j=0; j<N; j++) {
        int ref = g[j]->deref();
        if (!ref) {
            delete g[j];
        }
    }
    delete [] n;
    delete [] g;
}

void EGS_NDGeometry::setup() {
    n = new int [N+1];
    g = new EGS_BaseGeometry* [N];
}

void EGS_NDGeometry::printInfo() const {
    EGS_BaseGeometry::printInfo();
    egsInformation(" number of dimensions = %d\n",N);
    for (int j=0; j<N; j++)
        egsInformation(" dimension %d = %s (type %s)\n",
                       j+1,g[j]->getName().c_str(),g[j]->getType().c_str());
    egsInformation(
        "=======================================================\n");
}

void EGS_NDGeometry::setMedia(EGS_Input *input, int nmed, const int *mind) {
    EGS_Input *i;
    med = mind[0];
    while ((i = input->takeInputItem("set medium"))) {
        vector<int> inp;
        int err = i->getInput("set medium",inp);
        delete i;
        if (!err) {
            if (inp.size() == 2) {
                setMedium(inp[0],inp[0],mind[inp[1]]);
            }
            else if (inp.size() == 3) {
                setMedium(inp[0],inp[1],mind[inp[2]]);
            }
            else if (inp.size() == 4) setMedium(inp[0],inp[1],mind[inp[2]],
                                                    inp[3]);
            else if (inp.size() == 2*N+1) {
                setM(0,0,inp,inp[2*N]);
            }
            else egsWarning("EGS_NDGeometry::setMedia(): found %dinputs\n"
                                "in a 'set medium' input. 2, 3, 4, or %d are allowed\n",2*N+1);
        }
        else egsWarning("EGS_NDGeometry::setMedia(): wrong 'set medium'"
                            " input\n");
    }
}

void EGS_NDGeometry::setM(int ibase, int idim,
                          const vector<int> &ranges, int medium) {
    int istart = ranges[2*idim], iend = ranges[2*idim+1];
    if (istart < 0) {
        istart = 0;
    }
    int ndim = n[idim+1];
    for (int i=0; i<=idim; i++) {
        ndim /= n[i];
    }
    if (iend > ndim) {
        iend = ndim;
    }
    if (idim < N-1) for (int j=istart; j<iend; j++) {
            setM(ibase+j*n[idim],idim+1,ranges,medium);
        }
    else for (int j=istart; j<iend; j++) {
            setMedium(ibase+j*n[idim],ibase+j*n[idim]+1,medium);
        }
}

#ifdef EXPLICIT_XYZ

string EGS_XYZGeometry::type = "EGS_XYZGeometry";

EGS_XYZGeometry::EGS_XYZGeometry(EGS_PlanesX *Xp, EGS_PlanesY *Yp,
                                 EGS_PlanesZ *Zp, const string &Name) : EGS_BaseGeometry(Name),
    xp(Xp), yp(Yp), zp(Zp) {
    nx = xp->regions();
    ny = yp->regions();
    nz = zp->regions();
    nxy = nx*ny;
    nreg = nxy*nz;
    xp->ref();
    yp->ref();
    zp->ref();
    xpos = xp->getPositions();
    ypos = yp->getPositions();
    zpos = zp->getPositions();
}

EGS_XYZGeometry::~EGS_XYZGeometry() {
    if (!xp->deref()) {
        delete xp;
    }
    if (!yp->deref()) {
        delete yp;
    }
    if (!zp->deref()) {
        delete zp;
    }
}

void EGS_XYZGeometry::printInfo() const {
    EGS_BaseGeometry::printInfo();
    xp->printInfo();
    yp->printInfo();
    zp->printInfo();
    egsInformation(
        "=======================================================\n");
}

void EGS_XYZGeometry::setMedia(EGS_Input *input, int nmed, const int *mind) {
    EGS_Input *i;
    med = mind[0];
    while ((i = input->takeInputItem("set medium"))) {
        vector<int> inp;
        int err = i->getInput("set medium",inp);
        delete i;
        if (!err) {
            if (inp.size() == 2) {
                setMedium(inp[0],inp[0],mind[inp[1]]);
            }
            else if (inp.size() == 3) {
                setMedium(inp[0],inp[1],mind[inp[2]]);
            }
            else if (inp.size() == 4) setMedium(inp[0],inp[1],mind[inp[2]],
                                                    inp[3]);
            else if (inp.size() == 7) {
                int is = inp[0], ie = inp[1];
                if (is < 0) {
                    is = 0;
                }
                if (ie > nx-1) {
                    ie = nx-1;
                }
                int js = inp[2], je = inp[3];
                if (js < 0) {
                    js = 0;
                }
                if (je > nxy/nx-1) {
                    je = nxy/nx-1;
                }
                int ks = inp[4], ke = inp[5];
                if (ks < 0) {
                    ks = 0;
                }
                if (ke > nreg/nxy-1) {
                    ke = nreg/nxy-1;
                }
                //egsInformation("setting medium to %d between %d %d %d %d "
                //        "%d %d\n",mind[inp[6]],is,ie,js,je,ks,ke);
                for (int i=is; i<=ie; i++) {
                    for (int j=js; j<=je; j++) {
                        for (int k=ks; k<=ke; k++) {
                            int ireg = i + j*nx + k*nxy;
                            //egsWarning("set: %d %d %d %d   %d\n",
                            //        i,j,k,ireg,mind[inp[6]]);
                            setMedium(ireg,ireg,mind[inp[6]]);
                        }
                    }
                }
            }
            else if (inp.size() == 10) {
                int is = inp[0], ie = inp[1], idelta = inp[2];
                if (is < 0) {
                    is = 0;
                }
                if (ie > nx-1) {
                    ie = nx-1;
                }
                if (idelta <= 0) {
                    idelta = 1;
                }
                int js = inp[3], je = inp[4], jdelta = inp[5];
                if (js < 0) {
                    js = 0;
                }
                if (je > nxy/nx-1) {
                    je = nxy/nx-1;
                }
                if (jdelta <= 0) {
                    jdelta = 1;
                }
                int ks = inp[6], ke = inp[7], kdelta = inp[8];
                if (ks < 0) {
                    ks = 0;
                }
                if (ke > nreg/nxy-1) {
                    ke = nreg/nxy-1;
                }
                if (kdelta <= 0) {
                    kdelta = 1;
                }
                //egsInformation("setting medium to %d between %d %d %d %d "
                //        "%d %d\n",mind[inp[6]],is,ie,js,je,ks,ke);
                for (int i=is; i<=ie; i+=idelta) {
                    for (int j=js; j<=je; j+=jdelta) {
                        for (int k=ks; k<=ke; k+=kdelta) {
                            int ireg = i + j*nx + k*nxy;
                            //egsWarning("set: %d %d %d %d   %d\n",
                            //        i,j,k,ireg,mind[inp[6]]);
                            setMedium(ireg,ireg,mind[inp[9]]);
                        }
                    }
                }
            }
            else egsWarning("EGS_XYZGeometry::setMedia(): found %dinputs\n"
                                "in a 'set medium' input. 2, 3, 4, 7, or 10 are allowed\n");
        }
        else egsWarning("EGS_XYZGeometry::setMedia(): wrong 'set medium'"
                            " input\n");
    }
}

EGS_XYZGeometry *EGS_XYZGeometry::constructGeometry(const char *dens_file,
        const char *ramp_file, int dens_or_egsphant_or_interfile) {
    const static char *func = "EGS_XYZGeometry::constructGeometry";
    if (!dens_file || !ramp_file) {
        return 0;
    }
    ifstream ramp(ramp_file);
    if (!ramp) {
        egsWarning("%s: failed to open CT ramp file %s\n",func,ramp_file);
        return 0;
    }
    vector<string> med_names;
    vector<EGS_Float> rho_min,rho_max,rho_def;

    if (dens_or_egsphant_or_interfile == 2) {
        // interfile format of data
        int meds;
        ramp >> meds;
        for (int i=0; i < meds; i++) {
            string medname;
            EGS_Float rmin,rmax,rdef;
            ramp >> rmin >> rmax >> medname;
            rdef=(rmin+rmax) / 2.0; // is this correct definition of rdef ??
            if (ramp.eof() || ramp.fail() || !ramp.good()) {
                break;
            }
            egsInformation("Using medium %s for rho=%g...%g\n",medname.c_str(),
                           rmin,rmax);
            med_names.push_back(medname);
            rho_min.push_back(rmin);
            rho_max.push_back(rmax+1);
            rho_def.push_back(rdef);
        }
    }
    else while (1) {
            // other format of data
            string medname;
            EGS_Float rmin,rmax,rdef;
            ramp >> medname >> rmin >> rmax >> rdef;
            if (ramp.eof() || ramp.fail() || !ramp.good()) {
                break;
            }
            egsInformation("Using medium %s for rho=%g...%g\n",medname.c_str(),
                           rmin,rmax);
            med_names.push_back(medname);
            rho_min.push_back(rmin);
            rho_max.push_back(rmax);
            rho_def.push_back(rdef);
        }
    if (med_names.size() < 1) {
        egsWarning("%s: no media defined in the CT ramp "
                   "file %s!\n",func,ramp_file);
        return 0;
    }
    int Nx, Ny, Nz;
    EGS_Float *xx, *yy, *zz;
    float *rho;
    if (dens_or_egsphant_or_interfile == 0) {
        int my_endian = egsGetEndian();
        if (my_endian < 0) egsFatal("%s: machine has an unknown"
                                        " endianess!\n",func);
        ifstream dens(dens_file,ios::binary);
        if (!dens) {
            egsWarning("%s: failed to open density matrix file "
                       "%s\n",func,dens_file);
            return 0;
        }
        char their_endian;
        dens.read(&their_endian,1);
        if (their_endian != 0 && their_endian != 1)
            egsFatal("%s: density data created on a machine with"
                     " unknown endianess!\n",func);
        bool swap = my_endian != their_endian;
        dens.read((char *) &Nx,sizeof(int));
        if (swap) {
            egsSwapBytes(&Nx);
        }
        dens.read((char *) &Ny,sizeof(int));
        if (swap) {
            egsSwapBytes(&Ny);
        }
        dens.read((char *) &Nz,sizeof(int));
        if (swap) {
            egsSwapBytes(&Nz);
        }
        if (Nx < 1 || Ny < 1 || Nz < 1) {
            egsWarning("%s: invalid density matrix file: "
                       " Nx=%d Ny=%d Nz=%d\n",func,Nx,Ny,Nz);
            return 0;
        }
        float *x = new float [Nx+1];
        float *y = new float [Ny+1];
        float *z = new float [Nz+1];
        dens.read((char *)x, (Nx+1)*sizeof(float));
        if (dens.eof() || dens.fail() || !dens.good()) {
            egsWarning("%s: error while reading %d x-planes from"
                       " file %s\n",func,Nx+1,dens_file);
            delete [] x;
            delete [] y;
            delete [] z;
            return 0;
        }
        dens.read((char *)y, (Ny+1)*sizeof(float));
        if (dens.eof() || dens.fail() || !dens.good()) {
            egsWarning("%s: error while reading %d y-planes from"
                       " file %s\n",func,Ny+1,dens_file);
            delete [] x;
            delete [] y;
            delete [] z;
            return 0;
        }
        dens.read((char *)z, (Nz+1)*sizeof(float));
        if (dens.eof() || dens.fail() || !dens.good()) {
            egsWarning("%s: error while reading %d z-planes from"
                       " file %s\n",func,Nz+1,dens_file);
            delete [] x;
            delete [] y;
            delete [] z;
            return 0;
        }
        if (swap) {
            int j;
            for (j=0; j<Nx; j++) {
                egsSwapBytes(&x[j]);
            }
            for (j=0; j<Ny; j++) {
                egsSwapBytes(&y[j]);
            }
            for (j=0; j<Nz; j++) {
                egsSwapBytes(&z[j]);
            }
        }
        {
            int j;
            xx = new EGS_Float [Nx+1];
            for (j=0; j<=Nx; j++) {
                xx[j] = x[j];
            }
            yy = new EGS_Float [Ny+1];
            for (j=0; j<=Ny; j++) {
                yy[j] = y[j];
            }
            zz = new EGS_Float [Nz+1];
            for (j=0; j<=Nz; j++) {
                zz[j] = z[j];
            }
            delete [] x;
            delete [] y;
            delete [] z;
        }
        rho = new float [Nx*Ny*Nz];
        dens.read((char *)rho, Nx*Ny*Nz*sizeof(float));
        if (dens.fail()) {
            egsWarning("%s: failed reading mass densities from density matrux file\n",func);
            delete [] rho;
            delete [] xx;
            delete [] yy;
            delete [] zz;
            return 0;
        }
        if (swap) {
            for (int j=0; j<Nx*Ny*Nz; j++) {
                egsSwapBytes(&rho[j]);
            }
        }
    }
    else if (dens_or_egsphant_or_interfile == 2) {
        ifstream h_file(dens_file);
        if (!h_file) {
            egsWarning("%s: failed to open the interfile header %s\n",func,dens_file);
            return 0;
        }
        string data_file("");
        int data_type = -1;
        float scale_x=0.0, scale_y=0.0, scale_z=0.0;
        while (1) {
            string line, key, value;
            size_t pos;
            getline(h_file, line);
            if (h_file.eof() || h_file.fail() || !h_file.good()) {
                break;
            }

            pos = line.find(":=");
            if (pos == string::npos) {
                continue;
            }
            key = line.substr(0, int(pos));
            value = line.substr(int(pos)+2);
            while ((key[0] == '!') || (key[0] == ' ')) {
                key.erase(0,1);
            }
            while (key[key.length()-1] == ' ') {
                key.erase(key.length()-1, 1);
            }
            while (value[0] == ' ') {
                value.erase(0,1);
            }
            while (value[value.length()-1] == ' ') {
                value.erase(value.length()-1, 1);
            }

            if (key ==  "matrix size [1]") {
                sscanf(value.c_str(), "%u", &Nx);
            }
            else if (key ==  "matrix size [2]") {
                sscanf(value.c_str(), "%u", &Ny);
            }
            else if ((key ==  "number of slices") || (key ==  "number of images")) {
                sscanf(value.c_str(), "%u", &Nz);
            }
            else if (key ==  "scaling factor (mm/pixel) [1]") {
                sscanf(value.c_str(), "%f", &scale_x);
            }
            else if (key ==  "scaling factor (mm/pixel) [2]") {
                sscanf(value.c_str(), "%f", &scale_y);
            }
            else if (key ==  "slice thickness (pixels)") {
                sscanf(value.c_str(), "%f", &scale_z);    // ????????????????
            }
            else if (key ==  "name of data file") {
                data_file = value;
            }
            else if (key ==  "number format") {
                if ((value == "float") || (value == "FLOAT")) {
                    data_type = 0;
                }
                else if ((value == "unsigned integer") || (value == "UNSIGNED INTEGER")) {
                    data_type = 1;
                }
                else {
                    egsWarning("%s: unrecognised 'number format' type: %s \n",func,value.c_str());
                }
            }
        }
        if (Nx < 1 || Ny < 1 || Nz < 1 || scale_x <= 0.0 || scale_y <= 0.0 || scale_z <= 0.0 || data_file == "" || data_type == -1) {
            egsWarning("%s: invalid interfile header information: "
                       "Nx=%d Ny=%d Nz=%d scale_x=%f scale_y=%f scale_z=%f "
                       "data_file='%s' number_format=%d\n",func,Nx,Ny,Nz,scale_x,scale_y,scale_z,data_file.c_str(),data_type);
            return 0;
        }
        ifstream i_file(data_file.c_str(),ios::binary);
        if (!i_file) {
            egsWarning("%s: failed to open interfile data "
                       "%s\n",func,data_file.c_str());
            return 0;
        }
        scale_x /= 10.0;
        scale_y /= 10.0;
        scale_z /= 10.0; // convert to cm
        {
            int j;
            xx = new EGS_Float [Nx+1];
            for (j=0; j<=Nx; j++) {
                xx[j] = -(float)Nx*scale_x/2.0 + j*scale_x;
            }
            yy = new EGS_Float [Ny+1];
            for (j=0; j<=Ny; j++) {
                yy[j] = -(float)Ny*scale_y/2.0 + j*scale_y;
            }
            zz = new EGS_Float [Nz+1];
            for (j=0; j<=Nz; j++) {
                zz[j] = -(float)Nz*scale_z/2.0 + j*scale_z;
            }
        }
        rho = new float [Nx*Ny*Nz];
        if (data_type == 0) {
            i_file.read((char *)rho, Nx*Ny*Nz*sizeof(float));
        }
        else {
            unsigned short int *rho_tmp = new unsigned short int [Nx*Ny*Nz];
            i_file.read((char *)rho_tmp, (Nx*Ny*Nz)*sizeof(unsigned short int));
            for (int cc = 0; cc<Nx*Ny*Nz; cc++) {
                rho[cc] = (float)(rho_tmp[cc]);
            }
            delete [] rho_tmp;
        }
        if (i_file.fail()) {
            egsWarning("%s: failed reading interfile data\n",func);
            delete [] rho;
            delete [] xx;
            delete [] yy;
            delete [] zz;
            return 0;
        }
    }
    else {
        ifstream data(dens_file);
        if (!data) {
            egsWarning("%s: failed to open .egsphant file %s\n",func,dens_file);
            return 0;
        }
        int nmed;
        data >> nmed;
        if (data.fail()) {
            egsWarning("%s: failed reading number of media\n",func);
            return 0;
        }
        char buf [1024];
        int imed;
        data.getline(buf,1023);
        for (imed=0; imed<nmed; ++imed) {
            data.getline(buf,1023);
            if (data.fail()) {
                egsWarning("%s: failed reading medium name for %d'th medium\n",func,imed+1);
            }
            //egsInformation("Got line <%s>\n",buf);
        }
        // ignore estepe
        EGS_Float dum;
        for (imed=0; imed<nmed; ++imed) {
            data >> dum;
        }
        data >> Nx >> Ny >> Nz;
        if (data.fail()) {
            egsWarning("%s: failed reading number of voxels\n",func);
            return 0;
        }
        xx = new EGS_Float [Nx+1];
        yy = new EGS_Float [Ny+1];
        zz = new EGS_Float [Nz+1];
        int j;
        for (j=0; j<=Nx; ++j) {
            data >> xx[j];
        }
        if (data.fail()) {
            egsWarning("%s: failed reading x-planes\n",func);
            delete [] xx;
            delete [] yy;
            delete [] zz;
            return 0;
        }
        for (j=0; j<=Ny; ++j) {
            data >> yy[j];
        }
        if (data.fail()) {
            egsWarning("%s: failed reading y-planes\n",func);
            delete [] xx;
            delete [] yy;
            delete [] zz;
            return 0;
        }
        for (j=0; j<=Nz; ++j) {
            data >> zz[j];
        }
        if (data.fail()) {
            egsWarning("%s: failed reading z-planes\n",func);
            delete [] xx;
            delete [] yy;
            delete [] zz;
            return 0;
        }
        int nr = Nx*Ny*Nz;
        int med;
        //for(j=0; j<nr; ++j) data >> med;
        data.getline(buf,1023);
        for (int iz=0; iz<Nz; ++iz) {
            for (int iy=0; iy<Ny; ++iy) {
                data.getline(buf,1023);
            }
            data.getline(buf,1023);
        }
        if (data.fail()) {
            egsWarning("%s: failed reading media indeces matrix\n",func);
            delete [] xx;
            delete [] yy;
            delete [] zz;
            return 0;
        }
        rho = new float [nr];
        for (j=0; j<nr; ++j) {
            data >> rho[j];
        }
        if (data.fail()) {
            egsWarning("%s: failed reading mass density matrix\n",func);
            delete [] xx;
            delete [] yy;
            delete [] zz;
            delete [] rho;
            return 0;
        }
    }
    EGS_PlanesX *xp = new EGS_PlanesX(Nx+1,xx,"",EGS_XProjector("x-planes"));
    EGS_PlanesY *yp = new EGS_PlanesY(Ny+1,yy,"",EGS_YProjector("y-planes"));
    EGS_PlanesZ *zp = new EGS_PlanesZ(Nz+1,zz,"",EGS_ZProjector("z-planes"));
    EGS_XYZGeometry *result = new EGS_XYZGeometry(xp,yp,zp);
    EGS_Float rhomin=1e30,rhomax=0;
    int j;
    for (j=0; j<Nx*Ny*Nz; j++) {
        if (rho[j] > rhomax) {
            rhomax = rho[j];
        }
        if (rho[j] < rhomin) {
            rhomin = rho[j];
        }
    }
    egsInformation("Min. density found: %g\n",rhomin);
    egsInformation("Max. density found: %g\n",rhomax);
    int nmed = med_names.size();
    int *imed = new int [nmed];
    for (j=0; j<nmed; j++) {
        imed[j] = -2;
    }
    for (j=0; j<Nx*Ny*Nz; j++) {
        int i;
        for (i=0; i<nmed; i++) {
            if (rho[j] >= rho_min[i] && rho[j] < rho_max[i]) {
                break;
            }
        }
        if (i >= nmed) {
            egsWarning("%s: no material defined for density %g, leaving voxel %d"
                       " as vacuum\n",func,rho[j],j);
            result->setMedium(j,j,-1);
        }
        else {
            if (imed[i] < -1) {
                imed[i] = addMedium(med_names[i]);
                egsInformation("Using medium %s as mednum %d\n",
                               med_names[i].c_str(),imed[i]);
            }
            result->setMedium(j,j,imed[i]);
            if (rho_def[i] > 0) {
                EGS_Float rrho = rho[j]/rho_def[i];
                if (fabs(rrho-1) > 1e-4) {
                    result->setRelativeRho(j,j,rrho);
                }
            }
        }
    }
    delete [] rho;
    delete [] imed;
    return result;
}

string EGS_DeformedXYZ::def_type = "EGS_DeformedXYZ";

char EGS_DeformedXYZ::tetrahedra[] = {0,2,3,6,   0,6,3,7,   0,4,6,7,
                                      7,0,1,3,   7,1,0,4,   7,5,1,4
                                     };

char EGS_DeformedXYZ::plane_order[] = {0,1,2,  0,2,3,  0,3,1,  1,3,2};

char EGS_DeformedXYZ::enter_tetra1[] = {0, 3, 4, 0, 0, 2};
char EGS_DeformedXYZ::enter_plane1[] = {1, 1, 1, 3, 0, 3};
char EGS_DeformedXYZ::enter_tetra2[] = {2, 5, 5, 1, 3, 5};
char EGS_DeformedXYZ::enter_plane2[] = {0, 0, 2, 3, 2, 2};


char EGS_DeformedXYZ::tetra_data[] = {
    2,-1,  2,   // plane 0 in tetrahedron 0
    -1, 0,  1,   // plane 1 in tetrahedron 0
    0,-1,  3,   // plane 2 in tetrahedron 0
    1, 1,  4,   // plane 3 in tetrahedron 0
//------------------------------------------------------------------
    -1, 0,  0,   // plane 0 in tetrahedron 1
    -1, 0,  3,   // plane 1 in tetrahedron 1
    -1, 0,  2,   // plane 2 in tetrahedron 1
    1, 1,  5,   // plane 3 in tetrahedron 1
//------------------------------------------------------------------
    0,-1,  5,   // plane 0 in tetrahedron 2
    -1, 0,  1,   // plane 1 in tetrahedron 2
    -1, 0,  4,   // plane 2 in tetrahedron 2
    2, 1,  0,   // plane 3 in tetrahedron 2
//------------------------------------------------------------------
    -1, 0,  4,   // plane 0 in tetrahedron 3
    0, 1,  0,   // plane 1 in tetrahedron 3
    -1, 0,  1,   // plane 2 in tetrahedron 3
    2,-1,  5,   // plane 3 in tetrahedron 3
//------------------------------------------------------------------
    -1, 0,  3,   // plane 0 in tetrahedron 4
    -1, 0,  2,   // plane 1 in tetrahedron 4
    -1, 0,  5,   // plane 2 in tetrahedron 4
    1,-1,  0,   // plane 3 in tetrahedron 4
//------------------------------------------------------------------
    0, 1,  2,   // plane 0 in tetrahedron 5
    -1, 0,  4,   // plane 1 in tetrahedron 5
    2, 1,  3,   // plane 2 in tetrahedron 5
    1,-1,  1    // plane 3 in tetrahedron 5
};


EGS_DeformedXYZ::EGS_DeformedXYZ(EGS_PlanesX *Xp, EGS_PlanesY *Yp,
                                 EGS_PlanesZ *Zp, const char *defFile, const string &Name) :
    EGS_XYZGeometry(Xp,Yp,Zp,Name), vectors(0) {
    setDeformations(defFile);
    np[0] = nx;
    np[1] = ny;
    np[2] = nz;
    nxyp1 = nx + ny + 1;
}

int EGS_DeformedXYZ::setDeformations(const char *defFile) {
    ifstream data(defFile,ios::binary);
    if (!data) {
        egsWarning("Failed to open deformations file %s\n",defFile);
        return 1;
    }
    int nvec;
    data.read((char *)&nvec,sizeof(int));
    int nneed = (nx+1)*(ny+1)*(nz+1);
    if (nvec != nneed) {
        egsWarning("Inconsistent deformations file: %d instead of %d vectors\n",
                   nvec,nneed);
        return 2;
    }
    if (!vectors) {
        vectors = new EGS_Vector [nvec];
    }
    float tmp[3];
    int i,j,k;
    for (j=0; j<nvec; ++j)  {
        data.read((char *)tmp,3*sizeof(float));
        if (data.fail()) {
            egsWarning("Error while readinf vector %d from file\n",j+1);
            return 3;
        }
        vectors[j] = EGS_Vector(tmp[0],tmp[1],tmp[2]);
    }

    // add plane positions to vectors
    for (k=0; k<=nz; ++k) for (j=0; j<=ny; ++j) for (i=0; i<=nx; ++i) {
                int index = i + j*(nx+1) + k*(nx+1)*(ny+1);
                vectors[index] += EGS_Vector(xpos[i],ypos[j],zpos[k]);
            }

    for (j=0; j<24; ++j) {
        int i = tetrahedra[j];
        switch (i) {
        case 0:
            tnodes[j] = 0;
            break;
        case 1:
            tnodes[j] = 1;
            break;
        case 2:
            tnodes[j] = nx+1;
            break;
        case 3:
            tnodes[j] = nx+2;
            break;
        case 4:
            tnodes[j] = (nx+1)*(ny+1);
            break;
        case 5:
            tnodes[j] = (nx+1)*(ny+1)+1;
            break;
        case 6:
            tnodes[j] = (nx+1)*(ny+1)+nx+1;
            break;
        case 7:
            tnodes[j] = (nx+1)*(ny+1)+nx+2;
            break;
        }
    }

    nreg = 6*nx*ny*nz;

    return 0;
}

EGS_DeformedXYZ::~EGS_DeformedXYZ() {
    if (vectors) {
        delete [] vectors;
    }
}

string EGS_XYZRepeater::type = "EGS_XYZRepeater";

EGS_XYZRepeater::EGS_XYZRepeater(EGS_Float xmin, EGS_Float xmax,
                                 EGS_Float ymin, EGS_Float ymax,
                                 EGS_Float zmin, EGS_Float zmax,
                                 int Nx, int Ny, int Nz,
                                 EGS_BaseGeometry *G,
                                 const string &Name) :
    EGS_BaseGeometry(Name), g(G), nx(Nx), ny(Ny), nz(Nz) {
    dx = (xmax - xmin)/nx;
    dxi = 1/dx;
    ax = -xmin*dxi;
    dy = (ymax - ymin)/ny;
    dyi = 1/dy;
    ay = -ymin*dyi;
    dz = (zmax - zmin)/nz;
    dzi = 1/dz;
    az = -zmin*dzi;
    EGS_PlanesX *xp;
    EGS_PlanesY *yp;
    EGS_PlanesZ *zp;
    xp = new EGS_PlanesX(xmin,dx,nx,"",EGS_XProjector("x-planes"));
    yp = new EGS_PlanesY(ymin,dy,ny,"",EGS_YProjector("y-planes"));
    zp = new EGS_PlanesZ(zmin,dz,nz,"",EGS_ZProjector("z-planes"));
    xyz = new EGS_XYZGeometry(xp,yp,zp);
    xyz->ref();
    g->ref();
    nxy = nx*ny;
    nxyz = nx*ny*nz;
    ng = g->regions();
    nreg = nxyz*ng + 1;
    translation = new EGS_Vector [nxyz];
    for (int ix=0; ix<nx; ix++)
        for (int iy=0; iy<ny; iy++)
            for (int iz=0; iz<nz; iz++)
                translation[ix+iy*nx+iz*nxy] = EGS_Vector(xmin + dx*(0.5+ix),
                                               ymin + dy*(0.5+iy),
                                               zmin + dz*(0.5+iz));

}

EGS_XYZRepeater::~EGS_XYZRepeater() {
    if (!g->deref()) {
        delete g;
    }
    if (!xyz->deref()) {
        delete xyz;
    }
    delete [] translation;
}

void EGS_XYZRepeater::printInfo() const {
    EGS_BaseGeometry::printInfo();
    egsInformation("%d repetitions between xmin=%g and xmax=%g\n",
                   nx,xyz->getXPositions()[0],xyz->getXPositions()[nx]);
    egsInformation("%d repetitions between ymin=%g and ymax=%g\n",
                   ny,xyz->getYPositions()[0],xyz->getYPositions()[ny]);
    egsInformation("%d repetitions between zmin=%g and zmax=%g\n",
                   nz,xyz->getZPositions()[0],xyz->getZPositions()[nz]);
    const char *med_name = getMediumName(med);
    if (med_name) {
        egsInformation("box is filled with %s\n",med_name);
    }
    else {
        egsInformation("box is filled with vacuum\n");
    }
    egsInformation("Repeated geometry:\n");
    g->printInfo();
}

void EGS_XYZGeometry::voxelizeGeometry(EGS_Input *input) {
    string gname;
    int err = input->getInput("voxelize geometry",gname);
    if (err) {
        return;
    }
    egsInformation("\nEGS_XYZGeometry(%s)\n",name.c_str());
    egsInformation("  setting media from geometry %s\n",gname.c_str());
    EGS_BaseGeometry *geometry = EGS_BaseGeometry::getGeometry(gname);
    if (!geometry) {
        egsInformation("  this geometry does not exist -> media will be not set.\n");
        return;
    }
    EGS_AffineTransform *T = EGS_AffineTransform::getTransformation(input);
    if (region_media) {
        delete [] region_media;
    }
    if (rhor) {
        delete [] rhor;
        rhor = 0;
    }
    region_media = new short [nreg];
    bool hrs = geometry->hasRhoScaling();
    if (hrs) {
        has_rho_scaling = true;
        rhor = new EGS_Float [nreg];
    }
    else {
        has_rho_scaling = false;
    }
    EGS_Vector v1(xp->position(0),yp->position(0),zp->position(0));
    EGS_Vector v2(xp->position(nx),yp->position(ny),zp->position(nz));
    egsInformation("  top/left/front corner   : (%g,%g,%g)\n",v1.x,v1.y,v1.z);
    egsInformation("  bottom/right/back corner: (%g,%g,%g)\n",v2.x,v2.y,v2.z);
    if (T) {
        egsInformation("  the above are transformed as follows before looking up media:\n");
        T->transform(v1);
        T->transform(v2);
        egsInformation("  top/left/front corner   : (%g,%g,%g)\n",v1.x,v1.y,v1.z);
        egsInformation("  bottom/right/back corner: (%g,%g,%g)\n",v2.x,v2.y,v2.z);
    }
    for (int ix=0; ix<nx; ++ix) for (int iy=0; iy<ny; iy++) for (int iz=0; iz<nz; ++iz) {
                int ir = ix + iy*nx + iz*nxy;
                EGS_Vector v(0.5*(xp->position(ix)+xp->position(ix+1)),
                             0.5*(yp->position(iy)+yp->position(iy+1)),
                             0.5*(zp->position(iz)+zp->position(iz+1)));
                if (T) {
                    T->transform(v);
                }
                int ireg = geometry->isWhere(v);
                if (ireg >= 0) {
                    region_media[ir] = geometry->medium(ireg);
                    if (hrs) {
                        rhor[ir] = geometry->getRelativeRho(ir);
                    }
                }
                else {
                    region_media[ir] = -1;
                    if (hrs) {
                        rhor[ir] = 1;
                    }
                }
            }
    vector<string> options;
    options.push_back("no");
    options.push_back("yes");
    bool delete_geometry = input->getInput("delete geometry",options,0);
    if (delete_geometry) {
        if (geometry->deref() == -1) {
            egsInformation("  deleting geometry %s as requested\n",geometry->getName().c_str());
            delete geometry;
        }
        else {
            egsInformation("  not deleting geometry %s as requested due to remaining references\n",
                           geometry->getName().c_str());
            geometry->ref();
        }
    }
    egsInformation("\n");
}

const char *err_msg1 = "createGeometry(EGS_XYZRepeater)";

#endif

string EGS_NDGeometry::type = "EGS_NDGeometry";

extern "C" {

    EGS_NDG_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {
#ifdef EXPLICIT_XYZ
        const static char *func = "createGeometry(XYZ)";
        string type;
        int is_xyz = input->getInput("type",type);
        if (!is_xyz && input->compare("EGS_XYZRepeater",type)) {
            string base,medium;
            vector<EGS_Float> xr, yr, zr;
            int err1 = input->getInput("repeated geometry",base);
            int err2 = input->getInput("repeat x",xr);
            int err3 = input->getInput("repeat y",yr);
            int err4 = input->getInput("repeat z",zr);
            int err5 = input->getInput("medium",medium);
            EGS_BaseGeometry *g = 0;
            if (err1) {
                egsWarning("%s: missing 'repeated geometry' input\n",err_msg1);
            }
            else {
                g = EGS_BaseGeometry::getGeometry(base);
                if (!g) {
                    egsWarning("%s: no geometry named %s exists\n",err_msg1,
                               base.c_str());
                    err1 = 1;
                }
            }
            if (err2 || xr.size() != 3) {
                err1 = 1;
                egsWarning("%s: wrong/missing 'repeat x' input\n",err_msg1);
            }
            if (err3 || yr.size() != 3) {
                err1 = 1;
                egsWarning("%s: wrong/missing 'repeat y' input\n",err_msg1);
            }
            if (err4 || zr.size() != 3) {
                err1 = 1;
                egsWarning("%s: wrong/missing 'repeat z' input\n",err_msg1);
            }
            if (err1) {
                return 0;
            }
            EGS_Float xmin = xr[0], xmax = xr[1];
            int nx = (int)(xr[2]+0.1);
            if (xmin >= xmax || nx < 1) {
                egsWarning("%s: wrong 'repeat x' input xmin=%g xmax=%g nx=%d\n",
                           xmin,xmax,nx);
                err1 = 1;
            }
            EGS_Float ymin = yr[0], ymax = yr[1];
            int ny = (int)(yr[2]+0.1);
            if (ymin >= ymax || ny < 1) {
                egsWarning("%s: wrong 'repeat y' input ymin=%g ymax=%g ny=%d\n",
                           ymin,ymax,ny);
                err1 = 1;
            }
            EGS_Float zmin = zr[0], zmax = zr[1];
            int nz = (int)(zr[2]+0.1);
            if (zmin >= zmax || nz < 1) {
                egsWarning("%s: wrong 'repeat z' input zmin=%g zmax=%g nz=%d\n",
                           zmin,zmax,nz);
                err1 = 1;
            }
            if (err1) {
                return 0;
            }

            EGS_XYZRepeater *result =
                new EGS_XYZRepeater(xmin,xmax,ymin,ymax,zmin,zmax,nx,ny,nz,g);
            result->setName(input);
            if (!err5) {
                result->setMedium(medium);
            }
            result->setXYZLabels(input);
            result->setLabels(input);
            return result;
        }
        else if (!is_xyz && input->compare("EGS_XYZGeometry",type)) {
            string dens_file, ramp_file, egsphant_file, interfile_file;
            int ierr1 = input->getInput("density matrix",dens_file);
            int ierr2 = input->getInput("ct ramp",ramp_file);
            int ierr3 = input->getInput("egsphant file",egsphant_file);
            int ierr4 = input->getInput("interfile header",interfile_file);
            int dens_or_egsphant_or_interfile = -1;
            if (!ierr1) {
                dens_or_egsphant_or_interfile = 0;
            }
            else if (!ierr3) {
                dens_or_egsphant_or_interfile = 1;
                dens_file = egsphant_file;
            }
            else if (!ierr4) {
                dens_or_egsphant_or_interfile = 2;
                dens_file = interfile_file;
            }
            if (dens_or_egsphant_or_interfile >= 0 || !ierr2) {
                if (dens_or_egsphant_or_interfile < 0) {
                    egsWarning("%s: no 'density matrix', 'egsphant file' or 'interfile header' input\n",func);
                    return 0;
                }
                if (ierr2) {
                    egsWarning("%s: no 'ct ramp' input\n",func);
                    return 0;
                }
                EGS_XYZGeometry *result =
                    EGS_XYZGeometry::constructGeometry(dens_file.c_str(),ramp_file.c_str(),dens_or_egsphant_or_interfile);
                result->setName(input);
                return result;
            }
            vector<EGS_Float> xpos, ypos, zpos, xslab, yslab, zslab;
            int ix = input->getInput("x-planes",xpos);
            int iy = input->getInput("y-planes",ypos);
            int iz = input->getInput("z-planes",zpos);
            int ix1 = input->getInput("x-slabs",xslab);
            int iy1 = input->getInput("y-slabs",yslab);
            int iz1 = input->getInput("z-slabs",zslab);
            int nx, ny, nz;
            if (!ix1) {
                if (xslab.size() != 3) {
                    egsWarning("createGeometry(XYZ): exactly 3 inputs are required"
                               " when using 'x-slabs' input method\n");
                    ix1 = 1;
                }
                else {
                    nx = (int)(xslab[2]+0.1);
                    if (nx < 1) {
                        egsWarning("createGeometry(XYZ): number of slabs must be"
                                   " positive!\n");
                        ix1 = 1;
                    }
                    if (xslab[1] <= 0) {
                        egsWarning("createGeometry(XYZ): slab thickness must be"
                                   " positive!\n");
                        ix1 = 1;
                    }
                }
            }
            if (ix && ix1) {
                egsWarning("createGeometry(XYZ): wrong/missing 'x-planes' "
                           "and 'x-slabs' input\n");
                return 0;
            }
            if (!iy1) {
                if (yslab.size() != 3) {
                    egsWarning("createGeometry(XYZ): exactly 3 inputs are required"
                               " when using 'y-slabs' input method\n");
                    iy1 = 1;
                }
                else {
                    ny = (int)(yslab[2]+0.1);
                    if (ny < 1) {
                        egsWarning("createGeometry(XYZ): number of slabs must be"
                                   " positive!\n");
                        iy1 = 1;
                    }
                    if (yslab[1] <= 0) {
                        egsWarning("createGeometry(XYZ): slab thickness must be"
                                   " positive!\n");
                        iy1 = 1;
                    }
                }
            }
            if (iy && iy1) {
                egsWarning("createGeometry(XYZ): wrong/missing 'y-planes' "
                           "and 'y-slabs' input\n");
                return 0;
            }
            if (!iz1) {
                if (zslab.size() != 3) {
                    egsWarning("createGeometry(XYZ): exactly 3 inputs are required"
                               " when using 'z-slabs' input method\n");
                    iz1 = 1;
                }
                else {
                    nz = (int)(zslab[2]+0.1);
                    if (nz < 1) {
                        egsWarning("createGeometry(XYZ): number of slabs must be"
                                   " positive!\n");
                        iz1 = 1;
                    }
                    if (zslab[1] <= 0) {
                        egsWarning("createGeometry(XYZ): slab thickness must be"
                                   " positive!\n");
                        iz1 = 1;
                    }
                }
            }
            if (iz && iz1) {
                egsWarning("createGeometry(XYZ): wrong/missing 'z-planes' "
                           "and 'z-slabs' input\n");
                return 0;
            }
            EGS_PlanesX *xp = !ix1 ?
                              new EGS_PlanesX(xslab[0],xslab[1],nx,"",EGS_XProjector("x-planes")) :
                              new EGS_PlanesX(xpos,"",EGS_XProjector("x-planes"));
            EGS_PlanesY *yp = !iy1 ?
                              new EGS_PlanesY(yslab[0],yslab[1],ny,"",EGS_YProjector("y-planes")) :
                              new EGS_PlanesY(ypos,"",EGS_YProjector("y-planes"));
            EGS_PlanesZ *zp = !iz1 ?
                              new EGS_PlanesZ(zslab[0],zslab[1],nz,"",EGS_ZProjector("z-planes")) :
                              new EGS_PlanesZ(zpos,"",EGS_ZProjector("z-planes"));
            EGS_XYZGeometry *result = new EGS_XYZGeometry(xp,yp,zp);

            if (result) {
                egsWarning("**********************************************\n");
                EGS_BaseGeometry *g = result;
                result->setName(input);
                g->setMedia(input);
                result->voxelizeGeometry(input);

                // labels
                result->setXYZLabels(input);
                g->setLabels(input);
            }
            return result;
        }
#endif
        vector<EGS_BaseGeometry *> dims;
        EGS_Input *ij;
        int error = 0;
        while ((ij = input->takeInputItem("geometry",false)) != 0) {
            EGS_BaseGeometry *g = EGS_BaseGeometry::createSingleGeometry(ij);
            if (g) {
                dims.push_back(g);
                g->ref();
            }
            else {
                error++;
            }
            delete ij;
        }
        vector<string> gnames;
        int err1 = input->getInput("dimensions",gnames);
        if (!err1) {
            for (unsigned int j=0; j<gnames.size(); j++) {
                EGS_BaseGeometry *g = EGS_BaseGeometry::getGeometry(gnames[j]);
                if (g) {
                    dims.push_back(g);
                    g->ref();
                }
                else {
                    egsWarning("Geometry %s does not exist\n",gnames[j].c_str());
                    error++;
                }
            }
        }
        if (error) {
            egsWarning("createGeometry(ND_Geometry): %d errors while "
                       "creating/geting geometries defining individual dimensions\n",error);
            return 0;
        }
        if (dims.size() < 2) {
            egsWarning("createGeometry(ND_Geometry): why do you want to "
                       "construct a ND geometry with a single dimension?\n");
            input->print(0,cerr);
            for (int j=0; j<gnames.size(); j++) egsWarning("dimension %d: %s\n",
                        j+1,gnames[j].c_str());
        }
        int n_concav = 0;
        for (int j=0; j<dims.size(); j++) if (!dims[j]->isConvex()) {
                n_concav++;
            }
        bool is_ok = true;
        if (n_concav > 1) {
            egsWarning("createGeometry(ND_Geometry): a ND geometry can not have "
                       " more than one non-convex dimension, yours has %d\n",n_concav);
            is_ok = false;
        }
        else if (n_concav == 1) {
            if (dims[dims.size()-1]->isConvex()) {
                egsWarning("createGeometry(ND_Geometry): the non-convex "
                           "dimension must be the last dimension\n");
                is_ok = false;
            }
        }
        if (!is_ok) {
            for (int j=0; j<dims.size(); j++)
                if (dims[j]->deref()) {
                    delete dims[j];
                }
            return 0;
        }
        int hn_method=0;
        err1 = input->getInput("hownear method",hn_method);
        EGS_BaseGeometry *result;
        if (!err1 && hn_method == 1) {
            result = new EGS_NDGeometry(dims,"",false);
        }
        else {
            result = new EGS_NDGeometry(dims);
        }
        result->setName(input);
        result->setMedia(input);
        result->setLabels(input);
        return result;
    }


    void EGS_XYZGeometry::setXYZLabels(EGS_Input *input) {

        // x,y,z labels
        string inp;
        int err;

        err = input->getInput("set x label", inp);
        if (!err) {
            xp->setLabels(inp);
        }

        err = input->getInput("set y label", inp);
        if (!err) {
            yp->setLabels(inp);
        }

        err = input->getInput("set z label", inp);
        if (!err) {
            zp->setLabels(inp);
        }
    }


    int EGS_NDGeometry::ndRegions(int r, int dim, int dimk, int k, vector<int> &regs) {

        // skip looping over selected dimension
        if (dim == dimk) {
            r += k*n[dimk];
            if (dim == N-1) {
                regs.push_back(r);
            }
            else {
                ndRegions(r, dim+1, dimk, k, regs);
            }
        }

        // last dimension: end recursion and record global region number
        else if (dim == N-1) {
            for (int j=0; j<g[dim]->regions(); j++) {
                regs.push_back(r+j*n[dim]);
            }
        }

        // keep collecting by recursion
        else {
            for (int j=0; j<g[dim]->regions(); j++) {
                ndRegions(r + j*n[dim], dim+1, dimk, k, regs);
            }
        }
    }


    void EGS_NDGeometry::getLabelRegions(const string &str, vector<int> &regs) {

        // label defined in the sub-geometries
        vector<int> local_regs;
        for (int i=0; i<N; i++) {
            local_regs.clear();
            if (g[i]) {
                g[i]->getLabelRegions(str, local_regs);
            }
            if (local_regs.size() == 0) {
                continue;
            }

            // recurse to collect all global regions comprised in each local region
            for (int j=0; j<local_regs.size(); j++) {
                ndRegions(0, 0, i, local_regs[j], regs);
            }
        }

        // label defined in self (nd input block)
        EGS_BaseGeometry::getLabelRegions(str, regs);

    }


    void EGS_XYZGeometry::getLabelRegions(const string &str, vector<int> &regs) {

        vector<int> local_regs;

        // x plane labels
        local_regs.clear();
        xp->getLabelRegions(str, local_regs);
        for (int i=0; i<local_regs.size(); i++) {
            for (int j=0; j<ny; j++) {
                for (int k=0; k<nz; k++) {
                    regs.push_back(local_regs[i] + nx*j + nxy*k);
                }
            }
        }

        // y plane labels
        local_regs.clear();
        yp->getLabelRegions(str, local_regs);
        for (int j=0; j<local_regs.size(); j++) {
            for (int i=0; i<nx; i++) {
                for (int k=0; k<nz; k++) {
                    regs.push_back(i + nx*local_regs[j] + nxy*k);
                }
            }
        }

        // z plane labels
        local_regs.clear();
        zp->getLabelRegions(str, local_regs);
        for (int k=0; k<local_regs.size(); k++) {
            for (int i=0; i<nx; i++) {
                for (int j=0; j<ny; j++) {
                    regs.push_back(i + nx*j + nxy*local_regs[k]);
                }
            }
        }

        // label defined in self (xyz input block)
        EGS_BaseGeometry::getLabelRegions(str, regs);

    }


    void EGS_XYZRepeater::getLabelRegions(const string &str, vector<int> &regs) {

        vector<int> local_regs;

        // label in repeated geometry
        local_regs.clear();
        g->getLabelRegions(str, local_regs);
        for (int i=0; i<nx; i++) {
            for (int j=0; j<ny; j++) {
                for (int k=0; k<nz; k++) {
                    for (int r=0; r<local_regs.size(); r++) {
                        regs.push_back(ng*(i + nx*j + nxy*k) + local_regs[r]);
                    }
                }
            }
        }

        // x plane labels
        local_regs.clear();
        xyz->getXLabelRegions(str, local_regs);
        for (int i=0; i<local_regs.size(); i++) {
            for (int j=0; j<ny; j++) {
                for (int k=0; k<nz; k++) {
                    for (int r=0; r<ng; r++) {
                        regs.push_back(ng*(local_regs[i] + nx*j + nxy*k) + r);
                    }
                }
            }
        }

        // y plane labels
        local_regs.clear();
        xyz->getYLabelRegions(str, local_regs);
        for (int j=0; j<local_regs.size(); j++) {
            for (int i=0; i<nx; i++) {
                for (int k=0; k<nz; k++) {
                    for (int r=0; r<ng; r++) {
                        regs.push_back(ng*(i + nx*local_regs[j] + nxy*k) + r);
                    }
                }
            }
        }

        // z plane labels
        local_regs.clear();
        xyz->getZLabelRegions(str, local_regs);
        for (int k=0; k<local_regs.size(); k++) {
            for (int i=0; i<nx; i++) {
                for (int j=0; j<ny; j++) {
                    for (int r=0; r<ng; r++) {
                        regs.push_back(ng*(i + nx*j + nxy*local_regs[k]) + r);
                    }
                }
            }
        }

        // label defined in self (repeater input block)
        EGS_BaseGeometry::getLabelRegions(str, regs);

    }

}
