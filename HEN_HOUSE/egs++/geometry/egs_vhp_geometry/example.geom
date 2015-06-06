
###############################################################################
#
#  EGSnrc egs++ voxelized human phantom geometry example
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
#  Author:          Iwan Kawrakow, 2008
#
#  Contributors:
#
###############################################################################
#
#  An example input file defining a VHP geometry.
#
#  Note that the organ data file fax06_med1.bin and the micro-matrix data
#  files are not distributed with EGSnrc but may be obtained from R. Kramer.
#
###############################################################################


:start geometry definition:

    :start geometry:

        library = egs_vhp_geometry
        name    = fax06

        #
        # The binary organ data file
        #
        phantom data = fax06_med1.bin

        #
        # The ASCII media data file mapping organ indeces to media indeces
        # and defining organ and media names.
        #
        media data =  organ.data

        #
        # You can select a given slice range by uncomenting the following
        #
        # slice range = 0 180

        #
        # The bone surface cells layer thickness in cm
        #
        BSC thickness = 0.001
        #
        # The trabecular bone and bone marrow media names
        #
        TB medium = TRABECULAR BONE
        BM medium = BONE MARROW

        #
        # Use micro matrix data from micro10x.bin for organs
        # 127 (Ribs Spongiosa), 122 (Sternum Spongiosa),
        # 142 (Right clavicle Spongiosa), etc.
        #
        micro matrix = micro10x.bin 127 122 142 140 146 144
        #
        # similar as the above
        #
        micro matrix = micro12x.bin 147 148 126
        micro matrix = micro15x.bin 132 130 133 131 136 134 137 135
        micro matrix = micro20x.bin 128
        micro matrix = micro55x.bin 129 138

    :stop geometry:

    simulation geometry = fax06

:stop geometry definition:
