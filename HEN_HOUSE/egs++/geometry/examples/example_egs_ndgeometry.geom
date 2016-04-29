
###############################################################################
#
#  EGSnrc egs++ sample geometry
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
#  Author:          Reid Townson, 2016
#
#  Contributors:
#
###############################################################################
#
#  An example geometry of egs_ndgeometry.
#
#
###############################################################################


:start geometry definition:

    # First dimension
    :start geometry:
        name        = my_nd_iplanes
        library     = egs_iplanes
        axis        = 0 0 0  0 0 1
        angles      = 0 45 90 135
        # No media required
    :stop geometry:

    # Second dimension
    :start geometry:
        name        = my_nd_cylinders
        library     = egs_cylinders
        type        = EGS_ZCylinders
        radii       = 4 5
        # No medium required
    :stop geometry:

    # Third dimension
    :start geometry:
        name        = my_sphere
        library     = egs_spheres
        midpoint    = 0 0 0
        radii       = 10
    :stop geometry:

    # nd geometry
    :start geometry:
        name            = my_nd
        library         = egs_ndgeometry
        dimensions      = my_nd_iplanes my_nd_cylinders  my_sphere
        hownear method  = 1
        :start media input:
            media = water air water
            set medium = 0 0
            set medium = 1 1
            set medium = 2 0
            set medium = 3 1
            set medium = 4 0
            set medium = 5 1
            set medium = 6 0
            set medium = 7 1
        :stop media input:
    :stop geometry:

    simulation geometry = my_nd

:stop geometry definition:
