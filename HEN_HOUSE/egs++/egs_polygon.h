/*
###############################################################################
#
#  EGSnrc egs++ polygon headers
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
#
###############################################################################
*/


/*! \file egs_polygon.h
 *  \brief Polygons header file
 *  \IK
 */

#ifndef EGS_POLYGON_
#define EGS_POLYGON_

#include "egs_vector.h"
#include "egs_projectors.h"
#include "egs_math.h"

#include <vector>

using namespace std;

/*! \brief A class to represent a polygon in a plane (a 2D polygon).

  \ingroup egspp_main

  The EGS_2DPolygon class represents a polygon in a plane.
  It provides various functions which are useful for the implementation
  of the \link EGS_BaseGeometry required geometry methods \endlink
  of prisms and pyramids.
*/
class EGS_EXPORT EGS_2DPolygon {

public:

    /*! \brief Construct a polygon from the 2D points \a points.

      The optional \a Open argument has only a meaning for polygons
      with 3 points (triangles) and makes the triangle to be 'open',
      \em i.e. extend to infinity between the lines defined by the
      first and second point and the first and third point.
    */
    EGS_2DPolygon(vector<EGS_2DVector> &points, bool Open=false);

    /*! \brief Destructor */
    ~EGS_2DPolygon();

    /*! \brief Get the number of points (vertices) in this polygon object */
    int getN() const {
        return np-1;
    };

    /*! \brief Is the polygon convex ? */
    bool isConvex() const {
        return is_convex;
    };

    /*! \brief Get a line normal to the \a j'th polygon edge. */
    EGS_2DVector getNormal(int j) const {
        if (j >= 0 && j < np-1) {
            return a[j];
        }
        else {
            return EGS_2DVector();
        }
    };

    /*! \brief Get the nearest distance from \a x to the polygon.

      The argument \a in must be \c true, if the 2D position \a x is
      inside the polygon, \a false otherwise.
    */
    EGS_Float hownear(bool in, const EGS_2DVector &x) const {
        if (!open) {
            EGS_Float tperp = 1e30;
            bool do_it = true;
            for (int j=0; j<np-1; j++) {
                EGS_2DVector v(x - p[j]);
                EGS_Float lam = uj[j]*v;
                if (lam >= 0 && lam <= uj[j].length2()) {
                    do_it = false;
                    EGS_Float t = fabs(d[j] - x*a[j]);
                    if (t < tperp) {
                        tperp = t;
                    }
                }
                else if (lam < 0 && do_it) {
                    EGS_Float t = v.length();
                    if (t < tperp) {
                        tperp = t;
                    }
                }
                else {
                    do_it = true;
                }
            }
            return tperp;
        }
        EGS_2DVector v(x - p[0]);
        EGS_Float lam = uj[0]*v;
        EGS_Float tperp;
        bool do_it;
        if (lam <= uj[0].length2()) {
            do_it = false;
            tperp = fabs(d[0] - x*a[0]);
        }
        else {
            do_it = true;
            tperp = 1e30;
        }
        v = x - p[1];
        lam = uj[1]*v;
        if (lam >= 0) {
            EGS_Float t = fabs(d[1] - x*a[1]);
            if (t < tperp) {
                tperp = t;
            }
        }
        else if (do_it) {
            EGS_Float t = v.length();
            if (t < tperp) {
                tperp = t;
            }
        }
        return tperp;
    };

    /*! \brief Is the 2D point \a x inside the polygon ? */
    bool isInside(const EGS_2DVector &x) const {
        if (!open &&
                (x.x<xmin || x.x>xmax || x.y<ymin || x.y>ymax)) {
            return false;
        }
        if (is_convex) {
            int nn = open ? np-2 : np-1;
            for (int j=0; j<nn; j++)
                if (!inside(j,x)) {
                    return false;
                }
            return true;
        }
        if (!cpol->isInside(x)) {
            return false;
        }
        for (int j=0; j<ncut; j++) if (cut[j]->isInside(x)) {
                return false;
            }
        return true;
    };

