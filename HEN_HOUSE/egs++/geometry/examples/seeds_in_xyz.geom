
###############################################################################
#
#  EGSnrc egs++ sample brachytherapy seeds geometry
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
#  An example geometry input file for the egs++ geometry package.
#
#  This input file defines an egs++ geometry for a series of brachytherapy
#  seeds put into a xyz-geometry filled with water.
#
#  It demonstrates the use of a transformed geometry to easily create replicas
#  of the same geometry and place them at different locations and orientations.
#
#  It also demonstrates that the envelope of an envelope geometry may have
#  more than one region. This type of geometry would be useful to calculate
#  for instance interseed effects (although, one should use much smaller voxels
#  than in this example).
#
###############################################################################


:start geometry definition:

    ######################## begin of seed definition #######################

    ##################################### the set of planes needed for the
    #                                     CD geometry
    :start geometry:
        library = egs_planes
        name = planes
        type = EGS_Zplanes
        positions = -0.225 -0.175 0.175 0.225
    :stop geometry:

    ##################################### the top sphere
    #                                     (which will turn into a hemisphere
    #                                      after being put into region 0 of
    #                                      the set of planes geometry above
    :start geometry:
        library = egs_spheres
        name = seed_top
        midpoint = 0 0 -0.175
        radii = 0.05
        :start media input:
            media = titanium
        :stop media input:
    :stop geometry:

    ##################################### the set of cylinders
    :start geometry:
        library = egs_cylinders
        name = cladding
        midpoint = 0 0 0
        type = EGS_ZCylinders
        radii = 0.044 0.05
        :start media input:
            media = air titanium
            set medium = 0 1 0
            set medium = 1 2 1
        :stop media input:
    :stop geometry:

    ##################################### the first radioactive sphere
    #
    :start geometry:
        library = egs_spheres
        name = first_sphere
        midpoint = 0 0 -0.11
        radii = 0.03
        :start media input:
            media = resin
        :stop media input:
    :stop geometry:

    ##################################### the second radioactive sphere
    #
    :start geometry:
        library = egs_spheres
        name = second_sphere
        midpoint = 0 0 0
        radii = 0.03
        :start media input:
            media = resin
        :stop media input:
    :stop geometry:

    ##################################### the third radioactive sphere
    #
    :start geometry:
        library = egs_spheres
        name = third_sphere
        midpoint = 0 0 0.11
        radii = 0.03
        :start media input:
            media = resin
        :stop media input:
    :stop geometry:


    ##################################### the envelope geometry for the
    #                                     middle region
    :start geometry:
        library = egs_genvelope
        name = seed_center
        base geometry = cladding
        inscribed geometries = first_sphere second_sphere third_sphere
    :stop geometry:

    ##################################### the bottom sphere
    #                                     (which will turn into a hemisphere
    #                                      after being put into region 2 of
    #                                      the set of planes geometry above
    :start geometry:
        library = egs_spheres
        name = seed_bottom
        midpoint = 0 0 0.175
        radii = 0.05
        :start media input:
            media = titanium
        :stop media input:
    :stop geometry:


    ########################################## And now the actual geometry
    #                 which is a CD geometry made of the set of planes as
    #                 the base geometry and spheres and the cylinder
    #                 envelope as inscribed geometries for the 3 regions.
    #
    :start geometry:
        library = egs_cdgeometry
        name = seed
        base geometry = planes
        set geometry = 0 seed_top
        set geometry = 1 seed_center
        set geometry = 2 seed_bottom
    :stop geometry:

    ######################## end of seed definition #######################

    ######################## now define a XYZ-geometry to serve as the
    #                        envelope
    :start geometry:
        library   = egs_ndgeometry
        type      = EGS_XYZGeometry
        name      = xyz
        x-planes  = -5.5 -4.5 -3.5 -2.5 -1.5 -0.5 0.5 1.5 2.5 3.5 4.5 5.5
        y-planes  = -5.5 -4.5 -3.5 -2.5 -1.5 -0.5 0.5 1.5 2.5 3.5 4.5 5.5
        z-planes  = -5.5 -4.5 -3.5 -2.5 -1.5 -0.5 0.5 1.5 2.5 3.5 4.5 5.5
        :start media input:
            media = H2O700ICRU
        :stop media input:
    :stop geometry:

    ######################### make copies of the seed
    :start geometry:
        library = egs_gtransformed
        name = seed_1
        my geometry = seed
        :start transformation:
            translation = -3 -3 0
            rotation vector = 1 -1 0
        :stop transformation:
    :stop geometry:
    :start geometry:
        library = egs_gtransformed
        name = seed_2
        my geometry = seed
        :start transformation:
            translation = -2 -2 0
        :stop transformation:
    :stop geometry:
    :start geometry:
        library = egs_gtransformed
        name = seed_3
        my geometry = seed
        :start transformation:
            translation = -1 -1 0
            rotation vector = 1 -1 0
        :stop transformation:
    :stop geometry:
    :start geometry:
        library = egs_gtransformed
        name = seed_4
        my geometry = seed
        :start transformation:
            translation =  1  1 0
            rotation vector = 1 -1 0
        :stop transformation:
    :stop geometry:
    :start geometry:
        library = egs_gtransformed
        name = seed_5
        my geometry = seed
        :start transformation:
            translation =  2  2 0
        :stop transformation:
    :stop geometry:
    :start geometry:
        library = egs_gtransformed
        name = seed_6
        my geometry = seed
        :start transformation:
            translation =  3  3 0
            rotation vector = 1 -1 0
        :stop transformation:
    :stop geometry:
    :start geometry:
        library = egs_gtransformed
        name = seed_7
        my geometry = seed
        :start transformation:
            translation = 3 -3 0
        :stop transformation:
    :stop geometry:
    :start geometry:
        library = egs_gtransformed
        name = seed_8
        my geometry = seed
        :start transformation:
            translation = 2 -2 0
        :stop transformation:
    :stop geometry:
    :start geometry:
        library = egs_gtransformed
        name = seed_9
        my geometry = seed
        :start transformation:
            translation = 1 -1 0
        :stop transformation:
    :stop geometry:
    :start geometry:
        library = egs_gtransformed
        name = seed_10
        my geometry = seed
        :start transformation:
            translation = -1  1 0
        :stop transformation:
    :stop geometry:
    :start geometry:
        library = egs_gtransformed
        name = seed_11
        my geometry = seed
        :start transformation:
            translation = -2  2 0
        :stop transformation:
    :stop geometry:
    :start geometry:
        library = egs_gtransformed
        name = seed_12
        my geometry = seed
        :start transformation:
            translation = -3  3 0
        :stop transformation:
    :stop geometry:

    ########################### now define the final geometry
    :start geometry:
        library  = egs_genvelope
        base geometry = xyz
        name = seeds_in_box
        inscribed geometries = seed seed_1 seed_2 seed_3 seed_4 seed_5 seed_6 \
                         seed_7 seed_8 seed_9 seed_10 seed_11 seed_12
    :stop geometry:


    simulation geometry = seeds_in_box

:stop geometry definition:

