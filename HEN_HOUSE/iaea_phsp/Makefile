
###############################################################################
#
#  EGSnrc makefile for the IAEA phase space reading/writing shared library
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
#  Author:          Iwan Kawrakow, 2007
#
#  Contributors:    Ernesto Mainegra-Hing
#                   Blake Walters
#
###############################################################################


include $(EGS_CONFIG)
include $(SPEC_DIR)egspp.spec
include $(SPEC_DIR)egspp_$(my_machine).conf

DEFS = $(DEF1) -DBUILD_IAEA_DLL

library = iaea_phsp
lib_files = iaea_phsp iaea_header iaea_record  utilities
my_common_deps = utilities.h iaea_config.h

DSO2 = ..$(DSEP)$(EGSPP)$(DSEP)dso$(DSEP)$(my_machine)$(DSEP)
IEGS2 = ..$(DSEP)lib$(DSEP)$(my_machine)
INC2 = -I$(IEGS2)
common_h =
link2_libs =
lib_link1 =

include $(SPEC_DIR)egspp_libs.spec

$(make_depend)

$(DSO2)iaea_phsp.$(obje): iaea_record.h iaea_header.h $(my_common_deps)

$(DSO2)iaea_record.$(obje): $(my_common_deps)

$(DSO2)iaea_header.$(obje): iaea_record.h $(my_common_deps)

clean:
	$(REMOVE) $(DSO2)iaea_phsp.$(obje) $(DSO2)iaea_header.$(obje) $(DSO2)iaea_record.$(obje)

.PHONY: clean
