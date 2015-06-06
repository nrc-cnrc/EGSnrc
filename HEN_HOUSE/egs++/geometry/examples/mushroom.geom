
###############################################################################
#
#  EGSnrc egs++ sample mushroom geometry
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
#   This input file defines a geometry that looks like a mushroom.
#
#   The geometry is made from a set of planes, a sphere and a cylinder put
#   into the two regions defined by the planes as a CD geometry.
#
##############################################################################


:start geometry definition:

    ################################### The planes
    :start geometry:
        library   = egs_planes
        type      = EGS_Zplanes
        name      = the_planes
        positions = 0  4.0  5
    :stop geometry:

    ################################### The sphere
    :start geometry:
        library   = egs_spheres
        midpoint  = 0 0 -1
        radii     = 6
        name      = the_sphere
        :start media input:
            media = medium1
        :stop media input:
    :stop geometry:

    ################################### The cylinder
    :start geometry:
        library   = egs_cylinders
        type      = EGS_ZCylinders
        name      = the_cylinder
        radii     = 1
        :start media input:
            media = medium2
        :stop media input:
    :stop geometry:

    ################################### The CD geometry:
    :start geometry:
        library   = egs_cdgeometry
        name      = mushroom
        base geometry = the_planes
        set geometry = 1 the_sphere
        set geometry = 0 the_cylinder
    :stop geometry:

    simulation geometry = mushroom

:stop geometry definition:

