/*
###############################################################################
#
#  EGSnrc egs++ spheres geometry headers
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


/*! \file egs_spheres.h
 *  \brief A set of concentric spheres
 *  \author Declan Persram and Iwan Kawrakow
 */

#ifndef EGS_CSPHERES_
#define EGS_CSPHERES_

#include "egs_base_geometry.h"

#ifdef WIN32

    #ifdef BUILD_SPHERES_DLL
        #define EGS_SPHERES_EXPORT __declspec(dllexport)
    #else
        #define EGS_SPHERES_EXPORT __declspec(dllimport)
    #endif
    #define EGS_SPHERES_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_SPHERES_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_SPHERES_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_SPHERES_EXPORT
        #define EGS_SPHERES_LOCAL
    #endif

#endif

/*! \brief A set of concentric spheres

  \ingroup Geometry
  \ingroup ElementaryG

The EGS_cSpheres class implements the geometry methods for a set of
\f$N\f$ concentric spheres which define a geometry with \f$N\f$ regions.
As in the case of a \link EGS_CylindersT set of cylinders \endlink,
the region index within the
innermost sphere is 0 with the region index increasing towards the last sphere
in the set.

The additional keys needed to construct a set of spheres is
\verbatim

library = egs_spheres
midpoint = Ox, Oy, Oz
radii = list of sphere radii in increasing order

\endverbatim

Examples of the usage of sets of spheres can be found in the
\c cones.geom, \c hemisphere.geom, \c mushroom.geom, \c rounded_ionchamber.geom
and \c I6702.inp example geometry files.

*/
class EGS_SPHERES_EXPORT EGS_cSpheres : public EGS_BaseGeometry {

public:

    // construct some CONCENTRIC spheres
    EGS_cSpheres(int ns, const EGS_Float *radius, const EGS_Vector &position,
                 const string &Name = "");

    // destruct spheres from memory
    ~EGS_cSpheres() {
        if (nreg) {
            delete [] R2;
            delete [] R;
        }
    }

    // method to determine which spheres we are in(between)
    int inside(const EGS_Vector &x);

    bool isInside(const EGS_Vector &x);
    int isWhere(const EGS_Vector &x);

    EGS_Float howfarToOutside(int ireg, const EGS_Vector &x,
                              const EGS_Vector &u);
    // howfar is particle trajectory from sphere boundry
    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed=0, EGS_Vector *normal=0);

    // hownear - closest perpendicular distance to sphere surface
    EGS_Float hownear(int ireg, const EGS_Vector &x);

    int getMaxStep() const {
        return 2*nreg;
    };

    const string &getType() const {
        return type;
    };

    void printInfo() const;

private:

    EGS_Float *R2;                // radius^2
    EGS_Float *R;                 // radius
    EGS_Vector xo;                // for conc-spheres, all centres coincide
    static string type;
};

#endif
