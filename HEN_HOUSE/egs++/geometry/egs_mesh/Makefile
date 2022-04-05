###############################################################################
#
#  EGSnrc egs++ makefile to build the mesh geometry library
#
#  Copyright (C) 2022 Mevex Corporation
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
#  Authors:          Dave Macrillo,
#                    Matt Ronan,
#                    Nigel Vezeau,
#                    Lou Thompson,
#                    Max Orok
#
#  Revision:         Max Orok
#
###############################################################################

include $(EGS_CONFIG)
include $(SPEC_DIR)egspp.spec
include $(SPEC_DIR)egspp_$(my_machine).conf

DEFS = $(DEF1) -DBUILD_EGS_MESH_DLL

library = egs_mesh
lib_files = egs_mesh
my_deps = egs_transformations.h geometry$(DSEP)$(library)$(DSEP)msh_parser.h \
	geometry$(DSEP)$(library)$(DSEP)mesh_neighbours.h \
	geometry$(DSEP)$(library)$(DSEP)tetgen_parser.h

extra_dep = $(addprefix $(DSOLIBS), $(my_deps))

include $(SPEC_DIR)egspp_libs.spec

$(make_depend)

# unit tests
test_egs_mesh: test_egs_mesh.cpp egs_mesh.h egs_mesh.cpp msh_parser.h \
		mesh_neighbours.h \
        ../../egs_base_geometry.h \
        ../../egs_vector.h ../../../lib/$(my_machine)/egs_config1.h \
        ../../egs_libconfig.h ../../egs_functions.h $(ABS_DSO)$(libpre)egspp$(libext) \
        ../../dso/linux/libegs_mesh.so
	$(CXX) $(INC1) -I../../ -I../../../lib/$(my_machine)/ $(DEF1) $(opt) $(lib_link1) test_egs_mesh.cpp -otest_egs_mesh $(lib_link2) -legs_mesh

.PHONY: clean
clean:
	$(REMOVE) $(ABS_DSO)$(libpre)$(library)$(libext) $(DSO2)$(library).$(obje)
