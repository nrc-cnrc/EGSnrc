
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
#  Author:          Iwan Kawrakow, 2007
#
#  Contributors:
#
###############################################################################
#
#  An example of radial repetitions around the x-, y- and z-axis.
#
#  This file demonstrates the use of an EGS_RadialRepeater, which replicates
#  a geometry a given number of times on a rotation orbit around a user
#  specified axis. The main difficulty when using a radial repeater is to
#  determine the region of the geometry that is to be replicated in the
#  i-planes needed to divide space into the required number of segments.
#
###############################################################################


:start geometry definition:

    #
    # The geometry to be replicated by rotation around the z-axis
    #
    :start geometry:
        library = egs_spheres
        name = z_sphere
        midpoint = 30 0 0
        radii = 5
        :start media input:
            media = med1
        :stop media input:
    :stop geometry:
    #
    # The z-axis replication
    #
    :start geometry:
        library = egs_iplanes
        axis = 0 0 0 0 0 1
        type = EGS_RadialRepeater
        name = z_repetition
        repeated geometry = z_sphere
        number of repetitions = 12
        first angle = 0
    :stop geometry:

    #
    # The geometry to be replicated by rotation around the y-axis
    #
    :start geometry:
        library = egs_spheres
        name = y_sphere
        midpoint = 22 0 0
        radii = 2.999
        :start media input:
            media = med2
        :stop media input:
    :stop geometry:
    #
    # The y-axis replication
    #
    :start geometry:
        library = egs_iplanes
        axis = 0 0 0 0 1 0
        type = EGS_RadialRepeater
        name = y_repetition
        repeated geometry = y_sphere
        number of repetitions = 18
        first angle = -90
    :stop geometry:

    #
    # The geometry to be replicated by rotation around the x-axis
    #
    :start geometry:
        library = egs_spheres
        name = x_sphere
        midpoint = 0 0 17
        radii = 1.999
        :start media input:
            media = med3
        :stop media input:
    :stop geometry:
    #
    # The y-axis replication
    #
    :start geometry:
        library = egs_iplanes
        axis = 0 0 0 1 0 0
        type = EGS_RadialRepeater
        name = x_repetition
        repeated geometry = x_sphere
        number of repetitions = 24
        first angle = -180
    :stop geometry:

    #
    # Now a set of spheres filled with vacuum to be used as
    # base for a CD geometry to hold the 3 repetitions.
    #
    :start geometry:
        library = egs_spheres
        name = container
        radii = 19 25 36
    :stop geometry:

    #
    # We now inscribe the repetitions into the set of spheres
    # using a CD geometry
    #
    :start geometry:
        library = egs_cdgeometry
        name = repetitions
        base geometry = container
        set geometry = 0 x_repetition
        set geometry = 1 y_repetition
        set geometry = 2 z_repetition
    :stop geometry:

    simulation geometry = repetitions

:stop geometry definition:

