
###############################################################################
#
#  EGSnrc egs++ sample nd geometry, using an alternative model
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

#  An example geometry input file for the egs++ geometry package.
#
#  This input file defines an egs++ geometry for the same geometry as in
#  rz.geom, but using an alterantive model. It would be interesting to check
#  which model will result in a faster simulation.
#
###############################################################################


:start geometry definition:

    ################################### The outer cylinder
    :start geometry:
        library   = egs_cones
        type      = EGS_ConeStack
        name      = outer_cylinder
        axis      = 0 0 0   0 0 1
        :start layer:
            thickness    = 0.8
            top radii    = 1.3
            bottom radii = 1.3
            media        = 170C521ICRU
        :stop layer:
    :stop geometry:

    ################################### The inner cylinder
    :start geometry:
        library   = egs_cones
        type      = EGS_ConeStack
        name      = inner_cylinder
        axis      = 0 0 0.3   0 0 1
        :start layer:
            thickness    = 0.2
            top radii    = 1
            bottom radii = 1
            media        = AIR521ICRU
        :stop layer:
    :stop geometry:

    ################################### The simulation geometry
    :start geometry:
        library   = egs_genvelope
        name      = rz1
        base geometry = outer_cylinder
        inscribed geometries = inner_cylinder
    :stop geometry:

    simulation geometry = rz1

:stop geometry definition:

