/*
###############################################################################
#
#  EGSnrc egs++ polygon
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


/*! \file egs_polygon.cpp
 *  \brief Polygons implementation
 *  \IK
 */

#include "egs_polygon.h"

bool EGS_2DPolygon::checkCCW(const vector<EGS_2DVector> &points) {
    double A = 0;
    for (int j=0; j<points.size()-1; j++) {
        A += (points[j].x*points[j+1].y - points[j+1].x*points[j].y);
    }
    if (A > 0) {
        return true;
    }
    else {
        return false;
    }
}

EGS_2DPolygon::~EGS_2DPolygon() {
    delete [] a;
    delete [] p;
    delete [] d;
    delete [] pc;
    delete [] uj;
    if (!is_convex) {
        delete cpol;
        for (int j=0; j<ncut; j++) {
            delete cut[j];
        }
        delete [] cut;
    }
}

EGS_2DPolygon::EGS_2DPolygon(vector<EGS_2DVector> &points, bool Open) {
    int n = points.size();
    open = Open;
    EGS_2DVector test(points[0]-points[n-1]);
    if (test.length2() > 1e-8) {
        points.push_back(points[0]);
        n++;
    }
    bool ccw = checkCCW(points);
    // Remove points that are too close to each other.
    EGS_2DVector *pp = new EGS_2DVector [n];
    pp[0] = points[0];
    int jj=1;
    int j;
    for (j=1; j<n; j++) {
        EGS_2DVector aux(points[j]-pp[jj-1]);
        if (aux.length2() > 1e-8) {
            pp[jj++] = points[j];
        }
    }
    n = jj;
    // now check for points on a line
    for (j=0; j<n-2; j++) {
        EGS_2DVector aa(pp[j+1]-pp[j]), bb(pp[j+2]-pp[j]);
        if (fabs(aa%bb) < 1e-6) {
            pp[j+1] = pp[j+2];
            for (int i=j+2; i<n; i++) {
                pp[i] = pp[i+1];
            }
            n--;
        }
    }
    np = n;
    p = new EGS_2DVector [np];
    for (j=0; j<n; j++) {
        p[j] = pp[j];
    }
    delete [] pp;
    // now make line normals pointing inwards.
    a = new EGS_2DVector [np-1];
    d = new EGS_Float [np-1];
    uj = new EGS_2DVector [np-1];
    pc = new bool [np-1];
    xmin = 1e30, xmax = -1e30, ymin = 1e30, ymax = -1e30;
    for (j=0; j<np-1; j++) {
        pc[j] = true;
        if (p[j].x < xmin) {
            xmin = p[j].x;
        }
        if (p[j].x > xmax) {
            xmax = p[j].x;
        }
        if (p[j].y < ymin) {
            ymin = p[j].y;
        }
        if (p[j].y > ymax) {
            ymax = p[j].y;
        }
        uj[j] = p[j+1]-p[j];
        if (ccw) {
            a[j] = EGS_2DVector(-uj[j].y,uj[j].x);
        }
        else {
            a[j] = EGS_2DVector(uj[j].y,-uj[j].x);
        }
        a[j].normalize();
        d[j] = a[j]*p[j];
    }
    // now see if the polygon is convex
    is_convex = true;
    if (np == 4) {
        return;    // triangles are always convex.
    }
    open = false; // we can not have open polygons
    for (j=0; j<np-1; j++) {
        int j1 = j+1;
        if (j1 == np-1) {
            j1=0;
        }
        bool is_ok = true;
        for (int i=0; i<np-1; i++) {
            if (i != j && i != j1) {
                if (!inside(j,p[i])) {
                    is_ok=false;
                    break;
                }
            }
        }
        if (!is_ok) {
            is_convex = false;
            pc[j] = false;
        } //break; }
    }

    if (is_convex) {
        return;
    }

    // find a point that is part of the convex hull of this polygon
    int jstart;
    for (j=0; j<np-1; j++) {
        bool is_ok = true;
        for (int i=0; i<np; i++) {
            if (i < j || i > j+1) if (!inside(j,p[i])) {
                    is_ok=false;
                    break;
                }
        }
        if (is_ok) {  // found the point
            jstart = j;
            pp = new EGS_2DVector [np];
            int jj=0;
            int i;
            for (i=j; i<np-1; i++) {
                pp[jj++] = p[i];
            }
            for (i=0; i<=j; i++) {
                pp[jj++] = p[i];
            }
            break;
        }
    }

    vector<EGS_2DVector> chull;
    vector<EGS_2DVector> ipol;
    vector<EGS_2DPolygon *> tmp_pol;
    chull.push_back(pp[0]);
    chull.push_back(pp[1]);
    int nc=2;
    j=2;
    bool doing_chull=true;
    while (1) {
        EGS_2DVector s(pp[j]-chull[nc-1]);
        EGS_2DVector sperp(-s.y,s.x);
        if (!ccw) {
            sperp *= (-1.);
        }
        EGS_Float ds = sperp*chull[nc-1];
        bool all_inside=true;
        for (int i=j+1; i<n; i++) {
            if (sperp*pp[i] < ds) {
                all_inside=false;
                break;
            }
        }
        if (all_inside) {
            if (!doing_chull) {
                ipol.push_back(pp[j]);
                EGS_2DPolygon *newp = new EGS_2DPolygon(ipol);
                tmp_pol.push_back(newp);
                ipol.clear();
            }
            doing_chull = true;
            chull.push_back(pp[j]);
            nc++;
        }
        else {
            if (doing_chull) {
                doing_chull = false;
                ipol.push_back(chull[nc-1]);
            }
            ipol.push_back(pp[j]);
        }
        j++;
        if (j >= n) {
            break;
        }
    }
    cpol = new EGS_2DPolygon(chull);
    ncut = tmp_pol.size();
    cut = new EGS_2DPolygon* [ncut];
    for (j=0; j<ncut; j++) {
        cut[j] = tmp_pol[j];
    }
    delete [] pp;
}
