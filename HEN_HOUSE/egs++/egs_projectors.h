/*
###############################################################################
#
#  EGSnrc egs++ projectors headers
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


/*! \file egs_projectors.h
 *  \brief Header file for 2D vectors and projectors
 *  \IK
 */

#ifndef EGS_PROJECTORS_
#define EGS_PROJECTORS_

#include "egs_vector.h"
#include "egs_math.h"

#include <string>
using namespace std;

/*! \brief A class representing 2D vectors.

  \ingroup egspp_main

  This class is similar to EGS_Vector but represents 2D points.
  It defines various convenience functions for vector additions,
  multiplications, etc.
*/
class EGS_EXPORT EGS_2DVector {
public:
    EGS_Float x, y;
    EGS_2DVector() : x(0), y(0) {};
    EGS_2DVector(EGS_Float X, EGS_Float Y) : x(X), y(Y) {};
    EGS_2DVector(const EGS_2DVector &v) : x(v.x), y(v.y) {};
    EGS_2DVector &operator=(const EGS_Vector &v) {
        x = v.x;
        y = v.y;
        return *this;
    };
    EGS_2DVector operator+(const EGS_2DVector &v) const {
        return EGS_2DVector(x+v.x, y+v.y);
    };
    EGS_2DVector &operator+=(const EGS_2DVector &v) {
        x += v.x;
        y += v.y;
        return *this;
    };
    EGS_2DVector operator-(const EGS_2DVector &v) const {
        return EGS_2DVector(x-v.x, y-v.y);
    };
    EGS_2DVector &operator-=(const EGS_2DVector &v) {
        x -= v.x;
        y -= v.y;
        return *this;
    };
    EGS_2DVector operator*(const EGS_Float f) const {
        return EGS_2DVector(x*f,y*f);
    };
    EGS_2DVector &operator*=(const EGS_Float f) {
        x*=f;
        y*=f;
        return *this;
    };
    EGS_Float operator*(const EGS_2DVector &v) const {
        return x*v.x + y*v.y;
    };
    EGS_Float operator%(const EGS_2DVector &v) const {
        return x*v.y - y*v.x;
    };
    EGS_Vector crossProduct(const EGS_2DVector &v) const {
        return EGS_Vector(0,0,x*v.y - y*v.x);
    };
    EGS_Float length() const {
        return sqrt(x*x+y*y);
    };
    EGS_Float length2() const {
        return x*x+y*y;
    };
    void normalize() {
        EGS_Float tmp=1./length();
        x*=tmp;
        y*=tmp;
    };
};


/*! \brief A projector into the x-plane.

  \ingroup egspp_main

  For description of the various methods see the EGS_Projector
  documentation.
*/
class EGS_EXPORT EGS_XProjector {
public:
    EGS_XProjector(const string &Type) : type(Type) {};
    EGS_Float operator*(const EGS_Vector &x) const {
        return x.x;
    };
    EGS_Vector operator*(EGS_Float t) const {
        return EGS_Vector(t,0,0);
    };
    EGS_Float length() const {
        return 1;
    };
    EGS_2DVector getProjection(const EGS_Vector &x) const {
        return EGS_2DVector(x.y,x.z);
    };
    EGS_Float distance(const EGS_Vector &x) const {
        return x.x;
    }
    const string &getType() const {
        return type;
    };
    void printInfo() const {};
    EGS_Vector normal() const {
        return EGS_Vector(1,0,0);
    };
    EGS_Vector normal(const EGS_2DVector &x) const {
        return EGS_Vector(0,x.x,x.y);
    };
    EGS_Vector getPoint(const EGS_2DVector &x) const {
        return normal(x);
    };
private:
    string type;
};

/*! \brief A projector into the y-plane.

  \ingroup egspp_main

  For description of the various methods see the EGS_Projector
  documentation.
*/
class EGS_EXPORT EGS_YProjector {
public:
    EGS_YProjector(const string &Type) : type(Type) {};
    EGS_Float operator*(const EGS_Vector &x) const {
        return x.y;
    };
    EGS_Vector operator*(EGS_Float t) const {
        return EGS_Vector(0,t,0);
    };
    EGS_Float length() const {
        return 1;
    };
    EGS_2DVector getProjection(const EGS_Vector &x) const {
        return EGS_2DVector(x.x,x.z);
    };
    EGS_Float distance(const EGS_Vector &x) const {
        return x.y;
    }
    const string &getType() const {
        return type;
    };
    void printInfo() const {};
    EGS_Vector normal() const {
        return EGS_Vector(0,1,0);
    };
    EGS_Vector normal(const EGS_2DVector &x) const {
        return EGS_Vector(x.x,0,x.y);
    };
    EGS_Vector getPoint(const EGS_2DVector &x) const {
        return normal(x);
    };
private:
    string type;
};

/*! \brief A projector into the z-plane.

  \ingroup egspp_main

  For description of the various methods see the EGS_Projector
  documentation.
*/
class EGS_EXPORT EGS_ZProjector {
public:
    EGS_ZProjector(const string &Type) : type(Type) {};
    EGS_Float operator*(const EGS_Vector &x) const {
        return x.z;
    };
    EGS_Vector operator*(EGS_Float t) const {
        return EGS_Vector(0,0,t);
    };
    EGS_Float length() const {
        return 1;
    };
    EGS_2DVector getProjection(const EGS_Vector &x) const {
        return EGS_2DVector(x.x,x.y);
    };
    EGS_Float distance(const EGS_Vector &x) const {
        return x.z;
    }
    const string &getType() const {
        return type;
    };
    void printInfo() const {};
    EGS_Vector normal() const {
        return EGS_Vector(0,0,1);
    };
    EGS_Vector normal(const EGS_2DVector &x) const {
        return EGS_Vector(x.x,x.y,0);
    };
    EGS_Vector getPoint(const EGS_2DVector &x) const {
        return normal(x);
    };
private:
    string type;
};

/*! \brief A projector into any plane.

  \ingroup egspp_main
*/
class EGS_EXPORT EGS_Projector {
public:
    /*! \brief Construct a projector named \a Type. for a plane
     with plane normal \a A */
    EGS_Projector(const EGS_Vector &A, const string &Type);

    /*! \brief Construct a projector named \a Type for the plane
      defined by the 3 points \a x1, \a x2 and \a x3. */
    EGS_Projector(const EGS_Vector &x1, const EGS_Vector &x2,
                  const EGS_Vector &x3, const string &Type);

    /*! \brief Get the scalar product between \a x and the plane normal */
    EGS_Float operator*(const EGS_Vector &x) const {
        return a*x;
    };

    /*! \brief Get the plane normal scaled by \a t */
    EGS_Vector operator*(EGS_Float t) const {
        return a*t;
    };

    /*! \brief Get the length of the plane normal */
    EGS_Float length() const {
        return norm;
    };

    /*! \brief Get the 2D projection of the vector \a x onto the plane */
    EGS_2DVector getProjection(const EGS_Vector &x) const {
        return EGS_2DVector((x-xo)*v1,(x-xo)*v2);
    };

    /*! \brief Get the distance from \a x to the plane (positive, negative or
      zero, depending on which side of the plane the position \a x is). */
    EGS_Float distance(const EGS_Vector &x) const {
        return x*a-d;
    }

    /*! \brief Get the name (type) of this projector */
    const string &getType() const {
        return type;
    };

    /*! \brief Print some info about this projector using egsInformation */
    void printInfo() const;

    /*! \brief Get the normal to the projection plane */
    EGS_Vector normal() const {
        return a;
    };

    /*! \brief ? */
    EGS_Vector normal(const EGS_2DVector &x) const {
        return v1*x.x + v2*x.y;
    };

    /*! \brief Get a 3D position from the 2D position \a x */
    EGS_Vector getPoint(const EGS_2DVector &x) const {
        return xo + v1*x.x + v2*x.y;
    };
private:
    EGS_Vector a;         //!< plane normal
    EGS_Vector xo;        //!< point on the projection plane
    EGS_Vector v1,v2;     /*!< 2 2D vectors on the plane perpendicular to a and
                               to each other */
    EGS_Float  norm;      //! The length of the plane normal
    EGS_Float  d;         //! a*xo
    string type;          //! type (name) of this projector.
};

#endif
