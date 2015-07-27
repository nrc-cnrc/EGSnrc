
###############################################################################
#
#  EGSnrc egs++ configuration to build shared libraries
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
#  Contributors:
#
###############################################################################


.SUFFIXES: ;

ifeq ($(MACOSX),yes)

all: $(DSO2)$(libpre)$(library)$(libext) $(DSO2)$(libpre)$(library)$(libext_bundle)

$(DSO2)$(libpre)$(library)$(libext_bundle): $(lib_objects)
	$(CXX) $(INC2) $(DEFS) $(opt) $(shared_bundle) $(lib_link1) $(lib_objects) $(extra) $(lib_link2)

endif

$(DSO2)$(libpre)$(library)$(libext): $(lib_objects)
	$(CXX) $(INC2) $(DEFS) $(opt) $(shared) $(lib_link1) $(lib_objects) $(extra) $(lib_link2)

$(lib_objects): $(common_h2) $(extra_dep)

$(lib_objects):
	$(obj_rule2)

