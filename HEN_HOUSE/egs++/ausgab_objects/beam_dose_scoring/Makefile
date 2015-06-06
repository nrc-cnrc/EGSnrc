
###############################################################################
#
#  EGSnrc egs++ makefile to build beam dose scoring object
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
#  Author:          Blake Walters, 2014
#
#  Contributors:
#
###############################################################################


include $(EGS_CONFIG)
include $(SPEC_DIR)egspp.spec
include $(SPEC_DIR)egspp_$(my_machine).conf

DEFS = $(DEF1) -DBUILD_DOSE_SCORING_DLL

library = beam_dose_scoring
lib_files = beam_dose_scoring
my_deps = $(common_ausgab_deps)
extra_dep = $(addprefix $(DSOLIBS), $(my_deps))

#redefined from egspp.spec
INC2 = -I$(IEGS2) -I..$(DSEP).. -I$(HEN_HOUSE)/user_codes/beampp -I$(HEN_HOUSE)interface

include $(SPEC_DIR)egspp_libs.spec

$(make_depend)

test:
	@echo "common_h2: $(common_h2)"
	@echo "extra_dep: $(extra_dep)"
	@echo "depend_rule: $(depend_rule)"
