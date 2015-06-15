
###############################################################################
#
#  EGSnrc tutor4 application unit numbers
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
#  Author:          Iwan Kawrakow, 2003
#
#  Contributors:    Dave Rogers
#
###############################################################################
#
#  This file determines which files are to be connected to which Fortran I/O
#  unit. Lines starting with # are ignored. The first column is the Fortran
#  I/O unit number, the second a file extension. This .io file is for the
#  NRC user code tutor4.
#
#  The only additional file is unit 13 for possible output for the EGS_WINDOWS
#  graphics package. Unit 13 is connected to test.egsgph by default. to
#  generate file.egsgph run the code using:
#
#  tutor4 -o file -p tutor_data
#
###############################################################################
#
#
#
# we explicitely open the file in WATCH if needed => comment out the following
#13 .egsgph
