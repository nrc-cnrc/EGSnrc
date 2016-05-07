
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
#  Author:          Manuel Stoeckl, 2016
#
#  Contributors:
#
###############################################################################
#
#  An example geometry input file for the egs++ geometry package.
#
#  This input file defines an egs++ geometry for a rounded cube with a cut in it.
#
###############################################################################


:start geometry definition:

    ################################### The cut
    
    :start geometry:
        library   = egs_roundrect_cylinders
        type      = EGS_RoundRectCylinders
        name      = cut_path
        x-widths  = 0.05 50
        y-widths  = 0.4 50
        radii     = 0.05 0
        midpoint  = 0 0.3 0
        x-axis    = 0.37 0.55 0.74
        y-axis    = 0.74 0.55 0.37
    :stop geometry:
    
    ################################### The body. Slightly assymetric to avoid
                                      # overlapping region transitions in the
                                      # ndgeometry
    :start geometry:
        library   = egs_roundrect_cylinders
        type      = EGS_RoundRectCylindersXY
        name      = xycyl
        x-widths  = 0.5 1.0 1.3001 1.5001
        y-widths  = 0.5 1.0 1.3001 1.5001
        radii     = 0.5 1.0 0.0   0.2
        midpoint  = 0 0 0
    :stop geometry:
    
    :start geometry:
        library   = egs_roundrect_cylinders
        type      = EGS_RoundRectCylindersYZ
        name      = yzcyl
        x-widths  = 0.5 1.0 1.3 1.5
        y-widths  = 0.5 1.0 1.3 1.5
        radii     = 0.5 1.0 0.0 0.2
        midpoint  = 0 0 0
    :stop geometry:
    
    :start geometry:
        library   = egs_roundrect_cylinders
        type      = EGS_RoundRectCylindersXZ
        name      = zxcyl
        x-widths  = 0.5 1.0 1.2999 1.4999
        y-widths  = 0.5 1.0 1.2999 1.4999
        radii     = 0.5 1.0 0.0    0.2
        midpoint  = 0 0 0
    :stop geometry:

    ################################### Unified simulation geometry
    :start geometry:
        library   = egs_ndgeometry
        name      = cube
        dimensions= xycyl yzcyl zxcyl cut_path
        :start media input:
            media = vacuum red green blue cyan pink
            set medium = 0 3 0 3 0 3 1 1 4
            set medium = 0 2 0 2 0 2 1 1 0
            set medium = 0 0 0 0 0 0 1 1 2
            set medium = 1 1 1 1 1 1 1 1 1
            set medium = 2 2 2 2 2 2 1 1 3
            set medium = 2 3 1 1 2 3 1 1 5
            set medium = 2 3 2 3 1 1 1 1 5
            set medium = 1 1 2 3 2 3 1 1 5
            set medium = 3 3 1 1 3 3 1 1 0
            set medium = 1 1 3 3 3 3 1 1 0
            set medium = 3 3 3 3 1 1 1 1 0
        :stop media input:
    :stop geometry:

    simulation geometry = cube

:stop geometry definition:
