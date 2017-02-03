
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
#  An example geometry of EGS_iplanes.
#
#
###############################################################################


:start geometry definition:

    # The cylinder
    :start geometry:
        library = egs_cylinders
        type    = EGS_ZCylinders
        name    = rho_coordinates
        radii   = 2
    :stop geometry:

    # The planes
    :start geometry:
        library   = egs_planes
        type      = EGS_Zplanes
        positions = -2 2
        name      = z_coordinates
     :stop geometry:

    # The I-planes
     :start geometry:
        library   = egs_iplanes
        axis      = 0 0 0   0 0 1
        angles    = 0 30 60 90 120 150
           # above angles are in degrees, you can use
           # 'angles in radian' to define angles in radians
        name      = phi_coordinates
     :stop geometry:

     # The final geometry is a N-dimensional geometry
     # made from the cylinder, the planes and the I-planes
     :start geometry:
        library   = egs_ndgeometry
        name      = rho_z_phi
        dimensions = rho_coordinates z_coordinates phi_coordinates
        :start media input:
            media = carbon air water
            set medium =  0  0
            set medium =  1  1
            set medium =  2  2
            set medium =  3  0
            set medium =  4  1
            set medium =  5  2
            set medium =  6  0
            set medium =  7  1
            set medium =  8  2
            set medium =  9  0
            set medium = 10 1
            set medium = 11 2
         :stop media input:
     :stop geometry:

    simulation geometry = rho_z_phi

:stop geometry definition:
