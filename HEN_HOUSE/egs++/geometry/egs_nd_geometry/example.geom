
###############################################################################
#
#  EGSnrc egs++ nd geometry example
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
#  Author:          Iwan Kawrakow, 2008
#
#  Contributors:
#
###############################################################################
#
#  An example input file showing the voxelization of a geometry.
#
###############################################################################


:start geometry definition:

    #
    #  The geometry we will turn into a XYZ geometry
    #
    :start geometry:

        library = egs_vhp_geometry
        name    = fax06

        #
        # The binary organ data file
        #
        phantom data = ../egs_vhp_geometry/fax06_med1.bin

        #
        # The ASCII media data file mapping organ indeces to media indeces
        # and defining organ and media names.
        #
        media data =  ../egs_vhp_geometry/organ.data

    :stop geometry:

    #
    # The XYZ Geometry representation of the above
    #
    :start geometry:
        library = egs_ndgeometry
        type    = EGS_XYZGeometry
        name    = voxelized_fax06
        x-slabs = -11.94 0.12 199
        y-slabs = -13.14 0.12 219
        z-slabs = -10.86 0.12 181
        voxelize geometry = fax06  # name of the geometry to be voxelized
        # The transformation to be applied before looking up the medium
        # in the fax06 geometry
        :start transformation:
            translation = 28.44 13.2 10.86
        :stop transformation:
        delete geometry = yes
    :stop geometry:

    simulation geometry = voxelized_fax06

:stop geometry definition:
