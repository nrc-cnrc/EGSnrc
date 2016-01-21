/*
###############################################################################
#
#  EGSnrc egs++ transformations headers
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


/*! \file     egs_transformations.h
 *  \brief    Rotations and affine transformations header
 *  \IK
 ***************************************************************************/


#ifndef EGS_TRANSFORMATIONS_

#define EGS_TRANSFORMATIONS_

#include "egs_vector.h"
#include "egs_libconfig.h"
#include "egs_math.h"
#include "egs_functions.h"

#include <iostream>

using namespace std;

class EGS_Input;

/*! \brief A class for vector rotations.

  \ingroup egspp_main

 A rotation matrix object can be constructed by explicitely giving the
 9 coefficients of the 3x3 rotation matrix, from angles of rotation
 around the x-,y- and z-axis, from polar/azimuthal angles of rotations
 and from a given vector. The EGS_RotationMatrix class provides
 functions for matrix multiplication, vector rotation, determination
 of the determinant, etc.
*/
class EGS_EXPORT EGS_RotationMatrix {

protected:

    /*! The 9 rotation matrix coefficients */
    EGS_Float   rxx, rxy, rxz,
                ryx, ryy, ryz,
                rzx, rzy, rzz;

public:

    //! \brief Default constructor, results in a unit matrix object.
    EGS_RotationMatrix() {
        rxx=1;
        rxy=0;
        rxz=0;
        ryx=0;
        ryy=1;
        ryz=0;
        rzx=0;
        rzy=0;
        rzz=1;
    };

    //
    // hopefully no one will use this constructor
    // with a non-rotation matrix elements
    //
    /*! \brief Construct a rotation matrix object from 9 floating point numbers*/
    EGS_RotationMatrix(EGS_Float xx, EGS_Float xy, EGS_Float xz,
                       EGS_Float yx, EGS_Float yy, EGS_Float yz,
                       EGS_Float zx, EGS_Float zy, EGS_Float zz) {
        rxx=xx;
        rxy=xy;
        rxz=xz;
        ryx=yx;
        ryy=yy;
        ryz=yz;
        rzx=zx;
        rzy=zy;
        rzz=zz;
    };

    //! \brief Copy constructor
    EGS_RotationMatrix(const EGS_RotationMatrix &m) {
        rxx=m.rxx;
        rxy=m.rxy;
        rxz=m.rxz;
        ryx=m.ryx;
        ryy=m.ryy;
        ryz=m.ryz;
        rzx=m.rzx;
        rzy=m.rzy;
        rzz=m.rzz;
    };

    /*! \brief Is this object a real rotation matrix?

    Returns true if the object is a real rotation matrix, false otherwise.
    A 3x3 matrix \f$R\f$ is a rotation matrix if its determinant is unity
    and if \f$ R R^T\f$ is a unity matrix, where \f$ R^T\f$ is the transposed
    matrix.
    */
    bool isRotation() const {
        EGS_Float d = det() - 1;
        EGS_RotationMatrix t((*this)*inverse());
        if (fabs(d) > 1e-4 || !t.isI()) {
            return false;
        }
        return true;
    };

    // creates a matrix which, when applied to the vector v,
    // transforms it into a vector along the z-axis.
    // why z-axis?
    // well, it has to be one of the axis and in physics
    // things usually happen along the z-axis!
    /*! \brief Create a rotation matrix from the vector \a v.

    This constructs a matrix which, when applied to the vector \a v,
    transforms it into a vector along the z-axis. Why z-axis?
    Well, it has to be one of the axis and in physics things usually happen
    along the z-axis!
    */
    EGS_RotationMatrix(const EGS_Vector &v) {
        EGS_Float sinz = v.x*v.x + v.y*v.y;
        EGS_Float norm = sinz + v.z*v.z;
        if (norm < 1e-15) egsFatal("EGS_RotationMatrix::EGS_RotationMatrix: \n"
                                       "  no construction from a zero vector possible!\n");
        norm = sqrt(norm);
        if (sinz > 1e-15) {
            sinz = sqrt(sinz);
            EGS_Float cphi = v.x/sinz;
            register EGS_Float sphi = v.y/sinz;
            EGS_Float cost = v.z/norm;
            register EGS_Float sint = -sinz/norm;
            *this = rotY(cost,sint)*rotZ(cphi,sphi);
        }
        else { // v is along the z-axis => matrix is the unit transformation
            rxx = 1;
            rxy = 0;
            rxz = 0;
            ryx = 0;
            ryy = 1;
            ryz = 0;
            rzx = 0;
            rzy = 0;
            rzz = 1;
        }
    };

    // R_x(alpha) * R_y(beta) * R_z(gamma)
    /*! \brief Constructs a rotation matrix from rotation angles
    around the x-, y-, and z-axis as \f$R_x(\alpha) R_y(\beta) R_z(\gamma)\f$
    */
    EGS_RotationMatrix(EGS_Float alpha, EGS_Float beta, EGS_Float gamma) {
        *this = rotX(alpha)*rotY(beta)*rotZ(gamma);
    };

    // R_z(phi) * R_x(theta)
    /*! \brief Constructs a rotation matrix from the angles \a theta and
      \a phi (polar and azimuthal) as \f$R_z(\phi) R_x(\theta)\f$
     */
    EGS_RotationMatrix(EGS_Float phi, EGS_Float theta) {
        *this = rotZ(phi)*rotX(theta);
    };

    //! \brief Assignment operator
    EGS_RotationMatrix &operator=(const EGS_RotationMatrix &m) {
        rxx=m.rxx;
        rxy=m.rxy;
        rxz=m.rxz;
        ryx=m.ryx;
        ryy=m.ryy;
        ryz=m.ryz;
        rzx=m.rzx;
        rzy=m.rzy;
        rzz=m.rzz;
        return *this;
    };

    //! \brief Comparison operator
    bool operator==(const EGS_RotationMatrix &m) {
        return
            ((rxx == m.rxx) && (rxy == m.rxy) && (rxz == m.rxz) &&
             (ryx == m.ryx) && (ryy == m.ryy) && (ryz == m.ryz) &&
             (rzx == m.rxz) && (rzy == m.rzy) && (rzz == m.rzz)) ? true : false;
    };

    /*! \brief Returns \c true, if this object is a unity matrix, \a false
      otherwise */
    bool isI() const {
        return (rxx == 1 && rxy == 0 && rxz == 0 && ryx == 0 && ryy == 1 &&
                ryz == 0 && rzx == 0 && rzy == 0 && rzz == 1);
    };

    /*! \brief Returns the rotated a vector \f$ R \cdot \vec{v}\f$ */
    EGS_Vector operator*(const EGS_Vector &v) const {
        return EGS_Vector(rxx*v.x + rxy*v.y + rxz*v.z,
                          ryx*v.x + ryy*v.y + ryz*v.z,
                          rzx*v.x + rzy*v.y + rzz*v.z);
    };

    /*! \brief Multiplies the invoking object with \a m from the right
      and returns the result. */
    EGS_RotationMatrix operator*(const EGS_RotationMatrix &m) const {
        return EGS_RotationMatrix(
                   rxx*m.rxx+rxy*m.ryx+rxz*m.rzx, rxx*m.rxy+rxy*m.ryy+rxz*m.rzy,
                   rxx*m.rxz+rxy*m.ryz+rxz*m.rzz,
                   ryx*m.rxx+ryy*m.ryx+ryz*m.rzx, ryx*m.rxy+ryy*m.ryy+ryz*m.rzy,
                   ryx*m.rxz+ryy*m.ryz+ryz*m.rzz,
                   rzx*m.rxx+rzy*m.ryx+rzz*m.rzx, rzx*m.rxy+rzy*m.ryy+rzz*m.rzy,
                   rzx*m.rxz+rzy*m.ryz+rzz*m.rzz);
    };

    // r *= m means r = r*m
    /*! \brief Multiplies the invoking object with \a m from the right and
      assigns the resulting matrix to the invoking object returning a reference
      to it. */
    EGS_RotationMatrix &operator*=(const EGS_RotationMatrix &m) {
        return *this = operator * (m);
    };

    // r.multiply(m) means r = m*r;
    /*! \brief Multiplies the invoking object with \a m from the left and returns
      the result.

     Note: \f$ R \cdot M \ne R \cdot M\f$ and therefore one needs to distinguish
     between multiplication from the right as in operator*() and multiplication
     from the left.
    */
    void multiply(const EGS_RotationMatrix &m) {
        *this = m.operator * (*this);
    };

    /*! \brief Returns the inverse matrix.

      Note that the implementation simply assumes that the matrix is a real
      rotation matrix and therefore returns the transposed matrix.
    */
    EGS_RotationMatrix inverse() const {
        return EGS_RotationMatrix(rxx,ryx,rzx,rxy,ryy,rzy,rxz,ryz,rzz);
    };

    //! \brief Returns the transposed matrix.
    EGS_RotationMatrix T() {
        return EGS_RotationMatrix(rxx,ryx,rzx,rxy,ryy,rzy,rxz,ryz,rzz);
    };

    //! \brief Inverts the matrix and returns a reference to it.
    EGS_RotationMatrix &invert() {
        return *this = inverse();
    };

    /*! \brief Returns a rotation around the x-axis by the angle \f$\phi\f$
    with \a cphi, \a sphi = \f$ \cos(\phi), \sin(\phi)\f$.
    */
    static EGS_RotationMatrix rotX(EGS_Float cphi,EGS_Float sphi) {
        return EGS_RotationMatrix(
                   (EGS_Float)1,(EGS_Float)0,(EGS_Float)0,
                   (EGS_Float)0,        cphi,        sphi,
                   (EGS_Float)0,       -sphi,        cphi);
    };

    /*! \brief Returns a rotation around the y-axis by the angle \f$\phi\f$
    with \a cphi, \a sphi = \f$ \cos(\phi), \sin(\phi)\f$.
    */
    static EGS_RotationMatrix rotY(EGS_Float cphi,EGS_Float sphi) {
        return EGS_RotationMatrix(cphi, (EGS_Float)0,         sphi,
                                  (EGS_Float)0, (EGS_Float)1, (EGS_Float)0,
                                  -sphi, (EGS_Float)0,         cphi);
    };

    /*! \brief Returns a rotation around the z-axis by the angle \f$\phi\f$
    with \a cphi, \a sphi = \f$ \cos(\phi), \sin(\phi)\f$.
    */
    static EGS_RotationMatrix rotZ(EGS_Float cphi,EGS_Float sphi) {
        return EGS_RotationMatrix(cphi,        sphi, (EGS_Float)0,
                                  -sphi,        cphi, (EGS_Float)0,
                                  (EGS_Float)0,(EGS_Float)0, (EGS_Float)1);
    };

    //! \brief Returns a rotation around the x-axis by the angle \a phi
    static EGS_RotationMatrix rotX(EGS_Float phi) {
        return rotX(cos(phi),sin(phi));
    };

    //! \brief Returns a rotation around the y-axis by the angle \a phi
    static EGS_RotationMatrix rotY(EGS_Float phi) {
        return rotY(cos(phi),sin(phi));
    };

    //! \brief Returns a rotation around the z-axis by the angle \a phi
    static EGS_RotationMatrix rotZ(EGS_Float phi) {
        return rotZ(cos(phi),sin(phi));
    };

    /*! \brief Returns a rotation by the angle \a phi around the axis
      defined by the vector \a v.
     */
    static EGS_RotationMatrix rotV(EGS_Float phi, const EGS_Vector &v) {
        return rotV(cos(phi),sin(phi),v);
    };

    /*! \overload */
    static EGS_RotationMatrix rotV(EGS_Float cphi, EGS_Float sphi,
                                   const EGS_Vector &v) {
        EGS_RotationMatrix m(v);
        return (m.inverse())*(rotZ(cphi,sphi))*(m);
    };

    //! \brief Calculates and returns the determinant of the matrix
    EGS_Float det() const {
        return rxx*ryy*rzz + rxy*ryz*rzx + ryx*rzy*rxz -
               rxz*ryy*rzx - rxy*ryx*rzz - rzy*ryz*rxx;
    };

    /*! \brief Multiplies the invoking vector \a v from the right with the
      matrix \a m and returns the result.
     */
    friend EGS_Vector operator*(const EGS_Vector &v,
                                const EGS_RotationMatrix &m) {
        return EGS_Vector(v.x*m.rxx+v.y*m.ryx+v.z*m.rzx,
                          v.x*m.rxy+v.y*m.ryy+v.z*m.rzy,
                          v.x*m.rxz+v.y*m.ryz+v.z*m.rzz);
    };

    /*! \brief Multiplies the invoking vector \a v from the right with the
      matrix \a m and assigns the result to the invoking vector.
      Returns a reference to the resulting vector.
     */
    friend EGS_Vector &operator*=(EGS_Vector &v, const EGS_RotationMatrix &m) {
        v = v*m;
        return v;
    };

    inline EGS_Float xx() const {
        return rxx;
    };
    inline EGS_Float xy() const {
        return rxy;
    };
    inline EGS_Float xz() const {
        return rxz;
    };
    inline EGS_Float yx() const {
        return ryx;
    };
    inline EGS_Float yy() const {
        return ryy;
    };
    inline EGS_Float yz() const {
        return ryz;
    };
    inline EGS_Float zx() const {
        return rzx;
    };
    inline EGS_Float zy() const {
        return rzy;
    };
    inline EGS_Float zz() const {
        return rzz;
    };

};

/*! \brief A class providing affine transformations

  \ingroup egspp_main

  An affine transformation \f$T = (R,\vec{t})\f$
  consists of a rotation \f$R\f$ and a translation \f$\vec{t}\f$, so that
  \f$T \vec{x} = R \vec{x} + \vec{t}\f$. See the getTransformation()
  documentation for description of the keys needed to define
  an affine transformation.
 */
class EGS_EXPORT EGS_AffineTransform {

protected:

    EGS_RotationMatrix R;
    EGS_Vector         t;
    bool               has_t, has_R;

public:

    /*! \brief Constructs a unit affine transformation */
    EGS_AffineTransform() : R(),t(),has_t(false),has_R(false) {};

    /*! \brief Copy constructor */
    EGS_AffineTransform(const EGS_AffineTransform &tr) :
        R(tr.R),t(tr.t),has_t(tr.has_t),has_R(tr.has_R) {};

    /*! \brief Constructs an affine transformation object from the rotation
      \a m and translation \a v. */
    EGS_AffineTransform(const EGS_RotationMatrix &m, const EGS_Vector &v) :
        R(m),t(v) {
        if (t.length2() > 0) {
            has_t = true;
        }
        else {
            has_t = false;
        }
        if (R.isI()) {
            has_R = false;
        }
        else {
            has_R = true;
        }
    };

    /*! \brief Constructs an affine transformation object from the rotation
      \a m, which has no translation. */
    EGS_AffineTransform(const EGS_RotationMatrix &m) : R(m),t() {
        has_t = false;
        if (R.isI()) {
            has_R = false;
        }
        else {
            has_R = true;
        }
    };

    /*! \brief Constructs an affine transformation object from the translation
      \a v, which has no rotation. */
    EGS_AffineTransform(const EGS_Vector &v) : R(),t(v) {
        has_R = false;
        if (t.length2() > 0) {
            has_t = true;
        }
        else {
            has_t = false;
        }
    };

    /*! \brief Returns the multiplication of the invoking object with \a tr.

      The multiplication of 2 affine transformations \f$T_1=(R_1,\vec{t_1})\f$
      and \f$T_2=(R_2,\vec{t_2})\f$ is defined as the affine transformation
      \f$T=(R,\vec{t})\f$ which, when applied on any vecor \f$\vec{x}\f$,
      results in the same vector that one would obtain by first transforming
      it with \f$T_1\f$ and then with \f$T_2\f$. It is easy to see that
      \f$ R = R_2 \cdot R_1\f$ and \f$ t = R_2 \cdot \vec{t}_1 + \vec{t}_2\f$.
      */
    EGS_AffineTransform operator*(const EGS_AffineTransform &tr) const {
        return EGS_AffineTransform(R*tr.R,R*tr.t+t);
    };

    /*! \brief Returns the affine transformation \f$ (R \cdot m, \vec{t})\f$,
      where \f$R\f$ and \f$\vec{t}\f$ are the rotation and translation of the
      invoking object.*/
    EGS_AffineTransform operator*(const EGS_RotationMatrix &m) const {
        return EGS_AffineTransform(R*m,t);
    };

    /*! \brief Multiplies the invoking object from the right with \a tr.
      Returns a reference to the result.

      \sa operator *(EGS_AffineTransform &)
     */
    EGS_AffineTransform &operator*=(const EGS_AffineTransform &tr) {
        return *this = operator * (tr);
    };

    /*! \brief Multiplies the invoking object from the right with \a m.
      Returns a reference to the result.

      \sa operator *(EGS_RotationMatrix &)
     */
    EGS_AffineTransform &operator*=(const EGS_RotationMatrix &m) {
        return *this = operator * (m);
    };

    EGS_AffineTransform operator+(const EGS_Vector &v) const {
        return EGS_AffineTransform(R,t+v);
    };

    EGS_AffineTransform &operator+=(const EGS_Vector &v) {
        t += v;
        return *this;
    };

