
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
#  An example geometry of egs_elliptic_cylinders.
#
#
###############################################################################


:start geometry definition:
    :start geometry:
        name        = my_cylinders
        library     = egs_elliptic_cylinders
        type        = EGS_EllipticCylindersXZ
        midpoint    = 0
        x-radii     = 1 2 3
        y-radii     = 1 4 7
        :start media input:
            media = water air water
            set medium = 1 1
            set medium = 2 2
        :stop media input:
    :stop geometry:

    simulation geometry = my_cylinders

:stop geometry definition:
