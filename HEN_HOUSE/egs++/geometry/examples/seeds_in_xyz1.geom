
###############################################################################
#
#  EGSnrc egs++ sample brachytherapy seeds geometry, using a cd geometry
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
#  seeds put into a xyz-geometry filled with water. It is very similar to the
#  seeds_in_xyz.geom file but now the seeds are inscribed in a xyz geometry
#  using a CD geometry instead of an envelope.
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
        :stop transformation:
    :stop geometry:
    :start geometry:
        library = egs_gtransformed
        name = seed_4
        my geometry = seed
        :start transformation:
            translation =  1  1 0
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

    ########################### a space geometry to use as an envelope
    :start geometry:
        library = egs_space
        name = space
        :start media input:
            media = H2O700ICRU
        :stop media input:
    :stop geometry:

    ################# the above transformed seeds put into a space envelope
    :start geometry:
        library = egs_genvelope
        base geometry = space
        inscribed geometries = seed_1
        name = s1
    :stop geometry:
    :start geometry:
        library = egs_genvelope
        base geometry = space
        inscribed geometries = seed_2
        name = s2
    :stop geometry:
    :start geometry:
        library = egs_genvelope
        base geometry = space
        inscribed geometries = seed_3
        name = s3
    :stop geometry:
    :start geometry:
        library = egs_genvelope
        base geometry = space
        inscribed geometries = seed_4
        name = s4
    :stop geometry:
    :start geometry:
        library = egs_genvelope
        base geometry = space
        inscribed geometries = seed_5
        name = s5
    :stop geometry:
    :start geometry:
        library = egs_genvelope
        base geometry = space
        inscribed geometries = seed_6
        name = s6
    :stop geometry:
    :start geometry:
        library = egs_genvelope
        base geometry = space
        inscribed geometries = seed_7
        name = s7
    :stop geometry:
    :start geometry:
        library = egs_genvelope
        base geometry = space
        inscribed geometries = seed_8
        name = s8
    :stop geometry:
    :start geometry:
        library = egs_genvelope
        base geometry = space
        inscribed geometries = seed_9
        name = s9
    :stop geometry:
    :start geometry:
        library = egs_genvelope
        base geometry = space
        inscribed geometries = seed_10
        name = s10
    :stop geometry:
    :start geometry:
        library = egs_genvelope
        base geometry = space
        inscribed geometries = seed_11
        name = s11
    :stop geometry:
    :start geometry:
        library = egs_genvelope
        base geometry = space
        inscribed geometries = seed_12
        name = s12
    :stop geometry:
    :start geometry:
        library = egs_genvelope
        base geometry = space
        inscribed geometries = seed
        name = s0
    :stop geometry:

    ########################### now define the final geometry
    #   the annoing part here is that we have to calculate the region
    #   numbers ourself
    :start geometry:
        library  = egs_cdgeometry
        base geometry = xyz
        name = seeds_in_box
        set geometry = 629  s1
        set geometry = 641  s2
        set geometry = 653  s3
        set geometry = 665  s0
        set geometry = 677  s4
        set geometry = 689  s5
        set geometry = 701  s6
        set geometry = 635  s7
        set geometry = 645  s8
        set geometry = 655  s9
        set geometry = 675  s10
        set geometry = 685  s11
        set geometry = 695  s12
    :stop geometry:


    simulation geometry = seeds_in_box


:stop geometry definition:

