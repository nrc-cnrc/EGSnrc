
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
#  An example geometry of egs_cdgeometry.
#
#
###############################################################################


:start geometry definition:

    # The base geometry, this will be the Chopping Device (CD)
    # The base geometry can be any geometry, even a composite one
    :start geometry:
        name        = my_cd_planes
        library     = egs_planes
        type        = EGS_Zplanes
        positions   = -3 3 5
        # No media required
    :stop geometry:

    :start geometry:
        name        = my_cd_cylinder
        library     = egs_cylinders
        type        = EGS_ZCylinders
        radii       = 1.6 2
        :start media input:
            media = air water
            set medium = 1 1
        :stop media input:
    :stop geometry:

    :start geometry:
        name        = my_cd_sphere
        library     = egs_spheres
        midpoint = 0 0 3
        radii = 1.6 2
        :start media input:
            media = air water
            set medium = 1 1
        :stop media input:
    :stop geometry:

    # The composite geometry
    :start geometry:
        name            = my_cd
        library         = egs_cdgeometry
        base geometry   = my_cd_planes
        # set geometry = 1 geom means:
        # "in region 1 of the basegeometry, use geometry "geom"
        set geometry   = 0 my_cd_cylinder
        set geometry   = 1 my_cd_sphere
        # The final region numbers are attributed by the cd geometry object;
        # Use the viewer to determine region numbers
    :stop geometry:

    simulation geometry = my_cd

:stop geometry definition:
