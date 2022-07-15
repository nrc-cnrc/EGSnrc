/*
###############################################################################
#
#  EGSnrc egs++ roundrect cylinders geometry headers
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
#  Author:          Manuel Stoeckl, 2016
#
#  Contributors:    Frederic Tessier
#                   Hubert Ho
#                   Reid Townson
#
###############################################################################
#
#  A set of concentric rounded rectangle cylinders.
#
###############################################################################
*/


/*! \file egs_roundrect_cylinders.h
 *  \brief A set of concentric rounded rectangular cylinders: header
 *  \author Manuel Stoeckl
 */

#ifndef EGS_CYLINDERS_
#define EGS_CYLINDERS_

#include "egs_base_geometry.h"
#include "egs_math.h"
#include "egs_functions.h"
#include "egs_projectors.h"

#include <vector>
using namespace std;

#ifdef WIN32

    #ifdef BUILD_ROUNDRECT_CYLINDERS_DLL
        #define EGS_ROUNDRECT_CYLINDERS_EXPORT __declspec(dllexport)
    #else
        #define EGS_ROUNDRECT_CYLINDERS_EXPORT __declspec(dllimport)
    #endif
    #define EGS_ROUNDRECT_CYLINDERS_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_ROUNDRECT_CYLINDERS_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_ROUNDRECT_CYLINDERS_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_ROUNDRECT_CYLINDERS_EXPORT
        #define EGS_ROUNDRECT_CYLINDERS_LOCAL
    #endif

#endif

/*! \brief A set of concentric rounded rectangles

  \ingroup Geometry
  \ingroup ElementaryG

This template class implements the geometry methods for a set of
\f$N\f$ concentric founded rectangular cylinders which define a geometry of
\f$N\f$ regions. It can be specialized to orient along the x-, y-, z-, or an
arbitrary axis. Similar comments as in the case of
\link EGS_PlanesT planes \endlink
apply for the motivation for having 4 different rounded rectangular
cylinder classes. Although the cylinder axis by default passes through the
origin, the point through which the main axis passes can be modified.
Sets of cylinders at another location can be obtained by using a
\link EGS_TransformedGeometry
transformed geometry. \endlink The convention
for region numbering is that the region inside the inner-most cylinder
is region 0 with region index increasing towards the last cylinder in the
set. Note also that the geometry defined by a set of rounded rectangular
cylinders is unlimited in the direction along the cylinder axis.

The additional keys needed to construct a set of cylinders are
\verbatim

type = EGS_RoundRectCylinders or EGS_RoundRectCylindersXY or EGS_RoundRectCylindersYZ or EGS_RoundRectCylindersXZ
radii    = list of fillet radii
x-widths = list of cylinder half-widths in the x-direction
y-widths = list of cylinder half-widths in the y-direction
x-axis   = ax, ay, az
y-axis   = bx, by, bz
midpoint = px, py, pz

\endverbatim
The \c x-axis and \c y-axis keys are only needed for an object of type
EGS_RoundRectCylinders (and in fact are ignored for xz-, yz- or xz-cylinders).
The axis vectors don't need to be normalized to unity.

Radii and half-widths need to be given in increasing order. Different rounded
rectangles may not intersect.

A simple example:
\verbatim
:start geometry definition:

    ### a rounded rectangle cylinder
    :start geometry:
        name      = rounded_rect_cyl
        library   = egs_roundrect_cylinders
        type      = EGS_RoundRectCylindersXY
        x-widths  = 1 2
        y-widths  = 0.5 1
        radii     = 0.1 0.5
        :start media input:
            media = red green
            set medium = 1 1
        :stop media input:
    :stop geometry:

    ### planes to cut the cylinder
    :start geometry:
        name      = cd_planes
        library   = egs_planes
        type      = EGS_Zplanes
        positions = -2 2
    :stop geometry:

    ### cut cylinder with planes
    :start geometry:
        name      = rounded_rect
        library   = egs_cdgeometry
        base geometry = cd_planes
        set geometry  = 0 rounded_rect_cyl
    :stop geometry:

    ### simulation geometry
    simulation geometry = rounded_rect

:stop geometry definition:
\endverbatim
\image html egs_roundrect_cylinders.png "A simple rounded rectangle cylinders example"

A more complex example of the usage of rounded rectangle cylinder sets can be
found in the \c roundedrect_cylinders.geom sample geometry input file.
*/

template <class Tx, class Ty>
class EGS_ROUNDRECT_CYLINDERS_EXPORT EGS_RoundRectCylindersT :
    public EGS_BaseGeometry {

protected:

    EGS_Float *ax,  //!< Widths along the axis defined by Ax,
              *ay,  //!< Widths along the axis defined by Ay,
              *ar;  //!< Radii of roundings for each rect
    EGS_Vector xo;  //!< A point on the cylinder axis
    Tx Ax;          //!< The projection operator for the first ('x') axis
    Ty Ay;          //!< The projection operator for the second ('y') axis
    string mytype;  //!< The cylinder type

public:

    /*! \brief Desctructor.

    */
    ~EGS_RoundRectCylindersT() {
        if (nreg > 0) {
            delete [] ax;
            delete [] ay;
            delete [] ar;
        }
    };

    /*! \brief Construct a set of concentric rounded rectangles.

    Construct a set of concentric rounded rectangles named \a Name using
    the projection operators \a A_x and \a A_y.

    Parameters \a x_wid, \a y_wid, and \a rad should all have the same length
    and correspond to the half-widths in the projected x-direction and
    projected y-direction, as well as the radii of the fillets on the
    rectangles.

    \a position is a point on the cylinder axis
    */
    EGS_RoundRectCylindersT(const vector<EGS_Float> &x_wid,
                            const vector<EGS_Float> &y_wid,
                            const vector<EGS_Float> &rad,
                            const EGS_Vector &position, const string &Name,
                            const Tx &A_x, const Ty &A_y) : EGS_BaseGeometry(Name),
        xo(position), Ax(A_x), Ay(A_y) {
        int nc = rad.size();
        if (nc>0) {
            ax = new EGS_Float [nc];
            ay = new EGS_Float [nc];
            ar = new EGS_Float [nc];
            for (int i=0; i<nc; i++) {
                ax[i] = x_wid[i];
                ay[i] = y_wid[i];
                ar[i] = rad[i];
            }
            nreg=nc;
        }
        mytype = Ax.getType() + Ay.getType();
    }

    bool isInside(const EGS_Vector &src) {
        return inRing(nreg-1, src);
    };

    int isWhere(const EGS_Vector &x) {
        if (!isInside(x)) {
            return -1;
        }
        for (int j=0; j<nreg; j++) {
            if (inRing(j, x)) {
                return j;
            }
        }
        return nreg-1;
    };

    int inside(const EGS_Vector &x) {
        return isWhere(x);
    };

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed = 0, EGS_Vector *normal = 0) {
        // This _can_ be constructed using just hownear
        // and inside, but that's slow (using golden section search & smoothness hacks)

        EGS_Vector xp(x-xo);
        // (px,py),(qx,qy) is point + vector in projected space
        EGS_Float px = Ax*xp;
        EGS_Float py = Ay*xp;
        EGS_Float qx = Ax*u;
        EGS_Float qy = Ay*u;

        if (qx == 0.0 && qy == 0.0) {
            // perpendicular: distance not capped. No intersection
            // so newmed, normal need not be set
            return ireg;
        }

        // Step 1: Transform things so that p is in the first quadrant
        int xflip = px < 0 ? -1 : 1;
        int yflip = py < 0 ? -1 : 1;
        px *= xflip;
        py *= yflip;
        qx *= xflip;
        qy *= yflip;

        EGS_Float dist = -1;
        EGS_Float norm_x = 0.;
        EGS_Float norm_y = 0.;

        // Step 2: Test for an inside collision if applicable
        int inew = ireg;
        bool haveInternal = false;
        if ((ireg < 0 || ireg > 0)) {
            // Starting outside.
            EGS_Float ux,uy;
            int n = ireg < 0 ? nreg - 1 : ireg - 1;
            bool found = findLineIntersection(n, px, py, qx,qy,
                                              norm_x, norm_y, ux, uy);
            if (found) {
                bool forward = (qx * (ux-px) + qy * (uy - py) > 0) || (px == ux && py == uy);
                if (forward) {
                    inew = n;
                    dist = sqrt((ux-px)*(ux-px) + (uy-py)*(uy-py));
                    haveInternal = true;
                }
            }
        }

        // Step 3: Test for an outside collision if applicable
        if (ireg >= 0 && !haveInternal) {
            // To find interior intersections, extend ray and look back
            // -- any line from the inside ought to have 1 intersection.
            EGS_Float sc = 1 / sqrt(qx*qx+qy*qy);
            // Upper bound on interior diameter
            EGS_Float skip = 2 * (ax[ireg] + ay[ireg]);
            EGS_Float bx = px + qx * skip * sc;
            EGS_Float by = py + qy * skip * sc;
            EGS_Float xeflip = bx < 0 ? -1 : 1;
            EGS_Float yeflip = by < 0 ? -1 : 1;
            bx *= xeflip;
            by *= yeflip;
            EGS_Float cx = -qx*xeflip;
            EGS_Float cy = -qy*yeflip;
            EGS_Float ux=0.,uy=0.;
            bool found = findLineIntersection(ireg, bx, by, cx, cy,
                                              norm_x, norm_y, ux, uy);
            if (found) {
                norm_x *= -xeflip;
                norm_y *= -yeflip;
                ux *= xeflip;
                uy *= yeflip;

                dist = sqrt((ux-px)*(ux-px) + (uy-py)*(uy-py));
                inew = (nreg == ireg + 1) ? -1 : ireg+1;
            }
            else {
                // failure occurs occasionally due to rounding error
            }
        }

        // Set distance, new medium, normal vector (if not null), and return inew

        // Apply changes if beam is close enough
        if (dist < 0) {
            // Stay in current region (out of dist)
            return ireg;
        }

        // Modify dist to take into account unprojected component...
        dist *= sqrt(u.length2() / (qx*qx + qy*qy));
        if (dist <= t) {
            // Add a tiny amount to counteract backward rounding drift
            t = dist + boundaryTolerance;
            if (newmed) {
                *newmed = inew < 0 ? -1 : medium(inew);
            }
            if (normal) {
                *normal = Ax.normal()*norm_x*xflip + Ay.normal() * norm_y * yflip;
            }
            return inew;
        }
        else {
            return ireg;
        }
    };

    EGS_Float hownear(int ireg, const EGS_Vector &src) {
        EGS_Float x = fabs(Ax*(src-xo));
        EGS_Float y = fabs(Ay*(src-xo));

        if (ireg < 0) {
            return getRingDist(nreg - 1, x, y);
        }
        if (ireg == 0) {
            return getRingDist(0, x, y);
        }
        EGS_Float outdist = getRingDist(ireg, x, y);
        EGS_Float indist = getRingDist(ireg-1, x, y);
        return fmin(outdist,indist);
    };

    const string &getType() const {
        return mytype;
    };

    void printInfo() const {
        EGS_BaseGeometry::printInfo();
        egsInformation("Type = %s\n",mytype.c_str());
        egsInformation(" midpoint of cylinders = (%g,%g,%g)\n",xo.x,xo.y,xo.z);
        int j;
        egsInformation(" radii along 'x' =");
        for (j=0; j<nreg; j++) {
            egsInformation(" %g",ax[j]);
        }
        egsInformation("\n");
        egsInformation(" radii along 'y' =");
        for (j=0; j<nreg; j++) {
            egsInformation(" %g",ay[j]);
        }
        egsInformation("\n");
        egsInformation("===================================================\n");
    };

private:

    bool inRing(int ireg, const EGS_Vector &src) {
        EGS_Vector rc(src-xo);
        EGS_Float x = fabs(Ax*rc);
        EGS_Float y = fabs(Ay*rc);
        if (x >= ax[ireg] || y >= ay[ireg]) {
            return false;
        }
        EGS_Float dx = (x - (ax[ireg]-ar[ireg]));
        EGS_Float dy = (y - (ay[ireg]-ar[ireg]));
        if (dx < 0 || dy < 0) {
            return true;
        }
        bool incirc = (dx*dx + dy*dy) <= ar[ireg]*ar[ireg];
        return incirc;
    }

    EGS_Float getRingDist(int ireg, EGS_Float x, EGS_Float y) {
        // Either outside or inside of ring
        EGS_Float dx = (x - (ax[ireg] - ar[ireg]));
        EGS_Float dy = (y - (ay[ireg] - ar[ireg]));
        if (dx > 0 && dy > 0) {
            // Affected by curved section
            return fabs(sqrt(dx*dx+dy*dy)-ar[ireg]);
        }
        else if (dy >= 0) {
            // Y coordinate near edge
            return fabs(y - ay[ireg]);
        }
        else if (dx >= 0) {
            return fabs(x - ax[ireg]);
        }
        else {
            // Inside of the corner centers
            return fmin(fabs(x - ax[ireg]), fabs(y - ay[ireg]));
        }
    }

    bool findLineIntersection(int ring,
                              EGS_Float px, EGS_Float py, EGS_Float qx, EGS_Float qy,
                              EGS_Float &norm_x, EGS_Float &norm_y, EGS_Float &ux, EGS_Float &uy) {
        if (qx == 0.0) {
            // Straight horiz.
            if (px <= ax[ring] - ar[ring]) {
                norm_x = 0;
                norm_y = 1;
                ux = px;
                uy = ay[ring];
                return true;
            }
            else if (px <= ax[ring]) {
                EGS_Float dx = px - (ax[ring] - ar[ring]);
                EGS_Float dy = sqrt(ar[ring]*ar[ring]-dx*dx);
                EGS_Float md = 1/sqrt(dx*dx+dy*dy);
                norm_x = dx * md;
                norm_y = dy * md;
                ux = px;
                uy = dy + ay[ring]- ar[ring];
                return true;
            }
            else {
                //  out of bounds
                return false;
            }
        }
        else if (qy == 0.0) {
            // Straight vert
            if (py <= ay[ring] - ar[ring]) {
                norm_x = 1;
                norm_y = 0;
                ux = ax[ring];
                uy = py;
                return true;
            }
            else if (py <= ay[ring]) {
                EGS_Float dy = py - (ay[ring] - ar[ring]);
                EGS_Float dx = sqrt(ar[ring]*ar[ring]-dy*dy);
                EGS_Float md = 1/sqrt(dx*dx+dy*dy);
                norm_x = dx * md;
                norm_y = dy * md;
                ux = dx + ax[ring]- ar[ring];
                uy = py;
                return true;
            }
            else {
                //  out of bounds
                return false;
            }
        }
        else {
            // Flip again, so that both ix, iy are in the positive region
            EGS_Float ix = px + (qx / qy) * (ay[ring] - py);
            EGS_Float iy = py + (qy / qx) * (ax[ring] - px);

            // Now, casework:
            bool topface = fabs(ix) <= ax[ring] - ar[ring];
            bool sideface = fabs(iy) <= ay[ring] - ar[ring];

            if (fabs(ix) > ax[ring] + boundaryTolerance && fabs(iy) > ay[ring] + boundaryTolerance) {
                // fast case: definite miss
                return false;
            }

            bool sidehit = false;
            bool tophit = false;
            if (topface && sideface) {
                EGS_Float d2side = (px-ax[ring])*(px-ax[ring])+(py-iy)*(py-iy);
                EGS_Float d2top = (py-ay[ring])*(py-ay[ring])+(px-ix)*(px-ix);
                if (d2side < d2top) {
                    sidehit = true;
                }
                else {
                    tophit = true;
                }
            }
            else if (topface && py > ay[ring]) {
                tophit = true;
            }
            else if (sideface && px > ax[ring]) {
                sidehit = true;
            }

            // top/side hit not simultaneously true
            if (tophit) {
                ux = ix;
                uy = ay[ring];
                norm_x = 0;
                norm_y = 1;
                return true;
            }
            else if (sidehit) {
                ux = ax[ring];
                uy = iy;
                norm_x = 1;
                norm_y = 0;
                return true;
            }
            else {
                // Use normalized vectors for simplicity
                EGS_Float f = 1 / sqrt(qx*qx+qy*qy);
                EGS_Float nqx = qx * f;
                EGS_Float nqy = qy * f;

                // Construct circle axis nearest to intersections
                EGS_Float rx = (ax[ring] - ar[ring]);
                EGS_Float ry = (ay[ring] - ar[ring]);
                // Reinsert once warpage understood

                if (px <= ax[ring] && py <= ay[ring]) {
                    // Looking from inside corner
                }
                else if (px <= ax[ring]) {
                    // Looking from top
                    if (ix < 0) {
                        rx *= -1;
                    }
                }
                else if (py <= ay[ring]) {
                    // Looking from side
                    if (iy < 0) {
                        ry *= -1;
                    }
                }
                else {
                    // Looking from corner
                    if (ix < 0 && iy < 0) {
                        // Should never happen
                    }
                    else if (ix < 0) {
                        rx *= -1;
                    }
                    else if (iy < 0) {
                        ry *= -1;
                    }
                }

                // S1: intersect perp. radial ray with line -- note that it's just (-y,x)
                // S1: Find minimum distance between ray and line
                EGS_Float s = (nqy * (px - rx) - nqx * (py - ry));

                // S2: check if int too close/far
                if (fabs(s) <= ar[ring]) {
                    // S3: move pyth in appropriate direction along line
                    EGS_Float v = sqrt(fmax(ar[ring]*ar[ring] - s*s,0));
                    // Pick closer point... (x-product with norm is < 0)
                    norm_x = -nqy * (-s);
                    norm_y = nqx * (-s);

                    if (nqx * (norm_x + nqx*v) + nqy * (norm_y + nqy*v) < 0) {
                        norm_x += nqx*v;
                        norm_y += nqy*v;
                    }
                    else {
                        norm_x -= nqx*v;
                        norm_y -= nqy*v;
                    }
                    ux = rx + norm_x;
                    uy = ry + norm_y;
                    // |norm| is expected to equal ar[ring]
                    norm_x *= 1/ar[ring];
                    norm_y *= 1/ar[ring];
                    return true;
                }
                else {
                    return false;
                }
            }
        }
    }
};

#endif
