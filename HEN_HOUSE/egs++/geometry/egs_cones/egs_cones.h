/*
###############################################################################
#
#  EGSnrc egs++ cones geometry headers
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
#                   Ernesto Mainegra-Hing
#
###############################################################################
*/


/*! \file egs_cones.h
 *  \brief Various cone geometries: header
 *  \IK
 */

#ifndef EGS_CONES_
#define EGS_CONES_

#include "egs_base_geometry.h"
#include "egs_functions.h"
#include "egs_math.h"

using namespace std;

#ifdef WIN32

    #ifdef BUILD_CONES_DLL
        #define EGS_CONES_EXPORT __declspec(dllexport)
    #else
        #define EGS_CONES_EXPORT __declspec(dllimport)
    #endif
    #define EGS_CONES_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_CONES_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_CONES_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_CONES_EXPORT
        #define EGS_CONES_LOCAL
    #endif

#endif

// the geometry methods of cones are fairly CPU intensive
// => we will not bother to use templates to have separate
// versions for x-cones, y-cones, etc.

/*! \brief A single cone that may be open (i.e. extends to infinity
  or closed by a plane perpendicular to the cone axis

\ingroup Geometry
\ingroup ElementaryG

The EGS_SimpleCone class models a single cone and is therefore a special case of the EGS_ConeSet
geometry. The reasons for having a separate class for this case are i) It is conceptually simpler
and was implemented first ii) Its geometry methods are slightly faster than the methods of an
EGS_ConeSet representing a single cone iii) It provides the additional possibility of the cone
being closed (\em i.e. its extent limited by a plane perpendicular to the cone axis). An
EGS_SimpleCone object is defined using

\verbatim

library = egs_cones
type = EGS_SimpleCone
apex = Ox, Oy, Oz     # Position of the cone apex
axis = ax, ay, az     # the cone axis
opening angle = angle in degrees
or
opening angle in radian = angle in radians
height = cone height in cm

\endverbatim
The \c height key is optional and results in a cone extending
to infinity, if missing.

*/

class EGS_CONES_EXPORT EGS_SimpleCone : public EGS_BaseGeometry {

protected:

    EGS_Vector    xo;          // the cone apex.
    EGS_Vector    a;           // the cone axis.
    EGS_Float     gamma, g12;  // tangent of opening angle and 1+gamma^2
    EGS_Float     g12i;        // 1/sqrt(g12)
    EGS_Float     d1;          // (-) closing plane distance (if closed)
    EGS_Float     Ro, Ro2;     // radius at base (if closed) and Ro^2
    bool          open;        // true if cone is open
    bool          is_cyl;      // true if cone is cylindrical (within hard-coded tolerance)
    static string type;

public:

    // constructor
    EGS_SimpleCone(const EGS_Vector &Xo, const EGS_Vector &A, EGS_Float Gamma,
                   const EGS_Float *distance=0, const string &N="")

        : EGS_BaseGeometry(N), xo(Xo), a(A), gamma(Gamma), g12(1+Gamma*Gamma),
          open(true), is_cyl(false) {

        a.normalize();
        g12i = 1/sqrt(g12);

        // closed cone
        if (distance) {
            if (*distance > 0) {
                open = false;
                d1 = -(*distance);
                Ro = -d1*gamma;
                Ro2 = Ro*Ro;
            }
            else {
                egsWarning("EGS_SimpleCone: the distance to the closing plane must be positive\n");
            }
        }
        nreg = 1;
    }

    // constructor
    EGS_SimpleCone(const EGS_Vector &Xo, const EGS_Vector &A, EGS_Float dz,
                   EGS_Float Rtop, EGS_Float Rbottom, bool Open=true, const string &N="")

        : EGS_BaseGeometry(N), a(A), open(Open), is_cyl(false) {

        a.normalize();

        // avoid round-off problems.
        if (fabs(Rtop - Rbottom) < 2e-5) {          // flag cylinders to avoid round-off problems
            is_cyl = true;
            xo = Xo;
            Ro = Rtop;
            Ro2 = Ro*Ro;
            gamma = 0;
            g12 = 1;
            g12i = 1;
            d1 = -dz;
        }
        else if (Rtop < Rbottom) {                  // cone is opening along axis vector a
            EGS_Float aux = dz*Rtop/(Rbottom-Rtop);
            xo = Xo - a*aux;
            gamma = Rbottom/(aux+dz);
            g12 = 1 + gamma*gamma;
            g12i = 1/sqrt(g12);
            d1 = -dz-aux;
        }
        else {                                      // cone is closing along axis vector a
            EGS_Float aux = dz*Rbottom/(Rtop-Rbottom);
            xo = Xo + a*(aux+dz);
            a *= (-1);
            gamma = Rtop/(aux+dz);
            g12 = 1 + gamma*gamma;
            g12i = 1/sqrt(g12);
            d1 = -dz-aux;
        }
    }

    // destructor
    ~EGS_SimpleCone() {}

    // isInside
    bool isInside(const EGS_Vector &x) {

        EGS_Vector xp(x-xo);                        // vector from cone apex xo to test point x
        EGS_Float  aa = xp*a;                       // projection of xp on axis vector a

        // check bounds along cone axis
        if (!open && (aa < 0 || aa+d1 > 0)) {
            return false;
        }

        // check cylinder radius
        if (is_cyl) {
            EGS_Float r2 = xp.length2() - aa*aa;
            if (r2 <= Ro2) {
                return true;
            }
            return false;
        }

        // check cone radius
        EGS_Float r2 = xp.length2();
        if (r2 <= aa*aa*g12) {
            return true;
        }
        return false;
    }

    // isWhere
    int isWhere(const EGS_Vector &x) {
        if (isInside(x)) {
            return 0;
        }
        return -1;
    }

    // inside
    int inside(const EGS_Vector &x) {
        return isWhere(x);
    }

    // howfar
    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed=0, EGS_Vector *normal=0) {

        EGS_Vector  xp(x-xo);                       // vector from cone apex xo to test point x
        EGS_Float   aa = xp*a;                      // projection of xp on cone axis a
        EGS_Float   b  = u*a;                       // cos(t) between axis a and step vector u
        EGS_Float   r2 = xp.length2();              // length^2 of xp
        EGS_Float   c  = u*xp;                      // projection of xp on step vector u

        // handle cylinder
        if (is_cyl) {

            // u is parallel to a (u and a are normalized)
            if (fabs(b) >= 1) {
                return ireg;
            }

            EGS_Float A = 1 - b*b;                  // A = 1-cos^2(t) = sin^2(t)
            EGS_Float B = c - b*aa;                 // B = u * [xp - (xp*a)a] =  u * n
            EGS_Float C = r2 - aa*aa - Ro2;         // C = n^2 - R0^2
            EGS_Float d;                            // distance to cylinder along u

            // from inside cylinder
            if (!ireg) {
                EGS_Float D = B*B-A*C;              // R0^2 - [ n^2 - ((n*u)/sint)^2 ]
                if (D < 0 || (C > 0 && B > 0)) {
                    d = 0;
                }
                // ^ hopefully a precision problem.
                else {
                    d = B<=0 ? (sqrt(D)-B)/A : -C/(sqrt(D)+B);
                }
            }
            // from outside cylinder
            else {
                if (C < 0) {                        // inside cylinder: fp precision error
                    if (B < 0) {
                        d = 0;    // assume we are on the cylinder
                    }
                    else {
                        return ireg;    // u points away from cylinder
                    }
                }
                else {
                    if (B >= 0) {
                        return ireg;    // u points away from cylinder
                    }
                    EGS_Float D = B*B-A*C;          // R0^2 - [ n^2 - ((n*u)/sint)^2 ]
                    if (D < 0) {
                        return ireg;    // u line does not intersect cylinder
                    }
                    d = C/(sqrt(D)-B);              // solve (n + d*(u-a(u*a)))^2 - R0^2 = 0
                }
            }

            // distance to cylinder is longer than step length
            if (d > t) {
                return ireg;
            }

            // intersection with cylinder is beyond boundary planes
            if (!open) {
                EGS_Float aux = aa + d*b;
                if (aux < 0 || aux+d1 > 0) {
                    return ireg;
                }
            }

            // intersect with cylinder
            t = d;
            if (newmed && ireg < 0) {
                *newmed = med;
            }
            if (normal) {
                EGS_Float lam = aa+b*d;
                EGS_Vector aux(xp+u*d-a*lam);
                aux.normalize();
                *normal = ireg < 0 ? aux : aux*(-1);
            }

            return (!ireg ? -1 : 0);
        }

        // handle odd case where position numerically coincides with apex
        if (!xp.length2()) {

            int inew;

            // u is aiming inside the cone angle
            if (b >= g12i) {                        // g12i = cos(t_cone); b = cos(t)
                inew = 0;
                if (ireg < 0) {                     // we are coming from outside
                    t = 0;
                    if (newmed) {
                        *newmed = med;
                    }
                    if (normal) {
                        *normal = a*(-1);
                    }
                }
                else if (!open && b>0) {            // check closing plane
                    EGS_Float tt = -d1/b;           // tt = -(aa + d1)/b but aa=0 if here
                    if (tt <= t) {
                        t = tt;
                        inew = -1;
                        if (normal) {
                            *normal = a*(-1);
                        }
                    }
                }
            }
            // u is aiming outside the cone angle
            else {
                inew = -1;
                if (ireg >= 0) {                    // we are coming from inside
                    t = 0;
                    if (normal) {
                        *normal = a;
                    }
                }
            }

            // return new region index
            return inew;
        }

        // handle closing plane
        if (!open) {

            // from outside cone
            if (ireg < 0) {
                if (aa+d1 > 0 && b < 0) {           // outside closing plane, moving towards it
                    EGS_Float tt = -(aa+d1)/b;      // distance to closing plane
                    if (tt <= t) {
                        EGS_Float r2p = r2 + tt*(tt + 2*c);
                        if (r2p <= d1*d1*g12) {     // enter via the closing plane
                            if (newmed) {
                                *newmed = med;
                            }
                            if (normal) {
                                *normal = a;
                            }
                            t = tt;
                            return 0;
                        }
                    }

                    // if we are inside the conical surface, the only way to enter is via the
                    // closing plane. but if (r2 <= aa*aa*g12), we didn't enter: simply return
                    if (r2 <= aa*aa*g12) {
                        return ireg;
                    }
                }
            }
            // from inside cone
            else {
                if (b > 0) {                        // moving towards the closing plane.
                    EGS_Float tt = -(aa+d1)/b;      // distance to closing plane
                    if (tt <= t) {
                        EGS_Float r2p = r2 + tt*(tt + 2*c);
                        if (r2p <= d1*d1*g12) {     // exit via the closing plane
                            if (newmed) {
                                *newmed = -1;
                            }
                            if (normal) {
                                *normal = a*(-1);
                            }
                            t = tt;
                            return -1;
                        }
                    }
                }
            }
        }

        // At this point, we cannot enter or exit via the closing plane; all we need to check
        // is the conical surface. solve for t: (x+t*u)^2 = g12*[(x+t*u)*a]^2 (i.e., point
        // x+t*u is on cone surface).
        // solution: t = (-B +- sqrt(B^2-A*C))/A

        EGS_Float A   = 1 - b*b*g12;                // 1 - cos^2(t)/cos^2(t_cone)
        EGS_Float B   = c - aa*b*g12;
        EGS_Float C   = r2 - aa*aa*g12;
        EGS_Float tt  = 1e30;
        EGS_Float lam = -1;

        if (fabs(A) < 1e-6) {
            // moving parallel to the cone surface (A=0, within hard-coded tolerance):
            // solution: t = -C/(2*B), if t>0

            if ((!ireg && B>0) || (ireg && B<0)) {
                EGS_Float ttt = -C/(2*B);           // solution
                lam = aa+b*ttt;                     // (x+t*u)*a >= 0: on "positive" cone
                if (ttt >= 0 && lam >= 0) {
                    tt = ttt;    // distance to cone surface
                }
            }
        }
        else {
            // general solution, guarding against numerical instability
            // solution: t = (-B +- sqrt(B^2-A*C))/A
            //
            // pick the smallest positive root
            // A*C > 0 => roots have same sign
            // A*C < 0 => roots have opposite sign
            //
            // if C<0, then if A>0 pick '+ root',
            //         else if A<0 we must have B>0 for positive roots: pick '+ root'
            //         (guard against cancellation if B>0)
            //
            // if C>0, then if A<0 roots have opposite signs: pick '- root' (-|A| flips sign)
            //         else if A>0 we must have B<0 for positive roots: pick '- root'
            //         (guard against cancellation if B<0)
            //
            // by testing for ireg instead of the sign of C, we:
            // 1) avoid numerical instability at C=0
            // 2) reverse the choice for the upper cone: largest positive root or negative one.

            EGS_Float ttt = -1;
            EGS_Float D = B*B-A*C;                  // determinant
            if (D<0) {
                return ireg;    // no real solution: no intersection
            }

            if (!ireg && !(A<0 && B<0)) {           // inside cone
                if (B>0) {
                    ttt = -C/(B+sqrt(D));
                }
                else {
                    ttt = (sqrt(D)-B)/A;
                }
            }
            else if (ireg && !(A>0 && B>0)) {       // outside cone
                if (B<0) {
                    ttt = C/(sqrt(D)-B);
                }
                else {
                    ttt = -(sqrt(D)+B)/A;
                }
            }
            else {
                return ireg;    // no intersection
            }

            lam = aa+b*ttt;                         // (x+t*u)*a >= 0: on "positive" cone
            if (ttt >= -2e-5 && lam >= 0) {
                tt = ttt;    // why the hard-coded -2e-5 bound??
            }
        }

        // distance too far, or intersection beyond bounding plane
        if (tt > t || (!open && ireg && d1 + aa + tt*b > 0)) {
            return ireg;
        }


        // set distance and new region
        t = tt;
        int inew = ireg ? 0 : -1;

        // set medium and surface normal
        if (newmed) {
            *newmed = inew ? -1 : med;
        }
        if (normal) {
            *normal = xp + u*t - a*(lam*g12);
            normal->normalize();
            if (!ireg) {
                *normal *= (-1);
            }
        }

        // return new region
        return inew;
    }


    // hownear
    EGS_Float hownear(int ireg, const EGS_Vector &x) {

        EGS_Vector xp(x-xo);                        // position with respect to cone origin
        EGS_Float aa = xp*a;                        // position along cone axis
        EGS_Float ag = aa*gamma;                    // cone radius at axis position
        EGS_Float r2 = xp.length2();                // distance squared to cone origin

        // cylindrical case
        if (is_cyl) {

            // open, or within bounding planes
            if (open || (aa > 0 && aa+d1 < 0)) {
                r2 = sqrt(r2 - aa*aa);              // radial distance from axis
                if (ireg) {
                    return r2-Ro;    // from outside
                }
                else {
                    return Ro-r2;    // from inside
                }
            }

            r2 = sqrt(r2 - aa*aa);                  // radial distance from axis

            // inside the radius of cylinder: shortest distance is to closing plane
            if (r2 < Ro) {
                if (aa<0) {
                    return -aa;    // distance to "0" closing plane
                }
                else {
                    return aa+d1;    // distance to "d1" closing plane
                }
            }

            // outside the radius of cylinder: shortest distance is to cylinder cap "corner"
            EGS_Float aux = r2-Ro;
            if (aa<0) {
                return sqrt(aa*aa + aux*aux);    // distance to "0" cap
            }
            EGS_Float tp = aa+d1;
            return sqrt(tp*tp + aux*aux);           // distance to "d1" cap

        }

        // inside "negative" cone lobe
        if (aa < 0 && ag*ag > r2*g12) {             // cone tip is the nearest point
            return sqrt(r2);
        }

        // compute useful distances
        EGS_Float r2a2 = r2-aa*aa;                  // distance^2 to cone axis
        EGS_Float r2a  = sqrt(r2a2);                // distance to cone axis
        EGS_Float tc = fabs((ag-r2a)*g12i);         // distance to cone (perp. to cone surface)

        // open cone: tc is the shortest distance
        if (open) {
            return tc;
        }

        // closed cone: check closing plane
        EGS_Float tp = d1+aa;                       // signed axial distance to closing plane

        // inside cone
        if (!ireg) {
            tp = -tp;                               // adjust sign of distance
            if (tp < tc) {
                tc = tp;    // pick shortest distance
            }
        }
        // outside, but inside conical surface
        else if (tp > 0 && r2 < aa*aa*g12) {
            if (r2a2 > Ro2) {                       // outside the cone base cylinder
                EGS_Float aux = r2a - Ro;
                tc = sqrt(tp*tp + aux*aux);         // nearest point is cone base "corner"
            }
            else {
                tc = tp;    // inside the cone base cylinder
            }
        }
        // outside, and outside conical surface
        else if (tp + tc*gamma*g12i > 0) {          // nearest point is cone base "corner"
            EGS_Float aux = r2a + d1*gamma;         // distance to cone base cylinder
            tc = sqrt(aux*aux + tp*tp);             // distance to cone base "corner"
        }

        // return shortest distance to cone
        return tc;
    }

    // getType accessor
    const string &getType() const {
        return type;
    }

    // printInfo accessor
    void printInfo() const;

    // getApex accessor
    EGS_Vector getApex() const {
        return xo;
    }

    // getAxis accessor
    EGS_Vector getAxis() const {
        return a;
    }

    // getGamma accessor
    EGS_Float getGamma() const {
        return gamma;
    }

    // getRadius accessor
    EGS_Float getRadius(const EGS_Vector &x) const {
        if (is_cyl) {
            return Ro;
        }
        EGS_Float xp = (x-xo)*a;
        if (xp <= 0) {
            return 0;
        }
        return xp*gamma;
    }
};