    /*! \brief Applies the transformation to the vector \a v from the
      left and returns the result.
     */
    EGS_Vector operator*(const EGS_Vector &v) const {
        return (R*v + t);
    };

    /*! \brief Applies the transformation \a tr to the invoking vector from
      the right and returns the result. */
    friend EGS_Vector operator*(const EGS_Vector &v,
                                const EGS_AffineTransform &tr) {
        return ((v-tr.t)*tr.R);
    };

    /*! \brief Applies the transformation \a tr to the invoking vector from
      the right, assignes the result to \a v and returns a reference to it */
    friend EGS_Vector &operator*=(EGS_Vector &v,
                                  const EGS_AffineTransform &tr) {
        v = v*tr;
        return v;
    };

    /*! \brief Transforms the vector \a v */
    void transform(EGS_Vector &v) const {
        if (has_R) {
            v = R*v;
        }
        if (has_t) {
            v += t;
        }
    };

    /*! \brief Applies the inverse transformation to the vector \a v */
    void inverseTransform(EGS_Vector &v) const {
        if (has_t) {
            v -= t;
        }
        if (has_R) {
            v *= R;
        }
        //v -= t; v *= R;
    };

    /*! \brief Returns the inverse affine transformation */
    EGS_AffineTransform inverse() const {
        EGS_Vector tmp;
        tmp -= t*R;
        return EGS_AffineTransform(R.inverse(),tmp);
    };

    /*! \brief Applies the rotation to the vector \a v */
    void rotate(EGS_Vector &v) const {
        if (has_R) {
            v = R*v;
        }
    };
    /*! \brief Applies the inverse rotation to the vector \a v */
    void rotateInverse(EGS_Vector &v) const {
        if (has_R) {
            v *= R;
        }
    };
    /*! \brief Applies the translation to the vector \a v */
    void translate(EGS_Vector &v) const {
        v += t;
    };

    /*! \brief Returns the translation vector of the affine transformation
      object*/
    const EGS_Vector &getTranslation() const {
        return t;
    };
    /*! \brief Returns the rotation matrix of the affine transformation
      object*/
    const EGS_RotationMatrix &getRotation() const {
        return R;
    };

    /*! \brief Returns \c true if the object is a unity transformation,
      \c false otherwise. */
    bool isI() const {
        return (!has_R && !has_t);
    };

    /*! \brief Returns \c true if the transformation involves a translation,
      \c false otherwise. */
    bool hasTranslation() const {
        return has_t;
    };

    /*! \brief Returns \c true if the transformation involves a rotation,
      \c false otherwise. */
    bool hasRotation() const {
        return has_R;
    };

    /*! \brief Constructs an affine transformation object from the input
      pointed to by \a inp and returns a pointer to it.

    A transformation is defined in the input file using the following set
    of keys:
    \verbatim
    :start transformation:
        translation = tx, ty, tz
        rotation    = 2, 3 or 9 floating point numbers
        or
        rotation vector = 3 floating point numbers
    :stop transformation:
    \endverbatim
    There are many different ways to define a rotation. The
    <code> rotation vector </code> input defines a rotation which,
    when applied to the 3D vector defined by the input, transforms it
    into a vector along the positive z-axis. For instance, if one wanted
    to have a rotation of +45 degrees around the y-axis, one would use
    -1,0,1 as input to the <code> rotation vector </code> key.
    The input to the \c rotation key is interpreted as follows:
    - If followed by two floating point numbers, this input defines a rotation
      by the polar angle \f$\theta\f$ defined by the second input and the
      azimuthal angle \f$\phi\f$
      defined by the first input with both angles considered to be in radian
      (\em i.e. a rotation by \f$\theta\f$ around the x-axis followed by a rotation
      by \f$\phi\f$ around the z-axis)
    - If followed by three floating point numbers, this input defines a rotation
      that is the combination of a rotation by the third input in radian around
      the z-axis, followed by a rotation by the second input around the y-axis,
      followed by a rotation by the first input around the x-axis.
    - If followed by 9 floating point numbers, the 9 numbers are considered
      as the 9 elements of a 3x3 rotation matrix in the order \f$R_{xx}, R_{xy},
      R_{xz}, R_{yx}, R_{yy}, R_{yz}, R_{zx} R_{zy}, R_{zz}\f$.
    */
    static EGS_AffineTransform *getTransformation(EGS_Input *inp);

};

#endif
