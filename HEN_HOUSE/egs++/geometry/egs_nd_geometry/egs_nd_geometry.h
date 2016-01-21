/*
###############################################################################
#
#  EGSnrc egs++ nd geometry headers
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
#  Contributors:    Blake Walters
#                   Ernesto Mainegra-Hing
#                   Frederic Tessier
#
###############################################################################
*/


/*! \file egs_nd_geometry.h
 *  \brief N-dimensional geometries: header
 *  \IK
 */


#ifndef EGS_ND_GEOMETRY_
#define EGS_ND_GEOMETRY_

#ifdef WIN32

    #ifdef BUILD_NDG_DLL
        #define EGS_NDG_EXPORT __declspec(dllexport)
    #else
        #define EGS_NDG_EXPORT __declspec(dllimport)
    #endif
    #define EGS_NDG_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_NDG_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_NDG_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_NDG_EXPORT
        #define EGS_NDG_LOCAL
    #endif

#endif


#include "egs_base_geometry.h"

#include<vector>
#include <iomanip>
using std::vector;

/*! \brief A class modeling a N-dimensional geometry.

\ingroup Geometry
\ingroup CompositeG

To understand an N-dimensional geometry object, consider
an XYZ-geometry made from sets of \f$n_x+1\f$ x-planes, \f$n_y+1\f$ y-planes
and \f$n_z+1\f$ z-planes. This type of geometry is implemented in the
DOSXYZnrc user code and is useful \em e.g. for the modeling of
a computed tomography image of a patient undergoing radiotherapy.
The sets of x-, y- and z-planes define \f$n_x \cdot n_y \cdot n_z\f$
parallelepiped regions (a.k.a. voxels) in space.
The regions can be numbered in any way but it is customary to
use the convention that when a position is in the \f$i\f$'th
x-planes region (\em i.e. between the \f$i\f$'th and \f$i+1\f$'st x-plane),
in the \f$j\f$'th y-planes region and in the \f$k\f$'th z-planes region, it is
in region \f$i + n_x j + n_x \cdot n_y \cdot k\f$ in the XYZ-geometry.
A position is inside an XYZ-geometry if it is inside the x-planes
(\em i.e. it is between the first and last x-plane), inside
the y-planes and inside the z-planes.
To calculate the distance to any boundary along a given direction
(the howfar() method) for a position inside the XYZ-geometry,
one calculates the distance \f$t_x\f$ to the
x-planes, the distance \f$t_y\f$ to the y-panes, the distance \f$t_z\f$
to the z-planes and then takes the minimum of \f$t_x, t_y\f$ and \f$t_z\f$.
To calculate the minimum distance to a boundary in any direction
(the hownear() method) for a position inside the XYZ-geometry,
one calculates the minimum distances to a boundary in any direction
to the x-planes, y-planes and z-planes and takes the minimum of the three
distances. Consider now an RZ-geometry, \em i.e. a geometry made
from \f$n_z+1\f$ z-planes intersecting \f$n_r\f$ concentric cylinders with
their axis along the z-axis to form \f$n_z \cdot n_r\f$ regions.
As with the XYZ-geometry one can use the convention that
when a position is inside the \f$i\f$'th z-planes region and the
\f$j\f$'th cylinder region, it is in the \f$i + n_z \cdot j\f$'th region
of the RZ-geometry. To calculate the distance along a given direction
to the RZ-geometry for an inside position, one calculates the distances
\f$t_z\f$ to the planes and \f$t_r\f$ to the cylinders and takes the minimum
of the two. The minimum distance to a boundary in any direction for
an inside point is also calculated as the minimum of the corresponding
distances to the planes and to the cylinders.

The above discussion should make it clear that, once the howfar(),
hownear(), isInside(), etc., methods
are available for planes and cylinders, the algorithms for calculating
howfar(), hownear()}, isInside(), etc., is
essentially the same for XYZ- and RZ-geometries, with the only difference
that in the former case one uses 3 geometries to divide
the space into regions (\em i.e one has a 3-dimensional geometry)
and in the latter case only 2 geometries (\em i.e one has a
2-dimensional geometry).

An N-dimensional geometry is a generalization of this concept
to an arbitrary number of geometries (dimensions) of
arbitrary types (not just planes and/or cylinders):
An N-dimensional geometry is a geometry type that is constructed
from \f$N\f$ other geometries \f$G_0, G_1, ..., G_{N-1}\f$ defining
\f$n_0, n_1, ..., n_{N-1}\f$ regions so that
- The geometry consists of \f$n_0 \cdot n_1 \cdot ... \cdot n_{N-1}\f$ regions
- A point is inside the geometry if and only if it is inside
  all of the \f$G_0, ..., G_{N-1}\f$ geometries.
- If a point is inside regions \f$i_0, i_1, ..., i_{N-1}\f$
  in \f$G_0, G_1, ..., G_{N-1}\f$,
  it is inside the region \f$i\f$,
\f[
i = \sum_{j=0}^{N-1} i_j m_j~, \quad m_0 = 1~,~~~m_1 = n_0,~~~m_2 = n_0 n_1,
~~~ \cdots ~~~ m_i = m_{i-1} n_{i-1}~,
\f]
in the N-dimensional geometry (as in the case of the XYZ and RZ examples
above, this is just a convention that is convenient in practice).
- If the current position is in region \f$i\f$ inside the
  N-dimension geometry, the distance \f$t\f$ to a boundary in the
  N-dimension geometry along the direction of motion
  (needed by the howfar() method) is given by the minimum of
  the distances \f$t_1,...,t_N\f$ to a boundary in the geometries
  \f$G_0, ..., G_{N-1}\f$. If this minimum is given by the
  \f$j\f$'th constituent geometry \f$G_j\f$, the new region will be
  \f$i + m_j\f$,
  unless the particle would exit \f$G_j\f$, in which case the new region is -1.
  (this is a direct consequence of the region numbering convention
  specified in the above equation).
- If the current position is outside of the N-dimensional geometry,
  the distances to the entry point is calculated as follows:
    -# If the position is outside of \f$G_j\f$, calculate the
       distance \f$t_j\f$ to the entry point of \f$G_j\f$
    -# If this distance exists (\em i.e. the trajectory intersects \f$G_j\f$),
       check if \f$\vec{x} + \vec{u} t_j\f$ is inside all other geometries.
       If yes, exit the loop and return \f$t_j\f$
    -# Repeat a-b for all constituent geometries \f$j\f$.
    .
  This algorithm is also a generalization of the algorithms one
  uses to calculate distances to the boundary of XYZ-, RZ- and
  other 2 or 3 dimensional geometries.
- If the position is inside, the minimum perpendicular distance to a boundary
  \f$t_\perp\f$ in the N-dimensional geometry
  in any direction (needed for the hownear() method) is given
  by the minimum of the perpendicular distances from all constituent geometries.
- If the position is outside, the \f$t_\perp\f$ calculation
  is more complicated and depends upon the orthogonality of the constituent
  geometries. If the constituent geometries are considered to be orthogonal
  (more on this below),
  \f{equation}
  \label{tperp_ortho}
  t_\perp = \sqrt{ \sum_{j} t_{\perp, j}^2 }~.
  \f}
  Here, the summation runs over all constituents \f$j\f$ for which the position
  is outside. If the constituent geometries are not considered to be
  orthogonal, then \f$t_\perp\f$ is the minimum of all perpendicular distances
  just as in the inside case. To understand this algorithm, consider
  a XYZ-geometry (which is an orthogonal geometry). If a position
  is \em e.g. inside the x- and y-planes but outside the z-planes,
  the nearest distance to a boundary in any direction is the perpendicular
  distance to the first or last z-plane (depending on which side the position
  is). If the position is inside the x-planes but outside of the
  y-planes and the z-planes, the nearest distance to a boundary in
  any direction is the distance to one of the edges of the XYZ-geometry and
  is given by \f$\sqrt{t_{\perp,y}^2 + t_{\perp,z}^2}\f$ with
  \f$t_{\perp,y}\f$ and \f$t_{\perp,z}\f$ denoting the perpendicular distances
  to the y- and z-planes intersecting to form this edge. If the position
  is outside of all 3 plane sets, then the nearest distance to the
  XYZ-geometry is the distance to one of the corners, \em i.e.
  \f$\sqrt{t_{\perp,x}^2 + t_{\perp,y}^2 + t_{\perp,z}^2}\f$.
  Equation (1) is simply a generalization to
  \f$N\f$ dimensions. Consider now a 2-dimensional geometry made from the
  intersection of cones with parallel planes. It is relatively easy
  to see that for the regions outside of the cones and the planes
  Eq. (1) overestimates the actual minimum distance in
  any direction (which is the distance to a point on the circle or ellipse
  formed by the intersection of the outer-most conical surface with
  the first or last plane). This geometry is therefore not considered to
  be orthogonal and one uses the minimum of the
  perpendicular distances to the planes and the cones.

It should be clear from the above explanation of the implementation
of the various geometry methods that an N-dimensional geometry can be
constructed from any \f$N\f$ geometries provided that the underlying
algorithms apply for the geometrical structure being modeled.
An N-dimensional geometry is extremely useful and can be employed to model
a wide range of geometries:
- An XYZ geometry where the three constituents are sets of x-, y- and z-planes.
- A RZ geometry where the two constituents are sets of z-cylinders and z-planes
  (it is of course possible to model a ``RZ'' type geometry with an arbitrary
  orientation by using the any-axis and any-plane versions of the cylinder
  and plane sets)
- A polar coordinates geometry from sets of spheres and the EGS_ConeSet
  class with flag set to 2. This type of geometry
  is useful for instance in the calculation of point spread functions.
- One can construct hemi-spheres from a sphere and two planes, prisms with
  bases at any direction relative to the prism axis from the EGS_Prism
  class and one of the plane sets, cylinders with bases not perpendicular
  to the cylinder axis from a set of cylinders and one of the plane sets,
  cylinders divided into azimuthal segments from one of the cylinder classes
  and the EGS_IPlanes class, etc.

An N-dimensional geometry is defined using the following keys:
\verbatim
library = egs_ndgeometry
dimesions = list of names of previously defined geometries
hownear method = 0 or 1
\endverbatim
All constituent geometries of the N-dimensional geometry must have
been defined previously (\em i.e. their definition must appear
before the definition of the N-dimensional geometry in the input file).
The hownear() method input determines if the geometry constituents
are considered to be orthogonal with 0 corresponding to orthogonal
constituent geometries. It is very hard
to automatically determine whether the constituents of an
N-dimensional geometry are orthogonal and therefore it is the
responsibility of the user to provide this input.
Note, however, that the non-orthogonal hownear() version
can always be used, it will just underestimate \f$t_\perp\f$ for orthogonal
geometries and make the simulation run somewhat slower.

N-dimensional geometries are used in many of the example geometry files.
*/
class EGS_NDG_EXPORT EGS_NDGeometry : public EGS_BaseGeometry {

public:

    /*! Construct a N-dimensional geometry from the \a ng dimensions \a G.
    */
    EGS_NDGeometry(int ng, EGS_BaseGeometry **G, const string &Name = "",
                   bool O=true);

    /*! Construct a N-dimensional the array of dimensions \a G.  */
    EGS_NDGeometry(vector<EGS_BaseGeometry *> &G, const string &Name = "",
                   bool O=true);

    ~EGS_NDGeometry();

    bool isInside(const EGS_Vector &x) {
        for (int j=0; j<N; j++) if (!g[j]->isInside(x)) {
                return false;
            }
        return true;
    };

    int isWhere(const EGS_Vector &x) {
        int ireg = 0;
        for (int j=0; j<N; j++) {
            int ij = g[j]->isWhere(x);
            if (ij < 0) {
                return -1;
            }
            ireg += ij*n[j];
        }
        return ireg;
    };

    int inside(const EGS_Vector &x) {
        return isWhere(x);
    };

    EGS_Float howfarToOutside(int ireg, const EGS_Vector &x,
                              const EGS_Vector &u) {
        if (ireg < 0) {
            return 0;
        }
        int itmp = ireg;
        EGS_Float d = 1e30;
        for (int j=N-1; j>=0; j--) {
            int l = itmp/n[j];
            EGS_Float t = g[j]->howfarToOutside(l,x,u);
            if (t <= 0) {
                return t;
            }
            if (t < d) {
                d = t;
            }
        }
        return d;
    };

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed=0, EGS_Vector *normal=0) {
        if (ireg >= 0) {
            int itmp = ireg;
            int inext = -1, idelta, lnew_j;
            for (int j=N-1; j>=0; j--) {
                int l = itmp/n[j];
                int lnew = g[j]->howfar(l,x,u,t,0,normal);
                if (lnew != l) {
                    inext = j;
                    idelta = lnew - l;
                    lnew_j = lnew;
                }
                itmp -= l*n[j];
            }
            if (inext < 0) {
                return ireg;
            }
            int inew = lnew_j >= 0 ? ireg + idelta*n[inext] : lnew_j;
            //if( lnew_j < 0 ) return lnew_j;
            //int inew = ireg + idelta*n[inext];
            if (newmed) {
                *newmed = inew >= 0 ? medium(inew) : -1;
            }
            return inew;
        }
        for (int j=0; j<N; j++) {
            EGS_Float tj = t;
            bool is_in = g[j]->isInside(x);
            // Note: this logic fails if the geometry is not convex!
            if (!is_in) {
                EGS_Vector nn, *np=0;
                if (normal) {
                    np = &nn;
                }
                int i = g[j]->howfar(ireg,x,u,tj,0,np);
                if (i >= 0) {
                    EGS_Vector tmp(x + u*tj);
                    bool is_ok = true;
                    int res = 0;
                    for (int k=0; k<N; k++) {
                        if (k != j) {
                            int check = g[k]->isWhere(tmp);
                            if (check < 0) {
                                is_ok = false;
                                break;
                            }
                            res += check*n[k];
                        }
                    }
                    if (is_ok) {
                        t = tj;
                        res += i*n[j];
                        if (newmed) {
                            *newmed = medium(res);
                        }
                        if (normal) {
                            *normal = nn;
                        }
                        return res;
                    }
                }
            }
            // and so, to fix it, we do the following for non-convex
            // dimensions. But this only works if and only if there
            // is not more than 1 non-convex dimension and this
            // dimension is last in the list.
            else if (!g[j]->isConvex()) {
                EGS_Vector tmp(x);
                EGS_Float tleft = t;
                int ii = g[j]->isWhere(x);
                EGS_Float ttot = 0;
                while (1) {
                    EGS_Float tt = tleft;
                    int inew = g[j]->howfar(ii,tmp,u,tt);
                    if (inew == ii) {
                        break;
                    }
                    tleft -= tt;
                    tmp += u*tt;
                    ii = inew;
                    ttot += tt;
                    //egsWarning(" inew = %d tt = %g ttot = %g tmp = (%g,%g,%g)\n",inew,tt,ttot,tmp.x,tmp.y,tmp.z);
                    if (ii < 0) {
                        break;
                    }
                }
                //egsWarning("doing non-convex part: x = (%g,%g,%g) ii = %d t = %g ttot = %g\n",x.x,x.y,x.z,ii,t,ttot);
                if (ii < 0) {
                    EGS_Vector nn, *np = normal ? &nn : 0;
                    EGS_Float tt = tleft;
                    int inew = g[j]->howfar(ii,tmp,u,tt,0,np);
                    //egsWarning(" exited and tried to reenter: %d %g\n",inew,ttot+tt);
                    if (inew >= 0) {
                        tmp += u*tt;
                        bool is_ok = true;
                        int res = 0;
                        for (int k=0; k<N; k++) {
                            if (k != j) {
                                int check = g[k]->isWhere(tmp);
                                if (check < 0) {
                                    is_ok = false;
                                    break;
                                }
                                res += check*n[k];
                            }
                        }
                        if (is_ok) {
                            t = ttot + tt;
                            res += inew*n[j];
                            if (newmed) {
                                *newmed = medium(res);
                            }
                            if (normal) {
                                *normal = nn;
                            }
                            return res;
                        }
                    }
                }
            }
        }
        return -1;
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        EGS_Float tmin = 1e30;
        if (ireg >= 0) {
            int itmp = ireg;
            for (int j=N-1; j>=0; j--) {
                int l = itmp/n[j];
                EGS_Float t = g[j]->hownear(l,x);
                if (t < tmin) {
                    tmin = t;
                    if (tmin <= 0) {
                        return 0;
                    }
                }
                itmp -= l*n[j];
            }
            return tmin;
        }
        if (ortho) {
            int nc = 0;
            EGS_Float s1=0, s2=0;
            for (int j=0; j<N; j++) {
                if (!g[j]->isInside(x)) {
                    EGS_Float t = g[j]->hownear(-1,x);
                    nc++;
                    s1 += t;
                    s2 += t*t;
                }
            }
            if (nc == 1) {
                return s1;
            }
            return sqrt(s2);
        }
        else {
            EGS_Float tmin = 1e30;
            for (int j=0; j<N; j++) {
                EGS_Float t;
                int i = g[j]->isWhere(x);
                t = g[j]->hownear(i,x);
                if (t < tmin) {
                    tmin = t;
                    if (tmin <= 0) {
                        return 0;
                    }
                }
            }
            return tmin;
        }
    };

    int getMaxStep() const {
        int nstep = 1;
        for (int j=0; j<N; ++j) {
            nstep += g[j]->getMaxStep();
        }
        return nstep;
    };

    const string &getType() const {
        return type;
    };

    void printInfo() const;

    virtual void getLabelRegions(const string &str, vector<int> &regs);
    int ndRegions(int r, int dim, int dimk, int k, vector<int> &regs);