// A set of cones having the same axis and opening angle and
// apexes along the cone axis:
//
//          /\
//         /  \
//        /    \
//       /  /\  \
//      /  /  \  \
//     /  /    \  \
//    /  /  /\  \  \
//   /  /  /  \  \  \
//
// That sort of geometry is useful for modelling e.g. the tip of
// many ionization chambers.
//

/*! \brief A set of "parallel cones" (\em i.e. cones with the same axis and
opening angles but different apexes)

\ingroup Geometry
\ingroup ElementaryG

EGS_ParallelCones models a set of cones all having the same axis and
opening angle but different cone apex position along the cone axis.
\image html cones1.png "Parallel cones"
This type of geometry is useful, for instance, for modeling
the tip of an ionization chamber. It is defined via the following set
of keys
\verbatim

library = egs_cones
type = EGS_ParallelCones
apex                    = Ox, Oy, Oz     # Position of one cone apex
axis                    = ax, ay, az     # the cone axis
opening angle           = angle in degrees
or
opening angle in radian = angle in radians
apex distances          = list of distances from the apex

\endverbatim

*/
class EGS_ParallelCones : public EGS_BaseGeometry {

    EGS_Vector   *xo;           // the cone apexes
    EGS_Vector   a;             // the cone axis.
    EGS_Float    gamma, g12;    // tangent of opening angle and 1 + gamma^2
    EGS_Float    g12i;          // 1/sqrt(g12)
    EGS_Float    *d;            // distances from the first apex.
    int          nc;            // number of cones
    static string type;

public:

    // constructor
    EGS_ParallelCones(int Nc, const EGS_Vector &Xo, const EGS_Vector &A, EGS_Float Gamma,
                      const EGS_Float *distances, const string &N="")

        : EGS_BaseGeometry(N), a(A), gamma(Gamma), g12(1+Gamma*Gamma), nc(Nc) {

        a.normalize();
        g12i = 1/sqrt(g12);

        if (nc < 1) {
            nc=1;    // enforce nc >= 1
        }
        nreg = nc;                                  // one region per cone
        xo = new EGS_Vector[nc];                    // vector of cone apex positions
        xo[0] = Xo;                                 // apex of first cone

        // more than one cone
        if (nc > 1) {
            EGS_Float
            d_old = 0;
            d = new EGS_Float [nc-1];
            for (int j=1; j<nc; j++) {
                d[j-1] = distances[j-1];
                if (d[j-1] <= d_old) {
                    egsFatal("EGS_ParallelCones: " "distances must be in increasing order\n");
                }
                xo[j] = xo[0] + a*d[j-1];
                d_old = d[j-1];
            }
        }
    }

    // destructor
    ~EGS_ParallelCones() {
        delete [] xo;
        if (nc > 1) {
            delete [] d;
        }
    }

    // isInside
    bool isInside(const EGS_Vector &x) {
        EGS_Vector xp(x-xo[0]);                     // current position with respect to apex
        EGS_Float aa = xp*a;                        // current axial position
        if (aa < 0) {
            return false;    // on "negative" side of cones
        }
        EGS_Float r2 = xp.length2();                // distance^2 to apex
        if (r2 <= aa*aa*g12) {
            return true;    // inside outer cone
        }
        return false;
    };

    // isWhere
    int isWhere(const EGS_Vector &x) {
        EGS_Vector xp(x-xo[0]);                     // current position with respect to apex
        EGS_Float aa = xp*a;                        // current axial position
        if (aa < 0) {
            return -1;    // on "negative" side of apex
        }
        EGS_Float r2 = xp.length2();                // distance^2 to apex
        if (r2 > aa*aa*g12) {
            return -1;    // outside outer cone
        }
        EGS_Float r2a2 = r2 - aa*aa;                // distance to cone set axis
        for (int j=0; j<nc-1; j++) {                // loop over all inner cones
            EGS_Float aj  = aa - d[j];              // distance to apex of cone j
            EGS_Float ajg = aj*gamma;               // length of cone j'th side up to aj
            if (aj < 0 || r2a2 > ajg*ajg) {
                return j;    // on "negative" side or outside of cone j
            }
        }
        return nc-1;                                // then it must be in last cone
    }

    // inside
    int inside(const EGS_Vector &x) {
        return isWhere(x);
    }

    // howfar
    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u, EGS_Float &t,
               int *newmed=0, EGS_Vector *normal=0) {

        // not in innermost cone
        if (ireg < nc-1) {
            EGS_Vector xp;                          // a position
            bool hit = false;                       // howfar hit flag

            // apex position of "next" inner cone
            if (ireg < 0) {
                xp = x-xo[0];    // apex of outer cone
            }
            else {
                xp = x-xo[ireg+1];    // apex of next inner cone
            }

            // general solution
            EGS_Float aa = xp*a;                    // axial position in current cone
            EGS_Float b  = u*a;                     // cos(t), t = angle between a and u
            EGS_Float r2 = xp.length2();            // distance^2 to current cone apex
            EGS_Float c  = u*xp;                    // projection of xp on u
            EGS_Float A  = 1 - b*b*g12;
            EGS_Float B  = c - aa*b*g12;
            EGS_Float C  = r2 - aa*aa*g12;
            EGS_Float tt = 1e30;
            EGS_Float lam = -1;

            // moving parallel to cone surface
            if (fabs(A) < 1e-6) {                   // guarding against /0 in general solution
                EGS_Float ttt = -C/(2*B);           // distance to hit
                lam = aa+b*ttt;                     // axial position of hit
                if (ttt >= 0 && lam >= 0) {
                    tt  = ttt;
                    hit = true;
                }
            }
            // general solution
            else {
                EGS_Float D = B*B-A*C;
                if (D >= 0 && !(A > 0 && B > 0)) {
                    EGS_Float ttt = B < 0 ? C/(sqrt(D)-B) : -(sqrt(D)+B)/A;
                    lam = aa+b*ttt;
                    if (ttt >= 0 && lam >= 0) {
                        tt = ttt;
                        hit = true;
                    }
                }
            }
            if (tt <= t) {
                int inew = ireg < 0 ? 0 : ireg+1;
                t = tt;
                if (newmed) {
                    *newmed = medium(inew);
                }
                if (normal) {
                    *normal = xp + u*t - a*(lam*g12);
                    normal->normalize();
                }
                return inew;
            }
            if (ireg < 0 || hit) {
                return ireg;
            }
        }

        // inside and not hitting the next inner cone.
        // check the previous outer cone
        EGS_Vector xp(x-xo[ireg]);
        EGS_Float aa = xp*a, b = u*a, r2 = xp.length2(), c = u*xp;
        EGS_Float A = 1 - b*b*g12, B = c - aa*b*g12, C = r2 - aa*aa*g12;
        EGS_Float to = 1e30, lamo = -1;
        if (fabs(A) < 1e-6) {  // moving parallel to the cone surface.
            // for the outer cone we only have a solution if a*u < 0.
            // i.e. if we are moving towards the apex.
            if (b < 0) {
                EGS_Float ttt = -C/(2*B);
                lamo = aa+b*ttt;
                if (ttt >= 0 && lamo >= 0) {
                    to = ttt;
                }
            }
        }
        else {
            EGS_Float D = B*B-A*C;
            // avoid numerical problems when |A*C| << |B|
            if (D >= 0 && !(A < 0 && B < 0)) {
                EGS_Float ttt = B > 0 ? -C/(B+sqrt(D)) : (sqrt(D)-B)/A;
                lamo = aa+b*ttt;
                if (ttt >= 0 && lamo >= 0) {
                    to = ttt;
                }
            }
        }
        if (to <= t) {
            t = to;
            int inew = ireg-1;
            if (newmed) {
                *newmed = inew < 0 ? -1 : medium(inew);
            }
            if (normal) {
                *normal = xp + u*t - a*(lamo*g12);
                normal->normalize();
            }
            return inew;
        }
        return ireg;
    }

    // hownear
    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        EGS_Float tc;
        if (ireg < 0 || ireg < nc-1) {
            EGS_Vector xp;
            if (ireg < 0) {
                xp = x - xo[0];
            }
            else {
                xp = x - xo[ireg+1];
            }
            EGS_Float aa = xp*a;
            EGS_Float ag = aa*gamma;
            EGS_Float r2 = xp.length2();
            if (aa < 0 && ag*ag > r2*g12) {
                tc = sqrt(r2);
            }
            else {
                tc = fabs((ag-sqrt(r2-aa*aa))*g12i);
            }
            if (ireg < 0) {
                return tc;
            }
        }
        EGS_Vector xp(x-xo[ireg]);
        EGS_Float aa = xp*a;
        EGS_Float ag = aa*gamma;
        EGS_Float r2 = xp.length2();
        EGS_Float tco = fabs((ag-sqrt(r2-aa*aa))*g12i);
        return tco < tc ? tco : tc;
    }

    // getMaxStep
    int getMaxStep() const {
        return 2*nreg + 2;
    }

    // getType accessor
    const string &getType() const {
        return type;
    }

    // printInfo
    void printInfo() const;
};


// A set of cones having the same apex and axis but different opening angles.
// If flag = 0, only cones below the apex.
//         = 1, cones below and above the apex but excluding the space
//              outside of the last cone
//         = 2, all space

/*! \brief A set of cones with different opening angles but the same axis and
apexes.

\ingroup Geometry
\ingroup ElementaryG

EGS_ConeSet models a set of cones with the same apex and axis
but different opening angles.
\image html cones2.png "Set of cones"
It is defined via the following set
of keys
\verbatim

library = egs_cones
type = EGS_ConeSet
apex = Ox, Oy, Oz     # Position of the cone apex
axis = ax, ay, az     # the cone axis
opening angles = list of angles in degrees
or
opening angles in radian = list of angles in radians
flag = 0 or 1 or 2

\endverbatim
The meaning of the \c flag variable is as follows:
If set to 0, only the cones below the apex are considered
(the term "below" means that the scalar product
 \f$\vec{a} \cdot (\vec{x} - \vec{x}_0)\f$ is greater than zero for points
 \f$\vec{x}\f$ "below" the apex, where
 \f$\vec{a}\f$ is the cone axis and \f$\vec{x}_0\f$ the cone apex).
In this case a set of \f$N\f$ cones defines a geometry with \f$N\f$ regions
with the region within the inner-most cone (the cone with the smallest
opening angle) being region zero. If set to 1, the \f$N\f$ cones "below"
and \f$N\f$ cones "above" the apex are considered to form a geometry with
\f$2 N\f$ regions where regions \f$0,1,...,N-1\f$ are below the apex and
regions \f$N+1, N+2, ..., 2 N\f$ are the region indices for the
cones above the apex.
If \c flag is set 2,
the geometry will have \f$2 N + 1\f$ regions with the region outside
of the cone with the largest opening angle having index \f$N\f$.
An example for the use of an EGS_ConeSet is found in the
cones.geom example geometry file.

*/
class EGS_ConeSet : public EGS_BaseGeometry {

    EGS_Vector    xo;   // the common apex.
    EGS_Vector    a;    // the axis.
    EGS_Float     *gamma, *g12, *g12i;
    int           nc;
    int           flag;

    static string type;

public:

    EGS_ConeSet(const EGS_Vector &Xo, const EGS_Vector &A, int Nc,
                EGS_Float *Gamma, int Flag = 0, const string &Name = "") :
        EGS_BaseGeometry(Name), xo(Xo), a(A), flag(Flag) {
        a.normalize();
        if (Nc < 1) {
            egsFatal("EGS_ConeSet: number of cones must be positive\n");
        }
        nc = Nc;
        gamma = new EGS_Float [nc];
        g12 = new EGS_Float [nc];
        g12i = new EGS_Float [nc];
        if (flag < 0 || flag > 2) {
            egsWarning("EGS_ConeSet: flag must be 0,1 or 2, reseting to 0\n");
            flag = 0;
        }
        for (int j=0; j<nc; j++) {
            if (Gamma[j] <= 0) {
                egsFatal("EGS_ConeSet: gamma's must be positive\n");
            }
            if (j > 0) {
                if (Gamma[j] <= gamma[j-1]) egsFatal("EGS_ConeSet: "
                                                         "gamma's must be in increasing order\n");
            }
            gamma[j] = Gamma[j];
            g12[j] = 1 + gamma[j]*gamma[j];
            g12i[j] = 1/sqrt(g12[j]);
        }
        if (flag == 0) {
            nreg = nc;
        }
        else {
            nreg = 2*nc + 1;
        }
        if (flag == 1) {
            is_convex = false;
        }
    }

    ~EGS_ConeSet() {
        delete [] gamma;
        delete [] g12;
        delete [] g12i;
    }

    bool isInside(const EGS_Vector &x) {
        if (flag == 2) {
            return true;
        }
        EGS_Vector xp(x-xo);
        EGS_Float aa = xp*a;
        if (flag == 0 && aa < 0) {
            return false;
        }
        EGS_Float r2 = xp.length2();
        if (r2 <= aa*aa*g12[nc-1]) {
            return true;
        }
        return false;
    }

    int isWhere(const EGS_Vector &x) {
        EGS_Vector xp(x-xo);
        EGS_Float aa = xp*a;
        if (flag == 0 && aa < 0) {
            return -1;
        }
        EGS_Float r2 = xp.length2();
        int j;
        for (j=0; j<nc; j++) if (r2 <= aa*aa*g12[j]) {
                break;
            }
        if (j < nc) {
            if (aa >= 0) {
                return j;
            }
            return 2*nc-j;
        }
        if (flag != 2) {
            return -1;
        }
        return nc;
    }

    bool isRealRegion(int ireg) const {
        if (ireg < 0 || ireg >= nreg) {
            return false;
        }
        return flag != 1 ? true : (ireg != nc);
    }

    int inside(const EGS_Vector &x) {
        return isWhere(x);
    }

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed = 0, EGS_Vector *normal = 0) {
        EGS_Vector xp(x-xo);
        EGS_Float aa = xp*a, b = u*a, r2 = xp.length2(), c = u*xp;
        //if( debug ) egsWarning(" ireg = %d x = (%g,%g,%g) a = %g"
        //     " b = %g r2 = %g c = %g\n",ireg,x.x,x.y,x.z,aa,b,r2,c);
        if (!xp.length2()) {
            // handle odd case where position coincides with apex
            int inew = isWhere(xp + u);
            if (inew != ireg) {
                t = 0;
                if (normal) {
                    *normal = a;
                }
                if (newmed) {
                    *newmed = inew >= 0 ? medium(inew) : -1;
                }
            }
            return inew;
        }
        if (ireg != 0 && ireg != 2*nc) {
            // Not in the inner-most cone.
            // If outside (ireg < 0 || flag=2 and ireg=nc), check for
            // intersection with the outer-most cone
            // If inside, check for intersection with the inner cone.
            EGS_Float gam12;
            if (ireg < 0 || ireg == nc) {
                gam12 = g12[nc-1];
            }
            else {
                gam12 = ireg < nc ? g12[ireg-1] : g12[2*nc-ireg-1];
            }
            EGS_Float A=1-b*b*gam12, B=c-aa*b*gam12, C=r2-aa*aa*gam12;
            //if( debug ) egsWarning(" gam12 = %g A = %g B = %g "
            //        "C = %g\n",gam12,A,B,C);
            EGS_Float tt = -1, lam;
            bool hit = false;
            if (fabs(A) < 1e-6) {
                if ((ireg < nc && b > 0) || (ireg >= nc && b < 0)) {
                    tt = -C/(2*B);
                }
            }
            else {
                EGS_Float D = B*B-A*C;
                if (D >= 0 && !(A > 0 && B > 0)) {
                    tt = B < 0 ? C/(sqrt(D)-B) : -(sqrt(D)+B)/A;
                }
            }
            if (tt >= 0) {   // possible intersection
                lam = aa+b*tt;
                int inew;
                if (ireg == nc || ireg == -1) {
                    // outside the outer-most cone.
                    // if flag=1 or flag=2, we are allowed to hit either
                    // of the two cones
                    if (flag) {
                        hit = true;
                        inew = lam >= 0 ? nc-1 : nc+1;
                    }
                    // if flag=0, we are allowed to hit only the lower
                    // cone (lam >= 0)
                    else if (!flag && lam>=0) {
                        hit = true;
                        inew = nc-1;
                    }
                }
                else {
                    // inside. In this case we are only allowed to hit
                    // the cone on the same side of the apex.
                    if (lam*aa >= 0) {
                        hit = true;
                        inew = lam >= 0 ? ireg-1 : ireg+1;
                    }
                }
                if (hit && tt <= t) {
                    t = tt;
                    if (newmed) {
                        *newmed = medium(inew);
                    }
                    if (normal) {
                        *normal = xp + u*t - a*(lam*gam12);
                        normal->normalize();
                    }
                    return inew;
                }
            }
            if (ireg < 0 || ireg == nc || hit) {
                return ireg;
            }
        }
        //EGS_Float gam12 = ireg < nc-1 ? g12[ireg] : g12[2*nc-ireg];
        EGS_Float gam12 = ireg < nc ? g12[ireg] : g12[2*nc-ireg];
        EGS_Float A=1-b*b*gam12, B=c-aa*b*gam12, C=r2-aa*aa*gam12;
        EGS_Float tt=-1;
        if (fabs(A) < 1e-6) {
            if ((ireg < nc && b < 0) || (ireg > nc && b > 0)) {
                tt = -C/(2*B);
            }
        }
        else {
            EGS_Float D = B*B-A*C;
            // avoid numerical problems when |A*C| << |B|
            if (D >= 0 && !(A < 0 && B < 0)) {
                tt = B > 0 ? -C/(B+sqrt(D)) : (sqrt(D)-B)/A;
            }
        }
        if (tt < 0) {
            return ireg;
        }
        EGS_Float lam = aa + b*tt;
        if ((ireg < nc && lam < 0) || (ireg > nc && lam > 0)) {
            return ireg;
        }
        if (tt <= t) {
            t = tt;
            int inew;
            if (ireg < nc) {
                inew = ireg+1;
                if (inew == nc && flag < 2) {
                    inew = -1;
                }
            }
            else {
                inew = ireg-1;
                if (inew == nc && flag < 2) {
                    inew = -1;
                }
            }
            if (newmed) {
                *newmed = inew < 0 ? -1 : medium(inew);
            }
            if (normal) {
                *normal = xp + u*t - a*(lam*gam12);
                normal->normalize();
            }
            return inew;
        }
        return ireg;
    }

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        EGS_Float tc;
        EGS_Vector xp(x-xo);
        EGS_Float aa = xp*a, r2 = xp.length2(), ag;
        //egsWarning("hownear: ireg = %d x = (%g,%g,%g) aa = %g r2 = %g\n",
        //        ireg,x.x,x.y,x.z,aa,r2);
        if (ireg != 0 && ireg != 2*nc) {
            int i;
            if (ireg < 0 || ireg == nc) {
                i = nc-1;
            }
            else {
                i = ireg < nc ? ireg-1 : 2*nc-ireg-1;
            }
            ag = aa*gamma[i];
            if (flag == 0 && aa < 0 && ag*ag > r2*g12[i]) {
                tc = sqrt(r2);
            }
            else {
                tc = fabs((fabs(ag)-sqrt(r2-aa*aa))*g12i[i]);
            }
            //egsWarning(" i = %d ag = %g g12i = %g tc = %g\n",i,ag,g12i[i],tc);
            if (ireg < 0 || ireg == nc) {
                return tc;
            }
        }
        EGS_Float gam12i;
        if (ireg < nc) {
            ag = aa*gamma[ireg];
            gam12i = g12i[ireg];
        }
        else {
            ag = -aa*gamma[2*nc-ireg];
            gam12i = g12i[2*nc-ireg];
        }
        EGS_Float tco = fabs((ag-sqrt(r2-aa*aa))*gam12i);
        //egsWarning(" ag = %g tco = %g\n",ag,tco);
        return tco < tc ? tco : tc;
    }

    int getMaxStep() const {
        return 4*nreg + 2;
    }

    const string &getType() const {
        return type;
    }

    void printInfo() const;
};

//
// A cone stack is a collection of cones with possible
// different opening angles, apexes, etc., stacked together
// This is very similar to the cone stack component module in BEAM
//
//
#include <vector>
using std::vector;

/*! \brief A cone stack

\ingroup Geometry
\ingroup ElementaryG

The EGS_ConeStack class models a series of layers divided into conical segments stacked together to
form a potentially complex geometrical structure as shown in the following figure. \image html
cones3.png "A cone stack" Each layer may have a different set of arbitrary number of cones with
different opening angles and apexes, including cylinders (which are just special cases of cones).
There can be an arbitrary number of layers in a cone stack. The only restriction is that all cones
in a stack must have the same axis. A cone stack is defined using the following set of keys:

\verbatim
library = egs_cones
type = EGS_ConeStack
axis = Ox, Oy, Oz,  ax, ay, az
:start layer:
    input defining a layer
:stop layer:
...
:start layer:
   input defining a layer
:stop layer:
\endverbatim

The input to the \c axis key defines the axis of all cones as a position (<code>Ox,Oy,Oz</code>)
and direction (<code>ax,ay,az</code>). Note that the position also defines the position of the top
plane of the first layer in the stack. There can be an arbitrary number of <code>:start layer:
:stop layer:</code> sections each defining one layer in the stack in sequence. The input needed to
define a layer is

\verbatim
thickness = thickness of the layer
top radii = list of the top cone radii
bottom radii = list of the bottom cone radii
media = list of media names, one for each region
\endverbatim

The meaning of the \c thickness key should be clear. There must be exactly the same number \f$n\f$
of inputs to the other 3 keys defining \f$n\f$ regions filled with the media specified in the \c
media key. Note that the radii must be given in increasing order. Note also that the cone stack is
the only geometry type that must define its media, for all other geometry types media definition
can be done either within the definition of the geometry or "from the outside", \em i.e. by some
other composite geometry using this geometry as one of its components.

Sometimes the bottom radii of the cones in a layer can be the same as the top radii of the cones in
the next layer (\em e.g. when defining the flattening filter of a medical linear accelerator). In
such cases, the <code>top radii</code> key can and should be omitted and the geometry will
automatically take the radii from the previous layer. This has the added advantage of speeding up
the region finding algorithm: if the top radii of a layer are the same as the bottom radii of the
previous layer, one does not need to search for the new cone region when the boundary between
layers is crossed but can simply use the cone region from the previous layer.

A cone stack is useful, for instance, for defining the upper portion of the treatment head of
medical linear accelerators. Examples can be found in \c photon_linac.geom, \c car.geom and \c
rz1.geom example geometry files.

*/
class EGS_ConeStack : public EGS_BaseGeometry {

public:

    // constructor (empty cone stack)
    EGS_ConeStack(const EGS_Vector &Xo, const EGS_Vector &A, const string &Name)
        : xo(Xo), a(A), nl(0), nltot(0), nmax(0), same_Rout(true), Rout(0), Rout2(0),
          EGS_BaseGeometry(Name) {
        a.normalize();
    }

    // destructor
    ~EGS_ConeStack() {
        clear(true);
    }

    // add a layer
    void addLayer(EGS_Float thick, const vector<EGS_Float> &rtop,
                  const vector<EGS_Float> &rbottom,
                  const vector<string> &med_names);

    // get medium
    int medium(int ireg) const {
        int il = ireg/nmax;
        int ir = ireg - il*nmax;
        return cones[il][ir]->medium(0);
    }

    // isInside
    bool isInside(const EGS_Vector &x) {
        EGS_Float p = x*a;
        if (p < pos[0] || p > pos[nl]) {
            return false;
        }
        int il = findRegion(p,nl+1,pos);
        return cones[il][nr[il]-1]->isInside(x);
    }

    // isWhere
    int isWhere(const EGS_Vector &x) {
        EGS_Float p = x*a;
        if (p < pos[0] || p > pos[nl]) {
            return -1;
        }
        int il = findRegion(p,nl+1,pos);
        int ir = isWhere(il,x);
        return ir < 0 ? -1 : il*nmax+ir;
    }

    // inside
    int inside(const EGS_Vector &x) {
        return isWhere(x);
    }

    // isRealRegion
    bool isRealRegion(int ireg) const {
        if (ireg < 0 || ireg >= nreg) {
            return false;
        }
        int il = ireg/nmax;
        int ir = ireg - il*nmax;
        return (ir < nr[il]);
    }

