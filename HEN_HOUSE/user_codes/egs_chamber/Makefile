
###############################################################################
#
#  EGSnrc makefile to build egs++ egs_chamber application
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
#  Author:          Iwan Kawrakow, 2007
#
#  Contributors:
#
###############################################################################
#
#  As long as your own application is derived from EGS_AdvancedApplication and
#  there is a single C++ source file named $(USER_CODE).cpp, you can simply
#  use this Makefile by copying it into the application directory and changing
#  the USER_CODE variable below to the name of your application.
#
###############################################################################


include $(EGS_CONFIG)
include $(SPEC_DIR)egspp1.spec
include $(SPEC_DIR)egspp_$(my_machine).conf

# Specify the name of the user code.
# The name of the executable is determined from this variable.
#
USER_CODE = egs_chamber

# The following can be used to add user macros and mortran subroutines.
# The file(s) specified here are added after egsnrc.macros, machine.macros
# and egs_c_interface2.macros but before any files that have
# executable code.
#
EGSPP_USER_MACROS = egs_chamber.macros

# Specify from which base class this application is being derived.
# This has the effect of automatically compiling the base application
# class and including it into the list of object files.
#
EGS_BASE_APPLICATION = egs_advanced_application

# Specify the set of mortran sources used.
# Here we simply use the standard set defined in
# $HEN_HOUSE/specs/egspp1.spec
#
CPP_SOURCES = $(C_ADVANCED_SOURCES)


# Specify here other header files that your user code depends upon.
#
other_dep_user_code = $(ABS_EGSPP)egs_scoring.h

# User code defines
#
DEF_USER = -g

include $(HEN_HOUSE)makefiles$(DSEP)cpp_makefile
