
###############################################################################
#
#  EGSnrc egs++ sample octree sphere geometry
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
#  Author:          Frederic Tessier, 2008
#
#  Contributors:
#
###############################################################################
#
#   An example geometry input file for the egs++ geometry package.
#
#   This input file defines a geometry that looks like a car (the same
#   as car.geom), and then converts it to an octree.
#
##############################################################################
#
#   An example input file for the egs++ octree geometry
#
#   This input file defines a sphere and then converts it to an octree in
#   which the positive octant features a higher resolution.
#
##############################################################################


:start geometry definition:

    :start geometry:
        name            = my_sphere
        library         = egs_spheres
        :start media input:
            media       = my_medium
        :stop media input:
        midpoint        = 0 0 0
        radii           = 0.9
    :stop geometry:

    :start geometry:
        name            = my_octree
        library         = egs_octree
        :start octree box:
            box min     = -1 -1 -1
            box max     = 1 1 1
            resolution  = 32 32 32
        :stop octree box:
        child geometry  = my_sphere
        discard child = no
    :stop geometry:

     simulation geometry = my_octree

:stop geometry definition:
