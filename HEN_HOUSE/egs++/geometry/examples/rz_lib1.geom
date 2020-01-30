
###############################################################################
#
#  EGSnrc egs++ sample nd geometry
#  Copyright (C) 2016 Randle E. P. Taylor, Rowan M. Thomson,
#  Marc J. P. Chamberland, D. W. O. Rogers
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
#  Author:          Randle Taylor, 2016
#
#  Contributors:    Marc Chamberland
#                   Rowan Thomson
#                   Dave Rogers
#
###############################################################################
#
#  An example geometry input file for the egs++ geometry package.
#
#  This input file defines the same geometry as rz.geom but using the egs_rz
#  library rather than directly using an nd geometry.
#
###############################################################################


:start geometry definition:

    :start geometry:
        library   = egs_rz
        name      = rz
        radii     = 1 1.3
        z-planes  = 0  0.3  0.5  0.8

        :start media input:
            media = 170C521ICRU AIR521ICRU
            set medium = 1 1
        :stop media input:

    :stop geometry:

    simulation geometry = rz

:stop geometry definition:
