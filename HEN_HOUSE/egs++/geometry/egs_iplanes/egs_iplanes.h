/*
###############################################################################
#
#  EGSnrc egs++ iplanes geometry headers
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


/*! \file egs_iplanes.h
 *  \brief Intersecting planes: header
 *  \IK
 */

#ifndef EGS_IPLANES_
#define EGS_IPLANES_

#include "egs_base_geometry.h"
#include "egs_transformations.h"

#ifdef WIN32

    #ifdef BUILD_IPLANES_DLL
        #define EGS_IPLANES_EXPORT __declspec(dllexport)
    #else
        #define EGS_IPLANES_EXPORT __declspec(dllimport)
    #endif
    #define EGS_IPLANES_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_IPLANES_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_IPLANES_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_IPLANES_EXPORT
        #define EGS_IPLANES_LOCAL
    #endif

#endif

/*! \brief A set of planes intersecting in the same axis.

\ingroup Geometry
\ingroup ElementaryG

The EGS_IPlanes class implements the geometry methods for
a set of \f$N\f$ planes intersecting in the same axis that divides the
entire space into \f$2 N\f$ regions (\em i.e. any point in space is
inside an EGS_IPlanes geometry). This type of geometry is
useful, for instance, for defining azimuthal segments in
cylinders as a component of a composite geometry. It is defined
using the following set of keys:
\verbatim
library = egs_iplanes
axis = Px, Py, Pz, ax, ay, az
angles = list of angles of rotation around the axis for the planes in degrees
or
angles in radian = same as above but in radians
\endverbatim
Note that the \c axis key requires 6 inputs: the three coordinates of
a point on the axis and 3 direction cosines defining the axis direction.
Also note that angles must be given in increasing order and that they
must be between 0 and 180 degrees.

An example demonstrating the use of I-planes is found in rz_phi.geom.
*/
class EGS_IPLANES_EXPORT EGS_IPlanes : public EGS_BaseGeometry {

public:

    /*! \brief Construct a set of intersecting planes (iplanes)

    The common axis is defined by the direction \a A and point on the
    axis \a Xo, with \a np denoting the number of angles in the array
    \a angles. If \a degree is \c true, all angles are assumed to be in degree,
    otherwise all angles are considered to be in radian.
    */
    EGS_IPlanes(const EGS_Vector &Xo, const EGS_Vector &A, int np,
                const EGS_Float *angles, const string &Name = "",
                bool degree=true);

    /*! \brief Construct a set of intersecting planes (iplanes)

    The common axis is defined by the direction \a A and point on the
    axis \a Xo. The arrays \a aj and \a dj are of size \a np
    and define the \a np planes directly via the equation
    aj[j]*x = dj[j] for a position x.
    */
    EGS_IPlanes(const EGS_Vector &Xo, const EGS_Vector &A, int np,
                const EGS_Vector *aj, const EGS_Float *dj,
                const string &Name = "");

    /*! \brief Construct a set of intersecting planes (iplanes)

    The common axis is defined by the direction \a A and point on the
    axis \a Xo. It is assumed that space is divided uniformely into
    \a np segments, with \a first defining the angle of the first plane
    in radians.
    */
    EGS_IPlanes(const EGS_Vector &Xo, const EGS_Vector &A, int np,
                EGS_Float first=0, const string &Name = "");

    ~EGS_IPlanes();

    bool isInside(const EGS_Vector &x) {
        return true;
    };
    int isWhere(const EGS_Vector &x);

    int inside(const EGS_Vector &x);

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed = 0, EGS_Vector *normal = 0);

    EGS_Float hownear(int ireg, const EGS_Vector &x);

    const string &getType() const {
        return type;
    };

    void printInfo() const;

    EGS_Vector getAxisXo() const {
        return xo;
    };
    EGS_Vector getAxisDirection() const {
        return axis;
    };
    EGS_Vector getPlaneNormal(int j) const {
        return a[j];
    };
    EGS_Float  getPlanePosition(int j) const {
        return d[j];
    };

private:

    EGS_Vector  xo;               //!< a point on the common axis
    EGS_Vector  axis;             //!< the common axis direction.
    EGS_Vector  *a;               //!< the plane normals
    EGS_Float   *d;               //!< the plane positions
    static string type;           //!< the geometry type
};

/*! \brief A radial geometry replicator

\ingroup Geometry
\ingroup CompositeG

The main reason for the introduction of the EGS_RadialRepeater geometry is
convenience when defining a rotationally repetitive structure.
A EGS_RadialRepeater is a geometry where a base geometry is repeated
\f$N\f$ times radially around an axis specified by the user.
It is defined using
\verbatim
:start geometry:
    library = egs_iplanes
    type = EGS_RadialRepeater
    name = some_name
    axis = 0 0 0    0 0 1   # axis of rotation
    repeated geometry = some_geometry_name
    number of repetitions = N
    medium = medium_name
    first angle = phi_o
\endverbatim
The first 3 inputs in the axis keyword specify a point through
which the axis passes, the other 3 the axis direction (does not need to
be normalized to unity). The <code>repeated geometry</code> must specify
the name of an existing geometry, which can be of any type as long as
it fits completely within 1 radial segment after dividing space into
\f$N\f$ equally large segments defined by \f$N\f$ planes intersecting
in the specified axis (the EGS_RadialRepeater class uses internally
an EGS_Iplanes geometry). Note that an EGS_RadialRepeater geometry is infinite,
so one should inscribe it into a finite size geometry using a CD geometry
for viewing  with <code>egs_view</code>.
The <code>medium</code> input specifies the medium with which the
space outside of the replicated geometry is filled. It is optional and,
if missing, vacuum is assumed.
The \f$i\f$'th copy of the repeated geometry is rotated by
\f$ i \Delta \phi, ~\Delta \phi = 2 \pi/N\f$
around the rotation axis (\f$i\f$ goes from 0 to
\f$N-1\f$). The \f$i\f$'th copy must then fit within
the \f$i\f$'th radial segment of the i-planes, which is between the
planes with normals rotated by \f$\phi_0 + i \Delta \phi\f$ and
\f$\phi_0 + (i+1) \Delta \phi\f$ around the axis or rotation and
intersecting in the axis of rotation. This can be achieved by
setting \f$\phi_o\f$ appropriately using the <code>first angle</code>
input (this input is optional and, if missing, \f$\phi_0 = 0\f$ is assumed).
Obviously the proper choice of \f$\phi_o\f$ will depend on the location
of the geometry to be replicated and on the rotation axis.
One way to determine \f$\phi_o\f$ is to select a point inside the
geometry to be replicated and to follow the transformations beeing made
with particle positions and directions in the EGS_RadialRepeater geometry
method. Another, perhaps easier, approach is to simply experiment
with the value of \f$\phi_0\f$ until the replicated geometry looks as
expected in the geometry viewer (\f$\phi_0\f$ is typically a multiple of
\f$\Delta \phi\f$, unless you want to apply an extra rotation to the
geometry to be replicated).

Examples of the use of an EGS_RadialRepeater geometry can be found
in <code>radial_repetitions.geom, radial_repetitions1.geom</code> and
<code>gear.geom</code> example input files.
*/

class EGS_IPLANES_EXPORT EGS_RadialRepeater : public EGS_BaseGeometry {

public:

    EGS_RadialRepeater(const EGS_Vector &Xo, const EGS_Vector &A, int np,
                       EGS_BaseGeometry *G, EGS_Float first=0, const string &Name = "");

    ~EGS_RadialRepeater();

    bool isInside(const EGS_Vector &x) {
        return true;
    };

    int isWhere(const EGS_Vector &x) {
        int ir = iplanes->isWhere(x);
        //EGS_Vector xp = x*R[ir];
        EGS_Vector xp = (x-xo)*R[ir];
        int il = g->isWhere(xp);
        return il >= 0 ? ir*ng + il : nreg-1;
    };

    int medium(int ireg) const {
        if (ireg < 0 || ireg >= nreg) {
            return -1;
        }
        if (ireg == nreg-1) {
            return med;
        }
        int ir = ireg/ng;
        int il = ireg - ir*ng;
        return g->medium(il);
    };

    int inside(const EGS_Vector &x) {
        return isWhere(x);
    };

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed = 0, EGS_Vector *normal = 0) {
        if (ireg < 0) {
            egsFatal("\nEGS_RadialRepeater::howfar: position can not"
                     " be outside\n");
            return ireg;
        }
        //egsWarning("\nhowfar(ir=%d x=(%g,%g,%g) u=(%g,%g,%g) t=%g)\n",
        //        ireg,x.x,x.y,x.z,u.x,u.y,u.z,t);
        if (ireg < nreg-1) {  // in the repeated geometry
            int ir = ireg/ng;
            int il = ireg - ir*ng;
            //EGS_Vector xp = x*R[ir], up = u*R[ir];
            EGS_Vector xp = (x-xo)*R[ir], up = u*R[ir];
            //egsWarning("In repetition %d: xp=(%g,%g,%g) up=(%g,%g,%g)\n",
            //      ir,xp.x,xp.y,xp.z,up.x,up.y,up.z);
            int inew = g->howfar(il,xp,up,t,newmed,normal);
            //egsWarning("il=%d inew=%d t=%d\n",il,inew,t);
            if (inew < 0) {
                if (normal) {
                    *normal = R[ir]*(*normal);
                }
                inew = nreg-1;
                if (newmed) {
                    *newmed = med;
                }
            }
            else {
                inew += ir*ng;
            }
            return inew;
        }
        // outside of the replicas
        int ir = iplanes->isWhere(x);
        //egsWarning("outside repetions, ir=%d\n",ir);
        EGS_Float t_left = t;
        EGS_Vector xtmp(x);
        EGS_Float ttot = 0;
        //EGS_Vector tmp_n;
        //EGS_Vector *norm = normal ? &tmp_n : 0;
        while (1) {
            EGS_Float this_t = t_left;
            //EGS_Vector xp = xtmp*R[ir], up = u*R[ir];
            EGS_Vector xp = (xtmp-xo)*R[ir], up = u*R[ir];
            //egsWarning("xtmp=(%g,%g,%g)\n",xtmp.x,xtmp.y,xtmp.z);
            //egsWarning("xp=(%g,%g,%g) up=(%g,%g,%g)\n",xp.x,xp.y,xp.z,up.x,up.y,up.z);
            int inew = g->howfar(-1,xp,up,this_t,newmed,normal);
            //egsWarning("inew=%d t=%g\n",inew,this_t);
            if (inew >= 0) {
                t = ttot + this_t;
                if (normal) {
                    *normal = R[ir]*(*normal);
                }
                return ir*ng + inew;
            }
            int next_ir = iplanes->howfar(ir,xtmp,u,this_t,0,0);
            //egsWarning("next sector: %d t=%g\n",next_ir,this_t);
            if (next_ir == ir) {
                return ireg;
            }
            ttot += this_t;
            t_left -= this_t;
            xtmp += u*this_t;
            ir = next_ir;
        }
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        if (ireg < 0) {
            egsFatal("EGS_RadialRepeater::hownear: position can not"
                     " be outside\n");
            return 0;
        }
        if (ireg < nreg-1) {  // in the repeated geometry
            int ir = ireg/ng;
            int il = ireg - ir*ng;
            EGS_Vector xp = x*R[ir];
            return g->hownear(il,xp);
        }
        // outside of the replicas
        int ir = iplanes->isWhere(x);
        EGS_Vector xp = x*R[ir];
        return g->hownear(-1,xp);
    };

    int getMaxStep() const {
        return nrep*(g->getMaxStep() + 1);
    };

    const string &getType() const {
        return type;
    };

    void printInfo() const;

    void setRLabels(EGS_Input *input);
    virtual void getLabelRegions(const string &str, vector<int> &regs);

protected:

    EGS_IPlanes        *iplanes;
    EGS_BaseGeometry   *g;
    EGS_RotationMatrix *R;

    EGS_Vector         xo;

    int                nrep, ng;
    EGS_Float          phi_o;

    static string      type;


};


#endif
