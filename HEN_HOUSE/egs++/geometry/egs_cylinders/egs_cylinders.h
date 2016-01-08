/*
###############################################################################
#
#  EGSnrc egs++ cylinders geometry headers
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
#
###############################################################################
*/


/*! \file egs_cylinders.h
 *  \brief A set of concentric cylinders: header
 *  \author Declan Persram and Iwan Kawrakow
 */

#ifndef EGS_CYLINDERS_
#define EGS_CYLINDERS_

#include "egs_base_geometry.h"
#include "egs_math.h"
#include "egs_functions.h"

#include <vector>
using namespace std;

#ifdef WIN32

    #ifdef BUILD_CYLINDERS_DLL
        #define EGS_CYLINDERS_EXPORT __declspec(dllexport)
    #else
        #define EGS_CYLINDERS_EXPORT __declspec(dllimport)
    #endif
    #define EGS_CYLINDERS_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_CYLINDERS_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_CYLINDERS_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_CYLINDERS_EXPORT
        #define EGS_CYLINDERS_LOCAL
    #endif

#endif

/*! \brief A set of concentric cylinders

  \ingroup Geometry
  \ingroup ElementaryG

This template class implements the geometry methods for a set of
\f$N\f$ concentric
cylinders that define a geometry with \f$N\f$ regions with
specializations for a cylinders along the x-, y-, z- and any axis.
Similar comments as in the case of
\link EGS_PlanesT planes \endlink
apply for the motivation for having 4 different cylinder classes.
Note that in all cases the cylinder axis passes through the origin.
Sets of cylinders at another location can be obtained by using a
\link EGS_TransformedGeometry
transformed geometry. \endlink The convention
for region numbering is that the region inside the inner-most cylinder
is region 0 with region index increasing towards the last cylinder in the
set. Note also that the geometry defined by a set of cylinders is unlimited
in the direction along the cylinder axis.

The additional keys needed to construct a set of cylinders are
\verbatim

library = egs_cylinders
type = EGS_XCylinders or EGS_YCylinders or EGS_ZCylinders or EGS_Cylinders
radii = list of cylinder radii
axis  = ax, ay, az
midpoint = Ox, Oy, Oz

\endverbatim
The \c axis key is only needed for an object of type EGS_Cylinders
(and in fact is ignored for x-, y- or z-cylinders). The list of radii must
be given in increasing order. The components of the axis vector don't
need to be normalized to unity.

Examples of the usage of cylinder sets can be found in
\c I6702.inp, \c rounded_ionchamber.geom, \c rz.geom and \c rz_phi.geom
example geometry files.

\todo Get rid off the local projector classes, use the egspp classes
instead.

*/

template <class T>
class EGS_CYLINDERS_EXPORT EGS_CylindersT : public EGS_BaseGeometry {
protected:

    EGS_Float *R,  //!< Radii
              *R2; //!< Radii squared
    EGS_Vector xo; //!< A point on the cylinder axis
    T a;           //!< The projection operator

#ifdef CYL_DEBUG
    EGS_Vector last_x, last_u;
    EGS_Float  last_t, last_d, last_B, last_A;
    int        last_ireg, last_dir;
#endif

public:

    /*! \brief Desctructor.

    Deallocates the #R and #R2 arrays
    */
    ~EGS_CylindersT() {
        if (nreg) {
            delete [] R;
            delete [] R2;
        }
    };

    /*! \brief Construct a set of concentric cylinders.

    Construct a set of cylinders named \a Name using the projection
    operator \a A.
    \a nc is the number of cylinders, \a radius their radii,
    \a position is a point on the cylinder axis
    */
    EGS_CylindersT(int nc, const EGS_Float *radius,
                   const EGS_Vector &position, const string &Name,
                   const T &A) : EGS_BaseGeometry(Name), a(A), xo(position) {
        if (nc>0) {
            R=new EGS_Float [nc];
            R2=new EGS_Float [nc];

            for (int i=0; i<nc; i++) {
                R[i]=radius[i];
                R2[i]=radius[i]*radius[i];
            }
            nreg=nc;
        }
    };

    /*! \brief \overload */
    EGS_CylindersT(const vector<EGS_Float> &radius,
                   const EGS_Vector &position, const string &Name,
                   const T &A) : EGS_BaseGeometry(Name), a(A), xo(position) {
        if (radius.size()>0) {
            R=new EGS_Float [radius.size()];
            R2=new EGS_Float [radius.size()];

            for (int i=0; i<radius.size(); i++) {
                R[i]=radius[i];
                R2[i]=radius[i]*radius[i];
            }
            nreg=radius.size();
        }
    };

    bool isInside(const EGS_Vector &x) {
        EGS_Vector rc(x-xo);
        EGS_Float rp=a*rc, rho_sq=rc.length2()-rp*rp;
        if (rho_sq>R2[nreg-1]) {
            return false;
        }
        return true;
    };

    int isWhere(const EGS_Vector &x) {
        EGS_Vector rc(x-xo);
        EGS_Float rp=a*rc, rho_sq=rc.length2()-rp*rp;
        if (rho_sq>R2[nreg-1]) {
            return -1;
        }
        if (rho_sq<R2[0]) {
            return 0;
        }
        return findRegion(rho_sq,nreg-1,R2)+1;
    };

    int inside(const EGS_Vector &x) {
        EGS_Vector rc(x-xo);
        EGS_Float
        rp=a*rc,
        rho_sq=rc.length2()-rp*rp;

        if (rho_sq>R2[nreg-1]) {
            return -1;    // outside all cylinders
        }
        if (rho_sq<R2[0]) {
            return 0;    // inside central cylinder
        }

        // find particle region
        int ic=0,oc=nreg,ms;
        while (oc-ic>1) {
            ms=(ic+oc)/2;
            if (rho_sq<R2[ms]) {
                oc=ms;
            }
            else {
                ic=ms;
            }
        }
        return oc;
    };

    EGS_Float howfarToOutside(int ireg, const EGS_Vector &x,
                              const EGS_Vector &u) {
        if (ireg < 0) {
            return 0;
        }
        EGS_Float up=a*u;
        if (fabs(up)>=1) {
            return 1e30;    // parallel to axis
        }
        EGS_Float A=1-up*up;
        EGS_Vector rc(x-xo);
        EGS_Float rcp=a*rc, urc=u*rc;
        EGS_Float C=rc.length2()-rcp*rcp-R2[nreg-1];
        if (C >= 0) {
            return 0;    // outside within precision
        }
        EGS_Float B=urc-up*rcp;
        EGS_Float Dsq = sqrt(B*B-A*C);
        EGS_Float d = B > 0 ? -C/(Dsq + B) : (Dsq - B)/A;
        return d;
    };

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed = 0, EGS_Vector *normal = 0) {

        EGS_Float d=1e30;  // large distance to any boundry

        // projections
        double up=a*u;
        if (fabs(up)>=1) {
            return ireg;    // parallel to cylinder axis
        }
        double A=1-up*up;        // d^2 coefficient

        int dir=-1; // direction entering or exiting cylinder boundry

        EGS_Vector rc(x-xo);
        double rcp=a*rc, urc=u*rc;
        double B=urc-up*rcp;  // d^1 coefficient

        EGS_Float rad;

        // in any region?
        if (ireg>=0) {

            double C=rc.length2()-rcp*rcp-R2[ireg]; // d^0 coefficient

            // B>=0 ... particle travelling radially outwards
            if (B>=0 || !ireg) { // OR it is IN the centre cylinder
                rad = -R[ireg];
                dir=ireg+1;
                if (dir >= nreg) {
                    dir = -1;
                }
                double Dsq = B*B-A*C;
                if (Dsq > 0) {
                    Dsq = sqrt(Dsq);
                }
                else {
                    if (Dsq < -1e-2) egsWarning("\nEGS_CylindersT::howfar(): "
                                                    "the particle may not be in the region\n   we think it "
                                                    "is as Dsq = %g\n",Dsq);
                    Dsq = 0;
                }
                //d=-B+sqrt(B*B-A*C);
                d = B > 0 ? -C/(Dsq + B) : (Dsq - B)/A;
                if (d < 0) {
                    if (C > 1e-2) {
                        egsWarning("\nEGS_CylindersT::howfar(): the particle "
                                   "may not be in the region\n   we think it is as "
                                   "Cout = %g\n",C);
                        egsWarning("   ireg=%d R2=%g R2[%d]=%g\n",ireg,
                                   rc.length2()-rcp*rcp,ireg,R2[ireg]);
                        egsWarning("x=(%g,%g,%g) u=(%g,%g,%g)\n",
                                   x.x,x.y,x.z,u.x,u.y,u.z);
                        egsWarning("B=%g A=%g\n",B,A);
#ifdef CYL_DEBUG
                        egsWarning("last:\n");
                        egsWarning("x=(%g,%g,%g) u=(%g,%g,%g)\n",
                                   last_x.x,last_x.y,last_x.z,last_u.x,last_u.y,last_u.z);
                        egsWarning("B=%g A=%g ireg=%d inew=%d\n",
                                   last_B,last_A,last_ireg,last_dir);
                        egsWarning("t=%g d=%g\n",last_t,last_d);
#endif
                    }
                    d = 1e-16;
                }
            }

            // check for intersection with inner cylinder
            else {
                double dR2=R2[ireg]-R2[ireg-1];
                C+=dR2;
                double D_sq=B*B-A*C;

                if (D_sq<=0) { // outer cylinder intersection
                    rad = -R[ireg];
                    dir=ireg+1;
                    if (dir >= nreg) {
                        dir = -1;
                    }
                    /*
                    D_sq+=A*C;               // should we not use
                    C-=dR2;                  // d = -B + sqrt(D_sq+A*dR2)
                    d=-B+sqrt(D_sq-A*C);     // instead?
                    */
                    D_sq += A*dR2;
                    if (D_sq > 0) {
                        D_sq = sqrt(D_sq);
                    }
                    else {
                        if (D_sq < -1e-2)
                            egsWarning("\nEGS_CylindersT::howfar(): the "
                                       "particle may not be in the region\n   we think "
                                       "it is as D_sq = %g\n",D_sq);
                        D_sq = 0;
                    }
                    d = (D_sq - B)/A;
                }

                else  { // inner cylinder intersection
                    dir=ireg-1;
                    rad = R[dir];
                    //d=-B-sqrt(D_sq);
                    d = C/(sqrt(D_sq) - B);
                    if (d < 0) {
                        if (C < -1e-2) egsWarning("EGS_CylindersT::howfar(): "
                                                      "the particle may not be in the region we think it "
                                                      "is as Cin = %g\n",C);
                        d = 1e-16;
                    }
                }
            }
        }

        else {  // outside all regions
            if (B<0) { // particle travelling radially inwards
                double C=rc.length2()-rcp*rcp-R2[nreg-1],
                       D_sq=B*B-A*C;

                if (D_sq>0) { // cylinder intersection
                    dir=nreg-1;
                    rad = R[dir];
                    //d=-B-sqrt(D_sq);
                    d = C/(sqrt(D_sq) - B);
                    if (d < 0) {
                        if (C < -1e-2) {
                            egsWarning("EGS_CylindersT::howfar(): "
                                       "we think that the particle is outside, but C=%g\n",C);
                            egsWarning("  d=%g B=%g D_sq=%g\n",d,B,D_sq);
                            egsWarning("  ireg=%d x=(%g,%g,%g) u=(%g,%g,%g)\n",
                                       ireg,x.x,x.y,x.z,u.x,u.y,u.z);
                        }
                        d = 1e-16;
                    }
                }
            }
            else {
                return ireg;
            }
        }
        //d/=A;

#ifdef CYL_DEBUG
        if (isnan(d)) {
            egsWarning("d is nan: A=%g B=%g ireg=%d R2=%g\n",
                       A,B,ireg,rc.length2()-rcp*rcp);
        }

        last_x = x;
        last_u = u;
        last_B = B;
        last_A = A;
        last_ireg = ireg;
        last_dir = dir;
        last_t = t;
        last_d = d;
#endif

        // correct t-step
        if (d<t) {
            t=d;
            if (newmed) if (dir >= 0) {
                    *newmed = medium(dir);
                }
                else {
                    *newmed=-1;
                }
            if (normal) {
                EGS_Vector n(rc + u*t - a*(rcp+up*t));
                *normal = n*(1/rad);
            }
            return dir;
        }
        return ireg;
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        EGS_Vector rc(x-xo);
        EGS_Float
        rcp=a*rc,
        rho=sqrt(rc.length2()-rcp*rcp);

        // check outer cylinder first if inside geometry
        if (ireg>=0) {
            EGS_Float d=R[ireg]-rho;

            if (ireg) {
                EGS_Float dd=rho-R[ireg-1];
                if (dd<d) {
                    d=dd;
                }
            }

            return d;

        }

        else {
            return rho-R[nreg-1];
        }
    };

    int getMaxStep() const {
        return 2*nreg + 1;
    };

    const string &getType() const {
        return a.getType();
    };

    void printInfo() const {
        EGS_BaseGeometry::printInfo();
        a.printInfo();
        egsInformation(" midpoint of cylinders = (%g,%g,%g)\n",xo.x,xo.y,xo.z);
        egsInformation(" cylinder radii = ");
        for (int j=0; j<nreg; j++) {
            egsInformation("%g ",R[j]);
        }
        egsInformation("\n");
        egsInformation("=====================================================\n");
    };
};

#ifndef SKIP_DOXYGEN
class EGS_CYLINDERS_LOCAL XProjector {
public:
    XProjector() {};
    EGS_Float operator*(const EGS_Vector &x) const {
        return x.x;
    };
    EGS_Vector operator*(EGS_Float t) const {
        return EGS_Vector(t,0,0);
    };
    EGS_Float length() const {
        return 1;
    };
    string &getType() const {
        return type;
    };
    void printInfo() const {};
private:
    static string type;
};

class EGS_CYLINDERS_LOCAL YProjector {
public:
    YProjector() {};
    EGS_Float operator*(const EGS_Vector &x) const {
        return x.y;
    };
    EGS_Vector operator*(EGS_Float t) const {
        return EGS_Vector(0,t,0);
    };
    EGS_Float length() const {
        return 1;
    };
    string &getType() const {
        return type;
    };
    void printInfo() const {};
private:
    static string type;
};

class EGS_CYLINDERS_LOCAL ZProjector {
public:
    ZProjector() {};
    EGS_Float operator*(const EGS_Vector &x) const {
        return x.z;
    };
    EGS_Vector operator*(EGS_Float t) const {
        return EGS_Vector(0,0,t);
    };
    EGS_Float length() const {
        return 1;
    };
    string &getType() const {
        return type;
    };
    void printInfo() const {};
private:
    static string type;
};

class EGS_CYLINDERS_LOCAL Projector {
public:
    Projector(const EGS_Vector &A) : a(A) {
        norm = a.length();
        a.normalize();
    };
    EGS_Float operator*(const EGS_Vector &x) const {
        return a*x;
    };
    EGS_Vector operator*(EGS_Float t) const {
        return a*t;
    };
    EGS_Float length() const {
        return norm;
    };
    string &getType() const {
        return type;
    };
    void printInfo() const {
        egsInformation(" axis = (%g,%g,%g)\n",a.x,a.y,a.z);
    };

private:
    EGS_Vector a;
    EGS_Float  norm;
    static string type;
};
#endif

typedef EGS_CylindersT<XProjector> EGS_CylindersX;
typedef EGS_CylindersT<YProjector> EGS_CylindersY;
typedef EGS_CylindersT<ZProjector> EGS_CylindersZ;
typedef EGS_CylindersT<Projector> EGS_Cylinders;

#endif
