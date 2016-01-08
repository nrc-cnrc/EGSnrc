/*
###############################################################################
#
#  EGSnrc egs++ voxelized shape
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
#  Author:          Iwan Kawrakow, 2009
#
#  Contributors:    Frederic Tessier
#
###############################################################################
*/


/*! \file egs_voxelized_shape.cpp
 *  \brief A "voxelized shape": implementation
 *  \IK
 */

#include "egs_voxelized_shape.h"
#include "egs_input.h"
#include "egs_functions.h"

#include <fstream>
using namespace std;

void EGS_VoxelizedShape::EGS_VoxelizedShapeFormat0(const char *fname,
        const string &Name,EGS_ObjectFactory *f) {
    prob=0;
    xpos=0;
    ypos=0;
    zpos=0;
    map=0;
    nx=0;
    ny=0;
    nz=0;
    nxy=0;
    nreg=0;
    type=-1;
    const static char *func = "EGS_VoxelizedShape::EGS_VoxelizedShape";
    otype = "voxelized_shape";
    if (!fname) {
        return;
    }
    ifstream data(fname,ios::binary);
    if (!data) {
        egsWarning("%s: failed to open file %s\n",func,fname);
        return;
    }
    char endian, form;
    data.read(&endian,1);
    data.read(&form,1);
    if (data.fail()) {
        egsWarning("%s: failed to read endianess and format from %s\n",func,fname);
        return;
    }
    if (form < 0 || form > 1) {
        egsWarning("%s: unknwon format %d found in file %s\n",func,(int)form,fname);
        return;
    }
    if (endian != egsGetEndian()) {
        egsWarning("%s: data in %s from a machine with different endianess.\n"
                   "  Byte swaping not implemented yet.\n",func,fname);
        return;
    }
    short snx, sny, snz;
    data.read((char *)&snx,sizeof(short));
    data.read((char *)&sny,sizeof(short));
    data.read((char *)&snz,sizeof(short));
    if (data.fail()) {
        egsWarning("%s: failed to read number of voxels from %s\n",func,fname);
        return;
    }
    if (snx < 1 || snx > 10000 || sny < 1 || sny > 10000 || snz < 1 || snz > 10000) {
        egsWarning("%s: number of voxels seems strange: nx=%d, ny=%d, nz=%d\n",
                   func,snx,sny,snz);
        return;
    }
    nx = snx;
    ny = sny;
    nz = snz;
    nxy = nx*ny;
    nreg = nxy*nz;
    egsInformation("Distribution has %d x %d x %d voxels\n",nx,ny,nz);
    float *x = new float [nx+1], *y = new float [ny+1], *z = new float [nz+1];
    data.read((char *)x, (nx+1)*sizeof(float));
    data.read((char *)y, (ny+1)*sizeof(float));
    data.read((char *)z, (nz+1)*sizeof(float));
    if (data.fail()) {
        egsWarning("%s: failed to read voxel boundaries\n",func);
        delete [] x;
        delete [] y;
        delete [] z;
        return;
    }
    int nmap;
    float *p;
    if (form == 0) {
        p = new float [nreg];
        egsInformation("Format 0, reading %d values\n",nreg);
        data.read((char *)p, nreg*sizeof(float));
        if (data.fail()) {
            egsWarning("%s: failed to read probabilities\n",func);
            delete [] p;
            delete [] x;
            delete [] y;
            delete [] z;
            return;
        }
        nmap = nreg;
    }
    else {
        egsInformation("Format 1, reading data\n");
        data.read((char *)&nmap, sizeof(int));
        if (data.fail() || nmap < 1) {
            egsWarning("%s: failed to read nmap\n",func);
            delete [] x;
            delete [] y;
            delete [] z;
            return;
        }
        map = new int [nmap];
        p = new float [nmap];
        data.read((char *)map, nmap*sizeof(int));
        data.read((char *)p, nmap*sizeof(float));
        if (data.fail()) {
            egsWarning("%s: failed to read probabilities and bin numbers\n",func);
            delete [] p;
            delete [] map;
            delete [] x;
            delete [] y;
            delete [] z;
            return;
        }
    }
    EGS_Float *p1 = new EGS_Float [nmap];
    int j;
    for (j=0; j<nmap; ++j) {
        p1[j] = p[j];
    }
    delete [] p;
    egsInformation("Making alias table\n");
    prob = new EGS_SimpleAliasTable(nmap,p1);
    xpos = new EGS_Float [nx+1];
    ypos = new EGS_Float [ny+1];
    zpos = new EGS_Float [nz+1];
    for (j=0; j<=nx; ++j) {
        xpos[j] = x[j];
    }
    for (j=0; j<=ny; ++j) {
        ypos[j] = y[j];
    }
    for (j=0; j<=nz; ++j) {
        zpos[j] = z[j];
    }
    delete [] x;
    delete [] y;
    delete [] z;
    type = form;
    egsInformation("Done\n");
}

EGS_VoxelizedShape::EGS_VoxelizedShape(int file_format, const char *fname,
                                       const string &Name,EGS_ObjectFactory *f) : EGS_BaseShape(Name,f),
    prob(0), xpos(0), ypos(0), zpos(0), map(0), nx(0), ny(0), nz(0), nxy(0),
    nreg(0), type(-1) {
    const static char *func = "EGS_VoxelizedShape::EGS_VoxelizedShape";
    if (file_format == 0) { // binary file format -> call original constructor
        string s(fname);
        EGS_VoxelizedShapeFormat0(s.c_str());
        return;
    }
    if (file_format != 1) { // unknown file format
        egsWarning("%s: unknown file format = %d\n",func,file_format);
        return;
    }
    // interfile format -> continue
    otype = "voxelized_shape";
    if (!fname) {
        return;
    }
    ifstream h_file(fname);
    if (!h_file) {
        egsWarning("%s: failed to open file %s\n",func,fname);
        return;
    }
    string data_file("");
    int data_type = -1;
    int Nx, Ny, Nz;
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
            else if ((value == "signed integer") || (value == "SIGNED INTEGER")) {
                data_type = 2;
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
        return;
    }
    nx = Nx;
    ny = Ny;
    nz = Nz;
    nxy = nx*ny;
    nreg = nxy*nz;
    egsInformation("Distribution has %d x %d x %d voxels\n",nx,ny,nz);
    float *x = new float [nx+1], *y = new float [ny+1], *z = new float [nz+1];
    scale_x /= 10.0;
    scale_y /= 10.0;
    scale_z /= 10.0; // convert to cm
    {
        int j;
        for (j=0; j<=nx; j++) {
            x[j] = -(float)nx*scale_x/2.0 + j*scale_x;
        }
        for (j=0; j<=ny; j++) {
            y[j] = -(float)ny*scale_y/2.0 + j*scale_y;
        }
        for (j=0; j<=nz; j++) {
            z[j] = -(float)nz*scale_z/2.0 + j*scale_z;
        }
    }
    ifstream i_file(data_file.c_str(),ios::binary);
    if (!i_file) {
        egsWarning("%s: failed to open interfile data "
                   "%s\n",func,data_file.c_str());
        return;
    }
    int nmap = nreg;
    float *p = new float [nreg];
    if (data_type == 0) {
        i_file.read((char *)p, nreg*sizeof(float));
    }
    else if (data_type == 1) {
        unsigned short int *p_tmp = new unsigned short int [nreg];
        i_file.read((char *)p_tmp, nreg*sizeof(unsigned short int));
        for (int cc = 0; cc<nreg; cc++) {
            p[cc] = (float)(p_tmp[cc]);
        }
        delete [] p_tmp;
    }
    else {
        short int *p_tmp = new short int [nreg];
        i_file.read((char *)p_tmp, nreg*sizeof(short int));
        for (int cc = 0; cc<nreg; cc++) {
            p[cc] = (float)(p_tmp[cc]);
        }
        delete [] p_tmp;
    }
    EGS_Float *p1 = new EGS_Float [nmap];
    int j;
    for (j=0; j<nmap; ++j) {
        p1[j] = p[j];
    }
    delete [] p;
    egsInformation("Making alias table\n");
    prob = new EGS_SimpleAliasTable(nmap,p1);
    xpos = new EGS_Float [nx+1];
    ypos = new EGS_Float [ny+1];
    zpos = new EGS_Float [nz+1];
    for (j=0; j<=nx; ++j) {
        xpos[j] = x[j];
    }
    for (j=0; j<=ny; ++j) {
        ypos[j] = y[j];
    }
    for (j=0; j<=nz; ++j) {
        zpos[j] = z[j];
        egsInformation("%f ", zpos[j]);
    }
    delete [] x;
    delete [] y;
    delete [] z;
    type = 0;
    egsInformation("Done\n");
}

EGS_VoxelizedShape::~EGS_VoxelizedShape() {
    if (isValid()) {
        delete prob;
        delete [] xpos;
        delete [] ypos;
        delete [] zpos;
        if (type == 1) {
            delete [] map;
        }
    }
}


extern "C" {

    EGS_VOXELIZED_SHAPE_EXPORT EGS_BaseShape *createShape(EGS_Input *input,
            EGS_ObjectFactory *f) {
        static const char *func = "createShape(voxelized shape)";
        if (!input) {
            egsWarning("%s: null input?\n",func);
            return 0;
        }
        string fname;
        int err = input->getInput("file name",fname);
        int file_format;
        int err2 = input->getInput("file format",file_format);
        if (err) {
            egsWarning("%s: missing 'file name' input\n",func);
            return 0;
        }
        if (err2) {
            egsInformation("%s: 'file format' input missing. Using default 'binary'"
                           "file format \n",func);
            file_format = 0;
        }
        EGS_VoxelizedShape *shape = new EGS_VoxelizedShape(file_format, fname.c_str());
        if (!shape->isValid()) {
            delete shape;
            return 0;
        }
        shape->setName(input);
        shape->setTransformation(input);
        return shape;
    }

}
