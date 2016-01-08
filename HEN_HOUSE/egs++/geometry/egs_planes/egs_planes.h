/*
###############################################################################
#
#  EGSnrc egs++ planes geometry headers
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
#  Contributors:
#
###############################################################################
*/


/*! \file egs_planes.h
 *  \brief Sets of parallel planes and a plane collection
 *  \IK
 */

#ifndef EGS_PLANES_
#define EGS_PLANES_

#include "egs_base_geometry.h"
#include "egs_functions.h"
#include "egs_projectors.h"

#include <vector>
using std::vector;

#ifdef WIN32

    #ifdef BUILD_PLANES_DLL
        #define EGS_PLANES_EXPORT __declspec(dllexport)
    #else
        #define EGS_PLANES_EXPORT __declspec(dllimport)
    #endif
    #define EGS_PLANES_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_PLANES_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_PLANES_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_PLANES_EXPORT
        #define EGS_PLANES_LOCAL
    #endif

#endif

/*! \brief A set of parallel planes.

  \ingroup Geometry
  \ingroup ElementaryG

This template class implements the geometry methods for a set of one or
more parallel planes, which divide space into 1 or more regions.
The reason for using a template to generate separate classes for
x-, y- and z-planes and planes in any direction is efficiency: to
calculate the distance or the minimum perpendicular distance to a plane
one requires the scalar products \f$\vec{x} \cdot \vec{a}\f$ and
\f$\vec{u} \cdot \vec{a}\f$
where \f$\vec{a}\f$ is the plane normal and \f$\vec{x}\f$ and \f$\vec{u}\f$
are particle
position and direction. In the general case (plane normal in any direction)
each of these products require 5 floating point operations. In the special
case of x-, y- and z-planes they are calculated with a single assignment.
The calculation of the scalar products is done by a projector object in
the template class. In the case of a projector onto the x-, y- or z-planes
this is a single assignement.

The additional key-value pairs needed to construct a set of planes are
\verbatim
library = egs_planes
type = EGS_Xplanes or EGS_Yplanes or EGS_Zplanes or EGS_Planes
positions = list of plane co-ordinates
normal    = ax, ay, az (only needed for type = EGS_Planes)
\endverbatim
The plane positions \f$p_i\f$ are defined from
\f$ \vec{a} \cdot \vec{x} = p_i\f$, where \f$\vec{x}\f$ is a position on
the \f$i\f$'th plane. An alternative definition of the plane position is
to use
\verbatim
library = egs_planes
first plane = position of the first plane
slab thickness = dx1, dx2, ..., dxn
number of slabs = n1, n2, ..., nn
\endverbatim
which defines the set of planes to start at the position specified
by the <code>first plane</code> key, followed by \c n1 planes that
are \c dx1 apart, \a n2 planes that are \c dx2 apart, etc.
This is more convenient for defininig a regular grid with a large number
of planes.

It is worth noting that a set of \f$N\f$ planes defines \f$N-1\f$ regions,
except for \f$N=1\f$, where a single region is defined (the side of the plane
that the plane normal points to).

Parallel planes are used in almost all of the
\ref example_geometries "example geometries" provided
with the distribution.

*/

template <class T>
class EGS_PLANES_EXPORT EGS_PlanesT : public EGS_BaseGeometry {

protected:

    EGS_Float  *p;      //!< Plane positions.
    EGS_Float  p_last;  //!< The last plane
    EGS_Float  dp;
    int        n_plane; //!< Number of planes - 1
    bool       is_uniform;
    T          a;       //!< The projection operator.
    //EGS_Vector last_x, last_u;
    //EGS_Float  last_d;
    //int        last_ir, last_irnew;

    void       checkIfUniform() {
        is_uniform = true;
        if (nreg == 1) {
            dp = p_last - p[0];
            return;
        }
        dp = p[1] - p[0];
        for (int j=1; j<nreg; j++) {
            EGS_Float dpj = p[j+1] - p[j];
            if (fabs(dpj/dp-1) > 2e-5) {
                is_uniform = false;
                break;
            }
        }
    };

public:

    /*! Destructor. */
    ~EGS_PlanesT() {
        if (nreg) {
            delete [] p;
        }
    };

