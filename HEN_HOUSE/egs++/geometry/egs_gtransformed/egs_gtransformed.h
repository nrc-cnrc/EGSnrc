/*
###############################################################################
#
#  EGSnrc egs++ gtransformed geometry headers
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


/*! \file egs_gtransformed.h
 *  \brief A transformed geometry: header
 *  \IK
 */

#ifndef EGS_GTRANSFORMED_
#define EGS_GTRANSFORMED_

#include "egs_base_geometry.h"
#include "egs_transformations.h"

#ifdef WIN32

    #ifdef BUILD_GTRANSFORMED_DLL
        #define EGS_GTRANSFORMED_EXPORT __declspec(dllexport)
    #else
        #define EGS_GTRANSFORMED_EXPORT __declspec(dllimport)
    #endif
    #define EGS_GTRANSFORMED_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_GTRANSFORMED_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_GTRANSFORMED_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_GTRANSFORMED_EXPORT
        #define EGS_GTRANSFORMED_LOCAL
    #endif

#endif

/*! \brief A transformed geometry.

  \ingroup Geometry
  \ingroup CompositeG

A transformed geometry is a geometry that has been transformed to
a different location and/or orientation by the application of an
\link EGS_AffineTransform affine transformation \endlink
\f$T(R,\vec{t})\f$, where \f$R\f$ is a rotation matrix
(a 3x3 matrix with a determinant of unity) and \f$\vec{t}\f$ is a translation
vector (so that \f$\vec{x}' = R \vec{x} + \vec{t}\f$).
A transformed geometry is useful for the following two broad classes
of situations:
- It is easier or only possible to define a geometry at a location and
  orientation different from the location and orientation needed in
  the simulation (the second case is true for a box, for instance).
- One needs two or more replicas of the same geometry object placed at
  different locations/orientations.

The implementation of the geometry methods for this geometry is very simple:
one applies the inverse affine transformation to the particle position
and the inverse rotation to the particle direction (if needed) and
then uses the corresponding method of the geometry being transformed.
A transformed geometry is defined using the following keys:
\verbatim
library = egs_gtransformed
my geometry = name of a previously defined geometry
input defining the transformation
\endverbatim
The input defining the affine transformation is described in
EGS_AffineTransform::getTransformation().

Transformed geometries are used in the
<code>car.geom, chambers_in_box.geom, seeds_in_xyz.geom</code> and
\c seeds_in_xyz1.geom example geometry files.

*/
class EGS_GTRANSFORMED_EXPORT EGS_TransformedGeometry :
    public EGS_BaseGeometry {

protected:

    EGS_BaseGeometry    *g;   //!< The geometry being transformed
    EGS_AffineTransform T;    //!< The affine transformation
    string              type; //!< The geometry type

public:

    /*! \brief Construct a geometry that is a copy of the geometry \a G
    transformed by \a t
    */
    EGS_TransformedGeometry(EGS_BaseGeometry *G, const EGS_AffineTransform &t,
                            const string &Name = "") : EGS_BaseGeometry(Name), g(G), T(t) {
        type = g->getType();
        type += "T";
        nreg = g->regions();
        is_convex = g->isConvex();
        has_rho_scaling = g->hasRhoScaling();
    };

    ~EGS_TransformedGeometry() {
        if (!g->deref()) {
            delete g;
        }
    };

    void setTransformation(const EGS_AffineTransform &t) {
        T = t;
    };

    int computeIntersections(int ireg, int n, const EGS_Vector &x,
                             const EGS_Vector &u, EGS_GeometryIntersections *isections) {
        EGS_Vector xt(x), ut(u);
        T.inverseTransform(xt);
        T.rotateInverse(ut);
        return g->computeIntersections(ireg,n,xt,ut,isections);
        //return g->computeIntersections(ireg,n,x*T,u*T.getRotation(),isections);
    };
    bool isRealRegion(int ireg) const {
        return g->isRealRegion(ireg);
    };
    bool isInside(const EGS_Vector &x) {
        EGS_Vector xt(x);
        T.inverseTransform(xt);
        return g->isInside(xt);
        //return g->isInside(x*T);
    };
    int isWhere(const EGS_Vector &x) {
        EGS_Vector xt(x);
        T.inverseTransform(xt);
        return g->isWhere(xt);
        //return g->isWhere(x*T);
    };
    int inside(const EGS_Vector &x) {
        return isWhere(x);
    };

    int medium(int ireg) const {
        return g->medium(ireg);
    };

    EGS_Float howfarToOutside(int ireg, const EGS_Vector &x,
                              const EGS_Vector &u) {
        return ireg >= 0 ? g->howfarToOutside(ireg,x*T,u*T.getRotation()) : 0;
    };
    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed=0, EGS_Vector *normal=0) {
        EGS_Vector xt(x), ut(u);
        T.inverseTransform(xt);
        T.rotateInverse(ut);
        int inew = g->howfar(ireg,xt,ut,t,newmed,normal);
        //int inew = g->howfar(ireg,x*T,u*T.getRotation(),t,newmed,normal);
        if (inew != ireg && normal) {
            *normal = T.getRotation()*(*normal);
        }
        return inew;
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        EGS_Vector xt(x);
        T.inverseTransform(xt);
        return g->hownear(ireg,xt);
        //return g->hownear(ireg,x*T);
    };

    int getMaxStep() const {
        return g->getMaxStep();
    };

    // Not sure about the following.
    // If I leave the implementation that way, all transformed copies of a
    // geometry share the same boolean properties. But that may not be
    // what the user wants. So, I should implement both options:
    // all copies share the same properties and the user has the options
    // to define separate properties for each copy.
    bool hasBooleanProperty(int ireg, EGS_BPType prop) const {
        return g->hasBooleanProperty(ireg,prop);
    };
    void setBooleanProperty(EGS_BPType prop) {
        g->setBooleanProperty(prop);
    };
    void addBooleanProperty(int bit) {
        g->addBooleanProperty(bit);
    };
    void setBooleanProperty(EGS_BPType prop, int start, int end, int step=1) {
        g->setBooleanProperty(prop,start,end,step);
    };
    void addBooleanProperty(int bit, int start, int end, int step=1) {
        g->addBooleanProperty(bit,start,end,step);
    };

    const string &getType() const {
        return type;
    };

    EGS_Float getRelativeRho(int ireg) const {
        return g->getRelativeRho(ireg);
    }
    void setRelativeRho(int start, int end, EGS_Float rho);

    void setRelativeRho(EGS_Input *);

    virtual void getLabelRegions(const string &str, vector<int> &regs);

protected:

    /*! \brief Don't define media in the transformed geometry definition.

    This function is re-implemented to warn the user to not define
    media in the definition of a transformed geometry. Instead, media should
    be defined when specifying the geometry to be transformed.
    */
    void setMedia(EGS_Input *inp,int,const int *);

};

#endif
