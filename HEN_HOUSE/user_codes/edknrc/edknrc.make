
###############################################################################
#
#  EGSnrc edknrc application make definitions
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
#  Contributors:    Iwan Kawrakow
#
###############################################################################


# We use parallel processing (if there is a working C compiler)
# => set the extra objects and the extra libraries.

EGS_EXTRA_OBJECTS = $(CUTIL_OBJECTS)
EGS_EXTRA_LIBS = $(FLIBS)

# We prefer using the ranmar generator => we overwrite the
# default from $HEN_HOUSE/specs/all_common.spec
#
RANDOM = $(EGS_SOURCEDIR)ranmar

# We use #ifdef ... #endif for parallel processing implementation
# => Fortran file extension must me .F so that the C-preprocessor is
# automatically invoked.
#
FEXT = F

# The mortran sources
# Note: order is important!
# Note: don't forget the leading tabs on continuation lines!
#
SOURCES = $(EGS_SOURCEDIR)egsnrc.macros \
	   $(EGS_UTILS)timing.macros\
 	   $(MACHINE_MACROS) \
	   $(RANDOM).macros\
	   $(RANDOM).correlations\
	   $(EGS_SOURCEDIR)transportp.macros\
	   $(USER_CODE).mortran \
	   $(RANDOM).mortran \
	   $(EGS_UTILS)ensrc.mortran\
	   $(EGS_UTILS)geomsph.mortran\
	   $(EGS_SOURCEDIR)get_inputs.mortran\
	   $(EGS_UTILS)nrcaux.mortran \
	   $(MACHINE_MORTRAN) \
	   $(EGS_SOURCEDIR)egs_utilities.mortran \
	   $(EGS_SOURCEDIR)egs_parallel.mortran \
	   $(EGS_UTILS)xvgrplot.mortran\
	   $(EGS_SOURCEDIR)egsnrc.mortran
