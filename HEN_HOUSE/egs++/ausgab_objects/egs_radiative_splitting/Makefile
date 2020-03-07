
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

library = egs_radiative_splitting
lib_files = egs_radiative_splitting
my_deps = $(common_ausgab_deps)
extra_dep = $(addprefix $(DSOLIBS), $(my_deps))

include $(SPEC_DIR)egspp_libs.spec

$(DSO2)$(library).$(obje): $(library).cpp $(library).h

egsnrc_$(my_machine).$(obje): egsnrc_$(my_machine).F array_sizes.h
	$(F77) $(FC_FLAGS) $(FDEFS) -c $(FOUT)$@ $<

egsnrc_$(my_machine).F: $(CPP_SOURCES)
	@echo "Mortran compiling EGSnrc sources ..."
	@$(MORTRAN_EXE) -s -i -d $(MORTRAN_DATA) -f $(CPP_SOURCES) -o7 $@ \
	-o8 $(@:.F=.mortlst)

test:
	@echo "common_h2: $(common_h2)"
	@echo "extra_dep: $(extra_dep)"
