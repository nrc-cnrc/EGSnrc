###############################################################################
#
#  EGSnrc egs++ makefile to build mevex tet collection geometry
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
#  Author:          MJR, 2018
#
# Contributors: DMM
#
###############################################################################


include $(EGS_CONFIG)
include $(SPEC_DIR)egspp.spec
include $(SPEC_DIR)egspp_$(my_machine).conf

DEFS = $(DEF1) -DBUILD_MEVEX_TET_COLLECTION_DLL

library = egs_mesh
lib_files = egs_mesh
my_deps = egs_transformations.h
extra_dep = $(addprefix $(DSOLIBS), $(my_deps))

include $(SPEC_DIR)egspp_libs.spec

$(make_depend)
