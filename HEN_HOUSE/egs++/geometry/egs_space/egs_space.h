/*
###############################################################################
#
#  EGSnrc egs++ space geometry headers
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


/*! \file egs_space.h
 *  \brief The entire space as a geometry
 *  \IK
 */

#ifndef EGS_SPACE_
#define EGS_SPACE_

#include "egs_base_geometry.h"
#include "egs_functions.h"

#ifdef WIN32

    #ifdef BUILD_SPACE_DLL
        #define EGS_SPACE_EXPORT __declspec(dllexport)
    #else
        #define EGS_SPACE_EXPORT __declspec(dllimport)
    #endif
    #define EGS_SPACE_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_SPACE_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_SPACE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_SPACE_EXPORT
        #define EGS_SPACE_LOCAL
    #endif

#endif

/*! \brief The entire space as a geometry object.

\ingroup Geometry
\ingroup ElementaryG

 The EGS_Space class is the simplest possible geometry: it models the
 entire space as a single region filled with a given medium, \em i.e.
 any position is inside the geometry, any position is always infinitely
 away from a boundary and a line never intersects a boundary.

 The EGS_Space class is useful for using it as the envelope to some
 other geometry (see \em e.g. \c seeds_in_xyz1.geom). A space geometry is
 defined using

 \verbatim
 library = egs_space
 \endverbatim

*/
class EGS_SPACE_EXPORT EGS_Space : public EGS_BaseGeometry {

protected:

    static string type;

public:

    EGS_Space(const string &Name) : EGS_BaseGeometry(Name) {
        nreg=1;
    };

    bool isInside(const EGS_Vector &x) {
        return true;
    };

    int isWhere(const EGS_Vector &x) {
        return 0;
    };

    int inside(const EGS_Vector &x) {
        return 0;
    };

    EGS_Float howfarToOutside(int ireg, const EGS_Vector &x,
                              const EGS_Vector &u) {
        return 1e30;
    };

    int howfar(int ireg, const EGS_Vector &x, const EGS_Vector &u,
               EGS_Float &t, int *newmed=0, EGS_Vector *normal=0) {
        return ireg;
    };

    EGS_Float hownear(int ireg, const EGS_Vector &x) {
        return 1e30;
    };

    const string &getType() const {
        return type;
    };

    void printInfo() const {
        EGS_BaseGeometry::printInfo();
        egsInformation(
            "=======================================================\n");
    };

};

#endif
