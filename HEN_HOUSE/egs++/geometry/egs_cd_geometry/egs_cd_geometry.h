/*
###############################################################################
#
#  EGSnrc egs++ cd geometry headers
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
#  Contributors:    Ernesto Mainegra-Hing
#                   Frederic Tessier
#                   Reid Townson
#                   Marc Chamberland
#
###############################################################################
*/


/*! \file egs_cd_geometry.h
 *  \brief A CD-geometry: header
 *  \IK
 */

#ifndef EGS_CD_GEOMETRY_
#define EGS_CD_GEOMETRY_

#include "egs_base_geometry.h"
#include "egs_functions.h"

#ifdef WIN32

    #ifdef BUILD_CDGEOMETRY_DLL
        #define EGS_CDGEOMETRY_EXPORT __declspec(dllexport)
    #else
        #define EGS_CDGEOMETRY_EXPORT __declspec(dllimport)
    #endif
    #define EGS_CDGEOMETRY_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_CDGEOMETRY_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_CDGEOMETRY_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_CDGEOMETRY_EXPORT
        #define EGS_CDGEOMETRY_LOCAL
    #endif

#endif

#include <vector>
using std::vector;

/*! \brief A "combinatorial dimension" geometry.

  \ingroup Geometry
  \ingroup CompositeG

The name of this geometry type is not very intuitive but
resulted from an early, much less general implementation, which was
called a "combinatorial dimension" (CD) geometry. For a lack of
a better name, the CD notation was kept. A CD geometry
consists of a base geometry \f$G_B\f$, which defines \f$n_B\f$ regions.
For each of the \f$n_B\f$ regions there are geometries
\f$G_0, G_1, ..., G_{n_B-1}\f$,
which divide the \f$n_B\f$ regions into additional regions. As an example,
consider a set of 4 parallel planes that defines 3 regions. In region
0 there is a set of 2 spheres, in region 1 a set of 2 cylinders and
in region 2 another set of 2 spheres to define a cylindrical structure
with rounded ends as shown in
\htmlonly
the following figure
\endhtmlonly
\image html cd1.png "An example of a CD geometry."
\latexonly
Fig. \ref{fig_cd1}
\begin{figure}[h]
\begin{center}
\includegraphics[width=14cm]{/home/iwan/HEN_HOUSE/doc/pirs899/figures/cd1.eps}
\end{center}
\caption{\label{fig_cd1}
An example of a CD geometry. The 4 vertical planes form the
base geometry with 3 regions. The sets of 2 spheres divide
regions 0 and 2 into 2 additional regions and the set of 2 cylinders
region 1 into 2 additional regions. The solid lines indicate the
boundaries that are active during the simulation, the dotted lines
the boundaries which are defined by the various geometries but ignored
due to the way the logic of a CD geometry works.}
\end{figure}
\endlatexonly
As another example consider a
set of 2 cylinders defining 2 regions. In region 0 (within the inner cylinder)
there is a set of \f$N\f$ parallel planes that define \f$N-1\f$ regions and in
region 1 of the base cylinder there is a set of just 2 planes that defines
a single region, as shown in
\htmlonly
the following figure
\endhtmlonly
\image html cd2.png "An example of a CD geometry."
\latexonly
Fig. \ref{fig_cd2}.
\begin{figure}[h]
\begin{center}
\includegraphics[width=14cm]{/home/iwan/HEN_HOUSE/doc/pirs899/figures/cd2.eps}
\end{center}
\caption{\label{fig_cd2}
An example of a CD geometry. The base geometry is a set of two concentric
cylinders shown as the 3 horizontal lines. Region 0 of the cylindrical
geometry is divided into 14 regions by 15 parallel planes, region 1 into a
single region by 2 parallel planes.}
\end{figure}
\endlatexonly
One could use the second geometry
to calculate the depth dose curve of a beam incident from the top,
for instance.
The CD geometry type is very versatile and can be used to model a
wide range of geometrical structures. It is therefore important to understand
the logic behind the various geometry methods, which is as follows:
- A point is inside a CD geometry if it is inside \f$G_B\f$ and in addition
  it is inside the geometry for the region of \f$G_B\f$ in which the point is,
  if there is such a geometry defined (note: it is possible that a region
  of the CD geometry does not have an additional geometry defined).
- If a point is in region \f$j\f$ of \f$G_B\f$ and in region \f$i\f$
  of \f$G_j\f$ (the geometry belonging to region \f$j\f$ of \f$G_B\f$),
  the region index
  in the CD geometry is \f$j n_{\rm max} + i\f$, where \f$n_{\rm max}\f$ is the
  maximum number of regions for each of the \f$G_0, G_1, ..., G_{n_B-1}\f$.
- If the particle is inside a CD geometry in \f$G_B\f$ region \f$j\f$, then the
  distance to a boundary is the minimum of the distances to a boundary
  in \f$G_B\f$ and \f$G_j\f$. If the particle first crosses a boundary in
  \f$G_j\f$,
  the new region on the other side of the boundary
  is either \f$j n_{\rm max} + i'\f$, if the new region \f$i'\f$ is still
  inside of
  \f$G_j\f$, or -1 if the particle exits \f$G_j\f$.
  If the particle first crosses
  a boundary in \f$G_B\f$ into a region \f$j'\f$, then the new region is
  either \f$j' n_{\rm max} + i'\f$, if the new position is in region \f$i'\f$
  inside of \f$G_{j'}\f$, or -1, if the new position is outside of \f$G_{j'}\f$
- If a particle is outside of a CD geometry, then the distance to a boundary
  is calculated by following the trajectory until it either enters a
  region \f$j\f$ of \f$G_B\f$ that is already inside of \f$G_j\f$ or enters a
  geometry in one of the regions of \f$G_B\f$.
- When a particle is inside of a CD geometry in \f$G_B\f$ region \f$j\f$, then
  \f$t_\perp\f$ is given by the minimum of \f$t_\perp\f$ from \f$G_B\f$ and from
  \f$G_j\f$
- When a particle is outside of a CD geometry, then \f$t_\perp\f$ is
  given by the \f$t_\perp\f$ of \f$G_B\f$, if the position is outside of
  \f$G_B\f$,
  or by the minimum of the perpendicular distances to \f$G_B\f$ and \f$G_j\f$,
  if the particle is inside of region \f$j\f$ in \f$G_B\f$ (but is obviously
  outside of \f$G_j\f$).

Note that in many cases geometry objects that can be modeled as
CD geometries could also be modeled as unions with appropriately
assigned priorities (see EGS_UnionGeometry). However,
due to the fact that in the CD geometries only checks against a single
geometry in addition to \f$G_B\f$ are required, it is likely that a
CD model will be much more efficient than a union model. For instance,
the geometry shown in
\htmlonly
the first figure
\endhtmlonly
\latexonly
Fig. \ref{fig_cd1}
\endlatexonly
can be modeled as the
union of two sets of spheres and an N-dimensional geometry
made from a set of two cylinders and a set of two planes for the
middle portion by giving the N-dimensional geometry a higher
priority than the sets of spheres.

A CD geometry is defined in the input file using the following key-pair values
\verbatim
library = egs_cdgeometry
base geometry = name of a previously defined geometry
set geometry = region1,  name of a previously defined geometry
set geometry = region2,  name of a previously defined geometry
...
\endverbatim
If the same geometry divides several consecutive regions, one can use
\verbatim
set geometry = start region, stop region, name of a previously defined geometry
\endverbatim
There can be an arbitrary number of <code> set geometry</code> keys.

As concluding remark for the CD geometry type, it is worth noting
that the treatment head of a medical linear accelerator can be
efficiently modeled with the help of a CD geometry. This can
be seen in the \c photon_linac.geom example geometry file.
Many of the other examples also employ a CD-geometry.

A simple example:
\verbatim
:start geometry definition:

    # The base geometry, this will be the Chopping Device (CD)
    # The base geometry can be any geometry, even a composite one
    :start geometry:
        name        = my_cd_planes
        library     = egs_planes
        type        = EGS_Zplanes
        positions   = -3 3 5
        # No media required
    :stop geometry:

    :start geometry:
        name        = my_cd_cylinder
        library     = egs_cylinders
        type        = EGS_ZCylinders
        radii       = 1.6 2
        :start media input:
            media = air water
            set medium = 1 1
        :stop media input:
    :stop geometry:

    :start geometry:
        name        = my_cd_sphere
        library     = egs_spheres
        midpoint = 0 0 3
        radii = 1.6 2
        :start media input:
            media = air water
            set medium = 1 1
        :stop media input:
    :stop geometry:

    # The composite geometry
    :start geometry:
        name            = my_cd
        library         = egs_cdgeometry
        base geometry   = my_cd_planes
        # set geometry = 1 geom means:
        # "in region 1 of the basegeometry, use geometry "geom"
        set geometry   = 0 my_cd_cylinder
        set geometry   = 1 my_cd_sphere
        # The final region numbers are attributed by the cd geometry object;
        # Use the viewer to determine region numbers
    :stop geometry:

    simulation geometry = my_cd

:stop geometry definition:
\endverbatim
\image html egs_cd_geometry.png "A simple example with clipping plane 1,0,0,0"
*/
class EGS_CDGEOMETRY_EXPORT EGS_CDGeometry : public EGS_BaseGeometry {

public:


    EGS_CDGeometry(EGS_BaseGeometry *G1, EGS_BaseGeometry **G,
                   const string &Name = "", int indexing=0) : EGS_BaseGeometry(Name) {
        nmax = 0;
        new_indexing = false;
        reg_to_base = 0;
        local_start = 0;
        nbase = G1->regions();
        g = new EGS_BaseGeometry* [nbase];
        bg = G1; //bg->ref();
        for (int j=0; j<nbase; j++) {
            g[j] = G[j];
            if (g[j]) {
                //g[j]->ref();
                int n = g[j]->regions();
                if (n > nmax) {
                    nmax = n;
                }
            }
        }
        if (!nmax) {
            nmax = 1;
        }
        nreg = nbase*nmax;
        is_convex = false;
        if (indexing) {
            setUpIndexing();
        }
        setHasRhoScaling();
        setHasBScaling();
    };

    EGS_CDGeometry(EGS_BaseGeometry *G1, const vector<EGS_BaseGeometry *> &G,
                   const string &Name = "",int indexing=0) : EGS_BaseGeometry(Name) {
        if (!G1) egsFatal("EGS_CDGeometry: got a null pointer to the"
                              " base geometry?\n");
        nbase = G1->regions();
        new_indexing = false;
        reg_to_base = 0;
        local_start = 0;
        if (nbase != G.size()) egsFatal("EGS_CDGeometry: number of passed"
                                            " geometries (%d) is not the same as the number of regions (%d)\n",
                                            nbase,G.size());
        nmax = 0;
        g = new EGS_BaseGeometry* [nbase];
        bg = G1; //bg->ref();
        for (int j=0; j<nbase; j++) {
            g[j] = G[j];
            if (g[j]) {
                //g[j]->ref();
                int n = g[j]->regions();
                if (n > nmax) {
                    nmax = n;
                }
            }
        }
        if (!nmax) {
            nmax = 1;
        }
        nreg = nbase*nmax;
        is_convex = false;
        if (indexing) {
            setUpIndexing();
        }
        setHasRhoScaling();
        setHasBScaling();
    };


    ~EGS_CDGeometry() {
        for (int j=0; j<nbase; j++) {
            if (g[j]) {
                if (!g[j]->deref()) {
                    delete g[j];
                }
            }
        }
        delete [] g;
        if (!bg->deref()) {
            delete bg;
        }
        if (new_indexing) {
            if (reg_to_base) {
                delete [] reg_to_base;
            }
            if (local_start) {
                delete [] local_start;
            }
        }
    };

    int medium(int ireg) const {
        /*
        int ibase = ireg/nmax;
        if( g[ibase] ) return g[ibase]->medium(ireg-ibase*nmax);
        return bg->medium(ibase);
        */
        int ibase, ilocal;
        if (new_indexing) {
            ibase = reg_to_base[ireg];
            ilocal = ireg - local_start[ibase];
        }
        else {
            ibase = ireg/nmax;
            ilocal = ireg-ibase*nmax;
        }
        return g[ibase] ? g[ibase]->medium(ilocal) : bg->medium(ibase);
    };

    bool isRealRegion(int ireg) const {
        if (ireg < 0 || ireg >= nreg) {
            return false;
        }
        int ibase, icd;
        if (new_indexing) {
            ibase = reg_to_base[ireg];
            icd = ireg-local_start[ibase];
        }
        else {
            ibase = ireg/nmax;
            icd = ireg - ibase*nmax;
        }
        return g[ibase] ? g[ibase]->isRealRegion(icd) :
               bg->isRealRegion(ibase);
    };

    bool isInside(const EGS_Vector &x) {
        int ibase = bg->isWhere(x);
        if (ibase < 0) {
            return false;
        }
        if (g[ibase]) {
            return g[ibase]->isInside(x);
        }
        return true;
    };

    int isWhere(const EGS_Vector &x) {
        int ibase = bg->isWhere(x);
        if (ibase < 0) {
            return ibase;
        }
        int ir = 0;
        if (g[ibase]) {
            ir = g[ibase]->isWhere(x);
            if (ir < 0) {
                return ir;
            }
        }
        return new_indexing ? local_start[ibase] + ir : ibase*nmax + ir;
    };

    int inside(const EGS_Vector &x) {
        return isWhere(x);
    };

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed = 0, EGS_Vector *normal = 0) {
        if (ireg >= 0) {
            //
            // *** We are inside and ibase is the current base geometry
            //     region and icd the local region of the geometry inscribed
            //     in ibase (if any)
            int ibase, icd;
            if (new_indexing) {
                ibase = reg_to_base[ireg];
                icd = ireg-local_start[ibase];
            }
            else {
                ibase = ireg/nmax;
                icd = ireg - ibase*nmax;
            }
            //
            // *** See if we will hit a boundary in the base geometry.
            //     If we do, newmed and normal will get set accordingly.
            //
            int ibase_new = bg->howfar(ibase,x,u,t,newmed,normal);
            if (g[ibase]) {
                //
                // *** There is another geometry in this base geometry region.
                //     See if will hit one of its boundaries first.
                //
                int icd_new = g[ibase]->howfar(icd,x,u,t,newmed,normal);
                if (icd_new < 0) {
                    return icd_new;
                }
                // above: yes we do but the new region is outside
                // => we exit the entire geometry. newmed and normal
                // must have been set accordingly.
                if (icd_new != icd)
                    return new_indexing ? local_start[ibase] + icd_new :
                           ibase*nmax + icd_new;
                // above: yes we do and we enter the local region icd_new
                // => calculate the new region and return
                // newmed and normal must have been set in g[ibase]->howfar
            }
            if (ibase_new != ibase) {
                // we have entered a new base geometry region.
                if (ibase_new < 0) {
                    return ibase_new;
                }
                // but this region is outside => just return.
                if (g[ibase_new]) {
                    // there is a geometry in this new base geometry region.
                    // are we already inside?
                    EGS_Vector tmp(x + u*t);
                    int icd_new = g[ibase_new]->isWhere(tmp);
                    if (icd_new < 0) {
                        return icd_new;
                    }
                    // above: no, we are not => we exit the entire geometry.
                    // below: yes, we are in local region icd_new
                    // simply calculate the new global region and return
                    // newmed and normal have been set in bg->howfar above
                    if (newmed) {
                        *newmed = g[ibase_new]->medium(icd_new);
                    }
                    return new_indexing ? local_start[ibase_new] + icd_new :
                           ibase_new*nmax + icd_new;
                }
                // there is no cd geometry in this base geometry region
                // => simply calculate the new global region and return
                // newmed and normal have been set in bg->howfar above
                return new_indexing ? local_start[ibase_new] : ibase_new*nmax;
            }
            // new region is the same as old region (i.e. we don't reach
            // a boundary => simply return the old region.
            return ireg;
        }

        //
        // *** If here, we are outside.
        //     Are we already inside the base geometry ?
        //
        int ibase = bg->isWhere(x);
        EGS_Float tb=0, ttot=0;
        bool first_time = true;// first time checking base geometry?
        EGS_Vector n;
        int mednew;
        EGS_Vector *pn = normal ? &n : 0;
        int *pmednew = newmed ? &mednew : 0;
        if (ibase >= 0) {
            //
            // IK Feb 22 2007:
            // The following seems flawed. Yes, if there is no inscribed
            // geometry we can assume that ibase>=0 is due to roundoff,
            // but if there is, we might as well be about to enter this
            // geometry (but isInside() thinks that we are already inside).
            // What would be the best way to check? Don't know yet.
            //
            // already inside the base geometry.
            // unfortunately, due to roundoff we may think that
            // we are inside the base geometry but we are actually
            // outside. To make the logic below work, we need to
            // check if we are inside the geometry inscribed in region
            // ibase.
            int icd = g[ibase] ? g[ibase]->isWhere(x) : 0;
            if (icd >= 0) {

                // We think we are outside, but isWhere(x) reports that we are
                // inside. This can be caused by numerical roundoff.
                // There are 2 possibilities:
                //   a) The particle has just left geometry, but due to roundoff
                //      we still find it inside for the next step. This possibility
                //      can be checked by checking the distance to the exit point
                //   b) The particle is approaching the geometry but due to roundoff
                //      we already find it inside. This possibility can be checked
                //      by determining the distance to exit point moving along the
                //      direction oposite to the particle direction.
                //***********************************************************************
                // EMH April 12 2013: Check 0 for (b) below was ambiguosly defined as there
                // will be situations where u and -u will not hit the geometry. An univocally
                // defined test is to check whether particle actually enters the geometry from
                // the boundary or a location near it (round-off).
                // REMEMBER: - isWhere will return ir >= 0 when particle seats at a boundary.
                //           - We are here either due to roundoff or just at the boundary
                //
                // 0. Proper check for a and b)
                int ixold = new_indexing ? local_start[ibase] + icd :
                            ibase*nmax + icd;
                //EGS_Float tb_neg = 1e30; int ixnew_neg = howfar(ixold,x,u*(-1),tb_neg,0,pn);
                //EGS_Float tb_pos = 1e30; int ixnew_pos = howfar(ixold,x,u,tb_pos,0,pn);
                // Call to CD geometry howfar with updated region ixold. If it is aimed away from
                // geometry, it will return ixnew = -1 and assumption a) was correct. If it is
                // aimed into geometry, it will return ixnew >=0 and assumption b) was correct.
                tb = veryFar;
                int ixnew = howfar(ixold,x,u,tb,0,pn);
                // Enters geometry and at a boundary or very close to one.
                //if( ixnew_pos >= 0 && ixnew_neg < 0 && tb_neg <= boundaryTolerance && tb_pos <= boundaryTolerance) {
                if (ixnew >= 0 && tb <= boundaryTolerance) {                             // (b) is true
                    t = 0;
                    if (newmed) *newmed = g[ibase] ? g[ibase]->medium(icd) :
                                              bg->medium(ibase);
                    //if( normal ) *normal = (*pn)*(-1);
                    if (normal) {
                        *normal = *pn;
                    }
                    return ixold;
                }
                // Skip for concave geometries: particles can reenter the CD geometry after leaving.
                // Check 2. below will catch these cases.
                // This follow has been commented out because it fails in cases
                // where the inscribe geometries are themselves convex but
                // form a concave shape. Instead, just continue to do more checks.
//                 else if ((ixnew < 0 && tb <= boundaryTolerance) && (bg->isConvex()  &&
//                          (g[ibase] && g[ibase]->isConvex()))) {
//                     // (a) is true
//                     return ixnew;
//                 }

                // If a particle approaching the geometry sits on a boundary, we look back to see
                // if we just entered the geometry (the previous checks fail to catch this case).
                EGS_Float tb_neg = veryFar;
                int ixnew_neg = howfar(ixold,x,u*(-1),tb_neg,0,pn);
                if (ixnew_neg < 0 && tb_neg <= epsilon) {                             // (b) is true
                    t = 0;
                    if (newmed) {
                        *newmed = g[ibase] ? g[ibase]->medium(icd) : bg->medium(ibase);
                    }
                    if (normal) {
                        *normal = (*pn)*(-1);
                    }
                    return ixold;
                }

                //***********************************************************************

                // 1. Check if we exit the base geometry after a sufficiently small
                // distance.
                EGS_Float t1=veryFar, t2 = veryFar;
                int ibase_n, ic_n=0;
                ibase_n = bg->howfar(ibase,x,u,t1);
                if (ibase_n < 0 && t1 < boundaryTolerance) {
                    // Yes we do => we assume that it was a roundoff problem
                    // and in reality the particle was outside the base geometry.
                    // We then check if it will enter the base geometry.
                    EGS_Vector xtmp(x + u*t1);
                    tb = t-t1;
                    ibase_n = bg->howfar(ibase_n,xtmp,u,tb,pmednew,pn);
                    if (ibase_n < 0) {
                        return ibase_n;    // no, so just return.
                    }
                    // yes, so transport to entry point and follow normal outside logic
                    tb += t1;
                    ttot = tb;
                    ibase = ibase_n;
                    first_time = false;
                    goto do_checks;
                }
                // If here, particle is inside base geometry and also doesn't exit base geometry
                // very soon. So, we must do check 2.:
                // 2. Check if we exit the inscribed geometry after a sufficiently small
                // distance.
                if (g[ibase]) {
                    ic_n = g[ibase]->howfar(icd,x,u,t2);
                    if (ic_n < 0 && t2 < boundaryTolerance) {
                        // Yes we do => we assume that it was a roundoff problem
                        // and in reality the particle was outside the inscribed geometry.
                        // We move to the boundary and check again the base geometry.
                        EGS_Vector xtmp(x + u*t2);
                        ibase = bg->isWhere(xtmp);
                        if (ibase < 0) {
                            tb = t - t2;
                            ibase = bg->howfar(ibase,xtmp,u,tb,pmednew,pn);
                            if (ibase < 0) {
                                return ibase;
                            }
                            tb += t2;
                            ttot = tb;
                            first_time = false;
                        }
                        else {
                            tb = t2;
                            ttot = tb;
                        }
                        goto do_checks;
                    }
                }
                if (t1 < boundaryTolerance && ibase_n >= 0 && g[ibase_n]) {
                    // last resort.
                    EGS_Vector xtmp(x + u*t1);
                    int icdx = g[ibase_n]->isWhere(xtmp);
                    if (icdx < 0) {
                        tb = t1;
                        ttot = tb;
                        ibase = ibase_n;
                        first_time = true;
                        goto do_checks;
                    }
                }
                if (t1 > boundaryTolerance && t2 > boundaryTolerance) {
                    error_flag = 1;
                    egsWarning("EGS_CDGeometry::howfar: ireg<0, but position appears inside\n");
                    egsWarning(" name=%s base name=%s inscribed name=%s\n",name.c_str(),
                               bg->getName().c_str(),g[ibase] ? g[ibase]->getName().c_str() : "none");
                    egsWarning(" x=(%g,%g,%g) u=(%g,%g,%g) ibase=%d icd=%d\n",
                               x.x,x.y,x.z,u.x,u.y,u.z,ibase,icd);
                    egsWarning(" distance to boundaries: base=(%d,%g) inscribed=(%d,%g)\n",
                               ibase_n,t1,ic_n,t2);
                }
            }
            if (icd >= 0) {
                return ireg;
            }
            tb = 0;
            ttot = 0;
        } // yes, we are.
        else {
            // no, we are not. Check if we will enter the base geometry.
            first_time = false;
            tb = t;
            ibase = bg->howfar(ireg,x,u,tb,pmednew,pn);
            if (ibase < 0) {
                return ibase;    // no, we will not.
            }
            ttot = tb; // i.e. we enter the base geometry after a
            // path-length of tb (wich is less than t).
            // if this happens, newmed and normal are set
            // by the bg->howfar call.
        }

do_checks:
        EGS_Vector tmp(x + u*tb);  // position at which we are inside
        // the base geometry
        for (EGS_I64 loopCount=0; loopCount<=loopMax; ++loopCount) {
            if (loopCount == loopMax) {
                egsFatal("EGS_CDGeometry::howfar: Too many iterations were required! Input may be invalid, or consider increasing loopMax.");
                return -1;
            }
            if (!g[ibase]) { // no inscribed geometry in this base geometry region
                t = ttot;
                if (newmed) {
                    *newmed = mednew;
                }
                if (normal) {
                    *normal = n;
                }
                return new_indexing ? local_start[ibase] : ibase*nmax;
            }
            // => we have entered.
            int icd = -1;
            if (!first_time) { // already howfar-checked base geometry
                icd = g[ibase]->isWhere(tmp);
                if (icd >= 0) {
                    // already inside of the geometry of this base geometry
                    // region.
                    t = ttot;
                    if (newmed) {
                        *newmed = g[ibase]->medium(icd);
                    }
                    if (normal) {
                        *normal = n;
                    }
                    return new_indexing ? local_start[ibase] + icd :
                           ibase*nmax + icd;
                }
                first_time = false;
            }

            // see if we will enter a new base geometry region before t
            EGS_Float tnew = t - ttot;
            int ibase_new = bg->howfar(ibase,tmp,u,tnew,pmednew,pn);
            // see if we will enter the cd geometry of this base geometry
            // region.
            int icd_new = g[ibase]->howfar(-1,tmp,u,tnew,pmednew,pn);
            if (icd_new >= 0) {
                // yes, we will.
                t = ttot + tnew;
                if (newmed) {
                    *newmed = mednew;
                }
                if (normal) {
                    *normal = n;
                }
                return new_indexing ? local_start[ibase] + icd_new :
                       ibase*nmax + icd_new;
            }
            // if the base region is the same or we have exited the
            // base geometry, the particle never enters.

            //if( ibase_new == ibase || ibase_new < 0 ) return -1;
            if (ibase_new == ibase) {
                return -1;
            }
            if (ibase_new < 0) {
                tmp += u*tnew;
                ttot += tnew;
                tnew = t - ttot;
                ibase_new = bg->howfar(-1,tmp,u,tnew,pmednew,pn);
                if (ibase_new < 0) {
                    return -1;
                }
                //tb = tnew; ttot += tnew; first_time = false;
                //goto do_checks;
            }
            // OK, we are in a new base geometry now, adjust
            // position and path-length so-far and retry.
            if (tnew < boundaryTolerance) {
                tnew = boundaryTolerance;
            }
            tmp += u*tnew;
            ttot += tnew;
            ibase = ibase_new;
            first_time = false;
        }

        return ireg;
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        if (ireg >= 0) {
            int ibase, icd;
            if (new_indexing) {
                ibase = reg_to_base[ireg];
                icd = ireg - local_start[ibase];
            }
            else {
                ibase = ireg/nmax;
                icd = ireg - ibase*nmax;
            }
            EGS_Float t = bg->hownear(ibase,x);
            if (!g[ibase] || t <= 0) {
                return t;
            }
            EGS_Float t1 = g[ibase]->hownear(icd,x);
            return (t1 < t ? t1 : t);
        }
        int ibase = bg->isWhere(x);
        if (ibase < 0) {
            EGS_Float tt = bg->hownear(ireg,x);
            return tt;
        }
        EGS_Float t = bg->hownear(ibase,x);
        if (g[ibase]) {
            EGS_Float t1 = g[ibase]->hownear(-1,x);
            if (t1 < t) {
                t = t1;
            }
        }
        return t;
    };

    int getMaxStep() const {
        int nstep = 0;
        for (int j=0; j<bg->regions(); ++j) {
            if (g[j]) {
                nstep += g[j]->getMaxStep();
            }
            else {
                ++nstep;
            }
        }
        return nstep + 1;
    }

    bool hasBooleanProperty(int ireg, EGS_BPType prop) const {
        if (ireg >= 0 && ireg < nreg) {
            int ibase = ireg/nmax;
            return g[ibase] ?
                   g[ibase]->hasBooleanProperty(ireg - ibase*nmax,prop) :
                   bg->hasBooleanProperty(ibase,prop);
        }
        return false;
    };
    void setBooleanProperty(EGS_BPType) {
        setPropertError("setBooleanProperty()");
    };
    void addBooleanProperty(int) {
        setPropertError("addBooleanProperty()");
    };
    void setBooleanProperty(EGS_BPType,int,int,int step=1) {
        setPropertError("setBooleanProperty()");
    };
    void addBooleanProperty(int,int,int,int step=1) {
        setPropertError("addBooleanProperty()");
    };

    const string &getType() const {
        return type;
    };

    void  setRelativeRho(int start, int end, EGS_Float rho);
    void  setRelativeRho(EGS_Input *);
    EGS_Float getRelativeRho(int ireg) const {
        if (ireg < 0 || ireg >= nbase*nmax) {
            return 1;
        }
        int ibase = ireg/nmax;
        return g[ibase] ? g[ibase]->getRelativeRho(ireg-ibase*nmax) :
               bg->getRelativeRho(ibase);
    };

    void  setBScaling(int start, int end, EGS_Float bf);
    void  setBScaling(EGS_Input *);
    EGS_Float getBScaling(int ireg) const {
        if (ireg < 0 || ireg >= nbase*nmax) {
            return 1;
        }
        int ibase = ireg/nmax;
        return g[ibase] ? g[ibase]->getBScaling(ireg-ibase*nmax) :
               bg->getBScaling(ibase);
    };

    virtual void getLabelRegions(const string &str, vector<int> &regs);

protected:

    EGS_BaseGeometry *bg;
    EGS_BaseGeometry **g;
    int              nbase, nmax;
    static string    type;

    /* New indexing style:

       If the base geometry has \f$N_B\f$ regions and the inscribed
       geometries \f$G_j\f$ have \f$n_j\f$ regions, then the CD geometry
       will have \f$n_1 + n_2 + \cdots n_{N_B}\f$ regions
       (if there is no inscribed geometry is some region, then
       \f$n_j = 1\f$). Then, for a CD geometry region \f$i\f$
       the base region \f$i_B\f$ is given by reg_to_base[i] and
       the local region in the inscribed geometry (if any) is
       \f$i\f$ - local_start[\f$i_B\f$].
     */

    /*! If true, use new indexing style */
    bool             new_indexing;
    /*! If new indexing style is used, converts global region to
        base region */
    int              *reg_to_base;
    /*! If new indexing style is used, local_start[ibase] is the first
        region in base region ibase */
    int              *local_start;

    void setMedia(EGS_Input *inp, int, const int *);

private:

    void setPropertError(const char *funcname) {
        egsFatal("EGS_CDGeometry::%s: don't use this method\n  Define "
                 "properties in the constituent geometries instead\n");
    };

    void setHasRhoScaling() {
        has_rho_scaling = false;
        if (bg->hasRhoScaling()) {
            has_rho_scaling = true;
            return;
        }
        for (int j=0; j<nbase; j++) {
            if (g[j]) {
                if (g[j]->hasRhoScaling()) {
                    has_rho_scaling = true;
                    return;
                }
            }
        }
    };

    void setHasBScaling() {
        has_B_scaling = false;
        if (bg->hasBScaling()) {
            has_B_scaling = true;
            return;
        }
        for (int j=0; j<nbase; j++) {
            if (g[j]) {
                if (g[j]->hasBScaling()) {
                    has_B_scaling = true;
                    return;
                }
            }
        }
    };

    void setUpIndexing();
};

#endif
