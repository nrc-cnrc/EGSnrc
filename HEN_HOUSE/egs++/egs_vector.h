/*
###############################################################################
#
#  EGSnrc egs++ vector headers
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


/*! \file egs_vector.h
 *  \brief Provides methods for the manipulation of 3D vectors in
 *  cartesian co-ordinates.
 *  \IK
 */

#ifndef EGS_VECTOR_
#define EGS_VECTOR_

#include "egs_libconfig.h"

#include "egs_math.h"

/*! \brief A class representing 3D vectors.

  \ingroup egspp_main

  The EGS_Vector class is used throughout egspp for representing 3D
  vectors in cartesian co-ordinates
  (\em e.g. particle positions and directions). It provides
  various convenience methods for vector additions, subtractions,
  multiplications, etc.
*/
class EGS_EXPORT EGS_Vector {

public:

    EGS_Float x; //!< x-component
    EGS_Float y; //!< y-component
    EGS_Float z; //!< z-component

    EGS_Vector(EGS_Float xx, EGS_Float yy, EGS_Float zz) : x(xx), y(yy), z(zz) {};
    EGS_Vector(const EGS_Vector &v) : x(v.x), y(v.y), z(v.z) {};
    EGS_Vector() : x(0), y(0), z(0) {};

    EGS_Vector &operator=(const EGS_Vector &v) {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    };

    // EGS_Vector additions
    //
    EGS_Vector operator+(const EGS_Vector &v) const {
        return EGS_Vector(x+v.x, y+v.y, z+v.z);
    };
    EGS_Vector &operator+=(const EGS_Vector &v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    };

    // EGS_Vector substractions
    //
    EGS_Vector operator-(const EGS_Vector &v) const {
        return EGS_Vector(x-v.x,y-v.y,z-v.z);
    }
    EGS_Vector &operator-=(const EGS_Vector &v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    };

    // EGS_Vector multiplications
    //
    EGS_Vector operator*(const EGS_Float f) const {
        return EGS_Vector(x*f,y*f,z*f);
    };
    EGS_Vector &operator*=(const EGS_Float f) {
        x *= f;
        y *= f;
        z *= f;
        return *this;
    };
    friend EGS_Vector operator*(EGS_Float f, EGS_Vector &v) {
        return v*f;
    };
    EGS_Float operator*(const EGS_Vector &v) const {
        return x*v.x + y*v.y + z*v.z;
    };

    // vector product
    EGS_Vector times(const EGS_Vector &v) const {
        return EGS_Vector(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x);
    };
    EGS_Vector operator%(const EGS_Vector &v) const {
        return EGS_Vector(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x);
    };

    // scale
    EGS_Vector getScaled(const EGS_Vector &s) const {
        return EGS_Vector(x*s.x,y*s.y,z*s.z);
    };
    void scale(const EGS_Vector &s) {
        x *= s.x;
        y *= s.y;
        z *= s.z;
    };

    // Some other useful methods.
    EGS_Float length() const {
        return sqrt(x*x+y*y+z*z);
    };
    EGS_Float length2() const {
        return x*x+y*y+z*z;
    };
    void normalize() {
        EGS_Float tmp = 1./length();
        x *= tmp;
        y *= tmp;
        z *= tmp;
    };

    void rotate(EGS_Float cos_t, EGS_Float sin_t,
                EGS_Float c_phi, EGS_Float s_phi) {
        EGS_Float sin_z = x*x + y*y;
        if (sin_z > 1e-10) {
            sin_z = sqrt(sin_z);
            EGS_Float temp = sin_t/sin_z;
            EGS_Float temp_phi = z*c_phi;
            EGS_Float temp_x = x*cos_t;
            register EGS_Float temp_y = y*cos_t;
            EGS_Float temp_x1 = temp_phi*x-y*s_phi;
            EGS_Float temp_y1 = temp_phi*y+x*s_phi;
            x = temp*temp_x1+temp_x;
            y = temp*temp_y1+temp_y;
            z = z*cos_t-sin_z*sin_t*c_phi;
        }
        else {
            x = sin_t*c_phi;
            y = sin_t*s_phi;
            z *= cos_t;
        }
    };

};

#endif
