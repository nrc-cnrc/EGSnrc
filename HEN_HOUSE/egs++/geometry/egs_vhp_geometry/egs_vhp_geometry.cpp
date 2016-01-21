/*
###############################################################################
#
#  EGSnrc egs++ voxelized human phantom geometry
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
#  Contributors:    Frederic Tessier
#
###############################################################################
#
#  A "Voxelized Human Phantom" (VHP) geometry.
#
#  Although a VHP geometry is just a XYZ geometry, due to the huge number of
#  voxels one has to be more careful with memory use for media indices, etc.
#  That's why this separate geometry implementation. With Richard Kramer's
#  phantom data the implementation requires ~12 MB to hold all the organ data
#  in memory. Richard's ASCII phantom data was also transformed into binary
#  format with the result that loading them is almost instantaneous.
#
#  For now the implementation only provides the macro geometry, i.e., no
#  spongiosa micro matrices. Adding the micro matrices is the next step.
#
###############################################################################
*/


/*! \file egs_vhp_geometry.cpp
 *  \brief Voxelized Human Phantom (VHP) geometry: implementation
 *  \IK
 */

#include "egs_vhp_geometry.h"
#include "egs_input.h"
#include "egs_functions.h"

#include <vector>
#include <fstream>
#ifndef NO_SSTREAM
    #include <sstream>
    #define S_STREAM std::istringstream
#else
    #include <strstream>
    #define S_STREAM std::istrstream
#endif


using namespace std;

#ifndef SKIP_DOXYGEN
EGS_VoxelInfo *EGS_VoxelGeometry::v = 0;
int            EGS_VoxelGeometry::nv = 0;

VHP_OrganData::VHP_OrganData(const char *fname, int slice_min, int slice_max)
    : nslice(0), ok(false) {
    ifstream data(fname,ios::binary);
    if (!data) {
        egsWarning("VHP_OrganData: failed to open file %s for reading\n",
                   fname);
        return;
    }
    char endian;
    data.read(&endian,sizeof(endian));
    char my_endian = egsGetEndian();
    if (endian != my_endian) {
        egsWarning("VHP_OrganData: wrong endianess. \n"
                   "This machine has endianess %d, data was written on %d\n"
                   "This requires byte swaping, which has not been implemented yet\n",
                   (int)my_endian,(int)endian);
        return;
    }
    data.read((char *)&dx, sizeof(dx));
    data.read((char *)&dy, sizeof(dy));
    data.read((char *)&dz, sizeof(dz));
    data.read((char *)&nslice,sizeof(nslice));
    if (slice_min < 0) {
        slice_min = 0;
    }
    if (slice_max > nslice) {
        slice_max = nslice;
    }
    int nwant = slice_max - slice_min;
    slices = new VHP_SliceInfo [nwant];
    nx = 0, ny = 0;
    int is = 0;
    for (int islice=0; islice<nslice; ++islice) {
        if (islice >= slice_min) {
            slices[is].readData(data);
            int ni, nj;
            slices[is++].getSliceDimensions(ni,nj);
            if (ni > nx) {
                nx = ni;
            }
            if (nj > ny) {
                ny = nj;
            }
        }
        else {
            VHP_SliceInfo tmp;
            tmp.readData(data);
        }
        if (islice == slice_max - 1) {
            break;
        }
    }
    nxy = nx*ny;
    nslice = slice_max - slice_min;
    if (data.fail()) {
        egsWarning("VHP_OrganData: I/O error while reading data\n");
    }
    else {
        ok = true;
    }
}

VHP_OrganData::~VHP_OrganData() {
    if (nslice) {
        delete [] slices;
    }
}

void VHP_RowInfo::readData(istream &in) {
    unsigned short n;
    in.read((char *)&n,sizeof(n));
    if (n != nbin) {
        if (nbin > 0) {
            delete [] pix;
            delete [] org;
            nbin = 0;
        }
    }
    if (n < 1) {
        return;
    }
    if (!nbin) {
        nbin = n;
        pix = new unsigned short [nbin+1];
        org = new unsigned char [nbin];
    }
    in.read((char *)pix,(nbin+1)*sizeof(unsigned short));
    in.read((char *)org,nbin*sizeof(unsigned char));
}

void VHP_SliceInfo::readData(istream &in) {
    if (rinfo) {
        delete [] rinfo;
        rinfo = 0;
    }
    in.read((char *)&firstr,sizeof(firstr));
    in.read((char *)&lastr,sizeof(lastr));
    int nrow = lastr-firstr+1;
    if (nrow > 0) {
        rinfo = new VHP_RowInfo [nrow];
        for (int j=0; j<nrow; ++j) {
            rinfo[j].readData(in);
        }
    }
}
#endif

EGS_VHPGeometry::EGS_VHPGeometry(const char *phantom_file,
                                 const char *media_file, int slice_min, int slice_max,
                                 const string &Name) :
    EGS_BaseGeometry(Name), vg(0), micros(0), nmicro(0) {
    organs = new VHP_OrganData(phantom_file,slice_min,slice_max);
    if (!organs->isOK()) {
        egsWarning("EGS_VHPGeometry: failed to construct organ data\n");
        delete organs;
        organs = 0;
        return;
    }
    ifstream med_data(media_file);
    if (!med_data) {
        egsWarning("EGS_VHPGeometry: failed to open media data file %s\n",
                   media_file);
        delete organs;
        organs = 0;
        return;
    }
    int norg;
    med_data >> norg;
    if (norg < 1) {
        egsWarning("EGS_VHPGeometry: %d organs in media data file %s?\n",
                   norg,media_file);
        delete organs;
        organs = 0;
        return;
    }
    int j;
    for (j=0; j<256; ++j) {
        organ_media[j] = -1;
        organ_names[j] = "Undefined";
        organ_micro[j] = -1;
    }
    char buf[1024];
    med_data.getline(buf,1023);
    for (j=0; j<norg; ++j) {
        int iorg, imed;
        char c;
        med_data >> iorg >> imed >> c;
        med_data.getline(buf,1023);
        if (iorg < 0 || iorg > 255)
            egsWarning("EGS_VHPGeometry: bad organ number %d on line %d\n",
                       iorg,j+2);
        else {
            organ_media[iorg] = imed;
        }
        organ_names[iorg] = c;
        organ_names[iorg] += buf;
    }
    int nmed;
    med_data >> nmed;
    med_data.getline(buf,1023);
    vector<string> media_names;
    vector<int> media_indeces;
    for (j=0; j<nmed; ++j) {
        med_data.getline(buf,1023);
        string med(buf);
        media_names.push_back(med);
        media_indeces.push_back(EGS_BaseGeometry::addMedium(med));
    }
    egsInformation("EGS_VHPGeometry: media information\n");
    for (j=0; j<nmed; ++j) {
        egsInformation("Medium %s registered as medium number %d\n",
                       media_names[j].c_str(),media_indeces[j]);
    }
    for (j=0; j<256; ++j) {
        if (organ_media[j] >= 0) {
            if (organ_media[j] >= nmed) {
                egsWarning("EGS_VHPGeometry: invalid medium index %d\n",
                           organ_media[j]);
            }
            else {
                organ_media[j] = media_indeces[organ_media[j]];
            }
        }
    }
    EGS_Float dx, dy, dz;
    int nx, ny, nz;
    organs->getVoxelSizes(dx,dy,dz);
    organs->getPhantomDimensions(nx,ny,nz);
    vg = new EGS_VoxelGeometry(nx,ny,nz,dx,dy,dz);
    nreg = nx*ny*nz;
    nmacro = nreg;
    egsInformation("Phantom size in voxels: x=%d y=%d z=%d\n",nx,ny,nz);
    egsInformation("Voxel sizes: dx=%g dy=%g dz=%g\n",dx,dy,dz);
}

void EGS_VHPGeometry::setMicros(EGS_Input *input) {
    if (!vg || !organs) {
        egsWarning("EGS_VHPGeometry::setMicros: called for an invalid "
                   "VHP geometry\n");
        return;
    }
    if (!input->getInputItem("micro matrix")) {
        egsWarning("EGS_VHPGeometry::setMicros: no micro matrix definitions\n");
        return;
    }
    vector<EGS_MicroMatrixCluster *> mv;
    string dir;
    input->getInput("micro matrix folder",dir);
    EGS_Float t_bsc;
    bool ok = true;
    const static char *err_msg =
        "EGS_VHPGeometry::setMicros: wrong/missing '%s' input\n";
    int err = input->getInput("BSC thickness",t_bsc);
    if (err || t_bsc < 0) {
        egsWarning(err_msg,"BSC thickness");
        ok = false;
    }
    string tb_medium, bm_medium;
    err = input->getInput("TB medium",tb_medium);
    if (err) {
        egsWarning(err_msg,"TB medium");
        ok = false;
    }
    err = input->getInput("BM medium",bm_medium);
    if (err) {
        egsWarning(err_msg,"BM medium");
        ok = false;
    }
    if (!ok) {
        egsWarning("EGS_VHPGeometry::setMicros: wrong/missing inputs found\n"
                   "  => no micro matrices set for use\n");
        return;
    }
    int tb_med = addMedium(tb_medium), bm_med = addMedium(bm_medium);
    vector<string> minput;
    while (!input->getInput("micro matrix",minput)) {
        delete input->takeInputItem("micro matrix");
        if (minput.size() < 2) {
            egsWarning("EGS_VHPGeometry::setMicros: 'micro matrix' input"
                       " requires at least two inputs => ignored\n");
            continue;
        }
        string fname = dir.size() ? egsJoinPath(dir,minput[0]) : minput[0];
        EGS_MicroMatrixCluster *mcluster = new EGS_MicroMatrixCluster(
            vg->dx,vg->dy,vg->dz,t_bsc,tb_med,bm_med,fname.c_str());
        if (!mcluster->isValid()) {
            egsWarning("failed to construct micro cluster from %s\n",
                       fname.c_str());
            delete mcluster;
        }
        else {
            bool ok = true;
            vector<int> orgs;
            for (int i=1; i<minput.size(); ++i) {
                S_STREAM s(minput[i].c_str());
                int org;
                s >> org;
                if (s.fail()) {
                    egsWarning("EGS_VHPGeometry::setMicros: invalid input %s "
                               "for micro matrix %s\n",minput[i].c_str(),
                               minput[0].c_str());
                    ok = false;
                }
                else {
                    if (org < 0 || org > 255) {
                        egsWarning("EGS_VHPGeometry::setMicros: invalid input"
                                   " %d for micro matrix %s\n",org,minput[0].c_str());
                        ok = false;
                    }
                    else {
                        orgs.push_back(org);
                    }
                }
            }
            if (ok) {
                mv.push_back(mcluster);
                egsInformation("Using micro matrix data from\n    %s\nfor"
                               " organs:",fname.c_str());
                for (int i=0; i<orgs.size(); ++i) {
                    egsInformation(" %d",orgs[i]);
                    organ_micro[orgs[i]] = mv.size()-1;
                }
                egsInformation("\n");
            }
            else {
                delete mcluster;
            }
        }
    }
    if (mv.size() > 0) {
        if (nmicro > 0) {
            for (int i=0; i<nmicro; ++i) {
                delete micros[i];
            }
            delete [] micros;
        }
        nmicro = mv.size();
        micros = new EGS_MicroMatrixCluster* [nmicro];
        nmax = 0;
        for (int i=0; i<nmicro; ++i) {
            micros[i] = mv[i];
            int nr = micros[i]->nxy*micros[i]->nz;
            int nm = micros[i]->mxy*micros[i]->mz;
            if (nm*nr > nmax) {
                nmax = nm*nr;
            }
        }
        nmacro = vg->nxy*vg->nz;
        EGS_I64 ntot = nmax;
        ntot *= nmicro;
        ntot += nmacro;
        if (ntot > 2147483647) egsFatal("EGS_VHPGeometry::setMicros: "
                                            "too many micro regions\n");
        nreg = nmacro + nmax*nmicro;
        egsInformation("Using regions 0...%d to identify macro voxels\n",
                       nmacro-1);
        egsInformation("Using regions %d...%d to identify micro voxels\n",
                       nmacro,nreg-1);
    }
}

EGS_VHPGeometry::~EGS_VHPGeometry() {
    if (organs) {
        delete organs;
    }
    if (vg) {
        delete vg;
    }
    if (nmicro > 0) {
        for (int j=0; j<nmicro; ++j) {
            delete micros[j];
        }
        delete [] micros;
    }

}

void EGS_VHPGeometry::setMedia(EGS_Input *,int,const int *) {
    egsWarning("EGS_VHPGeometry::setMedia: don't use this method.\n"
               " Media are fixed by the organ data defining an VHP geometry\n");
}

void EGS_VHPGeometry::printInfo() const {
    EGS_BaseGeometry::printInfo();
    if (isOK()) {
        egsInformation("  voxel sizes: %g %g %g\n",vg->dx,vg->dy,vg->dz);
        egsInformation("  phantom size: %d %d %d\n",vg->nx,vg->ny,vg->nz);
    }
    else {
        egsInformation("  undefined VHP geometry\n");
    }
}

string EGS_VHPGeometry::type = "EGS_VHPGeometry";

#ifndef SKIP_DOXYGEN
VHPBox EGS_MicroMatrixCluster::bm_boxes[64];
bool   EGS_MicroMatrixCluster::bm_boxes_initialized = false;

EGS_MicroMatrixCluster::EGS_MicroMatrixCluster(
    EGS_Float Dx, EGS_Float Dy, EGS_Float Dz, EGS_Float bsc_thickness,
    int tb_med, int bm_med, const char *micro_file) : micros(0), vg(0),
    med_tb(tb_med), med_bm(bm_med) {
    //
    // *** open micro-matrix file
    //
    ifstream in(micro_file,ios::binary);
    if (!in) {
        egsWarning("EGS_MicroMatrixCluster: failed to open file %s\n",
                   micro_file);
        return;
    }

    //
    // *** read data
    //
    unsigned char Mx, My, Mz;
    in.read((char *)&Mx,sizeof(Mx));
    in.read((char *)&My,sizeof(My));
    in.read((char *)&Mz,sizeof(Mz));
    mx = Mx;
    my = My;
    mz = Mz;
    mxy = mx*my;
    nmic = mxy*mz;
    micros = new unsigned char *[nmic];
    in.read((char *)&Mx,sizeof(Mx));
    in.read((char *)&My,sizeof(My));
    in.read((char *)&Mz,sizeof(Mz));
    nx = Mx;
    ny = My;
    nz = Mz;
    nxy = nx*ny;
    int nxyz = nxy*nz, j;
    nreg = nxyz;
    {
        for (int j=0; j<nmic; ++j) {
            micros[j] = new unsigned char [nxyz];
            in.read((char *)micros[j],nxyz*sizeof(unsigned char));
            if (in.fail()) {
                egsWarning("EGS_MicroMatrixCluster: I/O error while reading "
                           "%d'th micto-matrix from file %s\n",j+1,micro_file);
                for (int i=0; i<=j; ++i) {
                    delete [] micros[j];
                }
                delete [] micros;
                micros = 0;
                return;
            }
        }
    }

    //
    // *** initialize micro- and macro-voxel sizes and numbers
    //
    dx = Dx;
    dy = Dy;
    dz = Dz;
    dxi = 1/dx;
    dyi = 1/dy;
    dzi = 1/dz;
    EGS_Float ddx = Dx/nx, ddy = Dy/ny, ddz = Dz/nz;
    egsInformation("Macro voxel sizes: %g %g %g\n",dx,dy,dz);
    egsInformation("Micro voxel sizes: %g %g %g\n",ddx,ddy,ddz);
    egsInformation("Number of micros: %d %d %d\n",mx,my,mz);
    egsInformation("Number of micro-voxles: %d %d %d\n",nx,ny,nz);

    //
    // *** create an object for geometry computations
    //
    vg = new EGS_VoxelGeometry(nx,ny,nz,ddx,ddy,ddz);

    //
    // *** initialize the 64 boxes needed for sub-micro-voxel energy deposition
    //     if needed
    //
    if (!bm_boxes_initialized) {
        bm_boxes_initialized = true;
        for (int ixm=0; ixm<2; ++ixm) {
            EGS_Float xmin = bsc_thickness*ixm;
            for (int ixp=0; ixp<2; ++ixp) {
                EGS_Float xmax = ddx - bsc_thickness*ixp;
                for (int iym=0; iym<2; ++iym) {
                    EGS_Float ymin = bsc_thickness*iym;
                    for (int iyp=0; iyp<2; ++iyp) {
                        EGS_Float ymax = ddy - bsc_thickness*iyp;
                        for (int izm=0; izm<2; ++izm) {
                            EGS_Float zmin = bsc_thickness*izm;
                            for (int izp=0; izp<2; ++izp) {
                                EGS_Float zmax = ddz - bsc_thickness*izp;
                                int ibox = ixm+2*ixp+4*iym+8*iyp+16*izm+32*izp;
                                bm_boxes[ibox].xmin = xmin;
                                bm_boxes[ibox].xmax = xmax;
                                bm_boxes[ibox].ymin = ymin;
                                bm_boxes[ibox].ymax = ymax;
                                bm_boxes[ibox].zmin = zmin;
                                bm_boxes[ibox].zmax = zmax;
                            }
                        }
                    }
                }
            }
        }
    }

    //
    // *** process micro data. After this loop the mico-matrices will contain
    //     0  for TB voxels
    //     1  for BSC-only voxels (possible if BSC thickness > voxel size/2)
    //     2  for BM-only voxels (i.e., no TB neighbours)
    //    >2  index of VHPBox corresponding to the BSC layers found in the voxel
    //
    //     we also compute the TB, BM and BSC volumes
    //
    v_tb  = new EGS_Float [nmic];
    v_bm  = new EGS_Float [nmic];
    v_bsc = new EGS_Float [nmic];
    double tot_tb = 0, tot_bm = 0, tot_bsc = 0;
    for (j=0; j<nmic; ++j) {
        int iz = j/mxy;
        int iy = (j-iz*mxy)/mx;
        int ix = j-iz*mxy-iy*mx;
        int ixm = ix-1;
        if (ixm < 0) {
            ixm = mx-1;
        }
        int ixp = ix+1;
        if (ixp >= mx) {
            ixp = 0;
        }
        int iym = iy-1;
        if (iym < 0) {
            iym = my-1;
        }
        int iyp = iy+1;
        if (iyp >= my) {
            iyp = 0;
        }
        int izm = iz-1;
        if (izm < 0) {
            izm = mz-1;
        }
        int izp = iz+1;
        if (izp >= mz) {
            izp = 0;
        }
        EGS_Float bsc_vol = 0, bm_vol = 0, tb_vol = 0;
        for (int jx=0; jx<nx; ++jx) {
            for (int jy=0; jy<ny; ++jy) {
                for (int jz=0; jz<nz; ++jz) {
                    int ireg = jx + jy*nx + jz*nxy;
                    if (micros[j][ireg]) {
                        int micxm = !(jx > 0 ? micros[j][ireg-1] :
                                      micros[ixm+iy*mx+iz*mxy][nx-1+jy*nx+jz*nxy]);
                        int micxp = !(jx < nx-1 ? micros[j][ireg+1] :
                                      micros[ixp+iy*mx+iz*mxy][jy*nx+jz*nxy]);
                        int micym = !(jy > 0 ? micros[j][ireg-nx] :
                                      micros[ix+iym*mx+iz*mxy][jx+(ny-1)*nx+jz*nxy]);
                        int micyp = !(jy < ny-1 ? micros[j][ireg+nx] :
                                      micros[ix+iyp*mx+iz*mxy][jx+jz*nxy]);
                        int miczm = !(jz > 0 ? micros[j][ireg-nxy] :
                                      micros[ix+iy*mx+izm*mxy][jx+jy*nx+(nz-1)*nxy]);
                        int miczp = !(jz < nz-1 ? micros[j][ireg+nxy] :
                                      micros[ix+iy*mx+izp*mxy][jx+jy*nx]);
                        if ((micxm && micxp && 2*bsc_thickness>ddx) ||
                                (micym && micyp && 2*bsc_thickness>ddy) ||
                                (miczm && miczp && 2*bsc_thickness>ddz)) {
                            micros[j][ireg] = 1; // i.e. all BSC
                            bsc_vol += 1;
                        }
                        else {
                            int ibox=micxm+2*micxp+4*micym+8*micyp+
                                     16*miczm+32*miczp;
                            micros[j][ireg] = ibox + 2;
                            if (!ibox) {
                                bm_vol += 1;
                            }
                            else {
                                double v =
                                    (bm_boxes[ibox].xmax-bm_boxes[ibox].xmin)*
                                    (bm_boxes[ibox].ymax-bm_boxes[ibox].ymin)*
                                    (bm_boxes[ibox].zmax-bm_boxes[ibox].zmin);
                                v /= (ddx*ddy*ddz);
                                bm_vol += v;
                                bsc_vol += 1 - v;
                            }
                        }
                    }
                    else {
                        tb_vol += 1;
                    }
                }
            }
        }
        egsInformation("Micro-matrix %d from %s:\n",j+1,micro_file);
        egsInformation("  TB volume fraction: %g\n",tb_vol/nxyz);
        egsInformation("  BM volume fraction: %g\n",bm_vol/nxyz);
        egsInformation(" BSC volume fraction: %g\n",bsc_vol/nxyz);
        egsInformation("               Total: %g\n",(tb_vol+bm_vol+bsc_vol)/nxyz);
        v_tb[j] = tb_vol*ddx*ddy*ddz;
        v_bm[j] = bm_vol*ddx*ddy*ddz;
        v_bsc[j] = bsc_vol*ddx*ddy*ddz;
        tot_tb += tb_vol;
        tot_bm += bm_vol;
        tot_bsc += bsc_vol;
    }
    egsInformation("Average volume fractions:\n");
    egsInformation("  TB volume fraction: %g\n",tot_tb/(nmic*nxyz));
    egsInformation("  BM volume fraction: %g\n",tot_bm/(nmic*nxyz));
    egsInformation(" BSC volume fraction: %g\n",tot_bsc/(nmic*nxyz));
}

EGS_MicroMatrixCluster::~EGS_MicroMatrixCluster() {
}
#endif

EGS_TestMicro::EGS_TestMicro(EGS_Float Dx, EGS_Float Dy, EGS_Float Dz,
                             EGS_Float bsc_t,int tb_med, int bm_med,const char *micro_file,
                             const string &Name) : EGS_BaseGeometry(Name) {
    micro = new EGS_MicroMatrixCluster(Dx,Dy,Dz,bsc_t,tb_med,bm_med,micro_file);
    vg = new EGS_VoxelGeometry(micro->mx,micro->my,micro->mz,Dx,Dy,Dz);
    nr = micro->nx*micro->ny*micro->nz;
    nreg = nr*micro->mx*micro->my*micro->mz;
}

void EGS_TestMicro::setMedia(EGS_Input *, int , const int *) {
    egsFatal("EGS_TestMicro::setMedia(EGS_Input*,int,const int *):\n"
             "  Don't use this method. Media are only set via the micro matrix"
             " data\n");
}

EGS_TestMicro::~EGS_TestMicro() {
}

string EGS_TestMicro::type = "EGS_TestMicro";

const static EGS_VHP_LOCAL char *vhp_error_msg1 =
    "createGeometry(VHP): wrong/missing %s input for micro_test geometry\n";

extern "C" {

    EGS_VHP_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {
        string type;
        int err = input->getInput("type",type);
        if (!err && type == "micro_test") {
            bool ok = true;
            vector<EGS_Float> vs;
            err = input->getInput("voxel sizes",vs);
            if (err || vs.size() != 3) {
                egsWarning(vhp_error_msg1,"'voxel sizes'");
                ok = false;
            }
            EGS_Float bsc_t;
            err = input->getInput("BSC thickness",bsc_t);
            if (err) {
                egsWarning(vhp_error_msg1,"'BSC thickness'");
                ok = false;
            }
            string tb_medium, bm_medium;
            int err1 = input->getInput("TB medium",tb_medium);
            int err2 = input->getInput("BM medium",bm_medium);
            if (err1) {
                egsWarning(vhp_error_msg1,"'TB medium'");
            }
            if (err2) {
                egsWarning(vhp_error_msg1,"'BM medium'");
            }
            if (err1 || err2) {
                ok = false;
            }
            string micro_data;
            err = input->getInput("micro data file",micro_data);
            if (err) {
                egsWarning(vhp_error_msg1,"'micro data file'");
                ok = false;
            }
            if (!ok) {
                return 0;
            }
            int tb_med = EGS_BaseGeometry::addMedium(tb_medium);
            int bm_med = EGS_BaseGeometry::addMedium(bm_medium);
            EGS_TestMicro *result = new EGS_TestMicro(vs[0],vs[1],vs[2],bsc_t,
                    tb_med,bm_med,micro_data.c_str());
            result->setName(input);
            result->setLabels(input);
            return result;
        }
        string phantom, media;
        int err1 = input->getInput("phantom data",phantom);
        int err2 = input->getInput("media data",media);
        if (err1) egsWarning("createGeometry(EGS_VHPGeometry): "
                                 "missing 'phantom data' input\n");
        if (err2) egsWarning("createGeometry(EGS_VHPGeometry): "
                                 "missing 'media data' input\n");
        if (err1 || err2) {
            return 0;
        }
        vector<int> srange;
        err1 = input->getInput("slice range",srange);
        EGS_VHPGeometry *result;
        if (!err1 && srange.size() == 2 && srange[1] > srange[0]) result =
                new EGS_VHPGeometry(phantom.c_str(),media.c_str(),srange[0],srange[1]);
        else {
            result = new EGS_VHPGeometry(phantom.c_str(),media.c_str());
        }
        if (!result->isOK()) {
            egsWarning("createGeometry(EGS_VHPGeometry): failed to construct "
                       "geometry");
            delete result;
            return 0;
        }
        result->setName(input);
        result->setMicros(input);
        result->setLabels(input);
        return result;
    }

}
