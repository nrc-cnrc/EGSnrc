
###############################################################################
#
#  EGSnrc egs++ sample nd_geometry egsphant geometry
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
#  This input demonstrates loading an egsphant geometry using the nd_geometry
#  library. Requires file ndgeom_egsphant.egsphant
#  to be in the same directory.
#
###############################################################################

:start geometry definition:

    :start geometry:
        library = egs_ndgeometry
        type = EGS_XYZGeometry
        name = my_egsphant_geom
        egsphant file = ndgeom_egsphant.egsphant
    :stop geometry:

    simulation geometry = my_egsphant_geom

:stop geometry definition:
