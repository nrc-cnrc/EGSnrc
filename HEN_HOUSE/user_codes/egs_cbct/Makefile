
###############################################################################
#
#  EGSnrc makefile to build egs++ egs_cbct application
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
#  Author:          Ernesto Mainegra-Hing, 2007
#
#  Contributors:    Iwan Kawrakow
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
USER_CODE = egs_cbct

# The following can be used to add user macros and mortran subroutines.
# The file(s) specified here are added after egsnrc.macros, machine.macros
# and egs_c_interface2.macros but before any files that have
# executable code.
#
EGSPP_USER_MACROS = egs_cbct.macros

# Specify from which base class this application is being derived.
# This has the effect of automatically compiling the base application
# class and including it into the list of object files.
#
EGS_BASE_APPLICATION = egs_advanced_application

# Specify the set of mortran sources used.
# Here we simply use the standard set defined in $HEN_HOUSE/specs/egspp1.spec
#
CPP_SOURCES = $(C_ADVANCED_SOURCES)

# Specify here other header files that your user code depends upon.
#
other_dep_user_code = $(ABS_EGSPP)egs_scoring.h

extra_user_files := egs_smoothing egs_utils egs_mortran egs_splitter egs_corrector

# User code defines
#
#DEF_USER = -DDEBUG_WEIGHTS
#DEF_USER = -DGDEBUG

common_egspp_h = egs_base_geometry.h egs_vector.h egs_math.h egs_functions.h

egs_cbct_deps = $(addprefix $(ABS_EGSPP), $(common_egspp_h) egs_advanced_application.h egs_application.h \
        egs_input.h egs_base_source.h egs_object_factory.h egs_rndm.h egs_transformations.h   \
	egs_interpolator.h) $(common_h_files1) egs_utils.h egs_cbct.h array_sizes.h

egs_smoothing_deps = $(addprefix $(ABS_EGSPP), egs_input.h egs_libconfig.h \
                     egs_functions.h) egs_smoothing.cpp egs_smoothing.h

egs_utils_deps = $(addprefix $(ABS_EGSPP), egs_input.h egs_libconfig.h \
                     egs_functions.h) egs_utils.h

egs_mortran_deps = $(addprefix $(ABS_EGSPP), egs_libconfig.h egs_functions.h) egs_mortran.h egs_cbct.h

egs_splitter_deps = $(addprefix $(ABS_EGSPP), egs_input.h egs_libconfig.h egs_functions.h) egs_splitter.h

egs_corrector_deps = $(addprefix $(ABS_EGSPP), egs_input.h egs_libconfig.h egs_functions.h) egs_corrector.h

include $(HEN_HOUSE)makefiles$(DSEP)cpp_makefile

all: $(target) smooth

smooth: $(USER_BINDIR)smooth$(EXE) ;

smooth3D: $(USER_BINDIR)smooth3D$(EXE) ;

$(USER_BINDIR)smooth$(EXE): smooth_$(my_machine).$(obje) egs_smoothing_$(my_machine).$(obje)
	$(CXX) $(opt) $(EOUT)$@ $^ $(lib_link1) $(link2_prefix)egspp$(link2_suffix)

$(USER_BINDIR)smooth3D$(EXE): smooth3D_$(my_machine).$(obje) egs_smoothing_$(my_machine).$(obje)
	$(CXX) $(opt) $(EOUT)$@ $^ $(lib_link1) $(link2_prefix)egspp$(link2_suffix)

egs_cbct_$(my_machine).$(obje): egs_cbct.cpp $(egs_cbct_deps)

egs_smoothing_$(my_machine).$(obje): egs_smoothing.cpp $(egs_smoothing_deps)

egs_utils_$(my_machine).$(obje): egs_utils.cpp $(egs_utils_deps)

egs_mortran_$(my_machine).$(obje): egs_mortran.cpp $(egs_mortran_deps)

egs_splitter_$(my_machine).$(obje): egs_splitter.cpp $(egs_splitter_deps)

egs_corrector_$(my_machine).$(obje): egs_corrector.cpp $(egs_corrector_deps)

smooth_$(my_machine).$(obje): smooth.cpp egs_smoothing.h
	$(object_rule)

smooth3D_$(my_machine).$(obje): smooth3D.cpp egs_smoothing.h
	$(object_rule)

realclean: clean
	rm -f $(USER_BINDIR)smooth$(EXE) smooth_$(my_machine).$(obje) smooth3D_$(my_machine).$(obje)

.PHONY: smooth smooth3D realclean
