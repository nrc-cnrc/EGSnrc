
###############################################################################
#
#  EGSnrc cavsphnrc application unit numbers
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
#  Author:          Ernesto Mainegra-Hing, 2003
#
#  Contributors:
#
###############################################################################
#
#  This file determines which files are to be connected to which Fortran I/O
#  unit. Lines starting with # are ignored. The first column is the Fortran
#  I/O unit number, the second a file extension. This .io file is for the NRC
#  user code cavsphnrc.
#
###############################################################################
#
#
#
1  .egslst
#  Unit 2 is explicitly opened if needed, depending on istore and/or irestart
#2  .egsrns
#  Unit 4 is explicitly opened if needed, depending on idat and/or irestart
#4  .egsdat
#9  .egseff
#  Unit 13 is explicitly opened if needed
#13 .egsgph
15 .errors
# Unit 23 is explicitly opened if needed
#23 .plotdat
# It appears that the following 2 are unused
#43 .egssrc
#44 .egssrctmp
