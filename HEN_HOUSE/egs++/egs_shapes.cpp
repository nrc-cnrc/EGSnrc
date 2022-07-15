/*
###############################################################################
#
#  EGSnrc egs++ shapes
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
#  Contributors:    Reid Townson
#
###############################################################################
*/


/*! \file egs_shapes.cpp
 *  \brief EGS_BaseShape and shape classes implementation
 *  \IK
 *
 *  Also provides a static shapes factory
 */

#include "egs_shapes.h"
#include "egs_input.h"
#include "egs_library.h"

#include <vector>
using namespace std;

static int __shape_count = 0;

static EGS_LOCAL EGS_TypedObjectFactory<EGS_BaseShape>
shape_creator(string("egs++/dso/")+CONFIG_NAME,"EGS_BaseShape");

EGS_BaseShape *EGS_BaseShape::createShape(EGS_Input *i) {
    if (!__shape_count) {
        shape_creator.addKnownObject(new EGS_PointShape);
        shape_creator.addKnownObject(new EGS_BoxShape);
        shape_creator.addKnownObject(new EGS_SphereShape);
        shape_creator.addKnownObject(new EGS_CylinderShape);
    }
    __shape_count++;
    EGS_Object *o = shape_creator.createSingleObject(i,"createShape",true);
    return dynamic_cast<EGS_BaseShape *>(o);
}

EGS_BaseShape *EGS_BaseShape::getShape(const string &Name) {
    EGS_Object *o = shape_creator.getObject(Name);
    return dynamic_cast<EGS_BaseShape *>(o);
}

void EGS_BaseShape::setTransformation(EGS_Input *input) {
    if (T) {
        delete T;
    }
    T = EGS_AffineTransform::getTransformation(input);
}

/****************************************************************************
 *
 *                          concrete shapes
 *
 ****************************************************************************/


/**********************  Point  *******************************/

EGS_Object *EGS_PointShape::createObject(EGS_Input *input) {
    vector<EGS_Float> pos;
    int err = input->getInput("position",pos);
    if (err) {
        egsWarning("EGS_PointShape::createShape: no 'position' input\n");
        return 0;
    }
    if (pos.size() != 3) {
        egsWarning("EGS_PointShape::createShape: found %d inputs "
                   "instead of 3\n",pos.size());
        return 0;
    }
    EGS_PointShape *res = new EGS_PointShape(EGS_Vector(pos[0],pos[1],pos[2]));
    res->setName(input);
    return res;
}

/**********************  Box **********************************/

EGS_Object *EGS_BoxShape::createObject(EGS_Input *input) {
    vector<EGS_Float> s;
    int err = input->getInput("box size",s);
    if (err) {
        egsWarning("EGS_BoxShape::createShape: no 'box size' input?\n");
        return 0;
    }
    EGS_AffineTransform *t = EGS_AffineTransform::getTransformation(input);
    EGS_BoxShape *result;
    if (s.size() == 1) {
        result = new EGS_BoxShape(s[0],t);
    }
    else if (s.size() == 3) {
        result = new EGS_BoxShape(s[0],s[1],s[2],t);
    }
    else {
        egsWarning("EGS_BoxShape::createShape: invalid 'box size' input\n");
        result = 0;
    }
    if (t) {
        delete t;
    }
    result->setName(input);
    return result;
}

/**********************  Sphere **********************************/

EGS_Object *EGS_SphereShape::createObject(EGS_Input *input) {
    EGS_Float r;
    int err = input->getInput("radius",r);
    if (err) {
        egsWarning("EGS_SphereShape::createShape: wrong/missing 'radius'"
                   " input\n");
        return 0;
    }
    EGS_SphereShape *result;
    vector<EGS_Float> xo;
    err = input->getInput("midpoint",xo);
    if (!err && xo.size() == 3) {
        result = new EGS_SphereShape(r,EGS_Vector(xo[0],xo[1],xo[2]));
    }
    else {
        result = new EGS_SphereShape(r);
    }
    result->setName(input);
    return result;
}

/**********************  Cylinder **********************************/

EGS_Object *EGS_CylinderShape::createObject(EGS_Input *input) {
    EGS_Float r, H;
    int err = input->getInput("radius",r);
    if (err) {
        egsWarning("EGS_CylinderShape::getShape: wrong/missing 'radius'"
                   " input\n");
        return 0;
    }
    err = input->getInput("height",H);
    if (err) {
        egsWarning("EGS_CylinderShape::getShape: wrong/missing 'height'"
                   " input\n");
        return 0;
    }
    vector<EGS_Float> phi_range;
    bool set_phi = false;
    if (!input->getInput("phi range",phi_range) && phi_range.size() == 2) {
        set_phi = true;
        phi_range[0] *= M_PI/180;
        phi_range[1] *= M_PI/180;
    }
    EGS_AffineTransform *t = EGS_AffineTransform::getTransformation(input);
    if (t) {
        EGS_CylinderShape *result = new EGS_CylinderShape(r,H,t);
        result->setName(input);
        if (set_phi) {
            result->setPhiRange(phi_range[0],phi_range[1]);
        }
        delete t;
        return result;
    }
    vector<EGS_Float> Xo, A;
    int err1 = input->getInput("midpoint",Xo);
    int err2 = input->getInput("axis",A);
    bool has_Xo = (err1 == 0 && Xo.size() == 3);
    bool has_A = (err2 == 0 && A.size() == 3);
    EGS_CylinderShape *result;
    if (has_Xo && has_A) result = new EGS_CylinderShape(r,H,
                EGS_Vector(Xo[0],Xo[1],Xo[2]),EGS_Vector(A[0],A[1],A[2]));
    else if (has_Xo) result = new EGS_CylinderShape(r,H,
                EGS_Vector(Xo[0],Xo[1],Xo[2]));
    else if (has_A) result = new EGS_CylinderShape(r,H,
                EGS_Vector(0,0,0),EGS_Vector(A[0],A[1],A[2]));
    else {
        result = new EGS_CylinderShape(r,H);
    }
    result->setName(input);
    if (set_phi) {
        result->setPhiRange(phi_range[0],phi_range[1]);
    }
    return result;
}