    /*! \brief Will the line defined by position \a x and direction \a u
      intersect the polygon ?

      This function returns \a true,
      if the line defined by position \a x and direction \a u intersects
      the polygon and \a false otherwise. If an intersection exists,
      \a t is set to the distance between \a x and the intersection point.
      In addition, if \a normal is not \c null, it is set to the normal
      to the edge being intersected. The argument \a in must be set to
      \a true, if \a x is inside the ploygon and to \a false otherwise.
    */
    bool howfar(bool in, const EGS_2DVector &x, const EGS_2DVector &u,
                EGS_Float &t, EGS_2DVector *normal = 0) {
        EGS_Float xp, up;
        bool res = false;
        int nn = open ? np-2 : np-1;
        if (in) {
            int jhit;
            for (int j=0; j<nn; j++)  {
                if ((up = u*a[j]) < 0 && (xp = x*a[j]) > d[j]) {
                    EGS_Float tt = d[j] - xp;
                    if (tt >= t*up) {
                        tt /= up;
                        bool ok = is_convex || pc[j];
                        if (!ok) {
                            EGS_Float lam = uj[j]*(x-p[j]+u*tt);
                            if (lam >= 0 && lam < uj[j].length2()) {
                                ok = true;
                            }
                        }
                        if (ok) {
                            t = tt;
                            res = true;
                            jhit = j;
                            //if( normal ) *normal = a[j];
                        }
                    }
                }
            }
            if (res && normal) {
                *normal = a[jhit];
            }
        }
        else {
            int jhit;
            if (open) {
                if ((up = u*a[0]) > 0 && (xp = x*a[0]) < d[0]) {
                    EGS_Float tt = (d[0] - xp)/up;
                    if (tt <= t) {
                        EGS_Float lam = uj[0]*(x-p[0]+u*tt);
                        if (lam < uj[0].length2()) {
                            t = tt;
                            res = true;
                            jhit = 0;
                        }
                    }
                }
                if ((up = u*a[1]) > 0 && (xp = x*a[1]) < d[1]) {
                    EGS_Float tt = (d[1] - xp)/up;
                    if (tt <= t) {
                        EGS_Float lam = uj[1]*(x-p[1]+u*tt);
                        if (lam > 0) {
                            t = tt;
                            res = true;
                            jhit = 1;
                        }
                    }
                }
            }
            else {
                for (int j=0; j<nn; j++)  {
                    if ((up = u*a[j]) > 0 && (xp = x*a[j]) < d[j]) {
                        EGS_Float tt = (d[j] - xp)/up;
                        if (tt <= t) {
                            EGS_Float lam = uj[j]*(x-p[j]+u*tt);
                            if (lam >= 0 && lam < uj[j].length2()) {
                                t = tt;
                                res = true;
                                jhit = j;
                                //if( normal ) *normal = a[j]*(-1);
                            }
                        }
                    }
                }
            }
            if (res && normal) {
                *normal = a[jhit]*(-1);
            }
        }
        return res;
    };

    /*! \brief Get the \a j'th point of this polygon. */
    EGS_2DVector getPoint(int j) const {
        if (j >= 0 && j < np) {
            return p[j];
        }
        else {
            return EGS_2DVector();
        }
    };


private:

    EGS_2DVector *p;   //!< the 2D points
    EGS_2DVector *a;   //!< the line normals pointing inwards
    EGS_2DVector *uj;  //!< the line vectors (i.e. uj[j]=p[j+1]-p[j])
    EGS_Float    *d;   //!< the line positions.
    bool         *pc;  //!< if pc[j] is true, p[j] is part of the convex hull.
    /*! Defines bounding rectangle */
    EGS_Float    xmin, xmax, ymin, ymax;
    int          np;   //!< number of points in the polygon
    bool         is_convex; //!< \c true, if the polygon is convex.
    bool         open; /*!< \c true, if the polygon is open
                         (set in the constructor)*/

    int          ncut;  /*!< number of polygons cutouts from the convex hull
            to make the actual polygon (only relevant for non-convex polygons)*/
    EGS_2DPolygon  **cut; //!< the cutout polygons
    EGS_2DPolygon  *cpol; //!< the convex polygon hull.

    /*! \brief Returns true, if the points \a point are in a
      counter-clockwise order. */
    static bool checkCCW(const vector<EGS_2DVector> &points);
    /*! \brief Is the point \a x inside the \a j'th edge ? */
    bool inside(int j, const EGS_2DVector &x) const {
        if (x*a[j] >= d[j]) {
            return true;
        }
        else {
            return false;
        }
    };

};

/*! \brief A template class for 3D polygons

  \ingroup egspp_main

  This class is a wrap-around the EGS_2DPolygon class for conveniently
  modeling polygons in 3D. It consists of a projector of type T, which projects
  the 3D position and/or direction onto the polygon plane and then
  uses the EGS_2DPolygon and projector methods to implement the various
  geometry related functions needed in the geometry methods of prisms and
  pyramids. The class is implemented as a template so that a single
  implementation can be used for polygons in the x-, y- and z-planes
  (fast) and a general plane in 3D (slower due to the slower projection
  operation).
*/
template <class T>
class EGS_EXPORT EGS_PolygonT {

public:

    /*! \brief Construct a 3D polygon defined by the 2D points \a points
      in the plane defined by the projector \a projector.

      The meaning of the \a Open parameter is the same as in
      EGS_2DPolygon::EGS_2DPolygon().
      */
    EGS_PolygonT(vector<EGS_2DVector> &points, const T &projector,
                 bool Open=false) : p(new EGS_2DPolygon(points,Open)), a(projector) {};

    /*! \brief Destructor */
    ~EGS_PolygonT() {
        delete p;
    };

    /*! \brief Is this polygon convex ? */
    inline bool isConvex() const {
        return p->isConvex();
    };

    /*! \brief Get the number of polygon points */
    inline int getN() const {
        return p->getN();
    };

    /*! \brief Get the \a j'th point */
    inline EGS_Vector getPoint(int j) const {
        return a.getPoint(p->getPoint(j));
    };

    /*! \brief Get the normal to the polygon plane */
    inline EGS_Vector getNormal() const {
        return a.normal();
    };

    /*! ? */
    inline EGS_Vector getNormal(const EGS_2DVector &x) const {
        return a.normal(x);
    };

    /*! \brief Get the normal to the \a j'th polygon edge */
    inline EGS_Vector getNormal(int j) const {
        return a.normal(p->getNormal(j));
    };

    /*! \brief Get the projection of \a x on the polygon plane */
    inline EGS_2DVector getProjection(const EGS_Vector &x) const {
        return a.getProjection(x);
    };

    /*! \brief Get the distance between \a x and the polygon plane */
    inline EGS_Float distance(const EGS_Vector &x) const {
        return a.distance(x);
    };

    /*! \brief Is the 2D point \a xp inside the polygon ? */
    inline bool isInside2D(const EGS_2DVector &xp) const {
        return p->isInside(xp);
    };

    /*! \brief Is the projection of the 3D point \a x on the polygon plane
      inside the polygon ? */
    inline bool isInside2D(const EGS_Vector &x) const {
        return p->isInside(a.getProjection(x));
    };

    /*! \brief Is the 3D point inside the polygon plane ? (\em i.e. on than
      side of the plane to which the plane normal points to) */
    inline bool isInside(const EGS_Vector &x) const {
        return (a.distance(x) >= 0);
    };

    /*! \brief Get the nearest distance between the projection of \a x and
      the polygon outline */
    inline EGS_Float hownear2D(bool in, const EGS_Vector &x) const {
        return p->hownear(in,a.getProjection(x));
    };

    /*! \brief Get the nearest distance between \a x and any point inside
      the polygon */
    inline EGS_Float hownear(bool in, const EGS_Vector &x) const {
        EGS_2DVector pos(a.getProjection(x));
        EGS_Float t1 = fabs(a.distance(x));
        if (p->isInside(pos)) {
            return t1;
        }
        EGS_Float t2 = p->hownear(true,pos);
        return sqrt(t1*t1+t2*t2);
    };

    /*! \brief Does the projection on the polygon plane of the trajectory
      defined by position \a x and direction \a u intersect the polygon ?

      \sa EGS_2DPolygon::howfar()
      */
    inline bool howfar2D(bool in, const EGS_Vector &x, const EGS_Vector &u,
                         EGS_Float &t, EGS_2DVector *normal = 0) const {
        EGS_2DVector dir(a.getProjection(u));
        if (u.length2() < 1e-8) {
            return false;
        }
        return p->howfar(in,a.getProjection(x),dir,t,normal);
    };

    /*! \brief Will the line defined by position \a x and direction \a u
      intersect the polygon plane at a position inside the polygon ?

      The interprtation of the return value and the value of \a in and
      \a t is the same as in EGS_2DPolygon::howfar()
      */
    inline bool howfar(bool in, const EGS_Vector &x, const EGS_Vector &u,
                       EGS_Float &t) const {
        EGS_Float up = a*u;
        if (in && up >= 0  || !in && up <= 0) {
            return false;
        }
        EGS_Float tt = -a.distance(x)/up;
        if (tt <= t) {
            EGS_Vector xp(x + u*tt);
            if (p->isInside(a.getProjection(xp))) {
                t = tt;
                return true;
            }
        }
        return false;
    };

    /*! \brief Get the polygon type */
    const string &getType() const {
        return a.getType();
    };

private:

    EGS_2DPolygon *p;
    T             a;

};

/*! \brief A 3D polygon in the x-plane */
typedef EGS_PolygonT<EGS_XProjector> EGS_PolygonYZ;
/*! \brief A 3D polygon in the y-plane */
typedef EGS_PolygonT<EGS_YProjector> EGS_PolygonXZ;
/*! \brief A 3D polygon in the z-plane */
typedef EGS_PolygonT<EGS_ZProjector> EGS_PolygonXY;
/*! \brief A 3D polygon in any plane */
typedef EGS_PolygonT<EGS_Projector>  EGS_Polygon;

/*! \brief Make a polygon from the 3D points \a points */
EGS_EXPORT EGS_Polygon *makePolygon(const vector<EGS_Vector> &points);

#endif
