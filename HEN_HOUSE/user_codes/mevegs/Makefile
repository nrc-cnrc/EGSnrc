###############################################################################
#
#  Makefile to build the mevegs application, based on the tutor7pp makefile
#
#  Copyright (C) 2019 Mevex Corporation
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

include $(EGS_CONFIG)
include $(SPEC_DIR)egspp1.spec
include $(SPEC_DIR)egspp_$(my_machine).conf

# Specify the name of the user code.
# The name of the executable is determined from this variable.
#
USER_CODE = mevegs

# The following can be used to add user macros and mortran subroutines.
# The file(s) specified here are added after egsnrc.macros, machine.macros
# and egs_c_interface2.macros but before any files that have
# executable code.
#
#
EGSPP_USER_MACROS = mevegs.macros

# Specify from which base class this application is being derived.
# This has the effect of automatically compiling the base application
# class and including it into the list of object files.
#
EGS_BASE_APPLICATION = egs_advanced_application

# Specify the set of mortran sources used.
# Here we simply use the standard set defined in $HEN_HOUSE/specs/egspp1.spec
#
CPP_SOURCES = $(C_ADVANCED_SOURCES)

include $(HEN_HOUSE)makefiles$(DSEP)cpp_makefile

# add our mesh directory to the include list 
# -- dep_user_code, other_dep_user_code not including this file? 
INCL += -I$(ABS_EGSPP)geometry$(DSEP)egs_mesh -I$(HEN_HOUSE)user_codes$(DSEP)$(USER_CODE)$(DSEP)include



# dep_user_code += -I$(ABS_EGSPP)$(DSEP)geometry$(DSEP)egs_mesh

# Specify here other header files that your user code depends upon.

# add optimization defaults
# opt += -march=native -O3 -ffast-math -Wall -pipe

# add our required libraries
fortran_libs += -lgmsh -legs_mesh -legs_planes -liaea_phsp

# windows only -lws2_32 -lshlwapi

#test_sources:
#	@echo "$(C_SOURCES)"
#
#test_objects:
#	@echo "egs_objects: $(egs_objects)"
#	@echo "user_objects: $(user_objects)"
#
#c_sources:
#	@echo "C_SOURCES: $(CPP_SOURCES)"
