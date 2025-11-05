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
#  Contributors:    Manuel Stoeckl
#                   Reid Townson
#                   Matthew Inglis-Whalen
#
###############################################################################
*/


/*! \file egs_vector.h
 *  \brief EGS_Vector methods for the manipulation of 3D vectors in
 *  cartesian coordinates.
 *  \IK
 */

#ifndef EGS_VECTOR_
#define EGS_VECTOR_

#include <string>

#include "egs_functions.h"
#include "egs_libconfig.h"

#include "egs_math.h"

/*! \brief A class representing 3D vectors.

  \ingroup egspp_main

  The EGS_Vector class is used throughout egs++ for representing 3D
  vectors in cartesian coordinates
  (\em e.g. particle positions and directions). It provides
  various convenience methods for vector additions, subtractions,
  multiplications, etc.
*/
class EGS_EXPORT EGS_Vector {

public:

    /// The x component of the vector
    EGS_Float x;
    /// The y component of the vector
    EGS_Float y;
    /// The z component of the vector
    EGS_Float z;

    /*! \brief
      Component constructor; sets individual components x, y, z.
    */
    EGS_Vector(EGS_Float xx, EGS_Float yy, EGS_Float zz) : x(xx), y(yy), z(zz) {}

    /*! \brief
      Copy constructor; copies x, y, z components of \a v.
    */
    EGS_Vector(const EGS_Vector &v) : x(v.x), y(v.y), z(v.z) {}

    /*! \brief
      Default constructor; all components set to zero.
    */
    EGS_Vector() : x(0), y(0), z(0) {}

    /*! \brief
      Assignment operator; applies componentwise assignment for x, y, z from the components of \a v.
    */
    EGS_Vector &operator=(const EGS_Vector &v) {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }

    /*! \brief
      Indexed access; for when you want to access the components by index {0, 1, 2} rather than by its name
    */
    EGS_Float operator[](size_t idx) const {
        // branchless implementation with no error handling
        return 0.5*(idx-1)*(idx-2)*x - idx*(idx-2)*y + 0.5*idx*(idx-1)*z;
    }

    /*! \brief
      Indexed access; for when you want to access the components by index {0, 1, 2} rather than by its name.
    */
    EGS_Float at(size_t idx) const {
        // This is simply a named version of operator[], for e.g. more convenient usage with pointers
        // I.e. 5 * v->at(0) is easier to read than 5 * (*v)[0]
        return 0.5*(idx-1)*(idx-2)*x - idx*(idx-2)*y + 0.5*idx*(idx-1)*z;
    }

    /*! \brief
      Compares the vector to the zero vector, with the default |v|^2
      tolerance of 1e-10 configurable with setTolerance()
    */
    bool isZero() const {
        return length2() < EGS_Vector::tolerance;
    }
    /*! \brief
      Comparison operator; returns true when all components
      of the two vectors are equal (up to the default tolerance of 1e-10)
    */
    bool operator==(const EGS_Vector &v) const {
        return operator-(v).isZero();
    }
    /*! \brief
      Comparison operator; returns true when all components
      of the two vectors are equal (relative to the default tolerance of 1e-10)
    */
    bool operator!=(const EGS_Vector &v) const {
        return !operator==(v);
    }

    /*! \brief
      Addition of an EGS_Vector with another EGS_Vector \a v
    */
    EGS_Vector operator+(const EGS_Vector &v) const {
        return EGS_Vector(x+v.x, y+v.y, z+v.z);
    }
    /*! \brief
      In-place addition of an EGS_Vector with another EGS_Vector \a v
    */
    EGS_Vector &operator+=(const EGS_Vector &v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    /*! \brief
      Negation operator; flips the sign of each component in the vector
    */
    EGS_Vector operator-() const {
        return EGS_Vector(-x, -y, -z);
    }
    /*! \brief
      Subtraction of an EGS_Vector by another EGS_Vector \a v
    */
    EGS_Vector operator-(const EGS_Vector &v) const {
        return EGS_Vector(x-v.x, y-v.y, z-v.z);
    }
    /*! \brief
      In-place subtraction of an EGS_Vector by another EGS_Vector \a  v
    */
    EGS_Vector &operator-=(const EGS_Vector &v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    /*! \brief
      Multiplication of an EGS_Vector by a float \a f on the right
    */
    EGS_Vector operator*(const EGS_Float f) const {
        return EGS_Vector(x*f, y*f, z*f);
    }
    /*! \brief
      In-place multiplication of an EGS_Vector by a float \a f
    */
    EGS_Vector &operator*=(const EGS_Float f) {
        x *= f;
        y *= f;
        z *= f;
        return *this;
    }
    /*! \brief
      Multiplication of an EGS_Vector by a float \a f on the left
    */
    friend EGS_Vector operator*(EGS_Float f, const EGS_Vector &v) {
        return v*f;
    }

    /*! \brief
      Division of an EGS_Vector by a float \a f
    */
    EGS_Vector operator/(EGS_Float f) const {
        return EGS_Vector(x/f, y/f, z/f);
    }
    /*! \brief
      In-place division of an EGS_Vector by a float \a f
    */
    EGS_Vector &operator/=(EGS_Float f) {
        x /= f;
        y /= f;
        z /= f;
        return *this;
    }

    /*! \brief
      Returns the inner product (dot product) of an EGS_Vector with another EGS_Vector \a v
    */
    EGS_Float dot(const EGS_Vector &v) const {
        return x*v.x + y*v.y + z*v.z;
    }
    /*! \brief
      Returns the inner product (dot product) of an EGS_Vector with another EGS_Vector \a v.
    */
    EGS_Float operator*(const EGS_Vector &v) const {
        // Syntactic sugar for the `dot` function
        return x*v.x + y*v.y + z*v.z;
    }

    /*! \brief
      Returns the vector product (cross product, symbol \times), of this EGS_Vector and another EGS_Vector \a v
    */
    EGS_Vector times(const EGS_Vector &v) const {
        return EGS_Vector(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
    }
    /*! \brief
      Returns the vector product (cross product), of this EGS_Vector and another EGS_Vector \a v.
    */
    EGS_Vector operator%(const EGS_Vector &v) const {
        // Syntactic sugar for the `times` function
        return EGS_Vector(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
    }

    /*! \brief
      Componentwise multiplication of an EGS_Vector by another EGS_Vector \a s.

      Equivalent to multiplying by a 3x3 diagonal matrix whose entries are the components of \a s
    */
    EGS_Vector getScaled(const EGS_Vector &s) const {
        return EGS_Vector(x*s.x, y*s.y, z*s.z);
    }
    /*! \brief
      In-place componentwise multiplication of an EGS_Vector by another EGS_Vector \a s.

      Equivalent to multiplying by a 3x3 diagonal matrix whose entries are the components of \a s
    */
    void scale(const EGS_Vector &s) {
        x *= s.x;
        y *= s.y;
        z *= s.z;
    }

    /*! \brief
      The L2 norm of the EGS_Vector
    */
    EGS_Float length() const {
        return sqrt(x*x+y*y+z*z);
    }
    /*! \brief
      The squared length of the EGS_Vector
    */
    EGS_Float length2() const {
        return x*x+y*y+z*z;
    }

    /*! \brief
      Returns a normalized copy of the EGS_Vector
    */
    EGS_Vector normalized() const {
        // In c++17 this should come with the [[nodiscard]] attribute
        EGS_Float norm = 1./length();
        return EGS_Vector(x*norm, y*norm, z*norm);
    }
    /*! \brief
      In-place normalization of the EGS_Vector
    */
    void normalize() {
        // equivalent to v /= v.length();
        EGS_Float tmp = 1./length();
        x *= tmp;
        y *= tmp;
        z *= tmp;
    }

    /*! \brief
      In-place rotation of an EGS_Vector that is assumed to be already normalized.
      The vector `r'` after rotation is the linear combination

      `r' = cθ r + sθ r⟂`

      where `r` is the original vector to which `r⟂` is orthogonal.
    */
    void rotate(EGS_Float cos_t, EGS_Float sin_t,
                EGS_Float c_phi, EGS_Float s_phi) {

        // this is an old function so it can't be renamed

        EGS_Float sin_z = x*x + y*y;
        if (sin_z > epsilon) {
            sin_z = sqrt(sin_z);
            EGS_Float temp = sin_t/sin_z;
            EGS_Float temp_phi = z*c_phi;
            x = x*cos_t + temp*(temp_phi*x - y*s_phi);
            y = y*cos_t + temp*(temp_phi*y + x*s_phi);
            z = z*cos_t - sin_z*sin_t*c_phi;
        }
        else {
            x = sin_t*c_phi;
            y = sin_t*s_phi;
            z *= cos_t;
        }
    }

    /*! \brief
      In-place rotation of an EGS_Vector with arbitrary (dimensionful) length.
      The vector `r'` after rotation is the linear combination

      `r' = cθ r + sθ r⟂`

      where `r` is the original vector to which `r⟂` is orthogonal, and these both share the same length.
    */
    void rotateDimensionful(
        EGS_Float cos_t,
        EGS_Float sin_t,
        EGS_Float c_phi,
        EGS_Float s_phi) {

        // follows the same algorithm as rotate() but accounting for arbitrary lengths

        EGS_Float rho_sqr = x*x + y*y;
        EGS_Float L_sqr = rho_sqr + z*z;

        if (rho_sqr > epsilon) {

            EGS_Float rho = sqrt(rho_sqr);
            EGS_Float L = sqrt(L_sqr);

            x = cos_t * x + sin_t * (x*z*c_phi - y*L*s_phi) / rho;
            y = cos_t * y + sin_t * (y*z*c_phi + x*L*s_phi) / rho;
            z = cos_t * z + sin_t * (-rho * c_phi);
        }
        else {
            x = z * sin_t * c_phi;
            y = z * sin_t * s_phi;
            z = z * cos_t;
        }
    }

    /*! \brief
      Returns a string representation of the vector
    */
    string toString() const {

        EGS_Float log10x = log10(x), log10y = log10(y), log10z = log10(z);
        int avg_magnitude = floor((log10x+log10y+log10z) / 3.);
        string str = "("
                     + to_string(x*pow(10, -avg_magnitude)) + ", "
                     + to_string(y*pow(10, -avg_magnitude)) + ", "
                     + to_string(z*pow(10, -avg_magnitude)) + ")x10^" + to_string(avg_magnitude);
        return str;
    }

    /*! \brief
      Adds a representation of the vector to the ostream
    */
    friend std::ostream &operator<<(std::ostream &os, const EGS_Vector &v) {
        return os << v.toString();
    }

#if __cplusplus >= 201703L

    /*! \brief
      Sets the tolerance of the `isZero` method.

      Returns a reference to itself so the operation can be chained.
    */
    EGS_Vector &setTolerance(EGS_Float tol) {
        EGS_Vector::tolerance = tol;
        return *this;
    }

private:

    /// The tolerance with which |v|^2 < tol is compared to qualify as a zero vector
    inline static EGS_Float tolerance = 1e-10;

#else
private:
    constexpr static EGS_Float tolerance = 1e-10;
#endif
};

#endif
