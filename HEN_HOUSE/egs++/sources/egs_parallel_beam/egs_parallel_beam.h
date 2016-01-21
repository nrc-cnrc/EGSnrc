/*
###############################################################################
#
#  EGSnrc egs++ parallel beam source headers
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


/*! \file egs_parallel_beam.h
 *  \brief A parallel beam
 *  \IK
 */

#ifndef EGS_PARALLEL_BEAM_
#define EGS_PARALLEL_BEAM_

#include "egs_vector.h"
#include "egs_base_source.h"
#include "egs_rndm.h"
#include "egs_shapes.h"


#ifdef WIN32

    #ifdef BUILD_PARALLEL_BEAM_DLL
        #define EGS_PARALLEL_BEAM_EXPORT __declspec(dllexport)
    #else
        #define EGS_PARALLEL_BEAM_EXPORT __declspec(dllimport)
    #endif
    #define EGS_PARALLEL_BEAM_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_PARALLEL_BEAM_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_PARALLEL_BEAM_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_PARALLEL_BEAM_EXPORT
        #define EGS_PARALLEL_BEAM_LOCAL
    #endif

#endif

/*! \brief A parallel beam.

  \ingroup Sources

A parallel beam is a source that delivers particles all having
the same direction with a position within
\link EGS_BaseShape any shape. \endlink
Although
planar shapes would make more sense for a parallel beam, due
to the abstract nature of shapes, any shape can be used.
A parallel beam is defined as follows:
\verbatim
:start source:
    library = egs_parallel_beam
    name = some_name
    :start shape:
        definition of the shape
    :stop shape:
    :start spectrum:
        definition of the spectrum
    :stop spectrum:
    direction = Ux Uy Uz
    charge = -1 or 0 or 1 for electrons or photons or positrons
:stop source:
\endverbatim

It is worth noting that the functionality of sources 0, 2, 10 and 13
from the RZ series of user codes and sources 0 and 1
in DOSXYZnrc can be reproduced with
the parallel beam source from the EGSnrc C++ class library.

*/
class EGS_PARALLEL_BEAM_EXPORT EGS_ParallelBeam :
    public EGS_BaseSimpleSource {

public:

    /*! \brief Constructor

    Construct a parallel beam with charge \a Q and spectrum \a Spec from
    a shape \a Shape
    */
    EGS_ParallelBeam(int Q, EGS_BaseSpectrum *Spec, EGS_BaseShape *Shape,
                     const string &Name="", EGS_ObjectFactory *f=0) :
        EGS_BaseSimpleSource(Q,Spec,Name,f), shape(Shape), uo(0,0,1) {
        setUp();
    };

    /*! \brief Constructor

    Construct a parallel beam from the information pointed to by \a inp.
    */
    EGS_ParallelBeam(EGS_Input *, EGS_ObjectFactory *f=0);
    ~EGS_ParallelBeam() {
        EGS_Object::deleteObject(shape);
    };

    void getPositionDirection(EGS_RandomGenerator *rndm,
                              EGS_Vector &x, EGS_Vector &u, EGS_Float &wt) {
        x = shape->getRandomPoint(rndm);
        u = uo;
        wt = 1;
    };

    EGS_Float getFluence() const {
        return count/shape->area();
    };

    bool storeFluenceState(ostream &) const {
        return true;
    };

    bool setFluenceState(istream &) {
        return true;
    };

    bool isValid() const {
        return (s != 0 && shape != 0);
    };

protected:

    EGS_BaseShape *shape; //!< The shape.
    EGS_Vector    uo;     //!< The direction of the particles.

    void setUp();

};

#endif