protected:

    int              N;       //!< Number of dimensions
    EGS_BaseGeometry **g;     //!< The dimensions
    int              *n;      //!< Used for calculating region indeces
    static string    type;    //!< The geometry type
    bool ortho;               //!< Is the geometry orthogonal ?

    void setup();

    /*! \brief Define media.

    This function is re-implemented to permit easier media definition
    in a N-dimensional geometry.

    */
    void setMedia(EGS_Input *inp, int nmed, const int *med_ind);

private:

    void setM(int ib, int idim, const vector<int> &ranges, int medium);
};

#ifdef EXPLICIT_XYZ
#include "../egs_planes/egs_planes.h"

/*! \brief An XYZ-geometry

  \ingroup Geometry
  \ingroup CompositeG

An EGS_XYZGeometry is a special case of an
\link EGS_NDGeometry N-dimensional geometry \endlink
but is provided as a separate object to offer an easier definition
because XYZ-geometries are frequently used in practice. An XYZ-geometry
is constructed using
\verbatim
:start geometry:
    library = egs_ndgeometry
    type = EGS_XYZGeometry
    x-planes = list of the x-plane positions
    y-planes = list of the y-plane positions
    z-planes = list of the z-plane positions
:stop geometry:
\endverbatim
and therefore does not require that separate EGS_Xplanes,
EGS_Yplanes and EGS_Zplanes objects are first defined and
then put together with the \c dimension key to form an XYZ-geometry
(although, it would not be wrong to define an XYZ-geometry that way).
The main saving is realized when defining the media of a heterogeneous
XYZ-geometry (see \ref geometry_media). By including
\verbatim
set medium = i1 i2 j1 j2 k1 k2 med
\endverbatim
keys in the media definition section, one can set the
medium in all regions between x-indeces \c i1 and \c i2 (inclusive),
y-indeces \c j1 and \c j2, and z-indeces \c k1 and \c k2 to the
medium with index \c med.

The following two possibilities for defining XYZ geometries have been
added since the 2.2.4 release of EGSnrc:
\verbatim
:start geometry:
    library = egs_ndgeometry
    type = EGS_XYZGeometry
    density matrix = density_file
    ct ramp = ramp_file
:stop geometry:
\endverbatim
where the <code>ramp_file</code> defines mass density to medium
conversion rules and the <code>density_file</code> defines
planes and mass densities. The format of the ct ramp file is
simple:
\verbatim
medium1  min_density1 max_density1 default_density1
medium2  min_density2 max_density2 default_density2
...
\endverbatim
and defines that <code>medium1</code> is to be used for all voxels
with a mass density between <code>min_density1</code> and
<code>max_density1</code>, etc., assuming <code>default_density1</code>, etc.,
id the default mass density for this medium.
There can be an arbitrary number of
lines in the ct ramp file. The <code>density_file</code> is a binary
file that contains: 1 byte set to 0 or 1 for data written on a big- or
little-endian machine,
number of regions in x-,y- and z-direction
(32 bit integers), followed by Nx+1 x-plane positions, Ny+1 y-plane
positions, Nz+1 z-plane positions (32 bit floats in increasing order),
followed by Nx*Ny*Nz mass densities (32 bit floats) with the
convention that the voxel (ix,iy,iz) is region
ix+iy*Nx+iz*Nx*Ny in the mass density matrix. The ct ramp file is
then used to convert the mass densities to media indeces.

The other new possibility to define a XYZ geometry is
\verbatim
:start geometry:
    library = egs_ndgeometry
    type = EGS_XYZGeometry
    x-slabs = Xo  Dx  Nx
    y-slabs = Yo  Dy  Ny
    z-slabs = Zo  Dz  Nz
:stop geometry:
\endverbatim
wich should be self explanatory.

*/

class EGS_NDG_EXPORT EGS_XYZGeometry : public EGS_BaseGeometry {

public:

    EGS_XYZGeometry(EGS_PlanesX *Xp, EGS_PlanesY *Yp, EGS_PlanesZ *Zp,
                    const string &Name = "");

    ~EGS_XYZGeometry();

    bool isInside(const EGS_Vector &x) {
        if (!xp->isInside(x)) {
            return false;
        }
        if (!yp->isInside(x)) {
            return false;
        }
        if (!zp->isInside(x)) {
            return false;
        }
        return true;
    };

    int isWhere(const EGS_Vector &x) {
        int ix = xp->isWhere(x);
        if (ix < 0) {
            return ix;
        }
        int iy = yp->isWhere(x);
        if (iy < 0) {
            return iy;
        }
        int iz = zp->isWhere(x);
        if (iz < 0) {
            return iz;
        }
        return ix + iy*nx + iz*nxy;
    };

    int inside(const EGS_Vector &x) {
        return isWhere(x);
    };

    int computeIntersections(int ireg, int n, const EGS_Vector &X,
                             const EGS_Vector &u, EGS_GeometryIntersections *isections) {
        if (n < 1) {
            return -1;
        }
        int ifirst = 0;
        EGS_Float t, t_ini = 0;
        EGS_Vector x(X);
        int imed;
        if (ireg < 0) {
            t = 1e30;
            ireg = howfar(ireg,x,u,t,&imed);
            if (ireg < 0) {
                return 0;
            }
            isections[0].t = t;
            isections[0].rhof = 1;
            isections[0].ireg = -1;
            isections[0].imed = -1;
            t_ini = t;
            ++ifirst;
            x += u*t;
        }
        else {
            imed = medium(ireg);
        }
        EGS_Float *px = xp->getPositions(),
                   *py = yp->getPositions(),
                    *pz = zp->getPositions();
        int iz = ireg/nxy;
        int ir = ireg - iz*nxy;
        int iy = ir/nx;
        int ix = ir - iy*nx;
        EGS_Float uxi, uyi, uzi, nextx, nexty, nextz;
        int dirx, icx, diry, icy, dirz, icz;
        if (u.x > 0)      {
            uxi = 1/u.x;
            dirx =  1;
            icx = 1;
        }
        else if (u.x < 0) {
            uxi = 1/u.x;
            dirx = -1;
            icx = 0;
        }
        else               {
            uxi = 1e33;
            dirx =  1;
            icx = 1;
        }
        if (u.y > 0)      {
            uyi = 1/u.y;
            diry =  1;
            icy = 1;
        }
        else if (u.y < 0) {
            uyi = 1/u.y;
            diry = -1;
            icy = 0;
        }
        else               {
            uyi = 1e33;
            diry =  1;
            icy = 1;
        }
        if (u.z > 0)      {
            uzi = 1/u.z;
            dirz =  1;
            icz = 1;
        }
        else if (u.z < 0) {
            uzi = 1/u.z;
            dirz = -1;
            icz = 0;
        }
        else               {
            uzi = 1e33;
            dirz =  1;
            icz = 1;
        }
        nextx = (px[ix+icx] - x.x)*uxi + t_ini;
        nexty = (py[iy+icy] - x.y)*uyi + t_ini;
        nextz = (pz[iz+icz] - x.z)*uzi + t_ini;
        for (int j=ifirst; j<n; j++) {
            isections[j].imed = imed;
            isections[j].ireg = ireg;
            isections[j].rhof = getRelativeRho(ireg);
            int inew;
            if (nextx < nexty && nextx < nextz) {
                t = nextx;
                ix += dirx;
                if (ix < 0 || ix >= nx) {
                    inew = -1;
                }
                else {
                    inew = ireg + dirx;
                    nextx = (px[ix+icx] - x.x)*uxi + t_ini;
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
                    nexty = (py[iy+icy] - x.y)*uyi + t_ini;
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
                    nextz = (pz[iz+icz] - x.z)*uzi + t_ini;
                }
            }
            isections[j].t = t;
            if (inew < 0) {
                return j+1;
            }
            ireg = inew;
            imed = medium(ireg);
        }
        return ireg >= 0 ? -1 : n;
    };

    EGS_Float howfarToOutside(int ireg, const EGS_Vector &x,
                              const EGS_Vector &u) {
        if (ireg < 0) {
            return 0;
        }
        int iz = ireg/nxy;
        int ir = ireg - iz*nxy;
        int iy = ir/nx;
        int ix = ir - iy*nx;
        EGS_Float  d = xp->howfarToOutside(ix,x,u);
        EGS_Float ty = yp->howfarToOutside(iy,x,u);
        if (ty < d) {
            d = ty;
        }
        EGS_Float tz = zp->howfarToOutside(iz,x,u);
        if (tz < d) {
            d = tz;
        }
        return d;
    };

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed=0, EGS_Vector *normal=0) {
        if (ireg >= 0) {
            int iz = ireg/nxy;
            int ir = ireg - iz*nxy;
            int iy = ir/nx;
            int ix = ir - iy*nx;
            int inew = ireg;
            if (u.x > 0) {
                EGS_Float d = (xpos[ix+1]-x.x)/u.x;
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
                EGS_Float d = (xpos[ix]-x.x)/u.x;
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
                EGS_Float d = (ypos[iy+1]-x.y)/u.y;
                if (d <= t) {
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
                EGS_Float d = (ypos[iy]-x.y)/u.y;
                if (d <= t) {
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
                EGS_Float d = (zpos[iz+1]-x.z)/u.z;
                if (d <= t) {
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
                EGS_Float d = (zpos[iz]-x.z)/u.z;
                if (d <= t) {
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
            if (newmed && inew >= 0) {
                *newmed = medium(inew);
            }
            return inew;
        }
        int face,ix,iy,iz;
        int inew = howfarFromOut(x,u,t,ix,iy,iz,face,normal);
        if (inew >= 0 && newmed) {
            *newmed = medium(inew);
        }
        return inew;
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        if (ireg >= 0) {
            int iz = ireg/nxy;
            int ir = ireg - iz*nxy;
            int iy = ir/nx;
            int ix = ir - iy*nx;
            EGS_Float t = xp->hownear(ix,x);
            EGS_Float t1 = yp->hownear(iy,x);
            if (t1 < t) {
                t = t1;
            }
            t1 = zp->hownear(iz,x);
            if (t1 < t) {
                t = t1;
            }
            return t;
        }
        int nc = 0;
        EGS_Float s1=0, s2=0;
        if (!xp->isInside(x)) {
            EGS_Float t = xp->hownear(-1,x);
            nc++;
            s1 += t;
            s2 += t*t;
        }
        if (!yp->isInside(x)) {
            EGS_Float t = yp->hownear(-1,x);
            nc++;
            s1 += t;
            s2 += t*t;
        }
        if (!zp->isInside(x)) {
            EGS_Float t = zp->hownear(-1,x);
            nc++;
            s1 += t;
            s2 += t*t;
        }
        if (nc == 1) {
            return s1;
        }
        return sqrt(s2);
    };


    EGS_Float getMass(int ireg) {
        if (ireg >= 0) {
            int iz = ireg/nxy;
            int ir = ireg - iz*nxy;
            int iy = ir/nx;
            int ix = ir - iy*nx;
            EGS_Float vol = (xpos[ix+1]-xpos[ix])*(ypos[iy+1]-ypos[iy])*
                            (zpos[iz+1]-zpos[iz]);
            EGS_Float dens = getRelativeRho(ireg);
            EGS_Float mass = vol*dens;
            return mass;
        }
        else {
            return 1.0;
        }
    }

    EGS_Float getBound(int idir, int ind) {
        EGS_Float bound;
        if (idir == 0) {
            if (ind>=0 && ind<=nx) {
                bound = xpos[ind];
            }
            else {
                egsWarning("Error in getBound: Looking for X bound out of range %d\n"
                           "Will set this to zero.\n",ind);
                bound = 0.0;
            }
        }
        else if (idir == 1) {
            if (ind>=0 && ind<=ny) {
                bound = ypos[ind];
            }
            else {
                egsWarning("Error in getBound: Looking for Y bound out of range %d\n"
                           "Will set this to zero.\n",ind);
                bound = 0.0;
            }
        }
        else if (idir == 2) {
            if (ind>=0 && ind<=nz) {
                bound = zpos[ind];
            }
            else {
                egsWarning("Error in getBound: Looking for Z bound out of range %d\n"
                           "Will set this to zero.\n",ind);
                bound = 0.0;
            }
        }
        else {
            egsWarning("Error in getBound: Dimension %d undefined in EGS_XYZGeometry.\n",idir);
            bound = 0.0;
        }
        return bound;
    }

    int getNRegDir(int idir) {
        if (idir==0) {
            return nx;
        }
        else if (idir==1) {
            return ny;
        }
        else if (idir==2) {
            return nz;
        }
        else {
            egsWarning("Error in getNregDir: Dimension %d undefined in EGS_XYZGeometry.\n",
                       idir);
            return 0;
        }
    }


    int getMaxStep() const {
        return nx+ny+nz+1;
    };

    int getNx() const {
        return nx;
    };
    int getNy() const {
        return ny;
    };
    int getNz() const {
        return nz;
    };

    EGS_Float *getXPositions() {
        return xp->getPositions();
    };
    EGS_Float *getYPositions() {
        return yp->getPositions();
    };
    EGS_Float *getZPositions() {
        return zp->getPositions();
    };

    static int getDigits(int i);

    const string &getType() const {
        return type;
    };

    void printInfo() const;

    static EGS_XYZGeometry *constructGeometry(const char *dens_or_egphant_file,
            const char *ramp_file, int dens_or_egphant=0);
    static EGS_XYZGeometry *constructCTGeometry(const char *dens_or_egphant_file);

    void voxelizeGeometry(EGS_Input *input);

    void setXYZLabels(EGS_Input *input);

    virtual void getLabelRegions(const string &str, vector<int> &regs);
    void getXLabelRegions(const string &str, vector<int> &regs) {
        xp->getLabelRegions(str, regs);
    }
    void getYLabelRegions(const string &str, vector<int> &regs) {
        yp->getLabelRegions(str, regs);
    }
    void getZLabelRegions(const string &str, vector<int> &regs) {
        zp->getLabelRegions(str, regs);
    }

protected:

    EGS_PlanesX      *xp;
    EGS_PlanesY      *yp;
    EGS_PlanesZ      *zp;
    EGS_Float        *xpos;
    EGS_Float        *ypos;
    EGS_Float        *zpos;
    int              nx, ny, nz, nxy;
    static string    type;

    void setup();

    void setMedia(EGS_Input *inp, int nmed, const int *med_ind);

    int howfarFromOut(const EGS_Vector &x, const EGS_Vector &u,
                      EGS_Float &t, int &ix, int &iy, int &iz,
                      int &face, EGS_Vector *normal=0) {
        ix = -1;
        EGS_Float t1;
        if (x.x <= xpos[0] && u.x > 0) {
            t1 = (xpos[0]-x.x)/u.x;
            ix = 0;
            face = 0;
        }
        else if (x.x >= xpos[nx] && u.x < 0) {
            t1 = (xpos[nx]-x.x)/u.x;
            ix = nx-1;
            face = 1;
        }
        if (ix >= 0 && t1 <= t) {
            EGS_Vector tmp(x + u*t1);
            iy = yp->isWhere(tmp);
            if (iy >= 0) {
                iz = zp->isWhere(tmp);
                if (iz >= 0) {
                    int inew =  ix + iy*nx + iz*nxy;
                    if (t1 < epsilon) {
                        int itest = howfar(inew, tmp, u, t1);
                        if (itest == -1 && t1 < epsilon) {
                            return -1;
                        }
                    }
                    if (normal) *normal = face == 0 ? EGS_Vector(-1,0,0) :
                                              EGS_Vector(1,0,0);
                    t = t1;
                    return inew;
                }
            }
        }
        iy = -1;
        if (x.y <= ypos[0] && u.y > 0) {
            t1 = (ypos[0]-x.y)/u.y;
            iy = 0;
            face = 2;
        }
        else if (x.y >= ypos[ny] && u.y < 0) {
            t1 = (ypos[ny]-x.y)/u.y;
            iy = ny-1;
            face = 3;
        }
        if (iy >= 0 && t1 <= t) {
            EGS_Vector tmp(x + u*t1);
            ix = xp->isWhere(tmp);
            if (ix >= 0) {
                iz = zp->isWhere(tmp);
                if (iz >= 0) {
                    int inew =  ix + iy*nx + iz*nxy;
                    if (t1 < epsilon) {
                        int itest = howfar(inew, tmp, u, t1);
                        if (itest == -1 && t1 < epsilon) {
                            return -1;
                        }
                    }
                    if (normal) *normal = face == 2 ? EGS_Vector(0,-1,0) :
                                              EGS_Vector(0, 1,0);
                    t = t1;
                    return inew;
                }
            }
        }
        iz = -1;
        if (x.z <= zpos[0] && u.z > 0) {
            t1 = (zpos[0]-x.z)/u.z;
            iz = 0;
            face = 4;
        }
        else if (x.z >= zpos[nz] && u.z < 0) {
            t1 = (zpos[nz]-x.z)/u.z;
            iz = nz-1;
            face = 5;
        }
        if (iz >= 0 && t1 <= t) {
            EGS_Vector tmp(x + u*t1);
            ix = xp->isWhere(tmp);
            if (ix >= 0) {
                iy = yp->isWhere(tmp);
                if (iy >= 0) {
                    int inew =  ix + iy*nx + iz*nxy;
                    if (t1 < epsilon) {
                        int itest = howfar(inew, tmp, u, t1);
                        if (itest == -1 && t1 < epsilon) {
                            return -1;
                        }
                    }
                    if (normal) *normal = face == 4 ? EGS_Vector(0,0,-1) :
                                              EGS_Vector(0,0, 1);
                    t = t1;
                    return inew;
                }
            }
        }
        return -1;
    };
};

/*! \brief A deformed XYZ-geometry

    \ingroup Geometry
    \ingroup CompositeG

    This class modelas a XYZ geometry where the voxel corners are transformed to new locations
    according to a user defined deformation field. This implementation
    divides each voxel into 6 tetrahedra, which makes howfar() and hownear()
    computations much easier. The downside of this approach is that the
    number of regions increases by a factor of 6.

    Note: this geometry is not usable yet because there is no way to define it
          in the input file.
 */
class EGS_NDG_EXPORT EGS_DeformedXYZ : public EGS_XYZGeometry {

public:

    EGS_DeformedXYZ(EGS_PlanesX *Xp, EGS_PlanesY *Yp, EGS_PlanesZ *Zp,
                    const char *defFile, const string &Name = "");

    ~EGS_DeformedXYZ();

    int isWhere(const EGS_Vector &x) {
        int ireg = EGS_XYZGeometry::isWhere(x);
        if (ireg >= 0) egsFatal("EGS_DeformedXYZ::isWhere(): this geometry "
                                    "does not allow the use of this method with position inside\n");
        return ireg;
    };

    int inside(const EGS_Vector &x) {
        return isWhere(x);
    };

    int medium(int ireg) const {
        return EGS_XYZGeometry::medium(ireg/6);
    };

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed=0, EGS_Vector *normal=0) {
        if (ireg >= 0) {
            int ir = ireg/6, tetra = ireg - 6*ir, tetra4 = 4*tetra;
            int ind[3];
            ind[2] = ir/nxy;
            ind[1] = (ir - ind[2]*nxy)/nx;
            ind[0] = ir - ind[2]*nxy - ind[1]*nx;
            int edge = ir + ind[1] + ind[2]*nxyp1;
            EGS_Vector n0(vectors[edge+tnodes[tetra4]]),
                       n1(vectors[edge+tnodes[tetra4+1]]-n0),
                       n2(vectors[edge+tnodes[tetra4+2]]-n0),
                       n3(vectors[edge+tnodes[tetra4+3]]-n0);
            EGS_Vector n0x(n0-x);
            EGS_Float disti[4],dpi[4];
            EGS_Vector normvec;
            normvec = n1%n2;
            dpi[0]=normvec*u;
            disti[0]=normvec*n0x;
            normvec = n3%n1;
            dpi[2]=normvec*u;
            disti[2]=normvec*n0x;
            normvec = n2%n3;
            dpi[1]=normvec*u;
            disti[1]=normvec*n0x;
            normvec = (n3-n1)%(n2-n1);
            dpi[3] = normvec*u;
            disti[3] = normvec*(n0x+n1);
            int nextplane = 4;
            EGS_Float mindist = t, mindp=1;
            for (int i=0; i<4; ++i) {
                if (dpi[i] > 0 && disti[i]*mindp < mindist*dpi[i]) {
                    mindist = disti[i];
                    mindp=dpi[i];
                    nextplane=i;
                }
            }
            if (nextplane == 4) {
                return ireg;
            }
            t = mindist/mindp;
            int j = 12*tetra + 3*nextplane;
            int next = tetra_data[j];
            int irnew = ir;
            if (next >= 0) {
                ind[next] += tetra_data[j+1];
                if (ind[next] < 0 || ind[next] >= np[next]) {
                    return -1;
                }
                irnew = ind[0] + ind[1]*nx + ind[2]*nxy;
            }
            if (newmed) {
                *newmed = EGS_XYZGeometry::medium(irnew);
            }
            if (normal) {
                switch (nextplane) {
                case 0:
                    *normal = n2%n1;
                    break;
                case 1:
                    *normal = n1%n3;
                    break;
                case 2:
                    *normal = n3%n2;
                    break;
                case 3:
                    *normal = (n2-n1)%(n3-n1);
                }
            }
            return 6*irnew + tetra_data[j+2];
        }
        int ix,iy,iz,face;
        int inew = howfarFromOut(x,u,t,ix,iy,iz,face,normal);
        if (inew < 0) {
            return inew;
        }
        if (newmed) {
            *newmed = EGS_XYZGeometry::medium(inew);
        }
        // need to determine the tetrahedron we are entering.
        int edge = inew + iy + iz*nxyp1;
        EGS_Vector tmp(x + u*t);
        int tetra1 = enter_tetra1[face], plane1 = enter_plane1[face];
        int node1 = tnodes[4*tetra1 + plane_order[3*plane1]],
            node2 = tnodes[4*tetra1 + plane_order[3*plane1+1]],
            node3 = tnodes[4*tetra1 + plane_order[3*plane1+2]];
        if (inTriangle(vectors[edge+node1],vectors[edge+node2],
                       vectors[edge+node3],tmp)) {
            return 6*inew + tetra1;
        }
        int tetra2 = enter_tetra2[face], plane2 = enter_plane2[face];
        int node1a = tnodes[4*tetra2 + plane_order[3*plane2]],
            node2a = tnodes[4*tetra2 + plane_order[3*plane2+1]],
            node3a = tnodes[4*tetra2 + plane_order[3*plane2+2]];
        if (inTriangle(vectors[edge+node1a],vectors[edge+node2a],
                       vectors[edge+node3a],tmp)) {
            return 6*inew + tetra2;
        }
        // roundoff problem
        egsWarning("deformed geometry: entering from face %d but inTriangle()"
                   " fails for both triangles\n",face);
        egsWarning("x_enter=(%16.10f,%16.10f,%16.10f)\n",tmp.x,tmp.y,tmp.z);
        return -1;
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        if (ireg < 0) {
            return EGS_XYZGeometry::hownear(ireg,x);
        }
        int ir = ireg/6, tetra = ireg - 6*ir, tetra4 = 4*tetra;
        int ind[3];
        ind[2] = ir/nxy;
        ind[1] = (ir - ind[2]*nxy)/nx;
        ind[0] = ir - ind[2]*nxy - ind[1]*nx;
        int edge = ir + ind[1] + ind[2]*nxyp1;
        EGS_Vector n0(vectors[edge+tnodes[tetra4]]),
                   n1(vectors[edge+tnodes[tetra4+1]]-n0),
                   n2(vectors[edge+tnodes[tetra4+2]]-n0),
                   n3(vectors[edge+tnodes[tetra4+3]]-n0);
        EGS_Vector n0x(n0-x);
        EGS_Float disti[4],dpi[4];
        EGS_Vector normvec;
        normvec = n1%n2;
        dpi[0]=normvec.length2();
        disti[0]=normvec*n0x;
        normvec = n3%n1;
        dpi[2]=normvec.length2();
        disti[2]=normvec*n0x;
        normvec = n2%n3;
        dpi[1]=normvec.length2();
        disti[1]=normvec*n0x;
        normvec = (n3-n1)%(n2-n1);
        dpi[3] = normvec.length2();
        disti[3] = normvec*(n0x+n1);
        EGS_Float mindist = 1e30, mindp=1;
        for (int i=0; i<4; ++i) {
            if (disti[i]*mindp < mindist*dpi[i]) {
                mindist = disti[i];
                mindp=dpi[i];
            }
        }
        return mindist/mindp;
    };

    int computeIntersections(int ireg, int n, const EGS_Vector &X,
                             const EGS_Vector &u, EGS_GeometryIntersections *isections) {
        return EGS_BaseGeometry::computeIntersections(ireg,n,X,u,isections);
    };

    EGS_Float howfarToOutside(int ireg, const EGS_Vector &x,
                              const EGS_Vector &u) {
        if (ireg < 0) {
            return 0;
        }
        int ir = ireg/6;
        int iz = ir/nxy;
        int ir1 = ir - iz*nxy;
        int iy = ir1/nx;
        int ix = ir1 - iy*nx;
        EGS_Float  d = xp->howfarToOutside(ix,x,u);
        EGS_Float ty = yp->howfarToOutside(iy,x,u);
        if (ty < d) {
            d = ty;
        }
        EGS_Float tz = zp->howfarToOutside(iz,x,u);
        if (tz < d) {
            d = tz;
        }
        return d;
    };

    int getMaxStep() const {
        return 6*(nx+ny+nz) + 1;
    };

    const string &getType() const {
        return def_type;
    };

    void printInfo() const {};

    /*!  Set deformations contained in the file \a defFile.
     *   \a defFile must be a binary file containing
     *     - number Nv of vectors (32 bit integer)
     *     - 3*Nv 32 bit floating point numbers defining the Nv deformation
     *       vectors.
     *   Note that the number of deformation vectors must be consistent with
     *   the number of voxels in the underlying XYZ geometry. The deformation
     *   for the corner of voxel (ix,iy,iz) is at localtion
     *     ix + iy*(Nx+1) + iz*(Nx+1)*(Ny+1)
     *   where Nx,Ny,Nz are the number of voxels.
     */
    int  setDeformations(const char *defFile);

    bool inTriangle(const EGS_Vector &n0, const EGS_Vector &n1,
                    const EGS_Vector &n2, const EGS_Vector &x) {
        EGS_Vector u1(n1-n0), u2(n2-n0), xp(x-n0);
        EGS_Float a=u1.length2(), b=u1*u2, c=u2.length2(), b1=xp*u1, b2=xp*u2;
        EGS_Float D=a*c-b*b;
        EGS_Float u=b1*c-b2*b;
        if (u<0||u>D) {
            return false;
        }
        EGS_Float v=b2*a-b1*b;
        if (v<0||u+v>D) {
            return false;
        }
        return true;
    };

protected:

    EGS_Vector   *vectors;

    int          tnodes[24];

    int          np[3];

    int          nxyp1;

    static string def_type;

    static char tetrahedra[];

    static char plane_order[];

    static char tetra_data[];

    static char enter_tetra1[];
    static char enter_plane1[];
    static char enter_tetra2[];
    static char enter_plane2[];


};

/*! \brief A geometry repeated on a regular XYZ grid.

  \ingroup Geometry
  \ingroup CompositeG

The main reason for the introduction of the EGS_XYZRepeater geometry is
convenience when defining this type of geometry in an input file, although
some optimizations in the implementation of the various geometry methods
has also been achieved. A EGS_XYZRepeater is a geometry where a base geometry
is repeated \f$N_x \times N_y \times N_z\f$ times on a regular XYZ grid.
It is defined using
\verbatim
:start geometry:
    library = egs_ndgeometry
    type = EGS_XYZRepeater
    repeated geometry = g_name
    repeat x = xmin xmax Nx
    repeat y = ymin ymax Ny
    repeat z = zmin zmax Nz
    medium = m_name
:stop geometry:
\endverbatim
where <code>g_name</code> is the name of a pre-existing geometry.
The <code>medium</code> input is optional. If present, the space within
<code>xmin...xmax,ymin...ymax,zmin...zmax</code> is filled with the
nedium named <code>m_name</code>, otherwise with vacuum. If the
repeated geometry has \f$N_r\f$ regions, then there will be
\f$N_x \times N_y \times N_z \times N_r + 1\f$ regions in the geometry,
with the space surrounding the repeated geometry having region index
\f$N_x \times N_y \times N_z \times N_r\f$. The repeated geometry
can be of any type and complexity, the only requirement is that it
completely fits within the box \f$-Delta x/2 \cdots Delta x/2,
-Delta y/2 \cdots Delta y/2,-Delta z/2 \cdots Delta z/2\f$ about
the origin, where \f$\Delta x\f$=<code>(xmax-xmin)/Nx</code>, etc.

See \c repeated_geometry.geom geometry file for an example of
EGS_XYZRepeater use.
*/
class EGS_NDG_EXPORT EGS_XYZRepeater : public EGS_BaseGeometry {

public:

    EGS_XYZRepeater(EGS_Float Xmin, EGS_Float Xmax,
                    EGS_Float Ymin, EGS_Float Ymax,
                    EGS_Float Zmin, EGS_Float Zmax,
                    int Nx, int Ny, int Nz, EGS_BaseGeometry *G,
                    const string &Name = "");

    ~EGS_XYZRepeater();

    int isWhere(const EGS_Vector &x) {
        int cell = xyz->isWhere(x);
        //egsWarning("isInside(%g,%g,%g): cell=%d\n",x.x,x.y,x.z,cell);
        if (cell < 0) {
            return cell;
        }
        EGS_Vector xp(x - translation[cell]);
        int ir = g->isWhere(xp);
        //egsWarning("xp=(%g,%g,%g) ir=%d\n",xp.x,xp.y,xp.z,ir);
        return ir >= 0 ? cell*ng + ir : nreg-1;
    };

    bool isInside(const EGS_Vector &x) {
        return xyz->isInside(x);
    };

    int inside(const EGS_Vector &x) {
        return isWhere(x);
    };

    int medium(int ireg) const {
        if (ireg == nreg-1) {
            return med;
        }
        int cell = ireg/ng;
        int ilocal = ireg - ng*cell;
        return g->medium(ilocal);
    };

    EGS_Float howfarToOutside(int ireg, const EGS_Vector &x,
                              const EGS_Vector &u) {
        if (ireg < 0) {
            return 0;
        }
        return xyz->howfarToOutside(0,x,u);
    };

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed=0, EGS_Vector *normal=0) {
        //egsWarning("\n***howfar(reg=%d,x=(%g,%g,%g),u=(%g,%g,%g),t=%g)\n",
        //        ireg,x.x,x.y,x.z,u.x,u.y,u.z,t);
        if (ireg < 0) {
            // outside of the box.
            int cell = xyz->howfar(ireg,x,u,t,0,normal);
            //egsWarning("entering: cell=%d t=%g\n",cell,t);
            if (cell >= 0) {
                if (newmed) {
                    *newmed = med;
                }
                return nreg-1;
            }
            return -1;
        }
        if (ireg >= 0) {
            if (ireg < nreg-1) {  // in one of the repetitions
                int cell = ireg/ng;
                int ilocal = ireg - cell*ng;
                EGS_Vector xp(x - translation[cell]);
                int inew = g->howfar(ilocal,xp,u,t,newmed,normal);
                //egsWarning("cell=%d il=%d inew=%d t=%g xp=(%g,%g,%g)\n",
                //        cell,ilocal,inew,t,xp.x,xp.y,xp.z);
                if (inew >= 0) {
                    return cell*ng + inew;
                }
                if (newmed) {
                    *newmed = med;
                }
                return nreg-1;
            }
        }
        // if here, we are in the box outside of all repetitions
        // find the cell we are in
        int ix = (int)(x.x*dxi + ax);
        if (ix >= nx) {
            ix = nx-1;
        }
        int iy = (int)(x.y*dyi + ay);
        if (iy >= ny) {
            iy = ny-1;
        }
        int iz = (int)(x.z*dzi + az);
        if (iz >= nz) {
            iz = nz-1;
        }
        int cell = ix + iy*nx + iz*nxy;
        //egsWarning("in box: ix=%d iy=%d iz=%d\n",ix,iy,iz);
        EGS_Float t_left = t;
        EGS_Vector xtmp(x);
        EGS_Float ttot = 0;
        while (1) {
            EGS_Float this_t = t_left;
            EGS_Vector xp(xtmp - translation[cell]);
            int inew = g->howfar(-1,xp,u,this_t,newmed,normal);
            //egsWarning("cell=%d tleft=%g xp=(%g,%g,%g) inew=%d this_t=%g\n",
            //        cell,t_left,xp.x,xp.y,xp.z,inew,this_t);
            if (inew >= 0) {
                t = ttot + this_t;
                return cell*ng + inew;
            }
            int next_cell = xyz->howfar(cell,xtmp,u,this_t,0,normal);
            //egsWarning("next: %d %g\n",next_cell,this_t);
            if (next_cell == cell) {
                return ireg;
            }
            if (next_cell < 0) {
                t = ttot + this_t;
                return -1;
            }
            ttot += this_t;
            t_left -= this_t;
            xtmp += u*this_t;
            cell = next_cell;
        }
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        if (ireg < 0) {
            return xyz->hownear(ireg,x);
        }
        if (ireg < nreg-1) {
            int cell = ireg/ng;
            int ilocal = ireg - cell*ng;
            EGS_Vector xp(x - translation[cell]);
            return g->hownear(ilocal,xp);
        }
        // if here, we are in the box outside of all repetitions
        // find the cell we are in
        int ix = (int)(x.x*dxi + ax);
        if (ix >= nx) {
            ix = nx-1;
        }
        int iy = (int)(x.y*dyi + ay);
        if (iy >= ny) {
            iy = ny-1;
        }
        int iz = (int)(x.z*dzi + az);
        if (iz >= nz) {
            iz = nz-1;
        }
        int cell = ix + iy*nx + iz*nxy;
        EGS_Vector xp(x - translation[cell]);
        EGS_Float t = g->hownear(-1,xp);
        EGS_Float tcell = xyz->hownear(cell,x);
        if (t < tcell) {
            return t;
        }
        for (int iix=ix-1; iix<=ix+1; ++iix) {
            if (iix >= 0 && iix < nx) {
                for (int iiy=iy-1; iiy<=iy+1; ++iiy) {
                    if (iiy >= 0 && iiy < ny) {
                        for (int iiz=iz-1; iiz<=iz+1; ++iiz) {
                            if (iiz >= 0 && iiz < nz) {
                                if (iix != ix || iiy != iy || iiz != iz) {
                                    int cell1 = iix+iiy*nx+iiz*nz;
                                    EGS_Vector tmp(x-translation[cell1]);
                                    EGS_Float t1 = g->hownear(-1,tmp);
                                    if (t1 < t) {
                                        t = t1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return t;
    };

    int getMaxStep() const {
        return nxyz*(g->getMaxStep() + 1) + 1;
    };

    const string &getType() const {
        return type;
    };

    void printInfo() const;

    void setXYZLabels(EGS_Input *input) {
        xyz->setXYZLabels(input);
    }

    virtual void getLabelRegions(const string &str, vector<int> &regs);

protected:

    EGS_XYZGeometry  *xyz;
    EGS_BaseGeometry *g;
    EGS_Vector       *translation;
    EGS_Float        dx, dxi, ax;
    EGS_Float        dy, dyi, ay;
    EGS_Float        dz, dzi, az;
    int              nx, ny, nz, nxy, nxyz, ng;

    static string    type;

};

#endif

#endif
