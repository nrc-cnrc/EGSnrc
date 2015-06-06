
###############################################################################
#
#  EGSnrc egs++ sample radial repeater geometry
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
#  This file demonstrates the use of an EGS_RadialRepeater, which replicates
#  a geometry a given number of times on a rotation orbit around a user
#  specified axis.
#
###############################################################################


:start geometry definition:

    #
    # Define one tooth of the gear
    #
    :start geometry:
        library = egs_ndgeometry
        type = EGS_XYZGeometry
        name = a_tooth
        x-planes = 18 25
        y-planes = -3 3
        z-planes = -5 5
        :start media input:
            media = med
        :stop media input:
    :stop geometry:

    #
    # Repeat the tooth radially around the z-axis
    #
    :start geometry:
        library = egs_iplanes
        axis = 0 0 0 0 0 1
        type = EGS_RadialRepeater
        name = the_teeth
        repeated geometry = a_tooth
        number of repetitions = 12
    :stop geometry:

    #
    # Now a cone stack to get an axis and the gear body
    #
    :start geometry:
        library = egs_cones
        type = EGS_ConeStack
        name = base_gear
        axis = 0 0 -20  0 0 1
        :start layer:                           # regions 0 1
            thickness    = 14.999
            top radii    = 5 30
            bottom radii = 5 30
            media = med1 vacuum
        :stop layer:
        :start layer:                           # regions 4 5 6 7
            thickness    = 10.002
            top radii    = 5 6 20 30
            bottom radii = 5 6 20 30
            media = med1 vacuum med vacuum
        :stop layer:
        :start layer:                           # regions 8 9
            thickness    = 14.999
            top radii    = 5 30
            bottom radii = 5 30
            media = med1 vacuum
        :stop layer:
    :stop geometry:

    #
    # Put the_teeth into region 7 of base_gear
    #
    :start geometry:
        library = egs_cdgeometry
        name = gear
        base geometry = base_gear
        set geometry = 7 the_teeth
    :stop geometry:

    simulation geometry = gear

:stop geometry definition:

