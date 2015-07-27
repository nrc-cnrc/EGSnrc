
###############################################################################
#
#  EGSnrc egspp configuration for building simple egs++ applications
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
#  Author:          Iwan Kawrakow, 2005
#
#  Contributors:    Blake Walters
#
###############################################################################


# The directory of the egs++ libraries relative to HEN_HOUSE
egspp = egs++
EGSPP = egs++$(DSEP)

# Relative path from the egs++ directory to the directory where object
# files and DSOs (DLLs) get put
dso = dso$(DSEP)$(my_machine)

# The absolute path to the egs++ libraries
abs_dso = $(HEN_HOUSE)$(EGSPP)$(dso)
ABS_DSO = $(abs_dso)$(DSEP)

# The absolute path to the main egs++ library directory
abs_egspp = $(HEN_HOUSE)$(egspp)
ABS_EGSPP = $(abs_egspp)$(DSEP)

# The absolute path to the C/C++ interface directory
egs_interface = $(HEN_HOUSE)interface
EGS_INTERFACE = $(egs_interface)$(DSEP)

# Include directories to look for include files
INCL = -I. -I$(HEN_HOUSE)lib$(DSEP)$(my_machine) -I$(abs_egspp) -I$(egs_interface)

# The standard set of mortran sources needed for the EGSnrc C++ interface
C_SIMPLE_SOURCES = $(EGS_SOURCEDIR)egsnrc.macros $(MACHINE_MACROS) \
            $(EGS_INTERFACE)egs_c_interface2.macros \
            $(EGSPP_USER_MACROS) \
            $(MACHINE_MORTRAN) $(EGS_SOURCEDIR)egs_utilities.mortran \
            $(EGS_SOURCEDIR)transportp.macros \
            $(EGS_SOURCEDIR)get_inputs.mortran \
            $(EGS_SOURCEDIR)egsnrc.mortran

C_ADVANCED_SOURCES = $(EGS_SOURCEDIR)egsnrc.macros $(MACHINE_MACROS) \
            $(EGS_SOURCEDIR)pegs4_macros.mortran \
            $(EGS_INTERFACE)egs_c_interface2.macros \
            $(EGSPP_USER_MACROS) \
            $(MACHINE_MORTRAN) $(EGS_SOURCEDIR)egs_utilities.mortran \
            $(EGS_INTERFACE)egs_c_interface2.mortran \
            $(EGS_SOURCEDIR)transportp.macros \
            $(EGS_SOURCEDIR)get_inputs.mortran\
            $(EGS_SOURCEDIR)get_media_inputs.mortran \
            $(EGS_SOURCEDIR)pegs4_routines.mortran \
            $(EGS_SOURCEDIR)egsnrc.mortran

# The base class from which the application is derived
#EGS_BASE_APPLICATION = egs_simple_application

# The standard set of object files needed
egs_files = $(EGS_BASE_APPLICATION) egsnrc egs_interface2
egs_objects = $(addsuffix _$(my_machine).$(obje),$(egs_files))
egs_lib_files = $(EGS_BASE_APPLICATION)_lib egsnrc egs_interface2
egs_lib_objects = $(addsuffix _$(my_machine).$(obje),$(egs_lib_files))

# The user objects
user_files = $(USER_CODE) $(extra_user_files)
user_objects = $(addsuffix _$(my_machine).$(obje),$(user_files))
user_lib_files = $(USER_CODE)_lib $(extra_user_files)
user_lib_objects = $(addsuffix _$(my_machine).$(obje),$(user_lib_files))

# The target (the executable)
target = $(USER_BINDIR)$(USER_CODE)$(EXE)

# The other target (the application DSO or DLL)
lib_target = $(USER_BINDIR)$(libpre)$(USER_CODE)$(libext)

# A standard rule for compiling C++ files
object_rule = $(CXX) $(INCL) $(DEF1) $(DEF_USER) $(opt) -c $(COUT)$@ $<
object_lib_rule = $(CXX) $(INCL) $(DEF1) $(DEF_USER) -DBUILD_APP_LIB $(opt) -c $(COUT)$@ $<

# Files that a lot of stuff depends upon
common_h_files1 = $(EGS_INTERFACE)egs_interface2.h $(HEN_HOUSE)lib$(DSEP)$(my_machine)$(DSEP)egs_config1.h $(ABS_EGSPP)egs_libconfig.h

common_h_files2 = $(ABS_EGSPP)egs_libconfig.h \
                  $(ABS_EGSPP)egs_base_geometry.h $(ABS_EGSPP)egs_vector.h \
                  $(ABS_EGSPP)egs_math.h $(ABS_EGSPP)egs_functions.h \
                  $(ABS_EGSPP)egs_base_source.h $(ABS_EGSPP)egs_input.h \
                  $(ABS_EGSPP)egs_object_factory.h

# Dependencies
dep_simple_application = $(ABS_EGSPP)egs_simple_application.cpp \
        $(ABS_EGSPP)egs_simple_application.h array_sizes.h \
        $(common_h_files1) $(common_h_files2) \
        $(ABS_EGSPP)egs_input.h $(ABS_EGSPP)egs_base_source.h \
        $(ABS_EGSPP)egs_object_factory.h $(ABS_EGSPP)egs_timer.h \
        $(ABS_EGSPP)egs_rndm.h

dep_advanced_application = $(ABS_EGSPP)egs_advanced_application.cpp \
        $(ABS_EGSPP)egs_advanced_application.h array_sizes.h \
        $(common_h_files1) $(common_h_files2) \
        $(ABS_EGSPP)egs_input.h $(ABS_EGSPP)egs_base_source.h \
        $(ABS_EGSPP)egs_object_factory.h $(ABS_EGSPP)egs_timer.h \
        $(ABS_EGSPP)egs_application.h $(ABS_EGSPP)egs_run_control.h


dep_user_code = $(USER_CODE).cpp array_sizes.h $(common_h_files1) \
        $(ABS_EGSPP)$(EGS_BASE_APPLICATION).h \
        $(common_h_files2) $(other_dep_user_code)

dep_egs_interface = $(EGS_INTERFACE)egs_interface2.c $(common_h_files1)
