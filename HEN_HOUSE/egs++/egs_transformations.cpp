/*
###############################################################################
#
#  EGSnrc egs++ transformations
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


/*! \file egs_transformations.cpp
 *  \brief EGS_AffineTransform::getTransformation implementation
 *  \IK
 */

#include "egs_transformations.h"
#include "egs_input.h"

#include <vector>
using std::vector;

EGS_AffineTransform *EGS_AffineTransform::getTransformation(EGS_Input *i) {
    if (!i) {
        return 0;
    }
    EGS_Input *input;
    bool delete_it = false;
    if (i->isA("transformation")) {
        input = i;
    }
    else {
        input = i->takeInputItem("transformation");
        if (!input) {
            return 0;
        }
        delete_it = true;
    }
    EGS_Vector t;
    vector<EGS_Float> tmp;
    int err = input->getInput("translation",tmp);
    if (!err && tmp.size() == 3) {
        t = EGS_Vector(tmp[0],tmp[1],tmp[2]);
    }
    EGS_AffineTransform *result;
    err = input->getInput("rotation vector",tmp);
    if (!err && tmp.size() == 3)
        result = new EGS_AffineTransform(
            EGS_RotationMatrix(EGS_Vector(tmp[0],tmp[1],tmp[2])),t);
    else {
        err = input->getInput("rotation",tmp);
        if (!err) {
            if (tmp.size() == 2) result = new EGS_AffineTransform(
                    EGS_RotationMatrix(tmp[0],tmp[1]),t);
            else if (tmp.size() == 3) result = new EGS_AffineTransform(
                    EGS_RotationMatrix(tmp[0],tmp[1],tmp[2]),t);
            else if (tmp.size() == 4) {
                EGS_Vector tmp1(tmp[0],tmp[1],tmp[2]);
                EGS_RotationMatrix R1(tmp1);
                EGS_RotationMatrix R2(EGS_RotationMatrix::rotZ(tmp[3]));
                EGS_RotationMatrix Rtot = R1.inverse()*R2*R1;
                result = new EGS_AffineTransform(Rtot,t);
            }
            else if (tmp.size() == 9) {
                EGS_RotationMatrix R(tmp[0],tmp[1],tmp[2],
                                     tmp[3],tmp[4],tmp[5],
                                     tmp[6],tmp[7],tmp[8]);
                if (!R.isRotation())
                    egsWarning("getTransformation: the rotation specified by\n"
                               "   %g %g %g\n   %g %g %g\n   %g %g %g\n"
                               " is not a rotation\n",tmp[0],tmp[1],tmp[2],
                               tmp[3],tmp[4],tmp[5],tmp[6],tmp[7],tmp[8]);
                result = new EGS_AffineTransform(R,t);
            }
            else {
                result = new EGS_AffineTransform(EGS_RotationMatrix(),t);
            }
        }
        else {
            result = new EGS_AffineTransform(EGS_RotationMatrix(),t);
        }
    }
    if (delete_it) {
        delete input;
    }
    return result;
}
