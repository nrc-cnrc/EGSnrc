
###############################################################################
#
#  EGSnrc tutor4 application make definitions
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
#  Contributors:
#
###############################################################################
#
#  The .make file MUST be present in the user code directory, even if it is
#  just an empty file.
#
#  The .make file can be used to
#
#  - change the standard set of files needed to build a user code by including
#    a "SOURCES = ..." statement.
#
#  - modify the compilation options, e.g.,
#    FCFLAS = -fno-automatic -finit-local-zero
#    to make all local variables static with G77.
#
#  - Pass C preprocessor directive on the command line, e.g.,
#    FCFLAGS = $(FCFLAGS) -Dmy_code
#
###############################################################################


# In the case of the tutor 1-7 codes, we use the .make file to remove the
# egs_c_utils.o file from the link step (because no parallel processing is
# being used).
#
EGS_EXTRA_OBJECTS =
