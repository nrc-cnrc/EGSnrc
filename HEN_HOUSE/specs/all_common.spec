
###############################################################################
#
#  EGSnrc common configuration
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
#  This file is included by EGSnrc makefiles via the configuration file. It
#  contains definitions of various directories, the default random number
#  generator, the default set of sources, etc. These definitions are the same
#  on all plarforms.
#
###############################################################################


# The standard Makefile
#
EGS_MAKEFILE = $(HEN_HOUSE)makefiles$(DSEP)standard_makefile

# The standard Makefile for user codes written in C.
#
EGS_C_MAKEFILE = $(HEN_HOUSE)makefiles$(DSEP)c_makefile

# Relative directories for executables and libraries
#
BIN_SUBDIR = bin$(DSEP)$(my_machine)$(DSEP)
LIB_SUBDIR = lib$(DSEP)$(my_machine)$(DSEP)

# Main installation executables and libraries
#
EGS_BINDIR = $(HEN_HOUSE)$(BIN_SUBDIR)
EGS_LIBDIR = $(HEN_HOUSE)$(LIB_SUBDIR)

# User executables and libraries
#
USER_BINDIR = $(EGS_HOME)$(BIN_SUBDIR)
USER_LIBDIR = $(EGS_HOME)$(LIB_SUBDIR)

# EGSnrc sources
#
EGS_SOURCEDIR = $(HEN_HOUSE)src$(DSEP)

# Utilities coming with EGSnrc
#
EGS_UTILS = $(HEN_HOUSE)utils$(DSEP)

# The default random number generator
# You can overwrite either on the command line or in your
# Makefile
#
RANDOM = $(EGS_SOURCEDIR)ranlux

# User code directory
#
EGS_USERCODES = $(HEN_HOUSE)user_codes$(DSEP)

# The main area user code directory
#
MAIN_UCODE_DIR = $(EGS_USERCODES)$(USER_CODE)$(DSEP)

# The fortran file (without extension) to be created by the Mortran compiler
#
FORTRAN_FILE = $(USER_CODE)_$(my_machine)

# The executable
#
EXECUTABLE = $(USER_BINDIR)$(USER_CODE)$(WHAT)$(EXE)

# What about OMEGA stuff? I'm puting it in here for now, so
# that I can play around with the local makefiles.
#
OMEGA = $(HEN_HOUSE)omega$(DSEP)

# Mortran stuff
#
MACHINE_MORTRAN = $(EGS_LIBDIR)machine.mortran
MACHINE_MACROS = $(EGS_LIBDIR)machine.macros
MORTRAN_EXE = $(EGS_BINDIR)mortran3.exe
MORTRAN_DATA = $(EGS_BINDIR)mortran3.dat

# Extra objects to be linked to build a user code.
#
EGS_EXTRA_OBJECTS =

# Extra libraries to be used in the link step
#
EGS_EXTRA_FLIBS = $(FLIBS)

# The standard set of files
#
SOURCES = $(EGS_SOURCEDIR)egsnrc.macros \
	   $(MACHINE_MACROS) $(RANDOM).macros \
	   $(USER_CODE).mortran \
	   $(RANDOM).mortran $(EGS_UTILS)nrcaux.mortran $(MACHINE_MORTRAN) \
	   $(EGS_SOURCEDIR)egs_utilities.mortran $(EGS_SOURCEDIR)egsnrc.mortran

# PEGS4 stuff
#
PEGS4_EXE = $(EGS_BINDIR)pegs4.exe

# Replacement for various scripts
# Do we need this ?
#
EGS_COMPILE = $(EGS_BINDIR)egs_compile$(EXE)

# The fortran file extension
#
FEXT = f

#******************************************************************************
# The following set of definitions is for interfacing EGSnrc to C/C++
#******************************************************************************
#
# C/C++ interafece file directory
#
C_INTERFACE = $(HEN_HOUSE)interface$(DSEP)

# Include directories for the C/C++ compiler
#
C_INCLUDES = -I$(HEN_HOUSE)lib$(DSEP)$(my_machine) -I$(HEN_HOUSE)interface -I.

# The set of mortran files needed for the C/C++ interface
#
C_SOURCES =   $(EGS_SOURCEDIR)egsnrc.macros \
		$(MACHINE_MACROS) $(C_INTERFACE)egs_c_interface1.macros \
		$(RANDOM).macros\
		$(USER_CODE).macros \
		$(RANDOM).mortran \
		$(MACHINE_MORTRAN) \
		$(EGS_SOURCEDIR)egs_utilities.mortran \
		$(EGS_SOURCEDIR)egsnrc.mortran

