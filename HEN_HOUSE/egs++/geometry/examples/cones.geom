
###############################################################################
#
#  EGSnrc egs++ sample cone set geometry
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
#   An example geometry input file for the egs++ geometry package.
#
#   This input file demonstrates the use of a cone set. Play around with the
#   flag key in the definition of the cones to see how the geometry changes
#   for flag = 0, 1 and 2.
#
##############################################################################


:start geometry definition:

    ###################################### Define a set of cones
    #                       with the same apex and axis but different
    #                       opening angles.
    #                       A cone set extends to infinity and therefore
    #                       we will use it as part of 2D geometry to limit
    #                       its extend.
    :start geometry:
        library = egs_cones
        type = EGS_ConeSet
        name = cone_set
        apex = 0 0 0
        axis = 0 0 1
        opening angles = 10 20 30 40 50 60
        flag = 1         # you can also set this to 0 or 2
    :stop geometry:

    #################################### This sphere will be used to limit
    #                     the size of the conical regions by a rounded end
    #                     as a part of a 2D geometry.
    :start geometry:
        library = egs_spheres
        name    = sphere
        midpoint = 0 0 0
        radii = 5
    :stop geometry:

    #################################### Now the actual geometry made from the
    #                    set of cones and the above sphere.
    :start geometry:
        library = egs_ndgeometry
        name = cones
        dimensions = sphere cone_set
        hownear method = 1
        :start media input:
            media = med1 med2 med3 med4 med5 med6
            set medium = 0 0
            set medium = 1 1
            set medium = 2 2
            set medium = 3 3
            set medium = 4 4
            set medium = 5 5
        :stop media input:
    :stop geometry:

    simulation geometry = cones

:stop geometry definition:

