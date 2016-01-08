/*
###############################################################################
#
#  EGSnrc egs++ line shape headers
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


/*! \file egs_line_shape.h
 *  \brief A line shape
 *  \IK
 */

#ifndef EGS_LINE_SHAPE_
#define EGS_LINE_SHAPE_

#include "egs_shapes.h"
#include "egs_rndm.h"
#include "egs_alias_table.h"

#ifdef WIN32

    #ifdef BUILD_LINE_SHAPE_DLL
        #define EGS_LINE_SHAPE_EXPORT __declspec(dllexport)
    #else
        #define EGS_LINE_SHAPE_EXPORT __declspec(dllimport)
    #endif
    #define EGS_LINE_SHAPE_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_LINE_SHAPE_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_LINE_SHAPE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_LINE_SHAPE_EXPORT
        #define EGS_LINE_SHAPE_LOCAL
    #endif

#endif

/*! \brief A line shape

\ingroup Shapes

A line shape consisting of one or more line segments that delivers
points uniformly distributed along the line segments. This shape is
defined via
\verbatim
:start shape:
    library = egs_line_shape
    points = list of 2D positions, at least 2 required.
:stop shape:
\endverbatim
Note that the line is assumed to be in the xy-plane at z=0.
For other lines, attach an \link EGS_AffineTransform affine
transformation\endlink to the line shape.

*/
class EGS_LINE_SHAPE_EXPORT EGS_LineShape : public EGS_BaseShape {

public:

    EGS_LineShape(const vector<EGS_Float> &points, const string &Name="",
                  EGS_ObjectFactory *f=0);
    ~EGS_LineShape() {
        if (n > 0) {
            delete table;
            delete [] x;
            delete [] y;
        }
    };
    EGS_Vector getPoint(EGS_RandomGenerator *rndm) {
        int j = table->sampleBin(rndm);
        EGS_Float eta = rndm->getUniform();
        EGS_Float eta1 = 1 - eta;
        return EGS_Vector(x[j]*eta+x[j+1]*eta1,y[j]*eta+y[j+1]*eta1,0);
    };

protected:

    int  n;  // number of line segments
    EGS_Float *x, *y;
    EGS_AliasTable    *table;

};

#endif
