
###############################################################################
#
#  EGSnrc makefile to build egs++ egs_fac application
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
#  Author:          Iwan Kawrakow, 2008
#
#  Contributors:
#
###############################################################################


include $(EGS_CONFIG)
include $(SPEC_DIR)egspp1.spec
include $(SPEC_DIR)egspp_$(my_machine).conf

# Specify the name of the user code.
# The name of the executable is determined from this variable.
#
USER_CODE = egs_fac

# The following can be used to add user macros and mortran subroutines.
# The file(s) specified here are added after egsnrc.macros, machine.macros
# and egs_c_interface2.macros but before any files that have
# executable code.
#
EGSPP_USER_MACROS = egs_fac.macros

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


extra_user_files := egs_mortran egs_range_rejection egs_fac_simulation

# Specify here other header files that your user code depends upon.
#
other_dep_user_code = $(ABS_EGSPP)egs_scoring.h

common_egspp_h = egs_base_geometry.h egs_vector.h egs_math.h egs_functions.h

egs_fac_deps = $(addprefix $(ABS_EGSPP), $(common_egspp_h) egs_advanced_application.h egs_application.h \
        egs_input.h egs_base_source.h egs_object_factory.h egs_rndm.h egs_transformations.h egs_interpolator.h) \
        $(common_h_files1) egs_fac.h array_sizes.h egs_mortran.h egs_range_rejection.h \
        egs_fac_simulation.h

egs_mortran_deps = $(addprefix $(ABS_EGSPP), egs_advanced_application.h \
        egs_application.h $(common_egspp_h)) $(common_h_files1) egs_mortran.h array_sizes.h egs_fac.h

egs_fac_simulation_deps = $(addprefix $(ABS_EGSPP), egs_input.h egs_transformations.h \
        $(common_egspp_h)) $(common_h_files1) egs_fac_simulation.h

egs_range_rejection_deps = $(addprefix $(ABS_EGSPP), egs_interpolator.h egs_input.h \
        egs_rndm.h $(common_egspp_h)) $(common_h_files1)\
        egs_range_rejection.h array_sizes.h egs_mortran.h

# User code defines
#
#DEF_USER = -DCAVITY_DEBUG

include $(HEN_HOUSE)makefiles$(DSEP)cpp_makefile

egs_fac_$(my_machine).$(obje): egs_fac.cpp $(egs_fac_deps)

egs_mortran_$(my_machine).$(obje): egs_mortran.cpp $(egs_mortran_deps)

egs_fac_simulation_$(my_machine).$(obje): egs_fac_simulation.cpp $(egs_fac_simulation_deps)

egs_range_rejection_$(my_machine).$(obje): egs_range_rejection.cpp $(egs_range_rejection_deps)

