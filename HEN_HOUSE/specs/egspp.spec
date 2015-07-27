
###############################################################################
#
#  EGSnrc egspp configuration
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


# The directory of the egs++ libraries relative to HEN_HOUSE
EGSPP = egs++

# Relative path from the egs++ directory to the directory where object
# files and DSOs (DLLs) get put
dso = dso$(DSEP)$(my_machine)
DSO = $(dso)$(DSEP)

# The absolute path to the egs++ libraries
abs_dso = $(HEN_HOUSE)$(EGSPP)$(DSEP)$(dso)
ABS_DSO = $(abs_dso)$(DSEP)

# Ther relative path to the egs++ directory from the directories of
# individual geometry and source libraries.
DSOLIBS = ..$(DSEP)..$(DSEP)

# These get used in constructing file names
DSO1 = $(dso)$(DSEP)
DSO2 = $(DSOLIBS)$(DSO)

# The relative path to the directory where egs_config1.h resides starting from
# the egs++ directory
IEGS1 = ..$(DSEP)lib$(DSEP)$(my_machine)

# The relative path to the directory where egs_config1.h resides starting from
# the directories of the individual geometry and source libraries.
IEGS2 = $(DSOLIBS)$(IEGS1)

# The main egs++ library that all geometry and source libraries need to
# link against.
link2_libs = egspp

INC1 = -I$(IEGS1)
INC2 = -I$(IEGS2) -I..$(DSEP)..

common_shape_deps = egs_shapes.h egs_rndm.h egs_object_factory.h \
                    egs_transformations.h

common_source_deps = egs_base_source.h $(common_shape_deps)

lib_objects = $(addprefix $(DSO2), $(addsuffix .$(obje), $(lib_files)))

common_h = egs_vector.h egs_libconfig.h egs_input.h egs_base_geometry.h \
           egs_functions.h egs_math.h

common_h2 = $(addprefix $(DSOLIBS), $(common_h)) $(IEGS2)$(DSEP)egs_config1.h

extra_dep =

obj_rule1 = $(CXX) $(INC1) $(DEFS) $(opt) -c $(COUT)$@ $(notdir $(basename $@)).cpp
obj_rule2 = $(CXX) $(INC2) $(DEFS) $(opt) -c $(COUT)$@ $(notdir $(basename $@)).cpp

depend_rule = $(DSO2)$(1).$(obje): $(1).cpp $(1).h

make_depend = $(foreach a,$(lib_files),$(eval $(call depend_rule,$(a))))

lib_link2 = $(addprefix $(link2_prefix),$(addsuffix $(link2_suffix),$(link2_libs)))
