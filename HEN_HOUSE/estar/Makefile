###############################################################################
#
#  EGSnrc makefile for the ESTAR shared library
#  Copyright (C) 2026 National Research Council Canada
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
#  Author:          Reid Townson
#
#  Contributors:
#
###############################################################################

include $(EGS_CONFIG)
include $(SPEC_DIR)egspp.spec
include $(SPEC_DIR)egspp_$(my_machine).conf

DEFS = $(DEF1) -DBUILD_ESTAR_DLL

library  = estar
lib_files = estar estar_dataTables estar_dataParser estar_formulaCalcs

DSO2     = $(HEN_HOUSE)$(DSEP)$(EGSPP)$(DSEP)dso$(DSEP)$(my_machine)$(DSEP)
IEGS2    = $(HEN_HOUSE)lib$(DSEP)$(my_machine)
INC2     = -I$(IEGS2) -I$(ESTAR_DIR)
common_h =
link2_libs =
lib_link1 =

include $(SPEC_DIR)egspp_libs.spec

$(make_depend)

$(DSO2)estar.$(obje): $(ESTAR_DIR)estar.cpp \
        $(ESTAR_DIR)estar.h \
        $(ESTAR_DIR)estar_dataTables.h \
        $(ESTAR_DIR)estar_dataParser.h \
        $(ESTAR_DIR)estar_formulaCalcs.h

$(DSO2)estar_dataTables.$(obje): $(ESTAR_DIR)estar_dataTables.cpp \
        $(ESTAR_DIR)estar_dataTables.h

$(DSO2)estar_dataParser.$(obje): $(ESTAR_DIR)estar_dataParser.cpp \
        $(ESTAR_DIR)estar_dataParser.h \
        $(ESTAR_DIR)estar_dataTables.h

$(DSO2)estar_formulaCalcs.$(obje): $(ESTAR_DIR)estar_formulaCalcs.cpp \
        $(ESTAR_DIR)estar_formulaCalcs.h \
        $(ESTAR_DIR)estar_dataTables.h

clean:
	$(REMOVE) $(DSO2)estar.$(obje) \
	          $(DSO2)estar_dataTables.$(obje) \
	          $(DSO2)estar_dataParser.$(obje) \
	          $(DSO2)estar_formulaCalcs.$(obje)

.PHONY: clean
