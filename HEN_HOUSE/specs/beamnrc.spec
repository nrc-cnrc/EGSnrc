
###############################################################################
#
#  EGSnrc BEAMnrc configuration
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
#                   Reid Townson
#
###############################################################################


# The central OMEGA and BEAM locations
#
OMEGA_HOME = $(HEN_HOUSE)omega$(DSEP)
BEAM_HOME = $(OMEGA_HOME)beamnrc$(DSEP)
DOSXYZ_HOME = $(HEN_HOUSE)user_codes$(DSEP)dosxyznrc$(DSEP)
CM_HOME = $(BEAM_HOME)CMs$(DSEP)

# You may use the following to add extra object files if you need
#
BEAM_OBJECTS =

# Local (user specific) BEAM locations.
#
USER_BEAM_HOME = $(EGS_HOME)beamnrc$(DSEP)
SPEC_HOME = $(USER_BEAM_HOME)spec_modules$(DSEP)

# I guess, we will eventually want to use the new parallel implementation.
# => the file extension must be .F so that the C-preprocessor gets invoked.
#
FEXT = F

# Extra object files to be linked with a user code
#
#BEAM_EXTRA_OBJECTS = $(EGS_EXTRA_OBJECTS)
BEAM_EXTRA_OBJECTS = $(CUTIL_OBJECTS) $(BEAMLIB_OBJECTS) $(BEAM_OBJECTS) $(BEAMLIB_EXTRA_LIBS)

# The name of the beam_build executable.
#
BEAM_BUILD = $(EGS_BINDIR)beam_build.exe

# The standard BEAMnrc Makefile used by all local Makefiles
#
BEAM_MAKEFILE = $(HEN_HOUSE)makefiles$(DSEP)beam_makefile

# In BEAM we want to use ranmar as the default RNG
#
RANDOM = $(EGS_SOURCEDIR)ranmar

# This is the standard set of sources used to build a
# BEAMnrc user code.
# If you want to change it (e.g. use a local version of beamnrc.mortran, etc.)
# you'll have to overwrite this definition in the
#         sources.make
# file in your BEAM user code directory.
#
SOURCES = $(EGS_SOURCEDIR)egsnrc.macros \
          $(EGS_UTILS)timing.macros \
          $(MACHINE_MACROS) $(RANDOM).macros \
          $(EGS_SOURCEDIR)transportp.macros \
          $(EGS_SOURCEDIR)pegs4_macros.mortran \
          $(BEAM_HOME)beamnrc_user_macros.mortran \
          $(EGS_UTILS)phsp_macros.mortran $(IAEA_PHSP_MACROS)\
          $(BEAM_CODE)_macros.mortran \
          $(BEAM_HOME)beam_main.mortran\
          $(BEAM_HOME)beamnrc.mortran \
          $(EGS_UTILS)xvgrplot.mortran \
          $(BEAM_CODE)_cm.mortran \
          $(EGS_SOURCEDIR)egs_utilities.mortran \
          $(EGS_SOURCEDIR)get_inputs.mortran \
          $(EGS_SOURCEDIR)get_media_inputs.mortran \
          $(RANDOM).mortran \
          $(EGS_UTILS)nrcaux.mortran \
          $(MACHINE_MORTRAN) \
          $(EGS_SOURCEDIR)egs_parallel.mortran \
          $(EGS_SOURCEDIR)pegs4_routines.mortran \
          $(EGS_SOURCEDIR)egsnrc.mortran

LIB_SOURCES = $(BEAM_HOME)beam_lib.macros \
          $(EGS_SOURCEDIR)egsnrc.macros \
          $(EGS_UTILS)timing.macros \
          $(MACHINE_MACROS) $(RANDOM).macros \
          $(EGS_SOURCEDIR)transportp.macros \
          $(EGS_SOURCEDIR)pegs4_macros.mortran \
          $(BEAM_HOME)beamnrc_user_macros.mortran \
          $(EGS_UTILS)phsp_macros.mortran $(IAEA_PHSP_MACROS)\
          $(BEAM_CODE)_macros.mortran \
          $(BEAM_HOME)beam_lib.mortran\
          $(BEAM_HOME)beamnrc.mortran \
          $(EGS_SOURCEDIR)egs_utilities.mortran \
          $(BEAM_CODE)_cm.mortran \
          $(EGS_SOURCEDIR)get_inputs.mortran \
          $(EGS_SOURCEDIR)get_media_inputs.mortran \
          $(RANDOM).mortran \
          $(EGS_UTILS)nrcaux.mortran \
          $(MACHINE_MORTRAN) \
          $(EGS_SOURCEDIR)egs_parallel.mortran \
          $(EGS_SOURCEDIR)pegs4_routines.mortran \
          $(EGS_SOURCEDIR)egsnrc.mortran

# The git hash and branch
GIT_HASH =
ifeq ($(OS),Windows_NT)
    USING_GIT = $(shell cmd /C @ECHO OFF & git rev-parse --is-inside-work-tree 2>NUL)
    ifeq ($(USING_GIT),true)
        GIT_HASH = -DGIT_HASH="\"$(shell cmd /C git rev-parse --short=7 HEAD)\""
    else
        GIT_HASH = -DGIT_HASH="\"unknown\""
    endif
else
    GIT_HASH = -DGIT_HASH="\"$(shell if git rev-parse --is-inside-work-tree > /dev/null 2>&1; then git rev-parse --short=7 HEAD; fi)\""
endif

COMPILE_TIME =
ifeq ($(OS),Windows_NT)
    COMPILE_TIME = -DCOMPILE_TIME="\"$(shell cmd /C date /T)$(shell cmd /C time /T)\""
else
    COMPILE_TIME = -DCOMPILE_TIME="\"$(shell date -u +'%Y-%m-%d %H:%M:%S UTC')\""
endif
