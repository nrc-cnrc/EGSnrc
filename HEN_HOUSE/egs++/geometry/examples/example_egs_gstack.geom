
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
#  An example geometry of egs_gstack.
#
#
###############################################################################


:start geometry definition:

    :start geometry:
        library = egs_cones
        type = EGS_ConeStack
        name = my_conestack
        axis = 0 0 2.6 0 0 -1
        :start layer:
            thickness = 0.05
            top radii = 0.
            bottom radii = 0.0858
            media = water
        :stop layer:
        :start layer:
            thickness = 0.1
            top radii = 0. 0.0858
            bottom radii = 0.3125 0.35
            media = air water
        :stop layer:
        :start layer:
            thickness = 0.2
            bottom radii = 0.3125 0.35
            media = air water
        :stop layer:
        :start layer:
            thickness = 2
            top radii = 0.050 0.3125 0.35
            bottom radii = 0.050 0.3125 0.35
            media = water air water
        :stop layer:
    :stop geometry:

    :start geometry:
        name        = my_box
        library     = egs_box
        box size    = 3 3 .5
        :start media input:
            media = water
        :stop media input:
    :stop geometry:

    :start geometry:
        name            = my_stack
        library         = egs_gstack
        geometries      = my_conestack my_box
        tolerance       = 1e-4
    :stop geometry:

    simulation geometry = my_stack

:stop geometry definition:
