
###############################################################################
#
#  EGSnrc egs++ sample ionization chamber geometry
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
#  This input file defines an egs++ geometry for an ionization chamber with
#  rounded ends, graphite walls and a central electrode made of aluminum.
#
#  The chamber is modeled as a CD geometry. The base geometry of the CD
#  geometry is a set of 4 parallel planes forming 3 regions. The bottom
#  rounded end of the chamber is a set of spheres put into region 2 of the
#  planes. The top rounded end of the chamber is a 2D geometry made of set
#  of spheres and a set of cones put into region 0 of the planes. The middle
#  portion of the chamber is made by inscribing a set of cylinders into
#  region 1 of the planes.
#
###############################################################################


:start geometry definition:

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

    simulation geometry = chamber

:stop geometry definition:
