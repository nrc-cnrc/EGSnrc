
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
#  This input file defines the same geometry as rz_lib1.geom but uses slabs
#  and shells rather than using radii and z-planes inputs.
#
###############################################################################


:start geometry definition:

    :start geometry:

        library   = egs_rz
        name      = rz

        number of shells = 1 1
        shell thickness = 1 0.3

        first plane = 0
        number of slabs = 1 1 1
        slab thickness = 0.3 0.2 0.3

        :start media input:
            media = 170C521ICRU AIR521ICRU
            set medium = 1 1
        :stop media input:

    :stop geometry:

    simulation geometry = rz

:stop geometry definition:
