
###############################################################################
#
#  EGSnrc examin application make definitions
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


# We don't use parallel preprocessing => don't link egs_c_utils.o
#
EGS_EXTRA_OBJECTS =

# The mortran sources
# Note: order is important!
# Note: don't forget the leading tabs on continuation lines!
#
SOURCES = \
	$(EGS_SOURCEDIR)egsnrc.macros \
	$(EGS_UTILS)timing.macros\
	$(MACHINE_MACROS) \
	$(RANDOM).macros\
	$(USER_CODE).mortran \
	$(RANDOM).mortran \
	$(EGS_UTILS)xvgrplot.mortran \
	$(MACHINE_MORTRAN) \
	$(EGS_SOURCEDIR)egs_utilities.mortran \
	$(EGS_SOURCEDIR)egsnrc.mortran
