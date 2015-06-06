
###############################################################################
#
#  EGSnrc egs++ sample nd geometry
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
#  This input file defines an egs++ geometry for a RZ geometry modeled
#  with a N-dimensional geometry. The input file reproduces the geometry
#  defined in the cavrznrc_template.egsinp file in the EGSnrc distribution.
#
###############################################################################


:start geometry definition:

    ################################### The planes
    :start geometry:
        library   = egs_planes
        type      = EGS_Zplanes
        name      = the_planes
        positions = 0  0.3  0.5  0.8
    :stop geometry:

    ################################### The cylinders
    :start geometry:
        library   = egs_cylinders
        type      = EGS_ZCylinders
        name      = the_cylinders
        radii     = 1 1.3
    :stop geometry:

    ################################### The simulation geometry built from the
    #                                   planes and cylinders as a 2D geometry
    :start geometry:
        library   = egs_ndgeometry
        name      = rz
        dimensions= the_planes the_cylinders
        :start media input:
            media = 170C521ICRU AIR521ICRU
            set medium = 1 1
        :stop media input:
    :stop geometry:

    simulation geometry = rz

:stop geometry definition:

