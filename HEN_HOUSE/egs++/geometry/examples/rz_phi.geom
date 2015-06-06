
###############################################################################
#
#  EGSnrc egs++ sample i-planes geometry
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
#   This input file demonstrates the use of i-planes to divide a region into
#   phi segments. A 3D geometry is made from a cylinder, a set of planes and
#   a set of i-planes.
#
###############################################################################


:start geometry definition:

    ######################################## the cylinder
    :start geometry:
        library = egs_cylinders
        type    = EGS_ZCylinders
        name    = rho_coordinates
        radii   = 2
    :stop geometry:

    ######################################## the planes
    :start geometry:
        library   = egs_planes
        type      = EGS_Zplanes
        positions = -2 2
        name      = z_coordinates
     :stop geometry:

    ######################################## the I-planes
     :start geometry:
        library   = egs_iplanes
        axis      = 0 0 0   0 0 1
        angles    = 0 30 60 90 120 150
           # above angles are in degrees, you can use
           # 'angles in radian' to define angles in radians
        name      = phi_coordinates
     :stop geometry:

     ########################### The final geometry is a N-dimensional geometry
     #                made from the cylinder, the planes and the I-planes
     :start geometry:
        library   = egs_ndgeometry
        name      = rho_z_phi
        dimensions = rho_coordinates z_coordinates phi_coordinates
        :start media input:
            media = m1 m2 m3 m4 m5 m6 m7 m8 m9 m10 m11 m12
            set medium =  0  0
            set medium =  1  1
            set medium =  2  2
            set medium =  3  3
            set medium =  4  4
            set medium =  5  5
            set medium =  6  6
            set medium =  7  7
            set medium =  8  8
            set medium =  9  9
            set medium = 10 10
            set medium = 11 11
         :stop media input:
     :stop geometry:

    simulation geometry = rho_z_phi

:stop geometry definition:


