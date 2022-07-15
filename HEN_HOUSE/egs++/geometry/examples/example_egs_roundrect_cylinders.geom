
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
#  Author:          Frederic Tessier, 2016
#
#  Contributors:    Manuel Stoeckl
#                   Reid Townson
#
###############################################################################
#
#  An example geometry input file for the egs++ rounded rectangle geometry.
#
###############################################################################


:start geometry definition:

    ### a rounded rectangle cylinder
    :start geometry:
        name      = rounded_rect_cyl
        library   = egs_roundrect_cylinders
        type      = EGS_RoundRectCylindersXY
        x-widths  = 1 2
        y-widths  = 0.5 1
        radii     = 0.1 0.5
        :start media input:
            media = red green
            set medium = 1 1
        :stop media input:
    :stop geometry:

    ### planes to cut the cylinder
    :start geometry:
        name      = cd_planes
        library   = egs_planes
        type      = EGS_Zplanes
        positions = -2 2
    :stop geometry:

    ### cut cylinder with planes
    :start geometry:
        name      = rounded_rect
        library   = egs_cdgeometry
        base geometry = cd_planes
        set geometry  = 0 rounded_rect_cyl
    :stop geometry:

    ### simulation geometry
    simulation geometry = rounded_rect

:stop geometry definition:
