
###############################################################################
#
#  EGSnrc egs++ sample glib geometry
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
#  This input demonstrates loading an external geometry using the glib
#  library and inscribing it in an envelope geometry
#
###############################################################################


:start geometry definition:

    :start geometry:

        library      = egs_glib
        name         = seed
        include file = I6702.inp

    :stop geometry:

    :start geometry:

        library   = egs_box
        box size = 2
        name = xyz

        :start media input:
            media = H2O700ICRU
        :stop media input:

    :stop geometry:

    :start geometry:

        library              = egs_genvelope
        base geometry        = xyz
        inscribed geometries = seed
        name = phantom_w_seed

    :stop geometry:

    simulation geometry = phantom_w_seed

:stop geometry definition:
