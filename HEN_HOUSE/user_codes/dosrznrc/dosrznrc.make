
###############################################################################
#
#  EGSnrc dosrznrc application make definitions
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
#  Contributors:    Blake Walters
#
###############################################################################


# We use parallel processing and a BEAM DSO in source 23
# (if there is a working C compiler) => set the extra
# objects and the extra libraries.
#
EGS_EXTRA_OBJECTS = $(CUTIL_OBJECTS) $(BEAMLIB_OBJECTS) $(BEAM_OBJECTS)
EGS_EXTRA_LIBS = $(FLIBS) $(BEAMLIB_EXTRA_LIBS)

# We prefer using the ranmar generator => we overwrites the
# default from $HEN_HOUSE/specs/all_common.spec
#
RANDOM = $(EGS_SOURCEDIR)ranmar

# We need the C-preprocessor
#
FEXT = F

# The mortran sources
# Note: order is important!
# Note: don't forget the leading tabs on continuation lines!
#
SOURCES = \
	$(EGS_SOURCEDIR)egsnrc.macros \
	$(EGS_UTILS)timing.macros\
	$(MACHINE_MACROS) \
	$(RANDOM).macros\
	$(EGS_SOURCEDIR)transportp.macros\
        $(EGS_SOURCEDIR)pegs4_macros.mortran\
	$(EGS_UTILS)phsp_macros.mortran $(IAEA_PHSP_MACROS)\
	$(EGS_UTILS)srcrz.macros\
	$(USER_CODE).mortran \
	$(RANDOM).mortran \
	$(EGS_UTILS)srcrz.mortran\
	$(EGS_UTILS)ensrc.mortran\
	$(EGS_UTILS)geomrz.mortran\
	$(EGS_SOURCEDIR)get_inputs.mortran\
        $(EGS_SOURCEDIR)get_media_inputs.mortran\
	$(EGS_UTILS)grids.mortran\
	$(EGS_UTILS)nrcaux.mortran \
	$(EGS_UTILS)xvgrplot.mortran \
	$(MACHINE_MORTRAN) \
	$(EGS_SOURCEDIR)egs_utilities.mortran \
	$(EGS_SOURCEDIR)egs_parallel.mortran \
        $(EGS_SOURCEDIR)pegs4_routines.mortran\
	$(EGS_SOURCEDIR)egsnrc.mortran
