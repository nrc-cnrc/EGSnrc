/*
###############################################################################
#
#  EGSnrc egs++ shapes headers
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


/*! \file egs_shapes.h
 *  \brief Shapes header file
 *  \IK
 */

#ifndef EGS_SHAPES_
#define EGS_SHAPES_

#include "egs_vector.h"
#include "egs_transformations.h"
#include "egs_rndm.h"
#include "egs_object_factory.h"

#include <string>
using std::string;

class EGS_Input;

/*! \defgroup Shapes Shapes
  \brief Shapes are objects that can pick random points within
  a certain area or volume.

  Many of the sources used in the standard set of EGSnrc user codes
  and provided as classes derived from EGS_BaseSimpleSource
  determine the direction and position of a particle from the
  following generic algorithm:
    - Set the position \f$\vec{x}\f$ from either a fixed position or a
      random position picked within a certain region in space.
    - Pick a random position \f$\vec{x}'\f$ in some other region in space and
      determine the direction as
      \f$\vec{u}=(\vec{x}' - \vec{x})/|\vec{x}' - \vec{x}|\f$.

  This algorithm does not depend on the details of selecting
  the random positions and is therefore ideally implemented using
  abstract objects that are able to deliver a random position.
  Such objects are called 'shapes' in the egspp framework.
  All concrete shapes available in the EGSnrc C++ class library
  are derived from the abstract EGS_BaseShape class, which
  specifies the interface to shape objects. The most important method
  defined is \link EGS_BaseShape::getPoint() getPoint() \endlink,
  which takes a \link EGS_RandomGenerator random number generator \endlink
  as input and delivers a
  random 3D vector. All shapes can have an
  \link EGS_AffineTransform affine transformation \endlink \f$T\f$,
  which is applied to the random
  position before returning it to the caller.

  A large number of shapes is provided with the egspp package.
  Most shapes are implemented as small DSOs, which are loaded dynamically
  as needed. Such shapes require the \c library key to be
  present in their definition in the input file.
  A few of the most frequently used shapes are compiled
  into egspp and therefore require a \c type key instead of
  a \c library key.

*/

/*! \defgroup SurfaceS Surface Shapes
    \ingroup Shapes
    \brief Surface shapes are shapes that support the \link
    EGS_BaseShape::getPointSourceDirection() getPointSourceDirection()
    \endlink method.

    The base class for surface shapes EGS_SurfaceShape provides a generic
    implementation of the \link
    EGS_BaseShape::getPointSourceDirection() getPointSourceDirection()
    \endlink method
    as long as shapes derived from it define their area.
 */

/*! \brief Base shape class. All shapes in the EGSnrc C++ class
  library are derived from EGS_BaseShape.

  \ingroup Shapes
  \ingroup egspp_main

  This class specifies the interface to shape objects. For more details on
  shapes, see the \ref Shapes "Shapes module documentation"

 */
class EGS_EXPORT EGS_BaseShape : public EGS_Object {

public:

    /*! \brief Construct a shape named \a Name */
    EGS_BaseShape(const string &Name="",EGS_ObjectFactory *f=0) :
        EGS_Object(Name,f), T(0) {
        otype = "base_shape";
    };
    /*! \brief Destructor. Deletes #T if it is not \c null. */
    virtual ~EGS_BaseShape() {
        if (T) {
            delete T;
        }
    };

    /*! \brief Returns a random 3D vector.
     *
     * Uses the virtual function getPoint() to pick a random position and
     * then applies the affine transformation attached to the shape before
     * returning it.
     */
    virtual EGS_Vector getRandomPoint(EGS_RandomGenerator *rndm) {
        if (T) {
            return (*T)*getPoint(rndm);
        }
        else {
            return getPoint(rndm);
        }
    };

    /*! \brief Sample and return a random 3D vector.
     *
     * This virtual function must be reimplemented by derived classes
     * to sample and return random positions from a certain probability
     * distribution using the random number generator \a rndm.
     */
    virtual EGS_Vector getPoint(EGS_RandomGenerator *rndm) {
        egsFatal("You need to implement the getPoint function in your "
                 "derived class\n");
        return EGS_Vector();
    };

    /*! \brief Set the transformation attached to this shape.
     *
     * If the input pointed to by \a inp contains a valid transformation
     * definition, this function sets the affine transformation attached to the
     * shape according to this definition.
     *
     * \sa EGS_AffineTransform
     */
    void setTransformation(EGS_Input *inp);

    /*! \brief Set the transformation attached to this shape.
     *
     * The shape makes a copy of the transformation pointed to by \a t.
     */
    void setTransformation(EGS_AffineTransform *t) {
        if (T) {
            delete T;
        }
        T = new EGS_AffineTransform(*t);
    };

    /*! \brief Get a pointer to the affine transformation attached to this
     * shape. */
    const EGS_AffineTransform *getTransform() const {
        return T;
    };

    /*! \brief Create a shape from the information pointed to by \a inp.
     *
     * This static function creates a single shape from the input
     * pointed to by \a inp. It returns a pointer to the newly created
     * shape or \c null, if the information pointed to by \a inp was not
     * sufficient to create a shape.
     */
    static EGS_BaseShape *createShape(EGS_Input *inp);

    /*! \brief Get a pointer to the shape named \a Name.
     *
     * A static list of shapes created so far is maintained internally.
     * If a shape with name \a Name exists in this list, a pointer to
     * this shape is returned. Otherwise the return value is \c null.
     */
    static EGS_BaseShape *getShape(const string &Name);

    /*! Does this shape implement the getPointSourceDirection() method?
     *
     * This virtual function should be re-implemented in derived classes
     * if the shape supports the getPointSourceDirection() method.
     */
    virtual bool supportsDirectionMethod() const {
        return false;
    };

    /*! Get a random direction given a source position \a xo.
     *
     * This method is used to sample random directions by picking a
     * point \f$\vec{x}\f$ randomly on a certain surface and then
     * setting the direction to \f$(\vec{x}-\vec{x}_0)/|\vec{x}-\vec{x}_0|\f$.
     * This is useful for simulating a point source collimated to a certain
     * solid angle as defined by the surface from which the points
     * \f$\vec{x}\f$ are picked. The method should be re-implemented
     * by shapes being able to pick surface points and to properly set
     * the statistical weight \a wt so that a proper collimated point source
     * probability distribution results.
     */
    virtual void getPointSourceDirection(const EGS_Vector &xo,
                                         EGS_RandomGenerator *rndm, EGS_Vector &u, EGS_Float &wt) {
        egsFatal("getPointSourceDirection: you have to implement this "
                 "method for the %s shape if you want to use it\n",otype.c_str());
    };

    /*! Get the area of this shape.
     *
     * This method should be re-implemented by surface shapes to return
     * their area. It is used by some of the particle sources to define
     * fluence as the number of particles per unit area.
     */
    virtual EGS_Float area() const {
        return 1;
    };

protected:

    EGS_AffineTransform *T; //!< The affine transformation attached to the shape

};

/*! \brief A surface shape.
 *
 * \ingroup Shapes
 * \ingroup SurfaceS
 * \ingroup egspp_main
 *
 * For convenience, shapes that support the
 * EGS_BaseShape::getPointSourceDirection() method should be derived from
 * this class.
 */
class  EGS_EXPORT EGS_SurfaceShape : public EGS_BaseShape {

public:

    /*! \brief Construct a surface shape named \a Name. */
    EGS_SurfaceShape(const string &Name="",EGS_ObjectFactory *f=0) :
        EGS_BaseShape(Name,f), A(1) {};
    /*! \brief Destructor. Does nothing. */
    ~EGS_SurfaceShape() {};
    /*! \brief Always returns true. Shapes derived from this class \em must
     * implement the getPoint() method to return points on a given surface.
     */
    bool supportsDirectionMethod() const {
        return true;
    };
    /*! \brief Returns the area of this surface shape */
    EGS_Float area() const {
        return A;
    };
    /*! \brief Get a random direction given a source position \a Xo.
     *
     * \sa EGS_BaseShape::getPointSourceDirection()
     */
    void getPointSourceDirection(const EGS_Vector &Xo,
                                 EGS_RandomGenerator *rndm, EGS_Vector &u, EGS_Float &wt) {
        EGS_Vector xo = T ? Xo*(*T) : Xo;
        EGS_Vector x = getPoint(rndm);
        u = x - xo;
        EGS_Float d2i = 1/u.length2(), di = sqrt(d2i);
        u *= di;
        wt = A*fabs(u.z)*d2i;
        if (T) {
            T->rotate(u);
        }
    };

protected:

    /*! The surface area of this shape.
     *
     * Derived shapes must set \a A to their area (or reimplement the
     * area() function).
     */
    EGS_Float A;

};

/*! \brief A point shape. This is the simplest shape possible: it simply always
  returns the same point.

  \ingroup Shapes
  \ingroup egspp_main


A Point shape is specified in the input file via
\verbatim
:start shape:
    type = point
    position = px, py, pz
:stop shape:
\endverbatim
 */
class EGS_EXPORT EGS_PointShape : public EGS_BaseShape {

public:

    /*! \brief Construct a point shape located at \a Xo.*/
    EGS_PointShape(const EGS_Vector &Xo = EGS_Vector(),
                   const string &Name="",EGS_ObjectFactory *f=0) :
        EGS_BaseShape(Name,f), xo(Xo) {
        otype = "point";
    };
    ~EGS_PointShape() { };
    /*! \brief Returns a fixed point */
    EGS_Vector getPoint(EGS_RandomGenerator *) {
        return xo;
    };
    /*! \brief Creates a point shape from the input \a inp and returns
     * a pointer to it.
     */
    EGS_Object *createObject(EGS_Input *inp);

protected:

    EGS_Vector  xo; //!< The point position.

};

/*! \brief A box shape.
 *
 * \ingroup Shapes
 * \ingroup SurfaceS
 *
 *
 * Samples random points within a box of a given size or
 * on the box surface. Specified in the input file via
\verbatim
:start shape:
    type = box
    box size = 1 or 3 inputs defining the box size
:stop shape:
\endverbatim

 */
class EGS_EXPORT EGS_BoxShape : public EGS_BaseShape {

protected:

    EGS_Float  ax, ay, az; //!< The box size

public:

    /*! \brief Create a box shape with unit size. */
    EGS_BoxShape(const string &Name="",EGS_ObjectFactory *f=0) :
        EGS_BaseShape(Name,f), ax(1), ay(1), az(1) {
        otype="box";
    };
    /*! \brief Create a cube with size \a A. */
    EGS_BoxShape(EGS_Float A, const EGS_AffineTransform *t = 0,
                 const string &Name="",EGS_ObjectFactory *f=0) :
        EGS_BaseShape(Name,f), ax(A), ay(A), az(A) {
        if (t) {
            T = new EGS_AffineTransform(*t);
        }
        otype="box";
    };
    /*! \brief Create a box shape with size Ax,Ay,Az. */
    EGS_BoxShape(EGS_Float Ax, EGS_Float Ay, EGS_Float Az,
                 const EGS_AffineTransform *t = 0,
                 const string &Name="",EGS_ObjectFactory *f=0) :
        EGS_BaseShape(Name,f), ax(Ax), ay(Ay), az(Az) {
        if (t) {
            T = new EGS_AffineTransform(*t);
        }
        otype="box";
    };
    /*! \brief Destructor. Does nothing */
    ~EGS_BoxShape() { };

    /*! \brief Returns a point uniformely distributed within the box. */
    EGS_Vector getPoint(EGS_RandomGenerator *rndm) {
        EGS_Vector v(ax*(rndm->getUniform()-0.5),
                     ay*(rndm->getUniform()-0.5),
                     az*(rndm->getUniform()-0.5));
        return v;
    };

    /*! \brief Create a box shape from the information pointed to by \a inp and
     * return a pointer to it.
     */
    EGS_Object *createObject(EGS_Input *);

    /*! \brief Returns \c true. (It is easy to implement the
     * getPointSourceDirection() method for a box.)
     */
    bool supportsDirectionMethod() const {
        return true;
    };

    /*! \brief Sets the direction \a u by picking a random point uniformely the
     * on the box surface.
     * \sa EGS_BaseShape::getPointSourceDirection()
     */
    void getPointSourceDirection(const EGS_Vector &Xo,
                                 EGS_RandomGenerator *rndm, EGS_Vector &u, EGS_Float &wt) {
        EGS_Vector xo = T ? Xo*(*T) : Xo;
        EGS_Float eta = rndm->getUniform()*area();
        if (eta < 2*ax*ay) {
            u.x = ax*(rndm->getUniform()-0.5);
            u.y = ay*(rndm->getUniform()-0.5);
            if (eta < ax*ay) {
                u.z = az/2;
                wt = u.z - xo.z;
            }
            else {
                u.z = -az/2;
                wt = xo.z - u.z;
            }
        }
        else if (eta < 2*(ax*ay + ax*az)) {
            u.x = ax*(rndm->getUniform()-0.5);
            u.z = az*(rndm->getUniform()-0.5);
            if (eta < 2*ax*ay + ax*az) {
                u.y = ay/2;
                wt = u.y - xo.y;
            }
            else {
                u.y = -ay/2;
                wt = xo.y - u.y;
            }
        }
        else {
            eta -= 2*(ax*ay + ax*az);
            u.y = ay*(rndm->getUniform()-0.5);
            u.z = az*(rndm->getUniform()-0.5);
            if (eta < ay*az) {
                u.x = ax/2;
                wt = u.x - xo.x;
            }
            else {
                u.x = -ax/2;
                wt = xo.x - u.x;
            }
        }
        u -= xo;
        EGS_Float d2 = u.length2(), d = sqrt(d2);
        u *= (1/d);
        wt *= (area()/(d2*d));
        if (T) {
            T->rotate(u);
        }
    };
    /*! \brief Returns the box surface area.*/
    EGS_Float area() const {
        return 2*(ax*ay + ax*az + ay*az);
    };

};

/*! \brief A sphere shape.
 *
 * \ingroup Shapes
 * \ingroup SurfaceS
 * \ingroup egspp_main
 *
 *
Samples random points within a sphere or on the spherical surfcace.
Specified in the input file via
\verbatim
:start shape:
    type = sphere
    midpoint = Ox, Oy, Oz
    radius = the sphere radius
:stop shape:
\endverbatim
 */
class EGS_EXPORT EGS_SphereShape : public EGS_BaseShape {

protected:

    EGS_Float  R;     //!< The sphere radius
    EGS_Vector xo;    //!< The sphere midpoint

public:

    /*! \brief Construct a sphere of unit radius about the origin. */
    EGS_SphereShape(const string &Name="",EGS_ObjectFactory *f=0) :
        EGS_BaseShape(Name,f), R(1), xo() {
        otype="sphere";
    };
    /*! \brief Construct a sphere of radius \a r with midpoint \a Xo */
    EGS_SphereShape(EGS_Float r, const EGS_Vector &Xo = EGS_Vector(0,0,0),
                    const string &Name="",EGS_ObjectFactory *f=0) :
        EGS_BaseShape(Name,f), R(r), xo(Xo) {
        otype = "sphere";
    };
    /*! Destructor. Does nothing. */
    ~EGS_SphereShape() {};

    /*! \brief Returns a random point within the sphere. */
    EGS_Vector getPoint(EGS_RandomGenerator *rndm) {
        EGS_Float r = rndm->getUniform(), r1 = rndm->getUniform(),
                  r2 = rndm->getUniform();
        if (r1 > r) {
            r = r1;
        }
        if (r2 > r) {
            r = r2;
        }
        EGS_Float cost = 2*rndm->getUniform()-1;
        EGS_Float sint = sqrt(1-cost*cost);
        r1 = R*r*sint;
        EGS_Float cphi, sphi;
        rndm->getAzimuth(cphi,sphi);
        return xo + EGS_Vector(r1*cphi,r1*sphi,R*r*cost);
    };

    /*! Create a sphere shape from the information pointed to by \a inp, or null
     * if the information is insufficient.
     */
    EGS_Object *createObject(EGS_Input *inp);

    /*! \brief Returns \c true. (It is easy to implement the
     * getPointSourceDirection() method for a sphere.)
     */
    bool supportsDirectionMethod() const {
        return true;
    };

    /*! \brief Sets the direction \a u by picking a random point uniformely
     * on the sphere surface.
     * \sa EGS_BaseShape::getPointSourceDirection()
     */
    void getPointSourceDirection(const EGS_Vector &Xo,
                                 EGS_RandomGenerator *rndm, EGS_Vector &u, EGS_Float &wt) {
        EGS_Vector xo = T ? Xo*(*T) : Xo;
        EGS_Float cost = 2*rndm->getUniform()-1;
        EGS_Float sint = 1-cost*cost;
        EGS_Vector x;
        if (sint > 1e-10) {
            EGS_Float cphi, sphi;
            rndm->getAzimuth(cphi,sphi);
            sint = R*sqrt(sint);
            x.x = sint*cphi;
            x.y = sint*sphi;
            x.z = R*cost;
        }
        else {
            x.z = R*cost;
        }
        u = (x + this->xo) - xo;
        EGS_Float di = 1/u.length();
        u *= di;
        wt = u*x*4*M_PI*R*di*di;
    };
    /*! \brief Returns the sphere surface area.*/
    EGS_Float area() const {
        return 4*M_PI*R*R;
    };
};

/*! \brief A cylinder shape.

  \ingroup Shapes
  \ingroup SurfaceS
  \ingroup egspp_main


Samples random points within a cylinder or on the cylinder surfcace.
Specified in the input file via
\verbatim
:start shape:
    type = cylinder
    radius = the cylinder radius
    height = the cylinder height
    midpoint = Ox, Oy, Oz (optional)
    axis = ax, ay, az (optional)
:stop shape:
\endverbatim
If the optional \c midpoint and \c axis inputs are missing, the
cylinder is centered about the origin and has its axis along the
z-axis.

 */
class EGS_EXPORT EGS_CylinderShape : public EGS_BaseShape {

protected:

    EGS_Float  R;       //!< Cylinder radius
    EGS_Float  h;       //!< Cylinder height
    EGS_Vector xo;      //!< midpoint
    EGS_Vector a;       //!< Cylinder axis
    EGS_Float  phi_min;
    EGS_Float  phi_max;
    bool       has_phi; //!< True, if azimuthal range restricted

    /*! \brief Get a point uniformly distributed within a circle */
    inline void getPointInCircle(EGS_RandomGenerator *rndm, EGS_Float &x,
                                 EGS_Float &y) {
        if (!has_phi) {
            do {
                x = 2*rndm->getUniform()-1;
                y = 2*rndm->getUniform()-1;
            }
            while (x*x + y*y > 1);
            x *= R;
            y *= R;
        }
        else {
            EGS_Float r = R*sqrt(rndm->getUniform());
            EGS_Float eta = rndm->getUniform();
            EGS_Float phi = phi_min*(1-eta) + phi_max*eta;
            x = r*cos(phi);
            y = r*sin(phi);
        }
    };


public:

    /*! Construct a cylinder shape with unit radius and height centered about
     * the origin with an axis along the z-axis.
     */
    EGS_CylinderShape(const string &Name="",EGS_ObjectFactory *f=0) :
        EGS_BaseShape(), R(1), h(1), xo(), a(0,0,1),
        phi_min(0), phi_max(2*M_PI), has_phi(false) {
        otype="cylinder";
    };
    EGS_CylinderShape(EGS_Float r, EGS_Float H,
                      const EGS_Vector &Xo = EGS_Vector(0,0,0),
                      const EGS_Vector &A = EGS_Vector(0,0,1),
                      const string &Name="",EGS_ObjectFactory *f=0) :
        EGS_BaseShape(Name,f), R(r), h(H), xo(Xo), a(A),
        phi_min(0), phi_max(2*M_PI), has_phi(false) {
        EGS_RotationMatrix rmat(a);
        if (xo.length2() > 1e-10 || !rmat.isI()) {
            T = new EGS_AffineTransform(rmat.inverse(),xo);
        }
        otype="cylinder";
    };
    /*! Construct a cylinder shape with radius \a r and height \a H centered
     * about the origin with axis along the z-axis.
     */
    EGS_CylinderShape(EGS_Float r, EGS_Float H, const EGS_AffineTransform *t,
                      const string &Name="",EGS_ObjectFactory *f=0) :
        EGS_BaseShape(Name,f), R(r), h(H),
        phi_min(0), phi_max(2*M_PI), has_phi(false) {
        if (t) {
            T = new EGS_AffineTransform(*t);
        }
        otype="cylinder";
    };
    /*! Destructor. Does nothing. */
    ~EGS_CylinderShape() { };

    /*! Set a restriction on the azimuthal angular range */
    void setPhiRange(EGS_Float Phi_min, EGS_Float Phi_max) {
        if (Phi_min < Phi_max) {
            phi_min = Phi_min;
            phi_max = Phi_max;
        }
        else                    {
            phi_min = Phi_max;
            phi_max = Phi_min;
        }
        if (phi_max - phi_min < 1.99999*M_PI) {
            has_phi = true;
        }
        else {
            has_phi = false;
        }
    };

    /*! \brief Samples and returns a point uniformly distributed within the
     * cylinder.
     */
    EGS_Vector getPoint(EGS_RandomGenerator *rndm) {
        EGS_Float x,y;
        getPointInCircle(rndm,x,y);
        EGS_Float z = h*(rndm->getUniform()-0.5);
        return EGS_Vector(x,y,z);
    };

    /*! Creates and returns a pointer to a cylinder shape from the information
     * pointed to by \a inp, or \c null if the information is insufficient.
     */
    EGS_Object *createObject(EGS_Input *);

    /*! Get the cylinder radius */
    EGS_Float getRadius() const {
        return R;
    };
    /*! Get the cylinder height */
    EGS_Float getHeight() const {
        return h;
    };

    /*! \brief Returns \c true. (It is easy to implement the
     * getPointSourceDirection() method for a cylinder.)
     */
    bool supportsDirectionMethod() const {
        return true;
    };

    /*! \brief Sets the direction \a u by picking a random point uniformly
     * on the cylinder surface.
     * \sa EGS_BaseShape::getPointSourceDirection()
     */
    void getPointSourceDirection(const EGS_Vector &Xo,
                                 EGS_RandomGenerator *rndm, EGS_Vector &u, EGS_Float &wt) {
        EGS_Vector xo = T ? Xo*(*T) : Xo;
        EGS_Float eta = rndm->getUniform()*(R+h);
        EGS_Vector x;                               // point on cylinder with respect to midpoint
        EGS_Vector n;                               // normal to cylinder at point x
        if (eta < R) {
            getPointInCircle(rndm,x.x,x.y);
            if (2*eta < R) {
                x.z = h/2;    // top face: normal is up
                n.z = 1;
            }
            else {
                x.z = -h/2;    // bottom face: normal is down
                n.z = -1;
            }
        }
        else {
            EGS_Float cphi,sphi;
            rndm->getAzimuth(cphi,sphi);
            x.x = R*cphi;
            x.y = R*sphi;
            x.z = h*(rndm->getUniform()-0.5);
            n.x = x.x;
            n.y = x.y;                   // side face: normal is (x,y)
        }
        u = (x+this->xo) - xo;                      // direction vector from origin to cylinder point
        EGS_Float d2 = u.length2(), d = sqrt(d2);
        u *= (1/d);                                 // normalize direction vectors
        n.normalize();                              // normalize normal
        wt = u*n*area()/d2;
        if (T) {
            T->rotate(u);
        }
    };
    /*! \brief Returns the cylinder surface area. */
    EGS_Float area() const {
        return 2*M_PI*R*(R+h);
    };
};

#endif

