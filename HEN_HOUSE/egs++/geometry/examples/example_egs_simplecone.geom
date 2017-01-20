
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
#  An example geometry of EGS_SimpleCone.
#
#
###############################################################################


:start geometry definition:
    :start geometry:
        name        = my_simple_cone
        library     = egs_cones
        type        = EGS_SimpleCone
        apex        = 0 0 3
        axis        = 0 0 -1
        height      = 4
        opening angle = 30 # deg
        :start media input:
            media = water
        :stop media input:
    :stop geometry:

    simulation geometry = my_simple_cone

:stop geometry definition:
