/*
###############################################################################
#
#  EGSnrc egs++ transformed source headers
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


/*! \file egs_transformed_source.h
 *  \brief A transformed source
 *  \IK
 */

#ifndef EGS_TRANSFORMED_SOURCE_
#define EGS_TRANSFORMED_SOURCE_

#include "egs_vector.h"
#include "egs_base_source.h"
#include "egs_rndm.h"
#include "egs_shapes.h"


#ifdef WIN32

    #ifdef BUILD_TRANSFORMED_SOURCE_DLL
        #define EGS_TRANSFORMED_SOURCE_EXPORT __declspec(dllexport)
    #else
        #define EGS_TRANSFORMED_SOURCE_EXPORT __declspec(dllimport)
    #endif
    #define EGS_TRANSFORMED_SOURCE_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_TRANSFORMED_SOURCE_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_TRANSFORMED_SOURCE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_TRANSFORMED_SOURCE_EXPORT
        #define EGS_TRANSFORMED_SOURCE_LOCAL
    #endif

#endif

/*! \brief A transformed source

  \ingroup Sources

A transformed source is a source that takes a particle from any
other source and then applies an \link EGS_AffineTransform
affine transformation \endlink \f$T=(R,\vec{t})\f$ to the position
and the rotation \f$R\f$ of \f$T\f$ to the direction.
A transformed source is defined as follows:
\verbatim
:start source:
    library = egs_transformed source
    name = some_name
    source name = the name of a previously defined source
    :start transformation:
        input defining a transformation
    :stop transformation:
:stop source:
\endverbatim
See EGS_AffineTransform::getTransformation() for details
on the definition of an affine transformation.

*/
class EGS_TRANSFORMED_SOURCE_EXPORT EGS_TransformedSource :
    public EGS_BaseSource {

public:

    /*! \brief Construct a transformed source using \a Source as the
    source and \a t as the transformation
    */
    EGS_TransformedSource(EGS_BaseSource *Source,
                          EGS_AffineTransform *t,
                          const string &Name="", EGS_ObjectFactory *f=0) :
        EGS_BaseSource(Name,f), source(Source), T(0) {
        setUp(t);
    };
    /*! \brief Construct a transformed source from the input \a inp */
    EGS_TransformedSource(EGS_Input *, EGS_ObjectFactory *f=0);
    void setTransformation(EGS_AffineTransform *t) {
        if (T) {
            delete T;
            T = 0;
        }
        if (t) {
            T = new EGS_AffineTransform(*t);
        }
    };
    const EGS_AffineTransform *getTransform() const {
        return T;
    };
    ~EGS_TransformedSource() {
        EGS_Object::deleteObject(source);
        if (T) {
            delete T;
        }
    };

    EGS_I64 getNextParticle(EGS_RandomGenerator *rndm,
                            int &q, int &latch, EGS_Float &E, EGS_Float &wt,
                            EGS_Vector &x, EGS_Vector &u) {
        EGS_I64 c = source->getNextParticle(rndm,q,latch,E,wt,x,u);
        if (T) {
            T->rotate(u);
            T->transform(x);
        }
        return c;
    };
    EGS_Float getEmax() const {
        return source->getEmax();
    };
    EGS_Float getFluence() const {
        return source->getFluence();
    };
    bool storeState(ostream &data) const {
        return source->storeState(data);
    };
    bool setState(istream &data) {
        return source->setState(data);
    };
    bool addState(istream &data_in) {
        return source->addState(data_in);
    };
    void resetCounter() {
        source->resetCounter();
    };

    bool isValid() const {
        return (source != 0);
    };

protected:

    EGS_BaseSource *source; //!< The source being transformed
    EGS_AffineTransform *T; //!< The affine transformation

    void setUp(EGS_AffineTransform *t);

};

#endif
