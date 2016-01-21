/*
###############################################################################
#
#  EGSnrc egs++ prism geometry
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


/*! \file egs_prism.cpp
 *  \brief A prism geometry: implementation
 *  \IK
 */

#include "egs_prism.h"
#include "egs_input.h"

#include <vector>
using std::vector;

static EGS_PRISM_LOCAL string __prismX("EGS_PrismX");
static EGS_PRISM_LOCAL string __prismY("EGS_PrismY");
static EGS_PRISM_LOCAL string __prismZ("EGS_PrismZ");
static EGS_PRISM_LOCAL string __prism("EGS_Prism");

extern "C" {

    EGS_PRISM_EXPORT EGS_BaseGeometry *createGeometry(EGS_Input *input) {

        if (!input) {
            egsWarning("createGeometry(prism): null input?\n");
            return 0;
        }
        string type;
        int err = input->getInput("type",type);
        if (err) {
            egsWarning("createGeometry(prism): missing 'type' input\n");
            return 0;
        }
        EGS_BaseGeometry *g;
        vector<EGS_Float> p;
        err = input->getInput("points",p);
        if (err) {
            egsWarning("createGeometry(prism): missing 'points' input\n");
            return 0;
        }
        bool open = true;
        vector<EGS_Float> tmp;
        err = input->getInput("closed",tmp);
        if (!err && tmp.size() == 2) {
            open = false;
        }
        bool p_open = false;
        int itmp;
        err = input->getInput("open triangle",itmp);
        if (!err && itmp == 1) {
            p_open = true;
        }
        if (input->compare(type,__prismX) || input->compare(type,__prismY)
                || input->compare(type,__prismZ)) {
            int np = p.size()/2;
            if (np < 3) {
                egsWarning("createGeometry(prism): at least 3 points are required "
                           "to construct a prism\n");
                return 0;
            }
            vector<EGS_2DVector> points;
            for (int j=0; j<np; j++) {
                points.push_back(EGS_2DVector(p[2*j],p[2*j+1]));
            }
            if (open) {
                if (input->compare(type,__prismX))
                    g=new EGS_PrismX(new EGS_PolygonYZ(points,
                                                       EGS_XProjector(__prismX),p_open));
                else if (input->compare(type,__prismY))
                    g=new EGS_PrismY(new EGS_PolygonXZ(points,
                                                       EGS_YProjector(__prismY),p_open));
                else
                    g=new EGS_PrismZ(new EGS_PolygonXY(points,
                                                       EGS_ZProjector(__prismZ),p_open));
            }
            else {
                if (input->compare(type,__prismX))
                    g=new EGS_PrismX(new EGS_PolygonYZ(points,
                                                       EGS_XProjector(__prismX),p_open),tmp[0],tmp[1]);
                else if (input->compare(type,__prismY))
                    g=new EGS_PrismY(new EGS_PolygonXZ(points,
                                                       EGS_YProjector(__prismY),p_open),tmp[0],tmp[1]);
                else
                    g=new EGS_PrismZ(new EGS_PolygonXY(points,
                                                       EGS_ZProjector(__prismZ),p_open),tmp[0],tmp[1]);
            }
        }
        else {
            int np = p.size()/3;
            if (np < 3) {
                egsWarning("createGeometry(prism): at least 3 points are required"
                           " to construct a prism\n");
                return 0;
            }
            vector<EGS_Vector> points;
            for (int j=0; j<np; j++) {
                points.push_back(EGS_Vector(p[3*j],p[3*j+1],p[3*j+2]));
            }
            EGS_Vector aux(points[np-1]-points[0]);
            if (aux.length2() > 1e-6) {
                points.push_back(points[0]);
            }
            np = points.size();
            EGS_Projector pro(points[0],points[1],points[np-2],__prism);
            vector<EGS_2DVector> p2;
            {
                for (int j=0; j<np; j++) {
                    p2.push_back(pro.getProjection(points[j]));
                    EGS_Float d = pro.distance(points[j]);
                    if (fabs(d) > 1e-6) {
                        egsWarning("createGeometry(prism): "
                                   "points are not on a plane\n");
                        return 0;
                    }
                }
            }
            if (open) {
                g = new EGS_Prism(new EGS_Polygon(p2,pro,p_open));
            }
            else {
                g = new EGS_Prism(new EGS_Polygon(p2,pro,p_open),tmp[0],tmp[1]);
            }
        }
        g->setName(input);
        g->setMedia(input);
        g->setLabels(input);
        return g;
    }

}
