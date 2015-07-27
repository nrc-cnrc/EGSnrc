
###############################################################################
#
#  EGSnrc configuration for unix systems
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
#  This file is used by 'make' on unix systems.
#
###############################################################################


# The suffix of an executable
#
EXE =

SHELL = /bin/sh

# Preprocessor definies for C compilation
#
C_DEFINES =

# Sometimes I want to just leave a line blank in the output.
# When I just say echo, stupid Windows prints ECHO is off.
# => I'm using echo $(empty) and defining $(empty) to be realy empty on Unix
#    but to be a . on Windows
#
empty =

# The prefix and file extension of a shared library (DLL)
#
LIB_PRE = lib
LIB_SUF = .so

# Check if a file exists and if not, copy it from the second argument.
# If the file exists, check if it is more recent than the other
#
GET_FILE = if test -f $@; then \
		echo $(MAIN_UCODE_DIR)$@ is more recent than local version; \
	    else \
		cp $(MAIN_UCODE_DIR)$@ $@; \
	       echo Copied $(MAIN_UCODE_DIR)$@ to local area; \
	    fi

CHECK_USER_CODE = \
	if test -f $(USER_CODE).mortran; then \
	  if test -f $(MAIN_UCODE_DIR)$(USER_CODE).mortran; then \
	    $(MAKE) -s $(USER_CODE).mortran \
	      depend=$(MAIN_UCODE_DIR)$(USER_CODE).mortran \
	      check_message="$(the_message)"; \
	  else \
	    echo "$(USER_CODE).mortran does not exist on HEN_HOUSE"; \
	  fi \
	else \
	  if test -f $(MAIN_UCODE_DIR)$(USER_CODE).mortran; then \
	    cp $(MAIN_UCODE_DIR)$(USER_CODE).mortran $(USER_CODE).mortran; \
	    echo "Copied $(USER_CODE).mortran from HEN_HOUSE area"; \
	  fi \
	fi


# Some system dependent commands
REMOVE = rm -f

CHECK_DIR = test -d $@ ||
MKDIR = ( echo Creating directory $@ && mkdir $@ )

# Creating static libraries
AR = ar cvr
RANLIB = ranlib
lib_prefix = lib
lib_suffix = .a
CREATE_LIB = $(AR) $@ $^ && $(RANLIB) $@
CREATE_LIB1 = $(AR) $(the_library) $(the_objects) && $(RANLIB) $(the_library)

# The make utility used to build the GUI's
#
gui_make = $(make_prog)