    // howfar
    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed=0, EGS_Vector *normal=0) {

        EGS_Float xp = x*a;
        EGS_Float up = u*a;
        int dir;

        // inside conestack
        if (ireg >= 0) {
            EGS_Float tp;
            int il = ireg/nmax;                     // layer index

            // calculate distance to plane
            if (up > 0) {                           // u points along conestack axis
                dir = 1;                            // positive direction
                tp = (pos[il+1]-xp)/up;             // distance to 'next' plane
            }
            else if (up < 0) {                      // u points against conestack axis
                dir = -1;                           // negative direction
                tp = (pos[il]-xp)/up;               // distance to 'previous' plane
            }
            else {                                  // u is perpendicular to a (u*a = 0)
                dir = 0;                            // null direction
                tp = 1e30;                          // init to large distance
            }
            bool hitp = false;                      // assume we don't hit the plane
            if (tp <= t) {                          // check against maximum distance t
                hitp = true;                        // we hit the plane
                t = tp;                             // set t = distance to plane
            }

            // distance to outer cone
            int ir = ireg - il*nmax;                // cone index in current layer
            bool hitc = false;                      // assume we don't hit the cone
            int irnew = cones[il][ir]->howfar(0,x,u,t,newmed,normal);
            if (irnew < 0) {
                hitc = true;                        // hit the next cone
                irnew = ir < nr[il]-1 ? ir+1 : -1;  // next cone region, or -1 if beyond last cone
            }

            // distance to inner cone (for all but innermost cone)
            if (ir > 0) {
                int irnew1 = cones[il][ir-1]->howfar(-1,x,u,t,newmed,normal);
                if (!irnew1) {
                    hitc = true;                    // hit the previous cone
                    irnew = ir-1;                   // previous cone region
                }
            }

            // hit a cone boundary
            if (hitc) {
                if (irnew < 0) {
                    return irnew;    // we are moving out of conestack, return -1
                }
                if (newmed) {
                    *newmed = cones[il][irnew]->medium(0);
                }
                return il*nmax + irnew;
            }

            // did not hit a plane (nor a cone: should not happen!)
            if (!hitp) {
                return ireg;
            }

            // change layer
            int ilnew = il + dir;                   // new layer index
            if (normal) {
                *normal = dir > 0 ? a*(-1) : a;
            }
            if (ilnew < 0 || ilnew >= nl) {
                return -1;    // beyond conestack bounding planes => outside
            }

            // next layer radii are congruent (add nmax to ireg)
            if (dir > 0 && flag[il] >= 2) {
                if (newmed) {
                    *newmed = cones[il+1][ir]->medium(0);
                }
                return ireg+nmax;
            }

            // previous layer radii are congruent (subtract nmax from ireg)
            if (dir < 0 && (flag[il] == 1 || flag[il] == 3)) {
                if (newmed) {
                    *newmed = cones[il-1][ir]->medium(0);
                }
                return ireg-nmax;
            }

            // figure out region index in new layer
            il += dir;                              // new layer index
            EGS_Vector tmp(x+u*t);                  // position of the hit on new layer
            ir = isWhere(il,tmp);                   // get region index in that layer
            if (ir<0) {
                return ir;    // moved out of constack
            }
            if (newmed) {
                *newmed = cones[il][ir]->medium(0);
            }
            return il*nmax+ir;                      // return overall new region index
        }

        // outside of conestack

        EGS_Float  ttot=0;                          // cumulative travel distance
        EGS_Float  tp;                              // distance to next plane
        EGS_Vector tmp(x);                          // running position
        int il;                                     // current layer index

        // outside the first boundary plane
        if (xp <= pos[0]) {
            if (up <= 0) {
                return ireg;    // moving away from conestack: no hit
            }
            tp = (pos[0]-xp)/up;                    // distance to first boundary plane
            if (tp > t) {
                return ireg;    // distance greater than maximum t: no hit
            }
            tmp += u*tp;                            // move to first plane
            ttot = tp;                              // increase cumulative travel distance
            il   = 0;                               // index of current layer
            int ir = isWhere(0, tmp);               // region index in first layer
            if (ir >= 0) {                          // hit inside a cone in first layer: we're done
                //***************************************************************
                // EMH April 12 2013: Called from outside, but isWhere reports it is inside.
                // This is due to either the particle at a boundary or to round-off errors near a boundary.
                // To avoid getting into an infinite loop, check again if particle exits
                // the geometry from region ir.
                //
                // FT December 4 2013:
                // this isWhere call is using the tmp position and the LOCAL ir region number in
                // layer 0 of the ConeStack, so it is not inconsistent if x is outside and ir>=0.
                // BUT indeed we may be glancing on a "corner" of the ConeStack, so we should still
                // check if a subsequent call to howfar(ir,tmp,...) takes us outside within epsilon.
                // It that case we are not really entering the geometry.
                EGS_Float tb = 1e30;
                int inew_g = howfar(ir,tmp,u,tb,0,normal);
                if (inew_g < 0 && tb <= epsilon) {
                    return ireg;    // exits geometry
                }
                //***************************************************************
                t = tp;                             // distance to hit is distance to plane
                if (newmed) {
                    *newmed = cones[0][ir]->medium(0);
                }
                if (normal) {
                    *normal = a*(-1);
                }
                return ir;                          // return region number
            }
        }

        // outside the last boundary plane
        else if (xp >= pos[nl]) {
            if (up >= 0) {
                return ireg;    // moving away from the conestack: no hit
            }
            tp = (pos[nl]-xp)/up;                   // distance to last boundary plane
            if (tp > t) {
                return ireg;    // distance greater than maximum t: no hit
            }
            tmp += u*tp;                            // hit position on last plane
            ttot = tp;                              // increase cumulative travel distance
            il = nl-1;                              // index of last plane
            int ir = isWhere(il, tmp);              // region index in last layer
            if (ir >= 0) {                          // hit inside a cone in first layer: we're done
                //***************************************************************
                // EMH April 12 2013: Called from outside, but isWhere reports it is inside.
                // This is due to either the particle at a boundary or to round-off errors near a boundary.
                // To avoid getting into an infinite loop, check again if particle exits
                // the geometry from region ir.
                //
                // FT December 4 2013:
                // this isWhere call is using the tmp position and the LOCAL ir region number in
                // layer il of the ConeStack, so it is not inconsistent if x is outside and ir>=0.
                // BUT indeed we may be glancing on a "corner" of the ConeStack, so we should still
                // check if a subsequent call to howfar(il*nmax+ir) takes us outside within epsilon.
                // It that case we are not really entering the geometry.
                EGS_Float tb = 1e30;
                int inew_g = howfar(il*nmax+ir,tmp,u,tb,0,normal);
                if (inew_g < 0 && tb <= epsilon) {
                    return ireg;    // exits geometry
                }
                //***************************************************************
                t = tp;                             // distance to hit is distance to plane
                if (newmed) {
                    *newmed = cones[il][ir]->medium(0);
                }
                if (normal) {
                    *normal = a;    // set normal
                }
                return il*nmax+ir;                  // return region number
            }
        }

        // outside conestack, but within conestack boundary planes
        else {

            if (same_Rout) {
                // all outer "cones" are actually cylinders and all have the same radius. This
                // simplifies the logic a lot: just need to check against outer cylindrical cone of
                // the first layer: cones[0][nr[0]-1]. Don't forget to use temporary and normal
                // pointers here so that normal is only updated if the hit lies within the
                // conestack boundary planes!

                EGS_Float tt = t;
                EGS_Vector tmp_normal;

                //***************************************************************
                // EMH, April 12 2013: To prevent round-off error situations
                // where particles are stuck near boundaries or when a particle
                // seats at a boundary, check both, where it is, and whether it exits
                // the geometry. irnow SHOULD be -1, but if a particle is at a boundary or
                // very close to one, isWhere returns irnow >= 0 values.
                int irnow = isWhere(x);// irnow > 0 indicates round-off error near boundary
                // irnow = -1 indicates proper call from outside
                // howfar call using irnow instead of -1 in the hope particle gets out of boundary
                //
                // int irnew = cones[0][nr[0]-1]->howfar(irnow,x,u,tt,0,&tmp_normal);
                //
                // FT December 10 2013:
                // the howfar call above does not work because it uses irnow for the call to
                // SimpleCone->howfar: SimpleCone regions can only be 0 (inside) or -1 (outside).
                // If we find the inconsistent condition irnow >= 0 (inside, but call from outside),
                // then we are on a boundary. We see if howfar takes us out within epsilon.
                // If so, then we are not really entering the geometry.

                // fp inconsistency: irnow >= 0 (inside) but called with ireg = -1 (outside)
                if (irnow >= 0) {
                    EGS_Float tb = 1e30;
                    int inew_g = howfar(irnow,x,u,tb,0,&tmp_normal);
                    if (inew_g < 0 && tb <= epsilon) {
                        return ireg;    // exits geometry
                    }
                }
                //***************************************************************

                int irnew = cones[0][nr[0]-1]->howfar(ireg,x,u,tt,0,&tmp_normal);

                if (irnew < 0) {
                    return -1;    // no hit
                }
                EGS_Float aux = xp + up*tt;         // axis position of hit point on outer cylinder
                if (aux < pos[0] || aux > pos[nl] ||// beyond conestack bounding planes
                        (aux == pos[0]  && up <= 0)   ||// on first plane, going out
                        (aux == pos[nl] && up >= 0)) {  // on last plane, going out
                    return -1;                      // => no hit: we're done
                }
                il = findRegion(aux,nl+1,pos);      // layer index for axis position aux
                if (newmed) {
                    *newmed = cones[il][nr[il]-1]->medium(0);
                }
                if (normal) {
                    *normal = tmp_normal;    // set normal to normal from howfar
                }
                t = tt;                             // set distance
                return il*nmax + nr[il]-1;          // return region index
            }

            // get layer index for current position, projected on the conestack axis
            il = findRegion(xp,nl+1,pos);

            // guard agains round-off errors by checking if the position
            // is inside the outer cone in this layer (it shouldn't be, as the
            // particle is outside) and returning if it is.
            bool isc = cones[il][nr[il]-1]->isInside(x);
            if (isc) {
                // IK, March 7 2008: same problem as in CD geometry.
                // we think we are outside but we just found we are inside.
                // Hopefully a roundoff problem.
                EGS_Float tp = 1e30;
                if (up > 0) {
                    dir = 1;
                    tp = (pos[il+1] - xp)/up;
                }
                else if (up < 0) {
                    dir = -1;
                    tp = (pos[il] - xp)/up;
                }
                if (tp < epsilon) {
                    il += dir;
                    if (il < 0 || il >= nl) {
                        return ireg;
                    }
                    isc = cones[il][nr[il]-1]->isInside(x);
                }

                if (isc) {
                    EGS_Float tc = 1e30;
                    int isc_new = cones[il][nr[il]-1]->howfar(0,x,u,tc);
                    if (!(isc_new < 0 && tc < epsilon)) {
                        egsWarning("EGS_ConeStack::howfar: called from the outside"
                                   " but I find x=(%g,%g,%g) to be inside\n", x.x,x.y,x.z);
                        egsWarning("layer=%d distance to planes=%g\n",il,tp);
                        egsWarning("distance to outer cone=%g\n",tc);
                        error_flag = 1;
                        return ireg;
                    }
                }
            }
        }

        // traverse layers until we hit a cone, or else move beyond conestack boundary planes
        while (1) {

            // calculate distance to next plane boundary
            if (up > 0) {                           // moving along conestack axis a
                dir = 1;                            // positive direction
                tp = (pos[il+1] - xp)/up;           // total distance to next plane
            }
            else if (up < 0) {                      // moving against conestack axis a
                dir = -1;                           // negative direction
                tp = (pos[il] - xp)/up;             // total distance to previous plane
            }
            else {                                  // moving perpendicular to axis (u*a = 0)
                dir = 0;                            // null direction
                tp  = 1e30;                         // init to large distance
            }

            // distance to outer cone in layer 'il'
            EGS_Float tt = t - ttot;                // remaining maximum distance
            int tmp_med;
            EGS_Vector tmp_normal;
            int irnew = cones[il][nr[il]-1]->howfar(-1,tmp,u,tt,&tmp_med,&tmp_normal);
            if (!irnew) {                           // hit the outer cone
                if (tp > ttot + tt) {               // plane is further than cone: we're done
                    t = ttot+tt;                    // final distance to conestack
                    if (newmed) {
                        *newmed = tmp_med;    // set media
                    }
                    if (normal) {
                        *normal = tmp_normal;    // set normal
                    }
                    return il*nmax+nr[il]-1;        // return final region index
                }
            }
            if (tp > t || !dir) {
                break;    // guard against glancing hits
            }
            il += dir;                              // move to previous or next layer
            if (il < 0 || il >= nl) {
                break;    // enforce conestack bounding planes
            }
            ttot = tp;                              // increase cumulative travel distance
            tmp = x + u*tp;                         // move along to position hit on next plane

            int itest = isWhere(il,tmp);            // check where we are in next layer
            if (itest >= 0) {                       // we are inside a cone in next layer
                t = ttot;                           // update distance
                if (newmed) {
                    *newmed = cones[il][itest]->medium(0);
                }
                if (normal) {
                    *normal = dir > 0 ? a*(-1) : a;
                }
                return il*nmax + itest;             // return final region index
            }
        }
        return ireg;
    }

    // hownear
    EGS_Float hownear(int ireg, const EGS_Vector &x) {

        EGS_Float xp = x*a;                         // current position along the axis
        EGS_Float tp, tc;                           // distances

        // inside the conestack
        if (ireg >= 0) {
            int il = ireg/nmax;                     // current layer index
            int ir = ireg - il*nmax;                // region index in current layer
            tp = min(xp-pos[il],pos[il+1]-xp);      // min of distance to planes on either side
            tc = cones[il][ir]->hownear(0,x);       // distance to outer conical boundary
            if (ir > 0) {
                tc = min(tc,cones[il][ir-1]->hownear(-1,x));    // to inner conical boundary
            }
            return min(tp,tc);                      // return minimum distance
        }

        // outside the conestack; just check distances to layer planes, which is overkill, but in
        // general it would be worse to check hownear on the outer cones in all layer! To mitigate
        // hownear calls to layer planes, one can inscribe the conestack in a fitting envelope.

        if (xp <= pos[0]) {
            return pos[0] - xp;    // distance to first layer plane
        }
        if (xp >= pos[nl]) {
            return xp - pos[nl];    // distance to last layer plane
        }
        int il = findRegion(xp,nl+1,pos);           // find current layer index
        tp = min(xp-pos[il],pos[il+1]-xp);          // min of distance to planes on either side
        return min(tp,cones[il][nr[il]-1]->hownear(-1,x));  // min dist. to planes and outer cone

    }

    // getMaxStep
    int getMaxStep() const {
        int nstep = 0;
        for (int j=0; j<nl; ++j) {
            nstep += 2*nr[j] + 1;
        }
        return nstep + 1;
    }

    // getType
    const string &getType() const {
        return type;
    }

    // printInfo
    void printInfo() const;

    // nLayer
    int  nLayer() const {
        return nl;
    }

    // shiftLabels
    void shiftLabelRegions(const int i, const int index) {
        for (int k=0; k<labels[i].regions.size(); k++) {
            labels[i].regions[k] += index*nmax;
        }
    }



protected:

    EGS_Vector  xo;             // the position of the top layer
    EGS_Vector  a;              // the axis
    int         nl;             // number of layers
    int         nltot;          // size of the arrays
    int         nmax;           // max. number of radii in all layers
    bool        same_Rout;      // true, if all layers have same outer radius
    EGS_Float   Rout, Rout2;    // outer radius (if same_Rout is true)
    EGS_Float   *pos;           // the plane positions dividing the layers
    int         *nr;            // number of radii in each layer
    int         *flag;          // a flag for each layer:
    //   = 0 -> top and bottom radii different from adjacent layers
    //   = 1 -> top radii are the same as bottom of previous layer
    //   = 2 -> bottom radii are the same as top of next layer
    //   = 3 -> top and bottom radii same as adjacent layers
    EGS_SimpleCone ** *cones;   // the cones for each layer.
    static string type;

    // resize
    void resize();

    // clear
    void clear(bool);

    // isWhere
    inline int isWhere(int il, const EGS_Vector &x) {
        if (!cones[il][nr[il]-1]->isInside(x)) {
            return -1;
        }
        for (int j=0; j<nr[il]-1; j++) {
            if (cones[il][j]->isInside(x)) {
                return j;
            }
        }
        return nr[il]-1;
    }
};

#endif
