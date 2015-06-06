
###############################################################################
#
#  EGSnrc egs++ sample repeated geometry
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
#  A geometry example for egspp: a set of spheres repeated on a regular XYZ
#  grid modeled using an EGS_XYZRepeater geometry.
#
###############################################################################


:start geometry definition:

    #
    # The geometry to be repeated: a set of spheres
    #
    :start geometry:
        library = egs_spheres
        name    = a_sphere
        radii   = 0.5 0.9999
        :start media input:
            media = med1 med2
            set medium = 1 1
        :stop media input:
    :stop geometry:

    #
    # And now repeat 10x10x10 times
    #
    :start geometry:
        library = egs_ndgeometry
        type    = EGS_XYZRepeater
        name    = repeated_g
        repeated geometry = a_sphere
        repeat x = -10 10 10
        repeat y = -10 10 10
        repeat z = -10 10 10
        medium = air
    :stop geometry:

    simulation geometry = repeated_g

:stop geometry definition:
