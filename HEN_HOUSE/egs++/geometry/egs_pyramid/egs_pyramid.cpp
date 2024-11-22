/*
###############################################################################
#
#  EGSnrc egs++ pyramid geometry
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
#                   Marc Chamberland
#                   Reid Townson
#
###############################################################################
*/


/*! \file egs_pyramid.cpp
 *  \brief A pyramid geometry: implementation
 *  \IK
 */

#include "egs_pyramid.h"
#include "egs_input.h"
#include "egs_functions.h"

#include <vector>
using std::vector;

static EGS_PYRAMID_LOCAL string __pyrX = "EGS_PyramidX";
static EGS_PYRAMID_LOCAL string __pyrY = "EGS_PyramidY";
static EGS_PYRAMID_LOCAL string __pyrZ = "EGS_PyramidZ";
static EGS_PYRAMID_LOCAL string __pyr  = "EGS_Pyramid";

template<class T>
EGS_PyramidT<T>::EGS_PyramidT(T *P, const EGS_Vector &Xo, bool O,
                              const string &Name) : EGS_BaseGeometry(Name), p(P), xo(Xo),
    a(p->getNormal()), open(O) {
    nreg = 1;
    d = p->distance(xo);
    xop = p->getProjection(xo);
    if (fabs(d) < boundaryTolerance) egsFatal("%s: the tip is too close to"
                " the base (%g)\n",getType().c_str(),fabs(d));
    if (d < 0) {
        a *= (-1);
        d *= (-1);
    }
    EGS_Vector v2 = p->getPoint(0), v1;
    n = p->getN();
    s = new EGS_Polygon * [n];
    vector<EGS_2DVector> aux;
    for (int j=0; j<p->getN(); j++) {
        v1 = v2;
        v2 = p->getPoint(j+1);
        EGS_Vector aj = p->getNormal(j);
        EGS_Projector *pro = new EGS_Projector(v1,xo,v2,"");
        EGS_Vector n = pro->normal();
        if (n*aj < 0) {
            delete pro;
            pro = new EGS_Projector(v2,xo,v1,"");
            n = pro->normal();
            if (n*aj < 0)
                egsFatal("%s: n*aj < 0 for both normal orientations?\n",
                         getType().c_str());
        }
        aux.push_back(pro->getProjection(v1));
        aux.push_back(pro->getProjection(xo));
        aux.push_back(pro->getProjection(v2));
        s[j] = new EGS_Polygon(aux,*pro,open);
        aux.clear();
        delete pro;
    }
    is_convex = p->isConvex();
}

template<class T>
void EGS_PyramidT<T>::printInfo() const {
    EGS_BaseGeometry::printInfo();
}

extern "C" {

    EGS_PYRAMID_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {

        if (!input) {
            egsWarning("createGeometry(pyramid): null input?\n");
            return 0;
        }
        string type;
        int err = input->getInput("type",type);
        if (err) {
            egsWarning("createGeometry(pyramid): missing 'type' input\n");
            return 0;
        }
        EGS_BaseGeometry *g;
        vector<EGS_Float> p;
        err = input->getInput("points",p);
        if (err) {
            egsWarning("createGeometry(pyramid): missing 'points' input\n");
            return 0;
        }
        vector<EGS_Float> tip;
        err = input->getInput("tip",tip);
        if (err || tip.size() != 3) {
            egsWarning("createGeometry(pyramid): wrong/missing 'tip' input\n");
            return 0;
        }
        bool o = true;
        int is_closed;
        err = input->getInput("closed",is_closed);
        if (!err && is_closed == 1) {
            o = false;
        }
        //egsWarning("%s: open = %d\n",type.c_str(),o);
        if (input->compare(type,__pyrX) || input->compare(type,__pyrY) ||
                input->compare(type,__pyrZ)) {
            int np = p.size()/2;
            if (np < 3) {
                egsWarning("createGeometry(pyramid): at least 3 points are "
                           "required to construct a pyramid\n");
                return 0;
            }
            vector<EGS_2DVector> points;
            for (int j=0; j<np; j++) {
                points.push_back(EGS_2DVector(p[2*j],p[2*j+1]));
            }
            if (input->compare(type,__pyrX))
                g=new EGS_PyramidX(new
                                   EGS_PolygonYZ(points,EGS_XProjector(__pyrX)),
                                   EGS_Vector(tip[0],tip[1],tip[2]),o);
            else if (input->compare(type,__pyrY))
                g=new EGS_PyramidY(new EGS_PolygonXZ(points,
                                                     EGS_YProjector(__pyrY)),
                                   EGS_Vector(tip[0],tip[1],tip[2]),o);
            else
                g=new EGS_PyramidZ(new
                                   EGS_PolygonXY(points,EGS_ZProjector(__pyrZ)),
                                   EGS_Vector(tip[0],tip[1],tip[2]),o);
        }
        else {
            int np = p.size()/3;
            if (np < 3) {
                egsWarning("createGeometry(pyramid): at least 3 points are required"
                           " to construct a pyramid\n");
                return 0;
            }
            vector<EGS_Vector> points;
            for (int j=0; j<np; j++) {
                points.push_back(EGS_Vector(p[3*j],p[3*j+1],p[3*j+2]));
            }
            EGS_Vector aux(points[np-1]-points[0]);
            if (aux.length2() > epsilon) {
                points.push_back(points[0]);
            }
            np = points.size();
            EGS_Projector pro(points[0],points[1],points[np-2],__pyr);
            vector<EGS_2DVector> p2;
            {
                for (int j=0; j<np; j++) {
                    p2.push_back(pro.getProjection(points[j]));
                    EGS_Float d = pro.distance(points[j]);
                    if (fabs(d) > epsilon) {
                        egsWarning("createGeometry(pyramid): "
                                   "points are not on a plane\n");
                        return 0;
                    }
                }
            }
            g = new EGS_Pyramid(new EGS_Polygon(p2,pro),
                                EGS_Vector(tip[0],tip[1],tip[2]),o);
        }
        g->setName(input);
        g->setBoundaryTolerance(input);
        g->setMedia(input);
        g->setLabels(input);
        return g;
    }

}
