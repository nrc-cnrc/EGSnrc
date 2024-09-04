/*
###############################################################################
#
#  EGSnrc egs++ polygon shape
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
#  Contributors:    Randle Taylor
#                   Reid Townson
#
###############################################################################
*/


/*! \file egs_polygon_shape.cpp
 *  \brief A polygon shape
 *  \IK
 */

#include "egs_polygon_shape.h"
#include "egs_polygon.h"
#include "egs_input.h"
#include "egs_functions.h"
#include "egs_math.h"

EGS_TriangleShape::EGS_TriangleShape(const vector<EGS_Float> &points,
                                     const string &Name, EGS_ObjectFactory *f) : EGS_SurfaceShape(Name,f) {
    xo = points[0];
    yo = points[1];
    ax = points[2] - xo;
    ay = points[3] - yo;
    bx = points[4] - xo;
    by = points[5] - yo;
    A = 0.5*fabs(ax*by-ay*bx);
}

EGS_TriangleShape::EGS_TriangleShape(const EGS_Float *points,
                                     const string &Name, EGS_ObjectFactory *f) : EGS_SurfaceShape(Name,f) {
    xo = points[0];
    yo = points[1];
    ax = points[2] - xo;
    ay = points[3] - yo;
    bx = points[4] - xo;
    by = points[5] - yo;
    A = 0.5*fabs(ax*by-ay*bx);
}

EGS_PolygonShape::EGS_PolygonShape(const vector<EGS_Float> &points,
                                   const string &Name, EGS_ObjectFactory *f) : EGS_SurfaceShape(Name,f) {
    int np = points.size();
    n = np/2;
    EGS_Float auxx = points[np-2] - points[0];
    EGS_Float auxy = points[np-1] - points[1];
    EGS_Float *xc, *yc;
    if (auxx*auxx + auxy*auxy > epsilon) {
        n++;
    }
    xc = new EGS_Float [n];
    yc = new EGS_Float [n];
    vector<EGS_2DVector> p1;
    for (int j=0; j<np/2; j++) {
        xc[j] = points[2*j];
        yc[j] = points[2*j+1];
        p1.push_back(EGS_2DVector(xc[j],yc[j]));
    }
    if (n > np/2) {
        xc[n-1] = points[0];
        yc[n-1] = points[1];
        p1.push_back(EGS_2DVector(xc[n-1],yc[n-1]));
    }
    EGS_2DPolygon pol(p1);
    int ntr = 0;
    triangle = new EGS_TriangleShape* [n-2];
    np = n;
    EGS_Float p_tmp[6];
    while (np > 3) {
        for (int i=0; i<np-2; i++) {
            EGS_2DVector aux(0.5*(xc[i+2]+xc[i]),0.5*(yc[i+2]+yc[i]));
            if (pol.isInside(aux)) {
                bool is_ok = true;
                vector<EGS_2DVector> p2;
                for (int k=0; k<3; k++) {
                    p_tmp[2*k] = xc[i+k];
                    p_tmp[2*k+1] = yc[i+k];
                    p2.push_back(EGS_2DVector(xc[i+k],yc[i+k]));
                }
                EGS_2DPolygon tri(p2);
                for (int j=0; j<np-1; j++) {
                    if (j < i || j > i+2) {
                        EGS_2DVector tmp(xc[j],yc[j]);
                        if (tri.isInside(tmp)) {
                            is_ok = false;
                            break;
                        }
                    }
                }
                if (is_ok) {
                    EGS_TriangleShape *t = new EGS_TriangleShape(p_tmp);
                    triangle[ntr++] = t;
                    for (int j=i+1; j<np-1; j++) {
                        xc[j] = xc[j+1];
                        yc[j] = yc[j+1];
                    }
                }
                np--;
            }
        }
    }
    for (int k=0; k<3; k++) {
        p_tmp[2*k] = xc[k];
        p_tmp[2*k+1] = yc[k];
    }
    triangle[ntr++] = new EGS_TriangleShape(p_tmp);
    A = 0;
    for (int i=0; i<ntr; i++) {
        yc[i] = triangle[i]->area();
        A += yc[i];
    }
    table = new EGS_AliasTable(ntr,xc,yc,0);
    delete [] xc;
    delete [] yc;
}

extern "C" {

    EGS_POLYGON_SHAPE_EXPORT EGS_BaseShape *createShape(EGS_Input *input,
            EGS_ObjectFactory *f) {
        if (!input) {
            egsWarning("createShape(polygon): null input?\n");
            return 0;
        }
        vector<EGS_Float> points;
        int err = input->getInput("points",points);
        if (err) {
            egsWarning("createShape(polygon): no 'points' input\n");
            return 0;
        }
        int np = points.size();
        if ((np%2) != 0) {
            egsWarning("createShape(polygon): you must input an even number of"
                       " floating numbers\n");
            return 0;
        }
        if (np < 6) {
            egsWarning("createShape(polygon): you must input at least 3 2D points"
                       " to form a polygon\n");
            return 0;
        }
        EGS_BaseShape *shape;
        if (np == 6) {
            shape = new EGS_TriangleShape(points,"",f);
        }
        else {
            shape = new EGS_PolygonShape(points,"",f);
        }
        shape->setName(input);
        shape->setTransformation(input);
        return shape;
    }

}
