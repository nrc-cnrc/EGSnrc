
###############################################################################
#
#  EGSnrc egs++ sample octree car geometry
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
#  Author:          Frederic Tessier, 2008
#
#  Contributors:
#
###############################################################################
#
#   An example geometry input file for the egs++ geometry package.
#
#   This input file defines a geometry that looks like a car (the same as
#   car.geom), and then converts it to an octree.
#
##############################################################################


:start geometry definition:


    ######################################## the front wheels
    :start geometry:
        library = egs_cones
        type    = EGS_ConeStack
        name    = front_wheels
        axis    = -3.5 -3 -1  1 0 0
        :start layer:
            thickness    = 0.5
            top radii    = 2
            bottom radii = 2
            media        = wheel_medium
        :stop layer:
        :start layer:
            thickness    = 6
            top radii    = 0.5 2
            bottom radii = 0.5 2
            media        = axis_medium vacuum
        :stop layer:
        :start layer:
            thickness    = 0.5
            top radii    = 2
            bottom radii = 2
            media        = wheel_medium
        :stop layer:
    :stop geometry:


    ######################################## the rear wheels
    #    modeled using a transformed geometry of the front wheels
    :start geometry:
        library = egs_gtransformed
        my geometry = front_wheels
        :start transformation:
            translation = 0 6 0
        :stop transformation:
        name = rear_wheels
    :stop geometry:


    ######################################## the lower car body
    :start geometry:
        library = egs_box
        box size = 5 14 3
        name = lower_car_body
        :start media input:
            media = body_medium
        :stop media input:
        :start transformation:
            translation = 0 -1 0
        :stop transformation:
    :stop geometry:


    ####################################### the upper car body
    :start geometry:
        library = egs_box
        box size = 5  6 2
        name = upper_car_body
        :start transformation:
            translation = 0 0 2.5
        :stop transformation:
        :start media input:
            media = body_medium
        :stop media input:
    :stop geometry:


    ####################################### the lights
    :start geometry:
        library = egs_cones
        type = EGS_ConeStack
        axis = -1.5 -8.2 0   0 1 0
        :start layer:
            thickness = 0.25
            top radii = 0.75
            bottom radii = 0.75
            media = lights_medium
        :stop layer:
        name = left_light
    :stop geometry:
    :start geometry:
        library = egs_gtransformed
        my geometry = left_light
        name = right_light
        :start transformation:
            translation = 3.0 0 0
        :stop transformation:
    :stop geometry:


    ######################################## combine everything into a union
    :start geometry:
        library = egs_gunion
        name    = the_car
        geometries = upper_car_body lower_car_body front_wheels rear_wheels ,
                     left_light right_light
    :stop geometry:

    :start geometry:
        library = egs_box
        name    = the_box
        box size = 18
        :start media input:
            media = vacuum
        :stop media input:
    :stop geometry:

    :start geometry:
        library = egs_genvelope
        name = car_in_box
        base geometry = the_box
        inscribed geometries = the_car
     :stop geometry:


    ######################################## make an octree geometry out of the car

    :start geometry:
        library         = egs_octree
        name            = my_octree
        :start octree box:
            box min     = -4 -16 -4
            box max     = 4 16 4
            resolution  = 64 256 64
        :stop octree box:
        child geometry  = car_in_box
    :stop geometry:

     simulation geometry = my_octree

:stop geometry definition:

