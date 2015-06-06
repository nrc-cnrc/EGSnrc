
###############################################################################
#
#  EGSnrc egs++ sample ionization chambers in a box geometry
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
#  This input file defines an egs++ geometry for 3 ionization chambers with
#  rounded ends put into a box filled with water. For details of the
#  definition of the chamber geometry, see rounded_chamber.geom.
#
#  This input file demonstrates the use of a transformed geometry
#  to easily create replicas of the same geometry and place
#  these at different locations.
#
##############################################################################


:start geometry definition:


    ######################### chamber definition ###########################

    ################################## the set of planes used as the base
    #                                  of the CD geometry
    :start geometry:
        library   = egs_planes
        type      = EGS_Zplanes
        name      = base_planes
        positions = -2 -1 1 2
    :stop geometry:

    ################################# the set of spheres needed for the top
    #                                 we don't set the media here as media
    #                                 will be set in the 2D geometry below
    #
    :start geometry:
        library  = egs_spheres
        name     = top_spheres
        midpoint = 0 0 -1
        radii    = 0.7 1
    :stop geometry:

    ################################# the set of cylinders needed for the top
    #                                 we don't set the media here as media
    #                                 will be set in the 2D geometry below
    :start geometry:
        library  = egs_cylinders
        type     = EGS_ZCylinders
        name     = top_cylinders
        radii    = 0.3  0.7  1
    :stop geometry:

    ################################# the 2D geometry for the top chamber end
    #                                 It is made from the sets of spheres
    #                                 and cylinders defined above to form
    #                                 a geometry with 2x3=6 regions
    #                                 this geometry is inscribed directly into
    #                                 the set of planes and therefore we
    #                                 define its media
    #
    :start geometry:
        library    = egs_ndgeometry
        name       = top_chamber_part
        dimensions = top_spheres top_cylinders
        :start media input:
            media = 170C521ICRU AIR521ICRU AL521ICRU
            set medium = 0 5 0
            set medium = 0 2
            set medium = 1 2
            set medium = 2 1
        :stop media input:
    :stop geometry:

    ################################# the set of spheres needed for the bottom
    #                                 this geometry is inscribed directly into
    #                                 the set of planes and therefore we
    #                                 define its media
    #
    :start geometry:
        library  = egs_spheres
        name     = bottom_chamber_part
        midpoint = 0 0 1
        radii    = 0.7 1
        :start media input:
            media = 170C521ICRU AIR521ICRU
            set medium = 0 1
            set medium = 1 0
        :stop media input:
    :stop geometry:

    ################################# the set of cylinders needed for the middle
    #                                 this geometry is inscribed directly into
    #                                 the set of planes and therefore we
    #                                 define its media
    #
    :start geometry:
        library  = egs_cylinders
        type     = EGS_ZCylinders
        name     = middle_chamber_part
        radii    = 0.3  0.7  1
        :start media input:
            media = 170C521ICRU AIR521ICRU AL521ICRU
            set medium = 0 2
            set medium = 1 1
            set medium = 2 0
        :stop media input:
    :stop geometry:

    ################################# the final chamber geometry
    #
    :start geometry:
        library  = egs_cdgeometry
        name     = chamber
        base geometry = base_planes
        set geometry  = 0 top_chamber_part
        set geometry  = 1 middle_chamber_part
        set geometry  = 2 bottom_chamber_part
    :stop geometry:

    ############################## end of chamber definition ################

    ############################### The box filled with water
    :start geometry:
        library  = egs_box
        name     = water_box
        box size = 10         # i.e. a 10x10x10 box
        :start media input:
            media = H2O521ICRU
        :stop media input:
    :stop geometry:

    ############################## now make replicas of the chamber
    #
    ############################## a replica translated along the x axis
    :start geometry:
        library  = egs_gtransformed
        my geometry = chamber
        name     = left_chamber
        :start transformation:
            translation = -3 0 0
        :stop transformation:
    :stop geometry:
    ############################## a rotated and translated replica
    :start geometry:
        library  = egs_gtransformed
        my geometry = chamber
        name     = other_chamber
        :start transformation:
            rotation vector = 1 1 1
            translation = 1 2 1
        :stop transformation:
    :stop geometry:

    ############################# The simulation geometry, which is an
    #                             envelope geometry with the box as the
    #                             envelope and the 3 copies of the chamber
    #                             as the inscribed geometries.
    :start geometry:
        library  = egs_genvelope
        name = box_with_chambers
        base geometry = water_box
        inscribed geometries = chamber left_chamber other_chamber
    :stop geometry:

    simulation geometry = box_with_chambers

:stop geometry definition:
