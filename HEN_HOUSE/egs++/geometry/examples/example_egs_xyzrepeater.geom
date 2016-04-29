
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
#  An example geometry of egs_xyzrepeater.
#
#
###############################################################################


:start geometry definition:
    :start geometry:
        name        = my_sphere
        library     = egs_spheres
        midpoint = 0
        radii = 0.5
        :start media input:
            media = water
        :stop media input:
    :stop geometry:

    :start geometry:
        name            = my_xyz_repeater
        library         = egs_ndgeometry
        type            = EGS_XYZRepeater
        medium          = air

        repeated geometry = my_sphere

        # Format is: min, max, N
        repeat x = -1 4 3
        repeat y = -1 4 3
        repeat z = -1 4 3
    :stop geometry:

    simulation geometry = my_xyz_repeater

:stop geometry definition:
