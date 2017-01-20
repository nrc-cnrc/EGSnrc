
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
#  An example geometry of egs_radialrepeater.
#
#
###############################################################################


:start geometry definition:

    # An EGS_RadialRepeater geometry takes the entire space. Thus, in
    # order to be able to view it, we need to limit its size. Here we
    # use a big box filled with vacuum and will later inscribe the
    # radial repetitions into this box using a CD geometry logic.
    :start geometry:
        name        = my_box
        library     = egs_box
        box size    = 4
    :stop geometry:

    # The geometry that will be repeated
    :start geometry:
        name        = my_sphere
        library     = egs_spheres
        midpoint = 1 0 0
        radii = 0.2
        :start media input:
            media = water
        :stop media input:
    :stop geometry:

    :start geometry:
        name            = my_radial_repeater
        library         = egs_iplanes
        type            = EGS_RadialRepeater
        medium          = vacuum

        repeated geometry = my_sphere

        # Axis position, direction
        axis = 0 0 1  0 0 1

        number of repetitions = 8

    :stop geometry:

    :start geometry:
        name        = my_repetition_box
        library     = egs_cdgeometry
        base geometry = my_box
        set geometry = 0 my_radial_repeater
    :stop geometry:

    simulation geometry = my_repetition_box

:stop geometry definition:
