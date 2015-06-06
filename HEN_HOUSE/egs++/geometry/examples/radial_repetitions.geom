
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
#  An example geometry input file for egspp: radial repetitions.
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
    # Define the geometry to be replicated: a simple sphere
    # (but note that any other geometry could have been used).
    #
    :start geometry:
        library = egs_spheres
        name = a_sphere
        midpoint = 22 0 0
        radii = 3
        :start media input:
            media = med
        :stop media input:
    :stop geometry:

    #
    # An EGS_RadialRepeater geometry takes the entire space. Thus, in
    # order to be able to view it, we need to limit its size. Here we
    # use a big box filled with vacuum and will later inscribe the
    # radial repetitions into this box using a CD geometry logic.
    #
    :start geometry:
        library = egs_box
        name = a_big_box
        box size = 100
    :stop geometry:

    #
    # A radial repetition obtained by rotating around the z-axis.
    # For this rotation, the initial position of the sphere is
    # in region 0 of the i-planes => the 'first angle' input is 0 degrees.
    # (it could have been left out because a missing 'first angle' input
    # implies 0 degrees).
    #
    :start geometry:
        library = egs_iplanes
        axis = 0 0 0 0 0 1
        type = EGS_RadialRepeater
        name = z_repetition
        repeated geometry = a_sphere
        number of repetitions = 12
    :stop geometry:

    #
    # A radial repetition obtained by rotating around the y-axis.
    # In order to put a_sphere into region 0 of the i-planes, we must
    # specify 'first angle' as -90 degrees (or 270 degrees).
    #
    :start geometry:
        library = egs_iplanes
        axis = 0 0 0 0 1 0
        type = EGS_RadialRepeater
        name = y_repetition
        repeated geometry = a_sphere
        number of repetitions = 12
        first angle = -90
    :stop geometry:

    #
    # A radial repetition obtained by rotating around the (1,1,1) axis
    # In order to put a_sphere into region 0 of the i-planes, we must
    # specify 'first angle' as -60 degrees (or 300 degrees).
    #
    :start geometry:
        library = egs_iplanes
        axis = 0 0 0 1 1 1
        type = EGS_RadialRepeater
        name = axis111_repetition
        repeated geometry = a_sphere
        number of repetitions = 12
        first angle = -60
    :stop geometry:

    #
    # Now inscribe the repetition of your choice into the box
    # by uncommenting the appripriate line and commenting out the
    # other 2 choices.
    #
    :start geometry:
        library = egs_cdgeometry
        name = repetition_in_a_box
        base geometry = a_big_box
        #set geometry = 0 z_repetition
        #set geometry = 0 y_repetition
        set geometry = 0 axis111_repetition
    :stop geometry:

    simulation geometry = repetition_in_a_box

:stop geometry definition:

