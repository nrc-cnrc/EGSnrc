/*
###############################################################################
#
#  EGSnrc egs++ rz geometry headers
#  Copyright (C) 2016 Randle E. P. Taylor, Rowan M. Thomson,
#  Marc J. P. Chamberland, Dave W. O. Rogers
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
#  Author:          Randle Taylor, 2016
#
#  Contributors:    Marc Chamberland
#                   Rowan Thomson
#                   Dave Rogers
#                   Martin Martinov
#
###############################################################################
#
#  egs_rz was developed for the Carleton Laboratory for Radiotherapy Physics.
#
###############################################################################
*/


/*! \file egs_rz.h
 *  \brief An egs_nd_geometry wrapper to simplify RZ geometry creation
 *  \author Randle Taylor
 */

#include <string>
#include <algorithm>
#include <fstream>
#include "../egs_nd_geometry/egs_nd_geometry.h"

#ifndef EGS_RZ_
#define EGS_RZ_

#ifdef WIN32

    #define EGS_RZ_EXPORT __declspec(dllexport)
    #define EGS_RZ_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_RZ_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_RZ_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_RZ_EXPORT
        #define EGS_RZ_LOCAL
    #endif

#endif

/*! \brief a subclass of EGS_NDGeometry for conveniently defining an RZ geometry

\ingroup Geometry
\ingroup CompositeG

Input blocks for the RZ geometry look like the following:

\verbatim
:start geometry:
    name = phantom
    library = egs_rz
    radii =  1, 2, 3
    z-planes = -4 -3 -2 -1 0 1 2 3 4
    :start media input:
        media = M0
    :stop media input:
:stop geometry:
\endverbatim

which would create a `num_regions = n_r_bounds*(n_z_bounds - 1) = 3*(9-1) = 24`
region geometry with index numbers ranging from 0 to 23.

Alternatively you can specify the geometry using slabs & shells:

\verbatim
:start geometry:
    name = phantom
    library = egs_rz

    number of shells = 2 2 2
    shell thickness =  0.5 1 2

    first plane = -4
    number of slabs = 2 8 2
    slab thickness  = 1 0.5 1

    :start media input:
        media = M0
    :stop media input:
:stop geometry:
\endverbatim

Example geometries are available in the rz_lib1.geom and rz_lib2.geom files.

*/

class EGS_RZ_EXPORT EGS_RZGeometry: public EGS_NDGeometry {

private:

    vector<EGS_Float> radii; /*! cylinder radii. Note radii[0] is always set to 0 */
    vector<EGS_Float> zbounds;
    vector<EGS_Float> reg_vol; /* calculated vol of each region */
    static string RZType;

public:

    /*! ZDIR = 0, RDIR = 1 */
    enum DIRS {ZDIR, RDIR};

    /*! \brief RZ geometry constructor
     *
     * geoms must be a 2 length vector of [EGS_CylindersZ*, EGS_PlanesZ]
     * and the radii/zbounds of the geometries must be passed in order
     * to allow calculation of the region masses */
    EGS_RZGeometry(vector<EGS_BaseGeometry *> geoms,
                   vector<EGS_Float> rads, vector<EGS_Float> zbs, const string &name = "");

    ~EGS_RZGeometry() {};

    /*! \brief return RZ geometry type */
    const string &getType() const {
        return RZType;
    };

    /*! \brief get RZ boundary for a given direction and directional index
     *
     * This is useful for e.g. outputing bounds for a 3ddose file */
    EGS_Float getBound(int idir, int ind);


    /*! \brief get number of regions in a given direction
     *
     * This is useful for e.g. outputing bounds for a 3ddose file */
    int getNRegDir(int dir);

    /*! \brief get mass of a given region  */
    EGS_Float getVolume(int ireg);

};


#endif
