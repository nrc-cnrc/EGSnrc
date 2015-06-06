
###############################################################################
#
#  EGSnrc egs++ sample hemisphere geometry
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
#  This input file defines a hemisphere as a geometry for egs++. It uses a 2D
#  geometry to make the hemisphere out of a set of spheres and planes.
#
###############################################################################


:start geometry definition:

    #################################### the spheres
    #
    #      Note: we don't define any media here because the geometry
    #      will get filled with media in the ND-geometry definition
    #
    :start geometry:
        library = egs_spheres
        name    = sphere
        midpoint = 0 0 0
        radii = 1
    :stop geometry:

    #################################### the planes
    #
    #      Note: we don't define any media here because the geometry
    #      will get filled with media in the ND-geometry definition
    #
    :start geometry:
        library  = egs_planes
        type     = EGS_Planes
        name     = planes
        normal   = 1 1 1
        positions = 0        # a second plane at infinity will get defined
                             # internally, if we input a single position
    :stop geometry:

    ################################### the hemisphere
    :start geometry:
        library = egs_ndgeometry
        name    = hemisphere
        dimensions = planes sphere
        :start media input:
            media = H2O700ICRU
        :stop media input:
    :stop geometry:

    simulation geometry = hemisphere

:stop geometry definition:
