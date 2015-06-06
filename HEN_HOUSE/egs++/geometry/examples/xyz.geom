
###############################################################################
#
#  EGSnrc egs++ sample xyz geometry
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
#
#  An example geometry input file for the egs++ geometry package.
#
#  This input file defines an egs++ geometry for a simple xyz-geometry filled
#  with water and two inhomogeneities
#
###############################################################################


:start geometry definition:

    :start geometry:

        library   = egs_ndgeometry
        type      = EGS_XYZGeometry
        name      = xyz
        x-planes  = -11 -9 -7 -5 -3 -1 1 3 5 7 9 11
        y-planes  = -11 -9 -7 -5 -3 -1 1 3 5 7 9 11
        z-planes  = 0 0.5 1 1.5 2 2.5 3 3.5 4 4.5 5 5.5 6 6.5 7 7.5 8 8.5 9 \
                    10 10.5 11 13 14.5 16 17 20
        :start media input:
            media = H2O700ICRU AL700ICRU AIR700ICRU
            set medium = 4 5 4 5 4 10 1
            # the above sets the medium to AL700ICRU in all voxels with
            # ix=4,5 iy=4,5 iz=4,10
            set medium = 5 6 5 6 4 10 2
            # the above sets the medium to AIR700ICRU in all voxels with
            # ix=5,6 iy=5,6 iz=4,10 (i.e., some of the voxels previously
            # set to AL700ICRU get refilled with air).
        :stop media input:

    :stop geometry:

    simulation geometry = xyz

:stop geometry definition:

