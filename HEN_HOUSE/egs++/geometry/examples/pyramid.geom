
###############################################################################
#
#  EGSnrc egs++ sample pyramid geometry
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
#  This input file defines a cut pyramid as a geometry for egs++.
#
#  It uses a 2D geometry to make the cut a pyramid out of a pyramid and a set
#  of planes.
#
###############################################################################


:start geometry definition:

    ################################# the planes
    :start geometry:
        library = egs_planes
        type    = EGS_Zplanes
        positions = -4 4
        name    = planes
    :stop geometry:

    ################################ the pyramid
    :start geometry:
        library = egs_pyramid
        type    = EGS_PyramidZ
        name    = pyramid
        points  = -2,-2.05  -1,-1.05, -2,2.05   2,2.05  2,0.05  \
                  1,0  1,-2.05 -2,-2.05
        #points  = -2,-2.05  -2,2.05   2,2.05  2,-2.05
        tip     = 0 0 8
    :stop geometry:

    ################################ the final geometry
    :start geometry:
        library = egs_ndgeometry
        name    = cut_pyramid
        hownear method = 1
        dimensions = planes pyramid
        :start media input:
            media = AL521ICRU
        :stop media input:
    :stop geometry:

    simulation geometry = cut_pyramid

:stop geometry definition:
