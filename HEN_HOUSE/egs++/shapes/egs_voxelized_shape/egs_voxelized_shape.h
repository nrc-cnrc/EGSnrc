/*
###############################################################################
#
#  EGSnrc egs++ voxelized shape headers
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
#  Author:          Iwan Kawrakow, 2009
#
#  Contributors:    Frederic Tessier
#
###############################################################################
*/


/*! \file egs_voxelized_shape.h
 *  \brief A "voxelized shape": header
 *  \IK
 */

#ifndef EGS_VOXELIZED_SHAPE_
#define EGS_VOXELIZED_SHAPE_

#include "egs_shapes.h"
#include "egs_rndm.h"
#include "egs_math.h"
#include "egs_alias_table.h"

#ifdef WIN32

    #ifdef BUILD_VOXELIZED_SHAPE_DLL
        #define EGS_VOXELIZED_SHAPE_EXPORT __declspec(dllexport)
    #else
        #define EGS_VOXELIZED_SHAPE_EXPORT __declspec(dllimport)
    #endif
    #define EGS_VOXELIZED_SHAPE_LOCAL

#else

    #ifdef HAVE_VISIBILITY
        #define EGS_VOXELIZED_SHAPE_EXPORT __attribute__ ((visibility ("default")))
        #define EGS_VOXELIZED_SHAPE_LOCAL  __attribute__ ((visibility ("hidden")))
    #else
        #define EGS_VOXELIZED_SHAPE_EXPORT
        #define EGS_VOXELIZED_SHAPE_LOCAL
    #endif

#endif

/*! \brief A "voxelized shape"

\ingroup Shapes
\ingroup SurfaceS

A shape of this type can be used to pick points randomly within a XYZ grid
where each voxel has a given probability to be selected. This is useful,
for instance, to model a radioactivity distribution.
This shape is specified via
\verbatim
:start shape:
    library = egs_voxelized_shape
    file name = some_file
:stop shape:
\endverbatim
The \c some_file file must be a binary file containing the following information:
 - 1 byte indicating the endianess of the machine the file was created on
   (0 = big endian, 1 = little endian)
 - 1 byte indicating the file format (0 or 1, see bellow)
 - Three 16 bit integers for the number of voxels Nx, Ny, Nz
 - Nx+1 floats for the positions of the x-planes defining the XYZ grid
 - Ny+1 floats for the positions of the y-planes defining the XYZ grid
 - Nz+1 floats for the positions of the z-planes defining the XYZ grid
 - For format 0, Nx*Ny*Nz 32 bit floats defining the probability for the
   Nx*Ny*Nz voxels, where voxel (ix,iy,iz) is at position ix + iy*Nx + iz*Nx*Ny
 - For format 1, a 32 bit integer (N) indicating the number of data entries,
   followed by N 32 bit integers, followed by N 32 bit floats. In this case
   the integer define voxe indeces and the floats the corresponding probabilities.
   This format is useful when the number of non-zero probability voxels is small
   compared to the total number of voxels.
*/
class EGS_VOXELIZED_SHAPE_EXPORT EGS_VoxelizedShape : public EGS_BaseShape {

public:

    /*! \brief Conctructor

    Construct a voxelized shape from the data provided in the binary file
    \c fname
    */
    EGS_VoxelizedShape(int file_format, const char *fname,const string &Name="",
                       EGS_ObjectFactory *f=0);
    ~EGS_VoxelizedShape();
    void EGS_VoxelizedShapeFormat0(const char *fname,const string &Name="",
                                   EGS_ObjectFactory *f=0);
    EGS_Vector getPoint(EGS_RandomGenerator *rndm) {
        int bin = prob->sample(rndm);
        int voxel = type == 0 ? bin : map[bin];
        int iz = voxel/nxy;
        voxel -= iz*nxy;
        int iy = voxel/nx;
        int ix = voxel - iy*nx;
        EGS_Float eta_x = rndm->getUniform(),
                  eta_y = rndm->getUniform(),
                  eta_z = rndm->getUniform();
        return EGS_Vector(xpos[ix]*(1-eta_x) + xpos[ix+1]*eta_x,
                          ypos[iy]*(1-eta_y) + ypos[iy+1]*eta_y,
                          zpos[iz]*(1-eta_z) + zpos[iz+1]*eta_z);
    };

    bool isValid() const {
        return (type == 0 || type == 1);
    };

protected:

    EGS_SimpleAliasTable *prob;   ///! The alias table for randomly picking voxels
    EGS_Float  *xpos;             ///! The x-positions of the grid
    EGS_Float  *ypos;             ///! The y-positions of the grid
    EGS_Float  *zpos;             ///! The z-positions of the grid
    int        *map;              ///! Voxel map (for type=1)
    int        nx, ny, nz, nxy, nreg;
    int        type;
};

#endif
