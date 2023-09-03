
###############################################################################
#
#  EGSnrc configuration for Windows systems
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
#  Contributors:    Ernesto Mainegra-Hing
#                   Reid Townson
#
###############################################################################
#
#  This file is used by 'make' on Windows systems.
#
###############################################################################


# The file extension of executables
#
EXE = .exe

# Preprocessor definies for C compilation
#
C_DEFINES = -DWIN32

# Sometimes I want to just leave a line blank in the output.
# When I just say echo, stupid Windows prints ECHO is off.
# => I'm using echo $(empty) and defining $(empty) to be realy empty on Unix
#    but to be a . on Windows
#
empty = .

# The prefix and file extension of a shared library (DLL)
#
LIB_PRE =
LIB_SUF = .dll

# Check if a file exists and if not, copy it from the second argument.
# If the file exists, check if it is more recent than the other
#
FILE_TEST = if not exist $1 ( copy $2 $1 & \
		echo "Copied $1 from $(dir $2) to local area" ) \
		else ( echo "$1 on $(dir $2) is more recent than local $1" )
# Use the above in a make rule for a file
#
GET_FILE = $(call FILE_TEST,$@,$<)

#
CHECK_USER_CODE = \
	if exist $(USER_CODE).mortran ( \
	  if exist $(MAIN_UCODE_DIR)$(USER_CODE).mortran ( \
	    $(MAKE) -s $(USER_CODE).mortran \
	      depend=$(MAIN_UCODE_DIR)$(USER_CODE).mortran \
	      check_message="$(the_message)" \
	  ) \
	  else ( echo $(USER_CODE).mortran does not exist on HEN_HOUSE ) \
	) \
	else ( \
	  if exist $(MAIN_UCODE_DIR)$(USER_CODE).mortran ( \
	    copy $(MAIN_UCODE_DIR)$(USER_CODE).mortran $(USER_CODE).mortran \
	    echo Copied $(USER_CODE).mortran from HEN_HOUSE area \
	  ) \
	)

# Some system dependent commands
REMOVE = del /Q /F /S

CHECK_DIR = if not exist $@
MKDIR = echo Creating directory $@ && mkdir $@

# Creating static libraries
lib_prefix =
lib_suffix = .lib
ifeq ($(MinGW),yes)
CREATE_LIB = ar cvr $@ $^ && ranlib $@
else
CREATE_LIB = lib $^ -out:$@
endif
ifeq ($(MinGW),yes)
CREATE_LIB1 = ar cvr $(the_library) $(the_objects) && ranlib $(the_library)
else
CREATE_LIB1 = lib $(the_objects) -out:$(the_library)
endif

# The make utility used to build the GUI's
#
gui_make = $(make_prog)