    /*! \brief Construct a parallel plane set with \a np planes at
      positions \a pos.
     */
    EGS_PlanesT(int np, const EGS_Float *pos, const string &Name,
                const T &A) : EGS_BaseGeometry(Name), a(A) {
        if (np > 0) {
            p = new EGS_Float [np];
            for (int j=0; j<np; j++) {
                p[j] = pos[j]/a.length();
                if (j > 0) {
                    if (p[j] < p[j-1]) egsFatal("EGS_PlanesT::EGS_PlanesT: "
                                                    " plane positions must be in increasing order\n");
                }
            }
            if (np > 1) {
                nreg = np-1;
                p_last = p[nreg];
                n_plane = np-1;
            }
            else {
                nreg = 1;
                p_last = 1e15;
                n_plane = 0;
            }
        }
        else egsFatal("EGS_PlanesT::EGS_PlanesT: attempt to construct a "
                          "plane set with %d plane positions\n",np);
        checkIfUniform();
    };
    /*! \brief Construct a parallel plane set from the positions given by
               \a pos
     */
    EGS_PlanesT(const vector<EGS_Float> &pos, const string &Name, const T &A) :
        EGS_BaseGeometry(Name), a(A) {
        int np = pos.size();
        if (np > 0) {
            p = new EGS_Float [np];
            for (int j=0; j<np; j++) {
                p[j] = pos[j]/a.length();
                if (j > 0) {
                    if (p[j] < p[j-1]) egsFatal("EGS_PlanesT::EGS_PlanesT: "
                                                    " plane positions must be in increasing order\n");
                }
            }
            if (np > 1) {
                nreg = np-1;
                p_last = p[nreg];
                n_plane = np-1;
            }
            else {
                nreg = 1;
                p_last = 1e15;
                n_plane = 0;
            }
        }
        else egsFatal("EGS_PlanesT::EGS_PlanesT: attempt to construct a "
                          "plane set with %d plane positions\n",np);
        checkIfUniform();
    };
    /*! \brief Construct a parallel plane set starting at \a xo with
               uniform distance between the \a np+1 planes given by \a dx.
     */
    EGS_PlanesT(EGS_Float xo, EGS_Float dx, int np, const string &Name,
                const T &A) : EGS_BaseGeometry(Name), a(A) {
        if (np < 1) egsFatal("EGS_PlanesT::EGS_PlanesT: attempt to construct"
                                 " a plane set with %d plane positions\n",np);
        if (dx <= 0) egsFatal("EGS_PlanesT::EGS_PlanesT: attempt to construct"
                                  " a plane set with a non-positive slice thickness %g\n",dx);
        p = new EGS_Float [np+1];
        p[0] = xo;
        for (int j=0; j<np; j++) {
            p[j+1] = p[j] + dx;
        }
        nreg = np;
        p_last = p[nreg];
        n_plane = np;
        is_uniform = true;
        dp = dx;
    };

    EGS_Float *getPositions() {
        return p;
    };

    /*! Implements the \c %isInside() method for a set of parallel planes

    */
    bool isInside(const EGS_Vector &x) {
        EGS_Float xp = a*x;
        if (xp < p[0] || xp > p_last) {
            return false;
        }
        return true;
    };

    int isWhere(const EGS_Vector &x) {
        EGS_Float xp = a*x;
        if (xp < p[0] || xp > p_last) {
            return -1;
        }
        if (nreg == 1) {
            return 0;
        }
        //if( is_uniform ) { int res = (int) ((xp-p[0])/dp); return res; }
        return findRegion(xp,nreg,p);
    };

    int inside(const EGS_Vector &x) {
        return isWhere(x);
    };

    EGS_Float howfarToOutside(int ireg, const EGS_Vector &x,
                              const EGS_Vector &u) {
        if (ireg < 0) {
            return 0;
        }
        double xp = a*x, up = a*u;
        EGS_Float t = 1e30;
        if (up > 0) {
            t = (p_last-xp)/up;
        }
        else if (up < 0) {
            t = (p[0]-xp)/up;
        }
        return t;
    };

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed=0, EGS_Vector *normal=0) {
        //EGS_Float d = 1e30; int res=ireg;
        //EGS_Float xp = a*x, up = a*u;
        double d = 1e35;
        int res=ireg;
        double xp = a*x, up = a*u;
        if (ireg >= 0) {
            int dir = 0;
            bool warn=false;
            EGS_Float dist;
            if (up > 0 && ireg < n_plane) {
                d = (p[ireg+1]-xp)/up;
                //if( xp <= p[ireg+1] ) d = (p[ireg+1]-xp)/up;
                //else {
                //    dist = xp - p[ireg+1]; d = 0;
                //    if( dist > 2e-5 ) warn = true;
                //}
                dir = 1;
            }
            else if (up < 0) {
                d = (p[ireg]-xp)/up;
                //if( xp >= p[ireg] ) d = (p[ireg]-xp)/up;
                //else {
                //    dist = p[ireg] - xp; d = 0;
                //    if( dist > 2e-5 ) warn = true;
                //}
                dir = -1;
            }
            /*
            if( warn ) {
                egsWarning("In EGS_Planes::howfar(%s): particle is not"
                            " in region we think it is:\n",a.getType().c_str());
                //egsWarning("x=(%g,%g,%g) region=%d distance to plane=%g\n",
                //        x.x,x.y,x.z,ireg,dist);
                egsWarning("x=%g u=%g region=%d distance to plane=%g\n",
                        xp,up,ireg,dist);
            }
            */
            if (d > t) {
                res = ireg;
            }
            else {
                t = d;
                res = ireg + dir;
                if (res >= nreg) {
                    res = -1;
                }
                if (newmed) {
                    if (res >= 0) {
                        *newmed = medium(res);
                    }
                    else {
                        *newmed=-1;
                    }
                }
                if (normal) {
                    *normal = a.normal()*(-dir);
                }
            }
            return res;
        }
        if (xp <= p[0] && up > 0) {
            d = (p[0] - xp)/up;
            res = 0;
        }
        else if (xp >= p_last && up < 0) {
            d = (p_last - xp)/up;
            res = nreg-1;
        }
        else if (xp > p[0] && xp < p_last) {
            // we think we are outside but we are inside.
            // hopefully a truncation problem with a particle
            // backscattered at a boundary
            if (xp-p[0] < 3e-5 && up > 0) {
                d = 0;
                res = 0;
            }
            else if (p_last-xp < 3e-5 && up < 0) {
                d = 0;
                res = nreg-1;
            }
        }
        if (d <= t) {
            t = d;
            if (newmed) {
                *newmed = medium(res);
            }
            if (normal) if (up > 0) {
                    *normal = a.normal()*(-1.);
                }
                else {
                    *normal = a.normal();
                }
        }
        else {
            res = -1;
        }
        return res;
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        EGS_Float xp = a*x;
        if (ireg >= 0) {
            EGS_Float t = xp - p[ireg];
            if (ireg+1 <= n_plane) {
                EGS_Float t2 = p[ireg+1] - xp;
                if (t2 < t) {
                    t = t2;
                }
            }
            return t;
        }
        if (xp <= p[0]) {
            return p[0] - xp;
        }
        if (xp >= p_last) {
            return xp - p_last;
        }
        return 0; // this should not happen.
    };

    const string &getType() const {
        return a.getType();
    };

    void printInfo() const {
        EGS_BaseGeometry::printInfo();
        a.printInfo();
        if (is_uniform)
            egsInformation(" first plane at %g, %d slices of %g thickness\n",
                           p[0],nreg,dp);
        else {
            egsInformation(" plane positions = ");
            for (int j=0; j<nreg; j++) {
                egsInformation("%g ",p[j]);
            }
            if (n_plane == nreg) {
                egsInformation("%g ",p[n_plane]);
            }
            egsInformation("\n");
        }
        egsInformation(
            "=======================================================\n");
    };

    EGS_Float position(int j) const {
        return p[j];
    };

    EGS_Vector normal() const {
        return a.normal();
    };

};

typedef EGS_PlanesT<EGS_XProjector> EGS_PlanesX;
typedef EGS_PlanesT<EGS_YProjector> EGS_PlanesY;
typedef EGS_PlanesT<EGS_ZProjector> EGS_PlanesZ;
typedef EGS_PlanesT<EGS_Projector> EGS_Planes;

/*! \brief A collection of non-parallel planes.

\ingroup Geometry
\ingroup ElementaryG

This class implements the geometry methods
of a set of non-parallel planes. A position is considered to be
in the \f$j\f$'th region of a plane collection if it is
inside the \f$j\f$'th plane (\em i.e. the point is on that side
of the plane to which the plane normal points to) but outside the
\f$j+1\f$'st plane.
Note that a set of non-parallel planes
does not form a well defined set of regions in general.
The EGS_PlaneCollection class
is therefore only meant to be used for construction of composite geometries
where other geometrical structures restrict the extent of the geometry
to the portion of space where the non-parallel planes do not intersect
and therefore the regions formed by the plane collection are well defined.

The additional keys needed to define a plane collection are
\verbatim
library = egs_planes
type = EGS_PlaneCollection
normals = ax1,ay1,az1  ax2,ay2,az2, ...
positions = p1 p2 ...
\endverbatim
where there are \f$3 N\f$ floating number inputs for the \c normals key and
\f$N\f$ floating number inputs for the \c positions key needed to define
a collection with \f$N\f$ planes defining \f$N-1\f$ regions.

An example of the use of a plane collection is found in
the \ref linac_geom "photon_linac.geom" example geometry file. See also
\ref example_geometries
*/
class EGS_PLANES_EXPORT EGS_PlaneCollection : public EGS_BaseGeometry {

protected:

    EGS_Planes  **planes;  // the planes.
    int         np;        // number of planes.
    static string type;

public:

    EGS_PlaneCollection(int Np, const EGS_Float *pos, const EGS_Vector *norm,
                        const string &Name = "");
    ~EGS_PlaneCollection();
    bool isInside(const EGS_Vector &x) {
        return (planes[0]->isInside(x) && !planes[np-1]->isInside(x));
    };
    int isWhere(const EGS_Vector &x) {
        if (!planes[0]->isInside(x)) {
            return -1;
        }
        if (planes[np-1]->isInside(x)) {
            return -1;
        }
        for (int j=0; j<np-1; j++)
            if (planes[j]->isInside(x) && !planes[j+1]->isInside(x)) {
                return j;
            }
        return -1; // this should not happen.
    };
    int inside(const EGS_Vector &x) {
        return isInside(x);
    };
    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed=0, EGS_Vector *normal=0) {
        if (ireg >= 0) {
            int m1, m2;
            EGS_Vector n1, n2;
            EGS_Float t1 = t;
            int i1=planes[ireg]->howfar(0,x,u,t1,&m1,&n1);
            EGS_Float t2 = t;
            int i2=planes[ireg+1]->howfar(-1,x,u,t2,&m2,&n2);
            int res = ireg;
            if (i1 == -1 && i2 == 0) {
                if (t1 < t2) {
                    t = t1;
                    res = ireg-1;
                    if (normal) {
                        *normal = n1;
                    }
                }
                else {
                    t = t2;
                    res = ireg+1;
                    if (res > nreg-1) {
                        res = -1;
                    }
                    if (normal) {
                        *normal = n2;
                    }
                }
            }
            else if (i1 == -1) {
                t = t1;
                res = ireg-1;
                if (normal) {
                    *normal = n1;
                }
            }
            else if (i2 == 0) {
                t = t2;
                res=ireg+1;
                if (res > nreg-1) {
                    res = -1;
                }
                if (normal) {
                    *normal = n2;
                }
            }
            if (newmed && res != ireg) {
                if (res >= 0) {
                    *newmed = medium(res);
                }
                else {
                    *newmed = -1;
                }
            }
            return res;
        }
        if (!planes[0]->isInside(x)) {
            int iaux = planes[0]->howfar(-1,x,u,t,newmed,normal);
            return iaux;
        }
        int i1 = planes[nreg]->howfar(0,x,u,t,newmed,normal);
        if (i1 < 0) {
            return nreg-1;
        }
        return -1;
    };
    EGS_Float hownear(int ireg,const EGS_Vector &x) {
        if (ireg >= 0) {
            EGS_Float t1 = planes[ireg]->hownear(0,x);
            EGS_Float t2 = planes[ireg+1]->hownear(-1,x);
            if (t1 < t2) {
                return t1;
            }
            else {
                return t2;
            }
        }
        if (!planes[0]->isInside(x)) {
            return planes[0]->hownear(-1,x);
        }
        return planes[np-1]->hownear(0,x);
    };
    const string &getType() const {
        return type;
    };

    void printInfo() const;

};

#endif
