
###############################################################################
#
#  EGSnrc BEAMnrc default .io file
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
#  Author:          Iwan Kawrakow, 2004
#
#  Contributors:    Blake Walters
#
###############################################################################
#
#  This file determines which files are to be connected to which Fortran I/O
#  unit. Lines starting with # are ignored. The first column is the Fortran
#  I/O unit number, the second a file extension. This .io file is for the
#  NRC user code tutor7.
#
###############################################################################
#
#
# The standard .io file for BEAMnrcMP. The beam_build.exe program will check if
# BEAM_user_code.io exists in the BEAM user code directory and if not itwill
# copy this file over.

