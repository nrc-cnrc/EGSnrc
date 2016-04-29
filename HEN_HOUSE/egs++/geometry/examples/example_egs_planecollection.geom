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
#  An example geometry of EGS_PlaneCollection.
#
#
###############################################################################


:start geometry definition:
    :start geometry:
        name        = my_plane_collection
        library     = egs_planes
        type        = EGS_PlaneCollection
        normals     = 0,0,1  0.1,-0.1,1  -0.1,-0.3,1  0,0,1
        positions   = -5 -2 2 12
        :start media input:
            media = water air water
            set medium = 1 1
            set medium = 2 2
        :stop media input:
    :stop geometry:

    :start geometry:
        library = egs_spheres
        name    = sphere
        midpoint = 0 0 0
        radii = 5
    :stop geometry:

    # Now the actual geometry made from the
    # planes and the above sphere.
    :start geometry:
        library = egs_ndgeometry
        name = collection
        dimensions = sphere my_plane_collection
        hownear method = 1
        :start media input:
            media = water air
            set medium = 0 0
            set medium = 1 1
            set medium = 2 0
            set medium = 3 1
        :stop media input:
    :stop geometry:

    simulation geometry = collection

:stop geometry definition:
