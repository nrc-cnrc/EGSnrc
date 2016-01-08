/*
###############################################################################
#
#  EGSnrc egs++ voxelized human phantom geometry headers
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


/*! \file egs_vhp_geometry.h
 *  \brief Voxelized Human Phantom (VHP) geometry: header
 *  \IK
 */


#ifndef EGS_ND_GEOMETRY_
#define EGS_ND_GEOMETRY_

#ifdef WIN32

    #ifdef BUILD_VHP_DLL
        #define EGS_VHP_EXPORT __declspec(dllexport)
    #else
        #define EGS_VHP_EXPORT __declspec(dllimport)
    #endif
    #define EGS_VHP_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_VHP_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_VHP_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_VHP_EXPORT
        #define EGS_VHP_LOCAL
    #endif

#endif


#include "egs_base_geometry.h"
#include "egs_functions.h"

#include <iostream>
#include <string>

using namespace std;

#ifndef SKIP_DOXYGEN
/*! \brief A local class needed for the VHP implementation

 \internwarning
*/
struct EGS_VHP_LOCAL VHP_RowInfo {
    unsigned short  nbin;
    unsigned short *pix;
    unsigned char  *org;
    VHP_RowInfo() : nbin(0) {};
    ~VHP_RowInfo() {
        if (nbin > 0) {
            delete [] pix;
            delete [] org;
        }
    };
    int getBin(int i) const {
        int ml=0, mu=nbin;
        while (mu - ml > 1) {
            int mav = (ml+mu)/2;
            if (i < pix[mav]) {
                mu = mav;
            }
            else {
                ml = mav;
            }
        }
        return mu-1;
    };
    int getOrgan(int pixel) const {
        return pixel < pix[0] || pixel >= pix[nbin] ? 0 : org[getBin(pixel)];
    };
    int getSize() const {
        int result = sizeof(nbin) + sizeof(unsigned short *) +
                     sizeof(unsigned char *);
        if (nbin > 0) result += nbin*sizeof(unsigned char) +
                                    (nbin+1)*sizeof(unsigned short);
        return result;
    };
    void readData(istream &in);
};
#endif

#ifndef SKIP_DOXYGEN
/*! \brief A local class needed for the VHP implementation

 \internwarning
*/
struct EGS_VHP_LOCAL VHP_SliceInfo {
    unsigned short  firstr;
    unsigned short  lastr;
    VHP_RowInfo    *rinfo;
    VHP_SliceInfo() : rinfo(0) {};
    ~VHP_SliceInfo() {
        if (rinfo) {
            delete [] rinfo;
        }
    };
    void getSliceDimensions(int &nx, int &ny) {
        ny = lastr+1;
        nx = 0;
        int nrow = lastr - firstr + 1;
        for (int j=0; j<nrow; ++j) {
            int ni = rinfo[j].pix[rinfo[j].nbin];
            if (ni > nx) {
                nx = ni;
            }
        }
    };
    int getOrgan(int row, int pixel) const {
        return rinfo && row >= firstr && row <= lastr ?
               rinfo[row-firstr].getOrgan(pixel) : 0;
    };
    int getSize() const {
        int result = sizeof(firstr) + sizeof(lastr) +
                     sizeof(VHP_RowInfo *);
        if (rinfo) {
            int nrow = lastr-firstr+1;
            for (int j=0; j<nrow; ++j) {
                result += rinfo[j].getSize();
            }
        }
        return result;
    };
    void readData(istream &in);
};
#endif

#ifndef SKIP_DOXYGEN
/*! \brief A local class needed for the VHP implementation

 \internwarning
*/
class EGS_VHP_LOCAL VHP_OrganData {
public:
    VHP_OrganData(const char *fname, int slice_min=0, int slice_max=1000000);
    ~VHP_OrganData();

    int         nSlice() const {
        return nslice;
    };
    int         getOrgan(int islice, int i, int j) const {
        return islice >= 0 && islice < nslice ?
               slices[islice].getOrgan(i,j) : 0;
    };
    const VHP_SliceInfo &getVHP_SliceInfo(int j) const {
        return slices[j];
    };
    const VHP_SliceInfo &operator[](int j) const {
        return slices[j];
    };
    int getSize() const {
        int result = sizeof(nslice) + sizeof(VHP_SliceInfo *);
        if (nslice > 0) {
            for (int j=0; j<nslice; ++j) {
                result += slices[j].getSize();
            }
        }
        return result;
    };
    void getPhantomDimensions(int &Nx, int &Ny, int &Nz) {
        Nx = nx;
        Ny = ny;
        Nz = nslice;
    };
    void getVoxelSizes(EGS_Float &Dx, EGS_Float &Dy, EGS_Float &Dz) {
        Dx = dx;
        Dy = dy;
        Dz = dz;
    };
    int getOrgan(int ireg) const {
        int k = ireg/nxy;
        if (k < 0 || k >= nslice) {
            return 0;
        }
        int jj = ireg - k*nx*ny;
        int j = jj/nx;
        int i = jj - j*nx;
        return getOrgan(k,j,i);
    };

    bool isOK() const {
        return ok;
    };

protected:
    double          dx, dy, dz;
    VHP_SliceInfo  *slices;
    unsigned short  nslice;
    int             nx, ny, nxy;
    bool            ok;
};
#endif

#ifndef SKIP_DOXYGEN
/*! \brief A local class needed for the VHP implementation

 \internwarning
*/
struct EGS_VHP_LOCAL EGS_VoxelInfo {
    unsigned short ix, iy, iz;
};
#endif

#ifndef SKIP_DOXYGEN
/*! \brief A local class needed for the VHP implementation

 \internwarning
*/
class EGS_VHP_LOCAL EGS_VoxelGeometry {

public:

    EGS_Float        dx, dxi, xmin, xmax;
    EGS_Float        dy, dyi, ymin, ymax;
    EGS_Float        dz, dzi, zmin, zmax;
    int              nx, ny, nz, nxy;
    int              ix, iy, iz;

    static EGS_VoxelInfo *v;
    static int           nv;

    EGS_VoxelGeometry(int Nx, int Ny, int Nz,
                      EGS_Float Dx, EGS_Float Dy, EGS_Float Dz) :
        dx(Dx), dxi(1/Dx), xmin(0), xmax(Dx*Nx),
        dy(Dy), dyi(1/Dy), ymin(0), ymax(Dy*Ny),
        dz(Dz), dzi(1/Dz), zmin(0), zmax(Dz*Nz),
        nx(Nx), ny(Ny), nz(Nz), nxy(nx*ny) {};

    ~EGS_VoxelGeometry() {
        if (v) {
            delete [] v;
            v = 0;
            nv = 0;
        }
    };

    bool isInside(const EGS_Vector &x) {
        return (x.x >= xmin && x.x <= xmax &&
                x.y >= ymin && x.y <= ymax &&
                x.z >= zmin && x.z <= zmax) ? true : false;
    };

    void setIndeces(const EGS_Vector &x) {
        ix = (int)(x.x*dxi);
        if (ix >= nx) {
            ix = nx-1;
        }
        iy = (int)(x.y*dyi);
        if (iy >= ny) {
            iy = ny-1;
        }
        iz = (int)(x.z*dzi);
        if (iz >= nz) {
            iz = nz-1;
        }
    };

    int isWhere(const EGS_Vector &x) {
        if (!isInside(x)) {
            return -1;
        }
        setIndeces(x);
        return ix + iy*nx + iz*nxy;
    };

    int isWhereFast(const EGS_Vector &x) {
        setIndeces(x);
        return ix + iy*nx + iz*nxy;
    };

    int computeIntersections(int ireg, int n, const EGS_Vector &X,
                             const EGS_Vector &u, EGS_GeometryIntersections *isections) {
        if (n < 1) {
            return -1;
        }
        if (nv < n) {
            if (nv > 0) {
                delete [] v;
            }
            v = new EGS_VoxelInfo [n];
            nv = n;
        }
        int ifirst = 0;
        EGS_Float t, ttot = 0;
        EGS_Vector x(X);
        int imed;
        if (ireg < 0) {
            t = 1e30;
            ireg = howfar(ireg,x,u,t);
            if (ireg < 0) {
                return 0;
            }
            isections[0].t = t;
            isections[0].rhof = 1;
            isections[0].ireg = -1;
            isections[0].imed = -1;
            ttot = t;
            ++ifirst;
            x += u*t;
        }
        int iz = ireg/nxy;
        int ir = ireg - iz*nxy;
        int iy = ir/nx;
        int ix = ir - iy*nx;
        EGS_Float uxi, uyi, uzi, nextx, nexty, nextz, sx, sy, sz;
        int dirx, icx, diry, icy, dirz, icz;
        if (u.x > 0)      {
            uxi = 1/u.x;
            dirx =  1;
            nextx = (dx*(ix+1)-x.x)*uxi;
            sx = uxi*dx;
        }
        else if (u.x < 0) {
            uxi = 1/u.x;
            dirx = -1;
            nextx = (dx*ix-x.x)*uxi;
            sx = -uxi*dx;
        }
        else {
            dirx =  1;
            nextx = 1e33;
            sx = 1e33;
        }
        if (u.y > 0)      {
            uyi = 1/u.y;
            diry =  1;
            nexty = (dy*(iy+1)-x.y)*uyi;
            sy = uyi*dy;
        }
        else if (u.y < 0) {
            uyi = 1/u.y;
            diry = -1;
            nexty = (dy*iy-x.y)*uyi;
            sy = -uyi*dy;
        }
        else {
            diry =  1;
            nexty = 1e33;
            sy = 1e33;
        }
        if (u.z > 0)      {
            uzi = 1/u.z;
            dirz =  1;
            nextz = (dz*(iz+1)-x.z)*uzi;
            sz = uzi*dz;
        }
        else if (u.z < 0) {
            uzi = 1/u.z;
            dirz = -1;
            nextz = (dz*iz-x.z)*uzi;
            sz = -uzi*dz;
        }
        else  {
            dirz =  1;
            nextz = 1e33;
            sz = 1e33;
        }
        for (int j=ifirst; j<n; j++) {
            isections[j].ireg = ireg;
            v[j].ix = ix;
            v[j].iy = iy;
            v[j].iz = iz;
            int inew;
            if (nextx < nexty && nextx < nextz) {
                t = nextx;
                ix += dirx;
                if (ix < 0 || ix >= nx) {
                    inew = -1;
                }
                else {
                    inew = ireg + dirx;
                    nextx += sx;
                }
            }
            else if (nexty < nextz) {
                t = nexty;
                iy += diry;
                if (iy < 0 || iy >= ny) {
                    inew = -1;
                }
                else {
                    inew = ireg + nx*diry;
                    nexty += sy;
                }
            }
            else {
                t = nextz;
                iz += dirz;
                if (iz < 0 || iz >= nz) {
                    inew = -1;
                }
                else {
                    inew = ireg + nxy*dirz;
                    nextz += sz;
                }
            }
            isections[j].t = t;
            if (inew < 0) {
                return j+1;
            }
        }
        return ireg >= 0 ? -1 : n;
    };

    EGS_Float howfarToOutside(int ireg, const EGS_Vector &x,
                              const EGS_Vector &u) {
        if (ireg < 0) {
            return 0;
        }
        EGS_Float tx = u.x > 0 ? (xmax-x.x)/u.x :
                       u.x < 0 ? (xmin-x.x)/u.x : 1e35;
        EGS_Float ty = u.y > 0 ? (ymax-x.y)/u.y :
                       u.y < 0 ? (ymin-x.y)/u.y : 1e35;
        EGS_Float tz = u.z > 0 ? (zmax-x.z)/u.z :
                       u.z < 0 ? (zmin-x.z)/u.z : 1e35;
        return tx < ty && tx < tz ? tx : ty < tz ? ty : tz;
    };

    void setIndeces(int ireg) {
        iz = ireg/nxy;
        int ir = ireg - iz*nxy;
        iy = ir/nx;
        ix = ir - iy*nx;
    };

    //int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
    //        EGS_Float &t, EGS_Vector *normal=0) {
    //    setIndeces(ireg);
    //    return howfar(ireg,ix,iy,iz,x,u,t,normal);
    //};
    int howfarIn(int ireg, int Ix, int Iy, int Iz, const EGS_Vector &x,
                 const EGS_Vector &u, EGS_Float &t, EGS_Vector *normal=0) {
        ix = Ix;
        iy = Iy;
        iz = Iz;
        return howfarIn(ireg,x,u,t,normal);
    };

    int howfarIn(int ireg, const EGS_Vector &x, const EGS_Vector &u,
                 EGS_Float &t, EGS_Vector *normal=0) {
        int ixs = ix, iys = iy, izs = iz;
        int inew = ireg;
        if (u.x > 0) {
            EGS_Float d = (dx*(ix+1)-x.x)/u.x;
            if (d <= t) {
                t = d;
                if ((++ix) < nx) {
                    inew = ireg + 1;
                }
                else {
                    inew = -1;
                }
                if (normal) {
                    *normal = EGS_Vector(-1,0,0);
                }
            }
        }
        else if (u.x < 0) {
            EGS_Float d = (dx*ix-x.x)/u.x;
            if (d <= t) {
                t = d;
                if ((--ix) >= 0) {
                    inew = ireg - 1;
                }
                else {
                    inew = -1;
                }
                if (normal) {
                    *normal = EGS_Vector(1,0,0);
                }
            }
        }
        if (u.y > 0) {
            EGS_Float d = (dy*(iy+1)-x.y)/u.y;
            if (d <= t) {
                ix = ixs;
                t = d;
                if ((++iy) < ny) {
                    inew = ireg + nx;
                }
                else {
                    inew = -1;
                }
                if (normal) {
                    *normal = EGS_Vector(0,-1,0);
                }
            }
        }
        else if (u.y < 0) {
            EGS_Float d = (dy*iy-x.y)/u.y;
            if (d <= t) {
                ix = ixs;
                t = d;
                if ((--iy) >= 0) {
                    inew = ireg - nx;
                }
                else {
                    inew = -1;
                }
                if (normal) {
                    *normal = EGS_Vector(0,1,0);
                }
            }
        }
        if (u.z > 0) {
            EGS_Float d = (dz*(iz+1)-x.z)/u.z;
            if (d <= t) {
                ix = ixs;
                iy = iys;
                t = d;
                if ((++iz) < nz) {
                    inew = ireg+nxy;
                }
                else {
                    inew = -1;
                }
                if (normal) {
                    *normal = EGS_Vector(0,0,-1);
                }
            }
        }
        else if (u.z < 0) {
            EGS_Float d = (dz*iz-x.z)/u.z;
            if (d <= t) {
                ix = ixs;
                iy = iys;
                t = d;
                if ((--iz) >= 0) {
                    inew = ireg-nxy;
                }
                else {
                    inew = -1;
                }
                if (normal) {
                    *normal = EGS_Vector(0,0,1);
                }
            }
        }
        return inew;
    };

    int howfarOut(const EGS_Vector &x, const EGS_Vector &u,
                  EGS_Float &t, EGS_Vector *normal=0) {
        EGS_Float tlong = 2*t, d;
        int inew = -1;
        if (x.x <= xmin && u.x > 0) {
            ix = 0;
            d = (xmin-x.x)/u.x;
        }
        else if (x.x >= xmax && u.x < 0) {
            ix = nx-1;
            d = (xmax-x.x)/u.x;
        }
        else {
            d = tlong;
        }
        if (d <= t) {
            EGS_Float yy = x.y + u.y*d, zz = x.z + u.z*d;
            if (yy >= ymin && yy <= ymax && zz >= zmin && zz <= zmax) {
                t = d;
                iy = (int)(yy*dyi);
                if (iy > ny-1) {
                    iy = ny-1;
                }
                iz = (int)(zz*dzi);
                if (iz > nz-1) {
                    iz = nz-1;
                }
                if (normal) *normal = ix == 0 ? EGS_Vector(-1,0,0) :
                                          EGS_Vector(1,0,0);
                return ix + iy*nx + iz*nxy;
            }
        }
        if (x.y <= ymin && u.y > 0) {
            iy = 0;
            d = (ymin-x.y)/u.y;
        }
        else if (x.y >= ymax && u.y < 0) {
            iy = ny-1;
            d = (ymax-x.y)/u.y;
        }
        else {
            d = tlong;
        }
        if (d <= t) {
            EGS_Float xx = x.x + u.x*d, zz = x.z + u.z*d;
            if (xx >= xmin && xx <= xmax && zz >= zmin && zz <= zmax) {
                t = d;
                ix = (int)(xx*dxi);
                if (ix > nx-1) {
                    ix = nx-1;
                }
                iz = (int)(zz*dzi);
                if (iz > nz-1) {
                    iz = nz-1;
                }
                if (normal) *normal = iy == 0 ? EGS_Vector(0,-1,0) :
                                          EGS_Vector(0,1,0);
                return ix + iy*nx + iz*nxy;
            }
        }
        if (x.z <= zmin && u.z > 0) {
            iz = 0;
            d = (zmin-x.z)/u.z;
        }
        else if (x.z >= zmax && u.z < 0) {
            iz = nz-1;
            d = (zmax-x.z)/u.z;
        }
        else {
            d = tlong;
        }
        if (d <= t) {
            EGS_Float xx = x.x + u.x*d, yy = x.y + u.y*d;
            if (xx >= xmin && xx <= xmax && yy >= ymin && yy <= ymax) {
                t = d;
                ix = (int)(xx*dxi);
                if (ix > nx-1) {
                    ix = nx-1;
                }
                iy = (int)(yy*dyi);
                if (iy > ny-1) {
                    iy = ny-1;
                }
                if (normal) *normal = iz == 0 ? EGS_Vector(0,0,-1) :
                                          EGS_Vector(0,0,1);
                return ix + iy*nx + iz*nxy;
            }
        }
        return -1;
    };

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, EGS_Vector *normal=0) {
        if (ireg >= 0) {
            setIndeces(ireg);
            return howfarIn(ireg,x,u,t,normal);
        }
        return howfarOut(x,u,t,normal);
    };

    EGS_Float hownearIn(int Ix, int Iy, int Iz, const EGS_Vector &x) {
        ix = Ix;
        iy = Iy;
        iz = Iz;
        return hownearIn(x);
    };

    EGS_Float hownearIn(const EGS_Vector &x) {
        EGS_Float t1,t2,tx,ty,tz;
        t1 = x.x-dx*ix;
        t2 = dx - t1;
        tx = t1 < t2 ? t1 : t2;
        t1 = x.y-dy*iy;
        t2 = dy - t1;
        ty = t1 < t2 ? t1 : t2;
        t1 = x.z-dz*iz;
        t2 = dz - t1;
        tz = t1 < t2 ? t1 : t2;
        return tx < ty && tx < tz ? tx : ty < tz ? ty : tz;
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        if (ireg >= 0) {
            setIndeces(ireg);
            return hownearIn(x);
        }
        int nc = 0;
        EGS_Float s1=0, s2=0;
        if (x.x < xmin || x.x > xmax) {
            EGS_Float t = x.x < xmin ? xmin-x.x : x.x-xmax;
            nc++;
            s1 += t;
            s2 += t*t;
        }
        if (x.y < ymin || x.y > ymax) {
            EGS_Float t = x.y < ymin ? ymin-x.y : x.y-ymax;
            nc++;
            s1 += t;
            s2 += t*t;
        }
        if (x.z < zmin || x.z > zmax) {
            EGS_Float t = x.z < zmin ? zmin-x.z : x.z-zmax;
            nc++;
            s1 += t;
            s2 += t*t;
        }
        return nc == 1 ? s1 : sqrt(s2);
    };
};
#endif

#ifndef SKIP_DOXYGEN
/*! \brief A local class needed for the VHP implementation

 \internwarning
*/
struct VHPBox {
    EGS_Float xmin, xmax;
    EGS_Float ymin, ymax;
    EGS_Float zmin, zmax;
    bool isInside(const EGS_Vector &x) {
        return
            (x.x>=xmin&&x.x<=xmax&&x.y>=ymin&&ymax<=ymin&&x.z>=zmin&&x.z<=zmax);
    };
    bool willEnter(const EGS_Vector &x, const EGS_Vector &u, EGS_Float &t) {
        EGS_Float tlong = 2*t, d;
        if (x.x <= xmin && u.x > 0) {
            d = (xmin-x.x)/u.x;
        }
        else if (x.x >= xmax && u.x < 0) {
            d = (xmax-x.x)/u.x;
        }
        else {
            d = tlong;
        }
        if (d <= t) {
            EGS_Float yy = x.y + u.y*d, zz = x.z + u.z*d;
            if (yy >= ymin && yy <= ymax && zz >= zmin && zz <= zmax) {
                t = d;
                return true;
            }
        }
        if (x.y <= ymin && u.y > 0) {
            d = (ymin-x.y)/u.y;
        }
        else if (x.y >= ymax && u.y < 0) {
            d = (ymax-x.y)/u.y;
        }
        else {
            d = tlong;
        }
        if (d <= t) {
            EGS_Float xx = x.x + u.x*d, zz = x.z + u.z*d;
            if (xx >= xmin && xx <= xmax && zz >= zmin && zz <= zmax) {
                t = d;
                return true;
            }
        }
        if (x.z <= zmin && u.z > 0) {
            d = (zmin-x.z)/u.z;
        }
        else if (x.z >= zmax && u.z < 0) {
            d = (zmax-x.z)/u.z;
        }
        else {
            d = tlong;
        }
        if (d <= t) {
            EGS_Float xx = x.x + u.x*d, yy = x.y + u.y*d;
            if (xx >= xmin && xx <= xmax && yy >= ymin && yy <= ymax) {
                t = d;
                return true;
            }
        }
        return false;
    };
    EGS_Float howfarToOut(const EGS_Vector &x, const EGS_Vector &u) {
        EGS_Float t = 1e35, t1;
        if (u.x > 0) {
            t1 = (xmax-x.x)/u.x;
            if (t1 < t) {
                t = t1;
            }
        }
        else if (u.x < 0) {
            t1 = (xmin-x.x)/u.x;
            if (t1 < t) {
                t = t1;
            }
        }
        if (u.y > 0) {
            t1 = (ymax-x.y)/u.y;
            if (t1 < t) {
                t = t1;
            }
        }
        else if (u.y < 0) {
            t1 = (ymin-x.y)/u.y;
            if (t1 < t) {
                t = t1;
            }
        }
        if (u.z > 0) {
            t1 = (zmax-x.z)/u.z;
            if (t1 < t) {
                t = t1;
            }
        }
        else if (u.z < 0) {
            t1 = (zmin-x.z)/u.z;
            if (t1 < t) {
                t = t1;
            }
        }
        return t;
    };
};
#endif

#ifndef SKIP_DOXYGEN
/*! \brief A local class needed for the VHP implementation

 \internwarning
*/
class EGS_VHP_LOCAL EGS_MicroMatrixCluster {

public:

    EGS_MicroMatrixCluster(EGS_Float Dx, EGS_Float Dy, EGS_Float Dz,
                           EGS_Float bsc_thickness, int tb_med, int bm_med,
                           const char *micro_file);

    ~EGS_MicroMatrixCluster();


    int howfar(int imic, int ireg, int &lax, int &lay, int &laz, int &newmic,
               const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed=0, EGS_Vector *normal=0) {
        int inew = vg->howfar(ireg,x,u,t,normal);
        newmic = imic;
        if (inew < 0) {
            int iz = imic/(mx*my);
            int itmp = imic - iz*mx*my;
            int iy = itmp/mx;
            int ix = itmp - mx*iy;
            if (vg->ix < 0)       {
                --lax;
                if ((--ix) < 0) {
                    ix = mx-1;
                }
            }
            else if (vg->ix >= vg->nx) {
                ++lax;
                if ((++ix) >= mx) {
                    ix = 0;
                }
            }
            else if (vg->iy < 0)       {
                --lay;
                if ((--iy) < 0) {
                    iy = my-1;
                }
            }
            else if (vg->iy >= vg->ny) {
                ++lay;
                if ((++iy) >= my) {
                    iy = 0;
                }
            }
            else if (vg->iz < 0)       {
                --laz;
                if ((--iz) < 0) {
                    iz = mz-1;
                }
            }
            else if (vg->iz >= vg->nz) {
                ++laz;
                if ((++iz) >= mz) {
                    iz = 0;
                }
            }
            newmic = ix + iy*mx + iz*mx*my;
        }
        if (newmed) {
            *newmed = micros[newmic][inew] ? med_bm : med_tb;
        }
        return inew;
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        return vg->hownear(ireg,x);
    };

    int medium(int imic, int ireg) const {
        return micros[imic][ireg] ? med_bm : med_tb;
    };

    int medium(int ireg) const {
        int imic = ireg/nreg;
        ireg -= imic*nreg;
        return medium(imic,ireg);
    };

    int isWhere(int &imic, const EGS_Vector &x) {
        int ix = (int)(x.x*dxi), iy = (int)(x.y*dyi), iz = (int)(x.z*dzi);
        EGS_Vector x1(x - EGS_Vector(dx*ix, dy*iy, dz*iz));
        ix = ix%mx;
        iy = iy%my;
        iz = iz%mz;
        imic = ix + iy*mx + iz*mz;
        return vg->isWhere(x1);
    };

    void getIndeces(int ireg, int &imic, int &ix, int &iy, int &iz) {
        imic = ireg/nreg;
        ireg -= imic*nreg;
        iz = ireg/nxy;
        iy = (ireg - iz*nxy)/nx;
        ix = ireg - iz*nxy - iy*nx;
    };

    int isWhere(int jx, int jy, int jz, const EGS_Vector &x1, int *newmed=0) {
        int ix = jx%mx, iy = jy%my, iz = jz%mz;
        int imic = ix + iy*mx + iz*mz;
        int iloc = vg->isWhereFast(x1);
        if (newmed) {
            *newmed = medium(imic,iloc);
        }
        return imic*nreg + iloc;
    };

    bool isValid() const {
        return (vg && micros);
    };

    void getStepFractions(int imic, int ireg,
                          const EGS_Vector &xi, const EGS_Vector &xf,
                          EGS_Float &bsc_step, EGS_Float &bm_step) {
        bsc_step = 0;
        bm_step = 0;
        int mict = micros[imic][ireg];
        if (mict) {
            // a BM voxel => check step fractions
            if (mict == 1) {
                bsc_step = 1;    // all BSC
            }
            else if (mict == 2) {
                bm_step = 1;    // all BM
            }
            else {
                mict -= 2;
                // get positions in bm_boxes co-ordinate system
                int jz = ireg/nxy;
                int jy = (ireg-jz*nxy)/nx;
                int jx = ireg-jz*nxy-jy*nx;
                EGS_Vector xcorner(vg->dx*jx,vg->dy*jy,vg->dz*jz);
                EGS_Vector Xi(xi-xcorner), Xf(xf-xcorner);
                bool i_inside = bm_boxes[mict].isInside(Xi),
                     f_inside = bm_boxes[mict].isInside(Xf);
                // if both positions are inside the BM box, the entire step
                // is in the BM box
                if (i_inside && f_inside) {
                    bm_step = 1;
                }
                else {
                    // we have a fraction in BM and remainder in BSC
                    EGS_Vector u(Xf-Xi);
                    EGS_Float t = u.length();
                    EGS_Float ti = 1/t;
                    u *= ti;
                    if (!i_inside) {
                        // initial position outside of BM box => find entry
                        EGS_Float tt = t;
                        if (!bm_boxes[mict].willEnter(Xi,u,tt)) {
                            // line Xi->Xf does not intersect BM box
                            bsc_step = 1;
                            return;
                        }
                        // move to entry point
                        bsc_step = tt;
                        Xi += u*tt;
                    }
                    if (f_inside) {
                        // final position inside BM box. This implies initial
                        // position was outside. Because we already computed
                        // distance to BM box entry, the remainder goes to BM
                        bsc_step *= ti;
                        bm_step = 1 - bsc_step;
                        return;
                    }
                    // final position outside. this means that either the
                    // initial position was inside or that we moved to the
                    // entry point => BM fraction is distance to outside
                    bm_step = ti*bm_boxes[mict].howfarToOut(Xi,u);
                    bsc_step = 1 - bm_step;
                }
            }
        }
    };

    void getVolumes(int imic, EGS_Float &tb, EGS_Float &bm, EGS_Float &bsc) {
        tb = v_tb[imic];
        bm = v_bm[imic];
        bsc = v_bsc[imic];
    };


    // ********************************************************************

    int       nx, ny, nz, nxy, nreg;
    int       mx, my, mz;   //!< number of micro matrices in the cluster
    int       mxy, nmic;    //!< mx*my and mx*my*mz
    EGS_Float dx, dy, dz;   //!< macro voxel size
    EGS_Float dxi, dyi, dzi;//!< inverse macro voxel size
    EGS_Float t_bsc;        //!< thickness of the bsc layer

    int       med_tb;       //!< medium index for trabecular bone
    int       med_bm;       //!< medium index for bone marrow

    /*! Micro data. micro[j] is a pointer to the data of the j'th micro
        matrix in the cluster. micro[j][ix+iy*nx+iz*nxy] is the type of
        micro voxel:
          0 means pure trabecular bone
          1 means pure bone marrow
          2 means a marrow micro-voxel with one of more BSC layers
    */
    unsigned char     **micros;
    EGS_VoxelGeometry  *vg;   //!< The object used for geometry calculations
    EGS_Float          *v_tb; //!< Array with TB volumes for each micro-matrix
    EGS_Float          *v_bm; //!< Array with BM volumes for each micro-matrix
    EGS_Float          *v_bsc;//!< Array with BSC volumes for each micro-matrix

    static VHPBox bm_boxes[64];
    static bool   bm_boxes_initialized;

};
#endif

class EGS_VHP_EXPORT EGS_TestMicro : public EGS_BaseGeometry {

public:

    EGS_TestMicro(EGS_Float Dx, EGS_Float Dy, EGS_Float Dz, EGS_Float bsc_t,
                  int tb_med, int bm_med,const char *micro_file, const string &Name="");

    ~EGS_TestMicro();

    int  inside(const EGS_Vector &x)   {
        return isWhere(x);
    };
    bool isInside(const EGS_Vector &x) {
        return vg->isInside(x);
    }
    int isWhere(const EGS_Vector &x) {
        int ireg = vg->isWhere(x);
        if (ireg < 0) {
            return ireg;
        }
        EGS_Vector x1(x-EGS_Vector(vg->dx*vg->ix,vg->dy*vg->iy,vg->dz*vg->iz));
        int ix = vg->ix%micro->mx,
            iy = vg->iy%micro->my,
            iz = vg->iz%micro->mz;
        int imic = ix + iy*micro->mx + iz*micro->mxy;
        int iloc = micro->vg->isWhere(x1);
        return imic*nr + iloc;
    };

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed=0, EGS_Vector *normal=0) {
        if (ireg >= 0) {
            int voxel = ireg/nr;
            int ilocal = ireg - voxel*nr;
            vg->setIndeces(voxel);
            EGS_Vector x1(x-EGS_Vector(vg->dx*vg->ix,vg->dy*vg->iy,vg->dz*vg->iz));
            int inew = micro->vg->howfar(ilocal,x1,u,t,normal);
            if (inew == ilocal) {
                return ireg;
            }
            if (inew < 0) {
                if (micro->vg->ix < 0) {
                    if ((--vg->ix) < 0) {
                        return -1;
                    }
                    micro->vg->ix = micro->vg->nx-1;
                }
                else if (micro->vg->ix >= micro->vg->nx) {
                    if ((++vg->ix) >= vg->nx) {
                        return -1;
                    }
                    micro->vg->ix = 0;
                }
                else if (micro->vg->iy < 0) {
                    if ((--vg->iy) < 0) {
                        return -1;
                    }
                    micro->vg->iy = micro->vg->ny-1;
                }
                else if (micro->vg->iy >= micro->vg->ny) {
                    if ((++vg->iy) >= vg->ny) {
                        return -1;
                    }
                    micro->vg->iy = 0;
                }
                else if (micro->vg->iz < 0) {
                    if ((--vg->iz) < 0) {
                        return -1;
                    }
                    micro->vg->iz = micro->vg->nz-1;
                }
                else if (micro->vg->iz >= micro->vg->nz) {
                    if ((++vg->iz) >= vg->nz) {
                        return -1;
                    }
                    micro->vg->iz = 0;
                }
                inew = micro->vg->ix + micro->vg->iy*micro->vg->nx +
                       micro->vg->iz*micro->vg->nxy;
            }
            voxel = vg->ix + vg->iy*vg->nx + vg->iz*vg->nxy;
            if (newmed) {
                *newmed = micro->medium(voxel,inew);
            }
            return voxel*nr + inew;
        }
        int voxel = vg->howfar(ireg,x,u,t,normal);
        if (voxel < 0) {
            return voxel;
        }
        EGS_Vector x1(x + u*t);
        x1 -= EGS_Vector(vg->dx*vg->ix,vg->dy*vg->iy,vg->dz*vg->iz);
        int ilocal = micro->vg->isWhereFast(x1);
        if (ilocal < 0) {
            egsFatal("x=(%g,%g,%g) x+u*t=(%g,%g,%g) x1=(%g,%g,%g)\n",x.x,x.y,x.z,(x+u*t).x,(x+u*t).y,(x+u*t).z,x1.x,x1.y,x1.z);
        }
        if (newmed) {
            *newmed = micro->medium(voxel,ilocal);
        }
        return voxel*nr + ilocal;
    };

    EGS_Float howfarToOutside(int ireg, const EGS_Vector &x,
                              const EGS_Vector &u) {
        return vg->howfarToOutside(ireg,x,u);
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        if (ireg < 0) {
            return vg->hownear(ireg,x);
        }
        int voxel = ireg/nr;
        int ilocal = ireg - voxel*nr;
        vg->setIndeces(voxel);
        EGS_Vector x1(x-EGS_Vector(vg->dx*vg->ix,vg->dy*vg->iy,vg->dz*vg->iz));
        return micro->vg->hownear(ilocal,x1);
    };

    int medium(int ireg) const {
        int voxel = ireg/nr;
        int ilocal = ireg - voxel*nr;
        return micro->medium(voxel,ilocal);
    };

    int getMaxStep() const {
        return micro->mx*micro->nx + micro->my*micro->ny +
               micro->mz*micro->nz + 1;
    };

    const string &getType() const {
        return type;
    };

protected:

    EGS_VoxelGeometry      *vg;
    EGS_MicroMatrixCluster *micro;
    int                    nr;  //!< number of voxels in a micro-matrix

    void setMedia(EGS_Input *inp, int nmed, const int *med_ind);

    static string type;
};

/*! \brief A Voxelized Human Phantom (VHP) geometry.
 *
 * \ingroup Geometry
 * \ingroup CompositeG
 *
 * This geometry type can be used to model a voxelized representation of
 * a phantom. In principle a VHP geometry is the same as a XYZ geometry.
 * The main reason for providing a separate implementation is the huge
 * number of voxels needed to represent a whole body human phantom using
 * 1-2 mm resolution, which necessitates a more careful memory use.
 * In addition, this implementation provides the possibility to include
 * a micro-voxel representation of the human spongiosa obtained from
 * micro-CT images. A VHP geometry is defined as follows:
 * \verbatim
 * library      = egs_vhp_geometry
 * name         = some_name
 * phantom data = phantom_data_file
 * media data   = media_data_file
 * slice range  = min_slice max_slice
 * \endverbatim
 * In the above, the <code>slice range</code> input, which is optional,
 * can be used to select a given slice range instead of using the entire
 * phantom. The file <code>phantom_data_file</code> is a binary file
 * containing the definition of the phantom. The format of this file is as
 * follows:
 *   - one byte indicating the endianess of the machine where the data was
 *     created (0=big endian, 1=little endian).
 *   - three 64 bit floating point values defining voxel size in x-, y- and
 *     z-direction
 *   - number of slices <code>nslice</code> (16 bit unsigned integer value).
 *     Note that increasing slice number corresponds to increasing z coordinate.
 *   - followed by the data for the <code>nslice</code> slices.
 * The data for a slice consists of
 *   - first and last row (16 bit usigned integers)
 *   - followed by the last-first+1 data for a row
 * The data for a row consists of
 *   - number <code>nreg</code> of regions (16 bit usigned integer)
 *   - followed by <code>nreg+1</code> pixel indeces \f$p_j\f$
 *     (16 bit usigned integers)
 *   - followed by <code>nreg</code> organ indeces \f$O_j\f$
 *     (8 bit unsigned integers)
 * so that all voxels between \f$p_j\f$ and \f$p_{j+1}\f$ are set to
 * organ \f$O_j\f$.
 * This data structure, also used at run time,
 * provides a significant compression compared to
 * simply using an array of organ indeces (e.g., one needs about 12 MB to
 * represent Richard Kramer's FAX phantom compared to about 150 MB that would
 * be needed for the 1361 x 474 x 224 voxels of this phantom). Its disadvantage
 * is the slighly slower (\f$\sim\f$ 10-15%) access to the organ index of a
 * voxel.
 *
 * The file <code>media_data_file</code> is an ASCII file that defines
 * the organ names and the materials to be used for each organ. Its format
 * is as follows:
 *   - number \f$N\f$ of different organs
 *   - followed by \f$N\f$ lines consisting of organ index, medium index
 *     and organ name.
 *   - followed by number of media <code>nmed</code>
 *   - followed by <code>nmed</code> lines with a single medium name per line
 *     (media names are the actual medium names in the PEGS file to be used
 *     for the simulation).
 *
 * To add a micro-voxel representation of spongiosa, one includes in the
 * geometry definition the following inputs:
 * \verbatim
 * BSC thickness = thickness in cm
 * TB medium = medium name
 * BM medium = medium name
 * micro matrix = micro_data_file1 organ1 organ2 ...
 * micro matrix = micro_data_file2 organ1 organ2 ...
 * ...
 * \endverbatim
 * The <code>TB medium</code> and <code>BM medium</code> inputs define the
 * PEGS medium names of trabecular bone and bone marrow. The
 * <code>BSC thickness</code> sets the thickness of the bone surface cells
 * (BSC) layer. Note that in the current implementation the BSC thickness
 * can not exceed the micro-voxel size.
 * The binary files <code>micro_data_file_i</code> define micro matrix
 * clusters to replace macro voxels that have organ indeces
 * <code>organ1 organ2 ...</code>. A micro matrix cluster consists of
 * \f$M_x \times M_y \times M_z\f$ micro matrices of dimension
 * \f$N_x \times N_y \times N_z\f$. This cluster is repeated periodically in
 * spongiosa organs set to use this cluster. Note that the micro-matrix
 * voxel size is automatically determned from the macro voxel size and the
 * number of micro-voxels in a micro matrix. The format of the micro
 * matrix file is as follows:
 *   - \f$M_x, M_y, M_z\f$ (8 bit unsigned integers)
 *   - followed by \f$N_x, N_y, N_z\f$ (8 bit unsigned integers)
 *   - followed by \f$M_x \times M_y \times M_z \times N_x \times N_y \times N_z\f$
 *     8 bit integers defining the micro matrix medium (0 corresponds to
 *     trabecular bone, 1 to bone marrow).
 * Note that bone marrow voxels neighboring trabecular bone are automatically
 * marked to contain a BSC sub-volume.
 */

class EGS_VHP_EXPORT EGS_VHPGeometry : public EGS_BaseGeometry {

public:

    EGS_VHPGeometry(const char *phantom_file, const char *media_file,
                    int slice_min=0, int slice_max=1000000, const string &Name="");

    ~EGS_VHPGeometry();

    bool isInside(const EGS_Vector &x) {
        return vg->isInside(x);
    };

    int isWhere(const EGS_Vector &x) {
        int ireg = vg->isWhere(x);
        if (!nmicro) {
            return ireg;
        }
        int iorg = organs->getOrgan(vg->iz,vg->iy,vg->ix);
        int mict = organ_micro[iorg];
        if (mict < 0) {
            return ireg;    // i.e., no micros in this macro voxel
        }
        EGS_Vector x1(x - EGS_Vector(vg->dx*vg->ix,vg->dy*vg->iy,vg->dz*vg->iz));
        int micr = micros[mict]->isWhere(vg->ix,vg->iy,vg->iz,x1);
        return nmax*mict + micr + nmacro;
    };

    int inside(const EGS_Vector &x) {
        return isWhere(x);
    };

    int medium(int ireg) const {
        if (ireg < nmacro) {
            return organ_media[organs->getOrgan(ireg)];
        }
        ireg -= nmacro;
        int mict = ireg/nmax;
        int iloc = ireg - mict*nmax;
        return micros[mict]->medium(iloc);
    };

    int computeIntersections(int ireg, int n, const EGS_Vector &X,
                             const EGS_Vector &u, EGS_GeometryIntersections *isections) {
        // fix the following
        if (nmicro > 0) return EGS_BaseGeometry::computeIntersections(ireg,
                                   n,X,u,isections);
        if (n < 1) {
            return -1;
        }
        int result = vg->computeIntersections(ireg,n,X,u,isections);
        if (!result) {
            return result;    // no intersections
        }
        int nloop = result > 0 ? result : n;
        int first = isections[0].ireg >= 0 ? 0 : 1;
        EGS_VoxelInfo *v = vg->v;
        for (int j=first; j<nloop; ++j) isections[j].imed =
                organ_media[organs->getOrgan(v[j].iz,v[j].iy,v[j].ix)];
        return result;
    };

    EGS_Float howfarToOutside(int ireg, const EGS_Vector &x,
                              const EGS_Vector &u) {
        return vg->howfarToOutside(ireg,x,u);
    };

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed=0, EGS_Vector *normal=0) {
        if (nmicro < 1) {  // no micro matrices
            int inew = vg->howfar(ireg,x,u,t,normal);
            if (newmed && inew >= 0 && inew != ireg) {
                *newmed = organ_media[organs->getOrgan(vg->iz,vg->iy,vg->ix)];
            }
            return inew;
        }
        if (ireg >= 0 && ireg < nmacro) {
            // in macro matrix
            int inew = vg->howfar(ireg,x,u,t,normal);
            if (t < 0) {
                if (t < -1e-5) {
                    egsWarning("negative step %g in macro voxel\n",t);
                    egsWarning("x=(%g,%g,%g) u=(%g,%g,%g)\n",x.x,x.y,x.z,
                               u.x,u.y,u.z);
                    egsWarning("ix=%d iy=%d iz=%d\n",vg->ix,vg->iy,vg->iz);
                    egsFatal("quitting\n");
                }
                t = 1e-15;
            }
            if (inew < 0 || inew == ireg) {
                return inew;
            }
            // i.e., exits macro geometry or stays in same macro voxel
            int iorg = organs->getOrgan(vg->iz,vg->iy,vg->ix);
            int mict = organ_micro[iorg];
            if (mict < 0) {  // no micro matrix in the new macro voxel
                if (newmed) {
                    *newmed = organ_media[iorg];
                }
                return inew;
            }
            // if here, enters micro matrix.
            EGS_Vector x1(x + u*t -
                          EGS_Vector(vg->dx*vg->ix,vg->dy*vg->iy,vg->dz*vg->iz));
            int micr = micros[mict]->isWhere(vg->ix,vg->iy,vg->iz,x1,newmed);
            return nmax*mict + micr + nmacro;
        }
        if (ireg >= 0) {
            // in a micro matrix.
            int ireg1 = ireg - nmacro;
            int mict = ireg1/nmax;
            int iloc = ireg1 - mict*nmax;
            //return micros[mict]->medium(iloc);
            int imic, ix, iy, iz;
            EGS_MicroMatrixCluster *micro = micros[mict];
            micro->getIndeces(iloc,imic,ix,iy,iz);
            int ilocal = iloc - imic*micros[mict]->nreg;
            // now we have to figure out the macro voxel indeces
            // in principle one could simply use vg->isWhere(x), but
            // this is bound to get us in trouble with roundoff errors.
            // we therefore treat the special case where x is in an edge
            // voxel of the micro matrix in a special way
            int lax, lay, laz;
            EGS_Float aux = x.x*vg->dxi;
            if (ix > 0 && ix < micros[mict]->vg->nx-1) {
                lax = (int) aux;
            }
            else if (ix == 0) {
                lax = (int)(aux+0.5);
            }
            else {
                lax = (int)(aux-0.5);
            }
            aux = x.y*vg->dyi;
            if (iy > 0 && iy < micros[mict]->vg->ny-1) {
                lay = (int) aux;
            }
            else if (iy == 0) {
                lay = (int)(aux+0.5);
            }
            else {
                lay = (int)(aux-0.5);
            }
            aux = x.z*vg->dzi;
            if (iz > 0 && iz < micros[mict]->vg->nz-1) {
                laz = (int) aux;
            }
            else if (iz == 0) {
                laz = (int)(aux+0.5);
            }
            else {
                laz = (int)(aux-0.5);
            }
            EGS_Vector x1(x - EGS_Vector(vg->dx*lax,vg->dy*lay,vg->dz*laz));
            int inew = micro->vg->howfarIn(ilocal,ix,iy,iz,x1,u,t,normal);
            if (t < 0) {
                if (t < -1e-5) {
                    egsWarning("negative step %g in micro matrix\n",t);
                    egsWarning("x=(%g,%g,%g) u=(%g,%g,%g)\n",x.x,x.y,x.z,
                               u.x,u.y,u.z);
                    egsWarning("ix=%d iy=%d iz=%d\n",ix,iy,iz);
                    egsWarning("lax=%d lay=%d laz=%d\n",lax,lay,laz);
                    egsWarning("x1=(%g,%g,%g)\n",x1.x,x1.y,x1.z);
                    egsFatal("quitting\n");
                }
                t = 0;
            }
            if (inew == ilocal) {
                return ireg;
            }
            if (inew < 0) {
                // exiting current micro matrix => entering new
                // macro voxel.
                // 1. find which one and set new micro region
                if (micro->vg->ix < 0) {
                    if ((--lax) < 0) {
                        return -1;
                    }
                    micro->vg->ix = micro->vg->nx-1;
                }
                else if (micro->vg->ix >= micro->vg->nx) {
                    if ((++lax) >= vg->nx) {
                        return -1;
                    }
                    micro->vg->ix = 0;
                }
                else if (micro->vg->iy < 0) {
                    if ((--lay) < 0) {
                        return -1;
                    }
                    micro->vg->iy = micro->vg->ny-1;
                }
                else if (micro->vg->iy >= micro->vg->ny) {
                    if ((++lay) >= vg->ny) {
                        return -1;
                    }
                    micro->vg->iy = 0;
                }
                else if (micro->vg->iz < 0) {
                    if ((--laz) < 0) {
                        return -1;
                    }
                    micro->vg->iz = micro->vg->nz-1;
                }
                else if (micro->vg->iz >= micro->vg->nz) {
                    if ((++laz) >= vg->nz) {
                        return -1;
                    }
                    micro->vg->iz = 0;
                }
                // 2. micro matrix in new macro voxel?
                int iorg = organs->getOrgan(laz,lay,lax);
                mict = organ_micro[iorg];
                if (mict < 0) {  // no.
                    if (newmed) {
                        *newmed = organ_media[iorg];
                    }
                    return lax + lay*vg->nx + laz*vg->nxy;
                }
                inew = micro->vg->ix + micro->vg->iy*micro->vg->nx +
                       micro->vg->iz*micro->vg->nxy;
                int lix = lax%micros[mict]->mx;
                int liy = lay%micros[mict]->my;
                int liz = laz%micros[mict]->mz;
                imic = lix + micros[mict]->mx*liy + micros[mict]->mxy*liz;
            }
            if (newmed) {
                *newmed = micros[mict]->medium(imic,inew);
            }
            return nmax*mict + inew + nmacro;
        }
        // outside
        int imac = vg->howfarOut(x,u,t,normal);
        if (imac < 0) {
            return imac;
        }
        int iorg = organs->getOrgan(vg->iz,vg->iy,vg->ix);
        int mict = organ_micro[iorg];
        if (mict < 0) {
            if (newmed) {
                *newmed = organ_media[iorg];
            }
            return imac;
        }
        EGS_Vector x1(x + u*t);
        x1 -= EGS_Vector(vg->dx*vg->ix,vg->dy*vg->iy,vg->dz*vg->iz);
        int ilocal = micros[mict]->vg->isWhereFast(x1);
        int ix=vg->ix%micros[mict]->mx,
            iy=vg->iy%micros[mict]->my,
            iz=vg->iz%micros[mict]->mz;
        int imic = ix + iy*micros[mict]->mx + iz*micros[mict]->mxy;
        if (newmed) {
            *newmed = micros[mict]->medium(imic,ilocal);
        }
        return nmax*mict + imic*micros[mict]->nreg + ilocal + nmacro;
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        if (ireg < nmacro) {
            return vg->hownear(ireg,x);
        }
        ireg -= nmacro;
        int mict = ireg/nmax;
        int iloc = ireg - mict*nmax;
        int imic, ix, iy, iz;
        EGS_MicroMatrixCluster *micro = micros[mict];
        micro->getIndeces(iloc,imic,ix,iy,iz);
        int ilocal = iloc - imic*micro->nreg;
        // now we have to figure out the macro voxel indeces
        // in principle one could simply use vg->isWhere(x), but
        // this is bound to get us in trouble with roundoff errors.
        // we therefore treat the special case where x is in an edge
        // voxel of the micro matrix in a special way
        int lax, lay, laz;
        EGS_Float aux = x.x*vg->dxi;
        if (ix > 0 && ix < micro->vg->nx-1) {
            lax = (int) aux;
        }
        else if (ix == 0) {
            lax = (int)(aux+0.5);
        }
        else {
            lax = (int)(aux-0.5);
        }
        aux = x.y*vg->dyi;
        if (iy > 0 && iy < micro->vg->ny-1) {
            lay = (int) aux;
        }
        else if (iy == 0) {
            lay = (int)(aux+0.5);
        }
        else {
            lay = (int)(aux-0.5);
        }
        aux = x.z*vg->dzi;
        if (iz > 0 && iz < micro->vg->nz-1) {
            laz = (int) aux;
        }
        else if (iz == 0) {
            laz = (int)(aux+0.5);
        }
        else {
            laz = (int)(aux-0.5);
        }
        EGS_Vector x1(x - EGS_Vector(vg->dx*lax,vg->dy*lay,vg->dz*laz));
        return micro->vg->hownearIn(ix,iy,iz,x1);
    };

    int getMaxStep() const {
        if (!nmicro) {
            return vg->nx+vg->ny+vg->nz+1;
        }
        int nmx=0, nmy=0, nmz=0;
        for (int mict=0; mict<nmicro; ++mict) {
            if (micros[mict]->nx > nmx) {
                nmx = micros[mict]->nx;
            }
            if (micros[mict]->ny > nmy) {
                nmy = micros[mict]->ny;
            }
            if (micros[mict]->nx > nmz) {
                nmx = micros[mict]->nz;
            }
        }
        return vg->nx*nmx+vg->ny*nmy+vg->nz*nmz+1;
    };

    bool isOK() const {
        return (vg && organs);
    };

    int getNx() const {
        return vg->nx;
    };
    int getNy() const {
        return vg->ny;
    };
    int getNz() const {
        return vg->nz;
    };

    const string &getType() const {
        return type;
    };

    void printInfo() const;

    //static EGS_VHPGeometry* createGeometry(EGS_Input *);
    void setMicros(EGS_Input *);

protected:

    EGS_VoxelGeometry *vg;
    VHP_OrganData     *organs;

    int     organ_media[256];
    int     organ_micro[256];
    string  organ_names[256];
    EGS_MicroMatrixCluster **micros;
    int                    nmicro;
    int                    nmax, nmacro;

    static string    type;

    void setup();

    void setMedia(EGS_Input *inp, int nmed, const int *med_ind);

};

#endif
