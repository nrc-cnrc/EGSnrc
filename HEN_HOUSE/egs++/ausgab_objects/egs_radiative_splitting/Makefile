
###############################################################################
#
#  EGSnrc egs++ makefile to build radiative splitting object
#  Copyright (C) 2018 National Research Council Canada
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
#  Author:          Ernesto Mainegra-Hing, 2018
#
#  Contributors:
#
###############################################################################


include $(EGS_CONFIG)
include $(SPEC_DIR)egspp.spec
include $(SPEC_DIR)egspp_$(my_machine).conf

DEFS = $(DEF1) -DBUILD_RADIATIVE_SPLITTING_DLL

egs_interface = $(HEN_HOUSE)interface
EGS_INTERFACE = $(egs_interface)$(DSEP)

ABS_EGSPP = $(HEN_HOUSE)$(EGSPP)$(DSEP)

#required mortran files
CPP_SOURCES = $(EGS_SOURCEDIR)egsnrc.macros $(MACHINE_MACROS) \
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

dep_advanced_application = $(ABS_EGSPP)egs_advanced_application.cpp \
        $(ABS_EGSPP)egs_advanced_application.h array_sizes.h \
        $(ABS_EGSPP)egs_input.h $(ABS_EGSPP)egs_base_source.h \
        $(ABS_EGSPP)egs_object_factory.h $(ABS_EGSPP)egs_timer.h \
        $(ABS_EGSPP)egs_application.h $(ABS_EGSPP)egs_run_control.h

library = egs_radiative_splitting
lib_files = egs_radiative_splitting
my_deps = $(common_ausgab_deps)
extra_dep = $(addprefix $(DSOLIBS), $(my_deps)) egsnrc_$(my_machine).$(obje) egs_interface2_$(my_machine).$(obje) egs_advanced_application_lib_osx3.o
extra = egsnrc_$(my_machine).$(obje) egs_interface2_$(my_machine).$(obje) egs_advanced_application_lib_osx3.o
lib_link2 += $(fortran_libs)
interface_incl = -I. -I$(HEN_HOUSE)lib$(DSEP)$(my_machine) -I$(egs_interface)

include $(SPEC_DIR)egspp_libs.spec

$(DSO2)$(library).$(obje): $(library).cpp $(library).h

egs_advanced_application_lib_$(my_machine).$(obje): $(dep_advanced_application)
	$(CXX) $(interface_incl) $(DEF1) $(DEF_USER) $(opt) -c $(COUT)$@ $<

egs_interface2_$(my_machine).$(obje): $(EGS_INTERFACE)egs_interface2.c $(common_h_files1) array_sizes.h
	$(CXX) $(interface_incl) $(DEF1) $(DEF_USER) $(opt) -c $(COUT)$@ $<

egsnrc_$(my_machine).$(obje): egsnrc_$(my_machine).F array_sizes.h
	$(F77) $(FC_FLAGS) $(FDEFS) -c $(FOUT)$@ $<

egsnrc_$(my_machine).F: $(CPP_SOURCES)
	@echo "Mortran compiling EGSnrc sources ..."
	@$(MORTRAN_EXE) -s -i -d $(MORTRAN_DATA) -f $(CPP_SOURCES) -o7 $@ \
           -o8 $(@:.F=.mortlst)

test:
	@echo "common_h2: $(common_h2)"
	@echo "extra_dep: $(extra_dep)"
	@echo "CPP_SOURCES: $(CPP_SOURCES)"
	@echo "interface_incl: $(interface_incl)"
	@echo "extra: $(extra)"
	@echo "lib_objects: $(lib_objects)"
	@echo "lib_link1: $(lib_link1)"
	@echo "lib_link2: $(lib_link2)"
	@echo "def1: $(DEF1)"
	@echo "def_user: $(DEF_USER)"
	@echo "fc_flags: $(FC_FLAGS)"
